# Nokia HEIF: Python Binding

Python binding for the Nokia HEIF reader. Inspects HEIF/HEIC files, enumerates images and tracks, exposes item properties (`colr`, `pixi`, `clap`, `irot`, `imir`, `pasp`, ...), HDR metadata, entity groups, grids, overlays, gainmaps, and DASH segmented parsing, and yields decoder-ready encoded bitstreams.

Distribution name: `nokia-heif`. Import name: `heif`. Reader only.

> The binding **does not decode pixels.** `Image.bitstream()` returns an encoded HEVC / AVC / VVC / JPEG payload; feed it to whichever decoder you already use (`pillow_heif`, `imageio`, PyAV, `vvdec`, GStreamer, ...).

---

## Installation

Three install paths are documented below. Most users want **Quick Install**.

All paths require a working C++17 compiler, CMake, Git, Python 3.9 or newer, and pip on the machine that runs `pip install`. The compile step takes a few minutes the first time. Full OS-by-OS prerequisite commands, virtual-environment guidance, and the end-to-end walkthrough live in the [main README](../../README.md#just-want-to-read-a-heif-file-from-python).

### Quick Install

```bash
python3 -m pip install "git+https://github.com/nokiatech/heif.git@v4.0.0"
```

(On Windows use `python` instead of `python3`.)

Pip fetches the source from GitHub, runs the project's `scikit-build-core` backend (which drives CMake to compile `libheif_shared.so.4`), bundles the compiled library together with the Python wrapper into a wheel, and installs the wheel into the currently active Python environment. After it finishes, `import heif` works in that environment.

### One-Command Installer

For a fresh Linux or macOS machine with **no** prerequisites installed, a helper script automates everything end-to-end: OS detection, system-package install, sdist build, pip install, and verification.

From the repository root:

```bash
bash bindings/python/install-python-binding.sh
```

Or pipe directly from GitHub, no clone needed:

```bash
curl -fsSL https://github.com/nokiatech/heif/raw/v4.0.0/bindings/python/install-python-binding.sh | bash
```

Supported distributions: Debian / Ubuntu, Fedora / RHEL, Arch, and macOS (with Homebrew). Windows is not covered; use Quick Install above after installing the OS prerequisites manually.

### In-Tree Development Install

For Python-side iteration **without rebuilding C++ on every `pip install`**, use the alternate `pyproject.toml` shipped in this directory. It uses pure setuptools, skips the CMake build entirely, and assumes you have already built the shared library separately.

From the repository root:

```bash
# 1. Build the C++ library once (any build type works)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# 2. Register the binding as an editable install in your active Python env
pip install -e bindings/python/

# 3. Verify
python3 -c "import heif; print(heif.__version__)"
```

The library resolver in `heif/_ffi.py` walks up from the binding source to find the in-tree `build*/lib/` directory automatically. With this layout, editing any `.py` file under `bindings/python/heif/` takes effect on the next `import` with no reinstall, while rebuilding the C++ library is a separate `cmake --build build` step.

### Updating

Pip caches git URL installs aggressively. To force a fresh fetch and rebuild against the same tag (or a different one):

```bash
python3 -m pip install --upgrade --force-reinstall --no-cache-dir \
    "git+https://github.com/nokiatech/heif.git@v4.0.0"
```

Pin a specific commit by replacing `@v4.0.0` with `@<sha>`.

---

## Usage

```python
import heif

with heif.Reader("photo.heic") as r:
    print(r.major_brand)                  # FourCC, e.g. "heic", "mif1", "mif3"
    for img in r.master_images:
        print(img.item_type, img.width, img.height)
        payload = img.bitstream()         # encoded bytes, decoder-ready
```

`Image.bitstream()` returns the codec payload with parameter sets prepended and NAL start codes substituted for length prefixes where applicable, ready to hand to an HEVC / AVC / VVC / JPEG decoder.

---

## Error Handling

Every failure raises `heif.HeifError`, a `RuntimeError` subclass populated from the library's structured error slot:

```python
try:
    with heif.Reader("missing.heic") as r:
        ...
except heif.HeifError as e:
    print(e.code_name, e.origin_name, e.file, e.line)
    print(e.message)
    if e.cause:
        print("upstream:", e.cause)
```

The full error model (codes, origins, the diagnostic slot, the process-wide sink hook) is documented in [wiki § 03: Error Handling](../../wiki/03-error-handling.md).

---

## Examples

A fully commented walkthrough of the Reader, Image, Track, and error-handling APIs lives at [`examples/read_heif.py`](examples/read_heif.py). Run it from the repository root against any HEIC file:

```bash
python3 bindings/python/examples/read_heif.py photo.heic
```

The script walks file-level metadata, master images, image properties (rotation, mirror, color profile, HDR metadata), item references, derived items (grids, overlays), entity groups, tracks, and demonstrates a `HeifError` catch.

---

## Troubleshooting

Python-binding-specific failure modes:

- **`import heif` raises `OSError: cannot find libheif_shared`**: the resolver could not locate the shared library. Two fixes: either install the library system-wide with `cmake --install build` so the dynamic loader finds it on its own, or set `HEIF_LIBRARY_PATH` to the absolute path of the library file. The expected filename is `libheif_shared.so.4` on Linux, `libheif_shared.4.dylib` on macOS, `heif_shared.dll` on Windows.

  ```bash
  HEIF_LIBRARY_PATH=/abs/path/to/libheif_shared.so.4 python3 -c "import heif"
  ```

- **`OSError: library is SOVERSION X, binding expects 4`**: the installed shared library is from a different release than the Python binding. Reinstall both from the same source (`pip install --force-reinstall git+...@v4.0.0`).

- **`ModuleNotFoundError: No module named 'cffi'`**: only triggers if `heif/` was placed on `PYTHONPATH` manually without going through pip. Run `pip install "cffi>=1.16"` and retry.

- **Build failures during `pip install`** (compiler missing, CMake not found, Python.h not found): re-check the OS prerequisite list in the [main README](../../README.md#just-want-to-read-a-heif-file-from-python). On Debian/Ubuntu specifically, missing `python3-dev` is the most common cause; install it with `sudo apt install python3-dev`. For general first-time-build issues see [wiki § 01: Getting Started](../../wiki/01-getting-started.md).

---

**Links:** [Main README](../../README.md) · [Wiki](../../wiki/) · [Error Model](../../wiki/03-error-handling.md) · [Issues](https://github.com/nokiatech/heif/issues) · [License](../../LICENSE.TXT)

<sub>Parts of this repository were produced with the assistance of AI tooling, with human review.</sub>
