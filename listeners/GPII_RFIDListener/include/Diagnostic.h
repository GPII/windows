///////////////////////////////////////////////////////////////////////////////
//
// Diagnostic.h
//
// Copyright 2014 OpenDirective Ltd.
//
// Licensed under the New BSD license. You may not use this file except in
// compliance with this License.
//
// You may obtain a copy of the License at
// https://github.com/gpii/windows/blob/master/LICENSE.txt
//
// Access to the diagnostic information display
//
//	Project Files:
//		FlowManager.cpp
//		FlowManager.h
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _DIAGNOSTIC_H_
#define _DIAGNOSTIC_H_

#include <windows.h>

// Windows control
extern BOOL Diagnostic_Init(HINSTANCE hInstance);
extern void Diagnostic_CleanUp(void);
extern void Diagnostic_Show(BOOL bShow);
extern BOOL Diagnostic_IsShowing(void);

//Logging
typedef const BYTE far            *LPCBYTE;     // Strangley missing form windows portability types

extern void Diagnostic_PrintHexBytes(LPTSTR pszDest, size_t cchDest, LPCBYTE pBytes, size_t cBytes);
extern void Diagnostic_PrintCharBytes(LPTSTR pszDest, size_t cchDest, LPCBYTE pBytes, size_t cBytes);
extern void Diagnostic_LogString(LPCSTR pszPrefix, LPCSTR pszStr);
extern void Diagnostic_LogHexBlock(UINT uSector, UINT uBlock, LPCBYTE pbBlock, size_t cBytes);

#endif // _DIAGNOSTIC_H_