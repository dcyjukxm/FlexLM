/******************************************************************************

	    COPYRIGHT (c) 2000, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_pubkey.h,v 1.9 2003/01/13 22:13:12 kmaclean Exp $
 *
 *	Description:  public key info
 *
 *	D. Birns
 *	1/11/00
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_strkey.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "curves.h" /* Security Builder curves header file. */
#include "lcurves.h" /* Security Builder curves header file. */
#include "sbini.h"  /* Security Builder initialization API header file. */ 
#include "sbwrap.h" /* Security Builder key wrapping API header file. */
#include "sbdsa.h"  /* Security Builder ECDSA API header file. */
#if 0
#include "sbdes.h"  /* Security Builder DES API header file. */
#endif
#include "sbrc.h"   /* Security Builder error return code header file. */

#define LM_PUBKEY_CURVE79BIT 	ec79b01 
#define LM_PUBKEY_CURVE97BIT 	ec97b01 
#define LM_PUBKEY_CURVE113BIT 	sect113r1
#define LM_PUBKEY_CURVE163BIT 	ec163a02
#define LM_PUBKEY_CURVE239BIT 	ec239a03

void l_pubkey_err(LM_HANDLE *job, int minor, int status);
#define L_VERIFY_MEM keymem3
