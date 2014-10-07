#define binPath  "..\bin\debug\" 

; Our version is the greater of the version of listners
#define maxVersion(str fileA, str fileB) \
  ParseVersion(fileA, Local[0], Local[1], Local[2], Local[3]), \
  Local[4] = EncodeVer(Local[0], Local[1], Local[2], Local[3]), \
  ParseVersion(fileB, Local[0], Local[1], Local[2], Local[3]), \
  Local[5] = EncodeVer(Local[0], Local[1], Local[2], Local[3]), \
  DecodeVer(Max(Local[4], Local[5])) 

#define USB Str(AddBackslash(SourcePath) + binPath + "GPII_USBListener.exe")
#define RFID Str(AddBackslash(SourcePath) + binPath + "GPII_RFIDListener.exe")
#if GetFileVersion(USB) == "" || GetFileVersion(RFID) == ""
  #error A Listener EXE file is missing - check bin\Release
#endif
#define AppVersion maxVersion(USB, RFID)
 
; Define this to write the preprocessed output to a file Preprocessed.iss and open it in the IDE
;#define DebugPP

[Setup]
AppName=GPII Listeners
AppVersion={#AppVersion}
VersionInfoVersion={#AppVersion}
DefaultDirName={pf}\GPII
DefaultGroupName=GPII
UninstallDisplayIcon={app}\GPII_RFIDListener.exe
OutputDir=..\bin\Release
OutputBaseFilename=GPIIListenerSetup-{#AppVersion}

AppPublisher=OpenDirective for RtF
AppCopyright=Copyright (C) 2014 OpenDirective
AppPublisherURL=http://opendirective.com
;SetupIconFile=GPII.ico

[Tasks]
Name: "usb"; Description: "Install the &USB Listener"; Flags:  
Name: "usbturnkey"; Description: "Run the U&SB listener on startup"; Flags: unchecked checkedonce
Name: "rfid"; Description: "Install the &RFID / NFC Listener"; Flags:  
Name: "rfidturnkey"; Description: "Run the R&FID listener on startup"; Flags: unchecked checkedonce
Name: "desktop"; Description: "Add icons to desktop"; Flags: unchecked  
Name: "startmenu"; Description: "Add icons to start menu"; Flags: unchecked  

[Files]
Source: "{#binPath}GPII_USBListener.exe"; DestDir: "{app}"; Tasks: usb
Source: "{#binPath}GPII_RFIDListener.exe"; DestDir: "{app}"; Tasks: rfid
Source: "..\..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "GPII.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "GPII_USBListener.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "GPII_RFIDListener.ico"; DestDir: "{app}"; Flags: ignoreversion

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