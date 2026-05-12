@echo off
echo ========================================
echo Building SmartFinder Setup
echo ========================================
echo.

REM 设置版本号（每次更新时修改这里）
set VERSION=1.0.1
set BUILD_DATE=%date:~10,4%%date:~4,2%%date:~7,2%

echo Version: %VERSION%
echo Build Date: %BUILD_DATE%
echo.

REM 检查 searcher.exe 是否存在
if not exist "build\searcher.exe" (
    echo [ERROR] build\searcher.exe not found!
    echo Please run 'make all' first.
    pause
    exit /b 1
)

REM 更新 NSIS 文件中的版本号
echo [1/3] Updating version number...
powershell -Command "(Get-Content installer.nsi) -replace '!define APP_VERSION \".*\"', '!define APP_VERSION \"%VERSION%\"' | Set-Content installer.nsi"

REM 编译安装程序
echo [2/3] Compiling installer...
if exist "%PROGRAMFILES(x86)%\NSIS\makensis.exe" (
    "%PROGRAMFILES(x86)%\NSIS\makensis.exe" installer.nsi
) else if exist "%PROGRAMFILES%\NSIS\makensis.exe" (
    "%PROGRAMFILES%\NSIS\makensis.exe" installer.nsi
) else (
    echo [ERROR] NSIS not found! Please install NSIS first.
    echo Download: https://nsis.sourceforge.io/Download
    pause
    exit /b 1
)

REM 重命名输出文件
echo [3/3] Renaming output...
if exist "SmartFinderSetup.exe" (
    ren "SmartFinderSetup.exe" "SmartFinderSetup_%VERSION%_%BUILD_DATE%.exe"
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo Output: SmartFinderSetup_%VERSION%_%BUILD_DATE%.exe
echo.
pause