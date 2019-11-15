#include "pch.h"
#include <Windows.h>

#include <utility>

#include "PayloadProc.h"

using std::wstring;
using std::vector;
using std::pair;

#pragma comment (lib, "WindowsApp.lib")

[System::STAThread]
int wmain(int argc, wchar_t* argv[]) {
    pair<int, wchar_t**> payload { argc, argv };
    DWORD threadID { 0 };
    HANDLE thHandle { NULL };

    thHandle = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)handlePayload,
        (LPVOID)&payload,
        0,
        &threadID
    );

    WaitForSingleObject(thHandle, INFINITE);

    return 0;
}
