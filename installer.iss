#define MyAppName "WSLDroid"
#define MyAppVersion "0.1.0"
#define MyAppExeName "WSLDroidApp.exe"
#define DistroName "WSLDroid"

[Setup]
AppId={{A6D7A6D8-1D2C-4B4A-8B2C-7D5EDB8D7A10}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
DefaultDirName={autopf}\WSLDroid
DefaultGroupName={#MyAppName}
OutputBaseFilename=WSLDroid-Setup
SetupIconFile=icons\installer.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Tasks]
Name: "desktopicon"; Description: "Buat ikon desktop"; GroupDescription: "Ikon tambahan:"; Flags: unchecked

[Dirs]
Name: "C:\kernel"

[Files]
Source: "bin\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "kernel\wsl-kernel.zip"; DestDir: "C:\kernel"; Flags: ignoreversion
Source: "kernel\.wslconfig"; DestDir: "{code:GetUserProfile}"; DestName: ".wslconfig"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{sys}\wsl.exe"; Parameters: "--shutdown"; Flags: runhidden waituntilterminated; Description: "Terapkan konfigurasi WSL"; StatusMsg: "Menjalankan ulang WSL..."; Check: ShouldReloadWsl
Filename: "{app}\{#MyAppExeName}"; Description: "Jalankan {#MyAppName}"; Flags: nowait postinstall skipifsilent

[Code]
function GetUserProfile(Param: string): string;
begin
  Result := GetEnv('USERPROFILE');
end;

procedure RaiseIfFailed(const FileName, Params, WorkingDir, StepName: string);
var
  ResultCode: Integer;
begin
  if not Exec(FileName, Params, WorkingDir, SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    MsgBox(StepName + ' gagal dijalankan.', mbError, MB_OK);
    Abort;
  end;

  if ResultCode <> 0 then
  begin
    MsgBox(Format('%s gagal dengan exit code %d.', [StepName, ResultCode]), mbError, MB_OK);
    Abort;
  end;
end;

procedure DeleteIfExists(const FileName: string);
begin
  if FileExists(FileName) then
    DeleteFile(FileName);
end;

function ShouldReloadWsl: Boolean;
begin
  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  KernelZipPath: string;
  KernelExtractCmd: string;
  WslImageDir: string;
  WslTarXz: string;
  WslTar: string;
  DistroInstallDir: string;
  ImportCmd: string;
begin
  if CurStep <> ssPostInstall then
    Exit;

  KernelZipPath := ExpandConstant('C:\kernel\wsl-kernel.zip');
  KernelExtractCmd := 'Expand-Archive -LiteralPath "' + KernelZipPath + '" -DestinationPath "C:\kernel" -Force';
  RaiseIfFailed(
    ExpandConstant('{sys}\powershell.exe'),
    '-NoProfile -ExecutionPolicy Bypass -Command "' + KernelExtractCmd + '"',
    '',
    'Ekstraksi kernel WSL'
  );
  DeleteIfExists(KernelZipPath);

  WslImageDir := ExpandConstant('{app}\wsl');
  WslTarXz := ExpandConstant('{app}\wsl\debian.tar.xz');
  WslTar := ExpandConstant('{app}\wsl\debian.tar');
  DistroInstallDir := ExpandConstant('{app}\WSLDroid');

  ForceDirectories(WslImageDir);
  ForceDirectories(DistroInstallDir);

  RaiseIfFailed(
    ExpandConstant('{sys}\cmd.exe'),
    '/C tar -xf "' + WslTarXz + '" -C "' + WslImageDir + '"',
    WslImageDir,
    'Ekstraksi debian.tar.xz'
  );

  ImportCmd := '--import "WSLDroid" "' + DistroInstallDir + '" "' + WslTar + '" --version 2';
  RaiseIfFailed(
    ExpandConstant('{sys}\wsl.exe'),
    ImportCmd,
    WslImageDir,
    'Import distro WSL'
  );

  DeleteIfExists(WslTar);
  DeleteIfExists(WslTarXz);
end;