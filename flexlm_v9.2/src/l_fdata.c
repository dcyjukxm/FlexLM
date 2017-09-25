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
 *	Module: $Id: l_fdata.c,v 1.5 2003/01/13 22:41:51 kmaclean Exp $
 *
 *	Function:	public FEATDATA routines
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_fdata.h"
#include "l_prot.h"

/*
 *	l_more_featdata() - Create more FEATDATA structs, link in to current.
 *	result should be cast to FEATDATA
 */

void *
l_more_featdata(job, last)
LM_HANDLE *job;		/* Current license job */
void *last;
{
  FEATDATA *fd;

	fd = (FEATDATA *) l_malloc(job, sizeof(FEATDATA));
	if(!fd)
	{
		LM_SET_ERROR(job, LM_CANTMALLOC, 601, 0, 0, LM_ERRMASK_ALL);
		return NULL;
	}
	memset(fd, 0, sizeof(FEATDATA));
	fd->f[0] = '\0';
	fd->vendor_checkout_data[0] = '\0';
	fd->next = (FEATDATA *)NULL;
	fd->n = 0;
	fd->status = 0;
	fd->serialno = -1;
	if (last) ((FEATDATA *)last)->next = fd;
	return((void *)fd);
}

/*
 *	l_free_job_featdata
 */

void
l_free_job_featdata(job, featdata)
LM_HANDLE *job;
void FAR *featdata;
{
  FEATDATA *f = (FEATDATA *)featdata, *next;
	
	while (f)
	{
		next = f->next;
		if (f->conf)
		{
			if (f->conf->conf_featdata) 
			{
				l_free_conf(job, f->conf);
				f->conf = NULL;
			}
		}
		free(f);
		f = next;
	} 
}
