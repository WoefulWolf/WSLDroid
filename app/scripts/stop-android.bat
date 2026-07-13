@echo off
SET WSL_DISTRO=%1

echo [INFO] Killing RDP Emulator...
taskkill /F /IM rdp_emulator.exe >nul 2>&1

echo [INFO] Stopping Waydroid session...
wsl -d %WSL_DISTRO% waydroid session stop

echo [INFO] Killing Weston...
wsl -d %WSL_DISTRO% bash -c "pkill weston"

echo [INFO] Killing Socat...
wsl -d %WSL_DISTRO% bash -c "pkill socat"

echo [INFO] Android stopped successfully.
