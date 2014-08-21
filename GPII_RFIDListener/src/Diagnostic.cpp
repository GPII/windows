///////////////////////////////////////////////////////////////////////////////
//
// Diagnostic.cpp
//
// Access to the diagnostic window
//
//	Project Files:
//		Diagnostic.cpp
//		Diagnostic.h
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <windowsx.h>
#include <Strsafe.h>
#include <Diagnostic.h>


//---------------------------------------------------------
// Global Constants
//---------------------------------------------------------
static const char   MY_TITLE[] = "GpiiUserListener Diagnostic";
static const char   MY_CLASS[] = "gpiiDiagnosticClass";
static const int    MY_SIZE_X = 480;
static const int    MY_SIZE_Y = 450;
static const int    MY_FONT_HEIGHT = 16;

#define WM_MYLOG  WM_USER + 100

#define ID_EDIT 1000

//---------------------------------------------------------
// Global Variables
//---------------------------------------------------------
static HWND     g_hWnd = NULL;
static HFONT    g_hFont;

//---------------------------------------------------------
// Local Functions:
//---------------------------------------------------------
static ATOM		        _MyRegisterClass(HINSTANCE hInstance);
static LRESULT CALLBACK	_WndProc(HWND, UINT, WPARAM, LPARAM);

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
static ATOM _MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION); // FIXME we have icon files so why not use them?
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = MY_CLASS;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wcex);
}


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: Diagnostic_Init()
//
//  PURPOSE: Initialise the diagnostic window
//
//  COMMENTS:
//
//     Init from InitInstance().
//
///////////////////////////////////////////////////////////////////////////////
BOOL Diagnostic_Init(HINSTANCE hInstance)
{
    _MyRegisterClass(hInstance);

    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);

    HWND hWnd = CreateWindow(MY_CLASS, MY_TITLE, WS_OVERLAPPEDWINDOW | WS_SYSMENU,
        rc.right / 10, rc.bottom / 15,
        MY_SIZE_X, rc.bottom - 2 * (rc.bottom / 15), NULL, NULL, hInstance, NULL);

    if (!hWnd) 
        return FALSE;

    // Window style for the edit control.
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
        WS_BORDER | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY;

    // Create the edit control window.
    HWND hwndEdit = CreateWindowEx(
        0, TEXT("edit"),
        NULL,           // No Window title
        dwStyle,        // Window style
        0, 0, 0, 0,     // Set size in WM_SIZE of parent
        hWnd,           // Parent window
        (HMENU) ID_EDIT, // Control identifier
        hInstance,        // Instance handle
        NULL);

    if (!hwndEdit)
        return FALSE;

    // Select a non proportional font
    g_hFont = CreateFont(MY_FONT_HEIGHT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Courier New"));
    SetWindowFont(hwndEdit, g_hFont, FALSE);

    g_hWnd = hWnd;

    return TRUE;
}

void Diagnostic_CleanUp(void)
{
    DestroyWindow(g_hWnd);
}


void Diagnostic_Show(BOOL bShow)
{
    ShowWindow(g_hWnd, (bShow) ? SW_SHOW : SW_HIDE);

}

BOOL Diagnostic_IsShowing(void)
{
    return IsWindowVisible(g_hWnd);
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
LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    //-----------------------------------------------------------
    // Tray Icon Menu Commands to Show, Hide, Exit or Logout
    //-----------------------------------------------------------
    case WM_MYLOG:
    {
        LPCSTR pszStr = (LPCSTR)lParam;

        // Append text - note will eventually fill up
        const HWND hwndEdit = GetWindow(hWnd, GW_CHILD);
        int len = Edit_GetTextLength(hwndEdit);
        Edit_SetSel(hwndEdit, len, len);
        Edit_ReplaceSel(hwndEdit, pszStr);

        HANDLE hheap = GetProcessHeap();
        HeapFree(hheap, 0, (LPVOID)pszStr);
    }
    break;

    case WM_SETFOCUS:
    {
        const HWND hwndEdit = GetWindow(hWnd, GW_CHILD);
        SetFocus(hwndEdit);
        return 0;
    }

    case WM_SIZE:
        // Make the edit control the size of the window's client area. 
    {
        const HWND hwndEdit = GetWindow(hWnd, GW_CHILD);
        MoveWindow(hwndEdit,
            0, 0,                  // starting x- and y-coordinates 
            LOWORD(lParam),        // width of client area 
            HIWORD(lParam),        // height of client area 
            TRUE);                 // repaint window 
        return 0;
    }

    //-----------------------------------------------------------
    // Close [X] Hides the Window
    //-----------------------------------------------------------
    case WM_CLOSE:
    {
        const HWND hwndEdit = GetWindow(hWnd, GW_CHILD);
        Edit_SetText(hwndEdit, "");
    }
    break;

    //-----------------------------------------------------------
    // Cleanup resources
    //-----------------------------------------------------------
    case WM_DESTROY:
        DeleteObject(g_hFont); 
    break;
        
    //-----------------------------------------------------------
    // Default Window Proc
    //-----------------------------------------------------------
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

#define _countof(a) (sizeof(a)/sizeof(a[0]))


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: Diagnostic_LogString(LPCSTR pszPrefix, LPCSTR pszStr)
//
//  PURPOSE:  Cause a string to be logged in the daignostic window.
//
//  NOTES:    We PostMessage to be more thread safe. String is on Process Heap
//
///////////////////////////////////////////////////////////////////////////////
void Diagnostic_LogString(LPCSTR pszPrefix, LPCSTR pszString)
{
    pszPrefix = (pszPrefix) ? pszPrefix : "";
    pszString = (pszString) ? pszString : "";

    const STRSAFE_LPCSTR pszFormat = TEXT("%s: %s\r\n");

    HANDLE hheap = GetProcessHeap();
    SIZE_T cbHeap = strlen(pszPrefix) + strlen(pszString) + strlen(pszFormat);
    STRSAFE_LPSTR pszStrLog = (STRSAFE_LPSTR)HeapAlloc(hheap, 0, cbHeap);
    if (pszStrLog)
    {
        (void)StringCchPrintf(pszStrLog, cbHeap, pszFormat, pszPrefix, pszString);
        PostMessage(g_hWnd, WM_MYLOG, NULL, (LPARAM)pszStrLog);
    }
}

typedef const BYTE far            *LPCBYTE;

void Diagnostic_LogBlock(UINT uSector, UINT uBlock, LPBYTE pbBlock)
{
    static TCHAR pszPrefix[20];
    static TCHAR pszString[500];

    LPCTSTR pszFormatPrefix = TEXT("%.2hu-%.2hu");
    (void)StringCchPrintf(pszPrefix, _countof(pszPrefix), pszFormatPrefix, uSector, uBlock);

    LPCTSTR pszFormatString = TEXT("%.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX %.2hX ");
    (void)StringCchPrintf(pszString, _countof(pszString), pszFormatString, pbBlock[0], pbBlock[1], pbBlock[2], pbBlock[3], pbBlock[4], pbBlock[5], pbBlock[6], pbBlock[7], pbBlock[8], pbBlock[9], pbBlock[10], pbBlock[11], pbBlock[12], pbBlock[13], pbBlock[14], pbBlock[15]);

    Diagnostic_LogString(pszPrefix, pszString);
}
