#define AppName "Accessible OBS Studio"
#define AppVersion "1.0"
#define Publisher "Tiflo.Info"
#define Website "https://tiflo.info"
#define PluginId "accessible-obs-studio"

[Setup]
AppId={{BDA542EA-4E63-4F03-9F5B-B7A8CD8E470B}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL={#Website}
AppSupportURL={#Website}
AppUpdatesURL={#Website}
DefaultDirName={commonappdata}\obs-studio\plugins\{#PluginId}
DisableDirPage=yes
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
OutputDir=..\..\..\outputs
OutputBaseFilename=AccessibleOBSStudio-1.0-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
WizardImageFile=..\assets\tiflo-info-logo.png
WizardSmallImageFile=..\assets\tiflo-info-logo.png
SetupIconFile=
CloseApplications=no
RestartApplications=no
UninstallDisplayName={#AppName} {#AppVersion}
VersionInfoVersion=1.0.0.0
VersionInfoCompany={#Publisher}
VersionInfoDescription={#AppName} installer
VersionInfoCopyright=Copyright (C) 2026 {#Publisher}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Files]
Source: "..\package\bin\64bit\accessible-obs-studio.dll"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion
Source: "..\package\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE-GPL-2.0.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\NOTICE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\assets\tiflo-info-logo.jpg"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "..\assets\tiflo-info-logo.png"; DestDir: "{app}\assets"; Flags: ignoreversion

[Code]
var
  InstallProgressPage: TOutputProgressWizardPage;
  DownloadProgressBase: Integer;
  DownloadProgressSpan: Integer;

procedure InitializeWizard();
begin
  InstallProgressPage := CreateOutputProgressPage(
    'Installing Accessible OBS Studio',
    'Preparing the plugin and checking required Microsoft components.');
end;

function WebView2Installed: Boolean;
var
  Version: String;
  Key: String;
begin
  Key := 'SOFTWARE\Microsoft\EdgeUpdate\Clients\{F1E7E3A2-46B8-4A6C-8A53-EDC92C0D0399}';
  Result := RegQueryStringValue(HKLM32, Key, 'pv', Version) or
            RegQueryStringValue(HKLM64, Key, 'pv', Version) or
            RegQueryStringValue(HKCU, Key, 'pv', Version);
end;

function VCRuntimeInstalled: Boolean;
var
  Installed: Cardinal;
begin
  Result := RegQueryDWordValue(HKLM64,
    'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
    'Installed', Installed) and (Installed = 1);
end;

function OnDownloadProgress(const Url, FileName: String;
  const Progress, ProgressMax: Int64): Boolean;
begin
  if ProgressMax > 0 then
    InstallProgressPage.SetProgress(DownloadProgressBase +
      ((Progress * DownloadProgressSpan) div ProgressMax), 100)
  else
    InstallProgressPage.SetProgress(DownloadProgressBase, 100);
  Result := True;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ResultCode: Integer;
begin
  Result := '';
  InstallProgressPage.SetText('Checking required components...', '');
  InstallProgressPage.SetProgress(0, 100);
  InstallProgressPage.Show;
  try
    InstallProgressPage.SetProgress(5, 100);
    if not VCRuntimeInstalled then
    begin
      InstallProgressPage.SetText('Downloading Microsoft Visual C++ Runtime...', '');
      DownloadProgressBase := 5;
      DownloadProgressSpan := 35;
      try
        DownloadTemporaryFile('https://aka.ms/vc14/vc_redist.x64.exe',
          'vc_redist.x64.exe', '', @OnDownloadProgress);
      except
        Result := 'The Microsoft Visual C++ Runtime is required, but it could not be downloaded. Check the Internet connection and try again. ' + GetExceptionMessage;
        Exit;
      end;
      InstallProgressPage.SetText('Installing Microsoft Visual C++ Runtime...', '');
      InstallProgressPage.SetProgress(45, 100);
      if not Exec(ExpandConstant('{tmp}\vc_redist.x64.exe'),
        '/install /quiet /norestart', '', SW_HIDE, ewWaitUntilTerminated,
        ResultCode) or ((ResultCode <> 0) and (ResultCode <> 1638) and
        (ResultCode <> 3010)) then
      begin
        Result := 'The Microsoft Visual C++ Runtime could not be installed. Error code: ' + IntToStr(ResultCode);
        Exit;
      end;
      if ResultCode = 3010 then NeedsRestart := True;
    end;

    InstallProgressPage.SetProgress(50, 100);
    if not WebView2Installed then
    begin
      InstallProgressPage.SetText('Downloading Microsoft Edge WebView2 installer...', '');
      DownloadProgressBase := 50;
      DownloadProgressSpan := 25;
      try
        DownloadTemporaryFile('https://go.microsoft.com/fwlink/p/?LinkId=2124703',
          'MicrosoftEdgeWebview2Setup.exe', '', @OnDownloadProgress);
      except
        Result := 'Microsoft Edge WebView2 Runtime is required, but it could not be downloaded. Check the Internet connection and try again. ' + GetExceptionMessage;
        Exit;
      end;
      InstallProgressPage.SetText('Installing Microsoft Edge WebView2 Runtime...', 'This can take several seconds.');
      InstallProgressPage.SetProgress(80, 100);
      if not Exec(ExpandConstant('{tmp}\MicrosoftEdgeWebview2Setup.exe'),
        '/silent /install', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) or
        ((ResultCode <> 0) and (ResultCode <> 3010)) then
      begin
        Result := 'Microsoft Edge WebView2 Runtime could not be installed. Error code: ' + IntToStr(ResultCode);
        Exit;
      end;
      if ResultCode = 3010 then NeedsRestart := True;
    end;
    InstallProgressPage.SetText('Starting plugin installation...', '');
    InstallProgressPage.SetProgress(100, 100);
  finally
    InstallProgressPage.Hide;
  end;
end;
