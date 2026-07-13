@echo off
REM Wrap the PowerShell build script to make it easy to run
powershell.exe -ExecutionPolicy Bypass -File "%~dp0build.ps1" -QtPath "C:\runtime\QT\6.11.1\mingw_64" -QtToolsPath "C:\runtime\QT\Tools" -VcpkgPath "C:\vcpkg"
if %ERRORLEVEL% NEQ 0 (
    echo [-] Build failed!
    exit /b %ERRORLEVEL%
)
