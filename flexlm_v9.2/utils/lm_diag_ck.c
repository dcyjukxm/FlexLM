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
 *	Module: $Id: lm_diag_ck.c,v 1.4 2003/01/13 21:58:48 kmaclean Exp $
 *
 *	Function: 	lmutil_checkout
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  9/30/98
 *
 */
#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif
#include "lmutil.h"
#define NO_ENCRYPTION_CHECK
#define lc_checkout lmutil_checkout
#define l_checkout lmutil_l_checkout
#define l_local_verify_conf lmutil_local_verify_conf
#define l_ckout_ok lmutil_l_ckout_ok
extern void lmutil_l_ckout_ok();
#include "../src/lm_ckout.c"
