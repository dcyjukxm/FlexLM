/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_update.h,v 1.2 2003/01/13 22:13:13 kmaclean Exp $
 *
 *	Description:  Used by l_update.c
 *
 *	D. Birns
 *	11/29/94
 *
 *	Last changed:  11/29/94
 *
 */
#include "lm_machdep.h"
#include "lm_client.h"
/*- gkey flexlm v4.0 -v gsid -c ALL -p ALL */
#define ECODE1 0x10e46633
#define ECODE2 0x09098383

LM_CODE(code, ECODE1, ECODE2, 0x959355bd, 0xecab1287, 
	0x87738044, 0x62edda6a, 0xcfbf111c);
#define MASK 0x1093256e
