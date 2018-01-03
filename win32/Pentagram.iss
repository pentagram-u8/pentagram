[Setup]
AppCopyright=2011
AppName=Pentagram
AppVerName=Pentagram SVN
AppPublisher=The Pentagram Team
AppPublisherURL=http://pentagram.sourceforge.net/
AppSupportURL=http://pentagram.sourceforge.net/
AppUpdatesURL=http://pentagram.sourceforge.net/
DefaultDirName={pf}\Pentagram
DefaultGroupName=Pentagram
AllowNoIcons=true
AlwaysUsePersonalGroup=false
EnableDirDoesntExistWarning=false
OutputDir=C:\Ultima8
OutputBaseFilename=Pentagramwin32
DisableStartupPrompt=true
AppendDefaultDirName=false    
SolidCompression=true
DirExistsWarning=no   

[Icons]
Name: {group}\{cm:UninstallProgram, Pentagram}; Filename: {uninstallexe}
Name: {group}\AUTHORS; Filename: {app}\AUTHORS.txt; WorkingDir: {app}; Comment: ChangeLog; Flags: createonlyiffileexists
Name: {group}\ChangeLog; Filename: {app}\ChangeLog.txt; WorkingDir: {app}; Comment: ChangeLog; Flags: createonlyiffileexists
Name: {group}\COPYING; Filename: {app}\COPYING.txt; WorkingDir: {app}; Comment: ChangeLog; Flags: createonlyiffileexists
Name: {group}\FAQ; Filename: {app}\FAQ.txt; WorkingDir: {app}; Comment: FAQ; Flags: createonlyiffileexists
Name: {group}\Pentagram; Filename: {app}\pentagram.exe; WorkingDir: {app}; Comment: pentagram; Flags: createonlyiffileexists
Name: {group}\README; Filename: {app}\README.txt; WorkingDir: {app}; Comment: README; Flags: createonlyiffileexists

[Dirs]
Name: {app}\tools

[Files]
Source: AUTHORS.txt; DestDir: {app}; Flags: ignoreversion
Source: ChangeLog.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.txt; DestDir: {app}; Flags: ignoreversion
Source: FAQ.txt; DestDir: {app}; Flags: ignoreversion
Source: pentagram.exe; DestDir: {app}; Flags: ignoreversion
Source: pentagram.ini.example; DestDir: {app}; Flags: ignoreversion
Source: README.txt; DestDir: {app}; Flags: ignoreversion isreadme
Source: README-SDL.txt; DestDir: {app}; Flags: ignoreversion
Source: SDL.dll; DestDir: {app}; Flags: ignoreversion
Source: SDL_ttf.dll; DestDir: {app};
Source: tools\amf2mod.exe; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\data2c.exe; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\disasm.exe; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\flexpack.exe; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\fold.exe; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\maps.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\musiceggs.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\musicflx.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\shapeconv.exe; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\shapes.cmp.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\snippets.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8anim.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8cheat.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8fonts.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8gfxfmt.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8mapfmt.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8npc.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8savfmt.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8sfxfmt.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8skf.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8typeflag.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8usecode.txt; DestDir: {app}\tools\; Flags: ignoreversion
Source: tools\u8weapon.txt; DestDir: {app}\tools\; Flags: ignoreversion

[INI]
//Windows 95/98/ME
Filename: {app}\pentagram.ini; Section: pentagram; Key: defaultgame; String: u8; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: fullscreen; String: no; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: scalex; String: 320; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: scaley; String: 240; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: scaler; String: point; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: width; String: 640; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: height; String: 480; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: bpp; String: 32; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: ttf; String: no; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: midi_driver; String: windows; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: skipstart; String: no; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: pentagram; Key: textdelay; String: 8; Flags: createkeyifdoesntexist; MinVersion: 1, 0
Filename: {app}\pentagram.ini; Section: u8; Key: path; String: {code:GetDataDir}; MinVersion: 1, 0

//WINDOWS NT4/2000/XP/7
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: defaultgame; String: u8; Flags: createkeyifdoesntexist; MinVersion: 0, 1 
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: fullscreen; String: no; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: scalex; String: 320; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: scaley; String: 240; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: scaler; String: point; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: width; String: 640; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: height; String: 480; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: bpp; String: 32; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: ttf; String: no; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: midi_driver; String: windows; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: skipstart; String: no; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: pentagram; Key: textdelay; String: 8; Flags: createkeyifdoesntexist; MinVersion: 0, 1
Filename: {userappdata}\Pentagram\pentagram.ini; Section: u8; Key: path; String: {code:GetDataDir}; MinVersion: 0, 1

[Run]
Filename: {app}\pentagram.exe; Flags: nowait skipifdoesntexist postinstall skipifsilent; Description: Launch Pentagram

[Code]
var
  DataDirPage: TInputDirWizardPage;

procedure InitializeWizard;
begin
  // Create the page

  DataDirPage := CreateInputDirPage(wpSelectDir,
    'Select Game Folder', 'Select the folder where Ultima 8: Pagan is installed',
    'Please enter the path where Ultima 8: Pagan is installed.',
    False, '');
  DataDirPage.Add('');

  DataDirPage.Values[0] := GetPreviousData('DataDir', '');
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  // Store the selected folder for further reinstall/upgrade
  SetPreviousData(PreviousDataKey, 'DataDir', DataDirPage.Values[0]);
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  // Set default folder if empty
  if DataDirPage.Values[0] = '' then
     DataDirPage.Values[0] := ExpandConstant('{sd}\UC\Ultima8');
  Result := True;
end;

function GetDataDir(Param: String): String;
begin
  { Return the selected DataDir }
  Result := DataDirPage.Values[0];
end;
