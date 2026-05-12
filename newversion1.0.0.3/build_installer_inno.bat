@echo off
echo Building with Inno Setup...

set VERSION=1.0.1
set OUTPUT=SmartFinderSetup_v%VERSION%.exe

REM 编译
iscc /DMyAppVersion="%VERSION%" installer.iss

if exist "SmartFinderSetup.exe" (
    ren "SmartFinderSetup.exe" "%OUTPUT%"
    echo Created: %OUTPUT%
)

pause