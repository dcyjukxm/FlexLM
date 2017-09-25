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
 *	Module: $Id: ls_max.c,v 1.8 2003/01/13 22:22:37 kmaclean Exp $
 *
 *	Function:	
 *
 *	Description: functions needed for options MAX
 *
 *	Parameters:
 *
 *	Return:
 *
 *	D. Birns
 *	9/19/96
 *
 *	Last changed:  11/16/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsfeatur.h"		/*- Dynamic feature data */
#include "ls_aprot.h"   

static int max_reached lm_args(( int , FEATURE_LIST *, int , char *, short [], int));
static int max_brothers lm_args(( int, USERLIST *, char *, short []));

/*
 *	ls_max_exceeded -- sets and returns LM_MAXLIMIT if exceeded
 */
int
ls_max_exceeded( FEATURE_LIST *f, CLIENT_DATA *client, int need)
{
  OPTIONS *o;
  GROUP *g;

	for (o = f->opt; o; o = o->next)
	{
		if (o->type != OPT_MAX) continue;
		if(o->type2 == OPT_GROUP)
		{
			for (g = client->groups; g; g = g->next)
			{
				if (L_STREQ(g->name, o->name) && 
					max_reached(o->type2, f, o->count, o->name, o->inet_addr, need))
				{
					lm_job->lm_errno = LM_MAXLIMIT;
					return LM_MAXLIMIT;
				}
			}
		}
		else if (o->type2 == OPT_HOST_GROUP)
		{
			for (g = client->hostgroups; g; g = g->next)
			{
				if (L_STREQ(g->name, o->name) && 
					max_reached(o->type2, f, o->count, 
								o->name, o->inet_addr, need))
				{
					lm_job->lm_errno = LM_MAXLIMIT;
					return LM_MAXLIMIT;
				}
			}
		}
		else
		{
			if (o->type2 == OPT_USER )
			{
				if (!L_STREQ(client->name, o->name)) 
					continue;
			}
			else if (o->type2 == OPT_HOST && o->name)
			{
				if (!L_STREQ(client->node, o->name)) 
					continue;
			}
			else if (o->type2 == OPT_DISPLAY)
			{
				if (!L_STREQ(client->display, o->name)) 
					continue;
			}
			else if ((o->type2 == OPT_INTERNET) || 
						(o->type2 == OPT_HOST))
			{
				if (l_inet_cmp(&client->inet_addr[0], 
						&o->inet_addr[0])) 
					continue;
			}
			else if (o->type2 == OPT_PROJECT)
			{
				if (!client->project ||
					!L_STREQ(client->project, o->name)) 
					continue;
			}
			if (max_reached(o->type2, f, o->count, o->name, o->inet_addr, need))
			{
				lm_job->lm_errno = LM_MAXLIMIT;
				return LM_MAXLIMIT;
			}
		}
						
	}
	return 0;
}
/*
 *	max_reached:
 *	return boolean:  true if limit of this type for this name has 
 *		  already been reached
 */
static 
int
max_reached(type2, flist, limit, name, inet_addr, need)
int type2;
FEATURE_LIST *flist;
int limit;
char *name;
short inet_addr[];
int need;
{
  USERLIST *u;
  int count = 0;

	for (u = flist->u; u; u = u->next)
	{
		count += max_brothers(type2, u, name, inet_addr);
	}
	return ((count + need) > limit);
}
static 
int
max_brothers(type2, uhead, name, inet_addr)
int type2;
USERLIST *uhead;
char *name;
short inet_addr[];
{
  USERLIST *b;
  int count = 0;
  GROUP *g;
  CLIENT_DATA *c;

 
	for (b = uhead; b ; b = b->brother)
	{

		if (type2 ==  OPT_GROUP)
		{
			if (!(c = ls_client_by_handle(b->handle)))
				continue;
			for (g = c->groups; g; g = g->next)
			{
				if ( L_STREQ(g->name, name) && 
							(count < b->count))
					count = b->count; 
			}
		}
		else if (type2 == OPT_HOST_GROUP)
		{
			if ( !(c = ls_client_by_handle(b->handle)))
				continue;
			for (g = c->hostgroups; g; g = g->next)
			{
				if ( L_STREQ(g->name, name) &&
							(count < b->count))
				count = b->count;
			}
		}
		else if (type2 == OPT_USER)
		{
			if (L_STREQ(b->name, name) && (count < b->count))
				count = b->count; /* new max */
		}
		else if ((type2 == OPT_HOST) && name)
		{
			if (L_STREQ(b->node, name) && (count < b->count))
				count = b->count; /* new max */
		}
		else if (type2 == OPT_DISPLAY)
		{
			if (L_STREQ(b->display, name) && (count < b->count))
				count = b->count; /* new max */
		}
		else if (type2 == OPT_PROJECT)
		{
			if (L_STREQ(b->project, name) && (count < b->count))
				count = b->count; /* new max */
		}
		else if ((type2 == OPT_INTERNET) || (type2 == OPT_HOST))
		{
			if ( !(c = ls_client_by_handle(b->handle)))
				continue;
			if (!l_inet_cmp(c->inet_addr, inet_addr) &&
						(count < b->count))
				count = b->count; /* new max */
		}
	}
	return count;
}
