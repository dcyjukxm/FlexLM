/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_cp_hostid.c,v 1.4 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function:	lc_copy_hostid
 *
 *	Description: 	returns malloc'd and copied hostid.
 *
 *	Parameters:	HOSTID *
 *
 *	Return:		HOSTID *
 *
 *	D. Birns
 *	9/15/95
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
HOSTID *  API_ENTRY
lc_copy_hostid(job, id)
LM_HANDLE *job;
HOSTID *id;
{
  HOSTID *ret, *p, *p2, *sav = (HOSTID *)0;

	if (LM_API_ERR_CATCH) return 0;
	for(p = id; p; p = p->next)
	{
		p2 = (HOSTID *)l_malloc(job, sizeof (HOSTID));
		memcpy(p2, p, sizeof(HOSTID));
		if (p->vendor_id_prefix)
		{
			(p2->vendor_id_prefix = (char *)l_malloc(job, 
					strlen(p->vendor_id_prefix) + 1));
			strcpy( p2->vendor_id_prefix, p->vendor_id_prefix);
		}
		if (sav) sav->next = p2;
		else ret = p2;
		sav = p2;
	}
	LM_API_RETURN(HOSTID *, ret)
}
