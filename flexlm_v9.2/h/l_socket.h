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
 *	Module: $Id: l_socket.h,v 1.3 2003/01/13 22:13:12 kmaclean Exp $
 *
 *	Function:	l_socket.h - Abstraction for sockets
 *
 *	M. Christiano
 *	2/27/94
 *
 *	Last changed:  12/26/96
 *
 */

#ifndef USE_WINSOCK

/*
 *	The "normal" socket stuff, for Unix
 */
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef VXWORKS
#include <hostlib.h>
#endif 

#else		/* WINSOCK */
/*-
 *	Geez, are these guys stupid.  Undefine all the stuff which they
 *	shouldn't define in winsock.h
 */
#ifdef FIONREAD
#undef FIONREAD
#undef FIOASYNC
#undef EWOULDBLOCK
#undef EINPROGRESS
#undef EALREADY
#undef ENOTSOCK
#undef EDESTADDRREQ
#undef EMSGSIZE
#undef EPROTOTYPE
#undef ENOPROTOOPT
#undef EPROTONOSUPPORT
#undef EOPNOTSUPP
#undef EAFNOSUPPORT
#undef EADDRINUSE
#undef EADDRNOTAVAIL
#undef ENOTDOWN
#undef ENETUNREACH
#undef ENETRESET
#undef ECONNABORTED
#undef ECONNRESET
#undef ENOBUFS
#undef ENOTCONN
#undef ESHUTDOWN
#undef ETOOMANYREFS
#undef ETIMEDOUT
#undef ECONNREFUSED
#undef ELOOP
#undef ENAMETOOLONG
#undef EHOSTDOWN
#undef EHOSTUNREACH
#undef ENOTEMPTY
#undef EPROCLIM
#undef EUSERS
#undef EDQUOT
#undef ESTALE
#undef EREMOTE
#undef ESOCKTNOSUPPORT
#undef EPFNOSUPPORT
#undef ENETDOWN
#undef EISCONN
#endif /* defined FIONREAD */
#include <pcsock.h>
#endif	/* USE_WINSOCK */
