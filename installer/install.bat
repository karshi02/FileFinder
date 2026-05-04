@echo off
setlocal

set APP_NAME=SmartFinder
set INSTALL_DIR=%ProgramFiles%\SmartFinder
set EXE_NAME=searcher.exe
set BUILD_EXE=build\%EXE_NAME%

echo.
echo ================================================
echo   Smart Finder Installer
echo ================================================
echo.

if not exist "%BUILD_EXE%" (
    echo [ERROR] %BUILD_EXE% not found.
    echo Please run build first.
    pause
    exit /b 1
)

echo [1/4] Creating directories...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%INSTALL_DIR%\data" mkdir "%INSTALL_DIR%\data"

echo [2/4] Copying files...
copy /Y "%BUILD_EXE%" "%INSTALL_DIR%\%EXE_NAME%" >nul

echo [3/4] Creating shortcuts (faster method)...
set "SHORTCUT=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Smart Finder.lnk"
set "DESKTOP_SC=%USERPROFILE%\Desktop\Smart Finder.lnk"


echo [4/4] Configuring startup...
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "SmartFinder" /t REG_SZ /d "\"%INSTALL_DIR%\%EXE_NAME%\"" /f >nul 2>&1

echo.
echo ================================================
echo   Installation Complete!
echo ================================================
echo + mode
echo install
echo.

start "" "%INSTALL_DIR%\%EXE_NAME%"

timeout /t 2 >nul
exit /b 0