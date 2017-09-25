/******************************************************************************

            COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of 
        Macrovision Corporation and is protected by law.  It may 
        not be copied or distributed in any form or medium, disclosed 
        to third parties, reverse engineered or used in any manner not 
        provided for in said License Agreement except with the prior 
        written authorization from Macrovision Corporation.


 *****************************************************************************/
/*      
 *      Module: $Id: wsock32.c,v 1.11.2.3 2003/06/26 21:58:29 sluu Exp $
 *      $
 *
 *      Function:       All Winsock API calls which are used by FLEXlm client
 *                      library.
 *
 *      Description:    This file generates a dummy WINSOCK.DLL for FLEXlm
 *                      Windows client to use in a node-lock environment
 *                      where a real network is not available.
 *
 *      Parameters:     standard WINSOCK API.  See <winsock.h>
 *                      
 *
 *      Return:         standard WINSOCK API.  See <winsock.h>
 *
 *      Chia-Chee Kuan/Blane Eisenberg
 *
 *      Last changed:  9/26/98
 *
 */


#include <winsock2.h>
#include <nspapi.h>
#include "flex_utils.h"

#include "ipexport.h"
/*#define WINSOCK_VERSION 0x0200*/  /* Winsock version that we are compatible with */
char * getenv1(char * name);

static char *errMsg = "FLEXlm WINSOCK usage error detected.  "
                      "You must make sure your license file does only "
                      "node-locking, or you must have a WINSOCK.DLL "
                      "in order to communicate with FLEXlm license server. ";

static char *errTitle = "FLEXlm WINSOCK error";
typedef  int (WINAPI * WINSOCK_SEND)(SOCKET s, const char FAR * buf, int len,
                                              int flags);


typedef  int (WINAPI * WINSOCK_IOCTLSOCKET) (SOCKET s, long cmd,
                                              u_long FAR *argp);

typedef int (WINAPI *WINSOCK_WSAGETLASTERROR)(void);
typedef int (WINAPI *WINSOCK_RECV) (SOCKET s, char FAR * buf, int len,
                                              int flags);
typedef int (WINAPI *WINSOCK_CLOSESOCKET) (SOCKET s);
typedef struct servent FAR * (WINAPI *WINSOCK_GETSERVBYNAME)
                        (const char FAR * name, const char FAR * proto);
typedef struct hostent FAR * (WINAPI *WINSOCK_GETHOSTBYNAME)
                        (const char FAR * name);
typedef int (WINAPI *WINSOCK_GETHOSTNAME) (char FAR * name, int namelen);

typedef int (WINAPI *WINSOCK_WSASTARTUP) (WORD wVersionRequired,
                                      LPWSADATA lpWSAData);
typedef int (WINAPI *WINSOCK_WSACLEANUP)(void);
typedef u_short (WINAPI *WINSOCK_HTONS) (u_short hostshort);
typedef u_long (WINAPI *WINSOCK_NTOHL) (u_long hostulong);
typedef int (WINAPI *WINSOCK_CONNECT) (SOCKET s,
                                const struct sockaddr FAR *name, int namelen);
typedef SOCKET (WINAPI *WINSOCK_SOCKET) (int af, int type, int protocol);
typedef int (WINAPI *WINSOCK_SELECT) (int nfds, fd_set FAR *readfds,
                        fd_set FAR *writefds, fd_set FAR *exceptfds,
                        const struct timeval FAR *timeout);
typedef u_short (WINAPI *WINSOCK_NTOHS)(u_short netshort);
typedef int (WINAPI * WINSOCK___WSAFDISSET)(SOCKET s, fd_set FAR *fd);
typedef int    (WINAPI * WINSOCK_BIND)(SOCKET s, const struct sockaddr FAR *name,int i);
typedef int    (WINAPI * WINSOCK_GETSOCKNAME)(SOCKET s, const struct sockaddr FAR * name, int FAR * namelen);
typedef SOCKET (WINAPI * WINSOCK_ACCEPT)  (SOCKET s, struct sockaddr FAR *addr,
                          int FAR *addrlen);
typedef int (WINAPI * WINSOCK_LISTEN) (SOCKET s, int backlog);
typedef void (WINAPI * WINSOCK_WSASET_LAST_ERROR)(int iError);
typedef int  (WINAPI * WINSOCK_SETSOCKOPT) (SOCKET s, int level, int optname,
                           const char FAR * optval, int optlen);

typedef int (WINAPI * WINSOCK_SENDTO) (SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen);

typedef int (WINAPI * WINSOCK_RECVFROM) (SOCKET s, char FAR * buf, int len, int flags,
                         struct sockaddr FAR *from, int FAR * fromlen);

typedef int (WINAPI * WINSOCK_SHUTDOWN) (SOCKET s, int how); 
typedef u_long (WINAPI * WINSOCK_HTONL) (u_long hostlong);  
typedef int (WINAPI * WINSOCK_GETSOCKOPT) (SOCKET s, int level, int optname, char * optval, int * optlen);
typedef struct hostent FAR * (WINAPI *WINSOCK_GETHOSTBYADDR)
                        (const char FAR * addr, int len, int type);
// WINSOCK 1.1 extensions ///////////////////////////////////////////////
typedef int (WINAPI * WINSOCK_ENUM_PROTOCOLS) (int * lpiProtocols, void * lpProtocolBuffer, DWORD * lpBufferlen);

typedef int (WINAPI * WINSOCK_GETADDRESSBYNAME)( DWORD dwNameSpace,
						GUID * lpServiceType, 
						char *lpServiceName ,
						int * lpiProtocols ,
						DWORD dwResolution, 
						SERVICE_ASYNC_INFO * lpServiceAsyncInfo ,
						void * lpCsaddrBuffer,
						DWORD * lpdwBufferLength,
						char * lpAliasBuffer ,
						DWORD * lpdwAliasBufferLength );


//WinSOCK 2.0 prototypes////////////////////////////////////////////

typedef WSAEVENT (WINAPI * WINSOCK_WSACREATEEVENT)(void);
typedef int (WINAPI * WINSOCK_WSAEVENTSELECT) (SOCKET s, WSAEVENT e, long x);
typedef DWORD (WINAPI * WINSOCK_WSAWAITFORMULTIPLEEVENTS) (    DWORD cEvents, const WSAEVENT FAR * lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
typedef int (WINAPI * WINSOCK_WSAENUMNETWORKEVENTS) ( SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);

typedef int (WINAPI * WSALOOKUPSERVICEBEGIN ) ( LPWSAQUERYSETA lpqsRestrictions,DWORD dwControlFlags,
    LPHANDLE  lphLookup );

typedef int ( WINAPI * WSALOOKUPSERVICENEXT ) ( HANDLE hLookup, DWORD dwControlFlags, LPDWORD lpdwBufferLength,
	LPWSAQUERYSETW   lpqsResults );

typedef int (WINAPI  * WSALOOKUPSERVICEEND) ( HANDLE  hLookup );

///////////////////ICMP prototypes /////////////////////////////////////

typedef HANDLE (WINAPI * ICMP_CREATEFILE ) ( void );
typedef  BOOL (WINAPI * ICMP_CLOSEHANDLE ) ( HANDLE IcmpHandle);
typedef DWORD ( WINAPI * ICMP_SENDECHO ) (HANDLE Ic, IPAddr De, LPVOID  RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    );


extern int      lm_max_masks;
 int winsockver2=FALSE;
 int network_installed;
 int icmp_installed;
 WORD WinsockVerRequested;
HINSTANCE winsock_hInst=0;      /* Winsock library instance handle */
HINSTANCE icmp_hInst=0; 
static WinsockRefCount=0;
static WINSOCK_SEND winsock_send  ;
static WINSOCK_IOCTLSOCKET winsock_ioctlsocket;
static  WINSOCK_WSAGETLASTERROR winsock_WSAGetLastError ;
static  WINSOCK_RECV winsock_recv ;
static  WINSOCK_CLOSESOCKET winsock_closesocket ;
static  WINSOCK_GETSERVBYNAME winsock_getservbyname ;
static  WINSOCK_GETHOSTBYNAME winsock_gethostbyname ;
static  WINSOCK_GETHOSTNAME winsock_gethostname ;
static  WINSOCK_WSASTARTUP winsock_WSAStartup ;
static  WINSOCK_WSACLEANUP winsock_WSACleanup ;
static  WINSOCK_HTONS  winsock_htons;
static  WINSOCK_NTOHL  winsock_ntohl;
static  WINSOCK_CONNECT winsock_connect ;
static  WINSOCK_SOCKET winsock_socket ;
static  WINSOCK_SELECT true_winsock_select ;        
static  WINSOCK_SELECT winsock_select ;        
static  WINSOCK_NTOHS  winsock_ntohs;
static  WINSOCK___WSAFDISSET winsock___WSAFDIsSet ;
static  WINSOCK_BIND winsock_bind;
static  WINSOCK_GETSOCKNAME winsock_getsockname;
static  WINSOCK_ACCEPT winsock_accept;
static  WINSOCK_LISTEN  winsock_listen;
static  WINSOCK_WSASET_LAST_ERROR winsock_wsaset_last_error;
static  WINSOCK_SETSOCKOPT winsock_setsockopt;
static  WINSOCK_RECVFROM winsock_recvfrom;
static  WINSOCK_SENDTO   winsock_sendto;
static  WINSOCK_SHUTDOWN winsock_shutdown;
static WINSOCK_HTONL  winsock_htonl;
static WINSOCK_GETSOCKOPT winsock_getsockopt;
static WINSOCK_GETHOSTBYADDR winsock_gethostbyaddr;
// WINSOCK 1.1 extensions 
static WINSOCK_ENUM_PROTOCOLS winsock_enum_protocols ;
static WINSOCK_GETADDRESSBYNAME winsock_getaddressbyname ;

// Winsock 2.0 calls


static WINSOCK_WSACREATEEVENT winsock_wsacreateevent;
static WINSOCK_WSAEVENTSELECT winsock_wsaeventselect;
static WINSOCK_WSAWAITFORMULTIPLEEVENTS winsock_wsawaitformultipleevents;
static WINSOCK_WSAENUMNETWORKEVENTS    winsock_wsaenumnetworkevents;



static WSALOOKUPSERVICEBEGIN winsock_wsalookupservicebegin ;
static WSALOOKUPSERVICENEXT winsock_wsalookupservicenext ; 
static WSALOOKUPSERVICEEND winsock_wsalookupserviceend ;

// ICMP calls
static ICMP_CREATEFILE icmp_createfile;
static  ICMP_SENDECHO icmp_sendecho;
static ICMP_CLOSEHANDLE icmp_closehandle;

//static int first_time=0;
static int Winsock_Initialized=0;
static int icmp_initialized=0;
int recvcount=0xdeadbeef;
int recverror=0xdeadbeef;
WSADATA wsadata;


/*
 * INITIALIZATION
 */

static BOOL winsock_real_init(BOOL bShowError)
{
  static BOOL getenv_check=FALSE, no_network=FALSE;
  
      
        if (!getenv_check)
        {
                no_network = getenv1("LM_NO_NETWORK") ? TRUE : FALSE;
                getenv_check=TRUE;
        }

        if (no_network)
                return FALSE;


        if ( winsock_hInst >= (void *) HINSTANCE_ERROR )
        {       
        
                if (Winsock_Initialized==0)
                        {       (void) (*winsock_WSAStartup)(WinsockVerRequested, &wsadata);
                                Winsock_Initialized=1;
                        }

        return TRUE;
        }
        /* 
         * Instruct Windows not to display the annoying error msg
         * when LoadLibrary() call fails to find the DLL.
         */
        SetErrorMode( SEM_NOOPENFILEERRORBOX );
                
        // Do this only if WINSOCK.DLL is found!
        if (GetVersion()< 0x80000000 && (winsock_hInst = LoadLibrary( "WS2_32.Dll" ))>= (void *) HINSTANCE_ERROR )
        {
                winsockver2 = TRUE;
        }
        else
        if ( (winsock_hInst = LoadLibrary( "WSOCK32.Dll" ))< (void *) HINSTANCE_ERROR )
        {
                        network_installed=0;
                        return FALSE;
                
        }
        else            winsockver2 = FALSE;



        if ( (icmp_hInst= LoadLibrary( "icmp.Dll" ))< (void *) HINSTANCE_ERROR )
        {
                        icmp_installed=0;
                
        }
        else
        {
        icmp_createfile= (ICMP_CREATEFILE)GetProcAddress( icmp_hInst, "IcmpCreateFile") ;
        icmp_sendecho=(ICMP_SENDECHO)GetProcAddress( icmp_hInst, "IcmpSendEcho") ;
        icmp_closehandle=(ICMP_CLOSEHANDLE)GetProcAddress( icmp_hInst, "IcmpCloseHandle");

        }

//#pragma warning (disable: 4113)
//#pragma warning (disable: 4047)

        winsock_send = (WINSOCK_SEND)GetProcAddress( winsock_hInst, "send" );
        winsock_ioctlsocket=(WINSOCK_IOCTLSOCKET) GetProcAddress( winsock_hInst, "ioctlsocket" );
        winsock_WSAGetLastError= (WINSOCK_WSAGETLASTERROR)GetProcAddress( winsock_hInst, "WSAGetLastError" );
        winsock_recv= (WINSOCK_RECV)GetProcAddress( winsock_hInst, "recv" );
        winsock_closesocket= (WINSOCK_CLOSESOCKET)GetProcAddress( winsock_hInst, "closesocket" );
        winsock_getservbyname = (WINSOCK_GETSERVBYNAME)GetProcAddress( winsock_hInst, "getservbyname" );
        winsock_gethostbyname = (WINSOCK_GETHOSTBYNAME)GetProcAddress( winsock_hInst, "gethostbyname" );
        winsock_gethostname = (WINSOCK_GETHOSTNAME)GetProcAddress( winsock_hInst, "gethostname" );        winsock_WSAStartup = (WINSOCK_WSASTARTUP)GetProcAddress( winsock_hInst, "WSAStartup" );
        winsock_WSACleanup = (WINSOCK_WSACLEANUP)GetProcAddress( winsock_hInst, "WSACleanup" );
        winsock_htons =  (WINSOCK_HTONS)GetProcAddress( winsock_hInst, "htons" );
        winsock_ntohl = (WINSOCK_NTOHL)GetProcAddress( winsock_hInst, "ntohl" );        
        winsock_connect = (WINSOCK_CONNECT)GetProcAddress( winsock_hInst, "connect" );
        winsock_socket = (WINSOCK_SOCKET)GetProcAddress( winsock_hInst, "socket" );
        true_winsock_select = (WINSOCK_SELECT)GetProcAddress( winsock_hInst, "select" );
        winsock_ntohs = (WINSOCK_NTOHS)GetProcAddress( winsock_hInst, "ntohs" );
        winsock___WSAFDIsSet = (WINSOCK___WSAFDISSET)GetProcAddress( winsock_hInst, "__WSAFDIsSet" );
        winsock_bind = (WINSOCK_BIND)GetProcAddress( winsock_hInst, "bind" );
        winsock_getsockname = (WINSOCK_GETSOCKNAME)GetProcAddress( winsock_hInst, "getsockname" );
        winsock_accept = (WINSOCK_ACCEPT)GetProcAddress( winsock_hInst, "accept" );
        winsock_listen = (WINSOCK_LISTEN)GetProcAddress( winsock_hInst, "listen" );
        winsock_wsaset_last_error = (WINSOCK_WSASET_LAST_ERROR)GetProcAddress( winsock_hInst, "WSASetLastError" );
        winsock_setsockopt = (WINSOCK_SETSOCKOPT)GetProcAddress( winsock_hInst, "setsockopt" );
        winsock_sendto = (WINSOCK_SENDTO)GetProcAddress( winsock_hInst, "sendto" );
        winsock_recvfrom = (WINSOCK_RECVFROM)GetProcAddress( winsock_hInst, "recvfrom" );
        winsock_htonl=(WINSOCK_HTONL)GetProcAddress( winsock_hInst, "htonl" );
        winsock_shutdown=(WINSOCK_SHUTDOWN)GetProcAddress( winsock_hInst, "shutdown" );
        winsock_getsockopt=(WINSOCK_GETSOCKOPT)GetProcAddress( winsock_hInst, "getsockopt" );
        winsock_gethostbyaddr=(WINSOCK_GETHOSTBYADDR)GetProcAddress( winsock_hInst, "gethostbyaddr" );
	winsock_enum_protocols = (WINSOCK_ENUM_PROTOCOLS)  GetProcAddress( winsock_hInst, "EnumProtocolsA" );
	winsock_getaddressbyname = (WINSOCK_GETADDRESSBYNAME) GetProcAddress( winsock_hInst, "GetAddressByNameA" );

        if (winsockver2 )
        {
                winsock_select=true_winsock_select;
                WinsockVerRequested=MAKEWORD(2, 0);
                winsock_wsacreateevent=(WINSOCK_WSACREATEEVENT)GetProcAddress( winsock_hInst, "WSACreateEvent" );
                winsock_wsaeventselect=(WINSOCK_WSAEVENTSELECT)GetProcAddress( winsock_hInst, "WSAEventSelect" );
                winsock_wsawaitformultipleevents=(WINSOCK_WSAWAITFORMULTIPLEEVENTS)GetProcAddress( winsock_hInst, "WSAWaitForMultipleEvents" );
                winsock_wsaenumnetworkevents=(WINSOCK_WSAENUMNETWORKEVENTS)GetProcAddress( winsock_hInst, "WSAEnumNetworkEvents" );
		winsock_wsalookupservicebegin = (WSALOOKUPSERVICEBEGIN) GetProcAddress( winsock_hInst, "WSALookupServiceBeginA");
		winsock_wsalookupservicenext = ( WSALOOKUPSERVICENEXT ) GetProcAddress( winsock_hInst, "WSALookupServiceNextA") ;
		winsock_wsalookupserviceend =  ( WSALOOKUPSERVICEEND ) GetProcAddress( winsock_hInst, "WSALookupServiceEnd");
      
        
        }
        else
        {
                winsock_select=true_winsock_select;
                WinsockVerRequested=MAKEWORD(1, 1);
        }
                        

        if ( !winsock_send              || !winsock_ioctlsocket ||
             !winsock_WSAGetLastError   || !winsock_recv ||
             !winsock_closesocket       || !winsock_getservbyname ||
             !winsock_gethostbyname     || !winsock_gethostname ||
             !winsock_WSAStartup        || !winsock_WSACleanup ||
             !winsock_htons             || !winsock_connect ||
             !winsock_socket            || !winsock_select ||
             !winsock___WSAFDIsSet      || !winsock_ntohs ||
             !winsock_bind              || !winsock_getsockname ||
             !winsock_ntohl             || !winsock_accept ||
             !winsock_listen            || !winsock_wsaset_last_error ||
             !winsock_setsockopt        || !winsock_sendto ||
             !winsock_recvfrom          || !winsock_htonl   ||
             !winsock_shutdown          || !winsock_getsockopt ||
             !winsock_gethostbyaddr     || (!winsockver2 && !winsock_enum_protocols) ||
	     (!winsockver2 && !winsock_getaddressbyname )	)                 
        {
                if (bShowError)
                        MessageBox( GetActiveWindow(), errMsg, errTitle, MB_OK |
				MB_ICONSTOP | MB_APPLMODAL | MB_SETFOREGROUND );
                return FALSE;
        }

        if (winsockver2 && ( !winsock_wsacreateevent || 
				!winsock_wsaeventselect ||
                             !winsock_wsawaitformultipleevents || 
			     !winsock_wsaenumnetworkevents ||
			     !winsock_wsalookupservicebegin  || 
			     !winsock_wsalookupservicenext ||
			     !winsock_wsalookupserviceend   ))
        {
                if (bShowError)
                        MessageBox( GetActiveWindow(), errMsg, errTitle, MB_OK |
				MB_ICONSTOP | MB_APPLMODAL | MB_SETFOREGROUND );
                return FALSE;
        }

         network_installed=1;

        (void) (*winsock_WSAStartup)(WinsockVerRequested, &wsadata);

        lm_max_masks=1024;
        WinsockRefCount ++;
        Winsock_Initialized=1;
        return TRUE;
}

/*
 *      Let all winsock startup call to be successful.  Check whether a
 *      WinSock library is really existing at the time of other WinSock
 *      function calls.
 */
int PASCAL FAR flex_WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
        if ( !winsock_real_init(FALSE) )
                return 0;
        
//      WinsockRefCount ++;
//      if (first_time == 1  )
//      {
//      first_time=0;
//      Winsock_Initialized=1;
//      return (*winsock_WSAStartup)(wVersionRequired, lpWSAData);
//      }
        return 0;       
}

int PASCAL FAR flex_WSACleanup(void)
{
        if ( !Winsock_Initialized )
                return 0;
        if (  !winsock_real_init(FALSE) )
                return 0;
        if ( winsock_hInst && winsock_WSACleanup )
                (*winsock_WSACleanup)();
        Winsock_Initialized=0;
        WinsockRefCount--;

        if ( !(WinsockRefCount)  && winsock_hInst)

        {
//              FreeLibrary(winsock_hInst);
//              winsock_hInst=0;
        }

        return 0;
}

int PASCAL FAR flex_send (SOCKET s, const char FAR * buf, int len, int flags)
{
        if ( !winsock_real_init(TRUE) )
                return -1;
        if (!len)
        {
//              _asm
//              {
//                      int 3
//              }
        }
        return (*winsock_send)(s, buf, len, flags);
}

int PASCAL FAR flex_ioctlsocket (SOCKET s, long cmd, u_long FAR *argp)
{
        if ( !winsock_real_init(TRUE) )
                return -1;

        return (*winsock_ioctlsocket)(s, cmd, argp);
}

int PASCAL FAR flex_WSAGetLastError(void)
{
        if ( !winsock_real_init(TRUE) )
                return -1;
        return (*winsock_WSAGetLastError)();
}

int PASCAL FAR flex_recv (SOCKET s, char FAR * buf, int len, int flags)
{ int ret_len;
        if ( !winsock_real_init(TRUE) )
                return -1;

        ret_len= (*winsock_recv)(s, buf, len, flags);
        recvcount=ret_len;
        if (ret_len<0)
        {
                recverror= flex_WSAGetLastError();

        }
        
        return ret_len;

}

int PASCAL FAR flex_closesocket (SOCKET s)
{ //char s1[128];
        if ( !winsock_real_init(TRUE) )
                return -1;
//              _asm
//              {
//                      int 3
//              }
//      sprintf(s1," socket closed is %d from %d",s,(abs(getpid()%1000)));
//      MessageBox(NULL,s1,NULL,MB_OK_ MB_SETFOREGROUND | MB_APPLMODAL);
        return (*winsock_closesocket)(s);
}

struct servent FAR * PASCAL FAR flex_getservbyname(const char FAR * name, 
                                             const char FAR * proto)
{
        if ( !winsock_real_init(TRUE) )
                return NULL;
        
        return (*winsock_getservbyname)(name,  proto);
}

struct hostent FAR * PASCAL FAR flex_gethostbyname(const char FAR * name)
{
        /*
         *        The assumption being made here is that the likelyhood
         *        of a host that matches a UTF-8 or MB string with a license 
         *        server on both is rare.
         */
        struct hostent *        pResult = NULL;
        char *                                pszName = NULL;
        int                                        size = 0;

        if(!winsock_real_init(FALSE))
                return NULL;

		/*
		 *	Convert to MB and try that first.
		 */
		pszName = l_convertStringUTF8ToMB(NULL, name, &size);
		if(pszName)
		{
			pResult = (*winsock_gethostbyname)(pszName);
		}

		if(pResult == NULL)
		{
			pResult = (*winsock_gethostbyname)(name);
		}

		if(pszName)
		{
			l_free(pszName);
			pszName = NULL;
		}


        return pResult;
}

int PASCAL FAR flex_gethostname (char FAR * name, int namelen)
{
	int result = SOCKET_ERROR;  
	if ( !winsock_real_init(TRUE) )
		return SOCKET_ERROR;
	result = (*winsock_gethostname)(name, namelen);
	if(result != SOCKET_ERROR)
	{
		char *		pszUTF8 = NULL;
		int			len = 0;
		/*
		 *	According to Citrix, this will give me a multi-byte string, which
		 *	I then need to convert to wide characters and then to UTF-8
		 */
		pszUTF8 = l_convertStringMBToUTF8(NULL, name, &len);
		if(pszUTF8)
		{
			strncpy(name, pszUTF8, len > namelen ? namelen - 1 : len);
			l_free(pszUTF8);
			pszUTF8 = NULL;
		}
		else
		{
			result = SOCKET_ERROR;	/* Return some error or another */
		}
	}
  
    return result;
}
int PASCAL FAR flex_getsockname (SOCKET  s,  struct sockaddr FAR * s2, int FAR * len)
{
        if ( !winsock_real_init(TRUE) )
                return -1;
        
        return (*winsock_getsockname)(s, s2,len);
}
int PASCAL FAR flex_bind (SOCKET s, const struct sockaddr FAR * s2, int len)
{
        if ( !winsock_real_init(TRUE) )
                return -1;
        
        return (*winsock_bind)(s, s2,len);
}


u_short PASCAL FAR flex_htons (u_short hostshort)
{
        if ( !winsock_real_init(TRUE) )
                return (unsigned short)NULL;

        return (*winsock_htons)(hostshort);
}

u_long PASCAL FAR flex_ntohl (u_long hostulong)
{
        if ( !winsock_real_init(TRUE) )
                return (unsigned long) NULL;

        return (*winsock_ntohl)(hostulong);
}
u_long PASCAL FAR flex_htonl ( u_long u)
{
        if ( !winsock_real_init(TRUE) )
                return (unsigned long) NULL;
        
        return (*winsock_htonl)(u);
}
int PASCAL FAR flex_connect (SOCKET s, const struct sockaddr FAR *name, int namelen)
{
        if ( !winsock_real_init(TRUE) )
                return (int) NULL;

        return (*winsock_connect)(s, name, namelen);
}

SOCKET PASCAL FAR flex_socket (int af, int type, int protocol)
{
	SOCKET s;

        if ( !winsock_real_init(TRUE) )
                return (unsigned int) NULL;

        s = (*winsock_socket)(af, type, protocol);

	/*
	 * Always make this socket non-inheritable. P4827
	 */
		if(s != INVALID_SOCKET)
		{
			/*
			 *	Don't even bother checking to see if handle is ALREADY
			 *	non inheritable.  When a firewall/vpn package is
			 *	installed, the call to GetHandleInformation() always
			 *	indicates that HANDLE_FLAG_INHERIT is not set.
			 *	If no firewall/vpn software is installed, the socket
			 *	always has it set after creation.  Doing it this way fixes bug
			 *	P6248.
			 */
			(void)SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);
		}

		return s;
}

int PASCAL FAR flex_select (int nfds, fd_set *readfds, fd_set  *writefds,
                    fd_set * exceptfds, const struct timeval FAR *timeout)
{
        int retVal;

        if ( !winsock_real_init(TRUE) )
                return -1;

/*
 *      Wait...
 */

        retVal = (*true_winsock_select)(
                        nfds,
                        readfds,
                        writefds,
                        exceptfds,
                        timeout);

        return retVal;
}

u_short PASCAL FAR flex_ntohs (u_short netshort)
{
        if ( !winsock_real_init(TRUE) )
                return 0;

        return (*winsock_ntohs)(netshort);
}

int PASCAL FAR flex___WSAFDIsSet(SOCKET s, fd_set FAR *fd)
{
        if ( !winsock_real_init(TRUE) )
                return -1;

        return (*winsock___WSAFDIsSet)(s, fd);
}

int PASCAL FAR flex_shutdown ( SOCKET s,int how)
{
        if ( !winsock_real_init(FALSE) )
                return (int) NULL;

        return (*winsock_shutdown)(s,how);

}

int PASCAL FAR flex_getsockopt ( SOCKET s,int level, int optname, char * optval, int * optlen)
{
        if ( !winsock_real_init(FALSE) )
                return (int) NULL;

        return (*winsock_getsockopt)(s,level,optname,optval,optlen);

}

struct hostent FAR * PASCAL FAR flex_gethostbyaddr(const char FAR * addr, int len, int type)
{
        if ( !winsock_real_init(FALSE) )
                return NULL;

        return (*winsock_gethostbyaddr)(addr,len,type);
}

int PASCAL FAR l__WSAGetLastError(void)
{
        return flex_WSAGetLastError();
}


int PASCAL FAR l__WSAFDIsSet(SOCKET s, fd_set FAR *fd)
{
        return  flex___WSAFDIsSet( s, fd);
}
int PASCAL FAR l____WSAFDIsSet(SOCKET s, fd_set FAR *fd)
{
        return  flex___WSAFDIsSet( s, fd);
}


int PASCAL FAR l__closesocket (SOCKET s)
{
        return  flex_closesocket ( s);
}

int PASCAL FAR l__connect (SOCKET s, const struct sockaddr FAR *name, int namelen)
{
        return  flex_connect ( s, name,  namelen) ;
}


struct hostent FAR * PASCAL FAR l__gethostbyname(const char FAR * name)
{
        return  flex_gethostbyname( name);
}
#if 0
int PASCAL FAR l__gethostname(char FAR * name, int namelen)
{
        return flex_gethostname(name,namelen);
}
#endif
u_short PASCAL FAR l__htons (u_short hostshort)
{
        return  flex_htons ( hostshort) ;
}

int PASCAL FAR l__ioctlsocket (SOCKET s, long cmd, u_long FAR *argp)
{
        return  flex_ioctlsocket ( s,  cmd, argp) ;

}

u_short PASCAL FAR l__ntohs (u_short netshort)
{
        return flex_ntohs (netshort);
}

int PASCAL FAR l__recv (SOCKET s, char FAR * buf, int len, int flags)
{
        return  flex_recv ( s,  buf, len, flags) ;

}


int PASCAL FAR l__select (int nfds, int *readfds, int *writefds,
                     int *exceptfds, const struct timeval FAR *timeout)

{
        return  flex_select (nfds, 
                (fd_set *) readfds, 
                (fd_set *) writefds,
                (fd_set *) exceptfds,
                timeout);

}


SOCKET PASCAL FAR l__socket (int af, int type, int protocol)
{
        return  flex_socket (af, type,  protocol)       ;
}


int PASCAL FAR l__send (SOCKET s, const char FAR * buf, int len, int flags)
{
        return flex_send ( s,  buf,  len,  flags) ;
}

int PASCAL FAR l__WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
        return flex_WSAStartup(wVersionRequired, lpWSAData);
}

int PASCAL FAR l__WSACleanup(void)
{
        return flex_WSACleanup();
}


SOCKET PASCAL FAR l__accept (SOCKET s, struct sockaddr FAR *addr,
                          int FAR *addrlen)
{

                if ( !winsock_real_init(TRUE) )
                return (unsigned int) -1;

        return  ( *winsock_accept )  ( s,   addr,addrlen);
}


int PASCAL FAR l__getsockname (SOCKET  s,  struct sockaddr FAR * s2, int FAR * len)
{
        return flex_getsockname (  s,   s2,  len);
}


int PASCAL FAR l__bind (SOCKET s, const struct sockaddr FAR * s2, int len)
{
        return  flex_bind ( s,     s2,  len);
}

int PASCAL FAR l__listen (SOCKET s, int backlog)
{
        if ( !winsock_real_init(TRUE) )
                return -1;

        return  (*winsock_listen)  ( s,  backlog);
}

struct servent FAR * PASCAL FAR l__getservbyname(const char FAR * name, 
                                             const char FAR * proto)
{
        return  flex_getservbyname( name, proto);

}

void PASCAL FAR l__WSASetLastError(int iError)
{
        if ( winsock_real_init(TRUE) )
        

                (*winsock_wsaset_last_error) ( iError);
}



int PASCAL FAR l__setsockopt (SOCKET s, int level, int optname,
                           const char FAR * optval, int optlen)
{
        if ( !winsock_real_init(TRUE) )
                return -1;

        return (* winsock_setsockopt) ( s,  level,  optname,optval,  optlen);
}

int PASCAL FAR l__recvfrom (SOCKET s, char FAR * buf, int len, int flags,
                         struct sockaddr FAR *from, int FAR * fromlen)
{
        if ( !winsock_real_init(TRUE) )
                return -1;

        return (* winsock_recvfrom) ( s,   buf,  len,  flags,from,  fromlen);
}



int PASCAL FAR l__sendto (SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen)
{
        if ( !winsock_real_init(TRUE) )
                return -1;

        return (* winsock_sendto) ( s, buf,  len,  flags, to,  tolen);
}

u_long PASCAL FAR l__ntohl (u_long hostulong)
{
        return flex_ntohl ( hostulong);
}

int PASCAL FAR l__shutdown ( SOCKET s,int how)
{
                return flex_shutdown(s,how);
}

int PASCAL FAR l__getsockopt ( SOCKET s,int l, int lname,char * opt, int * optl)
{
                return flex_getsockopt(s,l,lname,opt,optl);
}

u_long PASCAL FAR l__htonl ( u_long u)
{
                return flex_htonl(u);
}

struct hostent FAR * PASCAL FAR l__gethostbyaddr(const char FAR * addr, int len, int type)
{


        return flex_gethostbyaddr(addr,len,type);
}

DWORD
WINAPI
l__IcmpSendEcho(
    HANDLE                   IcmpHandle,
    IPAddr                   DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    )
{
        if ( !winsock_real_init(TRUE) && !icmp_installed && !icmp_sendecho  )
                return -1;

        return (* icmp_sendecho)(IcmpHandle,DestinationAddress,RequestData,RequestSize,
                RequestOptions,ReplyBuffer,ReplySize,Timeout);
}
BOOL WINAPI
l__IcmpCloseHandle( HANDLE  IcmpHandle)
{
        if ( !winsock_real_init(TRUE) && !icmp_installed && !icmp_closehandle)
                return -1;
        return (*icmp_closehandle)(IcmpHandle);
}


HANDLE WINAPI l__IcmpCreateFile( VOID)
{
        if ( !winsock_real_init(TRUE) && !icmp_installed && !icmp_createfile)
                return (void *)-1; 
        return (*icmp_createfile)();
}


int PASCAL FAR advanced_winsock_select (int nfds, fd_set *readfds, fd_set  *writefds,
                    fd_set * exceptfds, const struct timeval FAR *timeout)
{
        int retVal;

        if ( !winsock_real_init(TRUE) )
                return -1;

/*
 *      Wait...
 */

        retVal = (*true_winsock_select)(
                        nfds,
                        readfds,
                        writefds,
                        exceptfds,
                        timeout);

        return retVal;
}


WSAEVENT WINAPI  l__WSACreateEvent(void)
{

        if ( !winsock_real_init(TRUE) )
                return (WSAEVENT) WSA_INVALID_EVENT;

        return (* winsock_wsacreateevent) ( );

}

int WINAPI l__WSAEventSelect (SOCKET s, WSAEVENT e, long x)
{

        if ( !winsock_real_init(TRUE) )
                return -1;

        return (* winsock_wsaeventselect) ( s,   e,  x );


}

DWORD WINAPI l__WSAWaitForMultipleEvents (    DWORD cEvents, const WSAEVENT FAR * lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable)
{

        if ( !winsock_real_init(TRUE) )
                return -1;

        return (* winsock_wsawaitformultipleevents) ( cEvents,   lphEvents,  fWaitAll,  dwTimeout, fAlertable);

}

int WINAPI l__WSAEnumNetworkEvents  ( SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents)
{

        if ( !winsock_real_init(TRUE) )
                return -1;


        return (* winsock_wsaenumnetworkevents) ( s,   hEventObject ,  lpNetworkEvents );

}


int WinsockVersion()
{

        if ( !winsock_real_init(TRUE) )
                return -1;

        if (winsockver2) 
                return 2;
        else
                return 1;

}


int WINAPI l__EnumProtocols (int * lpiProtocols, void * lpProtocolBuffer, DWORD * lpBufferlen)
{
	if ( !winsock_real_init(TRUE) )
		return -1;
	return (* winsock_enum_protocols) (lpiProtocols, lpProtocolBuffer,lpBufferlen);

}
int WINAPI l__GetAddressByName ( DWORD dwNameSpace, GUID * lpServiceType, char *lpServiceName ,
	int * lpiProtocols , DWORD dwResolution, SERVICE_ASYNC_INFO * lpServiceAsyncInfo ,
	void * lpCsaddrBuffer, DWORD * lpdwBufferLength, char * lpAliasBuffer ,
	DWORD * lpdwAliasBufferLength )
{
	if ( !winsock_real_init(TRUE) )
		return -1;

	return  (* winsock_getaddressbyname ) (dwNameSpace, lpServiceType, lpServiceName,
			lpiProtocols, dwResolution , lpServiceAsyncInfo ,
			lpCsaddrBuffer, lpdwBufferLength , lpAliasBuffer, 
			lpdwAliasBufferLength ) ;
}			

int WINAPI l__WSALookupServiceBegin( LPWSAQUERYSETA lpqsRestrictions,DWORD dwControlFlags,
    HANDLE * lphLookup )
{

	if ( !winsock_real_init(TRUE) )
		return -1;

	return  (* winsock_wsalookupservicebegin ) ( lpqsRestrictions, dwControlFlags, lphLookup );
}    

int WINAPI l__WSALookupServiceNext( HANDLE hLookup, DWORD dwControlFlags, LPDWORD lpdwBufferLength,
	LPWSAQUERYSETW   lpqsResults )
{

	if ( !winsock_real_init(TRUE) )
		return -1;

	return  (* winsock_wsalookupservicenext )( hLookup, dwControlFlags, lpdwBufferLength, lpqsResults ) ;


}
int WINAPI  l__WSALookupServiceEnd( HANDLE  hLookup )
{

	if ( !winsock_real_init(TRUE) )
		return -1;

	return  (* winsock_wsalookupserviceend ) (hLookup );

}
