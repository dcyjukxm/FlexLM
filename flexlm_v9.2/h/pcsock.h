/*****************************************************************************r

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	     Module: $Id: pcsock.h,v 1.2 2003/01/13 22:13:13 kmaclean Exp $
 *
 *	Description:	FLEXlm definitions.
 *
 *	M. Christiano
 *	2/13/88
 *
 *	 Last changed:  9/25/98
 *
 */


#ifndef PCSOCK_H
#define PCSOCK_H

#include <winsock.h>
#if !defined( NLM ) && !defined(PC16)
#include <nspapi.h>
#endif
/*
 * We now define the WINSOCK wrapper API.
 */
#ifndef NLM
//#define WINAPI PASCAL FAR
//#endif /* NLM */

int WINAPI l____WSAFDIsSet(SOCKET s, fd_set *fd);
SOCKET WINAPI l__accept (SOCKET s, struct sockaddr FAR *addr, int *len);
int WINAPI l__bind (SOCKET s, const struct sockaddr FAR * s2, int len);
int WINAPI l__closesocket (SOCKET s);
int WINAPI l__connect (SOCKET s, const struct sockaddr *name, int namelen);
struct hostent * WINAPI l__gethostbyname(const char * name);
int WINAPI l__gethostname (char * name, int namelen);
struct servent * WINAPI l__getservbyname(const char * name, 
                                             const char * proto);
int WINAPI l__getsockname (SOCKET  s,  struct sockaddr * s2, int * len);
u_short WINAPI l__htons (u_short hostshort);
u_long WINAPI l__htonl (u_long hostlong);
int WINAPI l__ioctlsocket (SOCKET s, long cmd, u_long *argp);
int WINAPI l__listen (SOCKET s, int backlog);
u_long WINAPI l__ntohl (u_long hostulong);
u_short WINAPI l__ntohs (u_short netshort);
int WINAPI l__recv (SOCKET s, char * buf, int len, int flags);
int WINAPI l__recvfrom (SOCKET s, char *buf, int len, int flags,
		struct sockaddr *from, int *fromlen );
int WINAPI l__send (SOCKET s, const char * buf, int len, int flags);
int WINAPI l__sendto (SOCKET s, const char *buf, int len, int flags,
		const struct sockaddr *to, int tolen );
int WINAPI l__setsockopt (SOCKET s, int level, int optname,
		const char *optval, int optlen );
SOCKET WINAPI l__socket (int af, int type, int protocol);
int WINAPI l__WSACleanup(void);
int WINAPI l__WSAGetLastError(void);
void WINAPI l__WSASetLastError (int iError);
int WINAPI l__WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);
int WINAPI l__shutdown(SOCKET s, int how);
struct hostent * WINAPI l__gethostbyaddr(const char * addr,int len, int type);
/*
 * The select call has parameters more like the UNIX style of select.
 * These FD_SET is then translated inside the l__select call.
 */

int WINAPI l__select (int nfds, int *readfds, int *writefds,
                    int *exceptfds, const struct timeval FAR *timeout);
#if !defined ( NLM ) && ! defined (PC16)
#if defined( _WINSOCK2API_ )

int WINAPI l__WSALookupServiceBegin( LPWSAQUERYSETA lpqsRestrictions,DWORD dwControlFlags,
    LPHANDLE  lphLookup );

int WINAPI l__WSALookupServiceNext( HANDLE hLookup, DWORD dwControlFlags, LPDWORD lpdwBufferLength,
	LPWSAQUERYSETA   lpqsResults );

int WINAPI  l__WSALookupServiceEnd( HANDLE  hLookup );
#else
int WINAPI l__EnumProtocols (int * lpiProtocols, void * lpProtocolBuffer, DWORD * lpBufferlen);
int WINAPI l__GetAddressByName ( DWORD dwNameSpace, GUID * lpServiceType, char *lpServiceName ,
	int * lpiProtocols , DWORD dwResolution, SERVICE_ASYNC_INFO * lpServiceAsyncInfo ,
	void * lpCsaddrBuffer, DWORD * lpdwBufferLength, char * lpAliasBuffer ,
	DWORD * lpdwAliasBufferLength );
#endif /* WINSOCK2 */
#endif /* Not NLM */
/*
 * These macros will prevent any calls directly into the the Winsock
 * DLL.
 */

#define __WSAFDIsSet(a,b)      l____WSAFDIsSet(a,b)
#define accept(a,b,c)          l__accept(a,b,c)
#define bind(a,b,c)            l__bind(a,b,c)
#define closesocket(a)         l__closesocket(a)
#define connect(a,b,c)         l__connect(a,b,c)
#define gethostbyname(a)       l__gethostbyname(a)
#define gethostname(a,b)       l__gethostname(a,b)
#define getservbyname(a,b)     l__getservbyname(a,b)
#define getsockname(a,b,c)     l__getsockname(a,b,c)
#define htons(a)               l__htons(a)
#define htonl(a)               l__htonl(a)
#define ioctlsocket(a,b,c)     l__ioctlsocket(a,b,c)
#define listen(a,b)            l__listen(a,b)
#define ntohl(a)               l__ntohl(a)
#define ntohs(a)               l__ntohs(a)
#define recv(a,b,c,d)          l__recv(a,b,c,d)
#define recvfrom(a,b,c,d,e,f)  l__recvfrom(a,b,c,d,e,f)
#define select(a,b,c,d,e)      l__select(a,b,c,d,e)
#define send(a,b,c,d)          l__send(a,b,c,d)
#define sendto(a,b,c,d,e,f)    l__sendto(a,b,c,d,e,f)
#define setsockopt(a,b,c,d,e)  l__setsockopt(a,b,c,d,e)
#define shutdown(a,b)          l__shutdown(a,b)
#define socket(a,b,c)          l__socket(a,b,c)
#define WSACleanup()           l__WSACleanup()
#define WSAGetLastError()      l__WSAGetLastError()
#define WSASetLastError(a)     l__WSASetLastError(a)
#define WSAStartup(a,b)        l__WSAStartup(a,b)
#define gethostbyaddr(a,b,c)   l__gethostbyaddr(a,b,c)
#ifdef EnumProtocols
#undef EnumProtocols
#endif
#define EnumProtocols(a,b,c)   l__EnumProtocols(a,b,c)
#ifdef GetAddressByName
#undef GetAddressByName
#endif
#define GetAddressByName(a,b,c,d,e,f,g,h,i,j) l__GetAddressByName(a,b,c,d,e,f,g,h,i,j)  


#ifdef WSALookupServiceBegin
#undef WSALookupServiceBegin
#define WSALookupServiceBegin(a,b,c)   l__WSALookupServiceBegin(a,b,c)
#endif
 
#ifdef WSALookupServiceNext
#undef WSALookupServiceNext
#define WSALookupServiceNext(a,b,c,d)  l__WSALookupServiceNext(a,b,c,d)
#endif

#define WSALookupServiceEnd(a)  l__WSALookupServiceEnd(a)



#endif /*NLM */
#ifdef NLM
#define __WSAFDIsSet(a,b)      nlm_WSAFDIsSet(a,b)
#define accept(a,b,c)          nlm_accept(a,b,c)
#define bind(a,b,c)            nlm_bind(a,b,c)
#define closesocket(a)         nlm_closesocket(a)
#define connect(a,b,c)         nlm_connect(a,b,c)
#define gethostbyname(a)       nlm_gethostbyname(a)
#define gethostname(a,b)       nlm_gethostname(a,b)
#define getservbyname(a,b)     nlm_getservbyname(a,b)
#define getsockname(a,b,c)     nlm_getsockname(a,b,c)
#define htons(a)               nlm_htons(a)
#define htonl(a)               nlm_htonl(a)
#define ioctlsocket(a,b,c)     nlm_ioctlsocket(a,b,c)
#define listen(a,b)            nlm_listen(a,b)
#define ntohl(a)               nlm_ntohl(a)
#define ntohs(a)               nlm_ntohs(a)
#define recv(a,b,c,d)          nlm_recv(a,b,c,d)
#define recvfrom(a,b,c,d,e,f)  nlm_recvfrom(a,b,c,d,e,f)
#define select(a,b,c,d,e)      nlm_select(a,b,c,d,e)
#define send(a,b,c,d)          nlm_send(a,b,c,d)
#define sendto(a,b,c,d,e,f)    nlm_sendto(a,b,c,d,e,f)
#define setsockopt(a,b,c,d,e)  nlm_setsockopt(a,b,c,d,e)
#define shutdown(a,b)          nlm_shutdown(a,b)
#define socket(a,b,c)          nlm_socket(a,b,c)
#define WSACleanup()           nlm_WSACleanup()
#define WSAGetLastError()      nlm_WSAGetLastError()
#define WSASetLastError(a)     nlm_WSASetLastError(a)
#define WSAStartup(a,b)        nlm_WSAStartup(a,b)
#endif /*NLM */
#endif /*PC_SOCK_H */

