@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul 2>&1

echo.
echo  ╔══════════════════════════════════════╗
echo  ║     Smart Finder — Build Script      ║
echo  ╚══════════════════════════════════════╝
echo.

:: ── ตรวจสอบว่า makefile มีอยู่จริง ─────────────────────────────
if not exist "makefile" (
    echo [ERROR] ไม่พบ makefile
    echo ตรวจสอบว่าคุณอยู่ในโฟลเดอร์ที่ถูกต้องหรือไม่
    pause & exit /b 1
)

:: ── ตรวจ MSYS2 / MinGW gcc ───────────────────────────────────
set GCC=
for %%P in (
    "C:\msys64\ucrt64\bin\gcc.exe"
    "C:\msys64\mingw64\bin\gcc.exe"
    "C:\mingw64\bin\gcc.exe"
    "C:\mingw\bin\gcc.exe"
) do (
    if exist %%P (
        if "!GCC!"=="" set GCC=%%~P
    )
)

if "!GCC!"=="" (
    where gcc >nul 2>&1
    if !errorlevel!==0 set GCC=gcc
)

if "!GCC!"=="" (
    echo [ERROR] ไม่พบ gcc
    echo กรุณาติดตั้ง MSYS2 แล้วรัน:  pacman -S mingw-w64-ucrt-x86_64-gcc
    pause & exit /b 1
)
echo [1/4] Compiler: !GCC!

:: ── หา make ──────────────────────────────────────────────────
set MAKE=
for %%P in (
    "C:\msys64\ucrt64\bin\mingw32-make.exe"
    "C:\msys64\usr\bin\make.exe"
    "C:\msys64\mingw64\bin\mingw32-make.exe"
) do (
    if exist %%P (
        if "!MAKE!"=="" set MAKE=%%~P
    )
)
if "!MAKE!"=="" (
    where make >nul 2>&1
    if !errorlevel!==0 set MAKE=make
)
if "!MAKE!"=="" (
    where mingw32-make >nul 2>&1
    if !errorlevel!==0 set MAKE=mingw32-make
)
if "!MAKE!"=="" (
    echo [ERROR] ไม่พบ make
    echo กรุณาติดตั้ง MSYS2 หรือ MinGW
    pause & exit /b 1
)
echo [2/4] Make: !MAKE!

:: ── Clean ก่อน build ────────────────────────────────────────
echo [3/4] Cleaning and Building...
echo.

"!MAKE!" clean
if !errorlevel! neq 0 (
    echo [WARNING] Clean failed, continuing...
)

"!MAKE!"
if !errorlevel! neq 0 (
    echo.
    echo [ERROR] Build failed
    pause & exit /b 1
)

if not exist "build\searcher.exe" (
    echo [ERROR] ไม่พบ build\searcher.exe
    pause & exit /b 1
)
echo.
echo [3/4] Build OK — build\searcher.exe

:: ── สร้าง release folder ─────────────────────────────────────
echo [4/4] Packaging...
set RELEASE=release
if exist "%RELEASE%" (
    echo Removing old release folder...
    rmdir /s /q "%RELEASE%"
)
mkdir "%RELEASE%"

copy /Y "build\searcher.exe" "%RELEASE%\SmartFinder.exe" >nul
if !errorlevel! neq 0 (
    echo [ERROR] Copy failed
    pause & exit /b 1
)
echo         SmartFinder.exe copied

:: สร้าง README สั้นๆ
(
echo Smart Finder
echo ============
echo.
echo วิธีใช้:
echo   1. ดับเบิลคลิก SmartFinder.exe เพื่อเริ่มโปรแกรม
echo   2. กด Ctrl+Space เพื่อเปิดช่องค้นหา
echo   3. คลิกขวา tray icon ^(System Tray^) เพื่อตั้งค่า
echo      - เปลี่ยน hotkey
echo      - เปิด/ปิด Run at Startup
echo.
echo ข้อมูลจะถูกบันทึกใน:
echo   %%APPDATA%%\SmartFinder\
echo.
echo Uninstall:
echo   ลบโฟลเดอร์ %%APPDATA%%\SmartFinder\
echo   และลบ shortcut ที่ Desktop/Start Menu
) > "%RELEASE%\README.txt"

echo         README.txt created

:: ── สร้าง install.bat ใน release ────────────────────────────
(
echo @echo off
echo title Smart Finder Installer
echo setlocal enabledelayedexpansion
echo chcp 65001 ^>nul 2^>^&1
echo.
echo echo ==========================================
echo echo   Smart Finder Installer
echo echo ==========================================
echo echo.
echo.
echo :: ตรวจสอบว่า SmartFinder.exe มีอยู่
echo if not exist "%%~dp0SmartFinder.exe" ^(
echo     echo [ERROR] SmartFinder.exe not found
echo     pause
echo     exit /b 1
echo ^)
echo.
echo :: ติดตั้งไปที่ LocalAppData
echo set "TARGET=%%LOCALAPPDATA%%\SmartFinder"
echo if not exist "%%TARGET%%" mkdir "%%TARGET%%" 2^>nul
echo copy /Y "%%~dp0SmartFinder.exe" "%%TARGET%%\SmartFinder.exe" ^>nul
echo if !errorlevel! neq 0 ^(
echo     echo [ERROR] Copy failed
echo     pause
echo     exit /b 1
echo ^)
echo echo [OK] Installed to: %%TARGET%%
echo.
echo :: Desktop shortcut
echo echo Creating desktop shortcut...
echo powershell -NoProfile -Command "$ws=New-Object -ComObject WScript.Shell; $sc=$ws.CreateShortcut('%%USERPROFILE%%\Desktop\Smart Finder.lnk'); $sc.TargetPath='%%TARGET%%\SmartFinder.exe'; $sc.Save()" ^>nul 2^>^&1
echo echo [OK] Desktop shortcut created
echo.
echo :: Start Menu shortcut
echo echo Creating Start Menu shortcut...
echo set "SM=%%APPDATA%%\Microsoft\Windows\Start Menu\Programs"
echo if not exist "%%SM%%" mkdir "%%SM%%" 2^>nul
echo powershell -NoProfile -Command "$ws=New-Object -ComObject WScript.Shell; $sc=$ws.CreateShortcut('%%SM%%\Smart Finder.lnk'); $sc.TargetPath='%%TARGET%%\SmartFinder.exe'; $sc.Save()" ^>nul 2^>^&1
echo echo [OK] Start Menu shortcut created
echo.
echo :: Run at Startup
echo echo Adding to Windows Startup...
echo reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "SmartFinder" /t REG_SZ /d "\"%%TARGET%%\SmartFinder.exe\"" /f ^>nul 2^>^&1
echo echo [OK] Will run on startup
echo.
echo :: Launch program
echo echo Launching Smart Finder...
echo start "" "%%TARGET%%\SmartFinder.exe"
echo.
echo echo ==========================================
echo echo   Installation Complete!
echo echo ==========================================
echo echo.
echo echo Smart Finder is now running
echo echo Look for tray icon near clock
echo echo Press Ctrl+Space to search
echo echo.
echo pause
) > "%RELEASE%\install.bat"

echo         install.bat created

:: ── สร้าง uninstall.bat ใน release ──────────────────────────
(
echo @echo off
echo title Uninstall Smart Finder
echo setlocal enabledelayedexpansion
echo chcp 65001 ^>nul 2^>^&1
echo.
echo echo ==========================================
echo echo   Uninstall Smart Finder
echo echo ==========================================
echo echo.
echo echo This will remove:
echo echo   - %%LOCALAPPDATA%%\SmartFinder
echo echo   - Desktop shortcut
echo echo   - Start Menu shortcut
echo echo   - Startup registry entry
echo echo.
echo set /p confirm="Type YES to continue: 
echo if not "%%confirm%%"=="YES" ^(
echo     echo Uninstall cancelled
echo     pause
echo     exit /b 0
echo ^)
echo echo.
echo echo Stopping program...
echo taskkill /F /IM SmartFinder.exe 2^>nul
echo timeout /t 2 /nobreak ^>nul
echo.
echo echo Removing files...
echo rmdir /S /Q "%%LOCALAPPDATA%%\SmartFinder" 2^>nul
echo.
echo echo Removing shortcuts...
echo del /F /Q "%%USERPROFILE%%\Desktop\Smart Finder.lnk" 2^>nul
echo rmdir /S /Q "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\Smart Finder.lnk" 2^>nul
echo.
echo echo Removing startup entry...
echo reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "SmartFinder" /f 2^>nul
echo.
echo echo ==========================================
echo echo   Uninstall Complete!
echo echo ==========================================
echo timeout /t 3 /nobreak
) > "%RELEASE%\uninstall.bat"

echo         uninstall.bat created

:: ── สรุป ────────────────────────────────────────────────────
echo.
echo ==========================================
echo   Build Complete!
echo ==========================================
echo.
echo  ┌─────────────────────────────────────┐
echo  │  release\                           │
echo  │    SmartFinder.exe   ← โปรแกรมหลัก  │
echo  │    install.bat       ← ติดตั้ง       │
echo  │    uninstall.bat     ← ถอนการติดตั้ง │
echo  │    README.txt        ← คู่มือ        │
echo  └─────────────────────────────────────┘
echo.
echo  วิธีใช้:
echo    รันเลย  : ดับเบิลคลิก release\SmartFinder.exe
echo    ติดตั้ง : ดับเบิลคลิก release\install.bat
echo    ถอนรื้อ : ดับเบิลคลิก release\uninstall.bat
echo.

:: เปิด folder release ให้ดูเลย
explorer "%CD%\%RELEASE%"

echo.
pause
endlocal