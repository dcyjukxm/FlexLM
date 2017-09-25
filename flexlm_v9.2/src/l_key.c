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
/*-
 *	Module: $Id: l_key.c,v 1.16 2003/03/12 20:33:04 sluu Exp $
 *
 *	Function: l_key(v, k)
 *
 *	Description: Decodes the FLEXlm vendor key values for a vendor
 *
 *	Parameters:	(char *) v 	- The vendor name
 *			(long *) k 	- FLEXlm VENDOR keys (array)
 *
 *	Return:		(long *) - The decoded vendor keys
 *
 *	M. Christiano
 *	2/8/91
 *
 *	Encryption algorithms adapted from s_crypt.c of "The Software Meter"
 *		s_crypt.c is Copyright (c) 1991, 2003 by Macrovision Corporation
 *
 *	Last changed:  12/4/98
 *
 */
#include "lmachdep.h"

long l_zinit(char *c, int crokey_flag);

/*-
 *	l_hbs() - Half-byte swap a byte
 */
l_hbs(x) int x; {return(((x&0xf0)>>4)|((x&0x0f)<<4));}

/*-
 *	l_br() - Bit-reverse a byte
 */
l_br(x)
int x;
{
	return( (x & 0x80 ? 1    : 0) |
		(x & 0x40 ? 2    : 0) |
		(x & 0x20 ? 4    : 0) |
		(x & 0x10 ? 8    : 0) |
		(x & 0x8  ? 0x10 : 0) |
		(x & 0x4  ? 0x20 : 0) |
		(x & 0x2  ? 0x40 : 0) |
		(x & 0x1  ? 0x80 : 0));
}

/*-
 *	l_icf() - Inverse crypt function
 */

static
long
l_icf(x)
long x;
{
  int i0, i1, i2, i3;
  long o0, o1, o2, o3;

	i0 = (int)(x & 0xff);
	i1 = (int)((x & 0xff00) >> 8);
	i2 = (int)((x & 0xff0000) >> 16);
	i3 = (int)((x & 0xff000000) >> 24);
	o3 = l_hbs(l_br(i2));
	o2 = o3 ^ l_hbs(i3);
	o1 = o3 ^ l_br(i1);
	o0 = o1 ^ l_br(l_hbs(i0));
	return(o0 | (o1 << 8) | (o2 << 16) | (o3 << 24));
}

/*-
 *	l_zinit() - Create an initializing vector "Z" from a vendor
 *		    daemon name.
 */

long
l_zinit(char *c, int crokey_flag)
{
/*-  long z = 0x8a3ef725; 	(v2.2) */
/*-  long z = 0xa73f025b;	(v2.4) */
/*-  long z = 0xd193feb5; 	(v2.7[1-2]) */
/*-  long z = 0xcf53fa74; 	(v2.73 - v4.x) */
/*-  long z = 0x58a340f2; 	(v5.0 - v5.80) */
/*-  long z = 0x1504c935;	(v5.81 - v6.1)  */
/*-  unsigned long z  = 0x788F71D2;	(v7.x)  */
/*-	 unsigned long z = crokey_flag ? 0x67607419 : 0x3cde3ebf; (v8.x ...)  */
  unsigned long z = crokey_flag ? 0x62586954 : 0x72346B53; /*- (v9.x ...) */

  int i = 0;

	while (*c)
	{
		z ^= (((long) *c) << (i * 8));
		c++; i++;
/*-
 *		The '4' following is hard-coded because we're encoding
 *		into a 4-byte space, and this is the most portable way
 *		of specifying that. -- Daniel
 */
		if (i >= 4) i = 0;
	}
	return(z);
}

/*-
 *	l_key() - Decode the FLEXlm vendor keys
 */

void
l_key(v, k, o, len)
char *v;
unsigned long *k;
unsigned long *o;
int len;
{
#if 0
#ifdef FLEXLM_ULTRALITE
  long *o = (long *) LM_FUL_O_PTR;
#else
  static long o[L_KEY_O_LEN];
#endif
#endif
  long z;
  int i;
  int crokeys = k[0] ? 0 : 1; /* if k[0] is 0, then it's crokeys */

	z = l_zinit(v, crokeys);
	for (i =0; i < len; i++)
	{
		if (i)
			o[i] = o[i-1]^k[i-1]^l_icf(k[i]);
		else if (k[i])
			o[i] = z^l_icf(k[i]);
		else
			o[i] = z; /* crokeys */
	}
}


/*-
 *	Counts the number of bits set in an array of 4 longs
 */
l_c(k)
long *k;
{
  int i=3, j=93, x = 0;
  long z;

	for (;i>=0;i--,++k)
	{
		z= *k;
		for (;j>=31;j-=2)
		{
			if (z&0x1)
				x++;
			z>>=1;
		}
	j=93;
	}
	return(x);
}
