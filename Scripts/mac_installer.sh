#!/usr/bin/env bash
# ── macOS Installer Script — TB-303 Clone ─────────────────────────────────────
# Run this AFTER build_mac.sh has successfully completed.
# Creates a .pkg installer for distribution.
# Requires: pkgbuild and productbuild (included with Xcode Command Line Tools)

set -e

VERSION="1.0.0"
PRODUCT="TB303 Clone"
IDENTIFIER="com.daniel.tb303clone"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."
BUILD_DIR="$PROJECT_ROOT/build_mac"
ARTEFACTS="$BUILD_DIR/TB303Clone_artefacts/Release"
OUTPUT_DIR="$PROJECT_ROOT/Installer_Output"

mkdir -p "$OUTPUT_DIR"

echo "============================================"
echo "  TB-303 Clone - macOS Installer Creator"
echo "============================================"

# ── Stage VST3 ────────────────────────────────────────────────────────────────
VST3_SRC="$ARTEFACTS/VST3/TB303 Clone.vst3"
VST3_STAGE="$OUTPUT_DIR/stage_vst3/Library/Audio/Plug-Ins/VST3"

echo "[1/4] Staging VST3..."
mkdir -p "$VST3_STAGE"
cp -R "$VST3_SRC" "$VST3_STAGE/"

# ── Stage AU ──────────────────────────────────────────────────────────────────
AU_SRC="$ARTEFACTS/AU/TB303 Clone.component"
AU_STAGE="$OUTPUT_DIR/stage_au/Library/Audio/Plug-Ins/Components"

echo "[2/4] Staging AU..."
mkdir -p "$AU_STAGE"
cp -R "$AU_SRC" "$AU_STAGE/"

# ── Build component packages ──────────────────────────────────────────────────
echo "[3/4] Building packages..."

pkgbuild \
    --root "$OUTPUT_DIR/stage_vst3" \
    --identifier "${IDENTIFIER}.vst3" \
    --version "$VERSION" \
    --install-location "/" \
    "$OUTPUT_DIR/TB303Clone_VST3.pkg"

pkgbuild \
    --root "$OUTPUT_DIR/stage_au" \
    --identifier "${IDENTIFIER}.au" \
    --version "$VERSION" \
    --install-location "/" \
    "$OUTPUT_DIR/TB303Clone_AU.pkg"

# ── Create distribution XML ───────────────────────────────────────────────────
cat > "$OUTPUT_DIR/distribution.xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>TB-303 Clone v${VERSION}</title>
    <background file="background.png" mime-type="image/png" scaling="proportional" alignment="center"/>
    <welcome file="welcome.rtf"/>
    <options customize="always" require-scripts="false" rootVolumeOnly="true"/>
    <choices-outline>
        <line choice="vst3"/>
        <line choice="au"/>
    </choices-outline>
    <choice id="vst3" visible="true" enabled="true" selected="true"
            title="VST3 Plugin"
            description="Installs the VST3 plugin for Cubase, Ableton Live, etc.">
        <pkg-ref id="${IDENTIFIER}.vst3"/>
    </choice>
    <choice id="au" visible="true" enabled="true" selected="true"
            title="Audio Unit (AU) Plugin"
            description="Installs the AU plugin for Logic Pro, GarageBand, etc.">
        <pkg-ref id="${IDENTIFIER}.au"/>
    </choice>
    <pkg-ref id="${IDENTIFIER}.vst3">TB303Clone_VST3.pkg</pkg-ref>
    <pkg-ref id="${IDENTIFIER}.au">TB303Clone_AU.pkg</pkg-ref>
</installer-gui-script>
EOF

# ── Build final installer ─────────────────────────────────────────────────────
echo "[4/4] Creating final installer..."

productbuild \
    --distribution "$OUTPUT_DIR/distribution.xml" \
    --package-path "$OUTPUT_DIR" \
    "$OUTPUT_DIR/TB303Clone_v${VERSION}_macOS_Installer.pkg"

echo ""
echo "✓ Installer created:"
echo "  $OUTPUT_DIR/TB303Clone_v${VERSION}_macOS_Installer.pkg"
echo ""
echo "Distribute this .pkg file to install both VST3 and AU on any Mac."
