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
 *	Module: $Id: lu_getdtab.c,v 1.3 2003/01/13 22:41:48 kmaclean Exp $
 *
 *	Function: lu_getdtablesize()
 *
 *	Description: 4.2bsd functions that are "missing" on various
 *		     FLEXlm platforms.
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  1/22/97
 *
 */

#include "lmachdep.h"
#ifdef RLIMIT_FOR_GETDTABLESIZE
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef PC
#include "winsock.h"
#endif

#ifdef SYSCONF_FOR_GETDTABLESIZE
#ifdef DGUX
#ifdef SVR4
#include <sys/_int_unistd.h>
#else
#include <sys/m88kbcs.h>
#endif
#endif  /* DGUX */
#if defined( MOTO_88K) || defined(MOTOSVR4)
#include <unistd.h>
#endif  /* MOTO_88K */

long sysconf();
#endif

#undef GOTONE
#ifdef NO_getdtablesize

#if !defined(PC) && !defined(VXWORKS)
#include <sys/param.h> /* some systems need this for NOFILE */
#endif

lu_getdtablesize()
{
#ifdef RLIMIT_FOR_GETDTABLESIZE
#define GOTONE
	
  struct rlimit limit;

	errno = 0;
	getrlimit(RLIMIT_NOFILE, &limit);
	if (errno) return(NOFILE);
	else	   return((int) limit.rlim_cur);
#endif
#ifdef SYSCONF_FOR_GETDTABLESIZE
#define GOTONE

	return((int) sysconf(_SC_OPEN_MAX) - 1);
#endif

#ifdef PC
#define GOTONE
	return (FD_SETSIZE);
#endif /*pc */

#ifndef GOTONE
	return(NOFILE);
#endif
}

#endif
