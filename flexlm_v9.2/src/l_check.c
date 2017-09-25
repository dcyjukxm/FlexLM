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
 *	Module: $Id: l_check.c,v 1.42 2003/06/17 17:20:47 sluu Exp $
 *
 *	Function: l_check(job, feature, ver, n, key, conf, dup_select, status)
 *		  lc_status(job, feature) - return the status for a feature
 *		  lc_auth_data(fjob, eature) - return license line for feature
 *		  lc_idle(fjob, lag) - Tell FLEXlm if program is idle.
 *		  l_checkoff(job, feature) - Turn off checking for a feature.
 *		  l_featon(job, feature, version, n, key, conf)
 *
 *	Description: Turns on server checking for this feature.
 *
 *	Parameters:	(char *) feature - The feature to check on.
 *			(char *) version - The version of this feature.
 *			(int) n - How many licenses he has
 *			(VENDORCODE *) key - The vendor key
 *			(config *) conf - The license file line that worked.
 *			(int) dup_select - Duplicate selection criteria
 *			(int) status - (FEATQUEUE -> in queue, else got it)
 *			(int) flag - idle flag (0 -> not idle, != 0 -> idle)
 *
 *	Return:		None.  If the server for this feature goes
 *			away, a user-specified routine is called.  This
 *			routine is specified in lm_setup().
 *
 *			lm_auth_data() - (CONFIG *) - line that authorized
 *							the feature.
 *			lc_status() - (int) status of feature.
 *
 *	M. Christiano
 *	3/2/88
 *
 *	Last changed:  12/9/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "lgetattr.h"
#include "l_timers.h"
#include "lm_comm.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>
#ifndef NO_UIO_H
#include <sys/types.h>
#include <sys/uio.h>
#endif
#include "l_prot.h"
#include "l_fdata.h"

#ifdef USE_WINSOCK
#include <windows.h>
#endif
#include <time.h>

#ifndef RELEASE_VERSION
static char * l_check_debug = (char *)-99; 
/* #define DUMP_DEBUG*/		/* Define to output debug table dump */
extern void l_dump_check_tables lm_args((LM_HANDLE *job));
#endif


static void l_check (LM_HANDLE_PTR job);
static void install_signal lm_args((LM_HANDLE *));	
static void uninstall_signal lm_args((LM_HANDLE *));	
static void l_exitcall lm_args((LM_HANDLE *, char *));

static l_reconnect lm_args((LM_HANDLE *));
static FEATDATA *l_feat_find lm_args((LM_HANDLE *, char *, FEATDATA *, int));
static FEATDATA *l_feat_find_first_empty lm_args((LM_HANDLE *));
static l_has_floating_lic lm_args((LM_HANDLE *));		
static l_has_nodelock_lic lm_args((LM_HANDLE *));	

#ifndef RELEASE_VERSION
static char * l_dbg_feat = (char *)-99;
#endif

typedef int (LM_CALLBACK_TYPE *     LM_CALLBACK_EXIT)(char *feature);
typedef int (LM_CALLBACK_TYPE *     LM_CALLBACK_RECONNECT)(char *feature,
                                         int pass, int max, int interval);
typedef int (LM_CALLBACK_TYPE *		LM_CALLBACK_EXIT_EX)(LM_HANDLE * pJob, char * feature, void * pUserData);
typedef int (LM_CALLBACK_TYPE *		LM_CALLBACK_RECONNECT_EX)(LM_HANDLE * pJob, char * feature,
										int pass, int max, int interval, void * pUserData);
#define LM_CALLBACK_RECONNECT_TYPE_EX		(LM_CALLBACK_RECONNECT_EX)
#define LM_CALLBACK_EXIT_TYPE_EX			(LM_CALLBACK_EXIT_EX)

#define LM_CALLBACK_EXIT_TYPE           (LM_CALLBACK_EXIT)
#define LM_CALLBACK_RECONNECT_TYPE      (LM_CALLBACK_RECONNECT)

typedef int (LM_CALLBACK_TYPE * LM_CALLBACK_PERIODIC_CALL) ( void );
#define LM_CALLBACK_PERIODIC_CALL_TYPE (LM_CALLBACK_PERIODIC_CALL)

#ifndef NO_FLEXLM_CLIENT_API

int
API_ENTRY
l_check_conf(
	LM_HANDLE *		job,		/* Current license job */
	char *			feature,
	char *			version,
	int				n,
	VENDORCODE *	key,
	CONFIG *		conf,
	int				dup_select,
	int				status)		/* FEATQUEUE -> in the queue */
{
	FEATDATA *		featp = NULL;
	int				already_set = 0;
	static char *	diag = (char *)-1;

	if (diag == (char *)-1)
		diag = getenv("FLEXLM_DIAGNOSTICS");	/* overrun checked */
	if (diag && *diag >= '3')
	{
		fprintf( stderr, "Checkout succeeded: %s/%s\n", conf->feature,
				conf->code);


		fprintf( stderr, "\tLicense file: %s\n", 
					job->lic_files[conf->lf]);
		if ((job->daemon->socket >= 0) && 
				conf->server && *conf->server->name)
		{
			fprintf(stderr, "\tLicense Server: ");
			if (conf->server->port != -1)
				fprintf( stderr, "%d", conf->server->port);
			fprintf(stderr, "@%s", conf->server->name);

		}
		else
		{
			fprintf(stderr, "\tNo server used");
		}
		fprintf(stderr, "\n");
#ifndef RELEASE_VERSION
		fprintf(stderr, "strength = %d/override%d, sign_level = %d\n", 
			job->code.strength,
			job->L_STRENGTH_OVERRIDE,
			job->L_SIGN_LEVEL);
			
#endif
	}


/*
 *	Do some initialization of the feature data structures
 */
/*
 *	Turn on checking for this feature.  If no feature checking has
 *	been turned on, we must set up the timer, if the user has
 *	selected one.
 */

	featp = l_feat_find(job, feature, 0, 1);
	if (featp)
	{
/*
 *		We already have this feature - if we are reconnecting,
 *		or INQUEUE or NODELOCKED, however, we need to fill
 *		in the (newly updated) data for the feature.
 */
		if (!((featp->status & STAT_DIED) ||
		      (featp->status == STAT_INQUEUE)))
		{
			if (status == FEATQUEUE)
				featp->status = STAT_INQUEUE;
			else	
				featp->status &= STAT_NODELOCKED;
			if (featp->n < n)
				featp->n = n;
			if (l_compare_version(job, featp->ver, version) < 0) 
				strncpy(featp->ver, version, MAX_VER_LEN);
			/*
			 *	If a borrowed license, make sure that the existing entry reflects that.
			 */
			if( (conf->borrow_flags & LM_CONF_BORROWED) &&
				!(featp->conf->borrow_flags & LM_CONF_BORROWED) )
			{
				featp->conf->borrow_flags |= LM_CONF_BORROWED;
			}
			already_set = 1;
		}
	}

	if (!already_set)
	{
		if (!featp)
		{
			featp = l_feat_find_first_empty(job);
		}
		if (!featp) 
		{
			return 0;
		}

		job->feat_count++;
		(void) strcpy(featp->f, feature);
		(void) strcpy(featp->vendor_checkout_data, 
				job->options->vendor_checkout_data);
		strncpy(featp->ver, version, MAX_VER_LEN);
		featp->n = n;
		if (status == FEATQUEUE)
			featp->status = STAT_INQUEUE;
		else
			featp->status = 0;
		featp->key.type = key->type;
		featp->key.data[0] = key->data[0];
		featp->key.data[1] = key->data[1];
		featp->key.keys[0] = key->keys[0];
		featp->key.keys[1] = key->keys[1];
		featp->key.keys[2] = key->keys[2];
		featp->key.keys[3] = key->keys[3];
		featp->s = job->daemon->socket;
		featp->serialno = job->daemon->serialno;
		featp->dup_select = (short) dup_select;
		featp->conf = conf;
	}
	if (!job->timer && !job->mt_info)
	{
		install_signal(job);
	}
#ifndef RELEASE_VERSION
	if (l_dbg_feat == (char *)-99)
		l_dbg_feat = l_real_getenv("L_DEBUG_FEAT");	/* overrun checked */
	if (l_dbg_feat)
		l_dump_check_tables(job);
#endif
	return (!already_set);
}

/*
 *	Purpose:	Determines if a feature corresponds to a local borrow.
 *	Input:		job - FLEXlm job handle
 *				feature - name of feature to check
 *				code - signature of feature to check
 *	Returns:	0 - feature is not borrowed, else non 0
 */
int
l_feature_is_borrowed(
	LM_HANDLE *	job,
	char *		pszFeature,
	char *		pszCode)
{
	FEATDATA *	pFeat = NULL;
	int			iRV = 0;

	l_clear_error(job);

	pFeat = l_feat_find(job, pszFeature, pFeat, 1);
	if(pFeat)
	{
		if( (pFeat->status & STAT_BORROWED) &&
			(strcmp(pFeat->f, pszFeature) == 0) &&
			pFeat->conf &&
			(pFeat->conf->borrow_flags & LM_CONF_BORROWED) &&
			(strcmp(pFeat->conf->code, pszCode) == 0)
			)
		{
			iRV = 1;
		}
	}
	return iRV;
}

/*
 *	l_checkoff() - Turn off checking for a feature.  This happens
 *			when lc_checkin() is called.
 */

char *
l_checkoff(
	LM_HANDLE *	job,		/* Current license job */
	char *		feature,
	char *		code)
{
	FEATDATA *	featp = NULL;
	char *		rc = NULL;

	l_clear_error(job);
	while (featp = l_feat_find(job, feature, featp, 0))
	{

/* 
 *		STAT_BORROWED:  Normally, a checked out, counted feature has a
 *		file descriptor, but if it's borrowed it doesn't.
 *		We use STAT_BORROWED to indicate this.
 *		These records need to be kept.  The others are removed.
 */
		if (job->borrow_linger_minutes) 
			featp->status = STAT_BORROWED;

		featp->n = 0;

		if (!job->borrow_linger_minutes) featp->f[0] = '\0';

		featp->vendor_checkout_data[0] = '\0';
		if ((featp->status & STAT_NODELOCKED) == 0)
		{
			job->feat_count--;
			if (job->feat_count < 0) 
				    job->feat_count = 0;
			if (job->feat_count == 0)
			{
				if (job->timer || job->mt_info)
				{
					uninstall_signal(job);
				}
			}
			if (feature != LM_CI_ALL_FEATURES)
			{
				l_zcp (code, featp->conf->code, MAX_CRYPT_LEN);
				rc = code;
			}
			else
				rc = (char *)1; /* indicates success */
		}
		if (featp->conf->conf_featdata && !job->borrow_linger_minutes) 
		{
			l_free_conf(job, featp->conf);
			featp->conf = (CONFIG *)0;
		}
	}
#ifndef RELEASE_VERSION
	if (l_dbg_feat == (char *)-99)
		l_dbg_feat = l_real_getenv("L_DEBUG_FEAT");	/* overrun checked */
	if (l_dbg_feat)
		l_dump_check_tables(job);
#endif
	return(rc);
}

/*
 *	l_featon() - install a feature in the table for nodelocked, uncounted
 */
void
l_featon(
	LM_HANDLE *		job,		/* Current license job */
	char *			feature,
	char *			version,
	int				n,
	VENDORCODE *	key,
	CONFIG *		conf)
{
	int			already_set = 0;
	FEATDATA *	featp = NULL;

	featp = l_feat_find(job, feature, 0, 1);
	if (featp)
/*
 *		We already have this feature
 */
	{ 
		if (featp->n < n && n != 0)
			featp->n = n;
		if (l_compare_version(job, featp->ver, version)<0)
			strncpy(featp->ver, version, MAX_VER_LEN);
		featp->status = STAT_NODELOCKED;
		already_set++;
	}
	if (!already_set)
	{
		if (!featp) 
		{
			featp = l_feat_find_first_empty(job);
		}
		(void) strcpy(featp->f, feature);
		(void) strcpy(featp->vendor_checkout_data, 
				job->options->vendor_checkout_data);
		strncpy(featp->ver, version, MAX_VER_LEN);
		featp->n = n;
		if (n == 0) featp->n = -1;		/* So we can check against 0 */
		featp->status = STAT_NODELOCKED;
		featp->key.type = key->type;
		featp->key.data[0] = key->data[0];
		featp->key.data[1] = key->data[1];
		featp->key.keys[0] = key->keys[0];
		featp->key.keys[1] = key->keys[1];
		featp->key.keys[2] = key->keys[2];
		featp->key.keys[3] = key->keys[3];
		featp->conf = conf;
	}
#ifndef RELEASE_VERSION
	if (l_dbg_feat == (char *)-99)
		l_dbg_feat = l_real_getenv("L_DEBUG_FEAT");	/* overrun checked */
	if (l_dbg_feat)
		l_dump_check_tables(job);
#endif
}

static
int
sGetPackageInfo(
	LM_HANDLE *		job,
	CONFIG **		confp,
	VENDORCODE *	key,
	char *			version,
	LM_SERVER *		server)
{
	char			msg[LM_MSG_LEN + 1] = {'\0'};
	char *			str = NULL;
	char			id[3][MAX_HOSTID_LEN + 1];
	char *			cp = NULL;
	int				idx = 0, i = 0;
	int				ret = 0;
	CONFIG *		conf = *confp, *sav = *confp, *pkg;

	LM_SERVER *		s = NULL, * s_sav = NULL;
	LICENSE_FILE	lf;
	char			feat[MAX_CONFIG_LINE + 1] = {'\0'};
	char			feature[MAX_FEATURE_LEN + 1] = {'\0'};
	LM_SERVER_LIST *slp = NULL;
	int				package = 0;

	if (!conf || !*conf->code) 
	{
		LM_SET_ERRNO(job, LM_INTERNAL_ERROR, 528, 0); 
		return 0;
	}
	memset(msg, 0, sizeof(msg));
	msg[MSG_VD_INFO_PARAM-MSG_DATA] =  LM_VD_VERIFY_KEY;
	strcpy(&msg[MSG_VD_INFO_FEATURE-MSG_DATA], conf->feature);
	strcpy(feature, conf->feature);
	strcpy(&msg[MSG_VD_INFO_CODE-MSG_DATA], conf->code);
	if (!l_sndmsg(job, LM_VENDOR_DAEMON_INFO, msg))
		return 0;
	if (!(str = l_rcvmsg_str(job)))
		return 0;
	cp = str;
	for (idx = 0; *cp != '\n' && idx < 3; idx++)
	{
		sscanf(cp, "%s", id[idx]);	/* overrun threat */
		while (*cp != ' ')
            cp++;
		while (*cp == ' ')
            cp++;
	}
	cp++;
	memset(&lf, 0, sizeof(lf));
	lf.type = LF_STRING_PTR;
	lf.ptr.str.s = lf.ptr.str.cur = cp;

	l_lfgets(job, feat, MAX_CONFIG_LINE, &lf, 0);


	*confp = conf = (CONFIG *)l_malloc(job, sizeof(CONFIG));
	if(!conf)
	{
		LM_SET_ERROR(job, LM_CANTMALLOC, 598, 0, 0, LM_ERRMASK_ALL);
		return 0;
	}
	if (!l_parse_feature_line(job, feat, conf, 0))
	{
		free(str);
		l_free(conf);
		*confp = sav;
		return 0;
	}
	if (conf->type == CONFIG_PACKAGE) /* P5203 */
	{
		pkg = conf;
		package = 1;
		*confp = sav;
		sav->package_mask = LM_LICENSE_PKG_COMPONENT;
		sav->parent_pkg = pkg;
		l_lfgets(job, feat, MAX_CONFIG_LINE, &lf, 0);
		sav->parent_feat = conf = (CONFIG *)l_malloc(job, sizeof(CONFIG));
		if(!conf)
		{
			LM_SET_ERROR(job, LM_CANTMALLOC, 599, 0, 0, LM_ERRMASK_ALL);
			return 0;
		}
		if (!l_parse_feature_line(job, feat, conf, 0))
		{
			free(str);
			l_free(conf);
			l_free(pkg);
			sav->parent_feat = NULL;
			return 0;
		}
	}
	free(str);
	if (conf->lc_type_mask & LM_TYPE_FLOAT_OK)
	{
		conf->borrow_flags |= LM_CONF_FLOAT_OK; 
	/* 
	 *	This indicates that we checked it out from the server, 
	 *	and don't need to check the local hostid
	 */
	}
	/*
	 *	Add a server struct for this conf which has a correct hostid
	 *	Make sure it's correctly added to job->conf_server also.
	 */
	conf->server = server;

	for (s = conf->server, i = 0; i < idx ; i++, s = s->next)
	{
		if (!s)
		{
			s  = (LM_SERVER *)l_malloc(job, sizeof(LM_SERVER));
			if (s_sav)
                s_sav->next = s;
		}
		cp = id[i];
		if(s->idptr)
		{
			lc_free_hostid(job, s->idptr);
			s->idptr = NULL;
		}
		if (conf->users && l_get_id(job, &s->idptr, cp))
		{
			free(s->idptr);
			if (package)
			{
				l_free_conf(job, pkg->next);
				pkg->next = NULL;
				l_free_conf(job, pkg);
				pkg = NULL;
			}
			else
			{
				l_free_conf(job, conf);
				*confp = sav;
			}
			return 0;
		}
		s_sav = s;
	}
	if (!package) 
	{
		l_free_conf(job, sav);
		strcpy(conf->feature, feature);
	}
	ret = l_local_verify_conf(job, *confp, feature, version, key, 0, 0);
	if (package)
	{
		/*
		 *	Add parent_feat and parent_pkg to list.  This fixes a borrowing bug
		 *	where this information is not being written to registry/disk and subsequent
		 *	checkouts fail because we're unable to verify against the parent
		 *	feature/package.  This was happening when the local license file
		 *	specified USE_SERVER.  Adding this to the list will also insure
		 *	that it gets freed when the job handle is freed.  This fixes bug P6333.
		 */
		CONFIG *	pCur = job->line;

		while(pCur->next)
		{
			pCur = pCur->next;
		}

		pCur->next = (*confp)->parent_feat;
		(*confp)->parent_feat->next = (*confp)->parent_pkg;

		/*
		 *	Copy fields in enabling FEATURE/INCREMENT to the component.
		 *	This fixes bug P6376 and P7342.
		 */
		l_CopyPackageInfoToComponent(job, (*confp)->parent_feat, sav);
	}
	return ret;
}

/*
 *	Same_feat_or_pkg:  return 0=> success, non-zero failure 
 */
static
int
same_feat_or_pkg(
	LM_HANDLE *	job,
	char *		param,
	char *		feature,
	char *		code)
{
	CONFIG *	c = NULL;
	FEATDATA *	featp = NULL;

	featp = l_feat_find(job, feature, 0, 1);
	if (l_keyword_eq(job, param, feature))
		return 0;

	/* P5923 -- is it a package? */

	for (c = job->line; c; c = c->next)
	{
		if(l_keyword_eq(job, c->feature, feature))
		{
			if(!strncmp(c->code, code, strlen(code)))
			{
				/*
				 *	Check to see if this was a package, if yes, get info from server.
				 */
				if(job->savemsg[MSG_QUEUED_PACKAGE])
				{
					char	szParam[MAX_FEATURE_LEN+1] = {'\0'};

					/*	Save it for comparison later	*/
					strcpy(szParam, param);
					
					if(sGetPackageInfo(job, &c, &featp->key, featp->ver, job->daemon->server))
					{
						l_check_conf(job, feature, featp->ver,
							featp->n, &featp->key, c, featp->dup_select, 0);
						if((c->package_mask & LM_LICENSE_PKG_COMPONENT) &&
							c->parent_feat && 
							l_keyword_eq(job, c->parent_feat->feature, szParam))
						{
							return 0;
						}
					}
				}
				else
				{
					if(c->package_mask & LM_LICENSE_PKG_COMPONENT && c->parent_feat &&
						l_keyword_eq(job, c->parent_feat->feature, param))
					{
						return 0;
					}
				}
			}

		}
	}
	return LM_NOFEATURE;
}

/****************************************************************************/
/**	@brief	Determine if specified feature is a component of a PACKAGE.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	package		Name of PACKAGE
 *	@param	feature		Name of COMPONENT
 *	@param	code		Code of COMPONENT
 *
 *	@return	Non zero if feature is not a component of package, else 0.
 ****************************************************************************/
int
l_IsComponentOfPackage(
	LM_HANDLE *	job,
	char *		package,
	char *		feature,
	char *		code)
{
	return same_feat_or_pkg(job, package, feature, code);
}

/*
 *	lc_status() - return the status for a feature
 */

API_ENTRY
lc_status(
	LM_HANDLE *	job,		/* Current license job */
	char *	feature)
{
	FEATDATA *	featp = NULL;
	int			rc = NEVERCHECKOUT;

	if (LM_API_ERR_CATCH)
		return job->lm_errno;

	featp = l_feat_find(job, feature, 0, 1);
	if (featp)
	{
		if (featp->status & STAT_DIED)
			rc = CANTCONNECT;
		else if (featp->status & STAT_INQUEUE)
		{

			rc = FEATQUEUE;	/* Prepare for the worst */
/*
 *			"Normal"
 */
			while (l_msgrdy(job, LM_FEATURE_AVAILABLE))
			{
				char type = '\0', * param = NULL;
/*
 *				This could be our checkout msg - read it
 */
				if (l_rcvmsg(job, &type, &param))
				{
					if (type == LM_FEATURE_AVAILABLE)
						l_upgrade_feat(job, param);
					rc =  same_feat_or_pkg(job, param, 
							feature, featp->conf->code);
				}
			}
		}
		else
		{
			rc = 0;
		}
	}
	if (rc) 
	{
		LM_SET_ERRNO(job, rc, 20, 0);
	}
	LM_API_RETURN(int, rc)
}

/*
 *	lm_idle() - Tell FLEXlm if program is idle
 */
void
API_ENTRY
lc_idle(
	LM_HANDLE *	job,		/* Current license job */
	int			flag)
{
	if (flag && job->idle)
		return; /* already set, do nothing P3119 */
	job->idle = flag;
}

/*
 *	l_upgrade_feat() - Upgrade a feature from in queue to checked out.
 */
void API_ENTRY
l_upgrade_feat(
	LM_HANDLE *	job,		/* Current license job */
	char *		f)
{
	FEATDATA *	featp = NULL;

	featp = l_feat_find(job, f, 0, 1);
	if (featp)
	{
		if (featp->status & STAT_INQUEUE)
		{
			CONFIG *	x = NULL;

			x = (CONFIG *)l_malloc(job, sizeof(CONFIG));
			if(!x)
			{
				LM_SET_ERROR(job, LM_CANTMALLOC, 600, 0, 0, LM_ERRMASK_ALL);
				return;
			}
			l_conf_copy(job, x, featp->conf);
			featp->f[0] = '\0';	/* Erase it */
			featp->vendor_checkout_data[0] = '\0'; 
/*
 *			This conf_state indicates to l_free_job_featdata
 *			that this config was malloc'd for featdata, and
 *			should be free'd.  Normally, conf in featdata is
 *			that same conf as in the job->line, and doesn't
 *			get freed in l_free_job_featdata
 */
			x->conf_featdata = 1;

			l_check_conf(job, f, featp->ver, featp->n, &featp->key,
						x, featp->dup_select, 0);
		}
	}
}

/*
 *	lm_auth_data() - Return the license file line that authorized a feature.
 */
CONFIG *
API_ENTRY
lc_auth_data(
	LM_HANDLE *	job,		/* Current license job */
	char *		feature)
{
	if (LM_API_ERR_CATCH)
		return 0;
	LM_API_RETURN(CONFIG *, l_auth_data(job, feature))
}

CONFIG *
API_ENTRY
l_auth_data(
	LM_HANDLE *	job,		/* Current license job */
	char *		feature)
{
	FEATDATA *	featp = NULL;
	CONFIG *	conf = (CONFIG *)NULL;
	
	featp = l_feat_find(job, feature, 0, 1);
	if (featp && featp->conf)
	{
		if (featp->conf->conf_state  != LM_CONF_COMPLETE && 
			featp->conf->conf_state != LM_CONF_OPTIONS)
		{
			if ((conf = l_get_conf_from_server(job, featp->conf)) &&
					featp->conf->conf_featdata)
			{
				l_free_conf(job, featp->conf);
				featp->conf = conf;
				featp->conf->conf_featdata = 1;
			}
		}
		conf = featp->conf;
	} 
	if (!conf) 
		LM_SET_ERRNO(job, LM_NOFEATURE, 21, 0);

	return(conf);
}

/*
 *	l_check() - Check the health of the connection to the daemon.
 *	Find out if our server is still alive (Unless we are "idle")
 */
static
void API_ENTRY
l_check(LM_HANDLE * job)	/* Current license job */
{
	char			feature[MAX_FEATURE_LEN+1]; 
	int				i = 0;
	FEATDATA *		featp = NULL;
	static char *	flexlm_interval_ok = (char *)-1;

#ifdef DUMP_DEBUG
	l_dump_check_tables(job);
#endif

#ifndef RELEASE_VERSION
	if (l_check_debug == (char *)-99)
		l_check_debug = l_real_getenv("L_CHECK_DEBUG");
	if (l_check_debug)
	{
 (void) printf("l_check: inreceive: %d, feat_count: %d, socket: %d, usecount: %d\n",
			job->flags & LM_FLAG_INRECEIVE, job->feat_count, 
			job->daemon->socket, job->daemon->usecount);
	}
#endif

	/*
	 *	make sure this isn't called more than once every 20 seconds
	 *	if FLEXLM_INTERVAL_OK is set, don't do this check
	 */

	if (flexlm_interval_ok == (char *)-1)
		flexlm_interval_ok = getenv("FLEXLM_INTERVAL_OK");	/* overrun checked */
	if (flexlm_interval_ok == NULL)
	{
		long now = time(0);

		if ((now - job->last_heartbeat) < 20)
			return; /* out of here */
		job->last_heartbeat = now;
	}
		



    if ((job->idle < 2) && !(job->flags & LM_FLAG_INRECEIVE))
    {
		if (!(job->flags & LM_FLAG_RECONNECTING))
		{
			i = 0;
			if ((job->daemon->socket != LM_BAD_SOCKET) 
						&& (job->daemon->usecount > 0))
			{
				/*
				 *	    	First, find out if all is OK with the daemon.
				 *	    	select() would be sufficient for this, but we use a
				 *	    	heartbeat message for extra security, in case some
				 *	    	joker TSTPs the vendor daemon at the other end.
				 */
				if (job->daemon->commtype == LM_FILE_COMM)
					i = l_file_check(job);
				else
					i = l_rcvheart(job);	/* non-zero i => we are OK */
#ifndef RELEASE_VERSION
				if (l_check_debug == (char *)-99) 
					l_check_debug = l_real_getenv("L_CHECK_DEBUG");	/* overrun checked */
				if (l_check_debug)
					(void) printf("l_rcvheart() returns %d\n", i);
#endif
			}
			/*
			 *	    We just lost the connection to the daemon.
			 *	    Set up to start reconnection now.
			 */

			if ( (i == 0) && l_has_floating_lic(job) )
			{
#ifndef RELEASE_VERSION
				if (l_check_debug)
					(void) printf("reconnecting\n");
#endif
				job->flags |= LM_FLAG_RECONNECTING;
				for (featp = JOB_FEATDATA; featp; featp = featp->next)
				{
					if ( featp->f[0] && !(featp->status & STAT_NODELOCKED)
						&& !(featp->status & STAT_BORROWED))
					{
#ifndef RELEASE_VERSION
						 if (l_check_debug)
							(void) printf("%s set to STAT_DIED\n", 
										featp->f);
#endif
						featp->status |= STAT_DIED;
						job->feat_count--; /* Adjust for death */
						if (job->feat_count < 0) 
							job->feat_count = 0;
						lc_disconn(job, 0);
					}
				}
				lc_disconn(job, 1);		/*- Force it, just in case */
			}
			else
			{
				/*
				 *		Make sure all the features are running with this incarnation
				 *		of the daemon.  If not, get them to re-connect.
				 */
				for (featp = JOB_FEATDATA; featp; featp = featp->next)
				{
					if ( (featp->f[0] ) &&
						!(featp->status & STAT_NODELOCKED)&&
						!(featp->status & STAT_BORROWED)&&
					   (featp->serialno != job->daemon->serialno))
					{
						/*
						 *	The daemon died, and someone else opened the
						 *	connection to a new daemon.  Mark this one as
						 *	down, and re-connect for him.
						 */
						if (!(featp->status & STAT_DIED))
						{
							featp->status |= STAT_DIED;
								/*Adjust for death */
							job->feat_count--;
							if (job->feat_count < 0) 
								job->feat_count = 0;
						}
						job->flags |= LM_FLAG_RECONNECTING;
					}
				}
		
			}
			/* We have just switched to reconnection */
			if (job->flags & LM_FLAG_RECONNECTING)
			{
				if ((unsigned int)job->options->retry_count > 0 && 
									job->num_retries == 0)
				{
					/*
					 *	Reset timer interval to do retrys
					 */
					if ((job->options->retry_interval > 0) &&
							(job->timer || job->mt_info))
					{
						LM_TIMER t = (LM_TIMER )job->timer;

#if !defined(PC) && !defined(VXWORKS)


						(void) l_timer_mt_change(job, t,
									(int) LM_TIMER_UNSET,
									job->options->retry_interval * 1000,
									(FP_PTRARG)LM_TIMER_UNSET,
									(int)LM_TIMER_RETRY,
									job->options->retry_interval * 1000);
#endif
				
					}
				}
			}
		}
		if ((job->flags & LM_FLAG_RECONNECTING) || l_has_nodelock_lic(job) )
		{
			int err;
			long now = (long) time(0);

			if (job->flags & LM_FLAG_RECONNECTING)
				err = CANTCONNECT;	/* Assume we won't connect */
			else
				err = LM_NOTTHISHOST;	/* Assume host id changed */

			/*
			 *		The cast to unsigned ensures that if retry_count is -1,
			 *		it's effectively MAXINT
			 */
			if (job->num_retries < (unsigned int)job->options->retry_count)
			{
				job->num_retries++;

				job->last_failed_reconnect = now;
				if (!(err = l_reconnect(job)))
				{					
					int j, min;
					long last;
					/* 
					 *			We got all our features back - reset the
					 *			reconnection variables.
					 */
					if (job->flags & LM_FLAG_RECONNECTING)
					{
						job->flags &=  ~LM_FLAG_RECONNECTING;
						if (job->recent_reconnects)
						{
							last = 0; 
							min = job->num_minutes < 10 ? 10  : 
										job->num_minutes;
							for (j = 0; j < min; j++)
							
							{
								if (!job->recent_reconnects[j] || 
									(last > 
									job->recent_reconnects[j]))
								{
									job->recent_reconnects[j] = now;
									break;
								}
								last = job->recent_reconnects[j];
							}
							if (j == min) 
							{
								job->recent_reconnects[0] = now;
							}
						}
					}
					job->num_retries = 0;
					/*
					 *			Set the timer back for checking
					 */
					if (job->options->check_interval > 0)
					{
						LM_TIMER t = (LM_TIMER )job->timer;

#if !defined(PC) && !defined(VXWORKS)
						(void) l_timer_mt_change(job, t,
								(int) LM_TIMER_UNSET,
								job->options->check_interval * 1000,
								(FP_PTRARG)LM_TIMER_UNSET,
								(int) LM_TIMER_CHECK,
								job->options->check_interval * 1000);
#endif
				
					}
					/*
					 *			If reconnection uses a timer, but checking doesn't,
					 *			set the user's signal handler back here.
					 */
				} 
				else
				{
					if (job->options->user_reconnect || 
						job->options->user_reconnect_ex)
					{
						for (featp = JOB_FEATDATA; featp; 
								featp = featp->next)
						{
							if (featp->f[0] && (featp->status & STAT_DIED)) 
							{
								if(job->options->user_reconnect_ex)
								{
									(*LM_CALLBACK_RECONNECT_TYPE_EX
									   job->options->user_reconnect_ex)
									   (job, 
									   featp->f, 
									   job->num_retries, 
									   job->options->retry_count,
									   job->options->retry_interval,
									   job->options->pUserData);
								}

								if(job->options->user_reconnect)
								{
									(*LM_CALLBACK_RECONNECT_TYPE
									   job->options->user_reconnect)
									   (featp->f, 
									   job->num_retries, 
									   job->options->retry_count,
									   job->options->retry_interval);
								}
							}
						}
					}
				}
			}

			if ((job->num_retries >=(unsigned int)job->options->retry_count 
				&& err != 0) 
#if 0 /*P5411_FIX*/
			||
			(job->lm_errno == LM_NOTTHISHOST)
#endif
				) 
			{
				/* job->num_retries = job->reconnecting = 0;*/
				job->num_retries =  0;
				job->flags &= ~(LM_FLAG_RECONNECTING);
				for (featp = JOB_FEATDATA; featp; featp = featp->next)
				{
					if (featp->f[0] && (featp->status & STAT_DIED) 
						&& !(featp->status & STAT_BORROWED))
					{
						featp->n = 0;
						(void) strncpy(feature, featp->f, 
								MAX_FEATURE_LEN);
						feature[MAX_FEATURE_LEN] = '\0';
						featp->f[0] = '\0';
						featp->vendor_checkout_data[0] = '\0';
						/* reset the error */
						LM_SET_ERRNO(job, err, 23, 0);
						l_exitcall(job, feature);
					}
				}
			}
		}
    }
    if (job->idle) job->idle++;
}

/*
 *	l_exitcall -- processing when all else has failed
 */
static
void
l_exitcall(
	LM_HANDLE *	job,		/* Current license job */
	char *		_f)
{
	if (l_getattr(job, LM_SET_ATTR) != LM_SET_ATTR_VAL)
		return; /* let them discover this via lc_status() */

	if (job->options->user_exitcall || 
		job->options->user_exitcall_ex)
	{
		if(job->options->user_exitcall_ex)
			(*LM_CALLBACK_EXIT_TYPE_EX job->options->user_exitcall_ex)(job, _f, job->options->pUserData);

		if(job->options->user_exitcall)
			(*LM_CALLBACK_EXIT_TYPE job->options->user_exitcall)(_f);
	}
	else
	{
		lc_perror(job, "Lost license, cannot re-connect");
#ifdef PC
		_exit(job->lm_errno);
#else /* Not PC */
		exit(job->lm_errno);
#endif /* PC */
	}
}


/*
 *	l_reconnect() - Reconnect to server for ALL checked-out features
 */

static
l_reconnect(LM_HANDLE *	job)		/* Re-connect to a server that has died */
{
	int			failed = 0;
	char		sav_vendor_checkout_data[MAX_VENDOR_CHECKOUT_DATA+1] = {'\0'};
	FEATDATA *	fd = NULL;

/*	
 *	Save current vendor_checkout_data
 */
	(void) strcpy(sav_vendor_checkout_data, job->options->vendor_checkout_data);

	for (fd = JOB_FEATDATA; fd; fd = fd->next)
	{
/*		
 *		Use this feature's vendor_checkout_data to re-checkout
 *		this feature
 */
		(void) strcpy(job->options->vendor_checkout_data,
					    fd->vendor_checkout_data);
		if (fd->f[0] && ((fd->status & STAT_DIED) ||
				  (fd->status & STAT_NODELOCKED) ||
				  (fd->status & STAT_BORROWED)))	  
		{
			/*
			 *	Must preserve status.  Lc_checkout calls
			 *	l_check when checkout succeeded, that changes
			 *	status.
			 */
			int org_status = fd->status;
			int flag = LM_CO_NOWAIT;
			int retcode;
		
			if ( org_status & STAT_INQUEUE ) 
			{
				flag = LM_CO_QUEUE;
			}
									     

/*
 *			If we're using the server, set the lic-key to
 *			PORT_HOST_PLUS -- this way if the license file
 *			has changed, the checkout will still succeed
 */
			if ((job->num_retries == 2) &&
					!(fd->status & STAT_NODELOCKED) &&
					!(fd->status & STAT_BORROWED))
			{
				CONFIG *	conf = NULL;
				conf = l_malloc(job, sizeof(CONFIG));
				if(!conf)
				{
					LM_SET_ERROR(job, LM_CANTMALLOC, 601, 0, 0, LM_ERRMASK_ALL);
					return failed = LM_CANTMALLOC;
				}
				l_conf_copy(job, conf, fd->conf); /* P4626 */
				strcpy(conf->code, 
					CONFIG_PORT_HOST_PLUS_CODE);
				conf->type = CONFIG_PORT_HOST_PLUS;
				conf->next = fd->conf->next;
				fd->conf->next = conf;
			}
				
			retcode = l_checkout(job, fd->f, fd->ver, fd->n, 
				flag, &(fd->key), fd->dup_select);
			
			if ( ((flag == LM_CO_NOWAIT) && retcode) ||
				((flag == LM_CO_QUEUE) && 
				(retcode != LM_MAXUSERS) && 
				(retcode != LM_FEATQUEUE)))
			{
				failed = job->lm_errno;
				fd->status |= STAT_DIED;			
			}
			else
			{
				if (org_status&STAT_DIED)
				{
					if (job->options->user_reconnect_done ||
						job->options->user_reconnect_done_ex)
					{
						if(job->options->user_reconnect_done_ex)
						{
							(*LM_CALLBACK_RECONNECT_TYPE_EX  job->options->user_reconnect_done_ex)
								(job, fd->f, job->num_retries, job->options->retry_count,
								 job->options->retry_interval, job->options->pUserData);
						}
						if(job->options->user_reconnect_done)
						{
							(*LM_CALLBACK_RECONNECT_TYPE  job->options->user_reconnect_done)
								(fd->f, job->num_retries, job->options->retry_count,
								 job->options->retry_interval);
						}
					}
				}
				fd->status &= ~STAT_DIED;			
			}
		}
	}
/*	
 *	Copy it back.
 */
	(void) strcpy(job->options->vendor_checkout_data,
		    sav_vendor_checkout_data);
	return(failed);
}
#endif /* NO_FLEXLM_CLIENT_API */



#ifndef RELEASE_VERSION
void
l_dump_check_tables(LM_HANDLE * job)
{
	FEATDATA *	fd = NULL;

	for (fd = JOB_FEATDATA; fd; fd = fd->next)
	{
		if (fd->f && *fd->f) 
		{
			(void) printf("Feature: %s [%s], #: %d, status: %d, socket: %d, serial#: %d\n",
							fd->f, fd->vendor_checkout_data, fd->n, fd->status, fd->s, fd->serialno);
		}
	}
}
#endif

#ifndef NLM 
/*
 *		Start the timers, if need be
 */
static 
void
install_signal(LM_HANDLE * job)	/* Current license job */
{
	if (job->options->check_interval > 0)
	{
		
		job->timer = 
				l_timer_mt_add (job, 
				job->options->timer_type,
				job->options->check_interval * 1000, 
				(FP_PTRARG)l_timer_heart, 
				LM_TIMER_CHECK, 
				job->options->check_interval * 1000);
	}
}

/*
 *	uninstall_signal(job) -- delete the job->timer
 */
static 
void 
uninstall_signal(LM_HANDLE * job)	/* Current license job */
{
	l_timer_mt_delete (job, job->timer);
	if ( job->timer )
		job->timer = 0;
}

/*
 *	l_feat_find(job, feature, last)  -- find a feature in FEATDATA
 */
static 
FEATDATA *
l_feat_find(
	LM_HANDLE *	job,		/* Current license job */
	char *		feature,
	FEATDATA *	lastfd, 
	int			borrowok)	/*
							 *	if !borrowok, then we don't want the
							 *	records which are already marked as STAT_BORROWED
							 *	-- so that we won't attempt to remark them
							 */
{
	FEATDATA *	fd = NULL;

	for (fd = lastfd ? lastfd : JOB_FEATDATA; fd; fd = fd->next)
	{
		if (feature == LM_CI_ALL_FEATURES)
		{
			if  (fd->n > 0)
				return fd;
			else
				return 0;
		}
		if (l_keyword_eq(job, fd->f, feature) 
			&& !strcmp(fd->vendor_checkout_data, 
				    job->options->vendor_checkout_data)
			&& (
				(fd->n > 0) ||
				borrowok && (fd->status == STAT_BORROWED)))
		{
			return fd;
		}
	}
	return (FEATDATA *)NULL;
}

/*
 *	l_feat_find_first_empty() -- find the first blank FEATDATA slot
 */

static 
FEATDATA *
l_feat_find_first_empty(LM_HANDLE * job)	/* Current license job */
{
	FEATDATA *	fd = (FEATDATA *)NULL, * lastfd = (FEATDATA *) NULL;

	for (fd = JOB_FEATDATA; fd && *fd->f ; fd = fd->next)
		lastfd = fd;
		
	if (fd == (FEATDATA *) NULL)
	{
		return fd = (FEATDATA *)l_more_featdata(job, lastfd);
	}
	return fd;
}

static l_has_floating_lic(LM_HANDLE * job)
{
	FEATDATA *	featp = NULL;
	
	for (featp = JOB_FEATDATA; featp; featp = featp->next)
	{
		if (!(featp->status & STAT_NODELOCKED)
			&& !(featp->status & STAT_BORROWED))
		{
			return 1;
		}
	}
	return 0;
}

static l_has_nodelock_lic(LM_HANDLE * job)
{
	FEATDATA *	featp = NULL;
	
	for (featp = JOB_FEATDATA; featp; featp = featp->next)
	{
		if (featp->status & STAT_NODELOCKED)
			return 1;
	}
	return 0;	
}
#endif /* NLM */
/*
 *	lc_timer() - Interval timer signal handler - checks health
 *			of connection to server by calling l_check()
 */

/*typedef int (LM_CALLBACK_TYPE * LM_CALLBACK_PERIODIC_CALL) ( void );*/
#define LM_CALLBACK_PERIODIC_CALL_TYPE (LM_CALLBACK_PERIODIC_CALL)

int
API_ENTRY
lc_timer(LM_HANDLE * job) /* Current license job */
{
	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	LM_API_RETURN(int, l_timer_heart(job))
}

int
API_ENTRY
l_timer_heart(LM_HANDLE * job)	/* Current license job */
{

/*
 *	Call the user back, if he requested it.  Check job->type and
 *	job->options against job handle which was freed.
 */

	if ( job->type && job->options )    
	{
		if (job->options->periodic_call)
		{
			job->options->periodic_counter++;
			if (job->options->periodic_counter == 
					job->options->periodic_count)
			{
				job->options->periodic_counter = 0;
				(*	LM_CALLBACK_PERIODIC_CALL_TYPE job->options->periodic_call)();
			}
		}
	}
/*
 *	Check the connection
 */
	l_check(job);
	return job->num_retries;
}
