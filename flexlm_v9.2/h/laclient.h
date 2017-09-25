/******************************************************************************

	    COPYRIGHT (c) 1997 ,2003 by Macrovision Corporation.
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
 *	Module: $Id: laclient.h,v 1.2 2003/01/13 22:13:11 kmaclean Exp $
 *
 *	Description: la_xxx prototypes
 *
 *	D. Birns
 *	6/24/97
 *
 *	Last changed:  10/7/97
 *
 */

int API_ENTRY la_reread lm_args(( LM_HANDLE_PTR, LM_CHAR_PTR, LMGRD_STAT_PTR, 
			LMGRD_STAT_PTR_PTR));
int API_ENTRY la_shutdown_vendor lm_args(( LM_HANDLE_PTR, LM_CHAR_PTR));
