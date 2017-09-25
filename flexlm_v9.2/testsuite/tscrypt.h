/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *	Module: $Id: tscrypt.h,v 1.3 2003/01/13 22:55:13 kmaclean Exp $
 *	Include this right after #include lm_code.h in modified lmcrypt.c
 *
 */

#include "../vendor/goodkeys"
#undef LM_STRENGTH
#define LM_STRENGTH LM_STRENGTH_PUBKEY
