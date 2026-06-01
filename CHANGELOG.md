# Changelog

All notable changes to the Nokia HEIF library are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [4.0.0]

Major version bump. Public ABI hardened for cross-language use, install tree restructured, SOVERSION introduced, comprehensive language bindings added, and a public C facade exposed for FFI consumers. Documentation overhauled across the repository.

### Breaking Changes

- **SOVERSION introduced as `4`.** Shared libraries now install as `libheif_shared.so.4` and `libheif_writer_shared.so.4` with proper SONAME symlinks. Consumers that linked the unversioned `libheif_shared.so` from v3.x must relink. `find_package(heif)` calls with a 3.x minimum will fail; bump to `find_package(heif 4.0.0)`.

- **Factory-only construction for `HEIFPP::Item` subclasses.** The concrete item subclasses no longer expose public constructors. Use the factory: `heif.create<HEIFPP::ExifItem>()` instead of `new HEIFPP::ExifItem(&heif)`.

- **`HEIF::ErrorInfo` is now opaque.** The v3.x struct carried `std::string` members in the public layout, which was unstable across CRT mismatches. Access via accessors: `info.code()`, `info.message()`, etc.

- **Public-header enums pinned to fixed underlying types.** `ErrorCode`, `EditType`, `MediaFormat`, `TrackSampleType`, `ItemPropertyType`, and `SampleType` are now `: std::uint8_t`; `HEIFPP::Result` is `: std::uint16_t`. Numeric values are unchanged; only the storage width changed.

- **ABI surface containment.** The exported symbol set is now an explicit allowlist. Accidentally-leaked internal symbols (`std::` template instantiations, anonymous-namespace helpers) are no longer exported. All documented `HEIF::*`, `HEIFPP::*`, and `heif_*` symbols are preserved.

### Added: Language Bindings

- **Python binding** (`bindings/python/`). Complete `cffi` ABI-mode binding wrapping the C facade. Covers file-based and stream-based reader initialisation, item, property, and image enumeration, bitstream extraction, tracks, image properties, HDR metadata, entity groups, grids, overlays, gainmaps, and DASH segmented parsing. Includes an example script, a `pip install`-able `pyproject.toml`, and a one-shot installer script.

- **JavaScript / WebAssembly binding** (`bindings/javascript-wasm/`). Complete `cwrap`-based binding compiled via Emscripten. Covers all C facade reader entry points with TypeScript declarations and a WebIDL surface spec.

- **Sdist install support.** The repository can produce a self-contained source tarball via `python -m build --sdist`. `pip install "git+https://github.com/nokiatech/heif.git@v4.0.0"` drives the CMake build automatically via `scikit-build-core`.

### Added: C Facade

- **Complete pure-C reader API** (`heifreader_c.h`). Opaque-handle API covering the full `HEIF::Reader` surface for FFI consumers (Python `cffi`, Rust `bindgen`, Go `cgo`, .NET P/Invoke, Emscripten `cwrap`). Includes:
  - File and stream-based initialisation.
  - Brand and file-level metadata queries.
  - Master image and type-filtered item enumeration.
  - Reference navigation (forward and inverse).
  - Item property enumeration and typed accessors (ispe, irot, imir, pixi, colr/nclx/ICC, pasp, clap, clli, mdcv, cclv, amve, reve, ndwt, lsel, tols, oinf, subs, auxc, rref, udes, altt, crtt, mdft, iscl, rloc, raw).
  - Grid, overlay, and tonemap/gainmap derived-item accessors.
  - Entity group enumeration and group-keyed properties.
  - Decoder-ready bitstream extraction (items and track samples).
  - Decoder configuration and parameter set access.
  - Full track/sequence surface: track info, samples, edit lists, sample flags, decoding order, decode dependencies, sample groupings (eqiv, stmi, refs, trif, spor, sulm, minp).
  - DASH segmented parsing (init segment, media segments, segment index, invalidation).

- **Pure-C error surface** (`heif_error_c.h`). Thread-local structured error inspection: code, origin, message, file, line, cause.

- **Pure-C version queries** (`heif_version_c.h`). Runtime version string, triple, and packed number. Stateless error-code and error-origin name lookups.

- **ABI handshake struct** (`heif_abi_handshake_t`). Size-prefixed POD for safe old-caller / new-library interoperation.

### Added: Reader and Writer Features

- **Mini-box (`mif3`) reading and writing.** Compact mini-box format support including tone-map (`tmap`) and gain-map (`gmap`) derived items.

- **HDR item-property surface.** Typed accessors for `clli`, `mdcv`, `ndwt`, `cclv`, `amve`, and `reve` properties, with the corresponding box types registered in the box factory so casts succeed at the reader surface.

- **L-HEVC multi-layer support.** Typed `lhvC` decoder configuration property, `lhv1` item recognition in the reader, and `oinf` operating-points-info typed property with full nested structure.

- **Sub-sample information (`subs`) as a first-class typed property.**

- **Layer Selector (`lsel`) and Target OLS (`tols`) typed properties.**

- **In-tree VVC parser** (`srcs/vvc-parser/`). Lightweight hand-written parser for VPS, SPS, PPS, and OPI NAL units. Feeds VVC decoder configuration, profile/tier/level extraction, and operating-point info into the reader and writer surfaces with no external dependency.

- **`readerapp --dump` JSON mode.** Human-readable JSON summary of any HEIF/HEIC/HMG file covering all file, item, and track metadata.

- **Overhauled reader/writer C++ API examples** (`srcs/examples/example.cpp`). Fifteen self-contained, runnable examples redesigned for clarity and approachability, targeting the raw `HEIF::Reader` / `HEIF::Writer` surface. Covers HEVC read and write, VVC mini-box write and expand, primary plus alpha layer, the error-handling protocol (return code plus `HEIF::ErrorInfo`), audio/video mux, DASH segment-streaming playback, and raw property round-trip.

- **HEIFPP worked-examples binary** (`srcs/examples/heifpp_example.cpp`). Six examples targeting the higher-level `HEIFPP` C++ API: load, iterate, properties, grid walk, write, and error handling.

- **Rich error diagnostics.** Thread-local `HEIF::ErrorInfo` side channel populated alongside integer return codes. Includes error code, origin subsystem, human-readable message, source location, and upstream exception cause. Process-wide error sink for centralised logging.

- **Trace sink** (`HEIF::setTraceSink`). Process-wide callback that centralises library diagnostic output. When no sink is installed, output passes through to `std::cerr` / `std::cout` unchanged.

- **One-shot convenience wrappers.** Inline `HEIF::readItem*` functions that encapsulate the size-probe protocol into a single call returning `Array<uint8_t>`.

### Added: Build and Packaging

- **Umbrella CMake target `heif::heif`.** Single `find_package(heif)` plus `target_link_libraries(myapp PRIVATE heif::heif)` replaces manual granular target assembly. The granular targets (`heif::heif_shared`, `heif::heif_writer_shared`, `heif::heifpp`, and their static counterparts) remain available for consumers who want fine-grained control.

- **`CMakePresets.json`.** Pre-defined presets for Debug, Release, RelWithDebInfo, and MinSizeRel builds.

- **Install component partitioning.** `cmake --install --component` supports `runtime`, `dev`, and `jni` components.

- **`heif.pc` pkg-config file.** Installed to `${libdir}/pkgconfig/` for non-CMake consumers.

- **`[[nodiscard]]` on `Reader::Create()` and `Writer::Create()`.**

- **WebAssembly BigInt support** (`-sWASM_BIGINT`).

- **Numeric version macros** (`HEIF_VERSION_MAJOR`, `_MINOR`, `_PATCH`, `HEIF_VERSION_NUMBER`, `HEIF_VERSION_AT_LEAST`).

- **`com.nokia.heif.BuildInfo`**: Java-tier version constants.

- **Emscripten integration.** `HEIF_DLL_PUBLIC` expands to `EMSCRIPTEN_KEEPALIVE` under emcc, eliminating manual export lists.

- **JavaScript / WASM build flow.** `build-with-emcmake.sh` now accepts `--prefix DIR` to build and install in one command. The deployable bundle (wasm glue, library, JS API, TypeScript declarations, package manifest) is auto-staged in `bindings/javascript-wasm/dist/` as a single drop-in directory.

### Changed

- **`BuildInfo::Version`** now uses `PROJECT_VERSION` plus an optional `+<git-sha>` suffix instead of raw `git describe`.

- **Compiler-warning cleanup across the codebase.**

### Deprecated

- **`CUSTOM_NEW` / `CUSTOM_DELETE` macros.** Replaced by type-safe function templates: `customNew<T>()`, `customDelete<T>()`, and `makeCustomUnique<T>()`. The macros remain available but new code should use the templates.

### Fixed: Reader

- `BitStream` copy constructor out-of-bounds read at stream end.
- `Heif::getTrack` missing bounds check on index.
- `getProperty(RawProperty)` out-of-bounds read on short data (less than 8 bytes).
- `MetaBox::getConfigBox` / `getConfigType` null-pointer dereference on property gaps.
- `extractItemInfoMap` null-pointer dereference on missing ISPE.
- `extractMetaBoxItemPropertiesMap` null-pointer dereference on missing RREF.
- Several `getProperty` overloads dereferenced `getPropertyByIndex` result without a null check.
- `ImageGrid` / `ImageOverlay` writer used `&&` instead of `||` for 32-bit field decision, silently truncating large single-dimension values.
- `heif_clap_t` offset fields were unsigned but the spec requires signed.
- `getItemsInDecodingOrder` over-emitted per composition time and duplicated dependency samples.
- `extractItems` did not admit `tmap` derived items, causing all tonemap accessors to return `NOT_FOUND`.
- `getFileInformation` did not populate `frameRate`, `hasTypeInformation`, `type.*`, or per-sample `segmentId` / `sampleFlags` / `earliestTimestamp` fields.
- Track `getItemDataWithDecoderParameters` probe size did not propagate due to an error-code spelling mismatch.
- Stream short-read: partial reads now loop to fill the buffer instead of failing immediately.
- Negative stream-read return mishandled as successful.
- Stream sticky-failure: successful `seek()` now resets error flags.
- `Reader::getItem(Tonemap)` never populated four `GainMapData` fields.

### Fixed: Writer

- Sample-to-group index off-by-one for `spor`, `spid`, `trif`, and `sulm`.
- `~WriterImpl` and `WriterImpl::clear` double-freed caller-owned `OutputStreamInterface*` pointers.
- Win32 `FileOutputStream::write` silent truncation above 2 GiB.

### Fixed: General

- `Reader::Create`, `Writer::Create`, `HeifReaderImpl::close`, and `HEIFPP::Heif::~Heif` crash under `bad_alloc` allocator pressure.
- Custom-allocator dangling-pointer at process exit when the embedder allocator was destructed first.
- Custom-allocator lazy-default install blocked legitimate embedder install.
- JNI `checkError` use-after-clear of the thread-local error slot.

## [3.7.1] 2025-03-11

### Fixed

- Missing `<limits>` include that caused build failures on some compilers.

## [3.6.0] 2020-06-25

### Added

- Support for several new properties.
- Image items with decoding dependencies.
- Extended type box and type combination box.
- Associating properties for entity groups.
- Reader support for segmented tracks.

### Changed

- Minor improvements and code cleanup.

## [3.5.0] 2019-07-10

### Improved

- Support for JPEGs and edit lists.

### Fixed

- Bug fixes, minor improvements and code cleanup.

## [3.4.0] 2018-11-06

### Changed

- Java API improvements.

### Fixed

- Bug fixes, code cleanup and robustness improvements.

## [3.3.0] 2018-06-13

### Added

- Java desktop build.
- Java API track and image sequence support.
- Java API entity grouping support.

## [3.2.0] 2018-04-06

### Added

- Java API wrapping the C++ libraries using JNI.
- Convenience C++ API (used by the Java API) wrapping the underlying reader and writer.

## [3.1.0] 2018-03-14

### Changed

- Reader API update.

### Fixed

- Bug fixes and examples update.

## [3.0.0] 2018-03-05

### Changed

- Writer executable replaced with writer library / API.
- Implementation based on the HEIF standard specification ISO/IEC 23008-12:2017.
