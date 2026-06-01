# 02 - Linking Your Project

## What this page covers

How to build a **brand-new application** from scratch that uses the
HEIF library. This is the missing-manual page that most C++ libraries
assume you already know.

By the end of this page you will have:

1. Installed the HEIF library to a chosen location
2. Created your own CMake project in a separate directory
3. Linked against the library using `find_package(heif)`
4. Compiled and run a small program that calls into the library

If you've never linked a third-party C++ library into your own CMake
project before, **read this page in full**. The steps are short, but
each one corresponds to a concept that breaks builds when skipped.

> **[!] Check the licensing path before you build a real consumer project**
>
> The walkthrough below produces a working application that links
> against this library. That's fully fine for a *proof of concept*,
> *internal evaluation*, or *academic project* - those are inside the
> default Nokia HEIF License v2.1's Licensed Field.
>
> If your "consumer project" is heading toward a shipping product, a
> paid SDK, or any monetised use of HEIF, the default license does
> **not** cover that path, and codec patent rights (H.264 / H.265 /
> H.266) are explicitly *not* granted either. Contact <heif@nokia.com>
> early to discuss commercial terms - doing it before you've built
> infrastructure on top of the API saves rework. See
> [README § License](../README.md#license) for the breakdown.

---

## What you need before starting

- A working build of the HEIF library - see
  **[01 - Getting Started](01-getting-started.md)**.
- The same toolchain (compiler + CMake) you used for that build.
  Mixing GCC- and Clang-built binaries on the same project is
  possible but error-prone for beginners.

---

## Concept primer: what is "installing" a library?

When you ran `cmake --build .` you created build artefacts inside
`build/`. Those artefacts know about each other because CMake wired
them up internally - but **other projects on your machine cannot use
them** unless you copy them to a stable, well-known layout.

That copy step is called *installation*. It puts:

- the compiled libraries (`.a`, `.so`, `.dll`, `.lib`) under `<prefix>/lib/`
- the public headers under `<prefix>/include/heif/`
- a small set of CMake files (`heifConfig.cmake`,
  `heifTargets.cmake`, `heifConfigVersion.cmake`) under
  `<prefix>/lib/cmake/heif/`

…where `<prefix>` is whatever directory you choose. On Linux/macOS
the system-wide convention is `/usr/local`; for development it's
common to use a per-project tmpdir to avoid polluting the system.

The CMake files are the magic ingredient. They let your downstream
project say `find_package(heif REQUIRED)` and have CMake automatically
find the right headers, libraries, link flags, transitive
dependencies, and version compatibility - without you typing any
hard-coded paths.

---

## Walkthrough

### Step 1: Install the library

From inside your existing build directory (the one from page 01):

```bash
# Replace /tmp/heif-install with anywhere convenient.
# /usr/local works too, but needs sudo on most systems.
cmake --install . --prefix /tmp/heif-install
```

You should see ~60 lines of `-- Installing: /tmp/heif-install/...`
messages. Verify the layout:

```bash
ls /tmp/heif-install
#   include/  lib/

ls /tmp/heif-install/include/heif
#   OutputStreamInterface.h   heifexport.h         heifstreaminterface.h
#   heifallocator.h           heifid.h             heifwriter.h
#   heifcommondatatypes.h     heifpp/              heifwriterdatatypes.h
#   heiferrorinfo.h           heifreader.h
#                             heifreaderdatatypes.h

ls /tmp/heif-install/lib/cmake/heif
#   heifConfig.cmake   heifConfigVersion.cmake   heifTargets.cmake   heifTargets-debug.cmake
```

If any of these are missing, your build was incomplete - return to
page 01 and re-run `cmake --build .`.

### Step 2: Create your project directory

Pick a location anywhere - it does **not** need to be inside the
HEIF source tree:

```bash
mkdir ~/my-heif-app
cd ~/my-heif-app
```

### Step 3: Write your application code

```cpp
// ~/my-heif-app/main.cpp
#include <cstdio>

// Raw C++ reader API (layer 1 - see 00-architecture-overview)
#include "heiferrorinfo.h"
#include "heifreader.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <file.heic>\n", argv[0]);
        return 1;
    }

    // Recommended for new code: install the library's reference error sink
    // so every diagnostic prints automatically. (See page 03 for details.)
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);

    auto* reader = HEIF::Reader::Create();
    HEIF::ErrorCode rc = reader->initialize(argv[1]);
    if (rc != HEIF::ErrorCode::OK) {
        // The sink already printed the verbose diagnostic. We just exit.
        HEIF::Reader::Destroy(reader);
        return 2;
    }

    HEIF::FileInformation info;
    reader->getFileInformation(info);
    std::printf("OK: file has %zu items\n",
                static_cast<std::size_t>(info.rootMetaBoxInformation.itemInformations.size));

    HEIF::Reader::Destroy(reader);
    return 0;
}
```

### Step 4: Write the project's CMakeLists.txt

```cmake
# ~/my-heif-app/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(my_heif_app CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Look up the installed library. find_package walks CMAKE_PREFIX_PATH
# (and a few system locations) looking for heifConfig.cmake.
find_package(heif REQUIRED)

add_executable(my_heif_app main.cpp)

# That's it for linking. The exported target carries:
#   - the include directories       (no manual -I flags)
#   - the library file path         (no manual -L or -l flags)
#   - any transitive dependencies   (e.g. -lpthread, -latomic)
#   - the right preprocessor defs   (e.g. -DHEIF_USE_STATIC_LIB)
target_link_libraries(my_heif_app PRIVATE heif::heif)
```

A few notes:

- `heif::heif` is the **umbrella target**. It pulls in the reader,
  the writer, and the HEIFPP convenience layer in one declaration.
  When both are installed it prefers the shared variants; on a
  static-only install it transparently falls back to the static
  ones. For first-touch projects this is the right choice.
- The `heif::` prefix is a *namespace* - a CMake convention that
  prevents collisions with same-named targets from other libraries.
- If you want finer control (e.g. shipping a single static
  executable, or linking only the reader for size reasons), the
  granular targets are still exported and you can use them
  directly: `heif::heif_static`, `heif::heif_shared`,
  `heif::heif_writer_static`, `heif::heif_writer_shared`,
  `heif::heifpp`. See [05 - CMake Options &
  Targets](05-cmake-options-and-targets.md) for the full menu.

### Step 5: Configure your project

You have to tell CMake where the library is installed. The cleanest
way is `CMAKE_PREFIX_PATH`:

```bash
cd ~/my-heif-app
cmake -S . -B build -DCMAKE_PREFIX_PATH=/tmp/heif-install
```

Look for a line like:

```
-- Found heif: /tmp/heif-install/lib/cmake/heif/heifConfig.cmake (found version "4.0.0")
```

If it says `Could not find a package configuration file provided by
"heif"`, your `CMAKE_PREFIX_PATH` is wrong - double-check the path.

### Step 6: Build and run

```bash
cmake --build .

# Try it on something that doesn't exist:
./my_heif_app /tmp/nonexistent.heic

# Expected output (the [heif] line is from the installed sink):
# [heif] [Reader_FileOpen@.../heifreaderimpl.cpp:116] FILE_OPEN_ERROR(8): HeifReaderImpl::initialize: stream is not good (file missing, permission denied, or stream broken)

# Try it on a real file:
./my_heif_app some_real_file.heic
# OK: file has 1 items
```

You now have a complete consumer project. Total: one `.cpp`, one
`CMakeLists.txt`, no third-party Find-modules, no manual paths.

---

## Variations

### Pinning a minimum library version

```cmake
find_package(heif 4.0.0 REQUIRED)
```

CMake will fail at configure time if it finds an older install. The
library uses `SameMajorVersion` compatibility - a 4.x install
satisfies a 4.y request as long as `y >= x`.

### Compile-time version checks

```cpp
#include "buildinfo.hpp"

#if HEIF_VERSION_AT_LEAST(3, 8, 0)
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);
#endif
```

Useful if your codebase has to compile against multiple HEIF
versions on different platforms.

### Using the C++ wrapper instead of the C ABI

```cpp
// main.cpp - using HEIFPP
#include "Heif.h"

HEIFPP::Heif heif;
auto* reader = heif.load("file.heic");
// ... RAII-managed, throws HEIFPP::Exception on failure
```

```cmake
# CMakeLists.txt - link the C++ wrapper target
target_link_libraries(my_heif_app PRIVATE heif::heifpp)
```

The HEIFPP headers are installed under `<prefix>/include/heif/heifpp/`
and `heif::heifpp` puts that directory directly on the include path,
so the include resolves as `"Heif.h"` (no prefix). If you link the
umbrella `heif::heif` target instead, both `<prefix>/include/heif`
and the HEIFPP sub-directory are on the path, so either `"Heif.h"`
or `"heifpp/Heif.h"` works - in-tree code (e.g.
`srcs/examples/heifpp_example.cpp`) uses the unprefixed form.

### Static vs shared linking

| Want… | Link… |
|---|---|
| One umbrella declaration, library decides shared vs static | `heif::heif` |
| Single-file executable, no runtime dependency on libheif | `heif::heif_static` |
| Multiple executables sharing one copy of the library | `heif::heif_shared` |
| Writer functionality (raw C++ writer; no C facade in v4.0.0) | add `heif::heif_writer_static` (or `_shared`) |
| The HEIFPP C++ convenience layer | add `heif::heifpp` |

You can mix: link `heif::heifpp` *and* `heif::heif_writer_static` to
get HEIFPP reader convenience plus the raw writer.

---

## Common problems

| Symptom | Likely cause | Fix |
|---|---|---|
| `Could not find a package configuration file provided by "heif"` | `CMAKE_PREFIX_PATH` not set or wrong | Verify `/tmp/heif-install/lib/cmake/heif/heifConfig.cmake` exists |
| `heif::heif: target not found` | Linked the wrong target name (e.g. `heif` or `libheif`) | Use the full `heif::heif` form (note the namespace `::` separator) |
| `heif::heif umbrella target cannot be assembled` | Install tree has only a partial reader+writer pair | Re-install with both reader and writer enabled, or link the granular targets directly |
| Linker errors about missing HEIFPP symbols | Your code uses HEIFPP but you linked only `heif::heif_static` | Switch to `heif::heif` (umbrella) or add `heif::heifpp` to `target_link_libraries` |
| `fatal error: heiferrorinfo.h: No such file` | Wrong include - should be `<heiferrorinfo.h>` not `<heif/heiferrorinfo.h>` | The `find_package` adds `<prefix>/include/heif` to the include path, so headers resolve as if they were at the root |
| Program runs but logs nothing on errors | You forgot to install the error sink | Call `HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr)` once at startup, or roll your own - see page 03 |
| Works on dev machine, fails on user machine with `cannot open shared object file: libheif_shared.so` | You used the shared library and the user doesn't have it installed | Either ship the `.so` alongside your binary, set `RPATH`, or switch to the `_static` target |

---

## Where to go next

- **[03 - Error Handling](03-error-handling.md)** - full coverage of
  the `setErrorSink` line you just added.
- **[05 - CMake Options & Targets](05-cmake-options-and-targets.md)**, 
  full reference of every exported target.
- **[Examples index](README.md#examples-by-tier)** - every example
  shipped in-tree, annotated by tier and audience.
- **`srcs/examples/heifpp_example.cpp`** - worked examples on the
  HEIFPP API (load + iterate, primary + properties, grid walk,
  write, grid write, error sink). Recommended starting point if you
  prefer the HEIFPP layer (which `heif::heif` includes by default).
- **`srcs/examples/example.cpp`** - worked examples on the raw
  `HEIF::Reader` / `HEIF::Writer` API, including the structured
  error model in the final demo. Use this as a reference when you
  want the minimum-surface C++ API directly.
