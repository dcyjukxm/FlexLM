/******************************************************************************

	    COPYRIGHT (c) 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_sha.h,v 1.2 2003/03/06 02:21:18 jwong Exp $
 *
 *	SHA-1 Module
 *
 *	J. Wong
 *	2/28/03
 *
 *	Last changed:  3/4/03
 *
 *
 *****************************************************************************/


/* h files included here to make this just one file ... */

/* global.h */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_ 1

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* BYTE defines a unsigned character */
typedef unsigned char BYTE;

#ifndef TRUE
  #define FALSE	0
  #define TRUE	( !FALSE )
#endif /* TRUE */

#endif /* end _GLOBAL_H_ */

/* sha.h */

#ifndef _SHA_H_
#define _SHA_H_ 1

/* #include "global.h" */

/* The structure for storing SHS info */

typedef struct 
{
	UINT4 digest[ 5 ];            /* Message digest */
	UINT4 countLo, countHi;       /* 64-bit bit count */
	UINT4 data[ 16 ];             /* SHS data buffer */
	int Endianness;
} SHA_CTX;

/* Message digest functions */

void SHAInit(SHA_CTX *);
void SHAUpdate(SHA_CTX *, BYTE *buffer, int count);
void SHAFinal(BYTE *output, SHA_CTX *);

#endif /* end _SHA_H_ */

/* endian.h */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_ 1

void endianTest(int *endianness);

#endif /* end _ENDIAN_H_ */
