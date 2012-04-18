/*!
USB User Listener for Windows

Copyright 2012 Astea Solutions AD

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/windows/LICENSE.txt
*/

#include <curl/curl.h>
#include <dbt.h>
#include <fstream>
#include <map>
#include <windows.h>

const char actionLogin[]		= "login";
const char actionLogout[]		= "logout";
const char flowManagerUrl[]		= "http://localhost:8081/user/";
const char windowClassName[]	= "gpiiWindowClass";
const char windowTitle[]		= "GPII User Listener [USB]";

char tokenFilepath[] = "X:\\.gpii-user-token.txt";
std::map<char, std::string> tokenMap;

ATOM				RegisterWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK	WindowProc(HWND, UINT, WPARAM, LPARAM);
char				GetDriveLetter(unsigned long);
DWORD WINAPI		ReadToken(LPVOID);
void				CallFlowManager(const char *, const char *);
void				MakeGetRequest(const char *);

/**
 * Entry point for the application. Registers the window class, creates a window of that
 * class and starts a message loop for getting events fired to the window.
 *
 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms632680(v=vs.85).aspx
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hwnd;
    MSG Msg;

    if(!RegisterWindowClass(hInstance))
    {
    	// TODO Handle errors upon registration of the window class.
    	return 0;
    }

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, windowClassName, windowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        // TODO Handle errors upon creation of the Window class.
        return 0;
    }

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

/**
 * Registers a Window class - a prerequisite for creating windows of that class.
 *
 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms633574(v=vs.85).aspx
 */
ATOM RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wc;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&wc);
}

/**
 * The Window procedure of the application. Handles messages sent to the window.
 *
 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms633569(v=vs.85).aspx
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        case WM_DEVICECHANGE:
		{
			PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR) lParam;
			switch (wParam)
			{
				case DBT_DEVICEARRIVAL:
					if (pHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
					{
						PDEV_BROADCAST_VOLUME pVol = (PDEV_BROADCAST_VOLUME) pHdr;
						if (!(pVol->dbcv_flags & DBTF_MEDIA)) // Handle USB only, exclude DVD/CDs
						{
							char driveLetter = GetDriveLetter(pVol->dbcv_unitmask);
							DWORD ThreadId = 0;
							CreateThread (0, 0, ReadToken, (LPVOID)driveLetter, 0, &ThreadId);
							// TODO Handle errors upon thread creation.
						}
					}
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					if (pHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
					{
						PDEV_BROADCAST_VOLUME pVol = (PDEV_BROADCAST_VOLUME) pHdr;
						if (!(pVol->dbcv_flags & DBTF_MEDIA)) // Handle USB only, exclude DVD/CDs
						{
							char cDriveLetter = GetDriveLetter(pVol->dbcv_unitmask);
							const char * token = tokenMap[cDriveLetter].c_str();
							//tokenMap.erase(cDriveLetter);
							CallFlowManager(token, actionLogout);
						}
					}
					break;
			}
		}
		break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

/**
 * Gets the drive letter of a volume based on its unit mask.
 *
 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa363249(v=vs.85).aspx
 */
char GetDriveLetter(unsigned long ulUnitMask)
{
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

/**
 * Reads the token form the USB drive inserted and calls the GPII FlowManager,
 * notifying it that a user with the specified token has logged in.
 */
DWORD WINAPI ReadToken(LPVOID params)
{
	char driveLetter = (char)params;
	tokenFilepath[0]=driveLetter;

	std::ifstream tokenFile(tokenFilepath);
	if (tokenFile.is_open())
	{
		char token[256];
		tokenFile.getline(token, 256);
		tokenMap.insert(std::pair<char, std::string>(driveLetter, token));
		CallFlowManager(token, actionLogin);
		tokenFile.close();
	}
	// TODO Handle errors when opening the file.
	return 0;
}

/**
 * Calls the GPII Flow Manager with a user token and action (login or logout).
 */
void CallFlowManager(const char * token, const char * action)
{
	char url[256];
	strcpy(url, flowManagerUrl);
	strcat(url, token);
	strcat(url, "/");
	strcat(url, action);
	MakeGetRequest(url);
}

/**
 * Uses libcurl to make a HTTP GET request to a specified URL.
 */
void MakeGetRequest(const char *url)
{
	CURL *curl;
	CURLcode responseCode;
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		responseCode = curl_easy_perform(curl);
		// TODO Check the response code and handle errors.
		curl_easy_cleanup(curl);
	}
}
