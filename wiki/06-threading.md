# 06 - Threading & Concurrency

> **Scope of this page.** The single source of truth for what is
> safe to call concurrently across the public HEIF surfaces - the
> raw C++ reader/writer (`heifreader.h` / `heifwriter.h`), the C
> ABI reader facade (`heifreader_c.h`; no writer C facade in
> v4.0.0), HEIFPP (`HEIFPP::Heif`), and the JNI bridge. Covers the
> factory, the live instance, the thread-local error slot, the
> process-wide sink, the custom allocator, and the custom-stream
> callbacks.
>
> If you only have ten seconds:
> **One `Reader` instance per consumer thread.** Everything else
> the library does - slot, sink, allocator, stream callbacks - is
> documented below; the rule above keeps you out of every sharp
> edge.

---

## What this page covers

1. The four-axis contract (factory · instance · slot · sink).
2. The custom allocator and custom-stream callbacks.
3. Embedder patterns (single-threaded, thread-per-instance,
   pool, server) - copy-paste recipes.
4. Anti-patterns the library will not protect you from.
5. Where to look in the source if you need to verify any of this.

---

## What you need before starting

* Familiarity with the error-reporting model
  ([03 - Error Handling](03-error-handling.md)) - this page builds
  on the slot/sink terminology established there.
* A reading knowledge of the public headers under
  `srcs/api/common/`, `srcs/api/reader/`, `srcs/api/writer/`.

---

## 1. The contract, on one screen

| Surface | Thread-safe? |
|---|:---:|
| `Reader::Create()` / `Destroy()` | YES |
| `Writer::Create()` / `Destroy()` | YES |
| `heif_reader_create()` / `heif_reader_destroy()` | YES |
| A live `Reader*` / `Writer*` / `heif_reader_t*` instance | **NO** |
| `Reader::SetCustomAllocator()` | Once at startup only |
| `Reader::GetVersion()` | YES |
| Error slot (`getErrorInfo()` / `heif_get_last_error_*`) | YES (thread-local) |
| `setErrorSink()` (the setter) | YES |
| The sink callback (your function) | May fire on any thread |
| `StreamInterface*` / `heif_stream_callbacks_t` | Same-thread only |
| `HEIFPP::Heif::load()` / `save()` | **NO** |
| Java `com.nokia.heif.Heif` | **NO** |

The four-word summary: **factory-safe, instance-unsafe, slot-thread-local, sink-may-fire-anywhere**.

Details on each row:

- **Factory (`Create` / `Destroy`):** Fully reentrant. The library
  does not retain any reference to the returned instance. The
  C-facade handle registry is a process-wide synchronised set, so
  concurrent create/destroy on different handles is fine.

- **Live instance:** One instance, one thread. Multiple instances
  may exist and be in use at the same moment, each on its own
  thread.

- **`SetCustomAllocator()`:** Set before the first `Create()` and
  never reset while any `Reader` / `Writer` is alive on any thread.
  See §3.

- **Error slot:** `thread_local`. Each thread sees only its own
  slot. Drain on the *same* thread that observed the non-OK return.

- **`setErrorSink()` setter:** The sink + user pair is published
  atomically. Calls in flight may briefly observe the previous pair,
  but never a torn pair.

- **Sink callback:** Invoked **on the failing thread**. If your sink
  writes to a shared logger, the logger must itself be thread-safe.
  See §4.

- **Stream callbacks:** The library never calls your callbacks
  concurrently on a single instance. A `StreamInterface*` may be
  shared across instances only if your implementation is itself
  thread-safe. See §5.

- **HEIFPP:** Same per-instance rule as `Reader`. Additionally,
  `Heif` may throw `std::logic_error` on contract violation; install
  one top-level `try/catch` per consumer thread.

- **Java:** Inherits the per-instance rule from HEIFPP. Use one
  `Heif` object per JVM thread. The JNI shim itself is thread-safe
  (entry-clear is atomic, exception translation is thread-local).

---

## 2. Why the per-instance rule

A `Reader` (and the C-facade `heif_reader_t*` that wraps it) holds
mutable internal state - the parsed metabox, several lazily-built
caches keyed on `ImageId` / `PropertyId` / `GroupId`, the
single-slot `cachedFileInfo`, and (when initialised via
`heif_reader_initialize_stream`) a borrowed pointer into the
caller's stream-callback bridge. None of this state is
synchronised. Two threads issuing reads against the same instance
can race on cache fills, the bitstream cursor inside `getItemData`,
or the error-slot-clearing path on entry.

Adding a mutex per instance was considered and rejected: most
embedders are already single-threaded per consumer (one decoder
thread per HEIF), and those who are not get no benefit from a
per-instance lock - they would still serialise all access through
that one lock. The right shape is **one instance per thread**,
which gives perfect parallelism for free.

---

## 3. The custom allocator

`HEIF::Reader::SetCustomAllocator()` is a **one-shot, process-wide
configuration call**. It is intended to run once at process start,
before any `Reader` or `Writer` is created.

The supported lifecycle is:

```cpp
// At process startup, before any HEIF::Reader::Create() call:
HEIF::Reader::SetCustomAllocator(&myAllocator);

// ... rest of program. Many threads may now Create / Destroy
// freely; every allocation routes through myAllocator.
```

Resetting the allocator at runtime is a narrow, single-threaded
operation: per the header comment on `SetCustomAllocator`, you
must first set it to `nullptr` (always succeeds), then set the new
pointer. Doing this while any other thread is inside a `Reader` /
`Writer` method is **undefined**: an in-flight allocation may
land on the old allocator, while the matching deallocation
discovers the new one.

If you need per-thread allocators, the recommendation is
*per-process partitioning* - use a different process, or wrap the
library so each consumer thread gets its own arena layered above
the single process-wide allocator.

---

## 4. The error sink

The sink is the only library callback that can fire on a thread
the embedder does not control directly. The contract is:

* **Setter is safe to call at any time** from any thread. The
  `(callback, user)` pair is published atomically; calls already
  in flight either see the new pair or the old pair, never a
  half-updated one.
* **Callback is invoked on the failing thread** - same call
  stack as the C++ throw site that triggered the diagnostic. Do
  not assume any particular thread identity.
* **No re-entry by default.** If your sink itself calls back into
  HEIF and that call also fails, the diagnostic from the inner
  call is suppressed (the sink would otherwise infinitely
  recurse). The slot is still populated for the inner call;
  callers can still drain it via `HEIF::getErrorInfo()`.
* **Side-channel only.** Every public entry point still returns
  `ErrorCode` and populates the slot. A sink that throws or
  takes a long time degrades the error path of the call that
  triggered it; it does not change the return value.
* **Sink lifetime.** The library does not own the callback or
  `user` pointer. Both must outlive every possible failure path,
  i.e. typically the entire process. If you need to remove a
  sink, set it to `nullptr` and let in-flight calls drain
  (atomic-load discipline guarantees no torn pair).

If your sink writes to a shared destination - a file, a logger,
`stderr`, a message queue - that destination must itself be
thread-safe. The sink does not serialise calls.

A reasonable production sink looks like:

```cpp
static std::mutex g_logMutex;

void mySink(const HEIF::ErrorInfo& info, void* user) noexcept {
    std::lock_guard<std::mutex> lk(g_logMutex);
    auto* fp = static_cast<FILE*>(user);
    char buf[1024];
    HEIF::formatErrorInfo(info, buf, sizeof(buf));
    std::fputs(buf, fp);
    std::fputc('\n', fp);
}

// At startup:
HEIF::setErrorSink(&mySink, stderr);
```

---

## 5. Custom stream callbacks

`HEIF::Reader::initialize(StreamInterface*)` and the C-facade
`heif_reader_initialize_stream(callbacks)` both let the embedder
provide its own byte-source. The thread-safety rules:

* The library calls your callbacks **only from the thread that
  called the `Reader` method**, never concurrently on a single
  instance. So *for the lifetime of one method call* you may
  treat your callbacks as single-threaded.
* The library also never holds a callback across method calls, 
  the bridge is fully drained at each return, so reentry from a
  signal handler or a different thread between calls is safe to
  the extent your stream implementation itself is.
* If you share **one** `StreamInterface*` across **multiple**
  `Reader` instances on **multiple** threads, your implementation
  must itself be thread-safe (mutex-protected state, atomic
  cursors, or whatever is appropriate for the underlying
  storage). The library does no synchronisation here.
* The C-facade `heif_stream_callbacks_t` is a *callback table*
  (function pointers + opaque `user_data`); the ban on function
  pointers in shared structs does not apply here because the struct
  is consumed by the library, not returned to the caller.
* The bridge that adapts `heif_stream_callbacks_t` =>
  `HEIF::StreamInterface` is owned by the `ReaderHandle` and
  destroyed *after* the underlying `Reader` in
  `heif_reader_destroy`, so the bridge outlives every callback
  the library can issue.

---

## 6. Embedder patterns

### 6.1 Single-threaded consumer

Default. No special handling.

```cpp
auto* r = HEIF::Reader::Create();
r->initialize("photo.heic");
// ... use r ...
HEIF::Reader::Destroy(r);
```

### 6.2 Thread-per-instance

The recommended multi-threaded pattern. Each thread holds its own
`Reader`. No locks, no contention.

```cpp
void worker(const std::string& path) {
    auto* r = HEIF::Reader::Create();
    if (r->initialize(path.c_str()) != HEIF::ErrorCode::OK) {
        // error slot is on this thread; drain via getErrorInfo()
        HEIF::Reader::Destroy(r);
        return;
    }
    // ... do all work for `path` on this thread ...
    HEIF::Reader::Destroy(r);
}

std::vector<std::thread> pool;
for (auto& path : files) pool.emplace_back(worker, path);
for (auto& t : pool) t.join();
```

### 6.3 Pool of reusable instances

When `Create()` cost matters (rare - it allocates a few KiB and
sets some defaults), keep a pool:

```cpp
class ReaderPool {
    std::mutex mu;
    std::stack<HEIF::Reader*> free;
public:
    HEIF::Reader* acquire() {
        std::lock_guard<std::mutex> lk(mu);
        if (!free.empty()) { auto* r = free.top(); free.pop(); return r; }
        return HEIF::Reader::Create();
    }
    void release(HEIF::Reader* r) {
        r->close();                       // reset reader state
        std::lock_guard<std::mutex> lk(mu);
        free.push(r);
    }
    ~ReaderPool() {
        while (!free.empty()) { HEIF::Reader::Destroy(free.top()); free.pop(); }
    }
};
```

Note that `close()` is the right reset point - it returns the
instance to the pre-`initialize()` state without dropping the
allocation.

### 6.4 Server / request-per-instance

Web servers, RPC dispatchers, image-processing pipelines. The
right shape is *one instance per request*, with the request's
worker thread owning create + use + destroy. Lifetime matches
the request's HTTP context; no cross-request sharing.

```cpp
void handleRequest(const Request& req, Response& resp) {
    auto* r = HEIF::Reader::Create();
    if (r->initialize(req.path.c_str()) != HEIF::ErrorCode::OK) {
        HEIF::ErrorInfo info = HEIF::getErrorInfo();   // thread-local
        resp.send5xx(info);
        HEIF::Reader::Destroy(r);
        return;
    }
    // ... build response ...
    HEIF::Reader::Destroy(r);
}
```

If you also want a process-wide sink for log aggregation, install
it once at server startup; the per-request error drain through the
slot is independent and unaffected.

---

## 7. Common problems

### "I'm getting random `UNINITIALIZED` errors under load"

You're sharing one `Reader*` across threads. Look for the variable
in your code that holds the `Reader*` - if it's a global, a
singleton, a class member of a server-wide object, or anything
that is not thread-local / per-request, that's the bug. Move it
into the worker function or use the pool pattern (§6.3).

### "My sink fires from multiple threads simultaneously and corrupts my log"

The sink is invoked on the failing thread. If multiple threads
fail in parallel, your sink runs in parallel. Wrap the destination
in a mutex (§4 example) or write to a per-thread buffer that an
aggregator drains.

### "I changed `SetCustomAllocator` mid-run and the program crashed"

Don't. The header comment is normative: set once before the first
`Create()`, or set to `nullptr` and back only when no `Reader` /
`Writer` is alive on any thread.

### "JVM segfaults from the JNI bridge"

Check that your Android NDK build includes RTTI (`-frtti`) and
exceptions (`-fexceptions`) - see [04 - Java & Android](04-java-and-android.md).
A bridge that cannot translate the C++ exception type ends up
calling `std::terminate`, which the JVM logs as SIGSEGV. This is
not a thread-safety issue but it's the failure mode that gets
misdiagnosed as one most often.

### "My callback table for `heif_reader_initialize_stream` was deleted before destroy"

The library never retains the table past
`heif_reader_initialize_stream` *return* - the callback functions
themselves must remain valid for the life of the reader, but the
`heif_stream_callbacks_t` *struct* may be a stack local. The
bridge captures the function pointers and the `user_data` by
value at init time.

---

## 8. Where to verify in the source

| Claim | Source of truth |
|---|---|
| Per-instance "not safe" rule | `srcs/api/reader/heifreader.h` class brief (normative table) |
| C facade per-instance rule | `srcs/api/reader/heifreader_c.h` top-of-file comment |
| Slot is `thread_local` | `srcs/api/common/heiferrorinfo.h` - `THREADING` block |
| Sink atomic publication | `srcs/api/common/heiferrorinfo.h` - sink section + `srcs/common/heiferrorinfo.cpp` |
| Stream-callback bridge ownership | `srcs/api/reader/heifreader_c.h` `heif_stream_callbacks_t` comment + `srcs/reader/heifreader_c.cpp` `StreamCallbackBridge` |
| Handle-registry concurrency | `srcs/reader/heifreader_c.cpp` - `isLiveHandle` / live-set definitions |
| HEIFPP exception contract | `srcs/api-cpp/Heif.h` ownership block |
| JNI translator | `srcs/api-java/cpp/Helpers.cpp` |

If any of these disagree with the wiki, **the source wins**, 
file a bug noting the divergence. The wiki should be updated to
match, not the other way round.

---

## 9. Where to go next

* [03 - Error Handling](03-error-handling.md) - the slot/sink
  model this page builds on.
* [02 - Linking Your Project](02-linking-your-project.md)
  - the per-target compile/link settings (RTTI, exceptions) that
  the JNI / Obj-C++ bridges depend on.
* `bindings/python/heif/_reader.py` - a working consumer that
  obeys the per-instance rule (the `Reader` Python class wraps
  one `heif_reader_t*` per instance and never shares it).
* [07 - Writing a Language Binding](07-binding-authors.md) - if
  you are wrapping the C facade in another language, this page's
  §2.3 tells you exactly which parts of the threading contract
  your binding has to surface verbatim.
