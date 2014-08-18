///////////////////////////////////////////////////////////////////////////////
//
// FlowManager.cpp
//
// Access to the GPII flowmanager to log in and out
//
//	Project Files:
//		FlowManager.cpp
//		FlowManager.h
//
///////////////////////////////////////////////////////////////////////////////
#include <FlowManager.h>
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
const char * const FLOW_LOGIN = "login";
const char * const FLOW_LOGOUT = "logout";

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

#ifdef _DEBUG
            OutputDebugString(szRequest); // will show in VisualStudio and gdb
            OutputDebugString("\r\n");
#endif

#if defined(USE_FIDDLER)
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



void FlowManagerLogin(const char * szToken)
{
    _MakeCurlRequest(szToken, FLOW_LOGIN);
}

void FlowManagerLogout(const char * szToken) // FIXME should we keep state or can we have multiple concurrent logins?
{
    _MakeCurlRequest(szToken, FLOW_LOGOUT);
}

// End of file
