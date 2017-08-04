///////////////////////////////////////////////////////////////////////////////
//
// FlowManager.cpp
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
#include <FlowManager.h>
#include <Diagnostic.h>
#include <libcurl\curl.h>

#ifdef __MINGW_H
// not included in mingw headers
extern void WINAPI OutputDebugString(
    LPCTSTR lpOutputString
);
#endif

// Set TRUE to use fiddler2.com to debug http
#if defined(_DEBUG)
#define USE_FIDDLER	FALSE
#endif

//---------------------------------------------------------
// Flow Manager Constants
//---------------------------------------------------------
const char * const FLOW_MANAGER_URL = "http://localhost:8081/user";
const char * const FLOW_CARDON = "proximityTriggered";
const char * const FLOW_CARDOFF = "proximityRemoved";

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: MakeCurlRequest(char szUser,char szAction)
//
//  PURPOSE:  Uses libcurl to make a HTTP GET request to a specified URL.
//
//  EXAMPLE:
//
//            http://localhost:8081/user/123/proximityTriggered
//
///////////////////////////////////////////////////////////////////////////////
static int _MakeCurlRequest(const char* szUser, const char* szAction)
{
    static const int MAX_BUFFER = 256;

    if (lstrlen(szUser) > 0)
    {
        CURL *curl = curl_easy_init();
        if (curl)
        {
            char szRequest[MAX_BUFFER];
            char * szUserEscaped = curl_easy_escape(curl, szUser, 0);
            wsprintf(szRequest,"%s/%s/%s",FLOW_MANAGER_URL,szUserEscaped,szAction);
            Diagnostic_LogString("Token", szUser); // FIXME is ocaisionally getting printed twice but code doesn't seem to reach here twice
            Diagnostic_LogString("Action", szAction); // FIXME is ocaisionally getting printed twice but code doesn't seem to reach here twice
            Diagnostic_LogString("FlowManger URL", szRequest);

#if USE_FIDDLER == TRUE
            (void) curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888"); // use http://fiddler2.com to monitor HTTP
#endif
            (void) curl_easy_setopt(curl, CURLOPT_URL, szRequest);
            // TODO Check the response code and handle errors.
            CURLcode responseCode = curl_easy_perform(curl); // expect CURLE_WRITE_ERROR as no buffer given for incoming data
            (void) responseCode; // tell compiler we're not doing anything with it - just for debugging
            curl_free(szUserEscaped);
            curl_easy_cleanup(curl);
            return 1;
        }
    }
    return 0;
}

void FlowManagerCardOn(const char *szToken)
{
    _MakeCurlRequest(szToken, FLOW_CARDON);
}

#ifdef WANT_REMOVE_EVENT
void FlowManagerCardOff(const char *szToken)
{
    _MakeCurlRequest(szToken, FLOW_CARDOFF);
}
#endif

// End of file
