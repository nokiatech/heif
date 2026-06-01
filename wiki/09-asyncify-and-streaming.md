# 09 - Asyncify & Streaming in WebAssembly

> **Scope of this page.** How the HEIF library's synchronous C
> entry points interact with Emscripten's `-sASYNCIFY` when a
> wasm-hosted binding needs to `await` inside a stream callback.
> Covers when you need Asyncify, when you don't, the build flags,
> the stack-budget trade-off, callback-recursion constraints, and
> the `ASYNCIFY_IMPORTS` naming convention.
>
> If you only have ten seconds:
> **If you pre-buffer the entire file into wasm memory before
> calling `heif.open()`, you do not need Asyncify at all - and the
> reference JS binding at `bindings/javascript-wasm/` does exactly
> that.** Asyncify is only required when your stream callbacks
> must suspend to fetch bytes on demand.

---

## What this page covers

1. The mismatch: synchronous C calls vs asynchronous JS I/O.
2. Which C entry points invoke stream callbacks.
3. When you need Asyncify (and when you don't).
4. Build-flag recipe.
5. Stack budget and code-size trade-off.
6. Callback recursion and re-entrancy.
7. `ASYNCIFY_IMPORTS` naming.
8. Worked example: range-request streaming reader.
9. Alternatives to Asyncify.

---

## What you need before starting

* Familiarity with the stream-callback surface
  ([07 - Binding Authors, §3](07-binding-authors.md)) and the
  byte-fetch protocol ([08 - Byte-Fetch Protocol](08-byte-fetch-protocol.md)).
* A working Emscripten toolchain (≥ 3.1.50 for stable Asyncify;
  4.0+ recommended for `-fwasm-exceptions` compatibility).
* A reading knowledge of `heif_stream_callbacks_t` in
  `srcs/api/reader/heifreader_c.h`.

---

## 1. The mismatch

The HEIF C facade is entirely synchronous. When you call
`heif_reader_initialize_stream()`, the library calls your four
stream callbacks (`read`, `absolute_seek`, `tell`, `size`)
synchronously, potentially hundreds of times, before returning.
EOF is signalled by `read()` returning `0`; there is no separate
EOF callback. The same applies to
`heif_reader_parse_initialization_segment()` and
`heif_reader_parse_segment()` - each drives its stream to
completion before returning a status code.

In native code this is fine: the callbacks read from a file
descriptor or a memory buffer and return immediately.

In a browser, the data might come from a `fetch()` response or
an IndexedDB lookup. Your stream callback needs to `await` a
Promise - but it is being called from inside a synchronous wasm
call stack. JavaScript cannot suspend a synchronous wasm frame
to await a Promise. The call hangs, deadlocks, or traps.

**Emscripten's Asyncify** solves this by transforming the wasm
binary at compile time so that any C function on the call stack
can be "unwound" (its locals saved to a shadow stack), control
returned to the JS event loop, and then "rewound" (locals
restored) when the Promise resolves. It is cooperative
multitasking injected by the compiler.

---

## 2. Which entry points invoke stream callbacks

The following C facade entry points call into user-provided stream
callbacks during their execution:

| Entry point | Callback pattern |
|---|---|
| `heif_reader_initialize_stream` | Reads the entire file via the callback table; parses all boxes before returning. Heaviest callback usage - hundreds to thousands of read/seek/tell round-trips on a typical file. |
| `heif_reader_parse_initialization_segment` | Reads the DASH init segment via a `heif_stream_t*`. |
| `heif_reader_parse_segment` | Reads one DASH media segment via a `heif_stream_t*`. |
| `heif_reader_parse_segment_index` | Reads a `sidx` box via a `heif_stream_t*`. |

No other C facade entry points invoke stream callbacks. The
accessor functions (`heif_reader_get_*`, property queries,
bitstream extraction) operate on in-memory data that was parsed
during initialisation.

---

## 3. When you need Asyncify

**You need Asyncify if and only if** your stream callback
bodies must suspend to the JS event loop - typically because
they fetch bytes on demand from a network source, a
`ReadableStream`, IndexedDB, or any other async API.

**You do NOT need Asyncify if:**

* You pre-buffer the entire file into a `Uint8Array` and copy
  it to the wasm heap before calling `heif.open()`. The reference
  JS binding (`bindings/javascript-wasm/heif.js`) uses this
  strategy: `heif.open(bytes)` copies the caller's buffer into
  wasm memory, sets up purely synchronous in-memory callbacks
  (read = `memcpy` from a heap slab), and calls
  `heif_reader_initialize_stream`. No suspension, no Asyncify.
* You use the file-system API (`heif_reader_initialize_file`)
  with Emscripten's virtual FS pre-loaded via
  `Module.FS.writeFile` before the call.

The pre-buffer strategy is simpler, faster (no Asyncify
overhead), and sufficient for the vast majority of use cases.
Asyncify is the path for **streaming** scenarios where holding
the entire file in memory is unacceptable (very large files,
progressive rendering, constrained-memory environments).

---

## 4. Build-flag recipe

Add to your `emcc` link command (or `CMakeLists.txt`
`LINK_FLAGS`):

```
-sASYNCIFY=1
-sASYNCIFY_STACK_SIZE=65536
-sASYNCIFY_IMPORTS=['_my_async_read','_my_async_seek']
```

**`-sASYNCIFY=1`** enables the transformation. Without this,
Emscripten compiles all calls as normal synchronous wasm.

**`-sASYNCIFY_STACK_SIZE`** sets the shadow-stack budget in
bytes. 64 KB is a conservative starting point. The library's
internal call depth during parsing is modest (box nesting
rarely exceeds ~15 frames), but the total unwind depth
includes your glue code and any Emscripten runtime frames.
If you see `RuntimeError: unreachable` during an async
unwind, increase this value. Monitor
`Module.ASYNCIFY_STACK_SIZE` at runtime to tune.

**`-sASYNCIFY_IMPORTS`** names the JS functions that may
suspend. Only functions listed here are treated as potential
suspension points; everything else remains synchronous (this
is how Asyncify limits code-size and runtime overhead). List
the mangled names of your callback wrapper functions - the
ones registered via `addFunction` that internally `await`.

---

## 5. Stack budget and code-size trade-off

Asyncify instruments every function on every possible call
path between the module entry and the listed async imports.
This has two costs:

1. **Code size.** The wasm binary grows by 30–80 % depending
   on the proportion of functions that need instrumentation.
   The HEIF library is roughly 2.5 MB as a baseline wasm; an
   Asyncified build may reach 4–5 MB. Use
   `-sASYNCIFY_ONLY=['func1','func2',...]` to allowlist only
   the functions that actually need unwind support, which
   dramatically reduces bloat (at the cost of a runtime trap
   if a non-listed function is on the stack during an unwind).

2. **Runtime overhead.** Every instrumented function carries a
   small prologue/epilogue that checks whether an unwind or
   rewind is in progress. This adds ~5–15 % CPU overhead on
   the instrumented paths. Non-instrumented paths (the
   accessor functions, property queries) are unaffected.

For builds where both streaming and code size matter, consider
splitting the build into two variants: a lean non-Asyncify
build for the common pre-buffered path, and an Asyncify build
for the streaming path. The JS-visible API is identical; only
the wasm binary differs.

---

## 6. Callback recursion and re-entrancy

During a single `heif_reader_initialize_stream` call, the
library may invoke your read callback hundreds of times (once
per box header, once per box payload, once per property, etc.).
Each invocation is a separate synchronous call from the
library's perspective; with Asyncify, each one is a potential
suspension point.

Constraints:

* **No concurrent callbacks.** The library never calls two
  callbacks concurrently from the same entry point. Your
  callbacks are invoked strictly sequentially: read, return,
  read, return, seek, return, read, return, …

* **No re-entrancy into the library.** While a callback is
  executing (or suspended via Asyncify), you MUST NOT call
  any other `heif_reader_*` function on the same reader
  handle. The library's internal state machine is not
  re-entrant. This matches the threading contract
  ([06 - Threading](06-threading.md)): one call at a time
  per reader instance.

* **Callback ordering is deterministic but
  implementation-defined.** The library reads boxes in file
  order; the sequence of read/seek/tell calls for a given
  file is stable across library versions but is not part of
  the public contract. Do not depend on a specific callback
  sequence.

---

## 7. `ASYNCIFY_IMPORTS` naming

The `-sASYNCIFY_IMPORTS` list uses the *wasm-level* symbol
names of the JS functions that may suspend. When you register
a callback via `Module.addFunction(fn, 'iiiii')`, Emscripten
assigns it a table slot but the actual call goes through a
JS-side trampoline. The name you list in `ASYNCIFY_IMPORTS`
is the name of that trampoline.

In practice, the simplest approach is:

1. Define your async callbacks as named JS functions (not
   anonymous):

   ```js
   async function myAsyncRead(userData, buf, size, offset) {
       const data = await fetchRange(offset, size);
       Module.HEAPU8.set(data, buf);
       return 0;
   }
   ```

2. Register with `addFunction`:

   ```js
   const readPtr = Module.addFunction(myAsyncRead, 'iiiii');
   ```

3. List the internal name in the build flags. For
   `addFunction`-registered callbacks, Emscripten routes
   through `dynCall_*` trampolines. The reliable approach
   is to use the wildcard form:

   ```
   -sASYNCIFY_IMPORTS=['dynCall_*']
   ```

   This is broader than strictly necessary but avoids
   name-guessing. For tighter control, inspect the
   generated `.js` glue for the exact trampoline names.

---

## 8. Worked example: range-request streaming reader

Below is a sketch (not production code) of a streaming HEIF
reader that fetches byte ranges on demand via `fetch()` and
uses Asyncify to suspend the wasm call stack while the
network request is in flight.

```js
// Build with: -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=65536
//             -sASYNCIFY_IMPORTS=['dynCall_*']

class StreamingHEIFReader {
    constructor(module, url, fileSize) {
        this._m = module;
        this._url = url;
        this._fileSize = fileSize;
        this._pos = 0;
    }

    // read(user_data, buffer, size) -> int64 bytes read
    // (0 == EOF, negative == error). Position is implicit:
    // advances by the number of bytes returned, and is reset
    // by absolute_seek().
    async _read(userData, buf, size) {
        if (this._pos >= this._fileSize) return 0n;     // EOF
        const start = this._pos;
        const end   = Math.min(start + Number(size), this._fileSize) - 1;
        const resp  = await fetch(this._url, {
            headers: { Range: `bytes=${start}-${end}` }
        });
        const data = new Uint8Array(await resp.arrayBuffer());
        this._m.HEAPU8.set(data, buf);
        this._pos += data.length;
        return BigInt(data.length);
    }

    // absolute_seek(user_data, offset) -> int; non-zero on success.
    _seek(userData, offset) {
        this._pos = Number(offset);
        return 1;
    }

    // tell(user_data) -> int64 current absolute offset.
    _tell(userData) {
        return BigInt(this._pos);
    }

    // size(user_data) -> int64 total stream size
    // (or 0x7fffffffffffffff for an indeterminate network stream).
    _size(userData) {
        return BigInt(this._fileSize);
    }

    async open() {
        // Register callbacks as wasm function pointers. The signature
        // strings here are illustrative; consult the generated glue
        // for the exact dynCall_ signature that matches each callback's
        // i32/i64 parameter list.
        const readPtr = this._m.addFunction(this._read.bind(this), 'jiij');
        const seekPtr = this._m.addFunction(this._seek.bind(this), 'iij');
        const tellPtr = this._m.addFunction(this._tell.bind(this), 'ji');
        const sizePtr = this._m.addFunction(this._size.bind(this), 'ji');

        // Build heif_stream_callbacks_t on the heap:
        //   uint32_t struct_size;
        //   read / absolute_seek / tell / size  (four function pointers)
        //   void* user_data;
        // Total on wasm32: 4 + 4*4 + 4 = 24 bytes.
        const cbSize = 4 + 4 * 4 + 4;
        const cbPtr  = this._m._malloc(cbSize);
        this._m.HEAPU32[cbPtr >> 2]       = cbSize;
        this._m.HEAPU32[(cbPtr >> 2) + 1] = readPtr;
        this._m.HEAPU32[(cbPtr >> 2) + 2] = seekPtr;
        this._m.HEAPU32[(cbPtr >> 2) + 3] = tellPtr;
        this._m.HEAPU32[(cbPtr >> 2) + 4] = sizePtr;
        this._m.HEAPU32[(cbPtr >> 2) + 5] = 0;   // user_data unused here

        // This call will invoke _read many times, each
        // time suspending via Asyncify to await fetch().
        const rc = this._m._heif_reader_initialize_stream(
            this._reader, cbPtr);

        this._m._free(cbPtr);
        // Function pointers must stay registered until
        // the reader is destroyed.
        this._callbackPtrs = [readPtr, seekPtr, tellPtr, sizePtr];

        if (rc !== 0) {
            throw new Error('initialize_stream failed: ' + rc);
        }
    }
}
```

Key observations:

* Only `_read` is `async`; the other callbacks are synchronous and
  return immediately. Asyncify still instruments the entire call
  path because any callback *might* suspend.
* The `fetch()` inside `_read` is a real network request that may
  take tens or hundreds of milliseconds. Asyncify transparently
  unwinds the wasm stack, yields to the event loop, and rewinds
  when the Promise resolves.
* EOF is implicit: a `read()` that returns `0n` tells the library
  to stop. There is no separate EOF callback.
* For production use, add a byte-range cache to avoid re-fetching
  the same offset range when the library seeks backward (which it
  does frequently during box-header parsing).

---

## 9. Alternatives to Asyncify

| Alternative | Pros | Cons |
|---|---|---|
| **Pre-buffer** (`HEIF.open(bytes)`) | Zero overhead, simplest code path, smallest wasm binary. | Requires the entire file in memory before parsing begins. |
| **JSPI** (JS Promise Integration, Stage 3 TC39 proposal) | Native browser support for suspending wasm => JS => Promise chains; no compiler transformation needed; near-zero code-size and runtime overhead. | Not yet shipped in all browsers (Chrome 119+ behind a flag, Firefox nightly). When stable, this is the recommended replacement for Asyncify. |
| **Web Workers + SharedArrayBuffer** | Move wasm to a Worker; use `Atomics.wait` to block the Worker while the main thread fulfils the fetch. No Asyncify needed. | Requires `SharedArrayBuffer` (COOP/COEP headers), more complex architecture, main-thread coordination. |
| **Chunked parsing** | Pre-fetch in chunks, feed each chunk to `parseInitSegment` / `parseSegment`. | Only works for DASH-segmented content; does not help with monolithic HEIF files that `initialize_stream` must parse in one call. |

For the vast majority of web deployments, **pre-buffering is
the right answer**. Asyncify is the escape hatch for the
streaming edge case; JSPI will eventually supersede it.
