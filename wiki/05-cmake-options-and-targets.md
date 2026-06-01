# 05 - CMake Options & Exported Targets Reference

## What this page covers

A complete lookup of every CMake option, every exported library
target, every installed header, and every install-tree file. This
is **reference material**, not a tutorial - use it to look things
up, not to learn.

For a guided tour of how to *use* these targets, see
**[02 - Linking Your Project](02-linking-your-project.md)**.

---

## Build-time CMake options

Pass with `-D<NAME>=<value>` at configure time:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

| Option | Default | Effect |
|---|---:|---|
| `CMAKE_BUILD_TYPE` | `Release` | One of `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`. |
| `BUILD_ONLY_STATIC_LIB` | `OFF` | Don't build the `_shared` variants (`heif_shared`, `heif_writer_shared`). Useful for embedded targets where you don't want `.so`/`.dll` artefacts. |
| `HEIF_PYTHON_BINDING_ONLY` | `OFF` | Trim the build for Python binding wheel/sdist packaging. Skips CLI tools and the system-install rules. |

### Standard CMake options that matter

| Option | Notes |
|---|---|
| `CMAKE_CXX_STANDARD` | Forced to `17` internally; you generally don't need to set it. |
| `CMAKE_INSTALL_PREFIX` | Where `cmake --install` puts artefacts. Default `/usr/local`. |
| `CMAKE_PREFIX_PATH` | Where `find_package(heif)` looks for the install. Set this in your *consumer* project, not in this one. |
| `BUILD_SHARED_LIBS` | **Ignored.** This project decides static vs shared per-target via `BUILD_ONLY_STATIC_LIB`. |

---

## Exported library targets

After `find_package(heif REQUIRED)` succeeds in a consumer project,
these targets are available for `target_link_libraries`:

| Target | Type | Contents | When to use |
|---|---|---|---|
| `heif::heif` | interface (umbrella) | Reader + writer + HEIFPP in one target; prefers shared if installed, falls back to static | First-touch consumers, default for HEIFPP-friendly C++ apps |
| `heif::heif_static` | static | Reader (raw C++ `heifreader.h` + C ABI `heifreader_c.h`) + common code | Embedded C/C++ apps that only read HEIF and want a single-archive link |
| `heif::heif_shared` | shared | Same as `heif::heif_static`, as a `.so` / `.dylib` / `.dll` | FFI consumers (Python, JS/wasm, ...) and runtime-shared deployments |
| `heif::heif_writer_static` | static | Raw C++ writer (`heifwriter.h`) + common code | C++ apps that write HEIF (no writer C facade in v4.0.0) |
| `heif::heif_writer_shared` | shared | Same, shared variant | Shared-library writer link |
| `heif::heifpp` | static | HEIFPP convenience layer over both reader and writer | C++ apps wanting RAII, typed property accessors, the parent-owned object graph |

Notes:

- The `heif::heif` umbrella target is the right default. It pulls
  in reader, writer, and HEIFPP in one declaration. Prefer shared
  variants if both are installed, falls back to static otherwise.
  See `srcs/cmake/heifConfig.cmake.in` for the exact selection
  policy.
- `heif::heifpp` is a single static archive that *uses* the reader
  and writer headers but does not bundle their object code - you
  still need to link a reader and/or writer target alongside it.
  The umbrella handles that wiring for you.
- The `_shared` targets are only present if the project was built
  *without* `BUILD_ONLY_STATIC_LIB=ON`. If your consumer needs
  them, make sure your library install was built with shared libs
  enabled.
- Java/JNI artefacts (`heifjni`) are **not** exported via
  `find_package` - they're consumed via the JAR/AAR mechanism
  instead. See **[04 - Java & Android](04-java-and-android.md)**.

### What linking a target gives you for free

Each exported target carries:

- **`INTERFACE_INCLUDE_DIRECTORIES`** - the right `<prefix>/include/...`
  paths, so `#include "heiferrorinfo.h"` etc. resolve without
  manual `-I` flags.
- **`INTERFACE_LINK_LIBRARIES`** - transitive dependencies (e.g.
  `pthread` on Linux, `atomic` on 32-bit platforms).
- **`INTERFACE_COMPILE_FEATURES`** - `cxx_std_17` so your consumer
  inherits the C++17 requirement.
- **Position-independent code flag** when needed, so static libs
  can be linked into shared libs without re-build.

---

## Installed file layout

After `cmake --install <build> --prefix <prefix>`:

```
<prefix>/
├── include/
│   ├── heif/
│   │   ├── OutputStreamInterface.h
│   │   ├── heifallocator.h
│   │   ├── heifcommondatatypes.h
│   │   ├── heif_error_c.h            ← C ABI error slot + sink + version helpers
│   │   ├── heiferrorinfo.h           ← raw C++ error slot + sink + formatter
│   │   ├── heifexport.h              ← HEIF_DLL_PUBLIC etc.
│   │   ├── heifid.h
│   │   ├── heifreader.h              ← raw C++ Reader API
│   │   ├── heifreader_c.h            ← C ABI reader facade (SOVERSION-stable)
│   │   ├── heifreaderdatatypes.h
│   │   ├── heifstreaminterface.h
│   │   ├── heif_version_c.h          ← C ABI runtime version helpers
│   │   ├── heifwriter.h              ← raw C++ Writer API
│   │   ├── heifwriterdatatypes.h
│   │   ├── buildinfo.hpp             ← compile-time version macros
│   │   └── heifpp/                   ← HEIFPP convenience headers
│   │       ├── ErrorContext.h        ← HEIFPP slot + sink + formatter
│   │       ├── ErrorCodes.h
│   │       ├── Heif.h
│   │       ├── Reader.h
│   │       ├── Writer.h
│   │       └── ...
├── lib/
│   ├── libheif_static.a
│   ├── libheif_writer_static.a
│   ├── libheifpp.a
│   ├── libheif_shared.so.4         ← SONAME-versioned (Linux); only if BUILD_ONLY_STATIC_LIB=OFF
│   ├── libheif_shared.so           ← unversioned symlink (dev)
│   ├── libheif_writer_shared.so.4
│   ├── libheif_writer_shared.so
│   ├── cmake/
│   │   └── heif/
│   │       ├── heifConfig.cmake          ← what find_package(heif) finds
│   │       ├── heifConfigVersion.cmake   ← version + compatibility check
│   │       ├── heifTargets.cmake         ← target definitions
│   │       └── heifTargets-debug.cmake   ← per-config (also -release etc.)
│   └── pkgconfig/
│       └── heif.pc                   ← pkg-config descriptor (Cflags / Libs / Version)
```

Filenames vary slightly per platform:

| Platform | Static suffix | Shared suffix |
|---|---|---|
| Linux | `.a` | `.so` |
| macOS | `.a` | `.dylib` |
| Windows (MSVC) | `.lib` | `.dll` (+ `.lib` import) |
| iOS device | `_iphoneos.a` | `_iphoneos.dylib` |
| iOS simulator | `_iphonesimulator.a` | `_iphonesimulator.dylib` |

---

## `find_package(heif)` consumer recipes

### Minimum viable (umbrella)

```cmake
find_package(heif REQUIRED)
target_link_libraries(my_target PRIVATE heif::heif)
```

This is the recommended default - reader, writer, and HEIFPP are
all pulled in by one declaration, and shared variants are preferred
when available.

### Reader-only, single static archive

```cmake
find_package(heif REQUIRED)
target_link_libraries(my_target PRIVATE heif::heif_static)
```

### Pin a minimum version

```cmake
find_package(heif 4.0.0 REQUIRED)
```

`heifConfigVersion.cmake` is generated with `SameMajorVersion`
compatibility - a 4.x install satisfies a 4.y request as long as
`y >= x`. A 5.x install does **not** satisfy a 4.y request.

### Reader + Writer + C++ wrapper

```cmake
find_package(heif REQUIRED)
target_link_libraries(my_target PRIVATE
    heif::heifpp                # C++ wrapper (pulls in static reader+writer)
)
```

If you want C ABI + C++ wrapper side by side:

```cmake
target_link_libraries(my_target PRIVATE
    heif::heifpp
    heif::heif_writer_static    # explicit writer C ABI access
)
```

### Shared linkage at runtime

```cmake
target_link_libraries(my_target PRIVATE heif::heif_shared)
```

You'll need to ship `libheif_shared.so` (or equivalent) alongside
your binary, or set `RPATH` / `LD_LIBRARY_PATH` so the dynamic
loader can find it.

### Pkg-config

A `heif.pc` descriptor is installed under
`<prefix>/lib/pkgconfig/heif.pc`. CMake is still the recommended
integration path because it carries transitive targets and
respects the umbrella selection policy, but pkg-config works for
plain `Makefile` / autotools consumers:

```bash
export PKG_CONFIG_PATH=/tmp/heif-install/lib/pkgconfig
cc myapp.c $(pkg-config --cflags --libs heif)
```

---

## Install components

The install rules split into three components so packagers can
produce focused runtime / dev / JNI packages:

| Component | What goes in |
|---|---|
| `runtime` | Shared libraries (`libheif_shared.so.4`, `libheif_writer_shared.so.4`) and any installed CLI tools |
| `dev` | Static archives (`libheif*_static.a`, `libheifpp.a`), headers under `include/heif/`, CMake config files, `heif.pc` |
| `jni` | The JNI shared library (`libheifjni.so` / `heifjni.dll`) when built |

Invoke per component:

```bash
cmake --install build --component runtime --prefix /tmp/heif-rt
cmake --install build --component dev     --prefix /tmp/heif-dev
cmake --install build --component jni     --prefix /tmp/heif-jni
```

Omitting `--component` installs everything.

---

## Version macros (in `buildinfo.hpp`)

Compile-time version guards for code that has to compile against
multiple library versions:

```cpp
#include "buildinfo.hpp"

#if HEIF_VERSION_AT_LEAST(4, 0, 0)
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);
#endif
```

| Macro | Type | Meaning |
|---|---|---|
| `HEIF_VERSION_MAJOR` | `int` literal | Major version (e.g. `4`) |
| `HEIF_VERSION_MINOR` | `int` literal | Minor version (e.g. `0`) |
| `HEIF_VERSION_PATCH` | `int` literal | Patch version (e.g. `0`) |
| `HEIF_VERSION_NUMBER` | `int` expression | Encoded as `MAJOR<<16 \| MINOR<<8 \| PATCH` |
| `HEIF_VERSION_AT_LEAST(M,m,p)` | macro | True if the installed library is at least `M.m.p` |

Encoding follows the Vulkan / libcurl convention, so each component
caps at 255 and the comparison is a single integer compare.

For the *runtime* version string (set at build time from `git
describe`), use `BuildInfo::Version` and `BuildInfo::Time`.

---

## Custom CMake targets

Available after configure (run with `cmake --build . --target <name>`):

| Target | Purpose |
|---|---|
| `all` (default) | Everything (libraries + examples) |
| `install` | Install to `CMAKE_INSTALL_PREFIX` |
| `doc` | Build Doxygen HTML under `build/docs/doxygen_html/` (requires Doxygen; the target is only registered when Doxygen is found at configure time) |
| `example` | Just the worked-examples binary |
| `readerapp` | Just the CLI demo binary |

---

## Where to go next

- **[02 - Linking Your Project](02-linking-your-project.md)**, 
  end-to-end walkthrough using these targets.
- **`srcs/cmake/heifConfig.cmake.in`** - the install-time config
  template; useful if you want to understand exactly what
  `find_package(heif)` provides.
