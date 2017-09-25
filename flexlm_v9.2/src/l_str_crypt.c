/******************************************************************************

	    COPYRIGHT (c) 1990, 2003  by Macrovision Corporation.
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
 *	Module: $Id: l_str_crypt.c,v 1.5 2003/01/13 22:41:55 kmaclean Exp $
 *
 *	Function: l_str_crypt(string, len, code)
 *		  l_str_dcrypt(string, len, code)
 *
 *	Description: Encrypts/decrypts a string.
 *
 *	Parameters:	(char *) string - The message to be encrypted/decrypted
 *			(int) len - Length of message to encrypt/decrypt
 *			(long) code - Code to use to encrypt
 *
 *	Return:		(char *) - The message is encrypted/decrypted in place 
 *
 *	M. Christiano
 *	5/4/90
 *
 *	Last changed:  11/25/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

#define BLOCKSIZE 8	/* Use 64-bit blocks (8 bytes) for encryption */
#define round(x) ((x) + 0.5)

/*
 *	XOR two BLOCKSIZE quantities
 */

#define XOR(a, b, r)	for (j = 0; j < BLOCKSIZE; j++) { r[j] = a[j] ^ b[j]; }

/*
 *	Encrypt a string in place, return the result.
 */

char * API_ENTRY
l_str_crypt(string, len, code, v5_plus)
char *string;
int len;
long code;
int v5_plus;
{
/*
 *	string is the string to be encrypted, "code" is the
 *	initializing vector
 *	Use block chaining with cyphertext feedback.
 */

/*  FOR NOW:  Simply XOR the string with the key */

  int i = 0;
  char *out = string;
  char keys[4];
/*
 *	We would have to bump the comrev to include this
 */
	keys[0] = (char)(code & 0xff);         
	keys[1] = (char)((code >> 8) & 0xff);  
	keys[2] = (char)((code >> 16) & 0xff); 
	keys[3] = (char)((code >> 24) & 0xff); 

	while (len--)
	{
		*string ^= keys[i];
		i++;
		if (i > 3) i = 0;
		string++;
	}
	return (out);
}

char * API_ENTRY
l_str_dcrypt(string, len, code, v5_plus)
char *string;
int len;
long code;
int v5_plus;
{
	return (l_str_crypt(string, len, code, v5_plus));
}
