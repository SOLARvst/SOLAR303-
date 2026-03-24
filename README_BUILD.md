# TB-303 Clone — Build & Install Guide

## What you need (install once)

### Windows
1. **Visual Studio 2022** (Community, free) — https://visualstudio.microsoft.com/
   - During install select: "Desktop development with C++"
2. **CMake** — https://cmake.org/download/ (check "Add to PATH")
3. **Git** — https://git-scm.com/download/win
4. **Inno Setup** (for installer) — https://jrsoftware.org/isinfo.php

### macOS
1. **Xcode** (from App Store) + Command Line Tools:
   ```
   xcode-select --install
   ```
2. **CMake**:
   ```
   brew install cmake
   ```
   (Install Homebrew first from https://brew.sh if you don't have it)

---

## Build steps

### Windows
1. Double-click `Scripts\build_windows.bat`
2. Wait — first run downloads JUCE (~10 min)
3. Plugin will be at:
   `build_windows\TB303Clone_artefacts\Release\VST3\`

**To create an installer:**
1. Open Inno Setup Compiler
2. Open `Scripts\windows_installer.iss`
3. Press **Build → Compile**
4. Installer `.exe` will be in `Installer_Output\`

### macOS
```bash
chmod +x Scripts/build_mac.sh
./Scripts/build_mac.sh
```
First run downloads JUCE (~10 min).

Plugins will be at:
- VST3: `build_mac/TB303Clone_artefacts/Release/VST3/`
- AU:   `build_mac/TB303Clone_artefacts/Release/AU/`

**To create an installer:**
```bash
chmod +x Scripts/mac_installer.sh
./Scripts/mac_installer.sh
```

---

## Install the plugin

### Windows — manual install
Copy `TB303 Clone.vst3` folder to:
```
C:\Program Files\Common Files\VST3\
```
Then in Cubase/Ableton: **rescan VST plugins**.

### macOS — manual install
- VST3 → `~/Library/Audio/Plug-Ins/VST3/`
- AU   → `~/Library/Audio/Plug-Ins/Components/`

---

## Using the plugin

| Parameter | Description |
|-----------|-------------|
| **CUTOFF** | Filter cutoff frequency |
| **RES** | Resonance (self-oscillates at maximum) |
| **ENV MOD** | How much the envelope opens the filter |
| **DECAY** | Envelope decay time (10 ms – 2 s) |
| **ACCENT** | Accent strength (triggered by velocity > ~90) |
| **WAVE** | Sawtooth or Square oscillator |
| **SLIDE** | Portamento time (triggered by overlapping/legato notes) |
| **VOLUME** | Output volume |
| **TUNE** | Transpose in semitones (-24 to +24) |

**Accent**: play a note with velocity above ~90 to trigger accent mode.
**Slide**: play legato (hold one note and press the next) to trigger portamento.
