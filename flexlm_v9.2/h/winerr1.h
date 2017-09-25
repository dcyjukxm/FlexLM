/******************************************************************************

            COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
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
 *      Module: $Id: winerr1.h,v 1.2 2003/01/13 22:13:13 kmaclean Exp $
 *     
 *
 *      Function:       All Winsock API calls which are used by FLEXlm client
 *                      library.
 *
 *      Description:    This file winsock error messages
 *                      
 *                      .
 *
 *      Parameters:     
 *                      
 *
 *      Return:         
 *
 *      Blane Eisenberg
 *
 *      Last changed:  9/26/98
 *
 */


/* WSAEINTR */		"WinSock: Function call interupted", 
			"","","","",	
/* WSAEBADF */		"WinSock: Bad file descriptor", 
			"","","",
/* WSAEACCES */		"WinSock: Access denied", 
/* WSAEFAULT */		"WinSock: Bad address", 
			"","","","","","","",
/* WSAEINVAL */		"WinSock: Invalid argument",
			"", 
/* WSAEMFILE */		"WinSock: Too many open sockets",
			"","","","","","","","","","", 
/* WSAEWOULDBLOCK */	"WinSock: Operation would block", 
/* WSAEINPROGRESS */	"WinSock: Blocking operation already in progress", 
/* WSAEALREADY */	"WinSock: Operation already in progress", 
/* WSAENOTSOCK */ 	"WinSock: Specified socket is invalid", 
/* WSAEDESTADDRREQ */	"WinSock: Destination address required", 
/* WSAEMSGSIZE */	"WinSock: Message too long", 
/* WSAEPROTOTYPE */	"WinSock: Wrong protocol type for socket", 
/* WSAENOPROTOOPT */	"WinSock: Bad protocol option", 
/* WSAEPROTONOSUPPORT*/	"WinSock: Protocol not supported", 
/* WSAESOCKTNOSUPPORT*/	"WinSock: Socket type not supported", 
/* WSAEOPNOTSUPP */	"WinSock: Operation not supported", 
/* WSAEPFNOSUPPORT */	"WinSock: Protocol family not supported",
/* WSAEAFNOSUPPORT */	"WinSock: Address family not supported by protocol family", 
/* WSAEADDRINUSE */ 	"WinSock: Address already in use", 
/* WSAEADDRNOTAVAIL */	"WinSock: Invalid address", 
/* WSAENETDOWN */	"WinSock: Network is down", 
/* WSAENETUNREACH */	"WinSock: Network is unreachable", 
/* WSAENETRESET */	"WinSock: Network dropped connection on reset", 
/* WSAECONNABORTED */	"WinSock: Software caused connection abort", 
/* WSAECONNRESET */	"WinSock: Connection reset by peer", 
/* WSAENOBUFS */	"WinSock: No buffer space available", 
/* WSAEISCONN */	"WinSock: Socket is already connected", 
/* WSAENOTCONN */	"WinSock: Socket is not connected", 
/* WSAESHUTDOWN */	"WinSock: Cannot send after socket shutdown", 
/* WSAETOOMANYREFS */	"WinSock: Too many references", 
/* WSAETIMEDOUT */	"WinSock: Connection timed out", 
/* WSAECONNREFUSED */	"WinSock: Connection refused", 
/* WSAELOOP */		"WinSock: Loop", 
/* WSAENAMETOOLONG */	"WinSock: Name too long", 
/* WSAEHOSTDOWN */	"WinSock: Host is down", 
/* WSAEHOSTUNREACH 10065*/	"WinSock: No route to host",
			"","","","","","","","","","","","","","","","","","","","","","","","","", 
/* WSASYSNOTREADY 10091*/	"WinSock: Network subsystem is unavailable", 
/* WSAVERNOTSUPPORTED*/	"WinSock: WINSOCK.DLL version out of range", 
/* WSANOTINITIALISED 10093*/	"WinSock: Successful WSAStartup not yet performed", 


