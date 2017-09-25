/******************************************************************************

	    COPYRIGHT (c) 1992, 2003  by Macrovision Corporation.
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
 *	Module: $Id: ls_attr.c,v 1.10 2003/05/05 15:51:08 sluu Exp $
 *
 *	Function:	ls_attr_setup(), ls_get_attr()
 *
 *	Description: 	Sets up daemon attributes, retrieves them.
 *
 *	Parameters:	
 *
 *	Return:
 *
 *	M. Christiano
 *	3/6/92
 *
 *	Last changed:  08/07/97
 *
 */


#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsfeatur.h"
#include "ls_attr.h"
#include "ls_aprot.h" 

static char *feature;
static char *nlic;
static char *flag;
static char *version;
static CLIENT_DATA *client;
static char *dup_sel;
static char *ling;
static char *ecode;

void
ls_attr_setup(f, n, waitflag, user, ver, dup_select, linger, code)
char *f;	/* Feature string */
char *n;	/* Number of licenses */
char *waitflag;	/* Wait flag */
CLIENT_DATA *user;	/* Client data struct */
char *ver;	/* Version of feature */
char *dup_select;	/* Duplicate selection flags */
char *linger;	/* Linger interval */
char *code;	/* Feature encryption code from license file */
{
	feature = f;
	nlic = n;
	flag = waitflag;
	version = ver;
	client = user;
	dup_sel = dup_select;
	ling = linger;
	ecode = code;
}


/* VARARGS1 */	/* Second arg can be different type */
ls_get_attr(key, val)
int key;
char **val;
{
  int *i;
  CLIENT_DATA **c;
  int y;

	LM_SET_ERRNO(lm_job, 0, 319, 0);
	i = (int *) val;
	c = (CLIENT_DATA **) val;

	switch(key)
	{
		case LS_ATTR_FEATURE:
			*val = feature;
			break;

		case LS_ATTR_NLIC:
			if (*nlic == '*')
			{
				LM_SET_ERRNO(lm_job, NOSUCHATTR, 320, 0);
				y = 0;
			}
			else
			{
				(void) sscanf(nlic, "%d", &y);	/* overrun checked */
			}
			*i = y;
			break;

		case LS_ATTR_FLAG:
			if (*flag == '*')
			{
				LM_SET_ERRNO(lm_job, NOSUCHATTR, 321, 0);
				y = 0;
			}
			else
			{
				y = *flag - '0';
			}
			*i = y;
			break;

		case LS_ATTR_VERSION:
			if (*flag == '*')
			{
				LM_SET_ERRNO(lm_job, NOSUCHATTR, 322, 0);
				*val = "";
			}
			else
			{
				*val = version;
			}
			break;

		case LS_ATTR_CODE:
			if (*flag == '*')
			{
				LM_SET_ERRNO(lm_job, NOSUCHATTR, 323, 0);
				*val = "";
			}
			else
			{
				*val = ecode;
			}
			break;

		case LS_ATTR_CLIENT:
			*c = client;
			break;

		case LS_ATTR_DUP_SEL:
			if (*flag == '*')
			{
				LM_SET_ERRNO(lm_job, NOSUCHATTR, 324, 0);
				y = 0;
			}
			else
			{
				(void) sscanf(dup_sel, "%d", &y);	/* overrun checked */
			}
			*i = y;
			break;

		case LS_ATTR_LINGER:
			if (*flag == '*')
			{
				LM_SET_ERRNO(lm_job, NOSUCHATTR, 325, 0);
				y = 0;
			}
			else
			{
				(void) sscanf(ling, "%d", &y);	/* overrun checked */
			}
			*i = y;
			break;
 		
		case LS_ATTR_SERVER:   
			/* Kmaclean 3/20/03
			 * p6850
			 * C86003
			 * took this out a while ago because it did not do anything.
			 * However, Some customer is using it so they get an error 
			 * from the server NOSUCHATTR so.... it's back in now. */
 			*i = 0; 
			break;

		default:
			LM_SET_ERRNO(lm_job, NOSUCHATTR, 326, 0);
			break;
	}
	return(lm_job->lm_errno);
}
