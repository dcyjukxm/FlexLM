/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent 
 *
 *		to run on an unlicensed platform is a violation of your 
 *
 *		license agreement with Macrovision Corporation. 
 *
 *
 *****************************************************************************/
/*	
 *	Module: $Id: l_mddl.c,v 1.5 2003/01/13 23:22:28 kmaclean Exp $
 *	Function: l_intelid(job)
 *
 *	Description: Performs de-obfuscation processing.
 *
 *
 */

#ifdef PC
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

/*****************************************************************************
 *
 * l_demuddle  -  Takes in a buffer and decodes it.
 *
 ****************************************************************************/

void
l_demuddle( LM_HANDLE *job, unsigned char **buff_ptr, int *len_ptr )
{
  unsigned short mask;
  unsigned short next_val;
  unsigned char add_val;
  char *new_buff;
  char *function_portion;
  unsigned short a;
  char *wrPtr;
  unsigned char xor_buff[10];
  int xor_size;
  int unmuddled_length;
  int size;
  int i;
  int x;

	/*
	 * Take the first three bytes as the length.
	 */

	size = 0;
	for( i = 0; i < 3; i++ )
	{
		size <<= 8;
		size += (*buff_ptr)[i];
	}
	if (!size) return;

	/*
	 * Now, xor sum the function to get the
	 * decode mask.
	 */

	mask = 0;
	i = 0;
	function_portion = &(*buff_ptr)[3];

	while( i < size )
	{
		for( x = 0, next_val = 0; x < 2; x++, i++ )
		{
			next_val <<= 8;
			add_val = function_portion[i];
			next_val += add_val;
		}

		mask ^= next_val;
	}

	/*
	 * Take the next byte as the xor mask size, then take
	 * the next x bytes as the xor mask itself.
	 */

	xor_size = (int) function_portion[i++];
	for( x = 0; x < xor_size; x++ )
		xor_buff[x] = function_portion[i+x];


	/*
	 * Allocate a big chunk to decode into.  (Yes, this
	 * could be more efficient by computing how much smaller
	 * it needs to be.)
	 */

	if (!(new_buff = l_malloc( job, size ))) return;


	/*
	 * Repeatedly walk the mask, plucking off valid bytes.
	 */

	wrPtr = new_buff;
	unmuddled_length = 0;
	i = 0;
	a = 0x01;
	x = 0;

	while( i < size )
	{
		/*
		 * Reset the mask if needed.
		 */

		if( 15 <= x )
		{
			x = 0;
			a = 0x01;
		}

		if( mask & a )
		{
			unsigned char next_byte;

			next_byte = function_portion[i];
			next_byte ^= xor_buff[i % xor_size];
			*wrPtr++ = next_byte;
			++unmuddled_length;
		}

		++i;
		++x;
		a <<= 1;
	}

	/*
	 * Zero out the rest of the buffer so when the used
	 * portion gets zero'd it's all zeros.
	 */

	memset( wrPtr, 0, size - unmuddled_length );

	/*
	 * Return the buffer and the size we really needed.
	 */

	*buff_ptr = new_buff;
	*len_ptr = unmuddled_length;
}

/*****************************************************************************
 *
 * l_gcdecode  -  Removes the base level of obfuscation.
 *
 ****************************************************************************/

void
l_gcdecode( unsigned char *buff, int size )
{
	unsigned char xor_mask[] = { 'g'-' ', 's'-' ', 'i'-' ' };
	int i;

	for( i = 0; i < size; i++ )
		*buff++ ^= xor_mask[i % sizeof(xor_mask)];
}


#endif /* PC */
