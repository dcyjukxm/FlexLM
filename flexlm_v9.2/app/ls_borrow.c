/******************************************************************************

	    COPYRIGHT (c) 2001, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_borrow.c,v 1.22 2003/01/13 22:22:34 kmaclean Exp $
 *
 *	D. Birns
 *	2/01
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lgetattr.h"
#include "l_prot.h"
#include "ls_sprot.h"
#include "lsserver.h"
#include "lsfeatur.h"	/* Dynamic feature data */
#include "ls_aprot.h"	
#include "l_rand.h"	
#include "l_borrow.h"

void
ls_borrow_encrypt(char *str, int len, unsigned short salt)
{
  unsigned char *cp;
  unsigned int b1, b2;
  int i;
  char tmp;
  extern int (*L_NEW_JOB)();
  typedef unsigned int (*_borrowcb)
			lm_args((LM_VOID_PTR, LM_VOID_PTR, int , 
				unsigned char *, int, int *));
  int seeds[3];
  int crypt_seeds[3];
  _borrowcb borrowcb;
  int round;
  int eval = 0;

	borrowcb = (_borrowcb)L_NEW_JOB;
	b1 = borrowcb(0, 0, 2, 0, 0, 0);
	b2 = borrowcb(0, 0, 3, 0, 0, 0);
	if (!b1)
	{
		eval = 1;
		b1 = 1234;
		b2 = 5678;
	}
	
/* 
 *	Initialize the seeds
 */
	/*fprintf(stdout, "b1 %x b2, %x salt = %x len = %d\n", b1, b2, salt, len);*/
	salt += len;
	srand16((b1 + b2 - salt) & 0xffff, (b1 ^ b2 - salt) & 0xffff,
		((b2 + b2 - salt) >> 16) & 0xffff, crypt_seeds);
	srand16((b1 + b2 + salt) & 0xffff, (b1 ^ b2 + salt) & 0xffff,
		((b2 + b2 + salt) >> 16) & 0xffff, seeds);

/* 
 *	first encrypt with additions 
 */
	for (round = 0; round < L_BCRYPT_ROUNDS; round++)
	{
		rand16_2(crypt_seeds);
		rand16_2(crypt_seeds);

		for (cp = (unsigned char *)str, i = 0;i < len; i++, cp++)
		{
		  unsigned char c = rand16_2(crypt_seeds) & 0xff;
			*cp += c;
		}

/* 
 *		now shuffle the bytes 
 */
		rand16_2(seeds);
		rand16_2(seeds);
		rand16_2(seeds);
		for (cp = (unsigned char *)str, i = 0;i < (len - 1); i++, cp++)
		{
			if (rand16_2(seeds) % 2)
			{
				tmp = cp[0]; cp[0] = cp[1]; cp[1] = tmp; /*swap bytes*/
			}

		}
	}
	
}
