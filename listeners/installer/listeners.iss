//
// listeners.iss
//
// Copyright 2014 OpenDirective Ltd.
//
// Licensed under the New BSD license. You may not use this file except in
// compliance with this License.
//
// You may obtain a copy of the License at
// https://github.com/gpii/windows/blob/master/LICENSE.txt
//
// The research leading to these results has received funding from 
// the European Union's Seventh Framework Programme (FP7/2007-2013) 
// under grant agreement no. 289016.
//

; Defined here so can be compliled in IDE but can be overridden on command line
#define binPath  "..\bin\Release\" 
#define ouputDir binPath 

; Setup version is the greater of the listener versions
#define maxVersion(str fileA, str fileB) \
  ParseVersion(fileA, Local[0], Local[1], Local[2], Local[3]), \
  Local[4] = EncodeVer(Local[0], Local[1], Local[2], Local[3]), \
  ParseVersion(fileB, Local[0], Local[1], Local[2], Local[3]), \
  Local[5] = EncodeVer(Local[0], Local[1], Local[2], Local[3]), \
  DecodeVer(Max(Local[4], Local[5])) 

#define USBExe Str(AddBackslash(SourcePath) + binPath + "GPII_USBListener.exe")
#define RFIDExe Str(AddBackslash(SourcePath) + binPath + "GPII_RFIDListener.exe")
#if GetFileVersion(USBExe) == "" || GetFileVersion(RFIDExe) == ""
  #error A Listener EXE file is missing - check bin\Release
#endif
#define AppVersion maxVersion(USBExe, RFIDExe)
 
; Define this to write the preprocessed output to a file Preprocessed.iss and open it in the IDE
;#define DebugPP

[Setup]
AppName=GPII Listeners
AppVersion={#AppVersion}
AppPublisher=Raising the Floor
AppCopyright=(c) 2014 Members of the Raising the Floor Consortium"
AppPublisherURL=http://gpii.net

VersionInfoVersion={#AppVersion}
DefaultDirName={pf}\GPII
DefaultGroupName=GPII
UninstallDisplayIcon={app}\GPII_RFIDListener.exe
OutputDir={#ouputDir}
OutputBaseFilename=GPIIListenerSetup-{#AppVersion}

;SetupIconFile=GPII.ico

[Tasks]
Name: "usb"; Description: "Install the &USB Listener"; Flags:  
Name: "usbturnkey"; Description: "Run the U&SB listener on startup"; Flags: unchecked checkedonce
Name: "rfid"; Description: "Install the &RFID / NFC Listener"; Flags:  
Name: "rfidturnkey"; Description: "Run the R&FID listener on startup"; Flags: unchecked checkedonce
Name: "desktop"; Description: "Add icons to desktop"; Flags: unchecked  
Name: "startmenu"; Description: "Add icons to start menu"; Flags: unchecked  

[Files]
Source: "{#USBExe}"; DestDir: "{app}"; Tasks: usb
Source: "{#RFIDExe}"; DestDir: "{app}"; Tasks: rfid
Source: "..\..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "GPII.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "GPII_USBListener.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "GPII_RFIDListener.ico"; DestDir: "{app}"; Flags: ignoreversion
; Dependencies
; We copy the VS x86 redistributable CRT files in case they are not on the system
; C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT
; Note Inno does a 32bit install by default so sys is actually SysWow64 (yeah, I know!)
 Source: "MSVCRedist\*"; DestDir: "{sys}"; Flags: restartreplace uninsneveruninstall sharedfile

[Icons]
; Start menu
Name: "{group}\USB Listener"; Filename: "{app}\GPPI_USBListener.exe"; Comment: "Run USB listener"; IconFilename: "{app}\GPII_USBListener.ico"
Name: "{group}\RFID Listener"; Filename: "{app}\GPII_RFIDListener.exe"; Comment: "Run USB listener"; IconFilename: "{app}\GPII_RFIDListener.ico"
Name: "{group}\GPII Website"; Filename: "http://GPII.net"; Comment: "Visit the GPII website"; IconFilename: "{app}\GPII.ico"; Tasks: startmenu

; desktop - optional
Name: "{userdesktop}\GPII USB Listener"; Filename: "{app}\GPII_USBListener.exe"; Comment: "Run GPII USB listener"; IconFilename: "{app}\GPII_USBListener.ico"; Tasks:desktop
Name: "{userdesktop}\GPII RFID Listener"; Filename: "{app}\GPII_RFIDListener.exe"; Comment: "Run GPII RFID listener"; IconFilename: "{app}\GPII_RFIDListener.ico"; Tasks:desktop
Name: "{userdesktop}\GPII Website"; Filename: "http://GPII.net"; IconFilename: "{app}\GPII.ico"; Tasks: desktop

; startup items - optional
Name: "{commonstartup}\GPII USB Listener"; Filename: "{app}\GPII_USBListener.exe"; Comment: "Run GPII USB listener"; IconFilename: "{app}\GPII_USBListener.ico"; Tasks:usbturnkey and startmenu
Name: "{commonstartup}\GPII RFID Listener"; Filename: "{app}\GPII_RFIDListener.exe"; Comment: "Run GPII RFID listener"; IconFilename: "{app}\GPII_RFIDListener.ico"; Tasks:rfidturnkey and startmenu

#ifdef DebugPP
  #expr SaveToFile(AddBackslash(SourcePath) + binPath + "Preprocessed.iss"), \
        Exec(AddBackslash(CompilerPath) + "Compil32.exe", """" + AddBackslash(SourcePath) + binPath+ "Preprocessed.iss""")
#endif