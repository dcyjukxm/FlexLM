/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_slwg.c,v 1.5 2003/04/18 23:48:08 sluu Exp $
 *
 *	Description: 	Software Licensing Working Group function call 
 *			interface standard.  This module provides standard
 *			calls that work with FLEXlm.
 *
 *	M. Christiano
 *	6/18/90
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lm_slwg.h"
#include <stdio.h>


extern LM_HANDLE *lm_job;

static VENDORCODE code;
static short dup_group = LM_DUP_NONE;

/*----------------------------------------------------------------------------*/
/*
 *	General calls
 */
/*----------------------------------------------------------------------------*/
/*
 *	ls_init() - Initialize License System Activities and get a unique job_id
 */
int
ls_init(
	char *			vendor_id,		/* vendor ID */
	VENDORCODE *	vendor_key,		/* Vendor's encryption seeds */
	LM_HANDLE **	job_id)
{
	return(lc_init((LM_HANDLE *) NULL, vendor_id, vendor_key, job_id));
}

/*
 *	ls_terminate() - Terminate License System Operations
 */
/*ARGSUSED*/
int
ls_terminate(LM_HANDLE * job_id)
{
	lc_disconn(job_id, 0);
	return(0);
}

/*
 *	ls_log_message() - Put message in License System log file
 */

/*ARGSUSED*/
int
ls_log_message(
	LM_HANDLE *	job_id,
	int			length,
	char *		text)
{
  char *a = (char *) malloc(length + 1);

	if (a)
	{
		(void) bcopy(text, a, length);
		a[length] = '\0';
		lc_log(job_id, a);
		(void) free(a);
	}
	return(0);
}

/*----------------------------------------------------------------------------*/
/*
 *	License brokering calls
 */
/*----------------------------------------------------------------------------*/

/*
 *	lb_request() - Request License
 */

/*ARGSUSED*/
int
lb_request(
	LM_HANDLE *		job_id,	/* UNUSED */
	char *			product_id,	/* feature */
	char *			version,	/* version */
	int				amount,		/* nlic */
	int				queue,		/* in queue flag: 0 -> no, 1 -> asynchronous */
	int				check_period,	/* check interval - unused - in this model, user must poll daemon */
	LM_HANDLE **	license_handle) /* returned as product_id, i.e., feature */
{

	(void) lc_set_attr(job_id, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1);
	(void) lc_set_attr(job_id, LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)-1);
	/* FIXME: *license_handle = product_id; */
	return(lc_checkout(job_id, product_id, version, amount, 
				queue ? LM_CO_QUEUE : LM_CO_NOWAIT,
				&code, dup_group));
}

/*
 *	lb_check_wait() - See if a queued request is ready
 */

/*ARGSUSED*/
int
lb_check_wait(
	char *	license_handle,	/* feature */
	int		check_period,	/* UNUSED */
	int *	queue_position)	/* 0 - have license,  1 - in queue, 1000 - some error */
{
  int x;

	lm_job->lm_errno = 0;
	x = lm_status(license_handle);
	if (x == FEATQUEUE) *queue_position = 1;
	else if (x != 0) *queue_position = 1000;
	else *queue_position = 0;
	return(lm_job->lm_errno);
}

/*
 *	lb_wait_remove() - Remove an entry in a license wait queue
 */
int
lb_wait_remove(char * license_handle)
{
	lm_job->lm_errno = 0;
	(void) lm_checkin(license_handle, 1);
	return(lm_job->lm_errno);
}

/*
 *	lb_confirm() - Inform license system that a license is still in use
 */

/*ARGSUSED*/
int
lb_confirm(
	char *	license_handle,
	int		check_period)
{
	lm_job->lm_errno = 0;
	lc_timer((LM_HANDLE *) license_handle);
	return(_lm_errno);
}

/*
 *	lb_release() - Release a license
 */

int
lb_release(char * license_handle)
{
	lm_job->lm_errno = 0;
	(void) lm_checkin(license_handle, 1);
	return(_lm_errno);
}


/*
 *	lb_get_cur_users() - Determine current users of a license
 */
 
static LM_USERS *ulist = (LM_USERS *) NULL;
static LM_USERS *qlist = (LM_USERS *) NULL;
static char ulistfeature[MAX_FEATURE_LEN + 1];
static int tot = 0;

/*ARGSUSED*/
int
lb_get_cur_users(
	LM_HANDLE *	job_id,	/* UNUSED */
	char *		product_id,	/* feature */
	char *		version,	/* UNUSED */
	LM_USERS **	next,	/* IN: where to start, OUT: next entry */
	int			maximum_entries,	/* Max number he wants back */
	int			queued,		/* 0-> return current users.  1-> return queued users */
	int *		total_licenses,	/* Total # of licenses available */
	int *		number_of_entries,	/* # of entries we are returning */
	LM_USERS **	entries)	/* Entry list */
{
  int count = 0;
  LM_USERS *u;

	lm_job->lm_errno = 0;
	if (*next == LB_START || strcmp(ulistfeature, product_id))
	{
/*
 *		Read a new list
 */
		ulist = qlist = (LM_USERS *) NULL;
		tot = 0;
		(void) strcpy(ulistfeature, product_id);
		ulist = lm_userlist(product_id);
		if (ulist)
		{
			for (u = ulist; u; u = u->next)
			{
				if (*u->name == '\0')
				{
					tot = u->nlic;
					ulist = u->next;
					break;
				}
			}
/*
 *			Terminate the user list
 */
			for (u = ulist; u; u = u->next)
			{
				if (u->opts == INQUEUE || lm_isres(u->opts))
				{
					(u-1)->next = (LM_USERS *) NULL;
					qlist = u;
					break;
				}
			}
			for (u = qlist; u; u = u->next)
			{
				if (lm_isres(u->opts))
				{
					(u-1)->next = (LM_USERS *) NULL;
					break;
				}
			}
		}
	}
/*
 *	Now, return the data
 */
	if (ulist)
	{
		if (*next != LB_START)
		{
			*entries = *next;
			for (u = *next; u && maximum_entries-- > 0; u = u->next)
			{
				count++;
			}
		}
		else
		{
			*entries = queued ? qlist : ulist;
			for (u = *entries; u && maximum_entries-- > 0; 
							u = u->next)
			{
				count++;
			}
		}
		*next = u;	/* Return next pointer */
	}
	*number_of_entries = count;
	*total_licenses = tot;
	return(_lm_errno);

}

/*----------------------------------------------------------------------------*/
/*
 *	Usage metering calls
 */
/*----------------------------------------------------------------------------*/

	/*** NOT IMPLEMENTED ***/

um_put_record() { return(NOFEATURE); }
um_get_record() { return(NOFEATURE); }
um_delete_records() { return(NOFEATURE); }
um_purge_records()  { return(NOFEATURE); }
um_undelete_records() { return(NOFEATURE); }

