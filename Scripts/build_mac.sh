#!/usr/bin/env bash
set -e

echo "============================================"
echo "  TB-303 Clone - macOS Build Script"
echo "============================================"
echo ""

# ── Check dependencies ────────────────────────────────────────────────────────
check_dep() {
    if ! command -v "$1" &>/dev/null; then
        echo "[ERROR] '$1' not found."
        echo "$2"
        exit 1
    fi
}

check_dep cmake "Install CMake: brew install cmake  (or https://cmake.org/download/)"
check_dep git   "Install Xcode Command Line Tools: xcode-select --install"

echo "[OK] CMake and Git found."
echo ""

# ── Move to project root ──────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

# ── Configure ─────────────────────────────────────────────────────────────────
echo "[1/3] Configuring project (downloads JUCE on first run)..."
mkdir -p build_mac

cmake -S . -B build_mac \
      -G Xcode \
      -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
      -DCMAKE_BUILD_TYPE=Release

echo ""
echo "[2/3] Building Universal Binary (Intel + Apple Silicon)..."
cmake --build build_mac --config Release --parallel

echo ""
echo "[3/3] Build complete!"
echo ""
echo "┌─────────────────────────────────────────────────────────────────┐"
echo "│  Plugin locations:                                               │"
echo "│                                                                  │"
echo "│  VST3: build_mac/TB303Clone_artefacts/Release/VST3/            │"
echo "│  AU:   build_mac/TB303Clone_artefacts/Release/AU/              │"
echo "│                                                                  │"
echo "│  Install VST3 to: ~/Library/Audio/Plug-Ins/VST3/               │"
echo "│  Install AU  to: ~/Library/Audio/Plug-Ins/Components/          │"
echo "└─────────────────────────────────────────────────────────────────┘"
echo ""
