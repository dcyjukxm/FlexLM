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
/*	
 *	Module: $Id: l_any_set.c,v 1.2 2003/01/13 22:41:49 kmaclean Exp $
 *
 *	Function:	l_any_set(masks, size)
 *
 *	Description: 	Checks for any bits set in masks
 *
 *	Parameters:	(SELECT MASK) masks
 *			(int) size - Size of masks
 *
 *	Return:		0 - No bits set
 *			1 - Any bit set
 *
 *	M. Christiano
 *	3/6/91
 *
 *	Last changed:  3/10/98
 *
 */

#include "lmachdep.h"
#include "lmselect.h"

l_any_set(masks, size)
SELECT_MASK masks;
int size;
{
  int i;

	for (i=0; i<size; i++) if (((int *)masks)[i]) return(1);
	return(0);
}
