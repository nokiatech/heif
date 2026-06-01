# 07 - Writing a Language Binding

> **Scope of this page.** A how-to for anyone wrapping the HEIF C
> facade (`srcs/api/reader/heifreader_c.h` + `srcs/api/common/heif_error_c.h`)
> in another language: Rust, Go, .NET, Swift, Ruby, Node, Lua, or
> anything else. Distils every lesson the Python binding learned
> the hard way so you don't have to relearn them.
>
> If you only have ten seconds:
> **Bind the C facade, not HEIFPP. One handle per thread. Drain
> the error slot on every non-OK return. Pin to the SO-version major.**
> Everything else on this page is justification and detail.

---

## What this page covers

1. Why the C facade is the only sensible bind target.
2. The seven things every binding has to get right.
3. A worked reference - the Python binding under `bindings/python/`.
4. A pre-flight checklist for new bindings.
5. ABI / SO-version pinning rules.
6. Where to ask when the contract is unclear.

---

## What you need before starting

* The C facade header (`srcs/api/reader/heifreader_c.h`), which
  lists the full entry-point inventory.
* The error model ([03 - Error Handling](03-error-handling.md))
  - the error slot is non-negotiable for any binding.
* The thread-safety contract ([06 - Threading & Concurrency](06-threading.md))
  - every binding has to expose this contract verbatim or restrict it further.
* A built `libheif_shared.so.4` (or `.dll` / `.dylib`).

---

## 1. Bind the C facade - never HEIFPP directly

The library exposes three layers:

| Layer | Header | Stable across versions? | Bind it? |
|---|---|:---:|:---:|
| C ABI facade | `srcs/api/reader/heifreader_c.h` | **YES** - SO-major bumps signal break | YES **always** |
| HEIFPP C++ wrapper | `srcs/api-cpp/Heif.h` | No - symbol mangling churns | NO |
| Raw C++ `HEIF::Reader` | `srcs/api/reader/heifreader.h` | Source-stable within a minor, but **not** binary-stable across compilers / standard libraries | NO |

Only the C facade has:

* A documented ABI policy (tied to `SOVERSION`).
* `extern "C"` linkage with stable mangled names.
* Plain-old-data structs and integer error codes - no exceptions
  crossing the FFI boundary.
* A normative thread-safety contract ([06](06-threading.md)).
* A normative error-reporting contract ([03](03-error-handling.md)).

The raw C++ `HEIF::Reader` and HEIFPP are public C++ APIs (both
are documented in the Doxygen reference), but neither offers a
binary-stability promise: vtable layout, exception specs, and the
signatures of the project-internal `HEIF::Array<T>` /
`HEIF::String` types are free to change between minor versions.
Binding against them will work today and break in v4.1.0.

---

## 2. The seven things every binding has to get right

Each one is something the Python binding got wrong on the
first pass and corrected after testing surfaced the problem.

### 2.1 - Lifecycle: own the handle, expose RAII

**Contract.** `heif_reader_create()` returns an opaque pointer.
You own it until you call `heif_reader_destroy()`. After destroy,
the handle is dead: calling any other entry point with it will
return `INVALID_HANDLE` (use-after-destroy guard).

**Binding obligation.** Wrap the handle in your language's
deterministic-cleanup primitive:

| Language | Idiom |
|---|---|
| C++ | `std::unique_ptr<heif_reader_t, decltype(&heif_reader_destroy)>` |
| Rust | `Drop` impl on a wrapper struct holding `*mut heif_reader_t` |
| Go | `runtime.SetFinalizer` + explicit `Close()` returning `error` |
| Python | `__enter__`/`__exit__` + `__del__` fallback (see `bindings/python/heif/_reader.py`) |
| C# / .NET | `IDisposable` + `SafeHandle` |
| Java | `AutoCloseable` (see existing `com.nokia.heif.Heif`) |
| Swift | `deinit` on a `final class` |

**Always provide both** an explicit `Close` / `dispose` *and* a
GC-driven fallback. Never rely on GC alone - the underlying
library may hold OS resources (file descriptors, mmap regions).

**Initialisation failure.** If `heif_reader_initialize_*()` returns
non-OK, you **must** call `heif_reader_destroy()` before raising
your binding's exception. The Python binding does this in a
`try/except` around the initialise call; the C `Reader::Create()`
factory itself is allocation-only and cannot leave you with a
half-initialised handle, so destroy is always safe.

### 2.2 - Error handling: drain the error slot on every non-OK return

**Contract.** Every C entry point returns `int32_t` - `0` on
success, non-zero on failure. On non-zero, the thread-local error
slot has been populated with `(code, code_name, origin, origin_name,
file, line, message, cause)`. You drain it via the ten
`heif_get_last_error_*` accessors. The slot is **thread-local**, 
read it on the same thread that observed the non-OK return.

**Binding obligation.** Build one shared helper that:

1. Takes the integer return code and a binding-side context string
   (e.g. `"get_property_pixi_channel_label(property_id=4, channel_index=2)"`).
2. If `rc == 0`, returns immediately.
3. Otherwise, snapshots all eight slot fields into a structured
   value (`dict` / `struct` / record).
4. Calls `heif_clear_last_error()` so the slot does not leak into
   the next call.
5. Raises your binding's exception type, populated from the
   snapshot.

The Python binding implements this as `_check(rc, where)` in
`bindings/python/heif/_reader.py` - 11 lines, called by every
fallible C entry point. The exception type (`HeifError`) carries
all eight fields as attributes so consumers can branch on
`code_name` (a stable string identifier) rather than the integer
code (which is renumbering-stable today but not guaranteed
across major versions).

**The size-probe pattern.** Variable-length getters
(`heif_get_last_error_message`, `heif_reader_get_*_bitstream`,
…) follow the snprintf-style protocol:

1. Call with `(NULL, 0)` to learn the required size - returns
   either `OK` with the size written to `*size_out`, **or**
   `BUFFER_SIZE_TOO_SMALL` with the size written to `*size_out`.
   Both branches are valid; treat them identically.
2. Allocate a buffer of `required + 1` bytes (the +1 covers the
   NUL terminator on string getters; harmless on byte-buffer
   getters).
3. Call again with the real buffer.

Build one helper for this dance too - the Python binding has
`_fetch_bytes_via_probe()`. Every binding will need an equivalent.

> **For the cross-layer view** (how the same protocol surfaces in the
> raw C++ API, in HEIFPP, in JNI, and what it would look like in
> wasm), see [08 - Byte-Fetch Protocol](08-byte-fetch-protocol.md).
> That page is the canonical reference; this section only covers the
> binding-author slice.

**The "expected non-error" pattern.** A small set of returns are
not really errors - `PRIMARY_ITEM_NOT_SET` from
`heif_reader_get_primary_item` is the canonical example. Match
on `code_name` (a stable string), not on the integer code, and
clear the slot before returning the "not present" value to the
caller. See the `primary_image` property in `_reader.py` for the
canonical shape.

### 2.3 - Threading: expose the per-instance rule verbatim

**Contract.** [06 - Threading & Concurrency](06-threading.md).

**Binding obligation.** Document the same contract in your
binding's docs - do **not** weaken it (e.g. by saying "thread-safe
when called serially") and do **not** silently strengthen it (e.g.
by hiding a per-handle mutex inside the binding). Either is a lie
the embedder will pay for in production.

If your language has a "send-between-threads" / "share-between-
threads" type system (Rust `Send` / `Sync`, .NET `ConcurrentXxx`,
…) this is where it earns its keep:

| Type | `Send` (move to another thread) | `Sync` (use from many threads) |
|---|:---:|:---:|
| `Reader` handle wrapper | YES | **NO** |

For Rust specifically: `impl Send for Reader {}` is correct;
`impl Sync for Reader {}` is wrong. The same shape applies to any
language with a similar split (Swift `Sendable`, Kotlin coroutine
context, …).

The error sink - set via `HEIF::setErrorSink()` - fires on the
**failing thread**, so any binding-side global the sink writes to
(a logger, a span, a structured-event channel) must itself be
thread-safe. Document this on the sink-registration entry point
in your binding's docs.

### 2.4 - Custom allocator: at most once, at startup

**Contract.** `HEIF::Reader::SetCustomAllocator()` is process-wide
and one-shot. See [06 §3](06-threading.md#3-the-custom-allocator).

**Binding obligation.** Most bindings should **not** expose the
custom allocator at all - it is a niche feature for embedded /
constrained-memory targets. If you do expose it, gate it behind
an explicit `unsafe`-equivalent (Rust `unsafe fn`, C# `[Obsolete]`
attribute with rationale, Python module-level function not class
method) and document the one-shot semantics on the entry point.

Do **not** wrap the allocator in your binding's GC-aware
allocator - the library calls it from many threads with no
binding-side context, and any bridge that reentrantly enters
your runtime's GC will deadlock the first time the allocator
fires under collection pressure.

### 2.5 - Custom stream callbacks: lifetime is yours

**Contract.** The `heif_stream_callbacks_t` struct
(read / size / seek / tell + a `void* user`) is borrowed by the
library for the life of the `heif_reader_t*` instance that
consumed it. The library never stores it past `heif_reader_destroy()`.

**Binding obligation.**

1. Keep the callback struct **and** the `user` payload alive
   until the reader is destroyed. The simplest safe pattern is
   to store both as members on the binding's reader-wrapper
   object so they live for the same scope as the C handle.
2. The library calls your callbacks **single-threaded per
   instance** (see [06 §5](06-threading.md#5-custom-stream-callbacks)),
   so no internal synchronisation is needed for one reader.
   Sharing one `heif_stream_callbacks_t` across multiple
   readers is allowed only if your callback bodies are
   themselves thread-safe.
3. Marshal exceptions thrown from your callback bodies into a
   non-OK return code. **Never let an exception escape into C.**
   The library has no `noexcept` guard on callback edges.

### 2.5.1 - Segment streaming: multi-stream lifetime windows

**Contract.** Fragmented / DASH-style playback uses an opaque
`heif_stream_t*` handle (constructed via `heif_stream_create`
from a `heif_stream_callbacks_t` table, destroyed via
`heif_stream_destroy`) and four entry points:

| Entry point | Stream lifetime requirement |
|---|---|
| `heif_reader_parse_initialization_segment(reader, stream)` | The library retains `stream` (and its `user_data`) until `heif_reader_destroy(reader)`. |
| `heif_reader_parse_segment(reader, stream, segment_id, earliest_pts_in_ts)` | The library retains `stream` (and its `user_data`) until the matching `heif_reader_invalidate_segment(reader, segment_id)`. |
| `heif_reader_invalidate_segment(reader, segment_id)` | After this returns OK the matching `parse_segment` stream MAY be destroyed. |
| `heif_reader_parse_segment_index(reader, stream, out, *count_inout)` | The library uses `stream` only for the duration of this call; the caller MAY destroy it immediately after the function returns. |

`heif_stream_create` takes a *copy* of the callback table, 
the caller may free the `heif_stream_callbacks_t` immediately
after the constructor returns. The `user_data` pointer it
contains, however, is held by reference for the handle's
entire lifetime; whatever your `user_data` points at must
outlive the handle.

`earliest_pts_in_ts = UINT64_MAX` matches the C++ default-arg
behaviour (unset; only meaningful when feeding a partial
segment without an `sidx` box).

**Why this is harder than §2.5.** The single-stream path
keeps one stream alive for the whole reader lifetime - that
maps cleanly onto "store as a member on the reader wrapper."
Segment streaming is different: the reader holds raw pointers
into **N** caller-owned stream handles concurrently
(one init segment + one per active media segment), and each
has a different drop-window. A garbage-collected runtime that
frees the `user_data` mid-parse - because the wrapper went
out of scope or the GC ran - crashes the library on the
next callback into that stream. There is no `noexcept`
firewall on the callback edge to catch this.

**Recommended pattern (every GC'd runtime).** The reader
wrapper owns a strong reference to every stream handle it has
been handed, and drops each one only when its window closes:

```python
# Python (cffi); pseudocode for illustration.
class Reader:
    def __init__(self):
        self._c = lib.heif_reader_create()
        self._init_stream = None             # one init segment, lifetime = self
        self._segment_streams = {}           # {segment_id: heif_stream_t*}, dropped on invalidate

    def parse_initialization_segment(self, src):
        cb, ud = _build_callbacks(src)       # cb owns refs to the four C function pointers
        s = lib.heif_stream_create(cb)
        if s == ffi.NULL: raise RuntimeError(...)
        # Strong refs MUST outlive the C handle:
        self._init_stream = (s, cb, ud)
        _check(lib.heif_reader_parse_initialization_segment(self._c, s))

    def parse_segment(self, segment_id, src, earliest_pts_in_ts=2**64 - 1):
        cb, ud = _build_callbacks(src)
        s = lib.heif_stream_create(cb)
        if s == ffi.NULL: raise RuntimeError(...)
        self._segment_streams[segment_id] = (s, cb, ud)
        _check(lib.heif_reader_parse_segment(self._c, s, segment_id, earliest_pts_in_ts))

    def invalidate_segment(self, segment_id):
        _check(lib.heif_reader_invalidate_segment(self._c, segment_id))
        # Safe to drop the stream now - library has released its raw pointer:
        held = self._segment_streams.pop(segment_id, None)
        if held is not None:
            lib.heif_stream_destroy(held[0])

    def close(self):
        # Tear down reader BEFORE destroying its streams (reader still holds
        # raw pointers to init_stream + any non-invalidated segment streams):
        if self._c is not None:
            lib.heif_reader_destroy(self._c)
            self._c = None
        for held in self._segment_streams.values():
            lib.heif_stream_destroy(held[0])
        self._segment_streams.clear()
        if self._init_stream is not None:
            lib.heif_stream_destroy(self._init_stream[0])
            self._init_stream = None
```

The same shape applies to **JavaScript / wasm** (Embind
or hand-rolled FFI):

```js
// JS (wasm); pseudocode for illustration.
class Reader {
    constructor() {
        this._c = Module._heif_reader_create();
        this._initStream = null;             // {ptr, cb, userData}
        this._segmentStreams = new Map();    // segment_id -> {ptr, cb, userData}
    }

    parseInitializationSegment(src) {
        const held = buildCallbacks(src);    // pins JS function pointers in Emscripten's table
        const s = Module._heif_stream_create(held.cbPtr);
        if (s === 0) throw new Error("heif_stream_create returned NULL");
        this._initStream = { ptr: s, ...held };   // strong ref outlives the C handle
        check(Module._heif_reader_parse_initialization_segment(this._c, s));
    }

    parseSegment(id, src, earliestPtsInTs = 0xffffffffffffffffn) {
        const held = buildCallbacks(src);
        const s = Module._heif_stream_create(held.cbPtr);
        if (s === 0) throw new Error("heif_stream_create returned NULL");
        this._segmentStreams.set(id, { ptr: s, ...held });
        check(Module._heif_reader_parse_segment(this._c, s, id, earliestPtsInTs));
    }

    invalidateSegment(id) {
        check(Module._heif_reader_invalidate_segment(this._c, id));
        const held = this._segmentStreams.get(id);
        if (held) {
            Module._heif_stream_destroy(held.ptr);
            this._segmentStreams.delete(id);
        }
    }

    close() {
        if (this._c) { Module._heif_reader_destroy(this._c); this._c = 0; }
        for (const held of this._segmentStreams.values()) Module._heif_stream_destroy(held.ptr);
        this._segmentStreams.clear();
        if (this._initStream) { Module._heif_stream_destroy(this._initStream.ptr); this._initStream = null; }
    }
}
```

JS-specific traps:
- Function pointers passed via `addFunction` / Embind callbacks
  must stay registered for the handle's lifetime; do not
  `removeFunction` them until after `heif_stream_destroy`.
- Under `-sASYNCIFY`, callback bodies that `await` (e.g. JS
  `fetch()` for a ranged byte read) can re-enter the library;
  see the wider Asyncify guidance referenced in §3 - the
  same constraints that apply to the single-stream case
  apply per-handle here.

**Rust note.** A natural-feeling Rust wrapper makes the
lifetime contract type-system-checkable: store each
`heif_stream_t` as `Arc<StreamHandle>` on the `Reader`
wrapper; have `parse_initialization_segment` consume the
`Arc` and keep one in `Reader.init_stream`; have
`parse_segment` keep the `Arc` in
`Reader.segment_streams: HashMap<SegmentId, Arc<StreamHandle>>`;
have `invalidate_segment` `.remove()` from the map (drop is
deferred until `Drop` releases the last `Arc`, which is
exactly the right window). `Drop for Reader` MUST destroy
the C reader before any of the `Arc<StreamHandle>` slots, 
either order the fields explicitly or call
`heif_reader_destroy` from a hand-written `Drop` impl.

**Java / JNI note.** The in-tree JNI binding
(`srcs/api-java/cpp/`) does **not** expose any of the
segment-streaming entry points today - its `InputStream`
shim implements `HEIF::StreamInterface` directly for the
single-stream still-image case, and there is no JNI
counterpart to `parseInitializationSegment` / `parseSegment` /
`invalidateSegment` / `parseSegmentIndex`. A Java caller
that needs fragmented playback today must consume the
library through a different surface (the C facade via
JNA / a hand-rolled JNI bridge of its own). If JNI
parity is added later, the lifetime obligation is
identical: hold a `GlobalRef` on every `InputStream` /
`heif_stream_t` from `parseSegment` until the matching
`invalidateSegment`, and a `GlobalRef` on the init-segment
stream for the entire `Reader` lifetime; do **not** rely on
local references or weak references for either.

**Failure mode worth simulating.** Drop a `parse_segment`
stream *before* its matching `invalidate_segment` and call
any cache-warming entry point that touches the segmented
data (`heif_reader_get_track_sample_data`, sample-list
walks, etc.). The library will call `read` / `tell` /
`size` on the released stream and the binding will crash
with a use-after-free in the callback bridge. This is the
canonical regression test for any segment-streaming binding.

### 2.6 - Type safety: IDs and FourCCs

**Contract.**

* All ID types (`ImageId`, `PropertyId`, `GroupId`, `TrackId`,
  `SequenceId`, `EntityId`, `SampleId`) are exposed as `uint32_t`
  on the C facade.
* All FourCC fields are exposed as `char[5]` (4 bytes + NUL
  terminator). The library always writes the NUL.

**Binding obligation.**

* Wrap each ID type in a distinct nominal type in your binding
  (Rust newtype, Python `NewType`, .NET `record struct`). They
  are all `uint32_t` on the wire but mixing them up is a class
  of bug the type system can catch for free. The Python binding
  uses plain `int` for v0 and will tighten to `NewType` in a
  later phase - recommend you do the strong typing from day 1.
* Decode FourCC as **ASCII with `backslashreplace`** (or your
  language's nearest equivalent) - never UTF-8, never strict
  ASCII. Malformed assets in the wild ship FourCCs with high
  bytes, and a strict decode will raise inside your diagnostic
  path. See `fourcc_to_str()` in `bindings/python/heif/_ffi.py`.

### 2.7 - ABI / SO-version pinning

**Contract.** The library's `SOVERSION` is the single declared
ABI promise. Today: `libheif_shared.so.4` on Linux,
`libheif_shared.4.dylib` on macOS, `heif_shared.dll` on Windows.
A bump from `.so.4` => `.so.5` signals an incompatible C-ABI
break; symbols may have been removed or had their signatures
changed.

**Binding obligation.**

1. Look up the library by **the SO-major-versioned filename**,
   not the unversioned symlink:
   * Linux: `libheif_shared.so.4`, not `libheif_shared.so`.
   * macOS: `libheif_shared.4.dylib`, not `libheif_shared.dylib`.
   * Windows: `heif_shared.dll` (Windows does not version
     filenames; pin via your binding's manifest).
2. On binding load, optionally call `HEIF::Reader::GetVersion()`
   and assert the major matches the major your `cdef` / bindgen
   was generated against. The Python binding does not yet do
   this assertion (v0); recommend that any binding shipped to
   production users do.

   *C-only / cwrap-only bindings* (Emscripten, ctypes, cffi,
   bindgen, P/Invoke) should prefer the pure-C accessors in
   [`heif_version_c.h`](../srcs/api/common/heif_version_c.h):

   * `heif_get_runtime_version(&maj, &min, &pat)` - split triple,
     each out-pointer independently optional.
   * `heif_get_runtime_version_string()` - same string
     `Reader::GetVersion()` returns, NUL-terminated, static
     storage.
   * `heif_get_runtime_version_number()` - packed
     `(major<<16)|(minor<<8)|patch`, same encoding as the
     build-time `HEIF_VERSION_NUMBER` macro for one-integer
     compares.

   The major value tracks SOVERSION exactly, so a
   `runtime_major != my_compile_time_major` check at module load
   matches the dynamic-loader's SONAME pin and fails fast on
   mismatched artefacts.

   Pair this with [`heif_error_code_name`](../srcs/api/common/heif_error_c.h)
   (and the symmetric `heif_error_origin_name`) to translate any
   numeric error code returned by any C-facade entry point into
   the canonical name string - pure stateless lookup, does NOT
   consult the per-thread error slot, safe to call when no error
   has been raised. Bindings should NOT carry their own switch
   statement mapping codes to names; the library is the
   authoritative source.
3. When the library bumps SO-major, **bump your binding's major
   too**, and republish. Do not try to support multiple SO-majors
   from one binding release; the cdef / bindgen output is a
   point-in-time snapshot of the C ABI.

The canonical SO-version pin in the Python binding is at
`bindings/python/heif/_ffi.py:_default_library_name()` - every
new binding should have an analogous one-line declaration.

---

## 3. Worked reference: the Python binding

`bindings/python/heif/` is the reference implementation. Three
files covering the full reader surface:

| File | Role |
|---|---|
| `_ffi.py` | `cdef`, library lookup, slot drain, FourCC decode |
| `_reader.py` | `Reader`, `Image`, all property types, `_check` helper |
| `__init__.py` | Public re-exports |

Every pattern named in §2 has a concrete realisation in those
three files. Read them in this order:

1. `_ffi.py` top-of-file docstring - what `cdef` is and why we
   use ABI mode rather than API mode.
2. `_ffi.py` `_CDEF` literal - the ground truth for what a
   binding's symbol table looks like.
3. `_ffi.py` `drain_last_error()` - the slot-drain pattern,
   transcribed straight from the error-slot contract.
4. `_reader.py` `HeifError` + `_check` - the binding-side
   exception shape.
5. `_reader.py` `Reader.__init__` + `close` + `__enter__` /
   `__exit__` / `__del__` - the lifecycle shape.
6. `_reader.py` `_fetch_bytes_via_probe` - the size-probe helper.

Even if your target language is nothing like Python, those six
symbols are the seven contracts of §2 made concrete.

---

## 4. Pre-flight checklist for a new binding

Before announcing your binding as ready for review:

- [ ] **Layer.** Binds the C facade only - no `extern "C++"`,
      no HEIFPP, no internal headers.
- [ ] **Lifecycle.** Deterministic `close` / `dispose` plus a
      GC-driven fallback. Init failure releases the handle.
- [ ] **Error handling.** Every fallible entry point routes
      through the binding's `_check`-equivalent; raises a typed
      exception carrying all eight error-slot fields; clears the slot
      after raising.
- [ ] **Size-probe helper.** One shared implementation, used by
      every variable-length getter.
- [ ] **Threading docs.** Per-instance rule documented on the
      handle wrapper; `Sync`-equivalent denied at the type level
      where the language supports it.
- [ ] **Allocator.** Either not exposed, or exposed behind an
      `unsafe`-equivalent with the one-shot semantics documented.
- [ ] **Stream callbacks.** If exposed: lifetime tied to the
      reader wrapper; callback bodies marshal exceptions into
      non-OK return codes.
- [ ] **ID typing.** Each ID type is a distinct nominal type in
      your binding.
- [ ] **FourCC decoding.** ASCII with replacement-on-error,
      never strict.
- [ ] **SO-version pin.** Library lookup uses the major-versioned
      filename; binding's major version is tied to library's
      SO-major.
- [ ] **Smoke test.** At least one end-to-end test: open a
      HEIC file from the [heif_conformance](https://github.com/nokiatech/heif_conformance)
      repository, enumerate master images,
      pull a bitstream, assert non-empty.
- [ ] **Error test.** At least one negative test: open a
      non-existent file, assert your binding raises with
      `code_name == "FILE_OPEN_ERROR"`.
- [ ] **Docs link.** Your binding's README links back to this
      wiki page so readers can find the canonical contracts.

The Python binding is the reference for what a production-ready
binding looks like. Aim for comprehensive coverage of every
entry point your binding exposes.

---

## 5. Source-of-truth verification

If anything on this page contradicts the source, the source wins.
Here is where each contract lives:

| Contract | Source | Header |
|---|---|---|
| C-ABI entry-point list | `srcs/api/reader/heifreader_c.h` | `heif_reader_*` |
| Error slot accessors | `srcs/api/common/heif_error_c.h` | `heif_get_last_error_*` |
| Error sink registration | `srcs/api/common/heiferrorinfo.h` | `setErrorSink` |
| Thread-safety contract | `wiki/06-threading.md` (this wiki) | normative table at top of `heifreader.h` |
| ABI policy | `SOVERSION` in `srcs/CMakeLists.txt` | normative |
| SOVERSION pin | `srcs/CMakeLists.txt` (search for `SOVERSION`) | `project(heif VERSION <x>.<y>.<z>)` in `srcs/CMakeLists.txt` |
| Use-after-destroy guard | Enforced inside C facade | `srcs/reader/heifreader_c.cpp` |
| Stream callback contract | `srcs/api/reader/heifreader_c.h` (`heif_stream_callbacks_t`) | normative |
| Track / sample surface | `srcs/api/reader/heifreader_c.h` (track surface section, search for `heif_track_info_t`) | `heif_reader_get_track_*`, `heif_track_info_t`, `heif_sample_info_t`, `heif_edit_unit_t`, `heif_segment_info_t`, the seven `heif_sample_{eqiv,stmi,refs,trif,spor,sulm,minp}_t`, plus the seven inline `heif_sample_flags_*` spec-bit accessors |
| Reference binding | `bindings/python/heif/` | - |

---

## 6. Where to go next

* **Open a binding** in your language and start at §2.1. Work
  through 2.1 => 2.7 in order; each section depends on the
  previous one.
* **Look at the Python binding** as the worked reference - every
  pattern in §2 has a named implementation in `bindings/python/heif/`.
* **Read [06 - Threading & Concurrency](06-threading.md)** in full
  before writing any threading docs in your binding.
* **Read [03 - Error Handling](03-error-handling.md)** in full
  before writing the binding's `_check`-equivalent.
* When the contract is unclear, file an issue against the C facade
  header - *not* against your binding. Contract gaps need to be
  fixed at the source so every binding benefits.

### 6.1 - Open follow-ups for wasm/JS bindings

The in-tree reference JS / WebAssembly binding
(`bindings/javascript-wasm/`, shipped in v4.0.0) exercises the
full reader surface via Emscripten `cwrap` and pre-buffered
in-memory streams. Three additive items remain intentionally
deferred until a consumer has a concrete need that justifies
locking the contracts in. Each is small:

| Tier | What | Effort | Why deferred |
|------|------|--------|--------------|
| **B2** | Versioned handshake struct - size-prefixed `heif_abi_handshake_t { uint32_t struct_size; uint32_t major; uint32_t minor; uint32_t patch; uint32_t flags; }` returned by `heif_get_abi_handshake(*out)`. Thin convenience over the existing `heif_get_runtime_version_number()`. | ~1 h | Field shape (which feature flags? a build-config bitmap?) should be driven by an actual binding's needs, not pre-guessed. |
| **B3** | Asyncify documentation page covering how `heif_reader_initialize_stream` and the bitstream extractors interact with emcc's `-sASYNCIFY` when JS-side stream callbacks must `await fetch()`. Stack-budget, callback-recursion limits, `ASYNCIFY_IMPORTS` naming convention. | ~half day | Already covered at a workable level in [09 - Asyncify & Streaming](09-asyncify-and-streaming.md); deeper coverage is best written *with* a consumer that has hit a specific Asyncify gotcha. **No code change required** - B1 callbacks are already plain function pointers compatible with Asyncify. |
| **A3** | Wire `bindings/javascript-wasm/heif.webidl` (already shipped as a spec-grade reference of the JS-visible API) into an actual codegen pipeline - either emcc's `webidl_binder.py` or modern Embind - so the JS ↔ C glue layer is generated from the IDL instead of hand-rolled. | ~half day | Demand-driven. The shipped JS binding hand-rolls `cwrap()` and finds it workable; if a second consumer prefers Embind with TypeScript declarations driven by the IDL, A3 pays off. |

What ships today in v4.0.0: the full reader C facade self-exports
under emcc (no `EXPORTED_FUNCTIONS=[…]` list); plain-C stream
callbacks via `heif_stream_callbacks_t` +
`heif_reader_initialize_stream` (Tier B1); runtime-version
handshake via `heif_get_runtime_version{,_string,_number}()`;
stateless error-name helpers `heif_error_code_name(int32_t)` /
`heif_error_origin_name(int32_t)`; opt-in
`srcs/cmake/EmscriptenPreset.cmake` for sane wasm defaults; the
reference `bindings/javascript-wasm/` binding consuming all of
the above. That set is sufficient to ship a working reader
binding; B2/B3/A3 are the polish layer above it.

