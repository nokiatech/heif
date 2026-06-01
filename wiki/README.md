# HEIF Library - Wiki Pages

These pages are the **how-to** companion to the top-level `README.md`.
The README answers *"what is this and how do I build it?"*; these pages
answer *"how do I actually use it in my own project?"*

These files live in-tree (`wiki/`) so they version with the code.
They are written in plain Markdown and intended to be mirrored 1:1 to
the project's GitHub Wiki - the file names map directly to wiki page
slugs.

---

## [!] Before you invest time: license scope

This library ships under the **Nokia HEIF License v2.1**, which permits
use only for **non-commercial evaluation, testing, and academic
research**. The license also **does not** grant any codec patent rights
(H.264 / H.265 / H.266) - those must be licensed separately for any
real-world deployment.

If you're prototyping, evaluating, or researching, you're in scope and
these pages will get you running. If you're heading toward a product or
a revenue-generating integration, contact **<heif@nokia.com>** *before*
investing in the integration work - Nokia is open to commercial
licensing arrangements but the default license does not cover that path.

Full terms: [LICENSE.TXT](../LICENSE.TXT). Top-level [README § License](../README.md#license)
has a plainer-language breakdown.

---

## For people brand new to the library

| Start here | What you'll do |
|---|---|
| **[00 - Architecture Overview](00-architecture-overview.md)** | 10-minute tour of the four API layers, why two C++ APIs coexist, and a decision flowchart for "which target should I link?". **Read this first.** |
| **[01 - Getting Started](01-getting-started.md)** | Clone, build, and run a HEIF demo binary in 5 minutes. |
| **[02 - Linking Your Project](02-linking-your-project.md)** | Build a new application that links against this library using `cmake --install` and `find_package(heif)`. |

## For people who already have an application

| Page | Topic |
|---|---|
| **[03 - Error Handling](03-error-handling.md)** | Return codes, the thread-local diagnostic slot, the sink hook, and the formatter helpers across all three tiers (raw C++, HEIFPP, Java). |
| **[04 - Java & Android](04-java-and-android.md)** | Building the JNI bridge for desktop and Android. Where the JAR/AAR ends up and how to use the error sink from Java. |
| **[05 - CMake Options & Targets](05-cmake-options-and-targets.md)** | Complete reference of every CMake option, exported target, install component, and installed header. |
| **[06 - Threading & Concurrency](06-threading.md)** | Thread-safety contract for every public surface. Pattern recipes for single-threaded, server, and pool embedders. |
| **[07 - Writing a Language Binding](07-binding-authors.md)** | How-to for wrapping the C facade in another language. The seven things every binding must get right, with a pre-flight checklist. |
| **[08 - Byte-Fetch Protocol](08-byte-fetch-protocol.md)** | How raw bytes leave the library across every layer. Covers the probe-and-fetch protocol and where each layer hides or exposes it. |
| **[09 - Asyncify & Streaming](09-asyncify-and-streaming.md)** | When wasm bindings need Emscripten's `-sASYNCIFY` for on-demand byte fetch, when they do not, and the build-flag recipe. |
| **[10 - ABI Policy](10-abi-policy.md)** | What the library promises across versions: SOVERSION, semantic versioning, compatibility guarantees, deprecation policy, CRT/STL guidance. |
| **[11 - MP4 / ISOBMFF Support](11-mp4-support.md)** | What MP4 functionality the library provides, what it does not, codec/handler/brand coverage, fragmented-MP4 reader, and FAQ. |

---

## How these pages are organised

Each page follows the same structure so you can skim:

1. **What this page covers** (1-paragraph scope)
2. **What you need before starting** (prerequisites + assumed knowledge)
3. **Walkthrough** (numbered steps, every command shown in full)
4. **Common problems** (real failures + how to recognise them)
5. **Where to go next** (links to deeper pages or source files)

Code blocks that you should run literally are tagged with the shell:

```bash
# Run this in your terminal
cmake --build build
```

Code blocks that go *into a file* are tagged with the language and
captioned with the path:

```cmake
# CMakeLists.txt
find_package(heif REQUIRED)
```

---

## Source-of-truth reference

When the wiki and the source disagree, the source wins. Authoritative
locations for each topic:

| Topic | File |
|---|---|
| C ABI facade | `srcs/api/common/heif_error_c.h`, `srcs/api/common/heif_version_c.h`, `srcs/api/reader/heifreader_c.h` |
| Raw C++ API (reader/writer) | `srcs/api/reader/heifreader.h`, `srcs/api/writer/heifwriter.h`, `srcs/api/common/heiferrorinfo.h` |
| HEIFPP (C++ convenience wrapper) | `srcs/api-cpp/` |
| Java bindings | `srcs/api-java/java/com/nokia/heif/` |
| JNI bridge | `srcs/api-java/cpp/` |
| Error-handling slot + sink | `srcs/api/common/heiferrorinfo.h`, `srcs/common/heiferrorinfo.cpp` |
| Demo / example apps | See [Examples by tier](#examples-by-tier) below |
| Install-time CMake config | `srcs/cmake/heifConfig.cmake.in` (umbrella + granular targets) |

---

## Examples by tier

Every example shipped in-tree, indexed by which API tier it exercises
and who it's aimed at. All paths are relative to the repository root.

### Raw `HEIF::Reader` / `HEIF::Writer` (C++ - minimum surface)

For C++ developers who need the smallest possible API surface,
explicit `Create()` / `Destroy()` lifetimes, and direct `ErrorCode`
returns. Built by default; the binary lands in `build/bin/`.

| Example | What it shows |
|---|---|
| **`srcs/examples/example.cpp`** | 15 self-contained `exampleN()` functions: primary item (1), thumbnail fallback (2), master->thumbnail map (3), derived `iden` rendering (4), image-sequence track walk (5), alternative-track selection (6), HEIF copy / round-trip write (7), Exif metadata extraction (8), VVC MiniBox write (9), VVC MiniBox expand (10), HEIF + alpha-layer auxiliary write (11), raw-C++-tier error model (12), MP4 mux (13), DASH segment-streaming playback (14), and raw-property round-trip (15). The historical reference for "how do I do X with `HEIF::Reader` / `HEIF::Writer`?" |

### HEIFPP (`heif::heifpp` - recommended for new C++ apps)

For C++ developers who want a single owning root (`HEIFPP::Heif`),
typed property accessors, factory construction, and the structured
`ErrorContext` slot. Built by default.

| Example | What it shows |
|---|---|
| **`srcs/examples/heifpp_example.cpp`** | 6 demos: load + iterate master images, primary + property accessors, grid walk via `dynamic_cast`, single-image write, 3×2 grid write, and the HEIFPP error model (`setErrorSink` + `getLastError` + `formatErrorContext`). The HEIFPP mirror of `HEIFExample.java`. |

### C facade (`heif::heif_shared` - for FFI / non-C++ callers)

For developers binding from C, Rust, Go, Swift, .NET, JavaScript /
WebAssembly, etc. - the opaque-handle ABI behind `heifreader_c.h`
(`SOVERSION 4`). Two in-tree reference bindings consume this surface:
the Python binding (`bindings/python/heif/`) over `cffi`, and the
JS/WebAssembly binding (`bindings/javascript-wasm/`) over Emscripten
`cwrap`. Either one is a worked reference for how to consume the C
facade from another language. See [07 - Writing a Language
Binding](07-binding-authors.md) for the full guide.

### Language bindings (Python, JavaScript / WebAssembly, Java)

For developers integrating from a managed or web language. Each
binding is itself a worked example of the C facade or JNI bridge.

| Example | What it shows |
|---|---|
| **`bindings/python/heif/`** + **`bindings/python/README.md`** | Python via `cffi` over the C facade (`libheif_shared.so.4`). Reference binding implementation documented in [page 07](07-binding-authors.md). |
| **`bindings/javascript-wasm/`** + **`bindings/javascript-wasm/README.md`** | JavaScript / browser / Node via Emscripten `cwrap` over the C facade compiled to wasm. Demonstrates the pre-buffered path discussed in [page 09](09-asyncify-and-streaming.md). |
| **`srcs/api-java/java/com/nokia/heif/example/HEIFExample.java`** | Desktop Java via JNI on HEIFPP. Same scenarios as `heifpp_example.cpp`, expressed through `com.nokia.heif.HEIF`. Pair with [page 04](04-java-and-android.md). |

### How to run them

```bash
# Configure + build everything (default) - from the repository root
cmake -S . -B build && cmake --build build -j

# Run individual demos (need a HEIF asset on disk; grab one from
# https://github.com/nokiatech/heif_conformance and place as C003.heic)
./build/bin/example          path/to/file.heic
./build/bin/heifpp_example   path/to/file.heic   out.heic
```

The Python, JavaScript / WebAssembly, and Java examples have their
own run instructions on pages 07, 09, and 04 respectively.

---

## Page status

> **Note:** These pages document v4.0.0. See top-level
> `CHANGELOG.md` for the full list of changes since v3.7.1.

| Page | Last verified against |
|---|---|
| 00 - Architecture Overview | v4.0.0 |
| 01 - Getting Started | v4.0.0, Linux/GCC 12 |
| 02 - Linking Your Project | v4.0.0, Linux/GCC 12, CMake 3.25 |
| 03 - Error Handling | v4.0.0 |
| 04 - Java & Android | v4.0.0, OpenJDK 17 |
| 05 - CMake Options & Targets | v4.0.0 |
| 06 - Threading & Concurrency | v4.0.0 |
| 07 - Writing a Language Binding | v4.0.0 (Python + JS/wasm references) |
| 08 - Byte-Fetch Protocol | v4.0.0 |
| 09 - Asyncify & Streaming | v4.0.0, Emscripten 4.0 |
| 10 - ABI Policy | v4.0.0 |
| 11 - MP4 / ISOBMFF Support | v4.0.0 |

If you find a step that doesn't work as documented, that's a bug in
the wiki - please file an issue or PR against `wiki/`.
