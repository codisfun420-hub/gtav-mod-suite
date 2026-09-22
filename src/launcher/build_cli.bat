@echo off
echo [*] Building GTAVCli...
dotnet publish -c Release -r win-x64 --no-self-contained -p:PublishSingleFile=true
if %errorlevel% neq 0 (
    echo [!] Build FAILED
    pause
    exit /b 1
)
echo [OK] Build succeeded.
set OUT=bin\Release\net8.0-windows\win-x64\publish\GTAVCli.exe
echo [*] Deploying to Desktop...
copy /Y "%OUT%" "%USERPROFILE%\Desktop\GTAVCli.exe"
echo [OK] GTAVCli.exe op Desktop gezet.
echo.
echo [*] Shortcut bijwerken...
powershell -NoProfile -Command "$s=(New-Object -COM WScript.Shell).CreateShortcut('%USERPROFILE%\Desktop\GTA V Mod Launcher.lnk');$s.TargetPath='%USERPROFILE%\Desktop\GTAVCli.exe';$s.WorkingDirectory='%USERPROFILE%\Desktop';$s.IconLocation='%USERPROFILE%\Desktop\GTAVCli.exe';$s.Save()"
echo [OK] Klaar!
pause
