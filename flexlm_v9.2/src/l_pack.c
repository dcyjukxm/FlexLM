/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_pack.c,v 1.7 2003/04/16 17:26:36 brolland Exp $
 *
 *	Function:	 l_pack, l_unpack, l_pack_print
 *
 *	Description: 	Routines to pack and unpack and print
 *			bit-formatted buffer.
 *
 *	D. Birns
 *	7/30/97
 *
 *	Last changed:  10/24/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_pack.h"


static int dec_cksum_ok 	lm_args((char *));
static int dec_cksum		lm_args(( char *, int));
static void dec_add_cksum	lm_args(( char *));

unsigned long 
l_unpack(buf, ppos, len) 
unsigned char *buf; 
int *ppos; 
int len;
{
  int rshift, lshift, idx, pos = *ppos;
  long last, next, t;
  long sum, mask;

	*ppos += len;
	if (len == 32) mask = (~0 & 0xffffffff);
#ifndef PC16
	else mask = (1 << len) - 1;	
#else
        else mask = ((long)1 << len) - 1;
#endif
	idx = pos >> 3; 	/* same as "/ 8" */
	rshift = pos & 7;	/* same as "% 8" */
	sum = 0;
	last = buf[idx++] & 0xff;
	lshift = 0;
	while(len > 0) 
	{
		next = buf[idx++] & 0xff;
		t = (last | (next << 8)) >> rshift;
		sum |= t << lshift;
		lshift += 8;
		len -= 8;
		last = next;
	}
	return(sum & mask);
}

void 
l_pack(buf, ppos, len, val) 
unsigned char *buf;  /* whole result buffer */
int *ppos;  /* pointer to offset in buf to put result */
int len;  /* len in bits */
long val; /* value to pack */
{
  int c, mask, shift, pos = *ppos;
  int idx;
	
	*ppos += len;
	shift = pos & 7; /* same as % 8 */
	idx = pos >> 3;  /* same as / 8 */
	while(len > 0) 
	{
		mask = ((2 << (len - 1)) - 1) << shift;
			/* keep shift amt in [0..31] */
		c = buf[idx] & ~mask;
		c |= (val << shift) & mask;
		buf[idx++] = c & 0xff;
		val >>= 8 - shift;
		len -= 8 - shift;
		shift = 0;
	}
}

static int like_intel = -1;

void
l_pack_print(packbuf, len, obuf)
unsigned char *packbuf; 
int len; 
char *obuf;
{
 int i;
 unsigned short s = 0;
 unsigned char *sp = (unsigned char *)&s;
 char *op = obuf;

	if (len % 8) len = (len/8) + 1;
	else len /= 8;

	memset(obuf, 0, len);


	if (like_intel == -1)
	{	
		s = 1;
		like_intel = 0;
		if (*sp) like_intel = 1;
	}

	i = 0;
	s = 0;
	while (len > 0)
	{
		if (like_intel)
		{
			sp[0]  = packbuf[i];
			sp[1]  = packbuf[i+1];
		}
		else
		{
#ifdef CRAY
			if (sizeof(short) == 8) /*CRAY*/
			{
				sp[6]  = packbuf[i+1];
				sp[7]  = packbuf[i];
			}
			else if (sizeof(short) == 4) /*CRAY T3E*/
			{
				sp[2]  = packbuf[i+1];
				sp[3]  = packbuf[i];
			}
			else
#endif /* CRAY */
			{
				sp[0]  = packbuf[i+1];
				sp[1]  = packbuf[i];
			}
		}
		len-=2;
		i += 2;
/*
 *		There's an enhancement in the following code that
 *		we don't print out trailing zeros
 */
		if (len <= 0 && !s) 
			;
		else if ((len <= 0) || 
			(len <=2 && !(packbuf[i] + packbuf[i+ 1])))
#ifndef PC16
			sprintf(op, "%d", s);
#else
                        sprintf(op, "%u", s);
#endif
		else
		{
#ifdef PC16
			sprintf(op, "%05u%c", s, L_DECIMAL_DELIM);
#else
			sprintf(op, "%05d%c", s, L_DECIMAL_DELIM);
#endif
		}
		op += strlen(op);
		s = 0;
	}
	*op = 0;
	dec_add_cksum(obuf);
}
/* returns length */
int
l_pack_unprint(job, inputstr, opacked)
LM_HANDLE *job;
unsigned char *inputstr; /* in #####-#####... format */
unsigned char *opacked;
{
 int len = 0;
 unsigned int d;
 unsigned short s;
 unsigned char *cp = (unsigned char *)&s;
 int slen;
 char buf[MAX_CONFIG_LINE + 1];
 char *istr = buf;

	l_zcp(buf, (char *)inputstr, MAX_CONFIG_LINE);
	if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;

	if (!dec_cksum_ok((char *)istr))
	{
		LM_SET_ERROR(job, LM_BADDECFILE, 391, 0, istr, LM_ERRMASK_ALL);
		return LM_BADDECFILE;
	}

	memset(opacked, 0, MAX_CONFIG_LINE);
	slen = strlen((char *)istr);
	if (like_intel == -1)
	{	
		s = 1;
		like_intel = 0;
		if (*cp) like_intel = 1;
	}
	while (*istr)
	{
		sscanf((char *)istr, "%05d", &d);	/* overrun checked */
		s = (d & 0xffff);
		if (s > 0xff || slen >= 5)
		{
			slen -= 5;
			len += 2; /* in bytes */
			istr += 5;
		}
		else	
		{
			len +=1; /* in bytes */
			istr += slen; /* we're done */
			opacked[0] = (unsigned char) s & 0xff; /* last byte */
			break;
		}
		if (like_intel)
		{
			opacked[0]  = cp[0];
			opacked[1]  = cp[1];
		}
		else
		{
#ifdef CRAY
			if (sizeof(short) == 8) /*CRAY*/
			{	
				opacked[0]  = cp[7];
				opacked[1]  = cp[6];
			}
			else if (sizeof(short) == 4) /*CRAY T3E*/
			{	
				opacked[0]  = cp[3];
				opacked[1]  = cp[2];
			}
			else
#endif
			{
				opacked[0]  = cp[1];
				opacked[1]  = cp[0];
			}
		}
		opacked += 2;
		if (*istr == L_DECIMAL_DELIM) 
		{
			istr++;
			slen--;
		}

	}
	return len;
}

/*
 *	dec_cksum_ok
 *	1 == success, 0 == failure
 */
static
int
dec_cksum_ok(str)
char *str;
{
  int len = strlen(str);
  int cksum = str[len - 1] - '0';

	str[len - 1] = 0; /* null terminate */
	if (cksum != dec_cksum(str, len))
		return 0;
	else
		return 1;
}
/*
 *	Adds the checksum to the string 
 */
static
void
dec_add_cksum(str)
char *str;
{
  int len = strlen(str);
  int cksum = dec_cksum(str, len);

	if (str[len-6] == L_DECIMAL_DELIM)
		sprintf(&str[len], "%c%c", L_DECIMAL_DELIM, cksum + '0');
	else
		sprintf(&str[len], "%c", cksum + '0');
}
/*
 *	returns the checksum ("casting out nines" == % 10 ) on str  (<= 10)
 */
static
int
dec_cksum(str, len)
char *str;
int len;
{
  int i, cksum = 0;
	for (i = 0; i < len; i++)
	{
		if (isdigit(str[i]))
			cksum += (str[i] - '0');
		cksum %= 10;
	}
	return cksum;
}
