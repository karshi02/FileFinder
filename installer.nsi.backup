; ============================================================
;  SmartFinder Installer — NSIS Script
;  build: makensis installer.nsi  →  SmartFinderSetup.exe
; ============================================================

!define APP_NAME     "Smart Finder"
!define APP_EXE      "searcher.exe"
!define APP_VERSION  "1.0.0"
!define INSTALL_DIR  "$PROGRAMFILES\SmartFinder"
!define REG_KEY      "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartFinder"
!define RUN_KEY      "Software\Microsoft\Windows\CurrentVersion\Run"

Name            "${APP_NAME}"
OutFile         "SmartFinderSetup.exe"
InstallDir      "${INSTALL_DIR}"
RequestExecutionLevel admin
SetCompressor   lzma

; ---- ไอคอน installer (ถ้ามี) ----
; Icon "resources\icon.ico"

; ============================================================
;  หน้า installer
; ============================================================
Page directory
Page instfiles

; ============================================================
;  ติดตั้ง
; ============================================================
Section "Main" SEC_MAIN

    SetOutPath "$INSTDIR"

    ; copy ไฟล์หลัก
    File "build\${APP_EXE}"

    ; สร้างโฟลเดอร์ data
    CreateDirectory "$INSTDIR\data"

    ; สร้าง shortcut ใน Start Menu
    CreateDirectory "$SMPROGRAMS\SmartFinder"
    CreateShortCut  "$SMPROGRAMS\SmartFinder\Smart Finder.lnk" \
                    "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0

    ; shortcut บน Desktop (optional)
    CreateShortCut  "$DESKTOP\Smart Finder.lnk" \
                    "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0

    ; Run at Startup — รันพร้อม Windows
    WriteRegStr HKCU "${RUN_KEY}" "SmartFinder" "$INSTDIR\${APP_EXE}"

    ; ข้อมูล uninstall
    WriteRegStr   HKLM "${REG_KEY}" "DisplayName"     "${APP_NAME}"
    WriteRegStr   HKLM "${REG_KEY}" "DisplayVersion"  "${APP_VERSION}"
    WriteRegStr   HKLM "${REG_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr   HKLM "${REG_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr   HKLM "${REG_KEY}" "Publisher"       "SmartFinder"
    WriteRegDWORD HKLM "${REG_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${REG_KEY}" "NoRepair"        1

    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; รันโปรแกรมทันทีหลัง install
    Exec "$INSTDIR\${APP_EXE}"

    MessageBox MB_OK "Smart Finder installed!$\n$\nPress Ctrl+Space anywhere to search."

SectionEnd

; ============================================================
;  ถอนการติดตั้ง
; ============================================================
Section "Uninstall"

    ; หยุดโปรแกรมถ้ากำลังรันอยู่
    ExecWait "taskkill /F /IM ${APP_EXE}"

    ; ลบ startup entry
    DeleteRegValue HKCU "${RUN_KEY}" "SmartFinder"

    ; ลบ registry uninstall key
    DeleteRegKey HKLM "${REG_KEY}"

    ; ลบไฟล์และโฟลเดอร์
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\uninstall.exe"
    RMDir /r "$INSTDIR\data"
    RMDir "$INSTDIR"

    ; ลบ shortcuts
    Delete "$SMPROGRAMS\SmartFinder\Smart Finder.lnk"
    RMDir  "$SMPROGRAMS\SmartFinder"
    Delete "$DESKTOP\Smart Finder.lnk"

    MessageBox MB_OK "Smart Finder has been uninstalled."

SectionEnd
