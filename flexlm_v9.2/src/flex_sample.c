/*****************************************************************************
	COPYRIGHT (c) 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Corporation and is protected by law.
	It may 	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Corporation.
*****************************************************************************/
/*	$Id: flex_sample.c,v 1.3 2003/04/04 18:41:25 sluu Exp $	*/

/**	@file 	filenamehere
 *	@brief	A brief decscription here
 *	@version $Revision: 1.3 $
 *
 *	A longer description here
 *	If this code came from somewhere else, then put references here.
*****************************************************************************/



/**********************	SAMPLE FUNCTION COMMENT BLOCK************************/
/****************************************************************************/
/**	@brief	Convert an ASCII string to a hex array of bytes
 *
 *	@param	src		The ASCII string to convert
 *	@param	dst		The byte array destination.
 *	@param	count	Number of char's to convert.
 *
 *	@return	Success of failure
 *			-True if the correct number of hex chars were converted
 *			-False if a non-hex char was found or there was less than 'count'
 *				chars in the string
 ****************************************************************************/
static
BOOL
atohb(
	char *			src,
	unsigned char *	dst,
	int				count)
{
	/*
	 *	Some code here
	 */
}



