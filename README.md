# High Efficiency Image File Format (HEIF)

![License](https://img.shields.io/badge/license-Nokia%20HEIF%20v2.1-blue)
![Version](https://img.shields.io/badge/version-4.0.0-green)

HEIF is a visual media container format standardized by the Moving Picture Experts Group (MPEG) for storage and sharing of images and image sequences. It is based on the well-known [ISO Base Media File Format (ISOBMFF)](https://nokiatech.github.io/heif/technical.html) standard.

HEIF Reader/Writer Engine is an implementation of the [HEIF standard](https://nokiatech.github.io/heif/technical.html), demonstrating its powerful features and capabilities.

For MIAF specific code, please switch to the [MIAF branch](https://github.com/nokiatech/heif/tree/miaf).

> **License in one line:** Nokia HEIF License v2.1. Free for non-commercial evaluation, testing, and academic research. **Codec patents are not granted.** For product or commercial use, contact <heif@nokia.com>. Full details in the [License](#license) section.

**Links:** [HEIF Web-Site](https://nokiatech.github.io/heif) · [Wiki](https://github.com/nokiatech/heif/wiki) · [File Examples](https://nokiatech.github.io/heif/gallery.html) · [Conformance Suite](https://github.com/nokiatech/heif_conformance) · [License](#license)

---

<br>

## Features

HEIF is a media container format, not an image or video encoder. The quality of the visual media depends on the encoder used ([HEVC](https://www.itu.int/rec/T-REC-H.265), [AVC](https://www.itu.int/rec/T-REC-H.264), [JPEG](https://jpeg.org/), or [VVC](https://www.itu.int/rec/T-REC-H.266)). The container can encapsulate HEVC/AVC/JPEG/VVC bitstreams and is extensible to future codecs.

Key capabilities:

- Encapsulate still images (HEVC/SHVC/MV-HEVC/AVC/JPEG/VVC) and image sequences (HEVC/SHVC/MV-HEVC/AVC).
- Storage based on the widely adopted ISO Base Media File Format (ISOBMFF).
- Efficient storage of image bursts and cinemagraphs.
- Supports computational photography use cases.
- Both lossy and lossless image data storage.
- A better, easier way to distribute still images, image collections, and related metadata.

See [HEIF file examples](https://nokiatech.github.io/heif/gallery.html).

---

<br>

## Contents of the Repository

- ISO Base Media File Format (ISOBMFF) box parse/write source code: `srcs/common/`
- HEIF Reader library implementation: `srcs/reader/`
- HEIF Writer library implementation: `srcs/writer/`
- C facade headers (ABI-stable public API): `srcs/api/reader/`, `srcs/api/writer/`, `srcs/api/common/`
- HEIF Reader/Writer Java API (JNI): `srcs/api-java/`
- HEIF Reader/Writer convenience C++ API (HEIFPP): `srcs/api-cpp/`
- VVC bitstream parser: `srcs/vvc-parser/`
- HEIF Reader API usage example code: `srcs/examples/`
- Source code documentation ([Doxygen](https://www.doxygen.nl/)) and project docs: `docs/`
- Python Reader Binding: `bindings/python/`
- JavaScript/WebAssembly Binding: `bindings/javascript-wasm/`
- HEIF Web Site Content: [gh-pages branch](https://github.com/nokiatech/heif/tree/gh-pages)

---

<br>

## Platform & Binding Support Matrix

The container layer can encapsulate and extract HEVC, AVC, JPEG, and VVC bitstreams. None of the surfaces decode or encode pixels.

| Surface | Read | Write | Notes |
|---|---|---|---|
| **C++ Reader/Writer API** (`srcs/reader/`, `srcs/writer/`) | Yes | Yes | Full spec implementation. |
| **C Facade** (`srcs/api/reader/`, `srcs/api/writer/`) | Yes | Yes | ABI-stable, SOVERSION 4. |
| **HEIFPP API** (`srcs/api-cpp/`) | Yes | Yes | Convenience C++ wrapper. |
| **Java API** (`srcs/api-java/`) | Yes | Yes | Desktop JVM via JNI. |
| **Python Binding** (`bindings/python/`) | Yes | No | Reader only. |
| **JavaScript / WASM Binding** (`bindings/javascript-wasm/`) | Yes | No | Reader only. |

---

<br>

## Building

> **Don't need to build from source?** Jump directly to the binding:
> **[Python quick-install](#just-want-to-read-a-heif-file-from-python)** · **[JavaScript / WebAssembly](#want-to-use-it-from-javascript--webassembly)**

**What this build produces:** the C++ library (`libheif_shared`), the C facade, the HEIFPP convenience C++ API, and the Java JNI bridge. The Python binding and the JavaScript / WASM binding are **separate self-contained builds** with their own toolchains; running the commands below does **not** build them, and they do **not** require this build to be run first. See the [Python](#just-want-to-read-a-heif-file-from-python) and [JavaScript / WebAssembly](#want-to-use-it-from-javascript--webassembly) sections for those.

### Prerequisites

- **[CMake](https://cmake.org/)** 3.5 or newer (3.25+ for [CMake Presets](#cmake-presets-recommended)).
- A C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+).

### CMake Presets (recommended)

The repository ships a `CMakePresets.json` with pre-configured build flavours, so individual `-D` flags are not needed:

```bash
git clone https://github.com/nokiatech/heif.git
cd heif
cmake --list-presets         # show available presets
cmake --preset release       # configure
cmake --build --preset release
```

Libraries land in the preset's build directory under `lib/`, executables under `bin/`. Requires CMake 3.25+.

### Manual Configure

```bash
git clone https://github.com/nokiatech/heif.git
cd heif
cmake -S . -B build
cmake --build build
```

This builds the **Release** configuration by default. For **Debug**:

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

<details>
<summary><strong>Legacy build (pre-3.14 CMake or existing <code>build/</code> tree)</strong></summary>

If your CMake is older than 3.14 (no `-S` / `-B` support), or you already have a `build/` directory configured against `srcs/`, the traditional in-tree approach still works:

```bash
cd build
cmake ../srcs
cmake --build .
```

This points CMake directly at the `srcs/CMakeLists.txt` sub-project. `CMakePresets.json` is not picked up in this mode (presets reference the repository root).

</details>

### Platform Build Scripts

Pre-made scripts in `build/`:

| Script | Description |
|---|---|
| `build/cmake_MSVS_Win64.bat` | Windows: configure + build 64-bit Release with Visual Studio. |
| `build/ios/build.sh` | macOS: cross-compile for iOS device + simulator via Xcode. |

Windows, from inside `build/`:

```bat
cmake_MSVS_Win64.bat
```

Or directly:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### CMake Options

| Option | Default | Description |
|---|---|---|
| `-DCMAKE_BUILD_TYPE=<type>` | `Release` | `Debug`, `Release`, `RelWithDebInfo`, or `MinSizeRel`. |
| `-DBUILD_ONLY_STATIC_LIB=ON` | `OFF` | Build only static libraries (no `.so` / `.dll`). |

A complete option reference lives in [wiki § 05: CMake Options & Targets](wiki/05-cmake-options-and-targets.md).

### Installing

```bash
cmake --install build --prefix /path/to/install
```

Output layout:

- Libraries (`.a` / `.so`): `<prefix>/lib/`
- C API headers: `<prefix>/include/heif/`
- C++ API headers: `<prefix>/include/heif/heifpp/`
- CMake package files: `<prefix>/lib/cmake/heif/`

### Try the Examples

The build produces two example binaries under `bin/` that double as the canonical worked-API documentation. Each one is a single self-contained `.cpp` file you can read top-to-bottom and copy-paste from.

| Binary | Source | API tier | What it shows |
|---|---|---|---|
| `example` | [`srcs/examples/example.cpp`](srcs/examples/example.cpp) | Raw `HEIF::Reader` / `HEIF::Writer` C++ API | Fifteen worked examples: HEVC read and write, VVC mini-box write and expand, primary plus alpha layer, the error-handling protocol (return code plus `HEIF::ErrorInfo`), audio/video mux, DASH segment-streaming playback, and raw property round-trip. |
| `heifpp_example` | [`srcs/examples/heifpp_example.cpp`](srcs/examples/heifpp_example.cpp) | Higher-level `HEIFPP` convenience C++ API | Six examples: load, iterate, properties, grid walk, write, and error handling. |

Run them directly after `cmake --build`:

```bash
./build/bin/example          # 15 reader/writer C++ API examples
./build/bin/heifpp_example   # 6 HEIFPP convenience API examples
```

If you are evaluating which API tier to target, read both source files side by side: the same task (open a file, list items, extract a bitstream) is implemented in each tier and the trade-off between control and convenience is visible in fewer than fifty lines.

### Linking From Another CMake Project

After installation, downstream projects can use `find_package`:

```cmake
find_package(heif REQUIRED)
target_link_libraries(your_target heif::heifpp)
```

Full walkthrough including a hello-world from scratch in [wiki § 02: Linking Your Project](wiki/02-linking-your-project.md).

### Java API

Requires Java 8+ and Gradle. After building the C/C++ library:

```bash
cd build/java-desktop
gradle build
```

The JNI library (`heifjni.dll` / `heifjni.so`) from the C/C++ build must be on the Java library search path. Full walkthrough in [wiki § 04: Java](wiki/04-java-and-android.md).

---

<br>

## Just Want to Read a HEIF File From Python?

The Python binding installs through `pip`. There is no separate "build the C++ library first" step: pip downloads the source, compiles the C++ library on your machine, bundles it together with the Python wrapper, and installs the result into your Python environment. One command does all of it.

### Step 1: Install the Build Tools

The compile step needs a C++ compiler, CMake, Git, and pip. Install them once per machine.

**Ubuntu / Debian:**
```bash
sudo apt install -y git cmake build-essential python3-pip python3-venv
```

**Fedora / RHEL:**
```bash
sudo dnf install -y git cmake gcc-c++ python3-pip
```

**macOS** (with [Homebrew](https://brew.sh)):
```bash
brew install git cmake python
# C++ compiler comes from Xcode Command Line Tools:
xcode-select --install
```

**Windows:** install [Python 3.9+](https://www.python.org/downloads/windows/) (tick *"Add python.exe to PATH"*), [Git for Windows](https://git-scm.com/download/win), and [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/visual-cpp-build-tools/) with the *"Desktop development with C++"* workload. Open a fresh **"x64 Native Tools Command Prompt for VS 2022"** for the next step.

### Step 2: Pick the Python Environment That Will Use the Binding

`pip` installs a package into whatever Python it is asked to run with. By default that is your system or user Python, which means the package becomes visible to *every* Python script on the machine. Most Python projects use a **virtual environment** (a "venv") to keep each project's dependencies isolated, so installing a package for one project does not affect any other. If you do not already use venvs, this is the right time to start.

**Option A: create a fresh venv** (recommended if you don't already have one).

A venv is just a folder containing an isolated Python install. You pick where it lives. The two common conventions are:

- **Inside the project that will use it**, named `.venv`. This is the standard for project-bound venvs and is what most IDEs and tools look for automatically.
- **In a dedicated "playground" folder** when you are just trying the binding out and don't have a project yet.

Pick a location, then create and activate the venv. For example, in your project directory:

```bash
cd /path/to/your/project          # or: mkdir ~/heif-playground && cd ~/heif-playground
python3 -m venv .venv             # creates the .venv/ folder right here
source .venv/bin/activate         # Linux / macOS
# or on Windows:  .venv\Scripts\activate
```

Your shell prompt changes to show the venv name. From this point, `pip` and `python` mean the venv's pip and python; nothing leaks outside this directory. To leave the venv later: `deactivate`. To delete it entirely: just `rm -rf .venv`.

**Option B: activate an existing project venv** (when the binding will be used from a specific project):

```bash
cd /path/to/your/python/project
source .venv/bin/activate          # Linux / macOS
# or on Windows:  .venv\Scripts\activate
```

**Option C: skip venvs entirely** and install into your system or user Python. Fine for one-off experiments, not recommended for project work; just proceed to Step 3.

### Step 3: Install the Binding

Pick one of the two install paths. Both run the same compile step (a few minutes on the first run) and end with `Successfully installed nokia-heif-4.0.0`.

**Path A: install direct from GitHub.** No clone, no source on disk. Works from any directory:

```bash
python3 -m pip install "git+https://github.com/nokiatech/heif.git@v4.0.0"
```

(On Windows use `python` instead of `python3`.)

**Path B: install from a local clone.** Use this when you want the source on disk to inspect, switch branches, or develop against:

```bash
git clone https://github.com/nokiatech/heif.git
cd heif
python3 -m pip install .          # the dot means "build and install this directory"
```

### Step 4: Use It

Make sure the same environment from Step 2 is still active. Then open a Python script or shell and import the binding.

> **Note:** the *install* name is `nokia-heif` but the *import* name is `heif`.

```python
import heif

with heif.Reader("photo.heic") as r:
    print(r.major_brand)                       # e.g. "mif3"
    for img in r.master_images:
        print(img.item_type, img.width, img.height)
        payload = img.bitstream()              # encoded bytes
```

The binding **does not decode pixels**. `payload` is an encoded HEVC / AVC / VVC / JPEG bitstream; feed it to whichever decoder you already use (`pillow_heif`, `imageio`, PyAV, `vvdec`, ...).

### Next: The Worked Example

The four-line snippet above is the smallest useful program. For an end-to-end walkthrough that exercises the full reader surface (master images, properties, tracks, gainmaps, entity groups, DASH segments), read and run the example script:

- [`bindings/python/examples/read_heif.py`](bindings/python/examples/read_heif.py)

```bash
python3 bindings/python/examples/read_heif.py path/to/photo.heic
```

It is the Python counterpart to `srcs/examples/example.cpp` and the recommended starting point for anyone going beyond "open a file and read item types".

### What You Just Installed

- The package is registered with pip under the name `nokia-heif`. Run `pip list` to confirm it is there.
- The compiled C++ library (`libheif_shared.so.4` on Linux, `.dylib` on macOS, `.dll` on Windows) lives inside the installed package directory; no system-wide library install was needed.
- To use the binding from another project, install it into that project's environment too (activate the project's venv, then repeat Step 3).
- To remove it: `pip uninstall nokia-heif`.

This binding is **reader only** at this time; writing HEIF files from Python is not supported.

For the full API reference, the in-tree editable-install workflow for binding development, troubleshooting, and update instructions, see [`bindings/python/README.md`](bindings/python/README.md).

---

<br>

## Want to Use It From JavaScript / WebAssembly?

> ### See It Running Live
>
> The official HEIF web-site is the **live, in-production demo of this exact binding**:
>
> ## **[nokiatech.github.io/heif](https://nokiatech.github.io/heif)**
>
> Every image on the site is a HEIF, HEIC, or HMG file parsed in your browser by the WebAssembly build of this library. The [gallery](https://nokiatech.github.io/heif/gallery.html) is a working showcase: each tile is a real HEIF file fetched, parsed by `bindings/javascript-wasm/`, and handed off to a browser decoder for display. The [format-comparison sub-page](https://nokiatech.github.io/heif/comparison.html) is the only place that intentionally serves non-HEIF assets (JPEG, GIF), and only as direct side-by-side comparisons.
>
> Decoding pixels and deciding what to display with the parsed data is the *site's* job; the library only parses. The boundary between the two is exactly what `bindings/javascript-wasm/` exposes.
>
> **Tip:** open the browser dev-tools console on any page of the site and run `HEIFFileLogger.verbose = true`. Every subsequent parse prints the full extracted structure (boxes, items, properties, references) to the console. It is the fastest way to see what the binding actually returns for a given file without writing any code.

The `bindings/javascript-wasm/` directory contains a complete JS binding that compiles the library to WebAssembly via Emscripten, covering every C facade entry point. The build is self-contained: it produces its own static library, no separate C++ build is required first.

Build (requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) 4.0+ and CMake 3.12+):

```bash
source /path/to/emsdk/emsdk_env.sh
cd bindings/javascript-wasm
bash build-with-emcmake.sh
```

When the build finishes, the deployable bundle is staged in [`bindings/javascript-wasm/dist/`](bindings/javascript-wasm/dist/): copy that directory into your web project, or pass `--prefix /path` to the build script to install it directly.

Use in a web page:

```html
<script src="heif-raw.js"></script>
<script src="heif.js"></script>
<script>
  HEIF.load().then(function(heif) {
    fetch('photo.heic')
      .then(r => r.arrayBuffer())
      .then(buf => {
        const reader = heif.open(new Uint8Array(buf));
        console.log(reader.majorBrand, reader.itemSize(reader.primaryItemId));
        reader.close();
      });
  });
</script>
```

The binding **does not decode pixels**. `itemBitstream()` returns an encoded HEVC / AVC / VVC bitstream; feed it to your existing decoder (`libde265.js`, `vvdec`, etc.).

Full documentation: [`bindings/javascript-wasm/README.md`](bindings/javascript-wasm/README.md). TypeScript API reference: [`bindings/javascript-wasm/heif.d.ts`](bindings/javascript-wasm/heif.d.ts).

---

<br>

## Documentation & How-To Guides

The README above covers what the library is and how to build it. For everything beyond that (using the library in your own project, error handling in detail, Java integration, the full CMake options matrix), see the wiki pages under [`wiki/`](wiki/):

| Page | When you need it |
|---|---|
| [01: Getting Started](wiki/01-getting-started.md) | First-time build of the repository on your machine. |
| [02: Linking Your Project](wiki/02-linking-your-project.md) | Building your own application that links against the library. Covers `cmake --install`, `find_package(heif)`, and a full hello-world from scratch. |
| [03: Error Handling](wiki/03-error-handling.md) | The error-reporting model: integer return codes, the diagnostic slot, the process-wide sink hook, and the formatter helpers across all three tiers (C ABI, HEIFPP, Java). |
| [04: Java](wiki/04-java-and-android.md) | Building the JNI bridge for desktop JVM. |
| [05: CMake Options & Targets Reference](wiki/05-cmake-options-and-targets.md) | Complete lookup table of every build flag, exported target, and installed file. |
| [06: Threading & Concurrency](wiki/06-threading.md) | Normative thread-safety contract for every public surface (factory, `Reader` / `Writer` instance, error slot, sink, allocator, custom stream callbacks). Embedder pattern recipes for single-threaded, thread-per-instance, pool, and server consumers. |
| [07: Writing a Language Binding](wiki/07-binding-authors.md) | How-to for wrapping the HEIF C facade in another language (Rust, Go, .NET, Swift, ...). The seven contracts every binding has to get right, distilled from the Python binding. Includes a pre-flight checklist. |
| [08: Byte-Fetch Protocol](wiki/08-byte-fetch-protocol.md) | The probe-and-fetch protocol for variable-length payloads (item data, sample data, Exif/XMP) across every layer: raw `HEIF::Reader`, HEIFPP, the C facade, JNI, and language bindings. |
| [09: Asyncify & Streaming](wiki/09-asyncify-and-streaming.md) | When wasm-hosted bindings need Emscripten Asyncify to suspend stream callbacks for `await fetch()`, the build flags, the stack-budget trade-off, and a worked range-request streaming reader. |
| [10: ABI Policy](wiki/10-abi-policy.md) | What the library promises across versions: SOVERSION, semantic versioning, compatibility guarantees, deprecation policy. |
| [11: MP4 / ISOBMFF Support](wiki/11-mp4-support.md) | What MP4 functionality the library provides and does not, codec and brand coverage, fragmented-MP4 reader, FAQ. |

---

<br>

## News

- **v4.0.0**: VVC (H.266) support, C facade, Python and JavaScript/WASM bindings, hardened build system. See [`CHANGELOG.md`](CHANGELOG.md).
- **v3.7.1** (11.03.2025): Bug fixes. Fixed missing limits include.
- **v3.6.0** (25.06.2020): Added support for several new properties, image items with decoding dependencies, Extended type box and Type combination box, associating properties for entity groups, and reader support for segmented tracks.
- **v3.5.0** (10.07.2019): Improved support for JPEGs and edit lists. Bug fixes and code cleanup.
- **v3.4.0** (06.11.2018): Java API improvements. Bug fixes, code cleanup and robustness improvements.
- **v3.3.0** (13.06.2018): Java desktop build added, Java API includes track and image sequence as well as entity grouping support.
- **v3.2** (06.04.2018): Two new APIs added: a Java API (JNI) and a convenience C++ API (HEIFPP).
- **v3.1** (14.03.2018): Reader API update, bug fixes and examples update.
- **v3.0** (05.03.2018): Writer executable replaced with writer library / API. Based on [ISO/IEC 23008-12:2017](http://standards.iso.org/ittf/PubliclyAvailableStandards/c066067_ISO_IEC_23008-12_2017.zip).
- **2nd edition** (09.03.2017): Multi-layered image support, AVC support added.
- **Update** (24.02.2016): Specification changes and various fixes. Backwards compatibility not maintained.

HEIF conformance test candidate files: **[heif_conformance](https://github.com/nokiatech/heif_conformance)**

---

<br>

## License

Released under the **[Nokia HEIF License v2.1](LICENSE.TXT)**.

### What's Allowed by Default

The license grants a royalty-free, worldwide, irrevocable right to use, run, modify, and copy the software within the **Licensed Field**, which the license defines as:

> *"non-commercial purposes of evaluation, testing and academic research"*

In practice that covers:

- Building proofs of concept and prototypes.
- Internal R&D evaluations of HEIF as a format choice.
- Academic research, papers, and student projects.
- Reverse-engineering and interoperability testing.
- Any use that does not, directly or indirectly, generate revenue.

### What's *Not* Covered

The default license does **not** grant rights for:

- Shipping the software (or binaries derived from it) as part of a product or service that is sold, licensed, or otherwise monetised.
- Distributing the software as part of an SDK or platform that third parties pay for.
- Use inside a commercial pipeline that produces revenue-generating HEIF content at scale.

In addition, **codec patents are explicitly excluded**: the H.264 / H.265 (HEVC) / H.266 (VVC) / etc. patents that are necessarily exercised by any real HEIF encoder or decoder are **not** granted by this license and must be licensed separately from their respective patent holders or pools (for example MPEG LA, Access Advance, Via LA).

### Commercial Licensing

Nokia is open to discussing commercial licensing arrangements for product integrations. If your use case falls outside the Licensed Field, or if you would like to explore a paid licence covering both the software and any required codec patent rights, contact:

**<heif@nokia.com>**

Reaching out early, before significant integration work, is the recommended path. The internal C++ APIs change between releases; an early conversation lets Nokia and your team align on a stable contract and commercial terms before infrastructure is built on a moving foundation.

### Other Licensing Notes

All example media files (`*.heic`, `*.png`, `*.jpg`, `*.gif`) in this repository are under copyright © Nokia Technologies 2015-2026.

Third-party components bundled with this repository (for example the iOS toolchain CMake script) are subject to their own licenses; see [LICENSE.TXT](LICENSE.TXT).

For general questions about the source code, file an issue. For licensing questions, contact <heif@nokia.com>.

Copyright © 2015-2026 Nokia Technologies Ltd. All rights reserved.

---

<sub>Parts of this repository were produced with the assistance of AI tooling, with human review.</sub>
