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
 *	Module: $Id: l_platfm.c,v 1.2 2003/01/13 22:41:54 kmaclean Exp $
 *
 *	Function: 	l_platform_name()
 *
 *	Return: 	returns (char *) gplatform
 *
 *	Parameters:	none
 *
 *	D. Birns
 *	12/5/95
 *
 *	Last changed:  10/23/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
LM_CHAR_PTR
API_ENTRY
l_platform_name()
{
	return
#ifdef VMS
#ifdef ALPHA_V1
			"alpha_v1";
#endif /* ALPHA_V1 */
#ifdef VAX_V5
			"vax_v5";
#endif /* ALPHA_V1 */
#ifdef VAX_V6
			"vax_v6";
#endif /* VAX_V6 */
#else /* non-vms: */
			GPLATFORM;
#endif /* VMS */
}

