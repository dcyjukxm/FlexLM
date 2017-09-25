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
 *	Module: $Id: lu_gethost.c,v 1.2 2003/01/13 22:41:48 kmaclean Exp $
 *
 *	Function: lu_gethostname()
 *
 *	Description: 4.2bsd functions that are "missing" on various
 *		     FLEXlm platforms.
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  11/13/98
 *
 */

#include "lmachdep.h"

#if defined(VMS)
#include <stdio.h>
#define NEED_GETHOSTNAME
#endif

#if !defined (RELEASE_VERSION) && !defined (PC)
#define NEED_GETHOSTNAME
#endif

#ifdef NEED_GETHOSTNAME

#if defined( PCRT) | defined(RS6000)
#include <sys/utsname.h>
#endif

#ifdef USE_FLEXLM_DEBUG_GETHOSTNAME

#ifdef getenv
#undef getenv
#endif 
gethostname(name, namelen)
char *name;
int namelen;
{ return(lu_gethostname(name, namelen)); }

#endif

#ifdef getenv
#undef getenv
#endif 

lu_gethostname(name, namelen)
char *name;
int namelen;
{
#if defined( PCRT) | defined(RS6000)
  char *x;
  extern int uname();
  static struct utsname uts_name;
#else
  char *x, *getenv();
#endif

#ifdef VMS
  char *p;

	x = getenv("SYS$NODE");
	if (x)
	{
		p = index(x, ':');
		if (p) *p = '\0';
	}
#else
#if defined( PCRT) | defined(RS6000)
	if (uname(&uts_name) >= 0) 
	{
		x = uts_name.nodename;
	}
	else 
	{
		name[0] = '\0';
		return(-1);
	}
#else
	x = getenv("hostname");
#endif /* PCRT */
#endif /* VMS */
	if (x)
	{
		(void) strncpy(name, x, namelen);
	}
	else
	{
#ifdef VMS
		(void) printf("FLEXlm: No SYS$NODE logical name, exiting\n");
#else
		(void) printf("No \"hostname\" environment variable, exiting\n");
#endif /* VMS */
		(void) exit(1);
	}
	return(0);
}

#endif /* NEED_GETHOSTNAME */
