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
 *	Module: $Id: l_uppercase.c,v 1.8 2003/01/13 22:41:56 kmaclean Exp $
 *
 *	Function:	l_uppercase
 *
 *	Description: 	changes in place
 *
 *	Parameters:	(char *)str
 *
 *	Return:		void
 *
 *	D. Birns
 *	7/30/95
 *
 *	Last changed:  7/2/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"

/*
 *	uppercase fixes in place 
 */
void API_ENTRY 
l_uppercase(str)
char *str;
{
  unsigned char *cp;

	for (cp = (unsigned char *)str; *cp; cp++)
		*cp = l_toupper(*cp);
}
void API_ENTRY 
l_lowercase(str)
char *str;
{
  unsigned char *cp;

	for (cp = (unsigned char *)str; *cp; cp++)
		*cp = l_tolower(*cp);
}
