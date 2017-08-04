///////////////////////////////////////////////////////////////////////////////
// GPII_RFIDListener.cpp
//
// Copyright 2014 University of Wisconsin, Madison
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
// The GPII RFID listener has the following features
// User Listener which includes device arrival and removal,
// a user hot key (ESC), a task bar icon and menu, and support
// for the arrival and removal of smart cards.
//
// Files needed for this project:
//
//		Project Files:
//			GPII_RFIDListener.cpp
//			WinSmartCard.cpp
//			WinSmartCard.h
//			README.txt // FIXME now ../README.md
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
//			winscard.h
//			winscard.lib
//
//		Optional MSVC Files:
//			GPII_RFIDListener.dsp
//			GPII_RFIDListener.dsw
//
// Output files:
//		GPII_RFIDListener.exe
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
//           Changed s to require user in .gpii-user-token.txt
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
#include <shellapi.h>
#include <FlowManager.h>
#include <Diagnostic.h>
#include "WinSmartCard.h"

// MinGW doesn't define _countof in stdlib
/* _countof helper */
#if !defined (_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif  /* !defined (_countof) */

//---------------------------------------------------------
// Global Constants
//---------------------------------------------------------
const char   MY_TITLE[]  = "GpiiUserListener 1.2";
const char   MY_CLASS[]  = "gpiiWindowClass";
const int    MY_SIZE_X   = 450;
const int    MY_SIZE_Y   = 100;
#define      WM_MYTRAY    (WM_USER + 1)
#define      MY_HOTKEY    (WM_USER + 2)
#define      MY_SHOWSTATUS (WM_USER + 3)
#define      MY_SHOWDIAG  (WM_USER + 4)
#define      MY_EXIT      (WM_USER + 5)
#define      MY_TIMER     (WM_USER + 7)

//---------------------------------------------------------
// Global Variables:
//---------------------------------------------------------
static const int MAX_BUFFER = 256;
static char m_szStatus[MAX_BUFFER];		// FIXME potential buffer overruns as restricted length string functions not used.
static char m_szLatestUserID[MAX_BUFFER];
static char m_szReader[MAX_BUFFER];
static int  m_nLogin = 0;

//---------------------------------------------------------
// Local Functions:
//---------------------------------------------------------
static ATOM		        MyRegisterClass(HINSTANCE hInstance);
static BOOL             MyTrayIcon(HWND hWnd);
static BOOL             MyPopupMenu(HWND hWnd);
static BOOL	    		InitInstance(HINSTANCE);
static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

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
    wsprintf(m_szLatestUserID,"%s","");
    wsprintf(m_szReader,"%s",lpCmdLine);
    wsprintf(m_szStatus,"%s","Listening...");

    //-----------------------------------------------------
    // Initialize the window
    //-----------------------------------------------------
    if (!MyRegisterClass(hInstance)) return FALSE;
    if (!Diagnostic_Init(hInstance)) return FALSE;
    if (!InitInstance(hInstance)) return FALSE;
#ifdef _DEBUG
    Diagnostic_Show(TRUE);
#endif

    //-----------------------------------------------------
    // Main message loop:
    //-----------------------------------------------------
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WinSmartCardCleanUp();

    return (int) msg.wParam; // FIXME which is correct?
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    Register our window class so we can create it
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
    wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION); // FIXME we have icon files so why not use them?
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
//   PURPOSE: Creates a tray icon with message WM_MYTRAY
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
    int ret;

    GetCursorPos(&p);
    HMENU hPopupMenu = CreatePopupMenu();
    const UINT fStatusChecked = (IsWindowVisible(hWnd)) ? MF_CHECKED : 0;
    InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING | fStatusChecked, MY_SHOWSTATUS, "View status window");
    const UINT fDiagChecked = (Diagnostic_IsShowing()) ? MF_CHECKED : 0;
    InsertMenu(hPopupMenu, 2, MF_BYPOSITION | MF_STRING | fDiagChecked, MY_SHOWDIAG, "View Diagnostic Window");
    InsertMenu(hPopupMenu, 4, MF_BYPOSITION | MF_STRING, MY_EXIT, "Exit");
    SetMenuItemBitmaps(hPopupMenu, MY_SHOWDIAG, MF_BYCOMMAND, NULL, NULL);
    SetForegroundWindow(hWnd);

    ret = TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x,p.y,0,hWnd, NULL);
    DestroyMenu(hPopupMenu);
    return ret;
}


static void _FindReader(HWND hWnd)
{
    if (WinSmartCardInitialize(hWnd, m_szReader))
    {
        // Update status
        wsprintf(m_szStatus, "%s", "Listening...");
        InvalidateRect(hWnd, NULL, TRUE);

        // Log
        char szReader[MAX_BUFFER];
        WinSmartCardGetReader(szReader, MAX_BUFFER);
        Diagnostic_LogString("Initialised reader", szReader);
    }
    else
    {
        // Update Status
        if (lstrlen(m_szReader))
            wsprintf(m_szStatus, "%s %s", m_szReader, " READER NOT FOUND");
        else
            wsprintf(m_szStatus, "%s", "NO CARD READERS FOUND");
    }
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

    _FindReader(hWnd);

    SetTimer(hWnd,MY_TIMER,2000,NULL);

#ifdef _DEBUG
    ShowWindow(hWnd, SW_SHOW);
#endif

    return TRUE;
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
        WinSmartCardGetUser(m_szLatestUserID, MAX_BUFFER);
        wsprintf(m_szStatus, "%s %s", "CARD ARRIVE", m_szLatestUserID);
        InvalidateRect(hWnd, NULL, TRUE);
        FlowManagerCardOn(m_szLatestUserID);
        m_nLogin = SMART_CARD_ARRIVE;
        break;

    //-----------------------------------------------------------
    // Smart Card Removed
    //-----------------------------------------------------------
    case SMART_CARD_REMOVE:
#ifdef WANT_REMOVE_EVENT
        FlowManagerCardOff(m_szLatestUserID);
#endif
        wsprintf(m_szStatus, "%s", "Listening...");
        InvalidateRect(hWnd,NULL,TRUE);
        break;

    //-----------------------------------------------------------
    // Smart Reader Stopped
    //-----------------------------------------------------------
    case SMART_READER_STOPPED:
        wsprintf(m_szStatus,"%s","CARD READER STOPPED");
        InvalidateRect(hWnd,NULL,TRUE);
        break;
        /*
        		//-----------------------------------------------------------
        		// A device has changed
        		//-----------------------------------------------------------
        		case WM_DEVICECHANGE:
        			return TRUE;
        			break;
        */

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
    // Tray Icon Menu Commands to Show, Hide, or Exit
    //-----------------------------------------------------------
    case WM_COMMAND:
    {
        const WORD cmd = LOWORD(wParam);

        if ( cmd == MY_SHOWSTATUS)
        {
            ShowWindow(hWnd, (IsWindowVisible(hWnd)) ? SW_HIDE : SW_SHOW);
        }
        else if (cmd == MY_EXIT)
        {
            DestroyWindow(hWnd);
        }
        if (cmd == MY_SHOWDIAG)
        {
            Diagnostic_Show(!Diagnostic_IsShowing());
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
        wchar_t wsStatus[MAX_BUFFER];
        wchar_t wsCurrent[MAX_BUFFER];
        WinSmartCardGetReader(szReader,MAX_BUFFER);
        rt.top = rt.bottom*10/100;
        (void) MultiByteToWideChar(CP_UTF8, 0, m_szStatus, -1, wsStatus, (int) _countof(wsStatus));
        DrawTextW(hdc, wsStatus, (int)wcslen(wsStatus), &rt, DT_CENTER);
        rt.top = rt.bottom*40/100;
        wsprintf(sReading,"%s %s %s",szReader,"READER",
                 WinSmartCardPolling() ? "ONLINE": "OFFLINE");
        DrawText(hdc, sReading, (int) strlen(sReading), &rt, DT_CENTER);
        rt.top = rt.bottom*70/100;
        wsprintf(sCurrent,"%s %s","LATEST USER:",
                 lstrlen(m_szLatestUserID) ? m_szLatestUserID : "NONE");
        (void) MultiByteToWideChar(CP_UTF8, 0, sCurrent, -1, wsCurrent, _countof(wsCurrent));
        DrawTextW(hdc, wsCurrent, (int)wcslen(wsCurrent), &rt, DT_CENTER);

        EndPaint(hWnd, &ps);
    }
    break;

    //-----------------------------------------------------------
    // Check to see if the card reader is polling
    //-----------------------------------------------------------
    case WM_TIMER:
        if (wParam == MY_TIMER && !WinSmartCardPolling())
        {
            _FindReader(hWnd);
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
        KillTimer(hWnd,MY_TIMER);
        Diagnostic_CleanUp();
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

