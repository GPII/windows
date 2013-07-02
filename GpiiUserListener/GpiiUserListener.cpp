///////////////////////////////////////////////////////////////////////////////
// GpiiUserListener.cpp
//
// This file is used to test various features of an updated
// User Listener which includes USB device arrival and removal,
// a user hot key (ESC), a task bar icon and menu, and support
// for the arrival and removal of smart cards.
//
// Files needed for this project:
//
//		Project Files:
//			NfcUserTest.cpp		//FIXME appears to missing from the source
//			WinSmartCard.cpp
//			WinSmartCard.H
//
//		CURL Library Files:
//			curl.h
//			curlbuild.h
//			curlrules.h
//			curlver.h
//			easy.h
//			libcurl_imp.lib
//			libcurld_imp.lib
//			multi.h
//
//		WINSCARD Libary Files:
//			WINSCARD.H
//			Winscard.lib
//
//		Optional MSVC Files:
//			GpiiUserListener.dsp
//			GpiiUserListener.dsw
//
// Output files:
//		GpiiUserListener.exe
//		libcurl.dll
//
// Versions:
//
//    2012.05.05 Version 1.00
//    2012.05.06 Version 1.01 
//           Changed WM_USER messages from const int to #define
//           Removed unused responseCode variable
//           Added m_cUserDrive to check correct logout drive
//           Changed #include so "curl.h" files in same dir as this file
//           Changed curl library to libcurld_imp.lib
//           Compiled and added curl files from Curl 7.21.7
//    2012.05.06 Version 1.02 
//           Changed operation so card remove does not logout user
//           Touching the card a second time will logout user
//           Changed ESC logout to F11 logout
//    2012.05.06 Version 1.03 
//           Changed USB to require user in .gpii-user-token.txt
//           Set window to SW_HIDE, except for _DEBUG it is SW_SHOW
//           Removed MY_READER="ACS ACR122 0"
//           Change reader to be set by program command line argument
//               Example: GpiiUserListener.exe ACS ACR122 0
//               With no arguments, the first reader found is used
//           Changed WS_OVERLAPPEDWINDOW to WS_OVERLAPPED
//           Changed WM_CLOSE to Hide the Window but not Exit Program
//           Added Display to Show Current User and Reader
//    2012.05.07 Version 1.04
//           Added GpiiUserListener.sln file for Visual Studio 2010 
//    2012.05.12 Version 1.05
//           Changed authentication to key b
//    2012.05.12 Version 1.06
//           Changed so new card will automatically log out old user
//    2013.06.27 version 1.12
//			 Rebuilt from 7.25 curl source in VS2012 and reorganised curl files into own folder
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <dbt.h>
#include <shellapi.h>
#include "WinSmartCard.h"

// Set TRUE to use fiddler2.com tp dubug http
#if defined(_DEBUG)
  #define USE_FIDDLER	TRUE
#endif

//---------------------------------------------------------
// Let the compiler know we are including the CURL library
//---------------------------------------------------------

#define CURL_STATICLIB TRUE		// use staic linked version of libcurl
#include "libcurl\curl.h"

//---------------------------------------------------------
// Flow Manager Constants
//---------------------------------------------------------
const char GPII_USER_FILE[]   = ".gpii-user-token.txt";
const char FLOW_MANAGER_URL[] = "http://localhost:8081/user";
const char FLOW_LOGIN[] = "login";
const char FLOW_LOGOUT[] = "logout";

//---------------------------------------------------------
// Global Constants
//---------------------------------------------------------
const char   MY_TITLE[]  = "GpiiUserListener 1.12";
const char   MY_CLASS[]  = "gpiiWindowClass";
const int    MY_SIZE_X   = 450;
const int    MY_SIZE_Y   = 100;
#define      WM_MYTRAY    (WM_USER + 1)
#define      MY_HOTKEY    (WM_USER + 2)
#define      MY_SHOW      (WM_USER + 3)
#define      MY_HIDE      (WM_USER + 4)
#define      MY_EXIT      (WM_USER + 5)
#define      MY_LOGOUT    (WM_USER + 6)
#define      MY_TIMER     (WM_USER + 7)

//---------------------------------------------------------
// Global Variables:
//---------------------------------------------------------
const int MAX_BUFFER = 256;
static char m_szStatus[MAX_BUFFER];
static char m_szUserID[MAX_BUFFER];
static char m_szReader[MAX_BUFFER];
static char m_cUserDrive = 0;
static int  m_nLogin = 0;

//---------------------------------------------------------
// Lcal Functions:
//---------------------------------------------------------
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL                MyTrayIcon(HWND hWnd);
BOOL                MyPopupMenu(HWND hWnd);
BOOL				InitInstance(HINSTANCE);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
int                 MakeCurlRequest(char szUser,char szAction);
int                 UsbGetDriveLetter(LPARAM lParam);

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: WinMain()
//
//  PURPOSE: Program entry point
//
//  COMMENTS:
//
//    The user-provided entry point for a graphical Windows-based 
//    application. WinMain is the conventional name used for the
//    application entry point
//
///////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     lpCmdLine,
                     int       /*nCmdShow*/)
{
	MSG msg;

	//-----------------------------------------------------
	// Initialize global variables
	//-----------------------------------------------------
	wsprintf(m_szUserID,"%s","");
	wsprintf(m_szReader,"%s",lpCmdLine);
	wsprintf(m_szStatus,"%s","Listening...");

	//-----------------------------------------------------
	// Initialize the window
	//-----------------------------------------------------
	if (!MyRegisterClass(hInstance)) return FALSE;
	if (!InitInstance (hInstance)) return FALSE;

	//-----------------------------------------------------
	// Main message loop:
	//-----------------------------------------------------
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
///////////////////////////////////////////////////////////////////////////////
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MY_CLASS;
	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&wcex);
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: MyTrayIcon(HWND hWnd)
//
//   PURPOSE: Creates a tray icons with message WM_MYTRAY
//
//   COMMENTS:
//
///////////////////////////////////////////////////////////////////////////////
BOOL MyTrayIcon(HWND hWnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 100;
	nid.uCallbackMessage = WM_MYTRAY;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wsprintf(nid.szTip,"%s",MY_TITLE);
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	return Shell_NotifyIcon(NIM_ADD, &nid);
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: MyPopupMenu()
//
//   PURPOSE: Creates a popup menu
//
//   COMMENTS:
//
///////////////////////////////////////////////////////////////////////////////
BOOL MyPopupMenu(HWND hWnd)
{
    POINT p;
	GetCursorPos(&p);
	HMENU hPopupMenu = CreatePopupMenu();
    InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MY_SHOW, "Show Window");
    InsertMenu(hPopupMenu, 1, MF_BYPOSITION | MF_STRING, MY_HIDE, "Hide Window");
    InsertMenu(hPopupMenu, 2, MF_BYPOSITION | MF_STRING, MY_LOGOUT,"Logout");
    InsertMenu(hPopupMenu, 3, MF_BYPOSITION | MF_STRING, MY_EXIT, "Exit");
    SetForegroundWindow(hWnd);
    return TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x,p.y,0,hWnd, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: InitInstance(HANDLE)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global 
//        variable and create and display the main program window.
//
///////////////////////////////////////////////////////////////////////////////
BOOL InitInstance(HINSTANCE hInstance)
{
	HWND hWnd;
	RECT rc;

	GetWindowRect(GetDesktopWindow(),&rc);

	hWnd = CreateWindow(MY_CLASS, MY_TITLE,WS_OVERLAPPED|WS_SYSMENU,
					    rc.right/2-MY_SIZE_X/2,rc.bottom/3,
						MY_SIZE_X,MY_SIZE_Y,NULL,NULL,hInstance,NULL);

	if (!hWnd) return FALSE;

	MyTrayIcon(hWnd);
#if !defined(_DEBUG)
	RegisterHotKey(hWnd,MY_HOTKEY,0,VK_F11);
#endif //_DEBUG

	if (WinSmartCardInitialize(hWnd,m_szReader) == FALSE)
	{
		if (lstrlen(m_szReader))
			wsprintf(m_szStatus,"%s %s",m_szReader," READER NOT FOUND");
		else
			wsprintf(m_szStatus,"%s","NO CARD READERS FOUND");
	}

#ifdef _DEBUG
	ShowWindow(hWnd, SW_SHOW);
	WinSmartCardShowError();
#endif

	InvalidateRect(hWnd,NULL,TRUE);
	SetTimer(hWnd,MY_TIMER,2000,NULL);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: MakeCurlRequest(char szUser,char szAction)
//
//  PURPOSE:  Uses libcurl to make a HTTP GET request to a specified URL.
//
//  EXAMPLES:
//
//            http://localhost:8081/user/123/login
// 
//            http://localhost:8081/user/123/logout
//
///////////////////////////////////////////////////////////////////////////////
int MakeCurlRequest(const char* szUser,const char* szAction)
{
	if (lstrlen(szUser) > 0)
	{
		char szRequest[MAX_BUFFER];
		wsprintf(szRequest,"%s/%s/%s",FLOW_MANAGER_URL,szUser,szAction);

		CURL *curl = curl_easy_init();
		if (curl)
		{
#if defined(USE_FIDDLER)
			(void) curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888"); // use http://fiddler2.com to monitor HTTP
#endif
			(void) curl_easy_setopt(curl, CURLOPT_URL, szRequest);
			// TODO Check the response code and handle errors.
			CURLcode responseCode = curl_easy_perform(curl); // expect CURLE_WRITE_ERROR as no buffer given for incoming data
			curl_easy_cleanup(curl);
			return 1;
		}
	}
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: UsbDriveGetUser(DWORD dwUnitMask)
//
//  PURPOSE:  Reads the user from a file on a USB drive
//
///////////////////////////////////////////////////////////////////////////////
int UsbDriveGetUser(const char cDrive,char* szUser)
{
	char szPath[MAX_BUFFER];

	wsprintf(szPath,"%c:\\%s",cDrive,GPII_USER_FILE);

	FILE* hFile = fopen(szPath,"r");
	if (hFile != NULL)
	{
		fgets(szUser,MAX_BUFFER,hFile);
		fclose(hFile);
	}
	else
	{
		wsprintf(szUser,"%s","");
	}

	return lstrlen(szUser);
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: UsbGetDriveLetter(LPARAM lParam)
//
//  PURPOSE:  Converts lParam into a USB drive letter if needed.
//            CD, DVD, etc. are ignored and the function returns 0.
//
///////////////////////////////////////////////////////////////////////////////
int UsbGetDriveLetter(LPARAM lParam)
{
	if (!lParam) return 0;

	PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
	if (pHdr == NULL) return 0;
	if (pHdr->dbch_devicetype != DBT_DEVTYP_VOLUME) return 0;

    PDEV_BROADCAST_VOLUME pVol = (PDEV_BROADCAST_VOLUME) pHdr;
	if (pVol == NULL) return 0;
	if ((pVol->dbcv_flags & DBTF_MEDIA)) return 0;

	unsigned long ulUnitMask = pVol->dbcv_unitmask;

    char c;
    for (c = 0; c < 26; c++)
    {
        if (ulUnitMask & 0x01)
        {
            break;
        }
        ulUnitMask = ulUnitMask >> 1;
    }

    return (c + 'A');
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//  WM_MYTRAY	- a mouse command on the tray icon
//  WM_HOTKEY	- the user defined hotkey was pressed
//  WM_DEVICECHANGE - a change to a device on this pc
//
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		//-----------------------------------------------------------
		// Smart Card User Login
		//-----------------------------------------------------------
		case SMART_CARD_ARRIVE:
			if (m_nLogin == 0)
			{
				//-----------------------------------------------
				// login a new user
				//-----------------------------------------------
				WinSmartCardGetUser(m_szUserID,MAX_BUFFER);
				wsprintf(m_szStatus,"%s %s","CARD LOGIN",m_szUserID);
				InvalidateRect(hWnd,NULL,TRUE);
				MakeCurlRequest(m_szUserID,FLOW_LOGIN);
				m_nLogin = SMART_CARD_ARRIVE;
			}
			else
			{
				char szThisUser[MAX_BUFFER];
				WinSmartCardGetUser(szThisUser,MAX_BUFFER);
				if (lstrcmp(szThisUser,m_szUserID) == 0)
				{
					//-----------------------------------------------
					// logout the user
					//-----------------------------------------------
					wsprintf(m_szStatus,"%s %s","CARD LOGOUT",m_szUserID);
					MakeCurlRequest(m_szUserID,FLOW_LOGOUT);
					wsprintf(m_szUserID,"%s","");
					InvalidateRect(hWnd,NULL,TRUE);
					m_nLogin = 0;
				}
				else
				{
					//-----------------------------------------------
					// logout the old user and login new user
					//-----------------------------------------------
					MakeCurlRequest(m_szUserID,FLOW_LOGOUT);
					WinSmartCardGetUser(m_szUserID,MAX_BUFFER);
					MakeCurlRequest(m_szUserID,FLOW_LOGIN);
					wsprintf(m_szStatus,"%s %s","CARD LOGIN",m_szUserID);
					InvalidateRect(hWnd,NULL,TRUE);
				}
			}
			break;

		//-----------------------------------------------------------
		// Smart Card Removed
		//-----------------------------------------------------------
		case SMART_CARD_REMOVE:
#ifdef _DEBUG
			wsprintf(m_szStatus,"%s","CARD REMOVED");
			InvalidateRect(hWnd,NULL,TRUE);
#endif
			break;
			
		//-----------------------------------------------------------
		// Smart Reader Stopped
		//-----------------------------------------------------------
		case SMART_READER_STOPPED:
			wsprintf(m_szStatus,"%s","CARD READER STOPPED");
			InvalidateRect(hWnd,NULL,TRUE);
			break;

		//-----------------------------------------------------------
		// USB Login and Logout
		//-----------------------------------------------------------
		case WM_DEVICECHANGE:
			{
				char cDrive = (char)UsbGetDriveLetter(lParam);

				if (wParam == DBT_DEVICEARRIVAL && m_nLogin == 0)
				{
					m_cUserDrive = cDrive;
                    if (UsbDriveGetUser(cDrive,m_szUserID))
					{
						wsprintf(m_szStatus,"%s %s","USB LOGIN",m_szUserID);
						InvalidateRect(hWnd,NULL,TRUE);
						MakeCurlRequest(m_szUserID,FLOW_LOGIN);
						m_nLogin = DBT_DEVICEARRIVAL;
					}
				}
				else if (wParam == DBT_DEVICEREMOVECOMPLETE && 
						 m_nLogin == DBT_DEVICEARRIVAL && m_cUserDrive == cDrive)
				{
					wsprintf(m_szStatus,"%s","USB LOGOUT");
					MakeCurlRequest(m_szUserID,FLOW_LOGOUT);
					wsprintf(m_szUserID,"%s","");
					InvalidateRect(hWnd,NULL,TRUE);
					m_nLogin = 0;
				}
			}
			break;

#if !defined(_DEBUG)

		//-----------------------------------------------------------
		// Hotkey Logout
		//-----------------------------------------------------------
		case WM_HOTKEY:
			if (wParam == MY_HOTKEY && m_nLogin)
			{
				wsprintf(m_szStatus,"%s","F11 LOGOUT");
				MakeCurlRequest(m_szUserID,FLOW_LOGOUT);
				wsprintf(m_szUserID,"%s","");
				InvalidateRect(hWnd,NULL,TRUE);
				m_nLogin = 0;
			}
			break;
#endif // _DEBUG

		//-----------------------------------------------------------
		// Button Click on Tray Icon
		//-----------------------------------------------------------
		case WM_MYTRAY:
			if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN)
			{
				MyPopupMenu(hWnd);
			}
			break;

		//-----------------------------------------------------------
		// Tray Icon Menu Commands to Show, Hide, Exit or Logout
		//-----------------------------------------------------------
		case WM_COMMAND:
			{
				if (LOWORD(wParam) == MY_SHOW)
				{
					ShowWindow(hWnd,SW_SHOW);
				}
				else if (LOWORD(wParam) == MY_HIDE)
				{
					ShowWindow(hWnd,SW_HIDE);
				}
				else if (LOWORD(wParam) == MY_EXIT)
				{
					DestroyWindow(hWnd);
				}
				else if (LOWORD(wParam) == MY_LOGOUT)
				{
					if (m_nLogin)
					{
						wsprintf(m_szStatus,"%s","MENU LOGOUT");
						MakeCurlRequest(m_szUserID,FLOW_LOGOUT);
						wsprintf(m_szUserID,"%s","");
						m_nLogin = 0;
					}
					else
					{
						wsprintf(m_szStatus,"%s","NO USER TO LOGOUT");
					}
					InvalidateRect(hWnd,NULL,TRUE);
				}
			}
			break;

		//-----------------------------------------------------------
		// Paint the Current Status String
		//-----------------------------------------------------------
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc;
				hdc = BeginPaint(hWnd, &ps);
				RECT rt;
				GetClientRect(hWnd, &rt);
				char szReader[MAX_BUFFER];
				char sReading[MAX_BUFFER];
				char sCurrent[MAX_BUFFER];
				WinSmartCardGetReader(szReader,MAX_BUFFER);
				rt.top = rt.bottom*10/100;
				DrawText(hdc, m_szStatus, strlen(m_szStatus), &rt, DT_CENTER);
				rt.top = rt.bottom*40/100;
				wsprintf(sReading,"%s %s %s",szReader,"READER",
						 WinSmartCardPolling() ? "ONLINE": "OFFLINE");
				DrawText(hdc, sReading, strlen(sReading), &rt, DT_CENTER);
				rt.top = rt.bottom*70/100;
				wsprintf(sCurrent,"%s %s","CURRENT USER:",
						 lstrlen(m_szUserID) ? m_szUserID : "NONE");
				DrawText(hdc, sCurrent, strlen(sCurrent), &rt, DT_CENTER);


				EndPaint(hWnd, &ps);
			}
			break;

		//-----------------------------------------------------------
		// Check to see if the card reader is polling
		//-----------------------------------------------------------
		case WM_TIMER:
			if (wParam == MY_TIMER && !WinSmartCardPolling())
			{
				if (WinSmartCardInitialize(hWnd,m_szReader))
				{
					wsprintf(m_szStatus,"%s","Listening...");
					InvalidateRect(hWnd,NULL,TRUE);
				}
			}
			break;

		//-----------------------------------------------------------
		// Close [X] Hides the Window
		//-----------------------------------------------------------
		case WM_CLOSE:
			ShowWindow(hWnd,SW_HIDE);
#ifdef _DEBUG
			DestroyWindow(hWnd);
#endif
			break;

		//-----------------------------------------------------------
		// Destroy the Window and Exit
		//-----------------------------------------------------------
		case WM_DESTROY:
			if (m_nLogin) MakeCurlRequest(m_szUserID,FLOW_LOGOUT);
			KillTimer(hWnd,MY_TIMER);
			PostQuitMessage(0);
			break;

		//-----------------------------------------------------------
		// Default Window Proc
		//-----------------------------------------------------------
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

