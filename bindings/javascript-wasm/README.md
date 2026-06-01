# Nokia HEIF: JavaScript / WebAssembly Binding

JavaScript / WebAssembly binding for the Nokia HEIF library C facade. Complete coverage of the C facade reader API, compiled to WebAssembly via Emscripten.

Package name: `nokia-heif`. Reader only.

> The binding **does not decode pixels.** `HEIFReader.itemBitstream()` returns an encoded HEVC / AVC / VVC / JPEG payload; feed it to whichever decoder you already use (`libde265.js`, `vvdec`, etc.).

---

## Build

Prerequisites:

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), 4.0 or newer recommended, activated in the current shell (`source emsdk_env.sh`).
- CMake 3.12 or newer.

Build (one command):

```bash
cd bindings/javascript-wasm
bash build-with-emcmake.sh
```

When the build finishes, the script prints the bundle location and contents. The deployable drop-in bundle is auto-staged in [`dist/`](dist/):

```
dist/
    heif-raw.js       Emscripten module factory
    heif-raw.wasm     Compiled C++ library (~1 MB)
    heif.js           The JavaScript API
    heif.d.ts         TypeScript declarations
    package.json      npm manifest
    README.md         Bundle usage notes
```

First-time build takes a few minutes (the full C++ → wasm compile of the reader, common, and VVC-parser sources). Subsequent builds are incremental.

### Installing Elsewhere

To install the bundle into a project directory in the same command, pass `--prefix`:

```bash
bash build-with-emcmake.sh --prefix /path/to/your/project/vendor/heif
```

Or run the install step separately after building:

```bash
cmake --install bindings/javascript-wasm/build --prefix /path/to/your/project/vendor/heif
```

### Configuration Override

If `heif-raw.wasm` is served from a different path than `heif-raw.js` at runtime (for example a CDN), pass a `locateFile` override when loading:

```js
const heif = await HEIF.load({
    moduleArgs: {
        locateFile: (path) => 'https://cdn.example.com/wasm/' + path
    }
});
```

---

## Usage

### Browser

```html
<script src="heif-raw.js"></script>
<script src="heif.js"></script>
<script>
  HEIF.load().then(function(heif) {
    fetch('photo.heic')
      .then(r => r.arrayBuffer())
      .then(buf => {
        const reader = heif.open(new Uint8Array(buf));
        console.log(reader.majorBrand);                       // "heic", "mif1", ...
        console.log(reader.itemSize(reader.primaryItemId));   // {width, height}
        const bitstream = reader.itemBitstream(reader.primaryItemId);
        // Feed bitstream to your decoder (libde265.js, vvdec, ...)
        reader.close();
      });
  });
</script>
```

Load order matters: `heif-raw.js` must be loaded before `heif.js`. The factory registers itself as `globalThis.HEIFModule`, which `heif.js` then consumes.

### Node.js

```js
const HEIFModule = require('./heif-raw.js');
const HEIF = require('./heif.js');
const fs = require('fs');

const heif = await HEIF.load({ factory: HEIFModule });
const reader = heif.open(new Uint8Array(fs.readFileSync('photo.heic')));
console.log(reader.majorBrand, reader.itemSize(reader.primaryItemId));
reader.close();
```

In Node, the factory is passed explicitly via `{ factory }` rather than relying on a global.

---

## Reference

### API Coverage

All public functions from `heifreader_c.h`, `heif_error_c.h`, and `heif_version_c.h` are bound via `cwrap`. The `heif_sample_flags_*` static-inline helpers are reimplemented in pure JS. Full TypeScript declarations are in [`heif.d.ts`](heif.d.ts).

### DASH Segmented Parsing

```js
const reader = heif.open(new Uint8Array(0));    // placeholder
reader.parseInitSegment(initSegBytes);          // moov / ftyp
reader.parseSegment(mediaSegBytes, 1, 0);       // moof + mdat
const samples = reader.trackSamples(trackId);
reader.invalidateSegment(1);                    // free memory
reader.close();
```

---

## Internals

```
srcs/  (C++ library)
  v  compiled via Emscripten (--whole-archive, EMSCRIPTEN_KEEPALIVE)
heif-raw.wasm + heif-raw.js  (Emscripten glue)
  v  cwrap
heif.js  (HEIF / HEIFReader classes)
  v  public API
Application code
```

- **Emscripten glue** (`heif-raw.js`): loads the wasm, provides the heap and `Module.cwrap` / `Module.ccall`.
- **Binding** (`heif.js`): memory management (malloc/free), struct unpacking, the size-probe protocol, stream trampolines, the class API.
- **Application**: calls `heif.open(uint8Array)` or the DASH segmented API, then uses `HEIFReader` methods for data access.

### Error Handling

Every C facade call that returns `heif_error_code_t` passes through `_check()`, which drains the per-thread last-error slot (code, origin, line, message), clears it, and throws a structured `HEIFError`. The full error model is documented in [wiki § 03: Error Handling](../../wiki/03-error-handling.md).

### Size-Probe Protocol

Byte-returning calls follow `(buf, *size_inout)`:

1. Call with `buf = 0`. The library writes the required size into `*size_inout`.
2. Allocate `buf` of that size.
3. Call again. The library fills `buf`.

Encapsulated in the `fetchSized()` helper.

### Stream Trampolines

`heif_stream_callbacks_t` is packed manually in the wasm heap. `Module.addFunction()` creates wasm-callable JS callbacks (read, seek, tell, size). Lifetime is managed by `HEIFReader.close()`.

---

## Troubleshooting

JavaScript-binding-specific failure modes:

- **`heif.js: HEIFModule factory not found`**: `heif-raw.js` was not loaded before `heif.js` in the browser, or `{ factory }` was not passed in Node. Fix the load order, or pass the factory explicitly.
- **`failed to load wasm binary`** (404 in the browser console): `heif-raw.wasm` is not being served from the same URL as `heif-raw.js`. Either co-locate them, or pass a `locateFile` override in `moduleArgs` (see [Deploying](#deploying)).
- **`heif.js requires BigInt support`**: the runtime is older than Node.js 10.3 or pre-2020 browsers. The wasm is built with `-sWASM_BIGINT`; a current runtime is required.
- **Build failures during `bash build-with-emcmake.sh`** (Emscripten not found, CMake too old): verify `emcc --version` works in the current shell (the SDK must be activated with `source emsdk_env.sh`) and that CMake is 3.12 or newer.

---

**Links:** [Main README](../../README.md) · [TypeScript declarations](heif.d.ts) · [Wiki](../../wiki/) · [Error Model](../../wiki/03-error-handling.md) · [Issues](https://github.com/nokiatech/heif/issues) · [License](../../LICENSE.TXT)

<sub>Parts of this repository were produced with the assistance of AI tooling, with human review.</sub>
