/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lmhostid.c,v 1.11 2003/03/08 07:19:47 jwong Exp $
 *
 *	Function:	lc_hostid
 *
 *	Description: 	returns hostid as a string
 *
 *	Parameters:	int type -- from lmclient.h
 *			buf -- must be MAX_CONFIG_LEN size
 *
 *	Return:		int  -- lm_errno
 *
 *	D. Birns
 *	8/8/96
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
int API_ENTRY
lc_hostid(job, type, buf)
LM_HANDLE_PTR job;
int type;
char *buf;
{
	if (LM_API_ERR_CATCH) return job->lm_errno;
	LM_API_RETURN(int, l_hostid(job, type, buf))
	
}
int 
l_hostid( LM_HANDLE_PTR job, int type, char *buf)
{
   char * ret = 0;
   HOSTID *h;

/*
 *	P5250 -- test the result of l_gethostid and l_getid_type
 */
	if ((type == HOSTID_DEFAULT) && (h = l_gethostid(job))) 
			l_zcp(buf, ret = l_asc_hostid(job, h),MAX_CONFIG_LINE - 1);
	else 
		if (h = l_getid_type(job, type)) 
		l_zcp(buf, ret = l_asc_hostid(job, h), MAX_CONFIG_LINE - 1);
								
	if (!ret) return job->lm_errno;
	else return 0;
}
