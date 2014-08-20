///////////////////////////////////////////////////////////////////////////////
//
// Diagnostic.h
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

extern BOOL Diagnostic_Init(HINSTANCE hInstance);
extern void Diagnostic_Show(BOOL bShow);
extern BOOL Diagnostic_IsShowing(void);
extern void Diagnostic_LogString(LPCSTR pszPrefix, LPCSTR pszStr);
extern void Diagnostic_LogBlock(UINT uSector, UINT uBlock, LPBYTE pbBlock);

#endif // _DIAGNOSTIC_H_