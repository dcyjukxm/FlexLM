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
/*	$Id: lm_chk_conf.c,v 1.25 2003/05/08 18:32:52 jwong Exp $	*/

/**	@file		lm_chk_conf.c
 *	@brief		Validates entries in config structure.
 *	@version	$Revision: 1.25 $
 *
 ****************************************************************************/

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

static void addstr lm_args(( LM_HANDLE *, char **, char *));
static int l_check_fmt_ver_hostids lm_args(( LM_HANDLE *, HOSTID *));

char * API_ENTRY
lc_chk_conf(LM_HANDLE *job, CONFIG *conf, int check_name)
{
  char *ret = (char *)0;
  unsigned char *cp;
  CONFIG *c;

	if (LM_API_ERR_CATCH) return 0;
	if (conf->users == 0)
	{
		if (conf->lc_got_options & LM_LICENSE_DUP_PRESENT)
			addstr(job, &ret,
			"DUP_GROUP not valid with uncounted license");
		if( !conf->idptr && conf->type != CONFIG_PACKAGE)
			addstr(job, &ret,
			"Hostid required for uncounted feature");
		else if (conf->lc_type_mask & LM_TYPE_FLOAT_OK)
		{
		  HOSTID *h;
			for (h = conf->idptr; h; h = h->next)
			{

				if (!(LM_IS_FLEXID(h->type)))
				{
					addstr(job, &ret,
			"FLOAT_OK only valid with node-locked license");
					break;
				}
			}
			if (conf->floatid &&
				!memcmp(&conf->idptr->id.string, &conf->floatid->id.string,
				sizeof(conf->idptr->id.string)))
			{
					addstr(job, &ret,
			"Do not use same hostid for FLOAT_OK=hostid as HOSTID=");
					addstr(job, &ret,
			"The FLOAT_OK=hostid should be the Server's permanent hostid");
			}

		}
		if ((conf->lc_type_mask & LM_TYPE_USER_BASED) ||
			(conf->lc_type_mask & LM_TYPE_HOST_BASED) ||
			(conf->lc_type_mask & LM_TYPE_MINIMUM) )
			addstr(job, &ret,
			"HOST or USER BASED licenses must be counted");

	}
	else
	{
		if (conf->lc_type_mask & LM_TYPE_FLOAT_OK)
			addstr(job, &ret,
			"FLOAT_OK only valid with node-locked license");
	}
	if (check_name  && (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V8) < 0) )
	{
		for (cp = (unsigned char *)conf->feature; *cp; cp++)
			if (!isdigit(*cp) && !isalpha(*cp) &&
						(*cp != '_') && (*cp != '-')
			&& (*cp < 128))
				addstr(job, &ret,
		"Illegal char in feature name: only alpha-num and '_' allowed");
	}

	if ((conf->lc_options_mask & (LM_OPT_SUITE | LM_OPT_BUNDLE)) &&
					conf->type != CONFIG_PACKAGE)
		addstr(job, &ret, "SUITE only applies to PACKAGE lines");
	if ((conf->lc_options_mask & LM_OPT_SUPERSEDE) &&
		!conf->lc_issued && !conf->startdate[0] &&
			!l_extract_date(job, conf->code))
		addstr(job, &ret, "SUPERSEDE missing START= or ISSUED=");
	if ((conf->lc_type_mask & LM_TYPE_USER_BASED) &&
			(conf->lc_type_mask & LM_TYPE_HOST_BASED))
		addstr(job, &ret, "Can't combine USER_BASED and HOST_BASED");
	if (conf->components)
	{
		for (c = conf->components; c; c = c->next)
		{
			if (l_keyword_eq(job, c->feature, conf->feature))
			{
				addstr(job, &ret,
				"PACKAGE and COMPONENT name can't be identical");
			}
		}
	}
	if (conf->lc_issued)
	{
		char buf[10];
		int x, nflds, yr;
		nflds = sscanf(conf->lc_issued, "%d-%[^-]-%d", &x, buf, &yr );	/* overrun threat */
		if (nflds != 3)
			addstr(job, &ret, "ISSUED Invalid date format");
		else if (yr == 0)
			addstr(job, &ret, "ISSUED Can't have year 0");
	}

	LM_API_RETURN(char *, ret)
}

static
void
addstr(LM_HANDLE *job, char **ptr, char *str)
{
  char *new;
  char *old = *ptr;
  int len = 0;

	LM_SET_ERRNO(job, LM_BADFILE, 269, 0);
	if (old) len = strlen(old) + 1;
	len += strlen(str) + 1;
	new = (char *)l_malloc(job, len);

	if (old)
	{
		sprintf(new, "%s\n%s", old, str);
		free(old);
	}
	else strcpy(new, str);
	*ptr = new;
}

/*
 *	l_check_fmt_ver
 *	Check what's in the conf against what's allowed for this
 *	version.
 *	return: 0=success, 1 failure
 */

/****************************************************************************/
/**	@brief	Function that is called to check to see if components in a license
 *			are valid for a particular version.  WARNING! When adding new
 *			features that don't work with older versions of the product, you
 *			MUST put a check for that new feature here as failing to do	so
 *			will result in a license that is invalid if you specify lmcrypt
 *			with the -verfmt option.  For example, BORROW was added in v8,
 *			any attempt to generate	a license for a version prior to 8 with
 *			the BORROW keyword will fail.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	conf		CONFIG structure that represents the feature as read
 *						read from the license file.
 *
 *	@return	0 on success, else 1 on failure.
 ****************************************************************************/
int API_ENTRY
l_check_fmt_ver(LM_HANDLE *job, CONFIG *conf)
{

  LM_SERVER *s;
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V3) < 0)
	{
		if (conf->type == CONFIG_INCREMENT || conf->type ==
			CONFIG_UPGRADE )
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 364, 0);
			return job->lm_errno;
		}
		if (conf->lc_got_options)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 365, 0);
			return job->lm_errno;
		}
		if (conf->lc_type_mask | conf->lc_options_mask)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 366, 0);
			return job->lm_errno;
		}
		if (conf->lc_sign)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 522, 0);
			return job->lm_errno;
		}
		if (conf->lc_vendor_info || conf->lc_dist_info ||
			conf->lc_user_info || conf->lc_asset_info ||
			conf->lc_issuer || conf->lc_notice || conf->lc_issued)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 367, 0);
			return job->lm_errno;
		}
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V4) < 0)
	{
		if (conf->lc_got_options & (
			LM_LICENSE_CKSUM_PRESENT | LM_LICENSE_OVERDRAFT_PRESENT|
			LM_LICENSE_DUP_PRESENT))
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 368, 0);
			return job->lm_errno;
		}
		if (conf->type == CONFIG_PACKAGE)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 369, 0);
			return job->lm_errno;
		}
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V5) < 0)
	{
		if (conf->lc_type_mask)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 372, 0);
			return job->lm_errno;
		}
		if (conf->lc_options_mask & LM_OPT_SUPERSEDE)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 394, 0);
			return job->lm_errno;
		}
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V5_1) < 0)
	{
		if (conf->lc_type_mask & LM_TYPE_PLATFORMS)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 373, 0);
			return job->lm_errno;
		}
		if (conf->lc_supersede_list)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 374, 0);
			return job->lm_errno;
		}
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V6) < 0)
	{
		if (*conf->startdate)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 393, 0);
			return job->lm_errno;
		}
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V7) < 0)
	{
		if (conf->lc_sort)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 421, 0);
			return job->lm_errno;
		}
	}

#if 0
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V7_1) < 0)
	{
	}
#endif
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V8) < 0)
	{
		if (conf->lc_type_mask & LM_TYPE_FLOAT_OK)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 571, 0);
			return job->lm_errno;
		}
		if (conf->lc_type_mask & LM_TYPE_BORROW)
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 572, 0);
			return job->lm_errno;
		}
	}
	for (s = conf->server; s; s = s->next)
	{
		if (l_check_fmt_ver_hostids(job, s->idptr))
		{
			LM_SET_ERRNO(job, LM_LGEN_VER, 371, 0);
			return job->lm_errno;
		}
	}

	return 0;
}

/****************************************************************************/
/**	@brief	Function that is called to check to see if hostid type in a license
 *			are valid for a particular version.  WARNING! When adding new
 *			hostid types that don't work with older versions of the product, you
 *			MUST put a check for that new hostid type here as failing to do	so
 *			will result in a license that is invalid if you specify lmcrypt
 *			with the -verfmt option.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	idptr		HOSTID pointer to check for validity
 *
 *	@return	0 on success, else 1 on failure.
 ****************************************************************************/
static
int
l_check_fmt_ver_hostids(LM_HANDLE *job, HOSTID *idptr)
{
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V3) < 0)
	{
		if (idptr->type == HOSTID_STRING)
			return 1;
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V4) < 0)
	{
		if (idptr->type == HOSTID_INTERNET ||
			idptr->type >= HOSTID_VENDOR)
			return 1;

	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V5) < 0)
	{
		if (idptr->type == HOSTID_FLEXID1_KEY ||
		    idptr->type == HOSTID_FLEXID2_KEY ||
		    idptr->type == HOSTID_FLEXID3_KEY ||
		    idptr->type == HOSTID_FLEXID4_KEY ||
		    idptr->type == HOSTID_FLEXID5_KEY ||
		    idptr->type == HOSTID_FLEXID6_KEY)
			return 1;
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V5_1) < 0)
	{
		if (idptr->type == HOSTID_SERNUM_ID )
			return 1;
	}
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V9) < 0)
	{
		if (idptr->type == HOSTID_COMPOSITE )
			return 1;
	}

	return 0; /* success */

}
