/******************************************************************************

	    COPYRIGHT (c) 1991, 2003 by Macrovision Corporation.
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
 *	Module: $Id: utilscode.h,v 1.12 2003/03/03 23:55:50 sluu Exp $
 *
 *	Description: FLEXlm vendor codes for utilities.
 *
 *	M. Christiano
 *	5/16/90
 *
 *	Last changed:  12/21/98
 *
 */

/*-
 *	Use a JUNIOR key for lmdown, generate as follows:
 		lmvkey -v lmgrd -p ALL -c JUNIOR
 *
 *	(We only need new keys if the encryption algorithm changes)
 */

#define LM_VER_BEHAVIOR LM_BEHAVIOR_CURRENT
#define LM_STRENGTH LM_STRENGTH_LICENSE_KEY
#define CRO_KEY1 0
#define CRO_KEY2 0


LM_CODE(code, 0, 0, 
	/*- FLEXlm keys: */ 
	 0xb1de5081, 0x72f9b7d2, 0xb8e4a32a, 0x1b221d41, 0x671e57ea);

