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
 *	Module: $Id: l_file_stub.c,v 1.3 2003/04/18 23:48:02 sluu Exp $
 *
 *      Function:	l_file_check(), l_file_checkin(), l_file_checkout(), 
 *			l_file_checkq(), l_file_userlist(), l_file_remove(), 
 *			l_file_removeh(), l_file_sdata(), l_file_log()
 *
 *
 *	Description: 	Stubs for the file-based functions.
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/94
 *
 *	Last changed:  5/5/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

API_ENTRY
l_file_check(LM_HANDLE * job)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 36, 0);
	/*
	job->lm_errno = LM_NOFILEOPS;
	job->u_errno = 0;
	*/
	return(0);
}

char
API_ENTRY
l_file_checkout(
	LM_HANDLE *	job,
	CONFIG *	conf,
	int			nlic,
	char *		version,
	int			wait_flag,
	int			dup_group,
	int			linger_interval,
	char *		vendor_checkout_data,
	char **		param)			/* Returned message parameter */
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 37, 0);
	/*
	job->lm_errno = LM_NOFILEOPS;
	job->u_errno = 0;
	*/
	return('\0');
}

API_ENTRY
l_file_checkin(
	LM_HANDLE *	job,
	char *		feature,
	char *		code,
	char *		vendor_checkout_data)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 38, 0);
	/*
	job->lm_errno = LM_NOFILEOPS;
	job->u_errno = 0;
	*/
	return(0);
}

API_ENTRY
l_file_checkq(
	LM_HANDLE *	job,
	char *		feature)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 39, 0);
	/*
	job->lm_errno = LM_NOFILEOPS;
	job->u_errno = 0;
	*/
	return(0);
}

void
API_ENTRY
l_file_log(
	LM_HANDLE *	job,
	char *		msg)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 40, 0);
	/*
	job->lm_errno = LM_NOFILEOPS;
	job->u_errno = 0;
	*/
}

API_ENTRY
l_file_sdata(
	LM_HANDLE *	job,
	CONFIG *	conf,
	int			type,
	void *		ret)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 41, 0);
	/*
	job->u_errno = 0;
	job->lm_errno = LM_NOFILEOPS;
	*/
	return(LM_NOFILEOPS);
}

void
API_ENTRY
l_file_remove(
	LM_HANDLE *	job,
	CONFIG *	conf,
	char *		user,
	char *		host,
	char *		display)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 42, 0);
	/*
	job->u_errno = 0;
	job->lm_errno = LM_NOFILEOPS;
	*/
}


void
API_ENTRY
l_file_removeh(
	LM_HANDLE *	job,
	CONFIG *	conf,
	char *		handle)
{
	LM_SET_ERRNO(job, LM_NOFILEOPS, 43, 0);
	/*
	job->u_errno = 0;
	job->lm_errno = LM_NOFILEOPS;
	*/
}

void
API_ENTRY
l_file_userlist(
	LM_HANDLE *	job,
	CONFIG *	conf,
	LM_USERS **	ret)
{
	*ret = 0;
	LM_SET_ERRNO(job, LM_NOFILEOPS, 44, 0);
}
