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
 *	Module: $Id: ls_findbad.c,v 1.6 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function: ls_findbad(select_mask)
 *
 *	Description:	Find bad descriptor and close it.
 *
 *	Parameters:	(SELECT_MASK) select_mask - Descriptors selected
 *
 *	Return:		(int) - File descriptor that was closed, or -1 if
 *				no bad descriptors found.
 *			(SELECT_MASK) select_mask - Updated for new/broken 
 *								connections.
 *
 *	Find and close bad descriptors
 *	This is done by selecting on each descriptor in select_mask, until
 *	all that produce EBADF are found and closed.
 *
 *	M. Christiano
 *	11/4/88
 *
 *	Last changed:  12/26/96
 *
 */

#include "lmachdep.h"
#ifndef PC
#include <sys/time.h>
#endif
#include <errno.h>
#ifdef PCRT
#include <sys/types.h>
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"


ls_findbad(select_mask)
SELECT_MASK select_mask;
{
  SELECT_MASK x, ready_mask;
  int fd = 0;
  int nrdy,local_errno;
  int closed_fd = LM_BAD_SOCKET;
  struct timeval tout;

	tout.tv_sec = tout.tv_usec = 0;
	MASK_CREATE(ready_mask);
	MASK_CREATE(x);
	MASK_COPY(ready_mask, select_mask);	/* Try everything */

	while (ANY_SET((int *)ready_mask))
	{
#ifdef USE_WINSOCK
		/*	
		 *	We could use the while() like unix, however,
 		 * 	fd on NT can be any 32 bit value, which would
		 * 	make the while() take a long long time.
		 */
		fd = ((fd_set *)ready_mask)->fd_array[0];
#else
		while ((IS_SET((int *)ready_mask, fd)) == 0) 
			fd++;
#endif /* USE_WINSOCK */
		
		MASK_CLEAR(ready_mask, (unsigned)fd);	/* Clear this one */
		MASK_INIT(x, fd);
#ifdef USE_WINSOCK
		nrdy = l_select(0, x,  0, 0, &tout);
#else		
		nrdy = l_select(lm_nofile, (int *)x, (int *) 0, (int *) 0, &tout);
#endif
		local_errno=net_errno;
		if (nrdy<0 && ((local_errno==NET_EBADF)||
				(local_errno==NET_ENOTSOCK)))
		{
			closed_fd = fd;
			DLOG(("Closing (bad) fd %d (EBADF)\n", fd));
			ls_down((LM_SOCKET *)&fd, "findbad");
			break;
		}
	}
	MASK_DESTROY(ready_mask);
	MASK_DESTROY(x);
	return(closed_fd);
}
