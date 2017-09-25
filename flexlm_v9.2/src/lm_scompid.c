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
 *	Module: $Id: lm_scompid.c,v 1.9 2003/03/12 21:07:02 sluu Exp $
 *
 *	Function:    lc_init_simple_composite()
 *
 *	Description: initializes simple composite hostid structure.
 *
 *	Parameters:  (LM_HANDLE_PTR) job - current job handle.
 *                   (int *) id_type_list - list of HOSTID_.
 *                   (int) num_ids - number of HOSTID_ in array.
 *
 *	Return:      int - 0 if successful, < 0 failed, value set by errno.
 *
 *	J. Wong      04 Mar 2003
 *
 *	Last changed:  3/4/03
 *
 */

#include <errno.h>

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

#include "composite.h"

static
int
l_init_simple_composite_id(LM_HANDLE_PTR job, int *id_type_list, int num_ids)
{
int i, ret;
unsigned char buf[MAX_CONFIG_LINE];

	/* verify if hostids are availiable on platform */
	for (i = 0; i < num_ids; i++)
	{
		ret = lc_hostid(job, id_type_list[i], (char *)buf);
		if (ret != 0)
		{
			LM_SET_ERRNO(job, LM_COMPOSITEID_ITEM_ERR, 622, errno);
			return errno;
		}
	}

	if ( job->composite_init == NULL )
	{
		job->composite_init = l_malloc(job, sizeof(COMPOSITE_ID_INFO));
		if (errno == LM_CANTMALLOC)
		{
			LM_SET_ERRNO(job, LM_COMPOSITEID_INIT_ERR, 617, errno);
			return LM_CANTMALLOC;
		}

		/* init to NULL */
		job->composite_init->info_list = NULL;

		job->composite_init->id_count = num_ids;
		job->composite_init->info_list = (LM_COMPOSITEID_INFO*) l_malloc(job, sizeof(LM_COMPOSITEID_INFO)*num_ids);
		if (errno == LM_CANTMALLOC)
		{
			if (job->composite_init)
				l_free(job->composite_init);
			LM_SET_ERRNO(job, LM_COMPOSITEID_INIT_ERR, 620, errno);
			return LM_CANTMALLOC;
		}

		for (i=0; i<num_ids; i++)
			job->composite_init->info_list[i].id_type = id_type_list[i];
	}
	else
	{
		job->composite_init->id_count = num_ids;
		if (job->composite_init->info_list == NULL)
		{
			/* one more time */
			job->composite_init->info_list = (LM_COMPOSITEID_INFO*) l_malloc(job, sizeof(LM_COMPOSITEID_INFO)*num_ids);
			if (errno == LM_CANTMALLOC)
			{
				LM_SET_ERRNO(job, LM_COMPOSITEID_INIT_ERR, 621, errno);
				return LM_CANTMALLOC;
			}
		}
		if (job->composite_init->info_list)
			free(job->composite_init->info_list);

		job->composite_init->info_list = (LM_COMPOSITEID_INFO*) l_malloc(job, sizeof(LM_COMPOSITEID_INFO)*num_ids);
		if (errno == LM_CANTMALLOC)
		{
			if (job->composite_init)
				l_free(job->composite_init);
			LM_SET_ERRNO(job, LM_COMPOSITEID_INIT_ERR, 623, errno);
			return LM_CANTMALLOC;
		}

		for (i=0; i<num_ids; i++)
			job->composite_init->info_list[i].id_type = id_type_list[i];
	}
	return 0;
}


int API_ENTRY
lc_init_simple_composite(LM_HANDLE_PTR job, int id_type_list[], int num_ids)
{
	if (LM_API_ERR_CATCH) return job->lm_errno;
	LM_API_RETURN(int, l_init_simple_composite_id(job, id_type_list, num_ids))

}
