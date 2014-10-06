[Setup]
AppName=GPII Listeners
AppVersion=1.4
DefaultDirName={pf}\GPII
DefaultGroupName=GPII
UninstallDisplayIcon={app}\MyProg.exe
OutputDir=..\bin
OutputBaseFilename=GPIIListenersSetup-1_4

AppPublisher=OpenDirective for RtF
AppCopyright=Copyright (C) 2014 OpenDirective
AppPublisherURL=http://opendirective.com
;SetupIconFile=GPII.ico

[Tasks]
Name: "usb"; Description: "Install the &USB Listener"; Flags: checkedonce checkablealone 
Name: "usb/turnkey"; Description: "Run the U&SB listener whenever this user logs on to Windows"; Flags: unchecked checkedonce
Name: "rfid"; Description: "Install the &RFID / NFC Listener"; Flags: checkedonce checkablealone 
Name: "rfid/turnkey"; Description: "Run the R&FID listener whenever this user logs on to Windows"; Flags: unchecked checkedonce
Name: "desktop"; Description: "Add icons to desktop"; Flags: unchecked  
Name: "startmenu"; Description: "Add icons to start menu"; Flags: unchecked  

[Files]
Source: "..\bin\Debug\GPII_USBListener.exe"; DestDir: "{app}"; Tasks: usb
Source: "..\bin\Debug\GPII_RFIDListener.exe"; DestDir: "{app}"; Tasks: rfid
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
Name: "{userdesktop}\GPII USB Listener"; Filename: "{app}\GPII_USBListener.exe"; Comment: "Run USB listener"; IconFilename: "{app}\GPII_USBListener.ico"; Tasks:desktop
Name: "{userdesktop}\GPII RFID Listener"; Filename: "{app}\GPII_RFIDListener.exe"; Comment: "Run RFID listener"; IconFilename: "{app}\GPII_RFIDListener.ico"; Tasks:desktop
Name: "{userdesktop}\GPII Website"; Filename: "http://GPII.net"; Comment: "Visit the GPII website"; IconFilename: "{app}\GPII.ico"; Tasks: desktop

; startup items - optional
Name: "{userstartup}\GPII USB Listener"; Filename: "{app}\GPII_USBListener.exe"; Comment: "Run USB listener"; IconFilename: "{app}\GPII_USBListener.ico"; Tasks:usb/turnkey and startmenu
Name: "{userstartup}\GPII RFID Listener"; Filename: "{app}\GPII_RFIDListener.exe"; Comment: "Run RFID listener"; IconFilename: "{app}\GPII_RFIDListener.ico"; Tasks:rfid/turnkey and startmenu

