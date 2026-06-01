# 01 - Getting Started

## What this page covers

How to clone the repository, build the library, and run a working
demo binary on **your own machine** in roughly five minutes. By the
end you'll have proven that the library compiles in your environment
before you've written a single line of application code.

If you've never used a CMake-based C++ library before, this page is
for you.

> **License scope:** Building and running the library locally for
> evaluation/testing/research is fully covered by the default Nokia
> HEIF License v2.1. If you're heading further - e.g. shipping in a
> commercial product - see the [README § License](../README.md#license)
> first; commercial licensing is a separate conversation with Nokia
> (<heif@nokia.com>).

---

## What you need before starting

- **A C++17 compiler** - GCC 7+, Clang 5+, or MSVC 2017+.
  Check with `g++ --version` (Linux/macOS) or by opening a
  *Developer Command Prompt* (Windows).
- **CMake 3.5 or newer** (3.25+ recommended, required for the
  `CMakePresets.json` workflow shown below). Check with
  `cmake --version`.
- **Git** to clone the repository.
- ~500 MB of free disk space for build artefacts.

You do **not** need: an IDE, a package manager, sudo/admin rights,
the system's HEIC codec, or any HEIF sample files (those are
available separately from the [heif_conformance](https://github.com/nokiatech/heif_conformance) repository).

---

## Walkthrough

### Step 1: Clone the repository

```bash
git clone https://github.com/nokiatech/heif.git
cd heif
```

### Step 2: Configure with CMake

From the repository root, configure into a `build/` directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

If your CMake is 3.25+ you can use the shipped presets instead:

```bash
cmake --preset release      # configures into build-release/
```

Either way you should see ~50 lines ending with:

```
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/heif/build
```

If you see an error, jump to **Common problems** below.

> The repository has both a top-level `CMakeLists.txt` and an inner
> `srcs/CMakeLists.txt`. The top-level one is a thin wrapper that
> does nothing more than `add_subdirectory(srcs)`. Configuring from
> the project root - as shown above - is the supported entry point.

### Step 3: Build

```bash
cmake --build build -j         # plain configure
# or
cmake --build --preset release  # preset workflow
```

On a 4-core laptop this takes 2-5 minutes the first time.

The build is **silent on success**. When it finishes you'll have:

| Artefact | Location |
|---|---|
| Static libraries | `build/lib/libheif_static.a`, `build/lib/libheif_writer_static.a`, `build/lib/libheifpp.a` |
| Shared libraries | `build/lib/libheif_shared.so.4`, `build/lib/libheif_writer_shared.so.4` (Linux; macOS `.4.dylib`; Windows `.dll`) |
| Demo binaries | `build/bin/example`, `build/bin/heifpp_example`, `build/bin/readerapp` |

(Substitute `build-release/` for `build/` when using the preset.)

### Step 4: Run the demo

The `example` binary contains a series of worked demos exercising
the reader and writer APIs (the structured error model is in the
final one). Run it:

```bash
./build/bin/example
```

You will see a lot of `Can't find input file: C003.heic ...`
messages - those examples expect HEIF sample files from the
[heif_conformance](https://github.com/nokiatech/heif_conformance)
repository. What matters is that the
binary *runs*: each "Can't find" message is preceded by a
`[heif] [Reader_FileOpen@...] FILE_OPEN_ERROR(8): ...` line printed
automatically by the library's error sink.

That's it - your build works.


---

## Common problems

| Symptom | Likely cause | Fix |
|---|---|---|
| `cmake: command not found` | CMake not installed | `apt install cmake` (Debian/Ubuntu), `brew install cmake` (macOS), or download from cmake.org (Windows) |
| `Unknown argument: --preset` | CMake older than 3.25 | Use the plain `cmake -S . -B build` form, or upgrade CMake |
| Linker errors about `__atomic_load_8` on 32-bit ARM | Missing libatomic | Add `-latomic` to your link line (or `target_link_libraries(... atomic)`) |
| Build hangs at 90% on Windows | MSVC trying to PCH a huge file in a single core | Pass `-j 1` to force serial build, or use the Ninja generator |

---

## What you've learned

You now have a working build of the library. You haven't *used* it
from your own code yet - that's the next page.

---

## Where to go next

- **[02 - Linking Your Project](02-linking-your-project.md)**, 
  build a brand-new application that links against this library.
- **[05 - CMake Options & Targets](05-cmake-options-and-targets.md)**, 
  every build flag explained (Debug vs Release, static-only, etc.).
- **[03 - Error Handling](03-error-handling.md)** - what those
  `[heif] [Reader_FileOpen@...]` lines are and how to route them
  to your own logger.
