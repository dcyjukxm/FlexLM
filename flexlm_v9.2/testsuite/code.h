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
 *	Module: $Id: code.h,v 1.16 2003/03/03 23:55:49 sluu Exp $
 *
 *	Description: vendor code data for testsuite
 *
 *	M. Christiano
 *	2/25/91
 *
 *	Last changed:  12/21/98
 *
 */

/*
 *	Vendor code for FLEXlm lm_checkout() call
 */
#define ENCRYPTION_SEED1 0x87654321
#define ENCRYPTION_SEED2 0x12345678

#define ENCRYPTION_SEED3 0x11223344 
#define ENCRYPTION_SEED4 0x55667788 
#define LM_SIGN_LEVEL LM_SIGN2
#include "lm_code.h"
#undef LM_STRENGTH
#define LM_STRENGTH LM_STRENGTH_PUBKEY
#include "lmclient.h"
#define ALT_ENCRYPT_CODE_1 0x77774444
#define ALT_ENCRYPT_CODE_2 0x44447777
/*-
 *	Generate the FLEXlm vendor keys with:
 *		lmvkey -v demo -d (+18 months) -p ALL -c TESTSUITE
 */


#undef VENDOR_KEY1
#undef VENDOR_KEY2
#undef VENDOR_KEY3
#undef VENDOR_KEY4
#undef VENDOR_KEY5






#define VENDOR_KEY1 0xc155fac3
#define VENDOR_KEY2 0xcc19ee40
#define VENDOR_KEY3 0x5d3344d4
#define VENDOR_KEY4 0x0db45d64
#define VENDOR_KEY5 0x0b165dcb
/* altcode used to test LM_A_ALT_ENCRYPTION */
LM_CODE(altcode, ALT_ENCRYPT_CODE_1, ALT_ENCRYPT_CODE_2, VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

/* demof */

#define VENDOR_KEYf_1 0xc155faa5
#define VENDOR_KEYf_2 0xcc19ee26
#define VENDOR_KEYf_3 0x5d3344b2
#define VENDOR_KEYf_4 0x0db45d02
#define VENDOR_KEYf_5 0x6d165dcb
LM_CODE(codef, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEYf_1, VENDOR_KEYf_2, VENDOR_KEYf_3, 
				VENDOR_KEYf_4, VENDOR_KEYf_5);


/* demof2 */

#define VENDOR_KEYf2_1 0xc155b661
#define VENDOR_KEYf2_2 0xcc19dc37
#define VENDOR_KEYf2_3 0x5d3308fe
#define VENDOR_KEYf2_4 0x0db46f02
#define VENDOR_KEYf2_5 0x6d245dcb
LM_CODE(codef2, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEYf2_1, VENDOR_KEYf2_2, VENDOR_KEYf2_3, 
				VENDOR_KEYf2_4, VENDOR_KEYf2_5);
#undef LM_BORROW_OK
#define LM_BORROW_OK 1
