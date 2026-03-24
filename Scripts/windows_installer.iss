; ── Inno Setup script — TB-303 Clone Windows Installer ───────────────────────
; Run this with Inno Setup Compiler AFTER building with build_windows.bat
; Download Inno Setup: https://jrsoftware.org/isinfo.php

#define AppName      "TB303 Clone"
#define AppVersion   "1.0.0"
#define AppPublisher "Daniel"
#define AppURL       ""
#define VST3Source   "..\build_windows\TB303Clone_artefacts\Release\VST3\TB303 Clone.vst3"

[Setup]
AppId={{8F3A2B1C-4D5E-6F7A-8B9C-0D1E2F3A4B5C}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
OutputDir=..\Installer_Output
OutputBaseFilename=TB303Clone_v{#AppVersion}_Windows_Setup
SetupIconFile=
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; VST3 plugin (copies the entire .vst3 bundle folder)
Source: "{#VST3Source}\*"; \
    DestDir: "{commonpf64}\Common Files\VST3\TB303 Clone.vst3"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Run]
; Optional: open the VST3 folder after install
Filename: "{commonpf64}\Common Files\VST3\"; \
    Description: "Open VST3 installation folder"; \
    Flags: postinstall shellexec skipifsilent

[Messages]
FinishedLabel=TB-303 Clone has been installed.%n%nThe VST3 plugin is now in:%n  C:\Program Files\Common Files\VST3\%n%nPlease rescan plugins in Cubase or Ableton Live.
