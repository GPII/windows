///////////////////////////////////////////////////////////////////////////////
//
// WinSmartCard.cpp
//
// This file simplifies the Winscard interface by allowing the calling
// function to initialize a card read. This file will then start a new
// thread to test for card connections and disconnections. Each event
// will send a SMART_CARD_ARRIVE and SMART_CARD_REMOVE message to the
// main windows proc (WndProc). Note that WM_USER + 101 and 102 must
// not be used by that same main window.
//
// Note: This code was tested with reader "ACS ACR122 0"
//
//	Project Files:
//		WinSmartCard.cpp
//		WinSmartCard.H
//
//	WINSCARD Libary Files:
//		WINSCARD.H
//		Winscard.lib
//
// Versions:
//
//	  2012.05.05 Version 1.00
//	  2012.05.06 Version 1.01 
//           Changed WinSmartCardErrorString(DWORD code) to DWORD
//           Changed "Find the Desired Reader" to removed MinGW warning
//	  2012.05.06 Version 1.02 
//           Increase speed of polling but added longer delay after read
//	  2012.05.06 Version 1.03 
//           Added better support for hot swap and SCardReleaseContext
//           Changed WinSmartCardInitialize to not require a reader name
//    2012.05.12 Version 1.05
//           Changed authentication to key b
//    2012.05.13 Version 1.07
//           Changed user block to #4 and check for gototags data
//           Changed authenticate to check both key types A and B
//           Replaced all sprintf functions with lstrcpyn
//    2012.05.13 Version 1.11
//           Encapulated Apdu Authenticate and Read Record functions.
///////////////////////////////////////////////////////////////////////////////
#define STRICT
#include <windows.h>
#include <process.h>
#ifdef __MINGW_H
#define DEVICE_TYPE_SMARTCARD // stop redefinition as already defined in winioctrl.h
#endif
#include "winscard.h"       // not included in MinGW so have local copy
#include "WinSmartCard.h"

// FIXME Using windows::networking::proximity may simplify all this low level hackery

//-------------------------------------------------------------------
//  Application Protocol Data Unit (APDU) Constants
//-------------------------------------------------------------------
#define APDU_CLA_RFU                   0xFF
#define APDU_INS_GENERAL_AUTHENTICATE  0x86
#define APDU_INS_READ_BINARY           0xB0
#define APDU_KEY_TYPE_A                0x60
#define APDU_KEY_TYPE_B                0x61
#define NFC_WELL_KNOWN_RECORD          0xD1
#define NFC_RECORD_LENGTH_OFFSET       0x02
#define NFC_EXTRA_BYTES                0x03
#define NFC_DATA_OFFSET                0x07

//-------------------------------------------------------------------
// Constants For User Data
//-------------------------------------------------------------------
#define USER_BLOCK                     0x04  // user data block
#define USER_BYTES                       16  // user data bytes
#define USER_KEY                       0x00  // user auth key store

//-------------------------------------------------------------------
// Constants For This File
//-------------------------------------------------------------------
#define   POLLING_DELAY                 250  // polling delay in ms
#define   NO_READERS_FOUND             1001  // error finding readers
#define   READER_NOT_FOUND             1002  // error finding user reader
#define   POLLING_THREAD_FAILED        1003  // cannot start thread
#define   MAX_BUFFER                    256  // maximum string size

//-------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------
int          m_retCode = 0;
int          m_nReaderIndex = 0;
BOOL         m_bPolling = 0;
HWND         m_hWnd = 0;
HANDLE       m_hThread;
SCARDCONTEXT m_hContext = 0;
SCARDHANDLE  m_hCard = 0;
DWORD        m_dwProtocol = 0;
char         m_szReader[MAX_BUFFER];
char         m_szUser[MAX_BUFFER];
DWORD        m_dwTotalBytes = 0;

//-------------------------------------------------------------------
// Structure Used to Cleanup on Exit
//-------------------------------------------------------------------
struct Cleanup
{
     Cleanup() {}
    ~Cleanup()
	{
		if (m_bPolling)
		{
			m_bPolling = 0;
			WaitForSingleObject(m_hThread,3000);
			SCardDisconnect(m_hCard, SCARD_LEAVE_CARD);
			SCardReleaseContext(m_hContext);
		}
	}
} clean_exit;

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: ApduReadBlock()
//
//   PURPOSE:  Reads 16 bytes of data from block number byBlock into pRecv.
//
//   COMMENTS: Returns 16 (bytes read) if successfull or 0 for a failure.
//
///////////////////////////////////////////////////////////////////////////////
int ApduReadBlock(SCARDHANDLE  hCard,DWORD dwProtocol,BYTE byBlock,BYTE* pRecv)
{
	BYTE read_block[] =  { 0xFF, 0xB0, 0x00, 0x04, 0x10 };  // FIXME const
	read_block[3] = byBlock;

	DWORD dwRecvLen = 18;
	SCARD_IO_REQUEST io_req;
	io_req.cbPciLength = sizeof(io_req);
	io_req.dwProtocol = dwProtocol;

	if (SCardTransmit(hCard,&io_req,read_block,sizeof(read_block),
	    &io_req,pRecv,&dwRecvLen) != SCARD_S_SUCCESS) return 0;

	return (dwRecvLen - 2);
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: ApduAuthenticate()
//
//   PURPOSE:  Attempt to authenticate with key a and then key b.
//
//   COMMENTS: Returns 1 if successfull or 0 for a failure.
//
///////////////////////////////////////////////////////////////////////////////
int ApduAuthenticate(SCARDHANDLE hCard,DWORD dwProtocol)
{
	BYTE auth_key_a[] = { 0xFF, 0x86, 0x00, 0x00, 0x05, 0x01, 0x00, 0x04, 0x60, 0x00 }; // FIXME const
	BYTE auth_key_b[] = { 0xFF, 0x86, 0x00, 0x00, 0x05, 0x01, 0x00, 0x04, 0x61, 0x00 };

	BYTE pRecv[18];
	DWORD dwRecvLen = 2;
	SCARD_IO_REQUEST io_req;
	io_req.cbPciLength = sizeof(io_req);
	io_req.dwProtocol = dwProtocol;

	if (SCardTransmit(hCard,&io_req,auth_key_a,sizeof(auth_key_a),
	    &io_req,pRecv,&dwRecvLen) != SCARD_S_SUCCESS) return 0;

	if (ApduReadBlock(hCard,dwProtocol,4,pRecv) == 16) return 1;

	if (SCardTransmit(hCard,&io_req,auth_key_b,sizeof(auth_key_b),
	    &io_req,pRecv,&dwRecvLen) != SCARD_S_SUCCESS) return 0;

	if (ApduReadBlock(hCard,dwProtocol,4,pRecv) == 16) return 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: ApduAppendAsciiRecord()
//
//   PURPOSE:  Append the Ascii data from pSrc to pDst.
//             If pDst has no data, look for the record start 0xD1.
//
//   COMMENTS: Returns 1 if all bytes are ascii, or 0 if end of data.
//
///////////////////////////////////////////////////////////////////////////////
int ApduAppendAsciiRecord(char* pDst,BYTE* pSrc)
{
	const BYTE WELL_KNOWN_RECORD = 0xD1;
	const BYTE TEXT_DATA_OFFSET  = 0x07;
	const BYTE ASCII_MIN         = 0x20;
	const BYTE ASCII_MAX         = 0x7E;
	const BYTE BLOCK_BYTES       = 16;

	int nLen = strlen(pDst);

	for (int i=0;i<BLOCK_BYTES;i++)
	{
		if (nLen == 0)
		{
			if (pSrc[i] == WELL_KNOWN_RECORD &&
			    i < TEXT_DATA_OFFSET)
			{
				i += TEXT_DATA_OFFSET;
				pDst[nLen++] = pSrc[i];
				pDst[nLen] = 0;
			}
		}
		else if (pSrc[i] >= ASCII_MIN && pSrc[i] <= ASCII_MAX)
		{
			pDst[nLen++] = pSrc[i];
			pDst[nLen] = 0;
		}
		else
		{
			pDst[nLen] = 0;
			return 0;
		}
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
int ApduReadRecord(SCARDHANDLE hCard,DWORD dwProtocol,char* pRecord,int nMaxLen)
{
	const BYTE BLOCK_BYTES = 16;
	const BYTE BLOCK_START =  4;

	BYTE pRecv[256];

	pRecord[0] = 0;

	for (BYTE byBlock = BLOCK_START; byBlock < nMaxLen/BLOCK_BYTES + BLOCK_START; byBlock++)
	{
		if (ApduReadBlock(hCard,dwProtocol,byBlock,pRecv) != BLOCK_BYTES) return 0;
		if (ApduAppendAsciiRecord(pRecord,pRecv) == 0) return lstrlen(pRecord);
	}
	
	return lstrlen(pRecord);;
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardReadUser()
//
//   PURPOSE:  This function authenticates the user ID block on
//             the card and reads the user id.
//
//   COMMENTS: Returns 1 if successfull or 0 for a failure.
//
///////////////////////////////////////////////////////////////////////////////
int WinSmartCardReadUser() // FIXME - rwad token as user to concrete
{
	ZeroMemory(m_szUser,MAX_BUFFER);

	//-----------------------------------------------------------
	// Authenticate the card and read the user text
	//-----------------------------------------------------------
	ApduAuthenticate(m_hCard,m_dwProtocol);
	ApduReadRecord(m_hCard,m_dwProtocol,m_szUser,MAX_BUFFER);

	//-----------------------------------------------------------
	// Failed to read user text, default to GUEST
	//-----------------------------------------------------------
	if (lstrlen(m_szUser) == 0)
	{
		lstrcpyn(m_szUser,DEFAULT_USER_NAME,MAX_BUFFER);  // FIXME lose this state!
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardThread(void*)
//
//   PURPOSE:  Polls the Card Reader and Sends Messages to Main Window
//
//   COMMENTS: 
//
//
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI WinSmartCardThread(void*)
{
	BYTE  pAttrib[MAX_BUFFER];
	BOOL bConnected = FALSE;
	DWORD dwLength = 0;
	DWORD dwState = 0;
	DWORD dwProtocol= 0;
	DWORD dwAttrib = MAX_BUFFER;

	//---------------------------------------------------------------
	// Poll Until The Program Exits and Cleanup is Called
	//---------------------------------------------------------------
	while (m_bPolling)
	{
		//-----------------------------------------------------------
		// Connect to the Reader
		//-----------------------------------------------------------
		while (m_bPolling && bConnected == FALSE)
		{
			Sleep(POLLING_DELAY);
			m_retCode = SCardConnect(m_hContext,m_szReader,SCARD_SHARE_SHARED,
									 SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1,
									 &m_hCard,&m_dwProtocol);

			if (m_retCode == SCARD_S_SUCCESS)
			{
				bConnected = TRUE;
			}
			else if (m_retCode == SCARD_E_READER_UNAVAILABLE ||
					 m_retCode == SCARD_E_UNKNOWN_READER)
			{
				m_bPolling = FALSE;
				SCardReleaseContext(m_hContext);
				PostMessage(m_hWnd,SMART_READER_STOPPED,0,0);    
			}
		}

		//-----------------------------------------------------------
		// Try to read the user and post the arrive message
		//-----------------------------------------------------------
		bConnected = WinSmartCardReadUser();
		if (bConnected) PostMessage(m_hWnd,SMART_CARD_ARRIVE,0,0);    
		Sleep(1500);

		//-----------------------------------------------------------
		// Check for a Disconnection
		//-----------------------------------------------------------
		while (m_bPolling && bConnected == TRUE)
		{
			Sleep(POLLING_DELAY);
			m_retCode = SCardStatus(m_hCard,m_szReader,
			                        &dwLength,&dwState,&dwProtocol,
									pAttrib,&dwAttrib);
			if (m_retCode != SCARD_S_SUCCESS)
			{
				bConnected = FALSE;
				if (m_retCode == SCARD_W_REMOVED_CARD)
				{
					PostMessage(m_hWnd,SMART_CARD_REMOVE,0,0);    
				}
				else if (m_retCode == SCARD_E_READER_UNAVAILABLE)
				{
					m_bPolling = FALSE;
					SCardReleaseContext(m_hContext);
					PostMessage(m_hWnd,SMART_READER_STOPPED,0,0);    
				}
			}
		}
		Sleep(1500);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardGetUser(char* szUser)
//
//   PURPOSE:  Stores the user ID
//
//   COMMENTS:
//
///////////////////////////////////////////////////////////////////////////////
int WinSmartCardGetUser(char* szUser,int nMaxLen)
{
	lstrcpyn(szUser,m_szUser,nMaxLen);
	return lstrlen(szUser);
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardGetReader(char* szReader)
//
//   PURPOSE:  Stores the reader ID
//
//   COMMENTS:
//
///////////////////////////////////////////////////////////////////////////////
int WinSmartCardGetReader(char* szReader,int nMaxLen)
{
	lstrcpyn(szReader,m_szReader,nMaxLen);
	return lstrlen(szReader);
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardPolling(int code)
//
//   PURPOSE:  Returns the polling state
//
//   COMMENTS: 
//
///////////////////////////////////////////////////////////////////////////////
int WinSmartCardPolling()
{
	return m_bPolling;
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardErrorString(int code)
//
//   PURPOSE:  Creates the text of a card reader error
//
//   COMMENTS: These errors should be listed in scarderr.h,
//             however, some errors were missing out of some
//             versions, so they are hard coded for this file.
//             At some time in the future the error numbers 
//             be replaced with the correct variables. 
//
///////////////////////////////////////////////////////////////////////////////
const char* WinSmartCardErrorString(DWORD code)
{
	if (code == SCARD_S_SUCCESS) return "SCARD_S_SUCCESS";
	if (code == NO_READERS_FOUND) return "NO_READERS_FOUND";
	if (code == READER_NOT_FOUND) return "READER_NOT_FOUND";
	if (code == POLLING_THREAD_FAILED) return "POLLING_THREAD_FAILED";
	if (code == ((DWORD)0x80100001)) return "SCARD_F_INTERNAL_ERROR";
	if (code == ((DWORD)0x80100002)) return "SCARD_E_CANCELLED";
	if (code == ((DWORD)0x80100003)) return "SCARD_E_INVALID_HANDLE";
	if (code == ((DWORD)0x80100004)) return "SCARD_E_INVALID_PARAMETER";
	if (code == ((DWORD)0x80100005)) return "SCARD_E_INVALID_TARGET";
	if (code == ((DWORD)0x80100006)) return "SCARD_E_NO_MEMORY";
	if (code == ((DWORD)0x80100007)) return "SCARD_F_WAITED_TOO_LONG";
	if (code == ((DWORD)0x80100008)) return "SCARD_E_INSUFFICIENT_BUFFER";
	if (code == ((DWORD)0x80100009)) return "SCARD_E_UNKNOWN_READER";
	if (code == ((DWORD)0x8010000A)) return "SCARD_E_TIMEOUT";
	if (code == ((DWORD)0x8010000B)) return "SCARD_E_SHARING_VIOLATION";
	if (code == ((DWORD)0x8010000C)) return "SCARD_E_NO_SMARTCARD";
	if (code == ((DWORD)0x8010000D)) return "SCARD_E_UNKNOWN_CARD";
	if (code == ((DWORD)0x8010000E)) return "SCARD_E_CANT_DISPOSE";
	if (code == ((DWORD)0x8010000F)) return "SCARD_E_PROTO_MISMATCH";
	if (code == ((DWORD)0x80100010)) return "SCARD_E_NOT_READY";
	if (code == ((DWORD)0x80100011)) return "SCARD_E_INVALID_VALUE";
	if (code == ((DWORD)0x80100012)) return "SCARD_E_SYSTEM_CANCELLED";
	if (code == ((DWORD)0x80100013)) return "SCARD_F_COMM_ERROR";
	if (code == ((DWORD)0x80100014)) return "SCARD_F_UNKNOWN_ERROR";
	if (code == ((DWORD)0x80100015)) return "SCARD_E_INVALID_ATR";
	if (code == ((DWORD)0x80100016)) return "SCARD_E_NOT_TRANSACTED";
	if (code == ((DWORD)0x80100017)) return "SCARD_E_READER_UNAVAILABLE";
	if (code == ((DWORD)0x80100018)) return "SCARD_P_SHUTDOWN";
	if (code == ((DWORD)0x80100019)) return "SCARD_E_PCI_TOO_SMALL";
	if (code == ((DWORD)0x8010001A)) return "SCARD_E_READER_UNSUPPORTED";
	if (code == ((DWORD)0x8010001B)) return "SCARD_E_DUPLICATE_READER";
	if (code == ((DWORD)0x8010001C)) return "SCARD_E_CARD_UNSUPPORTED";
	if (code == ((DWORD)0x8010001D)) return "SCARD_E_NO_SERVICE";
	if (code == ((DWORD)0x8010001E)) return "SCARD_E_SERVICE_STOPPED";
	if (code == ((DWORD)0x8010001F)) return "SCARD_E_UNEXPECTED";
	if (code == ((DWORD)0x80100020)) return "SCARD_E_ICC_INSTALLATION";
	if (code == ((DWORD)0x80100021)) return "SCARD_E_ICC_CREATEORDER";
	if (code == ((DWORD)0x80100022)) return "SCARD_E_UNSUPPORTED_FEATURE";
	if (code == ((DWORD)0x80100023)) return "SCARD_E_DIR_NOT_FOUND";
	if (code == ((DWORD)0x80100024)) return "SCARD_E_FILE_NOT_FOUND";
	if (code == ((DWORD)0x80100025)) return "SCARD_E_NO_DIR";
	if (code == ((DWORD)0x80100026)) return "SCARD_E_NO_FILE";
	if (code == ((DWORD)0x80100027)) return "SCARD_E_NO_ACCESS";
	if (code == ((DWORD)0x80100028)) return "SCARD_E_WRITE_TOO_MANY";
	if (code == ((DWORD)0x80100029)) return "SCARD_E_BAD_SEEK";
	if (code == ((DWORD)0x8010002A)) return "SCARD_E_INVALID_CHV";
	if (code == ((DWORD)0x8010002B)) return "SCARD_E_UNKNOWN_RES_MNG";
	if (code == ((DWORD)0x8010002C)) return "SCARD_E_NO_SUCH_CERTIFICATE";
	if (code == ((DWORD)0x8010002D)) return "SCARD_E_CERTIFICATE_UNAVAILABLE";
	if (code == ((DWORD)0x8010002E)) return "SCARD_E_NO_READERS_AVAILABLE";
	if (code == ((DWORD)0x8010002F)) return "SCARD_E_COMM_DATA_LOST";
	if (code == ((DWORD)0x80100030)) return "SCARD_E_NO_KEY_CONTAINER";
	if (code == ((DWORD)0x80100031)) return "SCARD_E_SERVER_TOO_BUSY";
	if (code == ((DWORD)0x80100065)) return "SCARD_W_UNSUPPORTED_CARD";
	if (code == ((DWORD)0x80100066)) return "SCARD_W_UNRESPONSIVE_CARD";
	if (code == ((DWORD)0x80100067)) return "SCARD_W_UNPOWERED_CARD";
	if (code == ((DWORD)0x80100068)) return "SCARD_W_RESET_CARD";
	if (code == ((DWORD)0x80100069)) return "SCARD_W_REMOVED_CARD";
	if (code == ((DWORD)0x8010006A)) return "SCARD_W_SECURITY_VIOLATION";
	if (code == ((DWORD)0x8010006B)) return "SCARD_W_WRONG_CHV";
	if (code == ((DWORD)0x8010006C)) return "SCARD_W_CHV_BLOCKED";
	if (code == ((DWORD)0x8010006D)) return "SCARD_W_EOF";
	if (code == ((DWORD)0x8010006E)) return "SCARD_W_CANCELLED_BY_USER";
	if (code == ((DWORD)0x8010006F)) return "SCARD_W_CARD_NOT_AUTHENTICATED";
	if (code == ((DWORD)0x80100070)) return "SCARD_W_CACHE_ITEM_NOT_FOUND";
	if (code == ((DWORD)0x80100071)) return "SCARD_W_CACHE_ITEM_STALE";
	if (code == ((DWORD)0x80100072)) return "SCARD_W_CACHE_ITEM_TOO_BIG";
	return ("UNKNOWN CARD READER ERROR");
}

///////////////////////////////////////////////////////////////////////////////
//
//   FUNCTION: WinSmartCardShowError(int code)
//
//   PURPOSE:  Displays the error and returns 0 for a failure.
//
//   COMMENTS: 
//
///////////////////////////////////////////////////////////////////////////////
int WinSmartCardShowError()
{
	if (m_retCode != SCARD_S_SUCCESS)
		MessageBox(NULL,WinSmartCardErrorString(m_retCode),
		           "WinSmartCard Error",MB_ICONEXCLAMATION);
	return 0;
}

//-------------------------------------------------------------------
//
//   FUNCTION: WinSmartCardInitialize(HWND hWnd,const char* szReader)
//
//   PURPOSE:  Search for the desired reader and start polling to
//             send the SMART_CARD_ARRIVE and SMART_CARD_REMOVE
//             message to the main WndPro.
//
//   COMMENTS: If szReader is not specified, the first reader found
//             will be used.
//
//-------------------------------------------------------------------
int WinSmartCardInitialize(HWND hWnd,const char* szReader)
{
	int nFound = 0;
	int nLen = 0;
	DWORD dwSize = MAX_BUFFER;
	DWORD dwThreadId = 0;
    char readerList [MAX_BUFFER];

	//---------------------------------------------------------------
	// Stop polling if already running
	//---------------------------------------------------------------
	if (m_bPolling)
	{
		m_bPolling = 0;
		WaitForSingleObject(m_hThread,3000);
		SCardDisconnect(m_hCard, SCARD_LEAVE_CARD);
		SCardReleaseContext(m_hContext);
	}

	//---------------------------------------------------------------
	// Store the Window and Expected Messages
	//---------------------------------------------------------------
	m_hWnd = hWnd;
	lstrcpyn(m_szReader,szReader,MAX_BUFFER);

	//---------------------------------------------------------------
	// Establish Context
	//---------------------------------------------------------------
	m_retCode = SCardEstablishContext(SCARD_SCOPE_USER,
									  NULL,NULL,&m_hContext);
	if (m_retCode != SCARD_S_SUCCESS) return 0;

	//---------------------------------------------------------------
	// List the Card Readers
	//---------------------------------------------------------------
	m_retCode = SCardListReaders(m_hContext,NULL,readerList,&dwSize);
	if (m_retCode != SCARD_S_SUCCESS) return 0;
	if (readerList == NULL)
	{
		m_retCode = NO_READERS_FOUND;
		return 0;
	}

	//---------------------------------------------------------------
	// Find the Desired Reader
	//---------------------------------------------------------------
	if (lstrlen(szReader))
	{
		m_nReaderIndex = -1;
		char *p = readerList;
		while ( *p )
		{
			nLen = lstrlen(p) + 1;
			if ( *p != 0 )
			{
     			if (lstrcmp(szReader,p) == 0) m_nReaderIndex = nFound;
				nFound++;
			}
			p = &p[nLen];
		}
		if (m_nReaderIndex < 0)
		{
			m_retCode = READER_NOT_FOUND;
			return 0;
		}
	}
	//---------------------------------------------------------------
	// No Reader Specified -- Just Use The First One
	//---------------------------------------------------------------
	else
	{
		lstrcpyn(m_szReader,readerList,MAX_BUFFER);
		m_nReaderIndex = 0;
	}

	//---------------------------------------------------------------
	// Start the Polling Thread
	//---------------------------------------------------------------
	m_bPolling = TRUE;
	m_hThread = CreateThread( 
	  			  NULL,                // default security attributes
				  0,                   // use default stack size  
				  WinSmartCardThread,  // thread function name
				  0,                   // argument to thread function 
				  0,                   // use default creation flags 
				  &dwThreadId);        // returns the thread identifier 
	if (m_hThread == NULL)
	{
		m_bPolling = FALSE;
		m_retCode= POLLING_THREAD_FAILED;
		return 0;
	}

	return 1;
}
