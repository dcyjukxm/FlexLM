/******************************************************************************

	    COPYRIGHT (c) 1995, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lgclient.c,v 1.2 2003/01/13 22:41:44 kmaclean Exp $
 *
 *	Function: lg_encrypt()
 *
 *	Description: IMPORTANT -- this must never be a public function in
 *		     a DLL, since it contains vendor's seeds.
 *
 *	Parameters: msg, seeds
 *
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  12/10/95
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#ifdef LM_GENERIC_VD
#include "lm_comm.h"
#include "lg_code.h"
#include "lg_copy.h"

static void repeat_str lm_args((char *, int, char *));

void
lg_encrypt(msg, seed1, seed2)
char *msg;
unsigned long seed1, seed2;
{
  int i, j;
  char str[LM_MSG_LEN];
  char keys[LM_MSG_LEN];
  char seeds[(MAX_LONG_LEN * 2) + 2];
  int len;

	memset(msg, 0, LM_MSG_LEN);

/*
 *	Set up the plain text message first: cmd and sseds
 */
	msg[MSG_CMD] = LM_SEND_SEEDS;
	sprintf(seeds, "%x %x ", seed1, seed2);
/*
 *	repeatedly copy the seed string over the msg
 */
	repeat_str(seeds, LM_MSG_LEN - MSG_DATA, &msg[MSG_DATA]);
/* 
 *	Xor the copyright over the string
 */
	repeat_str(lgcopyright, LM_MSG_LEN - MSG_DATA, &msg[MSG_DATA]);
/* 
 *	NOTE:  Now Xor the flexlmd seeds
 */
	sprintf(keys, "%x%x", FREE_SEED_1^VENDOR_KEY5, 
					FREE_SEED_2^VENDOR_KEY5);
	len = strlen(keys);
	repeat_str(keys, LM_MSG_LEN - MSG_DATA, &msg[MSG_DATA]);
/* 
 *	reverse the result -- we may want to do something more complicated 
 *	later.
 */
	for (i = MSG_DATA, j = LM_MSG_LEN  - 1; j >= MSG_DATA; i++, j--)
		str[j] = msg[i];
	memcpy(&msg[MSG_DATA], &str[MSG_DATA], sizeof(str) - MSG_DATA);
}
static
void
repeat_str(str, len, ptr)
char *str;
int len;
char *ptr;
{
  int i, j, l= strlen(str);

	for (i = 0; i < len; i++)
		for (j = 0; j < l && (i < len); j++, i++)
			ptr[i] ^= str[j];
}
#endif /* GENERIC */
