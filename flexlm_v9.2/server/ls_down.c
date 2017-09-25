/******************************************************************************

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
 *	Module: $Id: ls_down.c,v 1.5 2003/04/18 23:47:59 sluu Exp $
 *
 *	Function: ls_down(fd, msg)
 *		  ls_sock_close(fd, msg)
 *
 *	Description:	Shuts down and closes a socket.
 *			ls_sock_close doesn't do the shutdown.
 *
 *	Parameters:	(int *) fd - file descriptor that was closed. 
 *			(char *) msg - Message for DLOG, if failure.
 *
 *	Return:		None
 *
 *	M. Christiano
 *	11/11/88
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include <errno.h>
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif

void ls_down(fd, msg)
LM_SOCKET *fd;
char *msg;
{

#ifndef RELEASE_VERSION
	if (l_real_getenv("LS_DOWN_DEBUG"))
		DLOG(("lsdown fd %d msg %s\n", *fd, msg)); 
#endif
	if (*fd != LM_BAD_SOCKET)
	{

#if !defined(MOTO_88K) && !defined(PC)
		if (shutdown(*fd, 2))
		{
#if 0
			DLOG(("shutdown error %s (%d): %s\n", msg, *fd, 
						SYS_ERRLIST(errno)));
#endif
		}
#endif
	}
	ls_sock_close(fd, msg);
}
/*
 *	ls_sock_close -- close a socket fd, and set to INVALID
 */
void ls_sock_close(fd, msg)
LM_SOCKET *fd;
char *msg;
{
	if (*fd != LM_BAD_SOCKET)
	{
#ifndef RELEASE_VERSION
		if (l_real_getenv("LS_DOWN_DEBUG")) 
			DLOG(("ls_sock_close fd %d\n", *fd)); 
#endif
		if (network_close(*fd))
		{
			DLOG(("close error %s (%d): %s\n", msg, *fd, 
						SYS_ERRLIST(errno)));
		}
	}
	ls_mask_clear( (int) *fd );
	*fd = LM_BAD_SOCKET;
}
