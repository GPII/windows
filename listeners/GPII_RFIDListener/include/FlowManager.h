///////////////////////////////////////////////////////////////////////////////
//
// FlowManager.h
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
// Access to the GPII flowmanager to log in and out
//
//	Project Files:
//		FlowManager.cpp
//		FlowManager.h
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _FLOWMANAGER_H_
#define _FLOWMANAGER_H_

// Also emit a "proximityRemoved" request.
//#define WANT_REMOVE_EVENT

void FlowManagerCardOn(const char * szToken);
#ifdef WANT_REMOVE_EVENT
void FlowManagerCardOff(const char *szToken);
#endif
void FlowManagerLogout(const char * szToken);

#endif // _FLOWMANAGER_H_
