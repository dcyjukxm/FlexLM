/******************************************************************************

	    COPYRIGHT (c) 1991, 2003 by Macrovision Corporation.
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
 *	Module: $Id: luhostid.c,v 1.6 2003/05/05 16:10:55 sluu Exp $
 *
 *	Function: lu_gethostid()
 *
 *	Description: 4.2bsd functions that are "missing" on various
 *		     FLEXlm platforms.
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *
 *	M. Christiano
 *	11/27/91
 *
 *	Last changed:  3/10/98
 *
 */

#include "lmachdep.h"

#ifdef NO_gethostid

#ifndef VXWORKS
#include <sys/utsname.h>
#include <sys/systeminfo.h>

long
lu_gethostid()
{
  char buf[101];
  long hostid;

/*
 *	gethostid replacement
 */
	sysinfo(SI_HW_SERIAL, buf, 100);
	sscanf(buf, "%ld", &hostid);	/* overrun checked */
	return(hostid);
}
#else 
/* VXWORKS version */
long lu_gethostid()
{
	/* HACK for now. */
	return 0;
}	

#endif 

#endif /* NO_gethostid */
