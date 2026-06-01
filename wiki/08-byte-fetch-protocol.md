# 08 - Byte-Fetch Protocol (probe-and-fetch)

> **Canonical reference.** Single source of truth for how raw bytes
> (image bitstreams, sample bitstreams, Exif/XMP/MIME payloads,
> protection scheme info) leave the library. Every layer - raw
> `HEIF::Reader`, HEIFPP, the C facade, JNI, language bindings - is
> documented here in one place. Touch-point headers cross-reference
> back to this page.
>
> **Audience.**
>
> * **Application developer** wondering *"why is the example file rolling
>   its own byte-fetch helpers?"* => §1, §2, §3.
> * **Library developer** about to add another `(buf, *size)`-style
>   accessor => §6.
> * **Binding author** wrapping the C facade in Python / Rust / Go /
>   .NET / Swift / … => §4 + page [07 - Binding Authors](07-binding-authors.md).

---

## 1. The problem in one paragraph

The library reads variable-length payloads (encoded image bytestreams,
sample bytestreams, Exif/XMP, `sinf` boxes). The unpacked size is
**not** the same as the size stored in the file: the library may
substitute bytestream NAL headers (`0x000001` for AVC/HEVC), reinsert
stripped NAL headers (compact-VVC items add 6 bytes - the `vvc1`
prefix that was stripped at write time), or prepend decoder
configuration records (the `*WithDecoderParameters` overloads). The
embedder cannot precompute the unpacked size from `ItemInformation`;
the library is the only place that knows it. The byte-fetch API is
therefore a **two-call protocol**: first ask the library for the size,
then ask for the bytes.

## 2. The protocol

```
┌─ Step 1 - PROBE ──────────────────────────────────────────────┐
│  Call with `buffer = nullptr` and `*size = 0`.                │
│  Library writes the required size into `*size`,               │
│  returns BUFFER_SIZE_TOO_SMALL.                                │
├─ Step 2 - ALLOCATE ───────────────────────────────────────────┤
│  Allocate `*size` bytes with whatever allocator you want      │
│  (heap, arena, JNI direct buffer, wasm heap slab, …).         │
├─ Step 3 - FETCH ──────────────────────────────────────────────┤
│  Call again with the allocated buffer and the size from       │
│  step 1. On OK, library writes the bytes and **narrows**      │
│  `*size` to the number of bytes actually written              │
│  (always `≤` the value passed in).                             │
└────────────────────────────────────────────────────────────────┘
```

Three reasons it looks like this at the ABI boundary:

1. **Allocator neutrality.** The library cannot return a `std::vector`
   or any owning container across the DLL boundary - the deleter must
   match the allocator that produced the storage, and that's not
   reliable across MSVC ↔ MinGW ↔ glibc ↔ JNI ↔ wasm.
2. **C-shape for FFI.** The same vtable entry is what the C facade,
   JNI, and any future binding wrap. `(buf, *size)` is the lowest
   common denominator that every language can speak.
3. **No exceptions across the ABI.** Returning a container would
   require its constructor to throw on OOM; that's forbidden across
   the DLL boundary. The probe protocol degrades cleanly to an
   `ErrorCode` return.

## 3. Where to use it (and where not to)

**C++ with `heif::heif_static` / `heif::heif_shared`:**
Use the `HEIF::readItem*` inline wrappers at the bottom of
`heifreader.h`. They return `Array<uint8_t>` and hide the probe
entirely. Only drop down to the raw `Reader::getItemData(...)` if
you have a custom allocator.

**C++ with HEIFPP (`heif::heifpp`):**
Use `HEIFPP::Sample::loadSampleData()`, then `getItemData()` /
`getItemDataSize()`. Also: `MimeItem::getData()`,
`CodedImageItem::getBitstream()`. The probe is hidden inside
HEIFPP. Do not reach into the underlying `HEIF::Reader` for byte
access while HEIFPP owns the parent.

**C consumer or language binding (Python / Rust / Go / .NET / Swift):**
Use `heif_reader_get_item_data` /
`heif_reader_get_item_data_with_decoder_parameters` from
`heifreader_c.h`. Build one shared probe helper in your binding
(the Python binding's `_fetch_bytes_via_probe()` is the reference).
Do not hand-roll per-call-site probe code.

**Java / Kotlin / Android:**
Use `CodedImageItem.getItemData()` (returns a `ByteBuffer`),
`MimeItem.getItemData()`, `ExifItem.getItemData()`. The probe is
hidden inside the JNI bridge. Do not call `Reader` directly for
byte access.

**Wasm / JS via the C facade:**
Use the same probe, allocating into the wasm heap with `_malloc`.
The `EmscriptenPreset.cmake` enables the C facade self-export so
every `heif_reader_*` is callable from JS without an
`EXPORTED_FUNCTIONS` list. Do not return a `std::vector` across the
embind boundary.

## 4. Per-layer cheat sheet

### Layer 1 - Raw `HEIF::Reader` (`srcs/api/reader/heifreader.h`)

Five virtual methods, all use the probe protocol:

| Method | Returns |
|---|---|
| `getItemData(ImageId, buf, *size, bytestreamHeaders=true)` | Item bytes (Exif / XMP / MIME / coded item, no decoder configs). |
| `getItemData(SequenceId, SequenceImageId, buf, *size, bytestreamHeaders=true)` | One sample of an image-sequence track. |
| `getItemDataWithDecoderParameters(ImageId, buf, *size)` | Coded item bytes with decoder config records prepended (decoder-ready). |
| `getItemDataWithDecoderParameters(SequenceId, SequenceImageId, buf, *size)` | Same, but for one sample of a track. |
| `getItemProtectionScheme(ImageId, buf, *size)` | The `'sinf'` box of a protected item. |

Five matching non-virtual `inline` wrappers in the same header
(below the class) hide the protocol for C++ callers:

| Wrapper | Returns |
|---|---|
| `HEIF::readItemData(reader, ImageId, [bytestreamHeaders], [&errOut])` | `Array<uint8_t>` (empty on failure, `errOut` carries the code). |
| `HEIF::readItemData(reader, SequenceId, SequenceImageId, …)` | `Array<uint8_t>`. |
| `HEIF::readItemDataWithDecoderParameters(reader, ImageId, …)` | `Array<uint8_t>`. |
| `HEIF::readItemDataWithDecoderParameters(reader, SequenceId, SequenceImageId, …)` | `Array<uint8_t>`. |
| `HEIF::readItemProtectionScheme(reader, ImageId, …)` | `Array<uint8_t>`. |

Use the wrappers unless you have a specific allocator requirement.
The doxygen anchor `@ref probe_protocol` next to the virtuals is the
in-header normative reference.

### Layer 2 - HEIFPP (`srcs/api-cpp/`)

HEIFPP **hides the probe**. It pre-records the item size when the
parent `Heif` parses the file, then issues a single fetch with that
size. From the HEIFPP user's perspective there is no probe, no
`BUFFER_SIZE_TOO_SMALL`, no two-call dance:

| HEIFPP method | What it does |
|---|---|
| `HEIFPP::Sample::loadSampleData(SequenceId)` | Calls `Reader::getItemData(track, sample, …)` with the size already known; converts to byte stream for AVC/HEVC. |
| `HEIFPP::Sample::getItemData()` / `getItemDataSize()` | Returns the cached buffer + size (must call `loadSampleData` first). |
| `HEIFPP::MimeItem::loadData()` / `getData()` / `getDataSize()` | Same pattern for MIME items (XMP and friends). |
| `HEIFPP::ExifItem::getData()` / `getDataSize()` | Same for Exif. |
| `HEIFPP::HEVCCodedImageItem::getBitstream(buf, size)` | Coded image bytestream; pre-cached during `Heif::load`. |
| `HEIFPP::JPEGCodedImageItem::getBitstream(buf, size)` | Same for JPEG. |
| `HEIFPP::VVCCodedImageItem::getBitstream(buf, size)` | Same for VVC. |

> **Note for library developers.** The HEIFPP path uses the *stored*
> item size, not the *unpacked* size the probe would report. For
> codecs where the two differ (compact-VVC unpacks to `stored + 6`
> after NAL-header reinsertion) HEIFPP's single-shot fetch can return
> `BUFFER_SIZE_TOO_SMALL`. The proper fix is to switch the HEIFPP
> internal load path to use the new `HEIF::readItem*` inline wrappers
> (Layer 1) rather than open-coding a single `getItemData` call.
> This is not yet done in v4.0.0; tracked as a follow-up.

### Layer 3 - C facade (`srcs/api/reader/heifreader_c.h`)

Probe protocol exposed verbatim. Every byte-returning entry point uses
`(uint8_t* buffer, uint64_t* size_inout)` with the convention from §2.
The function-level doxygen on `heif_reader_get_item_data_with_decoder_parameters`
contains the canonical C example. Bindings should wrap this in a
single shared helper rather than re-emit the dance at every call site
(see §4 of [07 - Binding Authors](07-binding-authors.md) for the
rationale and the Python reference implementation).

### Layer 4 - JNI / Java (`srcs/api-java/cpp/`)

JNI hides the probe by going through HEIFPP. The JNI bridge calls
`HEIFPP::CodedImageItem::getItemData()` (already pre-loaded), then
wraps the buffer in a `java.nio.ByteBuffer` via
`env->NewDirectByteBuffer(...)`. Java users never see the protocol.

The lifetime of the returned `ByteBuffer` is the lifetime of the
backing HEIFPP item; do not keep it alive past the parent `HEIF`
instance. See `04-java-and-android.md` for the full Java contract.

### Layer 5 - Bindings (Python today; Rust / Go / .NET / Swift / wasm next)

Build **one** shared probe helper per binding language. Rationale and
worked example: §2 of [07 - Binding Authors](07-binding-authors.md).
Reference implementation: `_fetch_bytes_via_probe()` in
`bindings/python/heif/_reader.py`.

For wasm specifically:

```js
// Probe.
const sizePtr = Module._malloc(8);
Module.setValue(sizePtr, 0, 'i64');
let rc = Module._heif_reader_get_item_data_with_decoder_parameters(
    reader, imageId, 0, sizePtr);
const needed = Number(Module.getValue(sizePtr, 'i64'));

// Allocate + fetch.
const bufPtr = Module._malloc(needed);
Module.setValue(sizePtr, BigInt(needed), 'i64');
rc = Module._heif_reader_get_item_data_with_decoder_parameters(
    reader, imageId, bufPtr, sizePtr);
const actual = Number(Module.getValue(sizePtr, 'i64'));

// Copy out, free.
const bytes = Module.HEAPU8.slice(bufPtr, bufPtr + actual);
Module._free(bufPtr);
Module._free(sizePtr);
```

## 5. The `+6` story (why precomputing doesn't work)

A compact-VVC item stored in the file as `N` bytes unpacks to `N + 6`
bytes when the library reinserts the `vvc1` NAL header that was
stripped at write time (this is what makes "compact" VVC compact, 
the `vvc1` length field that all standard players need is omitted on
disk and reconstructed at read time). The embedder does not know
which items in the file are compact and which are not without
parsing the box tree itself. The probe protocol moves that
responsibility into the library, where it belongs.

The same shape recurs in other codecs and modes:

| Codec / mode | Stored vs. unpacked | Source |
|---|---|---|
| Compact VVC (`vvs1` / mini-VVC) | `+ 6` for NAL header reinsertion | `srcs/reader/heifreaderaccessors.cpp` (search for `vvs1` rewrite path) |
| AVC / HEVC with `bytestreamHeaders=true` | NAL length fields (4 bytes each) replaced with start codes (3-4 bytes); net delta depends on NAL count | `srcs/common/nalutil.cpp` |
| Any coded item via `getItemDataWithDecoderParameters` | `+ sizeof(decoder configuration record)`, varies by codec | `srcs/reader/heifreaderaccessors.cpp` (decoder-config prepend path) |
| Multi-layer items | Per-layer length-field rewrites | same |

## 6. Adding a new byte-returning accessor (developer rule)

When you add a `(buf, *size)`-style getter to `HEIF::Reader` or
`HEIF::Writer`, you owe **three** things, every time:

1. **Header doxygen** that explicitly invokes `@ref probe_protocol`
   (defined in `srcs/api/reader/heifreader.h`). Don't say "memory
   buffer size" and stop - say "probe-and-fetch (see @ref
   probe_protocol)".
2. **A non-virtual `inline` wrapper** in the same header, returning
   `Array<uint8_t>`. Pattern: see `HEIF::readItemData` at the bottom
   of `heifreader.h`. The wrapper is header-only ⇒ no vtable change
   ⇒ no SOVERSION bump.
3. **A C-facade entry point** in `heifreader_c.h`, with the doxygen
   example and a back-reference to the C++ method. (When a writer
   C facade is eventually added, the same rule applies to
   `heifwriter_c.h` - it does not exist in v4.0.0.)

Do not ship the virtual without all three. The whole reason the byte
accessors had to be revisited in v4.0.0 is that prior generations
of the library shipped step 1 only.

## 7. Cross-references

* In-header anchor: `@ref probe_protocol` in `srcs/api/reader/heifreader.h`
  (the doxygen block above `getItemData`).
* C facade: rule 5 in the file-level docblock of
  `srcs/api/reader/heifreader_c.h`; per-method examples on
  `heif_reader_get_item_data_with_decoder_parameters` and
  `heif_reader_get_item_data`.
* HEIFPP: `srcs/api-cpp/Sample.{h,cpp}::loadSampleData`,
  `MimeItem.{h,cpp}::loadData`, `*CodedImageItem.{h,cpp}::getBitstream`.
* JNI: `srcs/api-java/cpp/CodedImageItemJNI.cpp::getItemDataNative`.
* Python: `bindings/python/heif/_reader.py::_fetch_bytes_via_probe`.
* Worked examples: `srcs/examples/example.cpp` (uses Layer 1
  wrappers), `srcs/examples/heifpp_example.cpp` (uses Layer 2),
  `bindings/python/heif/_reader.py` (uses Layer 3 via cffi).
* Changelog entry: `CHANGELOG.md` v4.0.0, `### Added`.
