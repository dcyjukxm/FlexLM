/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_encode.c,v 1.14 2003/05/05 16:10:54 sluu Exp $
 *
 *	Daniel Birns
 *	2/25/88
 *
 *	These functions used to be macros.  They were changed to
 *	functions to allow ansi compilers to check args.
 *
 *	Last changed:  9/9/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

void API_ENTRY
l_encode_int(where, i) 
char *where;
int i;
{
	(void) sprintf((where), "%d", (i));
}

void API_ENTRY
l_encode_long(where, l) 
char *where; 
long l;
{
	(void) sprintf((where), "%ld", (l));
}

void API_ENTRY
l_encode_long_hex(where, l) 
char *where;
long l;
{
	(void) sprintf((where), "%lx", (l));
}



void API_ENTRY
l_decode_int(from, i)
char *from;
int *i;
{
	*i = 0;		/* memory threat */
	(void) sscanf((from), "%d", (i));	/* overrun threat */
}

void API_ENTRY
l_decode_long(from, l) 
char *from;
long *l;
{
	*l = 0;		/* memory threat */
	(void) sscanf((from), "%ld", (l));	/* overrun threat */
}

void API_ENTRY
l_decode_long_hex(from, l) 
char *from;
long *l;
{
	*l = 0;		/* memory threat */
	(void) sscanf((from), "%lx", (l));	/* overrun threat */
}

void API_ENTRY
l_zcp(d, s, l) 
char *d, *s;
int l;
{
	if (d && s && l)
	{
		(void) strncpy(d, s, l); 
		*((d)+l) = '\0';
	} else if (d) *d = 0;
}

/* 
 *	l_compare_version emulates float and strcmp.
 *		assume the versions are valid -- l_valid_version 
 *		must have been called first
 */	

int API_ENTRY
l_compare_version(job, v1, v2)
LM_HANDLE *job;
char *v1, *v2;
{
/*
 *	"123.456" -- 123 is int and 456 is frac
 */

  char *v1_int, *v1_frac,
	*v2_int, *v2_frac, version1[MAX_VER_LEN+1], version2[MAX_VER_LEN+1];
  int i;
  char *cp;
  char pzero[2];	/* For Alpha/VMS, where you can't overwrite a constant
			 * like this: p = "0"; *p = 0;
			 */


/* 
 *	Assume they're equal, but calling function should check
 *	lm_errno
 */
  int ret = 0; 

	strcpy(pzero, "0");

/*
 *	Check the most obvious thing first, for speed
 */
	if (!strcmp(v1, v2)) return 0; 
	if (!strcmp(v1, LM_PKG_ANY_VERSION)) return 0;
	if (!strcmp(v2, LM_PKG_ANY_VERSION)) return 0;
/*
 *	Do it the hard way
 */
	while (*v1 == '0') v1++;
	while (*v2 == '0') v2++;

	l_zcp(version1, v1, MAX_VER_LEN);
	l_zcp(version2, v2, MAX_VER_LEN);
	v1_int = version1;
	v2_int = version2;
	v1_frac = v2_frac = pzero;

	cp = (strchr(version1, '.'));
	if (cp && *cp)
	{
		*cp = '\0';
		v1_frac = cp+1;
	}
	cp = (strchr(version2, '.'));
	if (cp && *cp)
	{
		*cp = '\0';
		v2_frac = cp+1;
	}
	if (!(ret = (strlen(v1_int) - strlen(v2_int))))
	{

		
		if (!(ret = strcmp(v1_int, v2_int)))
/* 
 *		integers are equal -- compare the fractional parts
 */
		{
/*		
 *			remove trailing zeros from fractional parts
 */
			for (i = (strlen(v1_frac)) - 1; i >= 0 ;i--)
			{
				if (v1_frac[i] == '0')
					/* null terminate */
					v1_frac[i] = '\0'; 
				else break;
			}
			for (i = (strlen(v2_frac)) - 1; i >= 0 ;i--)
			{
				if (v2_frac[i] == '0')
					/* null terminate */
					v2_frac[i] = '\0'; 
				else break;
			}
			ret = strcmp(v1_frac, v2_frac);
		}
	}
	return ret;
}
/* print in canonical binary format */
void
l_encode_16bit_packed( char *where, unsigned short val) /* 16-bit, 2-byte value */
{
  int i;
	for (i = 0; i < 2; i++)
	{
		where[i] = val & 0xff;
		val = val >> 8;
	}
}
void
l_decode_16bit_packed( char *where, unsigned short *val)
{
  int i;
  unsigned char *p = (unsigned char *)where;

	*val = 0;
	for (i = 0; i < 2; i++)
	{
		*val += p[i] << (i * 8);
	}
}
/* print in canonical binary format */
void
l_encode_32bit_packed(where, val) 
char *where;
unsigned long val; /* 32-bit, 4-byte value */
{
  int i;
	for (i = 0; i < 4; i++)
	{
		where[i] = val & 0xff;
		val = val >> 8;
	}
}
void
l_decode_32bit_packed(where, val)
char *where;
unsigned long *val;
{
  int i;
	*val = 0;
	for (i = 0; i < 4; i++)
	{
		/* "where" cast to unsigned char for P6655 */
		*val += ((unsigned char)where[i]) << (i * 8);
	}
}
