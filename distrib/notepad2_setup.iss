;* Notepad2-mod-jrb - Installer script
;*
;* Copyright (C) 2010-2017 XhmikosR
;*
;* This file is part of Notepad2-mod-jrb and adapted from Notepad2-mod.
;*
;* See License.txt for details.

; Requirements:
;   Inno Setup: http://www.jrsoftware.org/isdl.php

; Config
#define ProjectName              "Notepad2-mod-jrb"
#define ProjectHead              "Jonathan R.-Brochu"
#define ProjectRepo              "https://github.com/johnwait/notepad2-mod-jrb"
#define CopyrightNotice          "Copyright © 2004-2017, Florian Balmer et al.; Copyright © 2010-2017 XhmikosR; Copyright © 2019-2021" + ProjectHead + " et al."
#define AppTaskbarGrpId          "Notepad2"
;
#define Files_MainBinary         "Notepad2-jrb.exe"
#define Files_Config             "Notepad2.ini"
#define Files_License            "License_no-long-line-split.txt"
#define Files_Readme             "Readme-mod-jrb.txt"
#define Files_Readme2            "Readme-mod.txt"
#define Files_Readme3            "Readme.txt"
#define Files_Info               "Notepad2.txt"
;
#define SrcDirs_MainBinary_x86   "..\bin\Release_x86\"
#define SrcDirs_MainBinary_x64   "..\bin\Release_x64\"
#define SrcDirs_Config           ".\"
#define SrcDirs_License          ".\"
#define SrcDirs_Readme           "..\"
#define SrcDirs_Info             "..\"
;
#define InstSubDirs_PFiles       "Notepad2-jrb"
#define InstSubDirs_AppData      "Notepad2"
#define InstallerOutputPath      "."


; Preprocessor related stuff
#if VER < EncodeVer(5,5,9)
  #error Update your Inno Setup version (5.5.9 or newer)
#endif

#ifnexist SrcDirs_MainBinary_x86 + Files_MainBinary
  #pragma error "Compile " + ProjectName + " x86 first"
#endif

#ifnexist SrcDirs_MainBinary_x64 + Files_MainBinary
  #pragma error "Compile " + ProjectName + " x64 first"
#endif

#define VerMajor
#define VerMinor
#define VerBuild
#define VerRevision

#expr ParseVersion(SrcDirs_MainBinary_x86 + Files_MainBinary, VerMajor, VerMinor, VerBuild, VerRevision)

#define app_version   str(VerMajor) + "." + str(VerMinor) + "." + str(VerBuild) + "." + str(VerRevision)
#define app_name      ProjectName
#define app_copyright CopyrightNotice

[Setup]
AppId={#app_name}
AppName={#app_name}
AppVersion={#app_version}
AppVerName={#app_name} {#app_version}
AppPublisher={#ProjectHead}
AppPublisherURL={#ProjectRepo}
AppSupportURL={#ProjectRepo}
AppUpdatesURL={#ProjectRepo}
AppContact={#ProjectRepo}
AppCopyright={#app_copyright}
VersionInfoVersion={#app_version}
UninstallDisplayIcon={app}\{#Files_MainBinary}
;;#if defined(VS2017)
UninstallDisplayName={#app_name} {#app_version}
;;#else
;;UninstallDisplayName={#app_name} {#app_version} ({#compiler})
;;#endif
DefaultDirName={pf}\{#InstSubDirs_PFiles}
LicenseFile={#SrcDirs_License}{#Files_License}
OutputDir={#InstallerOutputPath}
;;#if defined(VS2017)
OutputBaseFilename={#app_name}_v{#app_version}_Setup
;;#else
;;OutputBaseFilename={#app_name}.{#app_version}_{#compiler}
;;#endif
SetupIconFile=Setup.ico
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=WizardSmallImageFile.bmp
Compression=lzma2/max
InternalCompressLevel=max
SolidCompression=yes
EnableDirDoesntExistWarning=no
AllowNoIcons=yes
ShowTasksTreeLines=yes
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableWelcomePage=yes
AllowCancelDuringInstall=no
MinVersion=6.1
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64
#ifexist "..\signinfo.txt"
SignTool=MySignTool
#endif
CloseApplications=true
SetupMutex='{#app_name}' + '_setup_mutex'


[Languages]
Name: en; MessagesFile: compiler:Default.isl


[Messages]
;;BeveledLabel     ={#app_name} {#app_version} - Compiled with {#compiler}
BeveledLabel     ={#app_name} {#app_version}
SetupAppTitle    =Setup - {#app_name}
SetupWindowTitle =Setup - {#app_name}


[CustomMessages]
en.msg_AppIsRunning          =Setup has detected that {#app_name} is currently running.%n%nPlease close all instances of it now, then click OK to continue, or Cancel to exit.
en.msg_AppIsRunningUninstall =Uninstall has detected that {#app_name} is currently running.%n%nPlease close all instances of it now, then click OK to continue, or Cancel to exit.
en.msg_DeleteSettings        =Do you also want to delete {#app_name}'s settings?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
#if defined(sse_required)
en.msg_simd_sse              =This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#elif defined(sse2_required)
en.msg_simd_sse2             =This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
en.tsk_AllUsers              =For all users
en.tsk_CurrentUser           =For the current user only
en.tsk_Other                 =Other tasks:
en.tsk_ResetSettings         =Reset {#app_name}'s settings
en.tsk_RemoveDefault         =Restore Windows notepad
en.tsk_SetDefault            =Replace Windows notepad with {#app_name}
en.tsk_StartMenuIcon         =Create a Start Menu shortcut


[Tasks]
Name: desktopicon;        Description: {cm:CreateDesktopIcon};     GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: desktopicon\user;   Description: {cm:tsk_CurrentUser};       GroupDescription: {cm:AdditionalIcons}; Flags: unchecked exclusive
Name: desktopicon\common; Description: {cm:tsk_AllUsers};          GroupDescription: {cm:AdditionalIcons}; Flags: unchecked exclusive
Name: startup_icon;       Description: {cm:tsk_StartMenuIcon};     GroupDescription: {cm:AdditionalIcons}
Name: quicklaunchicon;    Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked;             OnlyBelowVersion: 6.01
Name: reset_settings;     Description: {cm:tsk_ResetSettings};     GroupDescription: {cm:tsk_Other};       Flags: checkedonce unchecked; Check: SettingsExistCheck()
Name: set_default;        Description: {cm:tsk_SetDefault};        GroupDescription: {cm:tsk_Other};                                     Check: not DefaulNotepadCheck()
Name: remove_default;     Description: {cm:tsk_RemoveDefault};     GroupDescription: {cm:tsk_Other};       Flags: checkedonce unchecked; Check: DefaulNotepadCheck()


[Files]
Source: {#SrcDirs_MainBinary_x64}{#Files_MainBinary};  DestDir: {app};                                Flags: ignoreversion;                         Check: Is64BitInstallMode()
Source: {#SrcDirs_MainBinary_x86}{#Files_MainBinary};  DestDir: {app};                                Flags: ignoreversion;                         Check: not Is64BitInstallMode()
Source: {#SrcDirs_License}{#Files_License};            DestDir: {app};                                Flags: ignoreversion
Source: {#SrcDirs_Info}{#Files_Info};                  DestDir: {app};                                Flags: ignoreversion
Source: {#SrcDirs_Readme}{#Files_Readme};              DestDir: {app};                                Flags: ignoreversion
Source: {#SrcDirs_Readme}{#Files_Readme2};             DestDir: {app};                                Flags: ignoreversion
Source: {#SrcDirs_Readme}{#Files_Readme3};             DestDir: {app};                                Flags: ignoreversion
Source: {#SrcDirs_Config}{#Files_Config};              DestDir: {userappdata}\{#InstSubDirs_AppData}; Flags: onlyifdoesntexist uninsneveruninstall


[Icons]
;;Name: {commondesktop}\{#app_name}; Filename: {app}\{#Files_MainBinary}; Tasks: desktopicon\common; Comment: {#app_name} {#app_version}; WorkingDir: {app}; AppUserModelID: {#AppTaskbarGrpId}; IconFilename: {app}\{#Files_MainBinary}; IconIndex: 0
;;Name: {userdesktop}\{#app_name};   Filename: {app}\{#Files_MainBinary}; Tasks: desktopicon\user;   Comment: {#app_name} {#app_version}; WorkingDir: {app}; AppUserModelID: {#AppTaskbarGrpId}; IconFilename: {app}\{#Files_MainBinary}; IconIndex: 0
;;Name: {userstartmenu}\{#app_name}; Filename: {app}\{#Files_MainBinary}; Tasks: startup_icon;       Comment: {#app_name} {#app_version}; WorkingDir: {app}; AppUserModelID: {#AppTaskbarGrpId}; IconFilename: {app}\{#Files_MainBinary}; IconIndex: 0
;;Name: {#quick_launch}\{#app_name}; Filename: {app}\{#Files_MainBinary}; Tasks: quicklaunchicon;    Comment: {#app_name} {#app_version}; WorkingDir: {app};                                     IconFilename: {app}\{#Files_MainBinary}; IconIndex: 0


[INI]
Filename: {app}\{#Files_Config}; Section: Notepad2; Key: {#Files_Config}; String: %APPDATA%\{#InstSubDirs_AppData}\{#Files_Config}


[Run]
Filename: {app}\{#Files_MainBinary}; Description: {cm:LaunchProgram,{#app_name}}; WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked


[InstallDelete]
;;Type: files;      Name: {userdesktop}\{#app_name}.lnk;   Check: not IsTaskSelected('desktopicon\user')   and IsUpgrade()
;;Type: files;      Name: {commondesktop}\{#app_name}.lnk; Check: not IsTaskSelected('desktopicon\common') and IsUpgrade()
;;Type: files;      Name: {userstartmenu}\{#app_name}.lnk; Check: not IsTaskSelected('startup_icon')       and IsUpgrade()
;;Type: files;      Name: {#quick_launch}\{#app_name}.lnk; Check: not IsTaskSelected('quicklaunchicon')    and IsUpgrade(); OnlyBelowVersion: 6.01
Type: files;      Name: {app}\{#Files_Config}
Type: files;      Name: {app}\{#Files_Readme}


[UninstallDelete]
Type: files;      Name: {app}\{#Files_Config}
Type: dirifempty; Name: {app}


[Code]
const IFEO = 'SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\notepad.exe';

#if defined(sse_required) || defined(sse2_required)
function IsProcessorFeaturePresent(Feature: Integer): Boolean;
external 'IsProcessorFeaturePresent@kernel32.dll stdcall';
#endif


// Check if Notepad2 has replaced Windows Notepad
function DefaulNotepadCheck(): Boolean;
var
  sDebugger: String;
begin
  if RegQueryStringValue(HKLM, IFEO, 'Debugger', sDebugger) and
  (sDebugger = (ExpandConstant('"{app}\{#Files_MainBinary}" /z'))) then begin
    Log('Custom Code: {#app_name} is set as the default notepad');
    Result := True;
  end
  else begin
    Log('Custom Code: {#app_name} is NOT set as the default notepad');
    Result := False;
  end;
end;


#if defined(sse_required)
function IsSSESupported(): Boolean;
begin
  // PF_XMMI_INSTRUCTIONS_AVAILABLE
  Result := IsProcessorFeaturePresent(6);
end;

#elif defined(sse2_required)

function IsSSE2Supported(): Boolean;
begin
  // PF_XMMI64_INSTRUCTIONS_AVAILABLE
  Result := IsProcessorFeaturePresent(10);
end;

#endif


function IsOldBuildInstalled(sInfFile: String): Boolean;
begin
  if RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Notepad2') and
      FileExists(ExpandConstant('{pf}\{#InstSubDirs_PFiles}\' + sInfFile)) then
    Result := True
  else
    Result := False;
end;


function IsUpgrade(): Boolean;
var
  sPrevPath: String;
begin
  sPrevPath := WizardForm.PrevAppDir;
  Result := (sPrevPath <> '');
end;


// Check if Notepad2's settings exist
function SettingsExistCheck(): Boolean;
begin
  if FileExists(ExpandConstant('{userappdata}\{#InstSubDirs_AppData}\{#Files_Config}')) then begin
    Log('Custom Code: Settings are present');
    Result := True;
  end
  else begin
    Log('Custom Code: Settings are NOT present');
    Result := False;
  end;
end;


function UninstallOldVersion(sInfFile: String): Integer;
var
  iResultCode: Integer;
begin
  // Return Values:
  // 0 - no idea
  // 1 - error executing the command
  // 2 - successfully executed the command

  // default return value
  Result := 0;
  // TODO: use RegQueryStringValue
  if not Exec('rundll32.exe', ExpandConstant('advpack.dll,LaunchINFSectionEx ' + '"{pf}\{#InstSubDirs_PFiles}\' + sInfFile +'",DefaultUninstall,,8,N'), '', SW_HIDE, ewWaitUntilTerminated, iResultCode) then begin
    Result := 1;
  end
  else begin
    Result := 2;
    Sleep(200);
  end;
end;


function ShouldSkipPage(PageID: Integer): Boolean;
begin
  // Hide the license page if IsUpgrade()
  if IsUpgrade() and (PageID = wpLicense) then
    Result := True;
end;


procedure AddReg();
begin
  RegWriteStringValue(HKCR, 'Applications\{#Files_MainBinary}', 'AppUserModelID', '{#AppTaskbarGrpId}');
  RegWriteStringValue(HKCR, 'Applications\{#Files_MainBinary}\shell\open\command', '', ExpandConstant('"{app}\{#Files_MainBinary}" %1'));
  RegWriteStringValue(HKCR, '*\OpenWithList\{#Files_MainBinary}', '', '');
end;


procedure CleanUpSettings();
begin
  DeleteFile(ExpandConstant('{userappdata}\{#InstSubDirs_AppData}\{#Files_Config}'));
  RemoveDir(ExpandConstant('{userappdata}\{#InstSubDirs_AppData}'));
end;


procedure RemoveReg();
begin
  RegDeleteKeyIncludingSubkeys(HKCR, 'Applications\{#Files_MainBinary}');
  RegDeleteKeyIncludingSubkeys(HKCR, '*\OpenWithList\{#Files_MainBinary}');
end;


procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectTasks then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall)
  else if CurPageID = wpFinished then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonFinish);
end;


procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then begin
    if IsTaskSelected('reset_settings') then
      CleanUpSettings();

    if IsOldBuildInstalled('Uninstall.inf') or IsOldBuildInstalled('Notepad2.inf') then begin
      if IsOldBuildInstalled('Uninstall.inf') then begin
        Log('Custom Code: The old build is installed, will try to uninstall it');
        if UninstallOldVersion('Uninstall.inf') = 2 then
          Log('Custom Code: The old build was successfully uninstalled')
        else
          Log('Custom Code: Something went wrong when uninstalling the old build');
      end;

      if IsOldBuildInstalled('Notepad2.inf') then begin
        Log('Custom Code: The official Notepad2 build is installed, will try to uninstall it');
        if UninstallOldVersion('Notepad2.inf') = 2 then
          Log('Custom Code: The official Notepad2 build was successfully uninstalled')
        else
          Log('Custom Code: Something went wrong when uninstalling the official Notepad2 build');
      end;

      // This is the case where the old build is installed; the DefaulNotepadCheck() returns true
      // and the set_default task isn't selected
      if not IsTaskSelected('remove_default') then
        RegWriteStringValue(HKLM, IFEO, 'Debugger', ExpandConstant('"{app}\{#Files_MainBinary}" /z'));

    end;
  end;

  if CurStep = ssPostInstall then begin
    if IsTaskSelected('set_default') then
      RegWriteStringValue(HKLM, IFEO, 'Debugger', ExpandConstant('"{app}\{#Files_MainBinary}" /z'));
    if IsTaskSelected('remove_default') then begin
      RegDeleteValue(HKLM, IFEO, 'Debugger');
      RegDeleteKeyIfEmpty(HKLM, IFEO);
    end;
    // Always add Notepad2's AppUserModelID and the rest registry values
    AddReg();
  end;

end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete Notepad2's settings
  if CurUninstallStep = usUninstall then begin
    if SettingsExistCheck() then begin
      if SuppressibleMsgBox(CustomMessage('msg_DeleteSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDNO) = IDYES then
        CleanUpSettings();
    end;

    if DefaulNotepadCheck() then
      RegDeleteValue(HKLM, IFEO, 'Debugger');
    RegDeleteKeyIfEmpty(HKLM, IFEO);
    RemoveReg();

  end;
end;


procedure InitializeWizard();
begin
  WizardForm.SelectTasksLabel.Hide;
  WizardForm.TasksList.Top    := 0;
  WizardForm.TasksList.Height := PageFromID(wpSelectTasks).SurfaceHeight;
end;


function InitializeSetup(): Boolean;
begin
    Result := True;

#if defined(sse2_required)
    if not IsSSE2Supported() then begin
      SuppressibleMsgBox(CustomMessage('msg_simd_sse2'), mbCriticalError, MB_OK, MB_OK);
      Result := False;
    end;
#elif defined(sse_required)
    if not IsSSESupported() then begin
      SuppressibleMsgBox(CustomMessage('msg_simd_sse'), mbCriticalError, MB_OK, MB_OK);
      Result := False;
    end;
#endif

end;
