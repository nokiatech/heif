#!/usr/bin/env bash
# Build the HEIF JS/Wasm binding using Emscripten.
#
# Usage:
#   bash build-with-emcmake.sh                    # build only, stage to dist/
#   bash build-with-emcmake.sh --prefix <DIR>     # build + install to <DIR>
#   HEIF_ROOT=/path/to/heif bash build-with-emcmake.sh
#
# Prerequisites: Emscripten SDK activated (`source emsdk_env.sh`),
#                CMake >= 3.12.
#
# This script lives at <repo>/bindings/javascript-wasm/ and expects the
# HEIF library source tree two levels up (the repository root).
# Override by setting HEIF_ROOT before invoking.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HEIF_ROOT="${HEIF_ROOT:-$(cd "$SCRIPT_DIR/../.." && pwd)}"

# ---- Parse arguments ------------------------------------------------------
INSTALL_PREFIX=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)
            INSTALL_PREFIX="${2:-}"
            if [[ -z "$INSTALL_PREFIX" ]]; then
                echo "error: --prefix requires a directory argument" >&2
                exit 1
            fi
            shift 2
            ;;
        --prefix=*)
            INSTALL_PREFIX="${1#--prefix=}"
            shift
            ;;
        -h|--help)
            sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "error: unknown argument '$1' (try --help)" >&2
            exit 1
            ;;
    esac
done

# ---- Configure + build ----------------------------------------------------
echo "==> HEIF library root: $HEIF_ROOT"
echo "==> Binding source   : $SCRIPT_DIR"

mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"

echo "==> Configuring (emcmake)..."
emcmake cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DEMSCRIPTEN=true \
    -DEmscriptenBuild=ON \
    -DHEIF_ROOT="$HEIF_ROOT" \
    "$SCRIPT_DIR" > configure.log 2>&1 || {
        echo "error: configure failed; see $SCRIPT_DIR/build/configure.log" >&2
        tail -20 configure.log >&2
        exit 1
    }

echo "==> Compiling (this takes a few minutes on first run)..."
cmake --build . > build.log 2>&1 || {
    echo "error: build failed; see $SCRIPT_DIR/build/build.log" >&2
    tail -30 build.log >&2
    exit 1
}

# ---- Optional install -----------------------------------------------------
if [[ -n "$INSTALL_PREFIX" ]]; then
    echo "==> Installing to $INSTALL_PREFIX ..."
    cmake --install . --prefix "$INSTALL_PREFIX" > install.log 2>&1 || {
        echo "error: install failed; see $SCRIPT_DIR/build/install.log" >&2
        tail -20 install.log >&2
        exit 1
    }
fi

# ---- Summary --------------------------------------------------------------
echo
echo "=========================================================="
echo "Build complete."
echo
echo "Deployable bundle (auto-staged):"
echo "  $SCRIPT_DIR/dist/"
ls -lh "$SCRIPT_DIR/dist/" 2>/dev/null | awk 'NR>1 {printf "    %-18s %s\n", $9, $5}'
if [[ -n "$INSTALL_PREFIX" ]]; then
    echo
    echo "Installed to:"
    echo "  $INSTALL_PREFIX/"
    ls -lh "$INSTALL_PREFIX/" 2>/dev/null | awk 'NR>1 {printf "    %-18s %s\n", $9, $5}'
fi
echo
echo "Next: copy dist/* into your web project, or use --prefix to install"
echo "elsewhere. Load order in HTML: heif-raw.js before heif.js."
echo "=========================================================="
