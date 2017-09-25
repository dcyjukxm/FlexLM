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
 *	Module: $Id: ls_badserv.c,v 1.5 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function: ls_badserv(fd)
 *
 *	Description:	Updates the server data for closed file descriptor.
 *
 *	Parameters:	(int) fd - Descriptor that was closed. 
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	11/4/88
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

void
ls_badserv(fd)
int fd;
{
  LM_SERVER *ls;


	for (ls = ls_s_first(); ls; ls = ls->next)
	{
		if (fd == (int)ls->fd1 || fd == (int)ls->fd2)
		{
			DLOG(("ls_badserv calling ls_s_down\n"));
			ls_s_down(ls);
			break;
		}
	}
}
