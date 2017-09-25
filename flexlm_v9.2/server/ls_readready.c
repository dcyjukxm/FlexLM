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
 *	Module: $Id: ls_readready.c,v 1.7 2003/03/12 21:19:19 sluu Exp $
 *
 *	Function: ls_readready(ca, interval)
 *
 *	Description: Checks to see if a read can be done on descriptor "d"
 *
 *	Parameters:	(CLIENT_ADDR *) ca - The address to check
 *			(int) interval - How long to wait.
 *
 *	Return:		(int) - 1 - ready
 *				0 - not ready
 *
 *	M. Christiano
 *	6/3/88
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
#include "lmselect.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"

int
ls_readready(ca, interval)
CLIENT_ADDR *ca;			/* Address */
int interval;		/* How long to wait */
{
  SELECT_MASK select_mask;		/* File descriptors to select on */
  int nrdy;
  struct timeval ready_time;
  int i;

	return l_select_one(ca->addr.fd, 1, interval * 1000);
}
