; ================================================================
;  SmartFinder Setup — NSIS Installer Script
;
;  วิธี build:
;    1. ติดตั้ง NSIS จาก https://nsis.sourceforge.io
;    2. make rebuild           <- build searcher.exe ก่อน
;    3. makensis installer.nsi <- สร้าง SmartFinderSetup.exe
; ================================================================

Unicode true

!define APP_NAME     "Smart Finder"
!define APP_EXE      "searcher.exe"
!define APP_VERSION  "1.0.0"
!define PUBLISHER    "SmartFinder"
!define INSTALL_DIR  "$PROGRAMFILES\SmartFinder"
!define UNINST_KEY   "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartFinder"
!define RUN_KEY      "Software\Microsoft\Windows\CurrentVersion\Run"

!include "MUI2.nsh"
!include "LogicLib.nsh"

Name            "${APP_NAME} ${APP_VERSION}"
OutFile         "SmartFinderSetup.exe"
InstallDir      "${INSTALL_DIR}"
InstallDirRegKey HKLM "${UNINST_KEY}" "InstallLocation"
RequestExecutionLevel admin
SetCompressor   /SOLID lzma
ShowInstDetails show

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; ================================================================
Section "Smart Finder" SEC_MAIN
    SectionIn RO

    ExecWait 'taskkill /F /IM "${APP_EXE}"'

    SetOutPath "$INSTDIR"
    File "build\${APP_EXE}"
    CreateDirectory "$INSTDIR\data"

    ; shortcuts
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" \
                   "$INSTDIR\${APP_EXE}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" \
                   "$INSTDIR\uninstall.exe"
    CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

    ; Run at Startup (default ON)
    WriteRegStr HKCU "${RUN_KEY}" "SmartFinder" '"$INSTDIR\${APP_EXE}"'

    ; settings.ini default — startup=1, Ctrl+Space
    ${IfNot} ${FileExists} "$INSTDIR\data\settings.ini"
        FileOpen  $0 "$INSTDIR\data\settings.ini" w
        FileWrite $0 "mod=2$\r$\n"
        FileWrite $0 "vk=32$\r$\n"
        FileWrite $0 "startup=1$\r$\n"
        FileClose $0
    ${EndIf}

    ; uninstall registry
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayName"     "${APP_NAME}"
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayVersion"  "${APP_VERSION}"
    WriteRegStr   HKLM "${UNINST_KEY}" "Publisher"       "${PUBLISHER}"
    WriteRegStr   HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr   HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair"        1
    WriteUninstaller "$INSTDIR\uninstall.exe"

    Exec '"$INSTDIR\${APP_EXE}"'
SectionEnd

; ================================================================
Section "Uninstall"
    ExecWait 'taskkill /F /IM "${APP_EXE}"'
    DeleteRegValue HKCU "${RUN_KEY}" "SmartFinder"
    DeleteRegKey HKLM "${UNINST_KEY}"
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\uninstall.exe"
    RMDir /r "$INSTDIR\data"
    RMDir "$INSTDIR"
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
    RMDir  "$SMPROGRAMS\${APP_NAME}"
    Delete "$DESKTOP\${APP_NAME}.lnk"
SectionEnd
