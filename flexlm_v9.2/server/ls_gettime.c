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
 *	Module: $Id: ls_gettime.c,v 1.7 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function: ls_gettime()
 *
 *	Description: Gets the time of day
 *
 *	Parameters:	None
 *
 *	Return:		(struct tm *) - the local time of day
 *
 *	M. Christiano
 *	9/2/88
 *
 *	Last changed:  10/18/95
 *
 */

#include <time.h>
#include <sys/types.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

struct tm *
#ifdef THREAD_SAFE_TIME
ls_gettime(struct tm * ptst)
#else /* !THREAD_SAFE_TIME */
ls_gettime()
#endif
{
  time_t d;


	d = time(0);
#ifdef THREAD_SAFE_TIME
	localtime_r(&d, ptst);
	return ptst;
#else /* !THREAD_SAFE_TIME */
	return(localtime(&d));
#endif
}
