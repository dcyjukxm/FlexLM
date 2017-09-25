/******************************************************************************
 *
 *      Module: $Id: lm_code.h,v 1.47 2003/03/03 23:55:47 sluu Exp $
 *
 *          COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
 *      This software has been provided pursuant to a License Agreement
 *      containing restrictions on its use.  This software contains
 *      valuable trade secrets and proprietary information of
 *      Macrovision Corporation and is protected by law.  It may
 *      not be copied or distributed in any form or medium, disclosed
 *      to third parties, reverse engineered or used in any manner not
 *      provided for in said License Agreement except with the prior
 *      written authorization from Macrovision Corporation.
 ******************************************************************************
 *
 *      Description:    Used to generate lm_new.o/.obj and by license-
 *                      generators.
 *
 *      Once the kit is "built" (using make or nmake) this file is no longer
 *	needed, but should be stored somewhere safe.
 *
 *	Set the following values:
 *      VENDOR_KEY1-5 		Provided by Macrovision .
 *      VENDOR_NAME 		If not evaluating, set to vendor name.
 *      LM_SEED1-3 		Make up 3 32-bit numbers, (or use
 *				'lmrand1 -seed' to make up), keep secret, safe,
 *				and never change.
 *	CRO_KEY1-2 		Provided by Macrovision if CRO used.
 *      LM_STRENGTH: 		If using CRO, set to desired length
 *
 *	Upgrading: 		from version older than 8.1: Copy your
 *				ENCRYPTION_SEEDs from the old lm_code.h file.
 *				Make sure LM_STRENGTH matches, if you were
 *				using LM_STRENGTH
 */

#ifndef LM_CODE_H
#define LM_CODE_H
#include "lm_cro.h"
/*
 * 	Vendor keys:   		Enter keys received from Macrovision .
 *				Changing keys has NO impact on license files
 *				(unlike LM_SEEDs).
 */
#define VENDOR_KEY1 0xc945f9e3
#define VENDOR_KEY2 0x45083d28
#define VENDOR_KEY3 0xd42a1e0d
#define VENDOR_KEY4 0x16a59630
#define VENDOR_KEY5 0x0b165dc9
/*
 * 	Vendor name.  		Leave "demo" if evaluating.  Otherwise,
 *			 	set to your vendor daemon name.
 */
#define VENDOR_NAME "demo"
/*
 * 	Private SEEDs: 		Make up 3, 8-hex-char numbers, replace and
 *				guard securely.  You can also use the command
 *				'lmrand1 -seed' to make these numbers up
 */
#define LM_SEED1 0x12345678
#define LM_SEED2 0x87654321
#define LM_SEED3 0xabcdef01
/*
 *	Pick an LM_STRENGTH:
 */
#define LM_STRENGTH LM_STRENGTH_DEFAULT
/*
 *			      	If you're not using CRO, leave this as
 *			       	LM_STRENGTH_DEFAULT. If you're upgrading from
 *			       	pre-v7.1, and want no changes, set this to
 *			       	LM_STRENGTH_LICENSE_KEY.
 *			      	-----------------------------------------------
 *			       	LM_STRENGTH Options are
 *			       	LM_STRENGTH_DEFAULT:
 *			      	 Public key protection unused. Use SIGN=
 *			      	 attribute. sign length = 12
 *			       	CRO:
 *			       	LM_STRENGTH_113BIT, LOW:   sign length= 58 chars
 *			       	LM_STRENGTH_163BIT, MEDIUM:sign length= 84 chars
 *			       	LM_STRENGTH_239BIT, HIGH:  sign length=120 chars
 *				Pre-v7.1:
 *			       	LM_STRENGTH_LICENSE_KEY:   Use old license-key
 */
/*
 *	CRO Keys:		Provided by Macrovision if CRO used
 * 				Turned off by default.  Be sure to set
 * 				LM_STRENGTH above if CRO_KEYs are non-zero
 */
#define CRO_KEY1    0x83f480c3
#define CRO_KEY2    0x771cc02c


#include "lm_code2.h"
#endif /* LM_CODE_H */
