/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_redir_ver.c,v 1.2 2003/01/13 22:41:48 kmaclean Exp $
 *
 *	Function:	lc_redir_ver()
 *
 *	Description: 	Verification routine for hostid redirection
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(HOSTID *) from
 *			(HOSTID *) to
 *			(VENDORCODE *) code
 *			(char *) signature
 *
 *	Return:		1 - OK
 *			0 - bad
 *
 *	M. Christiano
 *	1/17/92
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

lc_redir_ver(job, from, to, code, signature)
LM_HANDLE *job;		/* Current license job */
HOSTID *from;
HOSTID *to;
VENDORCODE *code;
char *signature;
{
	return(1);
}
