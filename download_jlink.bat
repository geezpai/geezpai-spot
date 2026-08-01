@echo off
setlocal

set SCRIPT_DIR=%~dp0

echo [INFO] Download firmware with J-Link...
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%flash.ps1" -Runner jlink %*
set EXIT_CODE=%ERRORLEVEL%

if not "%EXIT_CODE%"=="0" (
    echo [ERROR] Download failed, exit code: %EXIT_CODE%
    pause
    exit /b %EXIT_CODE%
)

echo [INFO] Download completed.
pause
exit /b 0
