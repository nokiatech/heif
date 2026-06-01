#!/usr/bin/env bash
# ---------------------------------------------------------------------------
#  Nokia HEIF -- one-shot installer for the Python binding
#
#  Run this on a fresh Linux/macOS box to install the binding
#  end-to-end with a single command:
#
#      curl -fsSL 'https://github.com/nokiatech/heif/raw/main/bindings/python/install-python-binding.sh' | bash
#
#  ...or, after cloning the repo:
#
#      ./bindings/python/install-python-binding.sh
#
#  What it does:
#    1. Detects the OS (Debian/Ubuntu, Fedora/RHEL, Arch, macOS).
#    2. Installs system prerequisites (compiler, CMake, Python 3, pip, git).
#       Skips this step if every prerequisite is already present.
#    3. If not running from a clone of the repo, clones it into a
#       temporary directory.
#    4. Runs `python3 -m build --sdist` to produce the source tarball.
#    5. Runs `python3 -m pip install <tarball>` to do the actual install.
#       This drives the existing CMake build via scikit-build-core,
#       producing libheif_shared.so.4 inside the package directory
#       where tier 2 of the binding's resolver finds it.
#    6. Verifies the install with `python3 -c "import heif"`.
#
#  Failure mode: prints a clear error, exits non-zero, never leaves
#  the system in a half-installed state.  No `sudo` is invoked unless
#  step 2 has packages to install -- and the user is prompted then.
#
#  See bindings/python/README.md for the manual recipe this
#  script automates.
# ---------------------------------------------------------------------------

set -euo pipefail

# ---- 0. Pretty output -----------------------------------------------------
NC='\033[0m'
BOLD='\033[1m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'

step() { printf "${BOLD}==>${NC} ${BOLD}%s${NC}\n" "$1"; }
ok()   { printf "${GREEN}\xe2\x9c\x93${NC} %s\n" "$1"; }
warn() { printf "${YELLOW}!${NC} %s\n" "$1"; }
die()  { printf "${RED}\xe2\x9c\x97${NC} %s\n" "$1" >&2; exit 1; }

# ---- 1. Detect OS ---------------------------------------------------------

OS="unknown"
PKG_INSTALL=""
PKG_LIST=""

if [[ "$(uname)" == "Darwin" ]]; then
    OS="macos"
    if ! command -v brew >/dev/null 2>&1; then
        die "Homebrew not found. Install it from https://brew.sh first, then re-run this script."
    fi
    PKG_INSTALL="brew install"
    PKG_LIST="cmake python@3.12"
elif [[ -f /etc/os-release ]]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    case "${ID}${ID_LIKE:-}" in
        *debian*|*ubuntu*)
            OS="debian"
            PKG_INSTALL="sudo apt install -y"
            PKG_LIST="build-essential cmake git python3 python3-pip python3-venv"
            ;;
        *fedora*|*rhel*|*centos*|*rocky*|*almalinux*)
            OS="fedora"
            PKG_INSTALL="sudo dnf install -y"
            PKG_LIST="gcc-c++ cmake git python3 python3-pip"
            ;;
        *arch*)
            OS="arch"
            PKG_INSTALL="sudo pacman -S --noconfirm --needed"
            PKG_LIST="base-devel cmake git python python-pip"
            ;;
        *)
            warn "Unrecognised Linux distribution (ID=${ID}). Skipping system-package step."
            warn "Make sure you have a C++17 compiler, CMake >= 3.13, Python >= 3.9, and pip."
            OS="linux-other"
            ;;
    esac
else
    die "Unsupported OS. This script supports Linux (Debian/Ubuntu/Fedora/RHEL/Arch) and macOS. \
For Windows, see bindings/python/README.md."
fi

step "Detected OS: ${OS}"

# ---- 2. Install system prerequisites if needed ----------------------------

# Quick check: do we already have everything we need?  Skip the package
# install step entirely if so, to avoid an unnecessary sudo prompt.
have_all_prereqs() {
    command -v cmake     >/dev/null 2>&1 || return 1
    command -v git       >/dev/null 2>&1 || return 1
    command -v python3   >/dev/null 2>&1 || return 1
    command -v c++       >/dev/null 2>&1 || command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || return 1
    python3 -m pip --version >/dev/null 2>&1 || return 1
    return 0
}

if have_all_prereqs; then
    ok "All system prerequisites already present (cmake, git, python3, pip, C++ compiler)."
elif [[ -n "${PKG_INSTALL}" ]]; then
    step "Installing system prerequisites: ${PKG_LIST}"
    # shellcheck disable=SC2086
    ${PKG_INSTALL} ${PKG_LIST}
    ok  "System prerequisites installed."
fi

# ---- 3. Locate or clone the source tree -----------------------------------

# If we're already inside a clone of the repo, use it; otherwise clone
# into a tempdir.
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
REPO_ROOT=""

if [[ -f "${SCRIPT_DIR}/../../pyproject.toml" ]] && grep -q "scikit-build-core" "${SCRIPT_DIR}/../../pyproject.toml" 2>/dev/null; then
    REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
    step "Using existing source tree: ${REPO_ROOT}"
else
    REPO_ROOT="$(mktemp -d -t nokia-heif-XXXXXX)"
    step "Cloning source into ${REPO_ROOT}"
    git clone --depth=1 https://github.com/nokiatech/heif.git "${REPO_ROOT}"
    ok  "Source cloned."
fi

# ---- 4. Build the sdist ---------------------------------------------------

step "Installing Python build tooling (build, scikit-build-core)"
python3 -m pip install --user --upgrade build scikit-build-core cffi
ok  "Build tooling installed."

step "Building source distribution (sdist)"
cd "${REPO_ROOT}"
rm -rf dist
python3 -m build --sdist
SDIST_TARBALL="$(ls -1 dist/nokia-heif-*.tar.gz | head -n1)"
[[ -f "${SDIST_TARBALL}" ]] || die "sdist build did not produce dist/nokia-heif-*.tar.gz"
ok  "Built sdist: ${SDIST_TARBALL}"

# ---- 5. Install the sdist (this drives the C++ build) --------------------

step "Installing nokia-heif from sdist (this runs CMake -- ~3 minutes on a 4-core box)"
python3 -m pip install --user "${SDIST_TARBALL}"
ok  "nokia-heif installed."

# ---- 6. Verify ------------------------------------------------------------

step "Verifying install"
python3 -c "import heif; print('nokia-heif version:', heif.__version__); print('  module path:    ', heif.__file__)"
ok  "Install verified end to end."

cat <<EOF

${BOLD}Done.${NC} You can now use the binding from any Python script:

    import heif
    with heif.Reader('photo.heic') as r:
        print(r.master_images[0].width, 'x', r.master_images[0].height)

See bindings/python/README.md for the full API surface.
EOF
