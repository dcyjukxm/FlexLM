/******************************************************************************

	    COPYRIGHT (c) 1997, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lm_clean.c,v 1.2 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function: 	lc_cleanup
 *
 *	Description: 	For windows only at this time.
 *
 *	Parameters: 	None
 *
 *	Return:		None
 *
 *	D. Birns
 *	9/4/97
 *
 *	Last changed:  11/12/97
 *
 */


#include "lmachdep.h"
#include "lmclient.h"

void API_ENTRY
lc_cleanup()
{
/* P2909 */
#ifdef PC
#include "pcsock.h"

	WSACleanup();
#ifdef WINNT
	lc_cleanup_internal();
#endif

#endif
}
