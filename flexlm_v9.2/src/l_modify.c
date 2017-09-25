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
 *	Module: $Id: l_modify.c,v 1.3 2003/01/13 22:41:53 kmaclean Exp $
 *
 *	Function: l_modify(data)
 *
 *	Description: Modify a longword in a known way.
 *
 *	Parameters:	(long) data - Data to be modified
 *
 *	Return:		(long) - modified data.
 *
 *	M. Christiano
 *	5/4/90
 *
 *	Last changed:  10/18/95
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"

long API_ENTRY
l_modify(data)
long data;
{
	return(data ^ 0x012ff210);
}
