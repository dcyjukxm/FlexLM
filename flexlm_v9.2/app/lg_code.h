/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/*-
 *	Module: $Id: lg_code.h,v 1.3 2003/01/13 23:29:07 kmaclean Exp $
 *
 *	Description: 	Encryption codes to be used in a VENDORCODE macro 
 *			for FLEXlm daemons, create_license, lm_init(),
 *			and lm_checkout() call - modify these values 
 *			for your own use.  (The VENDOR_KEYx values
 *			are assigned by Macrovision Corporation).
 *
 *	example LM_CODE() macro:
 *
 *		LM_CODE(var_name, ENCRYPTION_SEED1, ENCRYPTION_SEED2,
 *					VENDOR_KEY1, VENDOR_KEY2, 
 *					VENDOR_KEY3, VENDOR_KEY4);
 *	Last changed:  10/23/96
 *
 */


#define FREE_SEED_1 0x54b23e83
#define FREE_SEED_2 0x30b07109

#define UPGRADE_SEED_1 0x3c306a3a
#define UPGRADE_SEED_2 0x18b1579e

/*
 *	FLEXlm vendor keys.
 *	Changing these keys has NO impact on license files (unlike 
 *	the CODEs).
 */
/*-
 *	Generate these keys with: lmvkey -v flexlmd -p ALL -c FLEXLMD
 */

#define VENDOR_KEY1 0xa68e9205 
#define VENDOR_KEY2 0x0c673804 
#define VENDOR_KEY3 0xb6032c85 
#define VENDOR_KEY4 0x85259c01 
#define VENDOR_KEY5 0xa2f2866e

/*
 *	FLEXlm vendor name
 */

#define VENDOR_NAME "flexlmd"

/*-
 *	Generate these keys with: 
 *		lmvkey -v flexlmd -p ALL -c FLEXLMD_UPGRADE_KEYS
 */

#define UPGRADE_KEY1 0xa68e9225 
#define UPGRADE_KEY2 0x0c673868 
#define UPGRADE_KEY3 0xb6032cea 
#define UPGRADE_KEY4 0x85259c6e 
#define UPGRADE_KEY5 0xa2f2866d
