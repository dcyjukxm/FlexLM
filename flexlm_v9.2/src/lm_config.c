/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_config.c,v 1.26.4.1 2003/07/02 23:50:41 sluu Exp $
 *
 *	Function: lc_get_config(job, feature)
 *		  lc_next_conf(job, feature, pos)
 *		  l_flush_config(job)
 *
 *	Description: Gets the configuration line for a given feature.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) feature - The desired feature line.
 *			(CONFIG **) pos - Position to start search at.
 *
 *	Return:		(struct config *) - The configuration structure
 *						for this feature, or
 *						NULL, if no feature found.
 *			pos - Returned by lc_next_conf()
 *
 *	Notes:		lc_get_config() will return the first line in
 *			the configuration file that matches the feature name.
 *
 *			l_flush_config() will flush the cached feature
 *			information.  This is useful if you are trying
 *			to switch license files while running.
 *
 *			lc_next_conf(job feature, pos) returns the "next" line 
 *			in the configuration file matching "feature".  
 *			The search is started from "pos".
 *
 *			Thus, 
 *				lc_next_conf(job, f, 0L);
 *
 *			is equivalent to:
 *
 *				lc_get_config(job, f);
 *
 *
 *	M. Christiano
 *	2/13/88
 *
 *	Last changed:  9/14/98
 *
 */
/*
 *	Define RETURN_FIRST_FEATURE_MATCH if the first match
 *	is desired, independent of the version number, otherwise
 *	comment out this definition.
 */
#define RETURN_FIRST_FEATURE_MATCH	/**/

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "l_prot.h"
#include "l_openf.h"
#include <stdio.h>


/***********************************************************************
 * Statics
 ************************************************************************/
static void error_check_conf(LM_HANDLE *job , CONFIG * conf);


/*
 *	l_flush_config() - Flush the cached license file line
 */

API_ENTRY
l_flush_config(job)
LM_HANDLE *job;		/* Current license job */
{
	l_init_file(job);		/* Re-read everything */
	job->lfptr = LFPTR_FIRST;	/* Point @ first file */
	job->conf = (CONFIG *) NULL;
#if REDIR /* Not yet (12/15/95) */
	l_do_redir(job);			/* re-read redirection */
#endif
	return(0);
}

/*
 *	lc_get_config() - Get the configuration line for a feature 
 *					(read from beginning of file)
 */
 
CONFIG * API_ENTRY
lc_get_config(job, feature)
LM_HANDLE *job;		/* Current license job */
char *feature;		/* Desired feature line */
{
  CONFIG *pos = 0;

	if (job->conf == (CONFIG *) NULL || !l_keyword_eq(job, feature, 
						job->conf->feature))
		job->conf = l_next_conf_or_marker(job, feature, &pos, 0, (char *)0);
	return(job->conf);
}


/*
 *	lc_next_conf() - Return the next valid feature line.
 */


CONFIG * API_ENTRY
lc_next_conf(job, feature, pos)
LM_HANDLE *job;		/* Current license job */
char *feature;		/* Desired feature */
CONFIG **pos;		/* Where to start search */
{
	if (LM_API_ERR_CATCH) return 0;

	LM_API_RETURN(CONFIG *, 
		l_next_conf_or_marker(job, feature, pos, 0, (char *)0))
}

/*
 *	l_next_conf_or_marker() - Wrapper used to support +port@host
 */

/***********************************************************************
 * Get the next CONFIG struct in the list.
 * Will call the server to get CONFIGs if the local CONFIG is a PORT_AT_HOST
 * 
 * Parameters:
 *		job;		 - Current license job 
 *		feature;	 - Desired feature 
 *		pos;		  -  Where to start search. On return this will be 
 * 						set to the next position in the CONFIG list. 
 * 						Set to CONFIG_EOF when there are no more CONFIGs
 *		marker_only;  - If true, return +port@host markers, otherwise,
 *						get CONFIGs from server 
 *		daemon_override;  - for +port@host and utilities that don't know
 *							the daemon name 
 * 
 * Return:
 * 		Pointer to a CONFIG or NULL if there are no more
 ************************************************************************/
CONFIG * API_ENTRY l_next_conf_or_marker(
						LM_HANDLE *job,		
						char *feature,		
						CONFIG **pos,		
						int marker_only,	
						char *daemon_override  
						)
{
  CONFIG *cf, *goodcf = (CONFIG *) NULL;
  char lastver[MAX_VER_LEN+1]; 
  int port_host_plus = 0;
  char last_server[MAX_HOSTNAME + 1 ];

	if ( pos == NULL )
	{
		LM_SET_ERRNO(job,LM_BADPARAM , 358, 0);
		return NULL;
	}
	if (*pos == 0) 
	{
		job->flags &= ~LM_FLAG_FEAT_FOUND; /* clear the flag */
		cf = 0;
	}

	*last_server = 0;
	if (!(*job->err_info.feature))
		l_zcp(job->err_info.feature, feature, MAX_FEATURE_LEN);
	strcpy(lastver, "XXX");
#define BOGUSLASTVER (!strcmp("XXX", lastver))
/*
 *	If we have never initialized the feature database, do it now
 *	and return on failure 
 */
	if (job->line == (CONFIG *) NULL)
	{
		l_init_file(job);
		/*
		 *	Read data from registry and put into list.
		 */
		if(job->l_new_job)
			l_read_borrow(job, feature);
	}

	if (job->lm_errno == NOCONFFILE  || job->lm_errno == BADFILE ||
	    job->lm_errno == NOREADLIC   || job->lm_errno == SERVNOREADLIC ||
		*pos == CONFIG_EOF )
	{
		goto exit_next_conf;
	}	
						  
	if (*pos == (CONFIG *) NULL) 
		*pos = job->line;	 /* Start off */

	for (cf = *pos; cf; cf = cf->next)
	{
		
/*

 *		For +port@host, at first there's always just ONE 
 *		marker record, FEATURE name "unknown", code 
 *		"PORT_HOST_PLUS_CODE"
 */

		if (cf->type == CONFIG_PORT_HOST_PLUS)
		{
			if (marker_only) /* request from checkout */
			{
				*pos = cf->next; 
				if (*pos == (CONFIG *) NULL)
					*pos = CONFIG_EOF;
				goodcf = cf;
				goto exit_next_conf;
			}
			else
			{
#ifndef NO_FLEXLM_CLIENT_API
			  CONFIG *conf, ctmp, *confp, *cflast, *cflastsav = 0;
			  int haveit;
				
				memset(&ctmp, 0, sizeof(ctmp));
/*
 *				P1981 -- have to find the last record
 *				for this feature name
 */
				for (cflast = job->line; 
					cflast && cflast != cf; 
							cflast = cflast->next)
				{
					
					if (l_keyword_eq(job, cflast->feature, 
								feature))
						cflastsav = cflast;
				}
				if (cflastsav && *cflastsav->php_next_conf_pos)
					strcpy(ctmp.php_next_conf_pos,
						cflastsav->php_next_conf_pos);
				strcpy(ctmp.code, CONFIG_PORT_HOST_PLUS_CODE);
				strcpy(ctmp.feature, feature);
				ctmp.server = cf->server;
				if (daemon_override)
					strcpy(ctmp.daemon, daemon_override);
				else
					strcpy(ctmp.daemon, cf->daemon);
				if (cf->server && 
					(job->lm_errno == LM_HOSTDOWN) &&
					!L_STREQ(last_server, cf->server->name))
				{	
					/* clear it */
					l_clear_error(job);
				}
				if (!(conf = 
					l_get_conf_from_server(job, &ctmp)))
				{
					if (cf->next)
						continue; 
					else
						goto exit_next_conf;
				}
				conf->lc_from_server = 1;
				conf->next = cf; /* to reset pos below */
/* 
 *				Put this config into job, right before marker
 *				BUG P1195 & P1190 -- if it's not already in
 *				the job!
 */
				haveit = 0;
				for (confp = job->line; confp; 
							confp = confp->next)
				{
					if (l_keyword_eq(job, confp->code, 
								conf->code)
					 && l_keyword_eq(job, confp->feature,
							feature))
						haveit = 1;
				}
								
				if (!haveit)
				{
					/* use server from marker */
					conf->server = cf->server;
					if (cf->last)
					{
						cf->last->next = conf;
						conf->last = cf->last;
					}
					else
					{
						job->line = conf;
					}
					cf->last = conf;
					cf = goodcf = conf;
					port_host_plus = 1;
				}
				if (!strncmp(conf->php_next_conf_pos, 
							"ffffffff", 8))
				{
/* 
 * 					we're at the end of the list 
 */
					*conf->php_next_conf_pos = '\0';
					if (conf->next->next)
						*pos = conf->next->next;
					else
						*pos = CONFIG_EOF;
				}
				else
					*pos = conf->next;
#endif /* NO_FLEXLM_CLIENT_API */
			}
		}
		if (l_keyword_eq(job, feature, cf->feature))
		{				/* Found it */
			if (!(job->flags & LM_FLAG_IS_VD) && 
				cf->L_CONF_FLAGS & L_CONF_FL_REJECT) /* P4859 */
				continue;
			if ((cf->type == CONFIG_FEATURE) ||
			    (cf->type == CONFIG_INCREMENT) ||
			    (cf->type == CONFIG_UPGRADE))
			{
#ifndef RETURN_FIRST_FEATURE_MATCH
				if (!BOGUSLASTVER && 
					l_compare_version(job, cf->v, lastver)
									<=0) 
					continue;
#endif
				/* P2374 */
				l_zcp(lastver, cf->version, MAX_VER_LEN);
				if (!port_host_plus)
				{
					goodcf = cf;
					*pos = cf->next;
					if (*pos == (CONFIG *) NULL)
						*pos = CONFIG_EOF;
				}
#ifdef RETURN_FIRST_FEATURE_MATCH
				break;
#endif
			}
		}
	}

exit_next_conf:
	if (!goodcf && !(job->flags & LM_FLAG_FEAT_FOUND))
	{
	  CONFIG *c;
	  int use_server = 1;
/* 	
 *	P3125:
 *	If the license has USE_SERVER, we haven't read any features in this
 *	license for this vendor vendor, then don't set this error.
 */
		if (job->daemon->server)
		{
			LM_SET_ERRNO(job, LM_NOFEATURE, 412, 0);
		}
		else
		{
			for (c = job->line; c; c = c->next)
			{
				if (c->type != CONFIG_PORT_HOST_PLUS)
					use_server = 0;
			}
			if (!use_server && !job->lm_errno)
			{
				LM_SET_ERRNO(job, LM_NOFEATURE, 357, 0);
			}
		}
	}
	if (goodcf && (!cf)) 
		goodcf = 0; /* p4579 */
	else if (goodcf && (cf->type != CONFIG_PORT_HOST_PLUS))
		job->flags |= LM_FLAG_FEAT_FOUND;

	return(goodcf);
}
/***********************************************************************
 * Determine if the license info in a config is already in the job->line list 
 * 
 * Parameters:
 * 		job		- the job that holds the list
 * 		lookfor	- the config we are looking for in the list
 * 
 * Return:
 * 		the pointer to the matching config in the list
 * 		NULL if a match is not found
 ************************************************************************/
CONFIG * is_confg_in_list(LM_HANDLE *job, CONFIG *lookfor)
{
	CONFIG * confp;
	
	for (confp = job->line; confp ; confp = confp->next)
	{
		/* this looks slow but it's really not.
		 * the first compare of the 'code' will fail most times.
		 * We only go through all the compares when a CONFIG is in the list
		 * and then it should only be once because there are not 
		 * (should not be) duplicates in the list 
		 */
		if (	   l_keyword_eq(job, confp->code, lookfor->code)
				&& l_keyword_eq(job, confp->feature,lookfor->feature)
				&& l_keyword_eq(job, confp->date,lookfor->date)
				&& l_keyword_eq(job, confp->version,lookfor->version)
				&& l_keyword_eq(job, confp->feature,lookfor->feature)
				&& l_keyword_eq(job, confp->lc_vendor_info,lookfor->lc_vendor_info)
				&& confp->users == lookfor->users
			)
		{
			/* found a match. return it */
			return confp;
		}
	}
	return NULL;
}	
/***********************************************************************
 * kmaclean 1/27/03
 * Ignore counted licenses read from the local license file. A counted license 
 * must come from the server.
 * 
 * Parameters:
 * 		job	 - the job handle
 * 		conf - the license to look at
 * 
 * Return:
 * 		1 - the license should be ignored
 * 		0 - do not ignore the license
 ************************************************************************/
int ignore_local_license(LM_HANDLE *job, CONFIG *conf)
{
	if (job->flags & LM_FLAG_IS_VD)
	{
		/* we are the vendor daemon. don't do this */
		return 0;
	}	
	if ( conf->file_order > 0 && conf->users > 0 && conf->type != CONFIG_PORT_HOST_PLUS)
	{
		/* and delete it from the config list since we always want 
		 * to use the config from the server if it is counted */
		 /* 
		  * However: Just to be sure we don't crash somewhere else,
		 * Make sure the config is not pointed to by a feature 
		 * somewhere. if it is then don't delete it. This will cause 
		 * dup configs in the list but it's better than crashing. */


/* out for testing because it doesn;t really work */
/*		 if ( ! is_conf_in_use(job, conf) )
 */
	 	l_free_conf(job, conf);
		
		return 1;
	}
	return 0;
}
/***********************************************************************
 * Copy the data from one conf to another.
 * Free's any pointers in the dest conf then coppies the pointers from the 
 * src conf.
 * 
 * Does not change the following: 
 * 		next and last pointers so that the dst CONFIG can already be in the conf list in the job
 * 		php_next_conf_pos - need to preserve the port at host list
 * 
 * Parameters:
 * 		job	- current job
 * 		dstConfig	- destination to copy to
 * 		srcConfig	- source config to copy to.
 ************************************************************************/
void copy_conf_data(LM_HANDLE * job, CONFIG *dstConf, CONFIG *srcConf)
{
	CONFIG tmpConf;
						
	/* need to make a shallow copy of the dstConf so we can delete 
	 * everything it points to later */
	tmpConf = *dstConf;

	/* do the copy */
	*dstConf = *srcConf;

	/* restore some things from the original */
	dstConf->next = tmpConf.next;
	dstConf->last = tmpConf.last;
	dstConf->lf = tmpConf.lf;  /* Fixes buf P6130 */ 
	l_zcp(dstConf->php_next_conf_pos,tmpConf.php_next_conf_pos, PHP_NEXT_CONF_SIZE - 1);

	/* free the old data */
	l_free_conf_data(job,&tmpConf);
}
				   
/***********************************************************************
 * Free  CONFIG 
 * Free the CONFIG and all the memory it points to.
 * Remove the CONFIG from the job->line list 
 * 
 * Parameters:
 * 		job - the job handle
 * 		c	- the config to free
 ************************************************************************/ 
void API_ENTRY l_free_conf(LM_HANDLE *job,CONFIG *c)
{
/* kmaclean for debugging. remove before build */	
/* error_check_conf(job,c);
 */ 

	l_free_conf_data(job,c);
	l_free_conf_no_data(job,c);
}
	
/***********************************************************************
 * Free a CONFIG but do not free the data in the config itself.
 * This is usually called after a config has been coppied with a shallow copy.
 * Free just the CONFIG do not touch the memory it points to.
 * Remove the CONFIG from the job->line list 
 * 
 * 
 * Parameters:
 * 		job - the job handle
 * 		c	- the config to free
 ************************************************************************/
void l_free_conf_no_data(LM_HANDLE *job,CONFIG *c)
{
	CONFIG *sav;
	CONFIG *p;
	/* if it's in the list then remove it */
	for (sav = NULL, p = job->line; p ; sav = p, p = p->next)
	{
		if (p == c) 
		{
			/* found it in the list */
			if (sav != NULL) 
			{
				/* it's not the first one in the list */
				sav->next = c->next;
			}
			else 
			{
				job->line = c->next;
				
			}
			if ( c->next != NULL )
			{
				/* we are not the last in the list so update the 
				 * back pointer of the next in the list */
				c->next->last = c->last;
			}	
			break;
		}
	}

	/*
	 *	remove references to this config -- ugh!
	 */
	for (p = job->line; p ; p = p->next)
	{
		if (p->parent_pkg == c) 
			p->parent_pkg = 0;
		if (p->parent_feat == c) 
			p->parent_feat = 0;
	}
	free(c); /* it's all clean now, we can free it */
}
/***********************************************************************
 * Free all the data in a CONFIG but do not free the CONFIG itself
 * 
 * Parameters:
 * 		job - the job handle
 * 		c	- the config to free
 ************************************************************************/
void l_free_conf_data(LM_HANDLE *job,CONFIG *c)
{
	CONFIG *next;
	if (c->type == CONFIG_PACKAGE && c->components) 
	{
		CONFIG *component;
		/* free component CONFIGs */
		for (component = c->components; component; component = next)
		{
			next = component->next;
			free(component);
		}
		c->components = NULL;
	}
	if (c->lc_vendor_def) 	free(c->lc_vendor_def);
	if (c->lc_vendor_info) 	free(c->lc_vendor_info);
	if (c->lc_dist_info) 	free(c->lc_dist_info);
	if (c->lc_user_info) 	free(c->lc_user_info);
	l_free_keylist(job, c->lc_keylist);
	c->lc_keylist = 0;
	if (c->lc_asset_info) 	free(c->lc_asset_info);
#ifdef SUPPORT_METER_BORROW
	if (c->lc_meter_borrow_info) 	free(c->lc_meter_borrow_info);
	if (c->meter_borrow_device) 	free(c->meter_borrow_device);
#endif /* SUPPORT_METER_BORROW */
	if (c->lc_issuer) 	free(c->lc_issuer);
	if (c->lc_notice) 	free(c->lc_notice);
	if (c->lc_prereq) 	free(c->lc_prereq);
	if (c->lc_sublic) 	free(c->lc_sublic);
	if (c->lc_dist_constraint) 	free(c->lc_dist_constraint);
	if (c->lc_serial) 	free(c->lc_serial);
	if (c->lc_issued) 	free(c->lc_issued);
	if (c->lc_supersede_list) 	
	{
		free(c->lc_supersede_list[0]);
		free(c->lc_supersede_list);
	}
	if (c->lc_platforms) 	
	{
		free(c->lc_platforms[0]);
		free(c->lc_platforms);
	}
	if (c->lc_w_binary) 	free(c->lc_w_binary);
	if (c->lc_w_argv0) 	free(c->lc_w_argv0);
	lc_free_hostid(job, c->idptr);
	if (c->floatid) lc_free_hostid(job, c->floatid);

}
/***********************************************************************
 *  debugging error checking.
 * is the conf we are about to free in use somewhere else?
 * 
 * Check the featdata list to see if any point to the conf : this is bad
 * 
 * then check all the confs pointed to by featdata to see if they are in 
 * the conf list
 * 
 * Parameters:
 * 		job		- the job handle
 * 		conf	- the config we are looking for. if it is NULL then 
 * 				  we only check teh feat data list against the config list.
 * 
 ************************************************************************/
#include "l_fdata.h"
#include <assert.h>

static void error_check_conf(LM_HANDLE *job , CONFIG * conf)
{
	FEATDATA *f = (FEATDATA *)job->featdata;
	int		found =  0;
	int		conffound =  0;
	CONFIG	* tmpConf;
		 
	/* go through the feature data list and verify the following:
	 * 
	 * 		1. The config that was passed in is not pointed to by a feature.
	 * 			We are trying to free this conf so it's bad if it's in the list
	 * 		2. the CONFIG it points to is in the CONFIG list in the job.
	 * 			If it's not then it is orphaned.  */
	while (f)
	{
		if (f->conf != NULL)
		{
			if (conf == f->conf )
				conffound++;
			/* now check to see that the conf in the feature is in the real 
			* conf list. */
/*			for ( tmpConf = job->line, found = 0; tmpConf != NULL ; tmpConf = tmpConf->next)
			{
				if ( tmpConf == f->conf )
					found++;
			}
			assert(found);
 */ 
		}
		f = f->next;
/* what is the conf->conf_featdata. 
 * Does this need to be checked too? */			  
	} 
	assert(conffound <= 1);
	/*
	 * Also check the conf that was passed in to see if it is in the 
	 * conf list. if it's not it may be bad */
/*	for ( tmpConf = job->line, found = 0; tmpConf != NULL ; tmpConf = tmpConf->next)
	{
		if ( tmpConf == f->conf )
			found++;
	}
	assert(found);
 */	
}	
