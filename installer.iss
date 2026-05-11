;==================================================
; SmartFinder Installer - Inno Setup Script
;==================================================

[Setup]
AppId={{SmartFinder-8A2C8D4E-9F3A-4B5E-8C7D-2E9F1A8B7C6D}
AppName=Smart Finder
AppVersion=1.0.1
AppPublisher=SmartFinder
DefaultDirName={pf}\SmartFinder
DefaultGroupName=Smart Finder
UninstallDisplayIcon={app}\searcher.exe
Compression=lzma2
SolidCompression=yes
OutputDir=.
OutputBaseFilename=SmartFinderSetup
PrivilegesRequired=admin
SetupIconFile=icon.ico
Uninstallable=yes
CreateUninstallRegKey=yes
UninstallDisplayName=Smart Finder

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "thai"; MessagesFile: "compiler:Languages\Thai.isl"

[Tasks]
Name: "desktopicon"; Description: "Create desktop shortcut"; GroupDescription: "Additional icons:"
Name: "startup"; Description: "Run at Windows startup"; GroupDescription: "Startup:"

[Files]
Source: "build\searcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Smart Finder"; Filename: "{app}\searcher.exe"
Name: "{group}\Uninstall Smart Finder"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Smart Finder"; Filename: "{app}\searcher.exe"; Tasks: desktopicon
Name: "{userstartup}\Smart Finder"; Filename: "{app}\searcher.exe"; Tasks: startup

[Run]
Filename: "{app}\searcher.exe"; Description: "Launch Smart Finder"; Flags: postinstall nowait skipifsilent

[UninstallDelete]
Type: dirifempty; Name: "{app}"