/******************************************************************************

	    COPYRIGHT (c) 1988, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lu_sleep.c,v 1.3 2003/01/13 22:41:48 kmaclean Exp $
 *
 *	Function: lu_sleep()
 *
 *	Description: 4.2bsd functions that are "missing" on various
 *		     FLEXlm platforms.
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"

/*
 *	Dummy sleep for PC
 */

#ifdef NO_sleep

lu_sleep(x)
unsigned x;
{
  int i;
	for (i=0; i< 10000*x; i++) ;
}

#endif
