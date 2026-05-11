@echo off
setlocal enabledelayedexpansion

echo ============================================
echo   SmartFinder Release Builder
echo ============================================
echo.

REM 获取版本号
set /p VERSION="Enter version (e.g., 1.0.1): "
if "%VERSION%"=="" set VERSION=1.0.1

set BUILD_DIR=build
set RELEASE_DIR=release_%VERSION%
set DATE=%date:~10,4%%date:~4,2%%date:~7,2%

echo.
echo Version: %VERSION%
echo Build Date: %DATE%
echo.

REM Step 1: Clean and build
echo [1/5] Cleaning old build...
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

echo [2/5] Compiling...
gcc -O2 -Wall main.c universal_scanner.c universal_ui.c hotkey.c frequent.c settings.c -o %BUILD_DIR%\searcher.exe -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 -ladvapi32 -luser32 -static

if errorlevel 1 (
    echo [ERROR] Compilation failed!
    pause
    exit /b 1
)

echo [3/5] Creating release directory...
if exist %RELEASE_DIR% rmdir /s /q %RELEASE_DIR%
mkdir %RELEASE_DIR%
mkdir %RELEASE_DIR%\data

echo [4/5] Copying files...
copy %BUILD_DIR%\searcher.exe %RELEASE_DIR%\ >nul
copy README.md %RELEASE_DIR%\ >nul 2>nul

REM Step 5: Create installer
echo [5/5] Creating installer...

REM Check for NSIS
if exist "%PROGRAMFILES(x86)%\NSIS\makensis.exe" (
    echo Creating NSIS installer...
    
    REM Create NSIS script
    (
    echo !define APP_NAME "Smart Finder"
    echo !define APP_EXE "searcher.exe"
    echo !define APP_VERSION "%VERSION%"
    echo !define INSTALL_DIR "$PROGRAMFILES\SmartFinder"
    echo.
    echo Name "${APP_NAME}"
    echo OutFile "SmartFinderSetup_%VERSION%.exe"
    echo InstallDir "${INSTALL_DIR}"
    echo RequestExecutionLevel admin
    echo.
    echo Section
    echo     SetOutPath "$INSTDIR"
    echo     File "%RELEASE_DIR%\searcher.exe"
    echo     CreateDirectory "$INSTDIR\data"
    echo     CreateShortCut "$SMPROGRAMS\SmartFinder.lnk" "$INSTDIR\searcher.exe"
    echo     CreateShortCut "$DESKTOP\SmartFinder.lnk" "$INSTDIR\searcher.exe"
    echo     WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "SmartFinder" "$INSTDIR\searcher.exe"
    echo     WriteUninstaller "$INSTDIR\uninstall.exe"
    echo SectionEnd
    echo.
    echo Section "Uninstall"
    echo     ExecWait "taskkill /F /IM searcher.exe"
    echo     Delete "$INSTDIR\searcher.exe"
    echo     Delete "$INSTDIR\uninstall.exe"
    echo     RMDir /r "$INSTDIR\data"
    echo     RMDir "$INSTDIR"
    echo     Delete "$SMPROGRAMS\SmartFinder.lnk"
    echo     Delete "$DESKTOP\SmartFinder.lnk"
    echo     DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "SmartFinder"
    echo SectionEnd
    ) > temp_installer.nsi
    
    "%PROGRAMFILES(x86)%\NSIS\makensis.exe" temp_installer.nsi
    del temp_installer.nsi
    move SmartFinderSetup_%VERSION%.exe %RELEASE_DIR%\ >nul
) else (
    echo NSIS not found, creating ZIP instead...
    powershell Compress-Archive -Path "%RELEASE_DIR%\*" -DestinationPath "SmartFinder_%VERSION%.zip" -Force
    move SmartFinder_%VERSION%.zip %RELEASE_DIR%\ >nul
)

echo.
echo ============================================
echo RELEASE READY!
echo ============================================
echo Output: %RELEASE_DIR%\
echo.
echo Files:
dir /b %RELEASE_DIR%\
echo.
echo To create setup.exe, install NSIS first:
echo https://nsis.sourceforge.io/Download
echo.

pause