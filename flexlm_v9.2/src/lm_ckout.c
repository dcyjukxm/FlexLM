/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovisino Inc and is protected by law.  It may
	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Inc.

 *****************************************************************************/
/*
 *	Module: $Id: lm_ckout.c,v 1.141.2.1 2003/07/02 23:50:41 sluu Exp $
 *
 *	Function: lc_checkout(job, feature, version, nlic, flag, key, dup_group)
 *
 *	Description: "checks out" a copy of the specified feature for use.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) feature - The ascii feature name desired.
 *			(char *) version - The version of the software.
 *			(int) nlic - The number of licenses you want.
 *			(int) flag - Checkout flag (see lmclient.h)
 *			(VENDORCODE *) key - The vendor-specific encryption
 *						seeds for this feature.
 *			(int) dup_group - The method for grouping duplicate
 *					   licenses, either LM_DUP_NONE, or
 *					   any combination of:
 *						LM_DUP_USER, LM_DUP_HOST,
 *						LM_DUP_DISP
 *
 *	Return:		(int) - 0 - OK, ie. we are running on the specified
 *					host.
 *				NOTTHISHOST - We are not running on the
 *						specified host.
 *
 *	M. Christiano
 *	2/18/88
 *
 *
 */

/*
 *	Notes: Define NO_ENCRYPTION_CHECK to build this into lmutil.
 *		This way, we can do a checkout without knowing the codes.
 */
#include "lmachdep.h"
#ifdef PC
#include <malloc.h>
#else
#include <errno.h>
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include <stdio.h>
#include "lm_comm.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef FNDELAY_IN_FILE_H
#include <sys/file.h>
#endif /* FNDELAY */
#include "l_borrow.h"

#ifndef PC
#ifdef ANSI
#include <stdlib.h>
#endif /* ANSI */
#endif /* !PC */
#define PHASE3

/***********************************************************************
 * file static function forwards
 ************************************************************************/
static int verify_conf_restrictions lm_args(( LM_HANDLE *, char *, CONFIG *));
static int verify_server_key lm_args(( LM_HANDLE *, CONFIG **, VENDORCODE *,
					char *, LM_SERVER *));
/*-
 *	We have our own private copies of many functions, as a way
 *	to thwart theft
 */
#define l_crypt_private l_ckout_crypt
#define l_string_key	l_ckout_string_key
static LM_CHAR_PTR API_ENTRY    l_ckout_crypt lm_args((LM_HANDLE_PTR job,
				      CONFIG_PTR conf, LM_CHAR_PTR sdate,
					      VENDORCODE_PTR code));
static LM_U_CHAR_PTR API_ENTRY
		l_ckout_string_key	lm_args((LM_HANDLE_PTR, LM_U_CHAR_PTR,
				int, VENDORCODE_PTR, unsigned long, char *));

#ifdef PC16
API_ENTRY l_baddate(LM_HANDLE_PTR);
int net_init=0;
#endif /* PC16 */

#ifdef USE_WINSOCK
#include <pcsock.h>
#if !defined(NO_ENCRYPTION_CHECK) && !defined(NLM)
#include "stderr.h"
#endif
#endif /* USE_WINSOCK */

#if defined(PC)
typedef int (LM_CALLBACK_TYPE * LM_CALLBACK_OUTFILTER) (CONFIG * conf);
#define LM_CALLBACK_OUTFILTER_TYPE (LM_CALLBACK_OUTFILTER)

typedef int (LM_CALLBACK_TYPE * LM_CALLBACK_OUTFILTER_EX) (LM_HANDLE * pJob, CONFIG * conf, void * pUserData);
#define LM_CALLBACK_OUTFILTER_TYPE_EX (LM_CALLBACK_OUTFILTER_EX)

#else
#define LM_CALLBACK_OUTFILTER_TYPE
#define LM_CALLBACK_OUTFILTER_TYPE_EX
#endif

#ifdef WINNT
#include "lm_attr.h"
#endif

#ifdef ENCRYPT
static lm_decrypt_code();
static lm_dummy();
static int first = 1;
#endif
static lm_start_real();
static int (*lm_start)();
static int good_lic_key lm_args((LM_HANDLE *, CONFIG *, VENDORCODE *));
static int l_send_hostid lm_args(( LM_HANDLE *, int, int));
static int checkout_from_server lm_args((LM_HANDLE *, int, CONFIG * *, int,
				char *, int *, VENDORCODE *, char *, int, int *,
				int *, char *));
static unsigned short last_ckout_sernum;


API_ENTRY
lc_checkout(
	LM_HANDLE_PTR			job,		/* Current license job */
	const LM_CHAR_PTR		feature,	/* The feature to be checked in */
	const LM_CHAR_PTR		version,	/* Feature's version */
	int						nlic,		/* Number of licenses to checkout */
	int						flag,		/* Checkout flag */
	const VENDORCODE_PTR	key,		/* The vendor's key */
	int						dup_group)	/* Duplicate license grouping criteria */
{
	int ret = 0;

	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	ret = l_checkout(job, feature, version, nlic, flag, key, dup_group);
	if (!ret)
	{
		l_clear_error(job);
		if(job->flags & LM_FLAG_BORROWED_CHECKOUT)
		{
			/*
			 *	Do the checkout again.
			 */
			job->flags ^= LM_FLAG_BORROWED_CHECKOUT;
			ret = l_checkout(job, feature, version, nlic, flag, key, dup_group);
			if(!ret)
				l_clear_error(job);
		}
	}
	LM_API_RETURN(int, ret);
}

int API_ENTRY
l_checkout(
	LM_HANDLE_PTR			job,		/* Current license job */
	const LM_CHAR_PTR		feature,	/* The feature to be checked in */
	const LM_CHAR_PTR		version,	/* Feature's version */
	int						nlic,		/* Number of licenses to checkout */
	int						flag,		/* Checkout flag */
	const VENDORCODE_PTR	key,		/* The vendor's key */
	int						dup_group)	/* Duplicate license grouping criteria */
{
	int			foo = 0, iReturn = 0;
	int			err = 0;
	extern int	l_ckout_borrow( LM_HANDLE_PTR, const LM_CHAR_PTR,
					const LM_CHAR_PTR, int, int, const VENDORCODE_PTR, int);

	if (feature && job )
		l_zcp(job->err_info.feature, feature, MAX_FEATURE_LEN);

	if (l_getattr(job, CHECKOUT) != CHECKOUT_VAL)
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 112, 0);
		return(job->lm_errno);
	}

	if ((feature == (char *) NULL) ||
	    (key == (VENDORCODE *) NULL)
		|| (nlic == 0)
		)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 113, 0);
		return(job->lm_errno);
	}
	/*
	 *	I'm trying to make this thing thread safe
	 *	The only possible hole is 2 threads at this line at the
	 *	same time both getting the same number.  Even then it's
	 *	only a problem if the feature name is the same...
	 */
	job->last_ckout_sernum = last_ckout_sernum++;
	/*
	 *	If we are encrypting the routine, malloc it and move the
	 *	code there, so that we can write to it.
	 */
	lm_start = lm_start_real;
	job->flags |= LM_FLAG_CHECKOUT;
	/*
	 *	Always try the local borrow file first.  This doesn't "do" the
	 *	borrow, but uses a local borrow file for checkout if a borrow
	 *	has previously been done.
	 */
	if (l_ckout_borrow(job, feature, version, nlic, flag, key, dup_group))
	{
		err = (*lm_start)(job, feature, version, nlic, flag, key, dup_group);
	}
	job->flags &= ~LM_FLAG_CHECKOUT;

	if (job->options->flags & LM_OPTFLAG_RETRY_CHECKOUT &&
		(err == LM_BADCOMM 	|| err == LM_CANTREAD ||
		err == LM_CANTCONNECT 	|| err == LM_CANTWRITE ||
		err == LM_SERVBUSY 	|| err == LM_BADHANDSHAKE ||
		err == LM_NOSERVRESP 	|| err == LM_SOCKETFAIL ||
		err == LM_SETSOCKFAIL 	|| err == LM_SERVBADCHECKSUM ||
		err == LM_SERVNOREADLIC ))
	{
		/*
		 *	If it's a communications error, we try again
		 */
		err = (*lm_start)(job, feature, version, nlic, flag, key,
			dup_group);
	}

	if (err && !*job->daemon->daemon)
	{
		CONFIG *c;
		for (c = job->line; c; c = c->next)
		{
			if ( (c->type == CONFIG_FEATURE ||
				 c->type == CONFIG_INCREMENT ) && l_keyword_eq( job, c->daemon, job->vendor))
			{
				break;
			}
		}
		if (!c) /* check if we should prompt the user */
		{
			job->err_info.flags |= LM_EIFLAG_NOVENDOR_FEATS;
		}
	}

#if defined(WINNT) && !defined(NO_ENCRYPTION_CHECK)
/* P2885 */
	if ( ( job->options->flags & LM_OPTFLAG_PC_PROMPT_FOR_FILE ) &&
		(( job->err_info.flags & LM_EIFLAG_NOVENDOR_FEATS ) ||
		(err == LM_NOFEATURE) ) )
	{

		char		szFile1[256] = {'\0'};
		int			rc = 0;
		CONFIG *	fl_conf;
		BOOL		skip_dialog =   FALSE;
		BOOL		skip_checkout = FALSE;
		BOOL		demo_enabled =  FALSE;
		BOOL		skip_locate =   FALSE;
		LM_ERR_INFO e;
		char *		saved_license_file = NULL;


		if ( err == LM_NOFEATURE  &&
			!( job->err_info.flags & LM_EIFLAG_NOVENDOR_FEATS  ) )
		{
			skip_locate = TRUE;
		}

		memset(&e, 0, sizeof(e));
		l_err_info_cp(job, &e, &job->err_info);

		/* Call Flexlock to see if this is installed  */


		if ( job->options->flags & LM_OPTFLAG_FLEXLOCK )
		{
			if (NULL == job->options->config_file)
				saved_license_file = NULL;
			else
			{
				saved_license_file= ( char* ) l_malloc( job,
					strlen(job->options->config_file ) +10);
				if ( saved_license_file )
					strcpy( saved_license_file,
					job->options->config_file );
			}

			rc = fl_available( job, feature , & fl_conf );

			if ( saved_license_file )
			{
				lc_disconn(job, 1);
				l_free_job_license(job);
				l_set_attr( job, LM_A_LICENSE_FILE,
					(LM_A_VAL_TYPE) saved_license_file );
				l_free( saved_license_file );
				l_flush_config(job);
			}

			switch ( rc )
			{
			case 0 :
				/*  FLEXlock is not available */
				/* disable demo button */
				break;

			case 1:
				/* Customer pressed Locate File/server button */
				/* disable Demo button */
				break;

			case 2:
				/* Yes, but not installed yet  */
				/* enable demo button  */
					demo_enabled=TRUE;
				break;

			case 3:
				/*  Yes, and it is running, use conf */
				/* skip putting up dialog.  Also load the */
				/* id as the LM_A_FLEXLOCK_INSTALL_ID  */
				skip_dialog=TRUE;
				skip_checkout = TRUE;
				l_install_conf( job , fl_conf );
				if( fl_conf->idptr )
				{
					job->options->flexlock_install_id =
						(long) fl_conf->idptr->id.data;
				}
				err = 0;
				break;

			case 4: /*  yes, but demo has expired  */
				/*  skip putting up dialog, and skip 2nd
				 *  checkout  */
				skip_dialog=TRUE;
				skip_checkout=TRUE;
				LM_SET_ERRNO(job, LM_NOCONFFILE, 514, 0);
				err = LM_NOCONFFILE;
			}


		}

		l_err_info_cp(job, &job->err_info, &e);
		l_free_err_info(&e);

/*  		If we are not to skip putting up the dialog, put it up and see
 *  		what happens. If it returns nonzero, then we will do the second
 *  		checkout
 */
		if ( ! skip_dialog  )
		{
			rc = l_prompt_lf(job, feature, &szFile1, demo_enabled,
							skip_locate );
			 /* the user made a selection */
			switch (rc )
			{
			case 0 : /* Cancel Pressed */
				skip_checkout = TRUE;

//				LM_SET_ERRNO(job, LM_NOCONFFILE, 515, 0);
//				err = LM_NOCONFFILE ;
				break;

			case 1: /* File/ Fulfill / Server Selected */
				break;

			case 2: /* Flexlock chosen */
				rc = fl_install( job, feature , & fl_conf );
				if ( 3 == rc )
				{
					skip_checkout = TRUE;
					l_install_conf (job, fl_conf );
					if( fl_conf->idptr )
					{
					    job->options->flexlock_install_id =
							(long) fl_conf->idptr->id.data;
					}
					err = 0 ;
				}
				else  /*  Pressed Cancel */
				{
					skip_checkout = TRUE;
				}
			}
		}

		/*(P4852) */
		fl_freedyndata();


		if  ( ! skip_checkout )
		{
		/* set the license file to the new value */
			l_set_attr(job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)szFile1);
		/* retry the new license file setting */
			l_clear_error(job);
			l_flush_config(job);
			err = (*lm_start)(job, feature, version, nlic, flag, key, dup_group);
			if ( !err )
			{
				l_update_lm_license(job,szFile1,job->vendor);
			}
		}

	}
#endif /* defined(WINNT) && !defined(NO_ENCRYPTION_CHECK) */




#ifdef ENCRYPT
	if (job->options->decrypt_flag)
		lm_decrypt_code(job, lm_start, lm_dummy);
#endif
	{
        CONFIG *cf;
		for (cf = job->line; cf; cf = cf->next)
			if (cf->L_CONF_FLAGS & L_CONF_FL_REJECT)
				cf->L_CONF_FLAGS &= ~L_CONF_FL_REJECT;
	}
	return(err);
}

static CONFIG *testconf = (CONFIG *) NULL;




static
int
sGetPriority(int iError)
{
	int	iRet = 0;

	switch(iError)
	{
	case LM_MAXUSERS:
		iRet = 0;
		break;

	case LM_TOOMANY:
		iRet = 1;
		break;

	case LM_USERSQUEUED:
		iRet = 2;
		break;

	case LM_MAXLIMIT:
		iRet = 3;
		break;

	case LM_RESVFOROTHERS:
		iRet = 4;
		break;

	case LM_BADFEATPARAM:
		iRet = 5;
		break;

	case LM_CHECKOUTFILTERED:
		iRet = 7;
		break;

	case LM_FEATNOTINCLUDE:
		iRet = 7;
		break;

	case LM_BADPLATFORM:
		iRet = 8;
		break;

	case LM_NOFEATURE:
		iRet = 9;
		break;

	case LM_NOSERVSUPP:
		iRet = 9;
		break;

	case LM_NONETOBORROW:
		iRet = 9;
		break;

	case LM_BORROW_TOOLONG:
		iRet = 9;
		break;

	case LM_TOOEARLY:
		iRet = 10;
		break;

	case LM_PLATNOTLIC:
		iRet = 10;
		break;

	case LM_SERVLONGGONE:
		iRet = 10;
		break;

	case LM_NOBORROWSUPP:
		iRet = 10;
		break;

	default:
		iRet = 20;
		break;
	}
	return iRet;
}

static
void
sUpdateServer(CONFIG **	ppUltimateDenialServer,
	      CONFIG *	pCurrentServer,
	      int *	piLastError,
	      int	iCurrentError)
{
	if(ppUltimateDenialServer && pCurrentServer)
	{
		if(sGetPriority(*piLastError) >= sGetPriority(iCurrentError))
		{
			*ppUltimateDenialServer = pCurrentServer;
			*piLastError = iCurrentError;
		}
	}
}


static
void
sHandleUD(CONFIG **	ppUDServer,
	  CONFIG *	pCurConf,
	  int *		piLastErr,
	  int		iCurErr)
{
	switch(iCurErr)
	{
		case LM_MAXUSERS:			/*  -4		Maximum number of users reached */
		case LM_NOFEATURE:			/*	-5		No such feature exists */
		case LM_LONGGONE:			/*	-10		Software Expired */
		case LM_NOSERVSUPP:			/*	-18		Server does not support this feature */
		case LM_USERSQUEUED:		/*	-24		Users already in queue for this feature */
		case LM_SERVLONGGONE:		/*	-25		Version not supported at server end */
		case LM_TOOMANY:			/*	-26		Request for more licenses than supported */
		case LM_TOOEARLY:			/*	-31		Start date for feature not reached */
		case LM_BADFEATPARAM:		/*	-37		dup_select mismatch for this feature */
		case LM_FEATNOTINCLUDE:		/*	-39		User/host not in INCLUDE list for feature */
		case LM_BADPLATFORM:		/*	-48		FLEXlm platform not enabled */
		case LM_CHECKOUTFILTERED:	/*	-53		Request rejected by vendor-defined filter */
		case LM_NONETOBORROW:		/*	-67		No licenses to borrow */
		case LM_NOBORROWSUPP:		/*	-68		License BORROW support not enabled */
		case LM_SERVOLDVER:			/*	-83		Server FLEXlm version older than client's */
		case LM_MAXLIMIT:			/*	-87		Checkout exceeds MAX specified in options file */
		case LM_PLATNOTLIC:			/*	-89		This platform not authorized by license */
		case LM_POOL:				/*	-93		This feature is available in a different license pool */
		case LM_RESVFOROTHERS:		/*	-101	All licenses are reserved for others" */
		case LM_BORROW_TOOLONG:		/*	-104	Cannot borrow that long */
		case LM_UNBORROWED_ALREADY:	/*	-105	Feature already returned to server */
		case LM_NOBORROWCOMP:		/*	-107	Can't borrow a PACKAGE component */
		case LM_BORROW_METEREMPTY:	/*	-108	Licenses all borrowed or meter empty */
			sUpdateServer(ppUDServer, pCurConf, piLastErr, iCurErr);
			break;
		default:
			break;
	}
}

/***********************************************************************
 * Is there a checkout filter set?
 ************************************************************************/
static int haveOutFilter(LM_HANDLE *job)
{
	if (job->options->outfilter || job->options->outfilter_ex || job->options->outfilter_last_ex )
		return 1;
	else
		return 0;
}
/***********************************************************************/
/** @brief Do the real work to checkout a license
 *
 * @param	job			Current license job
 * @param	feature		The feature to be checked in
 * @param	version		Version of the feature
 * @param	nlic		The number of licenses to checkout
 * @param	flag		Wait flag
 * @param	key		 	The vendor's key
 * @param	dup_group   Duplicate license grouping criteria
 *
 * @return standard error values
 *
 ************************************************************************/
static int lm_start_real(
		LM_HANDLE *			job,		/* Current license job */
		char *				feature,	/* The feature to be checked in */
		char *				version,	/* Version of the feature */
		int					nlic,		/* The number of licenses to checkout */
		int					flag,		/* Wait flag */
		VENDORCODE *		key,		/* The vendor's key */
		int					dup_group)/* Duplicate license grouping criteria */
{
	CONFIG *	conf = NULL;
	CONFIG *	pUDServer = NULL;
	CONFIG *	filepos = NULL;		/* Always start at 0 */
	int			gotmaxusers = 0;
	int			oldflags = 0;
	int			resetsocket = 0;
	int			totpass = 1;
	int			passnum = 0;
	int			realflag = 0;
	int			marker = 1;
	int			ret = 0;
	char		last_server[MAX_HOSTNAME + 1 ] = { "" };
	int			iLastError = 0;

	/*
	 *	    Check for valid version, return if no good
	 */
	if (!l_valid_version(version))
	{
		LM_SET_ERROR(job, LM_BAD_VERSION, 131, 0, version,
					LM_ERRMASK_ALL);
		return (LM_BAD_VERSION);

	}

	/*
	 *	If he wants to be queued, don't queue the first pass through here,
	 *	since we would then queue and checkout the same license!
	 */

	if (flag == LM_CO_QUEUE || flag == LM_CO_WAIT)
	{
		realflag = flag;
		totpass = 2;
		flag = 0;
	}

	/*
	 *	l_clear_error is purpose down here.  I don't want it to
	 *	be the first instruction in lc_checkout().  But, it must appear
	 *	before any return occurs that might be a "normal"  return.
	 *	Then we have to reset the feature name.
	 */
	l_clear_error(job);

	if (feature && job )
		l_zcp(job->err_info.feature, feature, MAX_FEATURE_LEN);


	for (passnum = 1; passnum <= totpass; passnum++)
	{
		if (passnum == 2)
		{
			flag = realflag;
			ret = 0;
		}

		/*************************************************************
		 * Kirk MacLean 5/7/2003
		 * P7014
		 * I took out a lot of code here dealing with port at host if a
		 * checkout filter is set. It all seems to work right when I
		 * set the marker flag based upon the existance of  a
		 * checkout filter.
		 **************************************************************/
		marker = ! haveOutFilter(job);
		while (conf = l_next_conf_or_marker(job, feature, &filepos,	marker, (char *)0))
		{

			/*
			 *	P3057:  the LOCALTEST/CONF_COMPLETE test below was added to
			 *		fix this bug
			 */
			if ((conf->type != CONFIG_PORT_HOST_PLUS) &&
				(((flag == LM_CO_LOCALTEST) && (conf->conf_state != LM_CONF_COMPLETE)) ||
				 !l_local_verify_conf(job, conf, feature, version, key, flag, 1)) )
			{
				if (flag != LM_CO_LOCALTEST ||
					!l_local_verify_conf(job, conf,	feature, version, key, flag, 1))
				{
					continue;
				}
			}

			/* Kirk MacLean 5/7/2003
			 * I think "conf->users || conf->type == CONFIG_PORT_HOST_PLUS"
			 * is unnecessary but the old code did this so I'm leaving it here.
			 */
			/*
			 *	sluu: 7/2/03
			 *	Added additional check of lc_from_server per kmac's request.  This is to fix
			 *	a problem where a node locked uncounted license was being checked out from
			 *	the license server, even if a checkout filter wasn't specified.
			 */
			if( (conf->type == CONFIG_PORT_HOST_PLUS) || (conf->users != 0) || (conf->lc_from_server) )
			{
				int rc = 0;

				/*
				 *				IF we are only doing the local checks, we
				 *				are done.
				 */
				if ((flag == LM_CO_LOCALTEST) &&
					(conf->type != CONFIG_PORT_HOST_PLUS) &&
					(conf->conf_state == LM_CONF_COMPLETE))
				{
					testconf = conf;
					ret = 0;
					break; /* all done */
				}

#ifndef NO_FLEXLM_CLIENT_API

				rc =  checkout_from_server(job, flag, &conf,
							nlic, feature, &gotmaxusers, key,
							version, dup_group, &resetsocket,
							&oldflags, last_server);
				if (rc <= 0)
				{
					ret = rc;
					break;

				}

				if(conf && conf->server && (conf->server->sflags & L_SFLAG_REPORT_LOG_ENABLED))
				{
					sHandleUD(&pUDServer, conf, &iLastError, job->lm_errno);
				}
				continue;
#endif /* NO_FLEXLM_CLIENT_API */
			}
			/*
			 *	    		Check to see if we are in demo mode or nodelocked
			 */
			else if (conf->idptr)
			{
				/*
				 *	    	re-encrypt the data, compare to the code in the file.
				 */
#ifndef NO_ENCRYPTION_CHECK
				if (conf->package_mask & LM_LICENSE_PKG_COMPONENT)
				{
					if (!conf->parent_pkg || !l_good_lic_key(job,conf->parent_pkg, key))
						continue;
					if (!conf->parent_feat || !l_good_lic_key(job,conf->parent_feat, key))
						continue;
				}
				else if (!l_good_lic_key(job, conf, key))
					continue;
				/*
				 *		Inform the l_check_conf module we have this license.
				 *		(If we are only doing the local test, don't "checkout"
				 *		 the license)
				 */
#ifndef NO_FLEXLM_CLIENT_API
				if (flag != LM_CO_LOCALTEST)
				{
					l_featon(job, feature, version, nlic, key,conf);
					/*
					 * 	Start periodic check for nodelocked
					 *	licenses.
					 */
					l_check_conf(job, feature, version, nlic, key, conf, dup_group, 0);
				}
				else
#endif /* NO_FLEXLM_CLIENT_API */
					testconf = conf;
#endif /* NO_ENCRYPTION_CHECK */
				job->lfptr = conf->lf; /* P4782 */
				ret = 0;
				break;
			}
			/*
			 *	    		hostid required but not there.
			 */
			else
			{
				if (conf->lc_future_minor)
				{
					LM_SET_ERRNO(job, LM_FUTURE_FILE, conf->lc_future_minor, 0);
				}
				else if (!job->lm_errno)
				{

					LM_SET_ERRNO(job, LM_BADFILE, 134, 0);
				}
			}
		} /* while */


		if (!conf)
		{
			/*
			 *	Handle ultimate denial message
			 */
			if(pUDServer)
			{
				/*
				 *	Try to checkout license again, this time specify to the
				 *	server to log an ultimate denial if unsuccessful.
				 */
				int	iRC = 0;
				int	iNewFlag = flag;

				iNewFlag |= LM_FLAG_LOG_UD_ON_FAILURE;
				iRC =  checkout_from_server(job, iNewFlag, &pUDServer,
							nlic, feature, &gotmaxusers, key,
							version, dup_group, &resetsocket,
							&oldflags, last_server);
				if (iRC <= 0)
				{
					ret = iRC;
					conf = pUDServer;
					goto gotit;
				}
			}

			if (job->lm_errno)
				ret = job->lm_errno;
			else if (!job->line)
			{
				LM_SET_ERRNO(job, LM_BADFILE, 413, 0);
				ret = LM_BADFILE;
			}
			else /*- ??? what else ??? */
			{
				LM_SET_ERRNO(job, LM_NOFEATURE, 414, 0);
				ret = LM_NOFEATURE;
			}
		}

gotit:

		if ((!ret) || (ret == LM_FEATQUEUE)) break;

		/* IF OK, we are done */
		filepos = 0;		/* start back at 0 */

	} /* for (passnum ...) */

#ifndef NO_FLEXLM_CLIENT_API

	if (flag == LM_CO_WAIT && resetsocket &&
					(job->daemon->socket!=LM_BAD_SOCKET))
		/*
		 *	Set socket back
		 */
	{
#ifdef PC
		u_long one = 1;
		network_control(job->daemon->socket, FIONBIO, &one);
#else
		network_control(job->daemon->socket, F_SETFL, oldflags);
#endif /* PC */
	}
#endif /* NO_FLEXLM_CLIENT_API */
	if (ret && ret != FEATQUEUE && gotmaxusers)
	{

		/*
		 *		This is the significant one
		 */

		LM_SET_ERRNO(job, LM_MAXUSERS, 132, 0);
		ret = LM_MAXUSERS;
	}
	if (ret && job->lm_errno &&
		(job->options->flags & LM_OPTFLAG_DIAGS_ENABLED))
	{
		static char *	diag = (char *)-1;

		if (diag  == (char *)-1)
			diag = getenv("FLEXLM_DIAGNOSTICS");	/* overrun checked */
		if (diag)
		{
			char **cp;
			lc_perror(job, "FLEXlm checkout error");

			cp = job->lic_files;
			if (cp)
			{
				fprintf(stderr, "license file(s): ");
				while (*cp)
				{
					fprintf(stderr, " %s", *cp++);
				}
				fprintf(stderr, "\n");
			}
			if (*diag == '2')
			{
				fprintf(stderr, "lm_checkout(\"%s\", %s, %d, 0x%x, ..., 0x%x)\n",
					feature, version, nlic, flag, dup_group);
			}
		}
	}
	if (!ret && conf && strncmp( LM_LICENSE_START, job->lic_files[conf->lf], strlen( LM_LICENSE_START )) )
	{
		int		s_len = 0;
		char	ttemp_buf[LM_MAXPATHLEN + 1] = {'\0'};
		char *	slash_ptr = NULL, * end_ptr = NULL;

		l_zcp(ttemp_buf,job->lic_files[conf->lf], LM_MAXPATHLEN);
		s_len= strlen( ttemp_buf );
		if ( s_len > 4 )
		{
			end_ptr=&ttemp_buf[ s_len - 4 ];
			if (l_keyword_eq(job, end_ptr, ".lic" ))
			{
		 		/* now strip off the whole file name and terminator */
				slash_ptr=strrchr( ttemp_buf ,PATHTERMINATOR );
				if ( NULL == slash_ptr ) /* the path had no path, only the local directory */
				{
					if (!getcwd( ttemp_buf, 511 ))
						return ret;
				}
				else
					*slash_ptr=0;

			}


		}
		l_update_license_file(job,  ttemp_buf ,
				job->vendor);
	}
	return(ret);		/* Return NULL if any errors */
}

/*
 *	l_local_verify_conf
 *	Verify everything we can locally
 *	Return 1 == success, 0 == failure
 */

int
l_local_verify_conf(
	LM_HANDLE *		job,
	CONFIG *		conf,
	char *			feature,
	char *			version,
	VENDORCODE *	key,
	int				flag, /* LM_CO_xxx */
	int				filterflag)
{
	char **	cpp = NULL;

/*
 *	If the vendor has a checkout filter, try it now.
 */
	if (filterflag && job->options->outfilter_ex &&
		(*LM_CALLBACK_OUTFILTER_TYPE_EX job->options->outfilter_ex)(job, conf, job->options->pUserData))
	{
		if (job->lm_errno == 0)
			LM_SET_ERRNO(job, LM_LOCALFILTER, 125, 0);
		return 0;
	}

	if (filterflag && job->options->outfilter &&
		(*LM_CALLBACK_OUTFILTER_TYPE job->options->outfilter)(conf))
	{
		if (job->lm_errno == 0)
			LM_SET_ERRNO(job, LM_LOCALFILTER, 125, 0);
		return 0;
	}

/*
 *	0 users -> unlimited users on a single host.  > 0 can be tied
 *	to a single host (in which case the hostid is checked above,
 *	or allowed on any hosts, in which case the hostid is not checked.)
 */
	if (conf->conf_state == LM_CONF_REMOVED)
	{
		LM_SET_ERRNO(job, LM_BADFILE, 397, 0);
		return 0;
	}
	if (conf->users && (conf->conf_state == LM_CONF_COMPLETE))
	{
/*
 *		No server is only allowed for 1 host/unlimited use
 */
		if (!conf->server)
		{
			if (conf->lc_future_minor)
			{
				LM_SET_ERRNO(job, LM_FUTURE_FILE, conf->lc_future_minor, 0);
			}
			else
			{
				LM_SET_ERRNO(job, LM_NO_SERVER_IN_FILE, 128, 0);
			}
			return 0;
		}
/*
 *	    	re-encrypt the data, compare to the code in the file.
 */
#ifndef NO_ENCRYPTION_CHECK
		if (conf->package_mask & LM_LICENSE_PKG_COMPONENT)
		{
			if (!conf->parent_pkg ||
				!l_good_lic_key(job, conf->parent_pkg, key))
			{
				return 0;
			}
			if (!conf->parent_feat ||
				!l_good_lic_key(job, conf->parent_feat, key))
			{
				return 0;
			}
		}
		else if (!l_good_lic_key(job, conf, key))
		{
			return 0;
		}
#endif /* NO_ENCRYPTION_CHECK */
	}


/*
 *	We must be asking for a version that is <= the version in the
 *	configuration file.	(Moved to the nodelocked case for v2.5).
 *	(Moved back to the main-line for v2.61).
 */
	if (l_compare_version( job, version, conf->version) > 0)
	{
        char buf[(MAX_VER_LEN * 2) + 10] = {'\0'};

		sprintf(buf, "%s > %s", version, conf->version);
		LM_SET_ERROR(job, LM_OLDVER, 126, 0, buf, LM_ERRMASK_ALL);
		return 0;
	}
/*
 *	Make sure we haven't expired
 */
	if (l_date(job, conf->date, L_DATE_EXPIRED))
	{
		return 0;		/* EXPIRED */
	}

/*
 *	... and we have started ....
 */
	if (l_start_ok(job, l_extract_date(job, conf->code)))
	{
		return 0;	/* Not started yet */
	}
	if (*conf->startdate && l_date(job, conf->startdate, L_DATE_START_OK))
	{
		return 0;	/* Not started yet */
	}
/*
 *	If we are tied to a single hostid, check it here
 *	If LM_CONF_FLOAT_OK, then we checked the license out from the
 *	server, and don't need to check it here
 */
	if (conf->idptr && (conf->idptr->type != HOSTID_DEMO) &&
			!(conf->borrow_flags & LM_CONF_FLOAT_OK))
	{
        int x = job->options->normal_hostid;
/*
 *		If the override specifies no extended hostid checking,
 *		then we must temporarily turn off extended checking.
 */
		if (conf->idptr->override == NO_EXTENDED)
		{
			job->options->normal_hostid = 1;
		}
		if (!l_host(job, conf->idptr))
		{

			if (conf->lc_type_mask & LM_TYPE_FLOAT_OK)
			{
/*
 *				We're using a dongle-based, float-ok
 *				license from the dongle, not the server
 *				(otherwise borrow_flags would have the
 *				FLOAT_OK bit set).
 * 				we have to ensure we don't run on the
 *				same node as the license server.  Otherwise,
 *				they could get 2 licenses:
 *				(one floating and one node-locked runnin on the
 *				server.  make sure we're *not* on the
 *				conf->floatid node.
 *				If !conf->floatid, then we don't check for this
 */
				if ((conf->floatid) &&
					!l_host(job, conf->floatid))
				{
					LM_SET_ERROR(job, LM_NOTONSERVER, 575,
					  0, l_asc_hostid(job, conf->floatid),
					  LM_ERRMASK_ALL);
					job->options->normal_hostid = x;
					return 0;
				}
			}
		}
		else
		{
			job->options->normal_hostid = x;
			return 0;
		}
		job->options->normal_hostid = x;
	}


/*
 *	verify platform
 */
	if (conf->lc_platforms)
	{
        char plat[MAX_PLATFORM_NAME + 1];
        int bad = 1;
        char *cp;

		if (*job->options->platform_override)
			l_zcp(plat,
				job->options->platform_override,
					MAX_PLATFORM_NAME);
		else
			l_zcp(plat, l_platform_name(),
						MAX_PLATFORM_NAME);
#ifdef SGI
		l_lowercase(plat);
#endif
		for (cp = plat; *cp; cp++)
		{
			if (*cp == '_')
			{
				for (cp++; *cp && isalpha(*cp); cp++)
					;
				*cp = 0;
				break;
			}
		}
		for (cpp = conf->lc_platforms; *cpp; cpp++)
		{
#ifdef SGI
			l_lowercase(*cpp);
#ifdef SGI64
			if (L_STREQ(*cpp, "sgir8_u") ||
				L_STREQ(*cpp, "sgi64_u"))
#else /* now 32-bit sgi */
			if (L_STREQ(*cpp, "sgi_u") ||
				L_STREQ(*cpp, "sgi32_u"))
#endif /* now all sgi */
			{
				bad = 0;
				break;
			}
#endif /* SGI */
			if (l_keyword_eq(job, *cpp, plat))
			{
				bad = 0;
				break;
			}
		}
		if (bad)
		{
            int		cnt = 0;
            char *	buf = NULL;

            for (cnt = 0, cpp = conf->lc_platforms; *cpp; cpp++, cnt++)
				;
			buf = l_malloc(job, (MAX_PLATFORM_NAME + 1) * cnt);

			sprintf(buf, "%s <> ", plat);
			for (cpp = conf->lc_platforms; *cpp; cpp++)
			{
				strcat(buf, *cpp);
				if (cpp[1])
                    strcat(buf, ", ");
			}
			LM_SET_ERROR(job, LM_PLATNOTLIC, 310, 0, buf, LM_ERRMASK_ALL);
			free(buf);
			return 0;
		}
	}
#ifndef VMS
    {
        int		day = 0, year = 0;
        char	month[10] = {'\0'};

        sscanf(conf->date, "%d-%[^-]-%d", &day, month, &year);	/* overrun threat */
        if (year && l_baddate(job))
            return 0;
    }
#endif /* VMS */

	/* Terminal Server client checking */
#if defined(PC)

	/*
		****** REMOVE THIS LATER ********
		code changed for a temporary fix until we can figure out how to use this
		call for NT 4. isTSOK() will always return 0. This is temporary l_ts.c,
		No changes in this code. only in l_ts.c
		****** REMOVE THIS LATER ********
	*/
	if ( (conf->users > 0) || (conf->lc_type_mask & LM_TYPE_TS_OK) )
      		;		/* do nothing, let the user go through */
    else if (isTSOK())	/* remote client, TS_OK not set */
    {
        LM_SET_ERRNO(job, LM_TSOK_ERR, 577, 0);
        /* err = LM_TSOK_ERR; */
        return 0;
    }
#endif

	/* is there a checkout filter to be called at the end of the local verify */
	if (filterflag && job->options->outfilter_last_ex &&
		(*LM_CALLBACK_OUTFILTER_TYPE_EX job->options->outfilter_last_ex)(job, conf, job->options->pUserData))
	{
		if (job->lm_errno == 0)
			LM_SET_ERRNO(job, LM_LOCALFILTER, 125, 0);
		return 0;
	}

	return 1; /* passed all tests */
}

#ifdef ENCRYPT

static lm_dummy() { }		/* Just so we know where lm_real_start ends */
static
int
lm_decrypt_code(
	LM_HANDLE *	job,		/* Current license job */
	int *		from,
	int *		to)
{
	while (from < to)
	{
		*from ^= job->options->decrypt_flag;
		from++;
	}
}

#endif

#if !defined(NO_ENCRYPTION_CHECK)

CONFIG *
API_ENTRY
lc_test_conf(LM_HANDLE * job)
{
	return(testconf);
}
#endif /* NO_ENCRYPTION_CHECK */

/*
 *	return 1 if good, 0 if bad
 */
#ifndef NO_ENCRYPTION_CHECK
int
l_good_lic_key(
	LM_HANDLE *		job,
	CONFIG *		conf,
	VENDORCODE *	key)
{
	VENDORCODE	vc;
	int			ok = 0;
	char *		code = NULL;
	int			str_res = 0;
	char *		sdate = NULL;
	L_KEY_FILTER *kf = 0;

	if ((!job->flags & LM_FLAG_SLOW_VERIFY)
		&& (conf->L_CONF_FLAGS & L_CONF_FL_VERIFIED))
	{
		return 1;
	}
	memcpy(&vc, key, sizeof(vc));
	if (! (job->flags & LM_FLAG_CLEAR_VKEYS))
		l_xorname(job->vendor, &vc);
	l_sg(job, job->vendor, &vc);
#if 1
	if ((job->L_SIGN_LEVEL) && !conf->lc_keylist)
	{
        char context[50];
        char num[2];
        num[1] = 0; /* null terminator */
#ifdef MONTAVISTA
        num[0] = (char)((char)job->L_SIGN_LEVEL + '0');
#else
        num[0] = (char)((char)job->L_SIGN_LEVEL + '0');
#endif /* MONTAVISTA */

        sprintf(context, "SIGN%s=", num);
		LM_SET_ERROR(job, LM_SIGN_REQ, 582, 0, context, LM_ERRMASK_ALL);
		goto  exit_good_lic_key;
	}
#endif
	if (conf->lc_keylist && job->L_SIGN_LEVEL)
	{
        LM_KEYLIST *kl;

		for (kf = (L_KEY_FILTER *)job->key_filters;
			kf && (kf->sign_level != (int)job->L_SIGN_LEVEL);
			kf = kf->next)
			;
/*
 *		Find the last KEY_FILTER for this job
 */
		if (kf)
		{
            int foundkl = 0;

            for (kl = conf->lc_keylist; kl; kl = kl->next)
			{
                char *	stdate = NULL;

				if ((kf->sign_level != kl->sign_level))
					continue;
				foundkl = 1;
				job->lc_this_keylist = kl;

				code = l_crypt_private(job, conf, stdate, &vc);
				job->lc_this_keylist = 0;
				if (code && *code)
				{
					ok = 1;
					break;
				}
			}
			if (!ok)
			{
				if (!kl && !foundkl)
				{
					char context[50] = {'\0'};
					char num[2] = {'\0'};

                    num[1] = 0; /* null terminator */
					num[0] = kf->sign_level > 1 ?
						kf->sign_level + '0': 0;
					sprintf(context, "SIGN%s=", num);
					LM_SET_ERROR(job, LM_SIGN_REQ, 526, 0,
						context, LM_ERRMASK_ALL);

				}
				else
				{
					LM_SET_ERRNO(job, LM_BADCODE, 523, 0);
				}
			}
		}
#if 0
		else
			ok = 1;
#endif
	}
	if (!kf)
	{
		if (!(conf->lc_keylist && job->L_SIGN_LEVEL))
		{
			job->flags |= LM_FLAG_MAKE_OLD_KEY;
		}
		sdate = l_extract_date(job, conf->code);
		code = l_crypt_private(job, conf, sdate, &vc);
		if (!(conf->lc_keylist && job->L_SIGN_LEVEL))
		{
			job->flags &= ~LM_FLAG_MAKE_OLD_KEY;
		}
		if (job->user_crypt_filter)
		{
			if (!code || !*code)
				str_res = 1;
		}
		else
		{
			if (conf->lc_keylist && job->L_SIGN_LEVEL)
			{
				if (!code || !*code || !*conf->code)  /*P5552 */
					str_res = 1;
				else
					STRNCMP(code, conf->lc_sign, MAX_CRYPT_LEN,
					str_res);
			}
			else
			{
				if (!code || !*code || !*conf->code)  /*P5552 */
					str_res = 1;
				else
					STRNCMP(code, conf->code, MAX_CRYPT_LEN,
								str_res);
			}
		}
		if (str_res)
		{
	/*
	 *	If user has set alternate encryption seeds,
	 *	try those also.
	 */
			if (job->options->alt_vendorcode.data[0] ||
				job->options->alt_vendorcode.data[1])
			{
				VENDORCODE altvc;
				VENDORCODE *v = &job->options->alt_vendorcode;
				memcpy((char *)&altvc, (char *)v, sizeof(altvc));
				l_sg(job, job->vendor, &altvc);
				code = l_crypt_private(job, conf, sdate, &altvc);
				ok = 1; /* assume success */
				if (!code || !*code || !*conf->code) /*P5552*/
					ok = 0;
				else
				{
					STRNCMP(code, conf->code, MAX_CRYPT_LEN, str_res);
					if (str_res)
						ok = 0; /* failed */
				}
			}

			if (!ok)
			{
				if (l_keyword_eq(job, job->vendor, conf->daemon))
				{
					LM_SET_ERRNO(job, LM_BADCODE, 130, 0);
				}
			}
		}
		else
			ok = 1;
	}

exit_good_lic_key:

	if (!ok && conf->lc_future_minor)
		LM_SET_ERRNO(job, LM_FUTURE_FILE, conf->lc_future_minor, 0);


	if (ok)
        conf->L_CONF_FLAGS |= L_CONF_FL_VERIFIED;
	else
        conf->L_CONF_FLAGS |= L_CONF_FL_REJECT;
	return ok;
}
#endif /* NO_ENCRYPTION_CHECK */
#ifndef NO_FLEXLM_CLIENT_API
/*
 *	l_send_hostid -- to support port@host-plus
 */
static
int
l_send_hostid(
	LM_HANDLE *	job,
	int			hostid_type,
	int			offset)
{
	char		msg[LM_MSG_LEN+1] = {'\0'};
	HOSTID *	idptr = NULL, hostid, * p = NULL;
	int			nomore = 0;


	memset(msg, '\0', LM_MSG_LEN+1);
	l_encode_int(&msg[MSG_HOSTID_TYPE], hostid_type);
	idptr = l_getid_type(job, hostid_type);
	for (p = idptr; p && offset; p = p->next)
	{
		offset--;
	}
	idptr = p;
/*
 *	If requested type is unavailable on this system, send
 *	message with hostid = 0
 */
	if (!idptr || !l_id_types_match(idptr->type, hostid_type))
	{
		nomore = 1;
	}
	else
	{
/*
 *		l_asc_hostid has to print exactly ONE hostid, not all,
 *		so we put this in a struct, with NULL next
 */
		memcpy(&hostid, idptr, sizeof(hostid));
	}
	hostid.next = 0;
	if (nomore)
		l_zcp(&msg[MSG_HOSTID], "NOMORE", MAX_HOSTID_LEN);
	else
		l_zcp(&msg[MSG_HOSTID], l_asc_hostid(job, &hostid),
					MAX_HOSTID_LEN);
	l_sndmsg(job, LM_HOSTID, &msg[MSG_DATA]);
	return 1;
}
/*
 *	checkout_from_server --
 *	returns:	1 -- skip to next conf
 *			2 -- continue
 *			0 -- all done with this conf, either error or success
 *			< 0 -- all done, and return this error
 */

static
int
checkout_from_server(
	LM_HANDLE *		job,
	int				flag,
	CONFIG **		confp, /* it's a pointer, because it may get reset if QUEUE */
	int				nlic,
	char *			feature,
	int *			gotmaxusers,
	VENDORCODE *	key,
	char *			version,
	int				dup_group,
	int *			resetsocket,
	int *			oldflags,
	char *			last_server)
{


	CONFIG *	conf = *confp;
	char *		param;	/* Returns from l_rcvmsg() */
	int			usefile = 0;
	char		type;
	char		msgparam[LM_MSG_LEN+1] = {'\0'};
	HOSTID *	h = NULL;
#ifdef PC
	char		display_buf[MAX_HOSTNAME + 5] = {'\0'};
#endif
	int			was_connected = (job->daemon->socket != LM_BAD_SOCKET);


	if (conf && conf->server && !L_STREQ(last_server, conf->server->name) &&
			(job->lm_errno == LM_HOSTDOWN))
	{
		/* clear it */
		l_clear_error(job);
	}

reconnect:
/*
 *	Because of Netware, this hostid call MUST precede the l_connect()
 */

	h = l_gethostid(job);

#ifdef PC
#ifdef PC16
	{
        WSADATA wsadata;

        if( WSAStartup( (WORD)WINSOCK_VERSION, &wsadata ))
		{
			LM_SET_ERRNO(job, LM_NONETWORK, 180, WSASYSNOTREADY);
			return (job->lm_errno);
		}
	}
#endif /* PC16 */
	*display_buf = 0;
	gethostname(display_buf, MAX_HOSTNAME);
	if (!job->options->display_override)
	{
		l_zcp(job->options->display_override, display_buf, MAX_DISPLAY_NAME - 1);
	}
#endif /* PC */


/*
 *	Now, find out if we are connected to a server yet
 *	(or even if we need to!)
 */
	if (conf->server &&  (conf->server->idptr &&
		conf->server->idptr->type == HOSTID_ANY) &&
		( conf->server->commtype == LM_FILE_COMM))
	{
/*
 *	Set up so no one will try any communications
 */
		usefile = 1;
		job->daemon->commtype = LM_FILE_COMM;
		job->daemon->socket = LM_BAD_SOCKET;
		job->daemon->server = conf->server;
	}

	else if (job->daemon->socket == LM_BAD_SOCKET ||
		conf->server != job->daemon->server)
	{
        int was = job->err_info.min_errno;

		usefile = 0;
		if (l_connect(job, conf->server, conf->daemon,
						job->options->commtype) < 0)
		{
			return 1;
		}
/*
 *    		Set up the encryption for later
 */
		if (job->daemon->socket !=
				LM_BAD_SOCKET &&
			job->daemon->encryption == 0)
		{
			l_handshake(job);
			if (!job->options-> no_traffic_encrypt
						&& !job->daemon->encryption)
			{
				job->flags |= LM_FLAG_FEAT_FOUND;
				return 1; /* continue */
			}
		}
		else if (was == job->err_info.min_errno)
/*
 *		If l_connect didn't set lm_errno, do it here
 */
		{
			LM_SET_ERROR(job, LM_CANTCONNECT, 138, 0, conf->server->name, LM_ERRMASK_ALL);
		}
	}
	if (!usefile && (job->daemon->socket == LM_BAD_SOCKET))
	{
		/* Can't connect */
		return 1; /* continue */
	}

/*-
 *	Reject v5.0c and v5.1 and v5.11 because of P2378, if
 *	port@host +
 *	Now, reject v6.0 - v7.2b
 */
	if (!(job->flags & LM_FLAG_LMDIAG) &&
		((
		(job->daemon->ver == 7 && job->daemon->rev <= 1) ||
		(job->daemon->ver == 7 && job->daemon->rev == 2  &&
			job->daemon->patch <= 'b') ||
		(job->daemon->ver == 6 ) )

		||

		(job->L_SIGN_LEVEL && (job->daemon->ver < 7))))

	{
		lc_disconn(job, 0);
		if (job->L_SIGN_LEVEL)
		{
			LM_SET_ERRNO(job, LM_SERVOLDVER, 527, 0);
		}
		else
		{
			LM_SET_ERRNO(job, LM_SERVOLDVER, 318, 0);
		}
		return 1;
	}

	job->daemon->usecount++;
/*
 *	Remember this stuff, so we don't have to talk to the
 *	server for multiple feature lines in one file if it
 *	would be fruitless.
 */
/*
 *	Send "checkout" message to server for this feature.
 */
	memset(msgparam, '\0', LM_MSG_LEN+1);
	l_encode_int(&msgparam[MSG_CO_N], nlic);
	if (strlen(&msgparam[MSG_CO_N]) > (size_t)4)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 139, 0);
		/*
		 *	will return with this error
		 */
		return 1; /* continue */
	}
	/* Truncate it */
	msgparam[MSG_CO_N-1] = '\0';
	strncpy(&msgparam[MSG_CO_VERSION], version, MAX_VER_LEN);
#ifdef NO_ENCRYPTION_CHECK /* lmdiag */
/*
*		lmdiag sets the TEST flag, so the server won't log,
*		or really checkout
*/
	msgparam[MSG_CO_WAIT] = MSG_CO_WAIT_LOCALTEST;
#else /* regular client */

	if (flag == LM_CO_WAIT || flag == LM_CO_QUEUE)
		msgparam[MSG_CO_WAIT] = MSG_CO_WAIT_QUEUE;
	else if (flag == LM_CO_LOCALTEST)
		msgparam[MSG_CO_WAIT] = MSG_CO_WAIT_LOCALTEST;
	else
		msgparam[MSG_CO_WAIT] = MSG_CO_WAIT_NO_WAIT;

#endif  /* NO_ENCRYPTION_CHECK -- lmdiag */

	strncpy(&msgparam[MSG_CO_FEATURE], feature,
						MAX_FEATURE_LEN);
	l_encode_int(&msgparam[MSG_CO_DUP_SEL], dup_group);
	l_encode_int(&msgparam[MSG_CO_LINGER], (int) job->options->linger_interval);
	l_zcp(&msgparam[MSG_CO_CODE], conf->code, MAX_CRYPT_LEN);
	if (conf->conf_state != LM_CONF_COMPLETE)
	{
		msgparam[MSG_CO_WAIT] |= MSG_CO_WAIT_FLAG_PORT_HOST_PLUS;
	}

	strncpy( &msgparam[MSG_CO_VENDOR_DEF],
				job->options->vendor_checkout_data,
					MAX_VENDOR_CHECKOUT_DATA);
	if (h)
	{
        char *	str = NULL;

        str = l_asc_hostid_len(job, h, 1);
		if (strlen(str) <= (size_t)MAX_SHORTHOSTID_LEN)
		{
			strncpy(&msgparam[MSG_CO_HOSTID], str,
						MAX_SHORTHOSTID_LEN);
		}
	}
	l_encode_16bit_packed(&msgparam[MSG_CO_SERNUM], job->last_ckout_sernum);
	if (job->borrow_linger_minutes)
	{
		if (!(job->options->flags & LM_OPTFLAG_BORROW_OK) )
		{
			LM_SET_ERRNO(job, LM_NOBORROWSUPP, 576, 0);
			return LM_NOBORROWSUPP;
		}
/*
 *		we're going to borrow from the server
 *		In this part, we send them the linger time, and flag to
 *		tell them it's a borrow-linger.
 */
		l_encode_int(&msgparam[MSG_CO_LINGER],
					job->borrow_linger_minutes * 60);
		msgparam[MSG_CO_FLAGS2] |= MSG_CO_FLAGS2_BORROW;
	}
	if (job->L_SIGN_LEVEL)
	{
#if 0
	  L_KEY_FILTER *kf = (L_KEY_FILTER *)job->key_filters;

		if (kf && kf->app_filter)
			msgparam[MSG_CO_FLAGS] |= (kf->sign_level << 1);
#endif
			msgparam[MSG_CO_FLAGS] |= ((int)(job->L_SIGN_LEVEL) << 1);
	}
	if (job->l_new_job)
		msgparam[MSG_CO_FLAGS] |= MSG_CO_HS_CB;

	if(flag & LM_FLAG_LOG_UD_ON_FAILURE)
	{
		msgparam[MSG_CO_ULTIMATE_DENIAL_ON_FAILURE] = 1;
	}
resend:
	if (!l_sndmsg(job, LM_CHECKOUT, &msgparam[MSG_DATA]))
	{
		lc_disconn(job, 1);
		if (was_connected)
		{
			was_connected = 0;
			goto reconnect;
		}
		return 1; /* continue */
	}
/*
*	Read status from server, and return the status
*/
reread:
	if (!l_rcvmsg_timeout(job, &type, &param,
					(flag == LM_CO_WAIT) ?
					L_RCVMSG_BLOCK :
					L_RCVMSG_DEFAULT_TIMEOUT))
	{
		lc_disconn(job, 1);
		if (was_connected)
		{
			was_connected = 0;
			goto reconnect;
		}
		return 1; /* continue */
	}
	job->flags |= LM_FLAG_FEAT_FOUND;
/*
 *	Now, type and param are filled in, either from the message
 *	from the server, or from l_file_checkout().
 */
	switch(type)
	{
	case LM_NUSERS:
		lc_disconn(job, 0);
		LM_SET_ERRNO(job, LM_MAXUSERS, 140, 0);
		(*gotmaxusers) = LM_MAXUSERS;
		break;
	case LM_NO_SUCH_FEATURE:
		lc_disconn(job, 0);
		LM_SET_ERRNO(job, LM_NOSERVSUPP, 141, 0);
		break;
	case LM_FEATURE_AVAILABLE:
		if (flag == LM_CO_WAIT && l_keyword_eq(job, feature, param))
		{
/*
 *			This one is for us
 */
#ifndef NO_FLEXLM_CLIENT_API
			l_check_conf(job, feature, version, nlic, key, conf,
								dup_group, 0);
#endif /* NO_FLEXLM_CLIENT_API */
			break; /* break -- all done */
		}
		else
		{
			/*
			 *	sluu: 6/17/2003 - P7315
			 *	This is a HACK to get around the fact that when queuing on
			 *	a PACKAGE/SUITE.  In this scenario, the user is queuing on
			 *	the PACKAGE and since the server sends that PACKAGE name
			 *	in the LM_FEATURE_AVAILABLE message FLEXlm client get's confused
			 *	here and doesn't realize that it's really for it because it only
			 *	knows about the feature.  To get around	this, we check to see if
			 *	the name of the feature available is really the PACKAGE for the
			 *	feature we're interested in.  If it	is, we've got the license......
			 */
			if(l_IsComponentOfPackage(job, param, feature, conf->code) == 0)
			{
#ifndef NO_FLEXLM_CLIENT_API
				l_check_conf(job, feature, version, nlic, key, conf, dup_group, 0);
#endif
				break;
			}
			else
			{
				l_upgrade_feat(job, param);
				goto reread;
			}
		}
	case LM_OK:

		l_ckout_ok(job, feature, version, nlic, key, confp,
						dup_group, param, flag);
		conf = *confp;
		if (job->lm_errno == LM_CANTMALLOC)
			return 0;
		break;

	case LM_BUSY:
		lc_disconn(job, 0);
		LM_SET_ERRNO(job, LM_SERVBUSY, 144, 0);
		break;

	case LM_BUSY_NEW:
		lc_disconn(job, 0);
		LM_SET_ERRNO(job, LM_BUSYNEWSERV, 145, 0);
		break;

	case LM_QUEUED:
		if (flag == LM_CO_WAIT && !(*resetsocket))
		{

/*
 *			Set socket to blocking
 */
#ifdef PC
			u_long zero = 0;

			network_control(job->daemon->socket, FIONBIO,
								&zero);
#else
			*oldflags = network_control(job->daemon->socket,
								F_GETFL, 0);
			network_control(job->daemon->socket, F_SETFL,
						(~FNDELAY) & (*oldflags));
#endif
					*resetsocket = 1;
		}
		/*else*/
		{
			CONFIG	ctmp, * c = NULL, * last = NULL;
			memcpy(&ctmp, conf, sizeof(ctmp));
			strcpy(ctmp.feature, feature);
			strcpy(ctmp.code, param);
			if (c = l_get_conf_from_server(job, &ctmp))
			{
				conf = *confp = c; /* reset ckout config ptr */
				/* append to job */
				for (c = job->line; c; c = c->next)
				{
					last = c;
				}
				last->next = conf;
			}

#ifndef NO_FLEXLM_CLIENT_API
			l_check_conf(job, feature, version, nlic, key, conf,
							dup_group, FEATQUEUE);
#endif
			if (flag != LM_CO_WAIT)
			{
				LM_SET_ERRNO(job, LM_FEATQUEUE, 146, 0);
			}
			else
				goto reread;
		}
		break;
#ifndef NO_FLEXLM_CLIENT_API
	case LM_NEED_HOSTID:
		{
            int hostid_type = 0, err = 0, offset = 0;

			sscanf(param, "%d %d", &hostid_type, &offset);	/* overrun threat */
			if (l_send_hostid(job, hostid_type, offset))
			{
				if (!l_rcvmsg(job, &type, &param))
				{
					lc_disconn(job, 0);
					return 0; /* break */
				}
				if (type != LM_OK)
				{
					lc_disconn(job, 0);
					l_decode_int(param, &err);
					LM_SET_ERRNO(job, err, 243, 0);
				}
			}
			if(!job->idptr)
				l_gethostid(job);
		}
		if (job->idptr)
		{
			msgparam[MSG_CO_HOSTID] = '\0';
goto resend;
		}
		else
		{
			LM_SET_ERRNO(job, LM_NOTTHISHOST, 409, 0);
		}

		break;
#endif /* NO_FLEXLM_CLIENT_API */
	default:
#ifndef NO_FLEXLM_CLIENT_API
		lc_disconn(job, 0);
#endif /* NO_FLEXLM_CLIENT_API */
		{
			int i = 0;
			l_decode_int(param, &i);
			if (i != LM_POOL || !job->err_info.maj_errno)
			{
				LM_SET_ERROR(job, i, 147, 0, 0, LM_ERRMASK_ALL);
			}
		}
		if (job->lm_errno == MAXUSERS || job->lm_errno == USERSQUEUED)
		{
			(*gotmaxusers) = job->lm_errno;
		}
		/*
		 *	Check to see if the server has the report log enabled.
		 */
		if(job->savemsg[MSG_REPORT_LOG_ENABLED])
		{
			conf->server->sflags |= L_SFLAG_REPORT_LOG_ENABLED;
		}
		break;
	}

	if ((job->lm_errno == 0) || (job->lm_errno == FEATQUEUE))
	{
		if ((flag == LM_CO_LOCALTEST ) && conf->type != CONFIG_PORT_HOST_PLUS)
		{
			testconf = conf;
		}

		return job->lm_errno; /* test no more confs */
	}

	return 1; /* test next conf */
}

#endif /* NO_FLEXLM_CLIENT_API */

/***********************************************************************
 * Re-verify the license with the server. Called after a checkout from the server.
 * Add a new config to the config list in the job.
 *
 * Parameters:
 * 		job			- The job
 * 		feature		- The feature desired.
 * 		version		- Version desired
 * 		nlic		-
 * 		key			-
 * 		confp		- On return set to the CONFIG point we either used or created
 * 		dup_group	-
 * 		param		- The feature line used for this checkout. I think this
 * 					  comes from the server.
 * 		flag		- If LM_CO_LOCALTEST
 ************************************************************************/
void
l_ckout_ok(
	LM_HANDLE *		job,
	char *			feature,
	char *			version,
	int				nlic,
	VENDORCODE *	key,
	CONFIG **		confp,
	int				dup_group,
	char *			param,
	int				flag)
{
	/*
	 *		Remember our license file
	 */
	int			did_malloc = 0;
	CONFIG *	conf = *confp;
	int			ok = 1;

	l_clear_error(job); /* ok -- that's why we're here */
	job->lfptr = conf->lf;
#ifndef NO_ENCRYPTION_CHECK
	if (!strcmp(conf->code, CONFIG_PORT_HOST_PLUS_CODE) ||
		(conf->conf_state != LM_CONF_COMPLETE))
	{
   		CONFIG *	c = NULL;
        char *		cp = param + 2;
        LM_SERVER * s = conf->server;

		/* kmaclean 3/7//03
		 * Changed this a lot to prevent duplicates from being placed
		 * in the config list.
		 *
		 * Also this may fix problem with crashing on a config we try to
		 * delete twice */
		c = (CONFIG *) l_malloc(job, sizeof (CONFIG));
		did_malloc = 1;
		l_parse_feature_line(job, cp, c, (char **)0);
		if (conf->conf_state & LM_CONF_COMPLETE)
			l_free_conf(job, c);
		else if (! (conf->conf_state & LM_CONF_COMPLETE))
		{
			/* the conf we used to checkout this license does not have
			 * everything we need. Use the conf that got sent back with the
			 * checkout reply. Re-verify it with the server */
			if (ok = verify_server_key(job, &c, key, version, s))
			{
				if ( conf->type == CONFIG_PORT_HOST_PLUS )
				{
					CONFIG *tmpConf;
					if ( (tmpConf = is_confg_in_list(job,c)) != NULL )
					{
						/* the conf we got from the server is already in the
						 * list so we need to overwrite it with this one. */
						conf = tmpConf;
						copy_conf_data(job,conf,c);
						l_free_conf_no_data(job, c);
						*confp = conf;
					}
					else
					{
						/* put the conf for this feature right before the
						 * port at host that got us here */
						c->next = conf->next;
						c->last = conf;
						c->lf = conf->lf;	/* Fixes buf P6130 */
						if (conf->next)
							conf->next->last = c;
						conf->next = c;
						conf = *confp = c;
						conf->conf_featdata = 0;	/* do not free this conf when the feature is freeed */
					}
				}
				else
				{
					/* kmaclean 2/11/03
					 * need to copy the new conf data into the old
					 * incomplete conf. Can't delete and replace the conf
					 * because something may already be pointing to it. */

					/* save the state of the flag that tells us if we should
					 * free this conf when the FEATDATA is freeed */
					int tmp_conf_featdata = conf->conf_featdata ;

					copy_conf_data(job,conf,c);
					conf->conf_featdata = tmp_conf_featdata;
					l_free_conf_no_data(job, c);
					*confp = conf;
				}
			}
			else
			{
				LM_SET_ERRNO(job, LM_BADCODE, 544, 0);
				l_free_conf_no_data(job, c);
				return;
			}
		}
	}
#endif /* NO_ENCRYPTION_CHECK*/
	/*
	 *		Turn on checking, to detect server death
	 */
#ifndef NO_FLEXLM_CLIENT_API
	if (flag == LM_CO_LOCALTEST)
	{
		/*if (testconf) l_free_conf(job, testconf); P2534 */
		testconf = conf;
		return;
	}
	else
	{
		if (!ok || !l_check_conf(job, feature, version, nlic, key, conf,
					dup_group, 0) && did_malloc)
		{
			return;
		}
	}
	if (job->borrow_linger_minutes &&
		!(conf->borrow_flags & LM_CONF_BORROWED))
	{
		/*
		 *		There server is already lingering, create the
		 *		local borrow file now
		 */
		l_borrow(job, feature, conf);
	}

#endif /* NO_FLEXLM_CLIENT_API */
}

#define SIGSIZE 4               /*- Change sig[] initialization if changed */

#ifndef NO_ENCRYPTION_CHECK
/*-
 *	Also used by flexcrypt -- notify if API changes.
 */
void
l_sg(
	LM_HANDLE *		job,
	char *			vendor_id,
	VENDORCODE *	key) /*- l_sg means "signature vendor_key5" */
{
	unsigned long	keys[4];
	char			sig[SIGSIZE] = {'\0'};
	/*- If you change this, you must change it also in utils/lmnewgen.c */
	/*- unsigned long x = 0xa8f38730;                   v3.1 */
	/*- unsigned long x = 0x7648b98e;                   v7.0 */
	unsigned long	x = 0x6f7330b8;                   /*- v8.x */
	extern void		(*L_UNIQ_KEY5_FUNC)();
	unsigned long	d0 = 0, d1 = 0;
	int				i = SIGSIZE-1;

	if (( job->options->flags & LM_OPTFLAG_CUSTOM_KEY5) && L_UNIQ_KEY5_FUNC)
	{
		(*L_UNIQ_KEY5_FUNC)(job, vendor_id, key);
		return;
	}
/*-
 *	Pre v6.1 style
 *      First, verify the key
 */
	/*keys =*/ l_key(vendor_id, &(key->keys[0]), keys, 4);
/*-                                     JUNK only in top 16 bits as of v2.2 */
/*-                                     JUNK XORed with date as of v2.4 */


	sig[0] = sig[1] = sig[2] = sig[3] = '\0';

	while (*vendor_id)
	{
		sig[i] ^= *vendor_id++;
		i--;
		if (i < 0)
            i = SIGSIZE-1;
	}
	d0 = key->data[0] ^ ((((long)sig[0] |
		    ((long)sig[1] << 8) |
		    ((long)sig[2] << 16) |
		    ((long)sig[3] << 24))
		    ^ x
		    ^ keys[1]
		    ^ keys[2]) & 0xffffffff);
	d1 = key->data[1] ^ ((((long)sig[0] |
		    ((long)sig[1] << 8) |
		    ((long)sig[2] << 16) |
		    ((long)sig[3] << 24))
		    ^ x
		    ^ keys[1]
		    ^ keys[2]) & 0xffffffff);
	if (d0 == key->data[0] )
        d0 = key->data[0] ^ x;
	if (d1 == key->data[1] )
        d1 = key->data[1] ^ x;

	key->data[0] = d0;
	key->data[1] = d1;

}

/*- This is left here mostly to obfuscate -- we don't use it */
unsigned long
l_svk(
	char *			vendor_id,
	VENDORCODE *	key) /*- l_svk means "signature vendor_key5" */
{
	unsigned long	keys[4] = { 0, 0, 0, 0};
	unsigned long	signature = 0;
	char			sig[SIGSIZE] = {'\0'};
	/*- unsigned long x = 0xa8f38730;                   v3.1 */
	/*-unsigned long x = 0x7648b98e;                    v7.0 */
	unsigned long	x = 0x6f7330b8;                   /*- v8.x */
	int				i = SIGSIZE-1;
/*-
 *      First, verify the key
 */
	l_key(vendor_id, &(key->keys[0]), keys, 4);
/*-                                     JUNK only in top 16 bits as of v2.2 */
/*-                                     JUNK XORed with date as of v2.4 */


	sig[0] = sig[1] = sig[2] = sig[3] = '\0';

	while (*vendor_id)
	{
		sig[i] ^= *vendor_id++;
		i--;
		if (i < 0)
            i = SIGSIZE-1;
	}
	signature = (long)sig[0] |
		    ((long)sig[1] << 8) |
		    ((long)sig[2] << 16) |
		    ((long)sig[3] << 24);
	signature ^= x;
	signature ^= keys[1];
	signature ^= keys[2];
	signature &= 0xffffffff;
	if (signature == 0)
        signature = x;      /* 0 invalid */
	return(signature);
}

int
API_ENTRY
lc_check_key(
	LM_HANDLE *				job,
	CONFIG *				conf,
	const VENDORCODE_PTR	key)	/* The vendor's key */
{

	if (LM_API_ERR_CATCH)
        return job->lm_errno;

	if (conf->conf_state != LM_CONF_COMPLETE)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 388, 0);
		LM_API_RETURN(int, LM_BADPARAM);
	}
	if (conf->package_mask & LM_LICENSE_PKG_COMPONENT)
	{
		if (!conf->parent_pkg || !conf->parent_feat)
		{
			LM_SET_ERRNO(job, LM_NOFEATURE, 513, 0);
			LM_API_RETURN(int, LM_NOFEATURE);
		}
		if ( !l_good_lic_key(job, conf->parent_pkg, key))
		{
			if (!conf->parent_pkg)
				LM_API_RETURN(int, job->lm_errno);
		}
		if ( !l_good_lic_key(job, conf->parent_feat, key))
		{
			LM_API_RETURN(int, job->lm_errno);
		}
	}
	else if (!l_good_lic_key(job, conf, key))
	{
		LM_API_RETURN(int, job->lm_errno);
	}
	LM_API_RETURN(int, 0);
}

/****************************************************************************/
/**	@brief	Utility for copying strings
 *
 *	@param	job			FLEXlm job handle.
 *	@param	pszInput	Input string to copy
 *	@param	ppszOutput	Pointer to string to receive string, if non NULL, will
 *						be freed.
 *
 *	@return	Pointer to string that was allocated.
 ****************************************************************************/
static
char *
sCopyString(
	LM_HANDLE *	job,
	char *		pszInput,
	char **		ppszOutput)
{
	char *	rv = NULL;
	if( (pszInput == NULL) || (ppszOutput == NULL) )
		goto done;

	if(*ppszOutput != NULL)
	{
		l_free(*ppszOutput);
		*ppszOutput = NULL;
	}
	rv = *ppszOutput = (char *)l_malloc(job, strlen(pszInput) + 1);
	if(*ppszOutput)
	{
		strcpy(*ppszOutput, pszInput);
	}

done:

	return rv;
}

/****************************************************************************/
/**	@brief	Copies fields from a PACKAGE to a COMPONENT
 *
 *	@param	job			FLEXlm job handle.
 *	@param	pParent		Parent whose data is to be copied from
 *	@param	pComponennt	Component that is target for new data
 *
 *	@return	None
 ****************************************************************************/
void
l_CopyPackageInfoToComponent(
	LM_HANDLE *	job,
	CONFIG *	pParent,
	CONFIG *	pComponent)
{
	if( (pParent == NULL) || (pComponent == NULL) )
		goto done;

	/*
	 *	Save type mask, this fixes bug 7266
	 */
	pComponent->lc_type_mask = pParent->lc_type_mask;
	pComponent->lc_max_borrow_hours = pParent->lc_max_borrow_hours;

	if(pParent->lc_vendor_def)		/* Vendor string */
	{
		(void)sCopyString(job, pParent->lc_vendor_def, &(pComponent->lc_vendor_def));
	}
	if(pParent->lc_vendor_info)		/* (Unencrypted) vendor info */
	{
		(void)sCopyString(job, pParent->lc_vendor_info, &(pComponent->lc_vendor_info));
	}
	if(pParent->lc_dist_info)		/* (not authenticated)*/
	{
		(void)sCopyString(job, pParent->lc_dist_info, &(pComponent->lc_dist_info));
	}
	if(pParent->lc_user_info)		/* (not authenticated)*/
	{
		(void)sCopyString(job, pParent->lc_user_info, &(pComponent->lc_user_info));
	}
	if(pParent->lc_asset_info)		/* (not authenticated)*/
	{
		(void)sCopyString(job, pParent->lc_asset_info, &(pComponent->lc_asset_info));
	}
	if(pParent->lc_issuer)			/* Who issued the license */
	{
		(void)sCopyString(job, pParent->lc_issuer, &(pComponent->lc_issuer));
	}
	if(pParent->lc_notice)			/* Intellectual prop.notice */
	{
		(void)sCopyString(job, pParent->lc_notice, &(pComponent->lc_notice));
	}
	if(pParent->lc_prereq)			/* Prerequesite products */
	{
		(void)sCopyString(job, pParent->lc_prereq, &(pComponent->lc_prereq));
	}
	if(pParent->lc_sublic)			/* Sub-licensed products */
	{
		(void)sCopyString(job, pParent->lc_sublic, &(pComponent->lc_sublic));
	}
	if(pParent->lc_dist_constraint)	/* extra distributor constraints */
	{
		(void)sCopyString(job, pParent->lc_dist_constraint, &(pComponent->lc_dist_constraint));
	}
	if(pParent->lc_serial)			/* Serial Number */
	{
		(void)sCopyString(job, pParent->lc_serial, &(pComponent->lc_serial));
	}
	if(pParent->lc_issued)			/* Date issued */
	{
		(void)sCopyString(job, pParent->lc_issued, &(pComponent->lc_issued));
	}
done:
	return;

}

static
int
verify_server_key(
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
	CONFIG *		conf = *confp, *sav = *confp, *pkg = NULL;
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
		LM_SET_ERROR(job, LM_CANTMALLOC, 602, 0, 0, LM_ERRMASK_ALL);
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
			LM_SET_ERROR(job, LM_CANTMALLOC, 602, 0, 0, LM_ERRMASK_ALL);
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
 *		This indicates that we checked it out from the server,
 *		and don't need to check the local hostid
 */
	}
/*
 *	Add a server struct for this conf which has a correct hostid
 *	Make sure it's correctly added to job->conf_server also.
 */

	conf->server = server;
	if(package)
		(*confp)->server = conf->server;

	for (s = conf->server, i = 0; i < idx ; i++, s = s->next)
	{
		if (!s)
		{
			s  = (LM_SERVER *)l_malloc(job, sizeof(LM_SERVER));
			if (s_sav)
                s_sav->next = s;
		}
		cp = id[i];
		/*s->idptr = (HOSTID *)l_malloc(job, sizeof (HOSTID));*/
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
				l_free_conf(job, pkg);
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
#endif /* NO_ENCRYPTION_CHECK */





/*-
 *	Include l_crypt.c, so that these functions won't be global
 *	on Unix, at least not from here.
 */
/*#define LM_CRYPT*/
#define LM_CKOUT
#include "l_crypt.c"
