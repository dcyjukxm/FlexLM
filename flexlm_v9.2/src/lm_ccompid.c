/******************************************************************************

	    COPYRIGHT (c) 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_ccompid.c,v 1.3 2003/03/08 07:19:46 jwong Exp $
 *
 *	Function:    lc_init_complex_composite_id()
 *
 *	Description: initializes complex composite hostid structure.
 *
 *	Parameters:	
 *
 *	Return:		 int - 0 if successful, 1 if failed. See errno for
 *                       further details of failure.
 *
 *	J. Wong
 *	3/4/03
 *
 *	Last changed:  3/4/03
 *
 */

#include <errno.h>

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
int API_ENTRY
lc_init_complex_composite_id(LM_HANDLE_PTR job, int id_type_list[], int num_ids)
{
	if (LM_API_ERR_CATCH) return job->lm_errno;
	LM_API_RETURN(int, l_init_complex_composite_id(job, id_type_list, num_ids))
	
}

int 
l_init_complex_composite_id(LM_HANDLE_PTR job, int id_type_list[], int num_ids)
{
	// phase 2-4
	return 0;
}

