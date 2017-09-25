/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_xstrcmp.c,v 1.2 2003/01/13 22:41:48 kmaclean Exp $
 *
 *	Function:	lc_xstrcmp(job, a, b)
 *
 *	Description: 	Compares two strings, like strcmp, but checks for NULL
 *
 *	Parameters:	(char *) a
 *			(char *) b
 *
 *	Return:		0 if:  both ptrs are NULL.
 *			       one ptr is NULL and the other ptr is empty string
 *			       both ptrs are non-null and strings are the same.
 *
 *	M. Christiano
 *	1/17/93
 *
 *	Last changed:  10/23/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

int 
API_ENTRY
lc_xstrcmp(job, a, b)
LM_HANDLE *job; /* unused for now */
char *a;
char *b;
{
	if (a == (char *) NULL)
	{
		if (b == (char *) NULL || *b == '\0') return(0);
	}
	else if (b == (char *) NULL)
	{
		if (*a == '\0') return(0);
	}
	else
	{
		return(strcmp(a, b));
	}
	return(1);
}
