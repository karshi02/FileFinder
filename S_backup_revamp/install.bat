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

echo [1/5] Creating install directory...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%INSTALL_DIR%\data" mkdir "%INSTALL_DIR%\data"

echo [2/5] Copying files...
copy /Y "%BUILD_EXE%" "%INSTALL_DIR%\%EXE_NAME%" >nul

echo [3/5] Creating Start Menu shortcut...
set "SHORTCUT=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Smart Finder.lnk"
powershell -NoProfile -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%SHORTCUT%'); $sc.TargetPath = '%INSTALL_DIR%\%EXE_NAME%'; $sc.WorkingDirectory = '%INSTALL_DIR%'; $sc.Save()"

echo [4/5] Creating Desktop shortcut...
set "DESKTOP_SC=%USERPROFILE%\Desktop\Smart Finder.lnk"
powershell -NoProfile -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%DESKTOP_SC%'); $sc.TargetPath = '%INSTALL_DIR%\%EXE_NAME%'; $sc.WorkingDirectory = '%INSTALL_DIR%'; $sc.Save()"

echo [5/5] Registering startup...
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "SmartFinder" /t REG_SZ /d "\"%INSTALL_DIR%\%EXE_NAME%\"" /f >nul

echo.
echo Installation complete!

start "" "%INSTALL_DIR%\%EXE_NAME%"

pause
endlocal