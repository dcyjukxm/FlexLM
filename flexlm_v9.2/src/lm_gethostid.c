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
 *	Module: $Id: lm_gethostid.c,v 1.6 2003/01/13 23:19:09 kmaclean Exp $
 *
 *	Function: lm_gethostid()
 *
 *	Description: FLEXlm equivalent of Unix gethostid()
 *
 *	Parameters:	lm_gethostid () - None
 *
 *	Return:		(HOSTID *) - the FLEXlm host ID.
 *
 *	M. Christiano
 *	9/3/89
 *
 *	Last changed:  7/24/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"


HOSTID * API_ENTRY
lc_gethostid(job)
LM_HANDLE *job;		/* Current license job */
{
	return(lc_getid_type(job, (short) L_DEFAULT_HOSTID));
}

HOSTID * API_ENTRY
l_gethostid(job)
LM_HANDLE *job;
{
	return(l_getid_type(job, (short) L_DEFAULT_HOSTID));
}
