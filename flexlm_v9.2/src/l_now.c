/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_now.c,v 1.9 2003/01/14 21:46:55 kmaclean Exp $
 *
 *	Function: l_now
 *
 *	Description: 	gets current time in milliseconds
 *
 *	Parameters:	none
 *
 *	Return:		(long) time -- this is low order bits of
 *					seconds + milliseconds.
 *					not useful for date.
 *					low order bits should be
 *					random
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  5/21/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_time.h"
#ifdef PC
#include <winsock.h>
#endif

long
l_now()
{
  struct timeval nowtv;
  struct timezone tz; /* unused */
  unsigned long l;

/*-
 *	this returns a bit more than a day's worth of milliseconds 
 */

    (void)l_gettimeofday(&nowtv, &tz);
	l = (nowtv.tv_sec % 10000) * 1000;
	l += (nowtv.tv_usec / 1000);
	return l; 
}
