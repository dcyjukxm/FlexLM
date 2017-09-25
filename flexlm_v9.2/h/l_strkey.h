/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_strkey.h,v 1.8 2003/01/13 22:13:13 kmaclean Exp $
 *
 *	Function:	l_strkey.h -- for use with l_strkey.c
 *
 *	D. Birns
 *	12/21/96
 *
 *	Last changed:  6/11/98
 *
 */
#if 1
#include "lmachdep.h"
#include "lmclient.h"
#include "lgetattr.h"
#endif
#ifndef L_STRKEY
#define L_STRKEY

#define PKGOPT "PKGOPT"
#define SUPERSEDE "SUPERSEDE"
#define PLATFORMS "PLATFORMS"
#define TYPESTR "TYPE"
#define BLOCKSIZE 8	/*- Use 64-bit blocks (8 bytes) for encryption */

#define MAXINPUTLEN (((MAX_SERVERS+1)*12) /* MAX_SERVERS of ethernet addr */ \
			+ MAX_FEATURE_LEN    /* feature */ \
			+ DATE_LEN 	     /* date */ \
			+ sizeof(long) 	     /* # licenses */ \
			+ (2 * sizeof(long))  /* version */  \
			+ (2 * sizeof(long))  /* fromversion */  \
			+ MAX_USER_DEFINED   /* User-defined string */ \
			+ sizeof(long)	     /* type (INC, UPG, etc) */ \
			+ MAX_CONFIG_LINE    /* Just to be sure, for opts */ \
			+ 4		     /* Start date */ \
			+ sizeof(LM_LICENSE_LINGER_STRING) + 1 + sizeof(long) \
			+ sizeof(LM_LICENSE_DUP_STRING) + 1 + sizeof(long) \
			+ sizeof(LM_LICENSE_SUITE_DUP_STRING) + 1 +  \
								sizeof(long) \
			+ sizeof(LM_LICENSE_WBINARY_STRING) + 1 + sizeof(long) \
			+ sizeof(LM_LICENSE_WLOSS_STRING) + 1 + sizeof(long) \
			+ sizeof(LM_LICENSE_OVERDRAFT_STRING) + 1 +  \
							sizeof(long) \
			+ sizeof(PKGOPT) + 1 + sizeof(long))

#define L_MOVELONG(l, p) { \
  long ldata = signed32(l);\
\
        *p++ ^= (unsigned char) (ldata & 0xff);\
	if (ldata > 255 || ldata < -256)\
		*p++ ^= (unsigned char) ((ldata >> 8) & 0xff);\
	if (ldata > 32000 || ldata < -32000)\
		*p++ ^= (unsigned char) (((ldata) >> 16) & 0xff);\
	if (ldata > 16000000 || ldata < -16000000)\
		*p++ ^= (unsigned char) ((ldata >> 24) & 0xff);\
	ldata = 0x3d4da1d6; /* random number -- to obfuscate */ \
}
		
static unsigned char *l_movelong	/*- Move a longword into a string */
		lm_args(( long data, unsigned char *p));
static unsigned char * atox lm_args(( LM_HANDLE *,unsigned char *s,unsigned long x ));
static void our_encrypt 		/*- Our encryption routine */
	    lm_args((unsigned char *s));
static long signed32 lm_args((long));
/*-
 *	XOR two BLOCKSIZE quantities
 */

#define XOR(a, b, r)	for (j = 0; j < BLOCKSIZE; j++) { r[j] = a[j] ^ b[j]; }

#define XOR_SEEDS_ARRAY_SIZ 20
#define XOR_SEEDS_INIT_ARRAY(_arr_) \
	_arr_[10][0]=8; _arr_[3][3]=5; _arr_[19][0]=10; _arr_[5][2]=3;\
	_arr_[5][3]=7; _arr_[6][0]=7; _arr_[6][1]=3; _arr_[6][2]=5;\
	_arr_[4][0]=3; _arr_[4][1]=0; _arr_[10][3]=5; _arr_[11][0]=6;\
	_arr_[12][3]=9; _arr_[13][0]=0; _arr_[13][1]=4; _arr_[5][0]=1;\
	_arr_[17][1]=2; _arr_[11][1]=1; _arr_[17][2]=4; _arr_[1][0]=9;\
	_arr_[1][1]=8; _arr_[12][1]=3; _arr_[1][2]=3; _arr_[16][1]=5; \
	_arr_[16][2]=1; _arr_[17][3]=8; _arr_[0][1]=5; _arr_[0][2]=4;\
	_arr_[19][1]=3; _arr_[19][3]=1; _arr_[1][3]=1; _arr_[2][0]=8;\
	_arr_[5][1]=10; _arr_[6][3]=11; _arr_[10][2]=2; _arr_[7][0]=0;\
	_arr_[7][2]=9; _arr_[18][2]=1; _arr_[2][1]=1; _arr_[4][3]=7;\
	_arr_[18][0]=5; _arr_[12][2]=8; _arr_[14][1]=10; _arr_[14][2]=8;\
	_arr_[18][1]=0; _arr_[3][2]=10; _arr_[2][2]=2; _arr_[0][0]=3;\
	_arr_[16][3]=0; _arr_[18][3]=4; _arr_[17][0]=0; _arr_[0][3]=11;\
	_arr_[11][3]=9; _arr_[2][3]=5; _arr_[3][0]=2; _arr_[3][1]=11;\
	_arr_[8][1]=4; _arr_[10][1]=4; _arr_[8][2]=1; _arr_[9][0]=11;\
	_arr_[9][1]=8; _arr_[9][2]=1; _arr_[9][3]=3; _arr_[19][2]=5;\
	_arr_[7][3]=4; _arr_[8][0]=0; _arr_[7][1]=1; _arr_[13][2]=2;\
	_arr_[13][3]=10; _arr_[14][3]=7; _arr_[8][3]=10; _arr_[15][0]=1;\
	_arr_[15][1]=11; _arr_[15][2]=0; _arr_[11][2]=0; _arr_[15][3]=3;\
	_arr_[16][0]=6; _arr_[14][0]=3; _arr_[12][0]=4; _arr_[4][2]=1;\

#ifdef LM_CKOUT
static unsigned char * API_ENTRY l_string_key lm_args(( LM_HANDLE_PTR, 
			unsigned char *, int, VENDORCODE *, unsigned long,
			 char *));
#else
static unsigned char * API_ENTRY l_string_key lm_args(( LM_HANDLE_PTR, 
				unsigned char *, int, VENDORCODE *, 
				unsigned long ));
#endif /* LM_CKOUT */
#endif /* L_STRKEY */
