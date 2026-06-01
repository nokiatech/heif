# 10 - ABI Policy

> **Applies to v4.0.0 and forward.** v3.x had no SOVERSION on shared
> libraries and no declared ABI policy.

---

## The short version

* **`libheif_shared.so.4` is the only stable ABI surface.**
  Only the `heif_*` symbols declared in the C facade headers
  (`heif_error_c.h`, `heif_version_c.h`, `heifreader_c.h`) are covered.
* **SOVERSION = project major version.** Today both are `4`.
  A bump to `5` is the only signal that deployed binaries need
  recompilation. Within v4.x, new symbols may appear in a minor
  release but nothing is removed or changed.

---

## What is stable

The C facade headers and **only** the C facade headers:

| Header | Symbols | Stability |
|---|---|---|
| `heif_error_c.h` | `heif_get_last_error_*`, `heif_clear_last_error`, `heif_has_last_error` | Stable across v4.x |
| `heif_version_c.h` | `heif_get_runtime_version`, `heif_get_runtime_version_string`, `heif_get_runtime_version_number` | Stable across v4.x |
| `heifreader_c.h` | `heif_reader_*`, `heif_stream_*` | Stable across v4.x |

A symbol is in scope if it is declared `extern "C"` in one of
these headers **and** exported from the shared library.

## What is NOT stable

| Surface | Why | What to do instead |
|---|---|---|
| HEIFPP (`Heif.h`, `Reader`, etc.) | C++ mangling, vtable layout, exception specs all change across compilers | Bind the C facade, or statically link `libheifpp.a` and accept a private copy |
| `HEIF::Reader` / `HEIF::Writer` (raw C++ API) | Public C++ headers, but no binary-stability promise: vtable layout and `HEIF::Array<T>` / `HEIF::String` shapes may change between minors | Bind the C facade for ABI-stable consumption; static-link the raw C++ when you control the build |
| Java JNI symbols | JNI bridge may add/reorder methods | Pin to the AAR / JAR version |
| Template instantiations leaking from headers | Visibility side-effect, not intentional | Do not `dlsym` anything not in a `_c.h` header |
| CMake target names | Stable within a major, but may change across majors | Use `find_package(heif)` and the `heif::*` targets |

---

## Versioning

The library follows **semantic versioning** scoped to the C facade:

| Component | Bumps when |
|---|---|
| **MAJOR** | Symbol removed, signature changed, struct layout changed, error-code value changed |
| **MINOR** | New entry points added, new error-code values appended |
| **PATCH** | Bug fixes with no ABI surface change |

### SOVERSION

`SOVERSION` equals the major version. Today: **4**.

```
libheif_shared.so        → symlink (dev)
libheif_shared.so.4      → SONAME  (runtime)
libheif_shared.so.4.0.0  → file    (runtime)
```

The runtime linker pins on the SONAME. A binary compiled today
runs against any future 4.x runtime without recompilation.

### Version sources

All derived from the single `project(heif VERSION 4.0.0)` in
`srcs/CMakeLists.txt`:

* Preprocessor: `HEIF_VERSION_MAJOR` / `_MINOR` / `_PATCH` (from `buildinfo.hpp`)
* Runtime (raw C++): `HEIF::Reader::GetVersion()`
* Runtime (C facade): `heif_get_runtime_version{,_string,_number}`
* CMake: `heif_VERSION` from `find_package(heif)`
* pkg-config: `Version:` field in `heif.pc`

---

## `find_package` compatibility

The installed CMake config uses `COMPATIBILITY SameMajorVersion`.

| What you need | What to write |
|---|---|
| Any 4.x | `find_package(heif 4 REQUIRED)` |
| At least 4.6 (uses an entry point added in 4.6) | `find_package(heif 4.6 REQUIRED)` |
| Exactly 4.6.x (reproducible build) | `find_package(heif 4.6 EXACT REQUIRED)` |

`SameMajorVersion` is correct because the ABI is locked across
the entire v4.x minor range - a consumer compiled against 4.6
runs against 4.7. `EXACT` is available for the rare case where
you need a specific minor.

---

## Compatibility guarantees (v4.x)

### Symbols

* **Removed:** never within v4.x.
* **Added:** allowed in any minor.
* **Renamed or signature-changed:** never within v4.x (that bumps major).

### Error codes

* Stable numeric values across v4.x.
* New codes may be appended in minors - always have a `default` arm
  in your `switch`.
* Bindings should branch on the **name string** from
  `heif_get_last_error_code_name()`, not the numeric value.

### Sentinels

* `0` = success (`HEIF_OK`).
* `NULL` from a constructor = allocation failure.
* `*size_out == 0` after a successful call = "the answer is empty".

These are stable across all future major versions.

### Memory ownership

Two patterns exist on the C facade:

| Pattern | Who allocates | Who frees |
|---|---|---|
| **Size-probe `(buf, *size)`** | Caller | Caller (no library-side free needed) |
| **Handle lifecycle** | Library (`heif_reader_create`) | Caller via paired destroy (`heif_reader_destroy`) |

There are no library-allocated byte buffers requiring a paired free
on the reader facade. A reader-only binding needs to wrap exactly
one destructor: `heif_reader_destroy`.

### Threading

Stable across v4.x. The contract may **strengthen** in a minor
(e.g. a single-threaded entry point becoming concurrent-safe)
but never weakens without a major bump.
See [06 - Threading](06-threading.md).

### Behaviour

* Same input → same output within v4.x.
* Same malformed input → same error code within v4.x.
* Diagnostic messages may improve in any release.

---

## Deprecation policy

No symbols are deprecated in v4.0.0. When deprecation becomes
relevant:

1. The C declaration gets `HEIF_DEPRECATED("use heif_reader_X instead")`
   which compiles to `__attribute__((deprecated))` / `__declspec(deprecated)`.
2. **Minimum two minor releases of overlap** before removal in the
   next major.
3. Both deprecation and removal are announced in `CHANGELOG.md`.

---

## CRT / STL notes

The C facade has zero `std::` types in any signature. This is why
a binding is not at the mercy of the embedder's standard library.

### Linux

* Built with `_GLIBCXX_USE_CXX11_ABI=1` (default since GCC 5).
  Consumers using `=0` will see link errors for HEIFPP - the C facade
  is unaffected.
* Mixing `libstdc++` / `libc++` is a HEIFPP concern, not a C facade
  concern.

### macOS

* `libc++` only. Set `CMAKE_OSX_DEPLOYMENT_TARGET` to the oldest
  macOS you need when distributing prebuilt binaries.

### Windows

* Default build: `/MD` (multithreaded DLL CRT). Consumers must also
  use `/MD` (or `/MDd` for debug).
* The C facade is immune to the cross-CRT free hazard because no
  allocation crosses the boundary.
* `/MT` consumers must rebuild the library with `/MT` to match.
* VS 2019 and VS 2022 supported. Older versions are not.

---

## Guidance by audience

### Distributors

* Split into `-runtime` (shared libs) and `-dev` (static libs +
  headers + cmake config + pkg-config) packages, plus an optional
  `-jni` for the Java bridge. The install rules already provide
  matching CMake components (`runtime`, `dev`, `jni`) - see
  [05 - CMake Options & Targets](05-cmake-options-and-targets.md#install-components).
* Pin downstream dependencies to `libheif4`, not `libheif`.
* No need to rebuild consumers on patch updates.

### Binding authors

* Load `libheif_shared.so.4`, not the unversioned symlink.
* Pin your binding's major to the library's SOVERSION.
* Branch on name strings, not numeric error codes.
* See [07 - Writing a Language Binding](07-binding-authors.md).

### Embedders shipping binaries

* Link against `libheif_shared.so.4` - your binary then runs
  against any 4.x runtime.
* If you statically link, you're responsible for re-shipping on fixes.
* If you use HEIFPP, statically link it so you control the compiler match.

---

## See also

* [05 - CMake Options & Targets](05-cmake-options-and-targets.md)
* [07 - Writing a Language Binding](07-binding-authors.md)
* [08 - Byte-Fetch Protocol](08-byte-fetch-protocol.md)
