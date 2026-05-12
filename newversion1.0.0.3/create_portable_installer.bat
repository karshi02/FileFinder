@echo off
title SmartFinder Portable Installer Creator

echo ========================================
echo Creating SmartFinder Portable Package
echo ========================================

set VERSION=1.0.1
set PACKAGE_NAME=SmartFinder_v%VERSION%_Portable

REM 创建临时目录
if exist "%PACKAGE_NAME%" rmdir /s /q "%PACKAGE_NAME%"
mkdir "%PACKAGE_NAME%"
mkdir "%PACKAGE_NAME%\data"

REM 复制文件
copy "build\searcher.exe" "%PACKAGE_NAME%\" >nul
copy "install.bat" "%PACKAGE_NAME%\" >nul 2>nul

REM 创建自解压安装包（需要 WinRAR 或 7-Zip）
if exist "%ProgramFiles%\WinRAR\WinRAR.exe" (
    echo Creating self-extracting archive...
    "%ProgramFiles%\WinRAR\WinRAR.exe" a -sfx -ep1 "%PACKAGE_NAME%.exe" "%PACKAGE_NAME%"
) else (
    echo Creating ZIP archive...
    powershell Compress-Archive -Path "%PACKAGE_NAME%" -DestinationPath "%PACKAGE_NAME%.zip" -Force
)

REM 清理临时目录
rmdir /s /q "%PACKAGE_NAME%"

echo.
echo Created: %PACKAGE_NAME%.exe or .zip
echo.
pause