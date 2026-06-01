# 00 - Architecture Overview (read this first)

## What this page covers

A 10-minute tour of *what is in this repository*, written for someone
who has never opened the source tree before. By the end of this page
you'll know:

* What the four API layers are and **which one you should reach for**.
* Why there are two C++ APIs, and what trade-off each one is making.
* Where the Java / Python bindings sit in that picture.
* What the library does **not** do (so you know what to plug in next
  to it).

Every other page in this wiki assumes the vocabulary established here.

> **License scope reminder:** the default Nokia HEIF License v2.1
> covers non-commercial evaluation, testing, and academic research.
> Codec patents (H.264 / H.265 / H.266) are **not** granted. See
> [README § License](../README.md#license) before integrating.

---

## What this library actually is (and isn't)

This is a **container parser and writer** for the HEIF / HEIC family
of files. These are built on the ISO Base Media File Format (the same
box-tree format as MP4). Concretely, the library can:

* Walk the box tree of a HEIF/HEIC file and surface its items,
  tracks, properties, groups, and references.
* Resolve derived images conceptually (grids, overlays, identity,
  derived items).
* Hand you the **encoded bitstream** for each coded image item
  (HEVC / AVC / VVC / JPEG bytes) plus the matching decoder
  configuration.
* Build the inverse - assemble those pieces into a valid HEIF/HEIC
  file on the writer side.

It deliberately does **not**:

* Decode pixels. Feed the bitstream to your codec of choice
  (`vvdec`, `libde265`, the OS HEIC codec, `pillow_heif`, PyAV, …).
* Encode pixels. Bring your own encoder; the library only assembles
  the container around an already-encoded bitstream.
* Do colour management, ICC transforms, EXIF rotation, or any other
  pixel-domain transform. It only provides the metadata describing them.

Keep that in mind when reading the rest of this wiki: every API
described here moves **container metadata + encoded bitstreams**, not
images-as-pixels.

---

## The four API layers

The repository exposes four concentric tiers. Pick the one that
matches your *consumer*, not your taste:

### Layer 1: Native C++ (raw core)

- **Headers:** `heifreader.h`, `heifwriter.h`, `heifcommondatatypes.h`,
  `heifstreaminterface.h`, `heifallocator.h`, `heiferrorinfo.h`
- **CMake target:** `heif::heif` (umbrella - prefers shared if both
  are installed), or pick one explicitly: `heif::heif_static` /
  `heif::heif_shared` (+ writer twins)
- **Linkage:** Direct static or shared link
- **ABI promise:** None (C++ vtable, mangling, exception specs)
- **Audience:** C++ apps that want the smallest possible surface and
  are happy to handle `Reader::Create()`/`Destroy()`, raw `ErrorCode`
  returns, and `Array<T>` POD types.

### Layer 2: HEIFPP (opinionated C++ wrapper)

- **Headers:** `heifpp/Heif.h`, `Reader.h` (~70 headers)
- **CMake target:** `heif::heifpp`
- **Linkage:** Static link (always rebuild against matching headers)
- **ABI promise:** None (C++ name mangling changes between minor
  versions)
- **Audience:** **Most C++ consumers of this library.** Apps that
  want RAII, parent-owned object graphs, idiomatic exceptions, and a
  higher-level domain model.

### Layer 3: C ABI facade

- **Headers:** `heifreader_c.h`, `heif_error_c.h`, `heif_version_c.h`
  (reader side only; no `heifwriter_c.h` in v4.0.0)
- **CMake target:** Lives inside `heif::heif_shared` (and
  `heif::heif_static`); reachable through the `heif::heif` umbrella
- **Linkage:** Either, but the **shared** form is the point
- **ABI promise:** **Yes**, `SOVERSION 4` today
- **Audience:** FFI consumers: Python `cffi`, Rust `bindgen`, Go
  `cgo`, .NET P/Invoke, Swift module-map, Emscripten `cwrap`, etc.

### Layer 4: Language bindings on top of layer 3

- **CMake target:** Not exported via `find_package` (each binding has
  its own build flow under `bindings/`)
- **Linkage:** Each binding loads `libheif_shared.so.4` at runtime,
  or links the C facade into its own wasm/native artefact
- **ABI promise:** Inherits layer 3's promise
- **Audience:** End-user code in not-C++ languages. Today:
  Python (`bindings/python/`) and JavaScript / WebAssembly
  (`bindings/javascript-wasm/`). Rust, Go, Swift, .NET are
  candidates - the binding-author guide ([07](07-binding-authors.md))
  covers any of them.

The Java/Android tier (`heifjni` + `com.nokia.heif.*`) sits on
**layer 2** today - the JNI translates Java `long` handles to
`HEIFPP::*` pointers - and is the original reason HEIFPP exists.
See [04 - Java & Android](04-java-and-android.md) for the full story
and the binding-author guide ([07](07-binding-authors.md)) for why a
hypothetical future rewrite onto layer 3 is interesting.

For one specific cross-cutting concern that surfaces on every layer
above - *how raw bytes (image bitstreams, sample bitstreams,
Exif/XMP/MIME, `sinf`) actually leave the library* - see
[08 - Byte-Fetch Protocol](08-byte-fetch-protocol.md). It is the
single source of truth for the probe-and-fetch convention and shows
which layers hide it (HEIFPP, JNI) and which expose it (raw `Reader`,
C facade, bindings).

---

## Why two C++ APIs (1 and 2) coexist

This is the part that surprises newcomers. The short version: they
optimise for two different things.

### Layer 1 (`HEIF::Reader`) is service-shaped

It looks like an OS service: factory `Create()` / `Destroy()`, every
method returns `ErrorCode` and writes through out-params, collections
come back as the project-internal `HEIF::Array<T>`. There are no
exceptions, no inheritance for the consumer, no implicit ownership.
It's the smallest possible surface above the parser:

```cpp
auto* reader = HEIF::Reader::Create();
HEIF::ErrorCode rc = reader->initialize(argv[1]);
HEIF::FileInformation info;
reader->getFileInformation(info);
HEIF::Reader::Destroy(reader);
```

That shape is great if you want to **wrap or proxy** it (which is
exactly what HEIFPP and the C facade both do), and tolerable for
direct use, but it's not very *idiomatic* C++.

### Layer 2 (`HEIFPP::Heif`) is domain-shaped

It models a HEIF file as an object graph. From `srcs/api-cpp/Heif.h`:

> `HEIFPP::Heif` is the **sole owner** of every domain object it
> produces: `Track`, `Sample`, `Item` (and subclasses `ImageItem`,
> `MetaItem`, etc.), `ItemProperty`, `DecoderConfig`, `EntityGroup`,
> `AlternativeTrackGroup`.
>
> 1. Do not `delete` pointers returned by `getTrack()`, `getItem()`,
>    `getImage()`, `getProperty()`, `getDecoderConfig()`, `getGroup()`.
> 2. To remove an object, use the matching `removeXxx()` method.
> 3. Outliving the parent is undefined.
> 4. A single `Heif` instance is **not** thread-safe.

So as a HEIFPP user you write:

```cpp
HEIFPP::Heif heif;
heif.load("photo.heic");                      // returns HEIFPP::Result
for (auto* item : heif.getImageItems()) {     // borrowed pointers, owned by `heif`
    auto* pasp = item->getProperty<HEIFPP::PixelAspectRatioProperty>();
    if (pasp) { /* ... */ }
}
heif.save("out.heic");
// destructor cleans up the entire object graph
```

You get:

* a single owning root (the `Heif` instance) instead of a manual
  `Create` / `Destroy` pair;
* **typed property accessors** (`getProperty<T>()`) instead of "fetch
  by ID, switch on FourCC";
* a complete writer model - `addImage()`, `addProperty()`,
  `addEntityGroup()`, `addTrack()`, etc.;
* I/O through `InputStream` / `OutputStream` interfaces (mirrored
  into Java as `com.nokia.heif.io.*`).

That is much closer to what a typical C++ application wants. The cost
is the ownership rules above (don't outlive the parent) and the lack
of a binary-stability promise: you must rebuild HEIFPP-linked code
when the library version changes.

### Layer 3 (`heif_reader_c.h`) is FFI-shaped

The C facade is *not a C++ API in disguise*. It exists because
layers 1 and 2 cannot safely cross a language boundary:

* `HEIF::Reader` is C++ (`extern "C++"`), may throw `std::bad_alloc`,
  and returns `HEIF::Array<T>` whose virtual destructor cannot be
  called safely from another language.
* HEIFPP is C++ *and* unstable: every minor version bump can change
  the compiled symbol names.

The facade uses a different approach: opaque handles, `extern "C"`,
integer error codes, a `thread_local` slot for diagnostics, plain
data structs with explicit `_reserved[N]` padding, no exceptions,
`(_count, _at)` accessors for collections, and snprintf-style size
probes for byte payloads. The eight design rules at the top of
`srcs/api/reader/heifreader_c.h` ensure that Python `cffi`, Rust
`bindgen`, Go `cgo`, .NET P/Invoke, and Swift module-map can all
consume the same header without surprises.

---

## Picking a tier (the "what should I link?" decision)

```
                Are you writing C++?
                /                  \
              YES                   NO
              /                       \
   Do you want RAII +              Use heif_reader_c.h
   typed properties +              + your language's
   exceptions +                    FFI tool
   "feels like a C++ library"?     (cffi / bindgen /
        /        \                  cgo / P/Invoke / …)
       YES        NO
       /           \
  heif::heifpp     heif::heif (umbrella)
  (HEIFPP)         or heif::heif_static
                   (raw HEIF::Reader)
```

In plain English:

* **C++ application code**: link `heif::heifpp`. This is where most
  C++ consumers should be. It is stable enough to use; the only catch
  is "rebuild when the library version changes". The worked example
  is `srcs/examples/heifpp_example.cpp` (which mirrors what
  `srcs/api-java/java/com/nokia/heif/example/HEIFExample.java` does
  on the Java side).
* **Embedded / resource-constrained C++**: link `heif::heif_static`
  directly (or `heif::heif` if you do not care which form is
  selected) to skip the HEIFPP wrapper. Smaller binary, but you
  get the raw `Create` / `Destroy` / `ErrorCode` shape. The worked
  example is `srcs/examples/example.cpp` (fifteen demos covering
  reader, writer, the structured error model in `example12()`,
  MP4 mux, DASH segment-streaming, and raw property round-trip).
* **Anything that must `dlopen()` the library and live across
  versions**: go through `heifreader_c.h`, regardless of language.
  Even C consumers benefit, because the C facade has a real ABI
  policy and the raw `HEIF::Reader` does not. A writer-side C
  facade is not part of v4.0.0; producing files from a non-C++
  language requires either calling HEIFPP through hand-written FFI
  or pre-building HEIF assets ahead of time. The Python binding
  (`bindings/python/heif/`) and the JS/WebAssembly binding
  (`bindings/javascript-wasm/`) are the worked references for
  consuming the C facade from another language.

---

## Where each piece lives

| Concern | Layer | Target | Worked example |
|---|---|---|---|
| Read HEIC from C++ (recommended) | HEIFPP | `heif::heifpp` | `srcs/examples/heifpp_example.cpp` |
| Read HEIC from C++ (minimum surface) | Raw | `heif::heif` or `heif::heif_static` | `srcs/examples/example.cpp` |
| Read HEIC from C / Rust / Go / .NET / Swift | C facade | `heif::heif_shared` | `bindings/python/heif/_reader.py` |
| Read HEIC from Java | JNI on HEIFPP | `heifjni` | `HEIFExample.java` |
| Read HEIC from Android | JNI on HEIFPP | AAR | `android-example/` |
| Read HEIC from Python | C facade (via cffi) | `import heif` | `bindings/python/README.md` |
| Read HEIC from JavaScript / browser | C facade (via wasm) | `heif.js` + `heif.wasm` | `bindings/javascript-wasm/README.md` |
| Write HEIC from C++ | HEIFPP or Raw | `heif::heifpp` or `heif::heif` | `heifpp_example.cpp` (single + grid); `example.cpp` writer demos (7, 9, 11, 13, 15) |
| Write HEIC from C / FFI | **Not yet possible** | No writer-side C facade in v4.0.0 | - |
| Error model (raw C++ tier) | Raw | `heiferrorinfo.h` | `example.cpp::example12` |
| Error model (HEIFPP) | HEIFPP | `heifpp/ErrorContext.h` | `heifpp_example.cpp` |
| Error model (Java) | JNI | `ErrorHandler.setErrorSink` | [04 - Java](04-java-and-android.md) |
| Decode / encode pixels | **Not provided** | Bring your own codec | - |

---

## How the bindings hang off the C facade

Layer 4 is the easy one once layer 3 is in place. Every binding
follows the same recipe (codified in [07 - Writing a Language
Binding](07-binding-authors.md)):

1. Load `libheif_shared.so.4` (major-versioned filename - never the
   unversioned symlink).
2. Generate a `cdef` / `bindgen` / `cgo` view of `heifreader_c.h` +
   `heif_error_c.h`.
3. Wrap `heif_reader_t*` in the language's deterministic-cleanup
   primitive (Python `__enter__`/`__exit__`, Rust `Drop`, Go
   `runtime.SetFinalizer`, C# `IDisposable`, Swift `deinit`, Java
   `AutoCloseable`).
4. Build one `_check(rc, where)` helper that drains the error
   slot on every non-OK return and raises the binding's exception
   type carrying all eight diagnostic fields.
5. Build one size-probe helper for variable-length byte/string
   getters.
6. Document the per-instance threading rule verbatim from
   [06 - Threading](06-threading.md).

The Python binding (`bindings/python/heif/`) is the worked
reference; the JavaScript / WebAssembly binding
(`bindings/javascript-wasm/`) shows the same recipe in a
radically different host environment. Read at least one of them
before starting a new binding in any language.

---

## Current status

> These pages document **v4.0.0**. See `CHANGELOG.md` for the
> full list of changes since v3.7.1.

Key additions in the v4.0.0 cycle:

* **Structured error model**: a `thread_local` `HEIF::ErrorInfo`
  slot, a process-wide sink (`HEIF::setErrorSink`), and a reference
  `HEIF::stderrErrorSink`. Fully covered in [03 - Error
  Handling](03-error-handling.md).
* **Thread-safety contract**: documented at the top of
  `srcs/api/reader/heifreader.h` and mirrored in
  [06 - Threading](06-threading.md).
* **Reader-side C facade** (`heifreader_c.h` +
  `heif_error_c.h` + `heif_version_c.h`) covering header, items,
  properties, grids, overlays, tone-map gainmaps, file / metabox /
  item features, decoder dependencies and parameter sets,
  protection scheme, brand combos, matrix, custom stream callbacks,
  and the full track / sample surface (track info, sample
  timestamps, edit lists, segment streaming, the seven VVC
  subpicture sample groupings).
* **Reference bindings**: Python (`bindings/python/`) and
  JavaScript / WebAssembly (`bindings/javascript-wasm/`), plus the
  binding-author guide ([07](07-binding-authors.md)).
* **`heif::heif` umbrella CMake target** so first-touch consumers
  do not have to choose between `_static` and `_shared` up front.

Not in v4.0.0 (candidates for later majors):

* **Writer-side C facade.** `heifwriter_c.h` does not exist; layer 3
  is read-only. Writing still requires layer 1 or 2.
* **Additional language bindings** (Rust, Go, Swift, .NET) - the
  binding-author guide is ready for them; the bindings themselves
  are not in the tree.

---

## Where to go next

* Brand new? Start with [01 - Getting Started](01-getting-started.md)
  to build the library, then [02 - Linking Your
  Project](02-linking-your-project.md) to link it.
* Already have an app that calls HEIFPP or `HEIF::Reader`?
  [03 - Error Handling](03-error-handling.md) and
  [06 - Threading](06-threading.md) are the two most-cited pages.
* Writing Java / Android code?
  [04 - Java & Android](04-java-and-android.md).
* Writing a binding in a new language?
  [07 - Writing a Language Binding](07-binding-authors.md). Start
  there; do not start by reading `heifreader_c.h` cold.
* Looking up a specific CMake target or installed file?
  [05 - CMake Options & Targets](05-cmake-options-and-targets.md).
