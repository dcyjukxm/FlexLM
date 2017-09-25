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
 *	Module: $Id: l_package.c,v 1.21 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function: 	l_post_pkg
 *			l_parse_package
 *
 *	Description:  	Process license PACKAGE line
 *
 *	Parameters:	l_parse_package:
 *			job		LM_HANDLE *
 *			line	-- char * line from license file
 *			temp	-- CONFIG * gets filled in with resulting PKG 
 *			data	-- char * temp space used for sscanf.
 *
 *			l_parse_component:
 *			job	-- LM_HANDLE *
 *			compstr	-- char * pointer to rest of PACKAGE line
 *			confp	-- pointer to CONFIG *: set if valid feature
 *				   l_parse_component will malloc and
 *				   initialize this struct
 *
 *	Return: 	l_parse_package:
 *				return 0 if ok, else lm_errno
 *			l_parse_component:
 *				char * -- 	0   EOF or error 
 *							if lm_errno set
 *						!0  pointer to remaining
 *
 *	D. Birns
 *	8/29/94
 *
 *	Last changed:  10/22/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include <errno.h>
#include <stdio.h>
#include "l_prot.h"

#define D(_x) (data+ (_x*(MAX_CONFIG_LINE+1)))
#define D2(_x, _y) (data+ (_x*(MAX_CONFIG_LINE+1)) + _y)
#define DATA(_x, _y)	*(data+ (_x*(MAX_CONFIG_LINE+1)) + _y)
#define PKGNAME 	0
#define PKGDAEMON 	1
#if 0
#define PKGVERSION 	2
#define PKGOPTS 	3
#else
#define PKGOPTS 	2
#endif
#define PKG_START_DATE "1-jan-0"
static int l_enable_pkg_feats lm_args(( LM_HANDLE *, CONFIG *, CONFIG *));


int API_ENTRY
l_parse_package(job, line, temp, data, errstr)
LM_HANDLE *job;
char *line; 	
CONFIG *temp;
char *data;
char **errstr;
{
  int nflds, ret = 0, err = job->lm_errno, i;  
  char type[MAX_CONFIG_LINE+1];
  char field[MAX_CONFIG_LINE+1];
  char *cp;
  CONFIG *confp, *next;
  char *parse_err;

	memset(temp, 0, sizeof(CONFIG));
/*
 *	First parse head of PACKAGE line
 */
	nflds = sscanf(line, "%s %s %s %[^\n]\n", type, D(PKGNAME), 	/* overrun checked */
		D(PKGDAEMON), D(PKGOPTS));
	if (nflds < PKGOPTS)
	{
		LM_SET_ERROR(job, LM_BADPKG, 533, 0, line, LM_ERRMASK_ALL);
		goto exit_l_parse_package;
	}
#if 0
	if (l_valid_version(D(PKGVERSION)))
	{
		strncpy(temp->version, D(PKGVERSION), MAX_VER_LEN);
	}
	else 
	{
		LM_SET_ERROR(job, LM_BAD_VERSION, 225, 0, D(PKGVERSION), 
							LM_ERRMASK_ALL);
		ret = LM_BAD_VERSION;
		goto exit_l_parse_package;
	}
#endif
	cp = D(PKGOPTS);
	{
	  char *v = field;
	  char *c = cp;
		while (*c && !isspace(*c))
			*v++ = *c++;
		*v = 0;
		if (l_valid_version(field) /*&& strcmp(field, "0")*/)
		{
			l_zcp(temp->version, field, MAX_VER_LEN);
			cp = c;
			while (isspace(*cp))
				cp++;
		}
		else
			strcpy(temp->version, LM_PKG_ANY_VERSION);
	}
	sscanf(cp, "%[^ 	]", field);	/* overrun checked */
	if (isxdigit(*field) && !strchr(field, '='))
	{
	  char *cp2 = temp->code;
		for(i = 0; isxdigit(*cp) &&  (i < MAX_CRYPT_LEN); i++)
		{
			*cp2++ = *cp++;
		}
		*cp2 = 0;
		temp->L_CONF_FLAGS |= L_CONF_FL_OLD_KEY;
	}
	else if (l_keyword_eq_n(job, cp, "start", 5))
	{
	  char *cp2 = temp->code;
		for(i = 0; *cp && !isspace(*cp) && (i < MAX_CRYPT_LEN); i++)
		{
			*cp2++ = *cp++;
		}
		*cp2 = 0;
		temp->L_CONF_FLAGS |= L_CONF_FL_OLD_KEY;
	}
		
/*
 *	get options 
 */
	while (isspace(*cp)) cp++;

	if (*cp && (parse_err = l_parse_attr(job, cp, temp))) 
	{
		if (errstr && !*errstr) *errstr = parse_err;
		goto exit_l_parse_package;
	}
		
	temp->type = CONFIG_PACKAGE;
	strncpy(temp->feature, D(PKGNAME), MAX_FEATURE_LEN);
	strncpy(temp->daemon, D(PKGDAEMON), MAX_DAEMON_NAME);

	if (!temp->components)
	{
		LM_SET_ERROR(job, LM_BADPKG, 291, 0, line, LM_ERRMASK_ALL);
		return 1;
	}

	strcpy(temp->date, PKG_START_DATE);
	if (err != job->lm_errno)
	{
		ret = job->lm_errno;
		goto exit_l_parse_package;
	}
		
	ret = 0;

exit_l_parse_package:

	if (ret) /* failure, clean up memory */
	{
		for (confp = temp->components; confp; confp = next)
		{
			next = confp->next;
			l_free_conf(job, confp);
		}
	}
	return ret;
}


/*
 *	l_parse_component:  feature[:version[:count]]
 */
LM_CHAR_PTR  API_ENTRY
l_parse_component(job, str, confpp, suite_flag)
LM_HANDLE *job;
char *str;	 /* components string */
CONFIG **confpp; /* pointer to CONFIG * -- gets set in this function */
int suite_flag; /* If true -- then we're going to delete pkg feature */
{
  char feature[MAX_CONFIG_LINE + 1];
  char version[MAX_CONFIG_LINE + 1];
  int users = -1;
  char *cp;
  CONFIG *confp;

	if (!str || !*str) return 0;
/*
 *	Skip leading spaces 
 */
	for (; *str && isspace(*str); str++) 	; /* null action */

	*feature = *version = '\0';
	if (!sscanf(str, "%[^: ]:%[^: ]:%d", feature, version, &users))	/* overrun checked */
	{
		LM_SET_ERROR(job, LM_BADPKG, 77, 0, str, LM_ERRMASK_ALL);
		return 0;
	}
	version[MAX_VER_LEN] = '\0';
	if (*version && !l_valid_version(version))
	{
		LM_SET_ERROR(job, LM_BAD_VERSION, 78, 0, version, LM_ERRMASK_ALL);
		return 0;
	} 
/*
 *	Error if users set and suite -- can't checkout package and
 *	component with different number of features
 */
	if (users>=0 && suite_flag)
	{
		LM_SET_ERROR(job, LM_BADPKG, 79, 0, str, LM_ERRMASK_ALL);
		return 0;
	}
		
	feature[MAX_FEATURE_LEN] = '\0'; /* truncate if necessary */
/*
 *	We have at least feature at this point.
 */
	confp = (CONFIG *)l_malloc(job, sizeof (CONFIG));
	if(!confp)
	{
		LM_SET_ERROR(job, LM_CANTMALLOC, 601, 0, 0, LM_ERRMASK_ALL);
		return 0;
	}
	*confpp = confp;
	memset((char *)confp, 0, sizeof(CONFIG));
	confp->type = CONFIG_FEATURE;
	strcpy(confp->feature, feature);
	strcpy(confp->version, version); /* may be empty */
	if (users == 0) /* 0 is invalid in COMPONENT */
	{
		LM_SET_ERROR(job, LM_BADPKG, 81, 0, str, LM_ERRMASK_ALL);
		return 0;
	}
	else if (users == -1) users = 1; /* if not set, use 1 */
	confp->users = users; 
/*
 *	Find next component or end of string
 */

	for (cp = str; *cp && !isspace(*cp); cp++) 	; /* null action */
/* 
 *	Skip trailing blanks
 */
	for (; *cp && isspace(*cp); cp++) 	; /* null action */
	if (!*cp) cp = (char *)0;
	return cp;
}


/*
 *	l_post_pkg -- post features in packages to real job
 */
void
l_post_pkg(job) 
LM_HANDLE *job;
{
  CONFIG *pkgloop, *nextpkg, *featloop, *nextconf;
  CONFIG *savefeat = (CONFIG *)0;

/* 
 *	First loop through each PACKAGE line
 */
	for (pkgloop = job->line; pkgloop; pkgloop = nextpkg)
	{
#ifdef NLM
		ThreadSwitch();
#endif /*NLM */
		nextpkg = pkgloop->next;
		if (pkgloop->type != CONFIG_PACKAGE)
		{
			continue;
		}
/* 
 *		Find all FEATUREs/INCREMENTs for this package
 */
		for (featloop = job->line; 
			featloop; featloop = nextconf)
		{
#ifdef NLM
                ThreadSwitch();
#endif /*NLM */

			nextconf = featloop->next;
			if ((featloop->type == CONFIG_FEATURE || 
				featloop->type == CONFIG_INCREMENT ||
				featloop->type == CONFIG_UPGRADE) && 
				l_keyword_eq(job, featloop->feature, 
						pkgloop->feature) &&
				(!l_compare_version(job, pkgloop->version,
					featloop->version) ||
				(featloop->type == CONFIG_UPGRADE && 
				!l_compare_version(job, pkgloop->version,
					featloop->fromversion))))
			{
				featloop->package_mask |= 
						LM_LICENSE_PKG_ENABLE;
/*
 *				We have a match -- explode package
 */

				if (l_enable_pkg_feats(job, featloop, pkgloop))
					return; /* fatal error */
				if  (pkgloop->lc_options_mask & LM_OPT_SUITE)
				{
					featloop->package_mask |= 
						LM_LICENSE_PKG_SUITE;
				} 
				else if (pkgloop->lc_options_mask & 
							LM_OPT_BUNDLE)
				{
					featloop->package_mask |= 
						LM_LICENSE_PKG_BUNDLE;
#if 1	/* BUNDLE_SUPPORT */
			/*
			 *	Comment this out for now as there isn't a way with lmcrypt currently to set the corresponding
			 *	bit.  We would need to rewrite lmcrypt such that it parses feature/increment lines to
			 *	see if they enable a PACKAGE/BUNDLE or not.  What's happening now is lmcrypt produces a value of
			 *	"DUP_GROUP=7" when generating the signature, we product the value of "DUP_GROUP=23"
			 *	when verifying it because we flip the bit for LM_DUP_BUNDLE.  This was tested with a DUP_GROUP of
			 *	UHD
			 */
							
#else 
					featloop->lc_dup_group |= LM_DUP_BUNDLE;
#endif 
				}
			}
			savefeat = featloop;
		}
/*
 *		move package to job->packages list
 */
		for (savefeat = (CONFIG *)0, featloop = job->line; 
			featloop; featloop = featloop->next)
		{
#ifdef NLM
                ThreadSwitch();
#endif /*NLM */

			if (featloop == pkgloop)
			{
			  CONFIG *savc, *c, *nextc;
				if (savefeat)
					savefeat->next = pkgloop->next;
				else
					job->line = pkgloop->next;
				pkgloop->next = (CONFIG *)0;
				for (savc = (CONFIG *)0, c = job->packages; 
							c ; c = (c)->next)
					savc = c;
				if (savc)
					savc->next = pkgloop;
				else
					job->packages = pkgloop;
/*
 *				Now remove component CONFIG structs
 */
				for (c = pkgloop->components; c; c = nextc)
				{
#ifdef NLM
       			         ThreadSwitch();
#endif /*NLM */
					nextc = c->next;
					l_free_conf(job, c);
				}
				pkgloop->components = (CONFIG *)0;
				break;
			}
			savefeat = featloop;
		}
	}
}

/*
 *	l_enable_pkg_feats()
 *	fill out the features read in by the PACKAGE line, multiplied
 *	by the FEATURE
 */
static 
int
l_enable_pkg_feats(job, feat, pkg)
LM_HANDLE *job;
CONFIG *feat; 		/* enabling FEATURE line */
CONFIG *pkg;		/* matching PACKAGE line */
{
	CONFIG *temp, *component, *next, *last = feat, *save = feat->next;

	if (!pkg->components)
		return 0; /* nothing to do */

	for(component = pkg->components; component; component = next)
	{
#ifdef NLM
                ThreadSwitch();
#endif /*NLM */

		next = component->next;
		temp = (CONFIG *)l_malloc(job, sizeof (CONFIG));
		last->next = temp;
		l_conf_copy(job, temp, feat);
		temp->package_mask = 0;
		temp->lc_options_mask &= ~LM_OPT_SUITE;
		temp->lc_options_mask &= ~LM_OPT_BUNDLE;
		strcpy(temp->feature, component->feature);
		if (*component->version)
			strcpy(temp->version, component->version);
		temp->users = component->users * feat->users;
		strcpy(temp->code, feat->code);
		temp->package_mask |= LM_LICENSE_PKG_COMPONENT;
		last = temp;
		temp->parent_pkg = pkg;
		temp->parent_feat = feat;
	}
	temp->next = save;
	return 0;
	
}
#ifndef RELEASE_VERSION
void
dump_configs(job)
LM_HANDLE *job;
{
  CONFIG *c;
  void dump_config lm_args((CONFIG *));

	printf("FEATURES:\n");
	for (c = job->line; c; c = c->next)
	{
		dump_config(c);
	}
	printf("PACKAGES:\n");
	for (c = job->packages; c; c = c->next)
	{
		dump_config(c);
	}
}
void
dump_config(c)
CONFIG *c;
{
  CONFIG *pkg;
  LM_KEYLIST *k;
	printf("0x%x: %s %s code %s v%s o:%d\n", c, 
		c->type == CONFIG_FEATURE ? "FEATURE" : 
		c->type == CONFIG_INCREMENT ? "INCREMENT" :
		c->type == CONFIG_PACKAGE ? "PACKAGE" :
		c->type == CONFIG_BORROW ? "BORROW" :
		c->type == CONFIG_PORT_HOST_PLUS ? "PORT_HOST_PLUS" :
		c->type == CONFIG_UPGRADE ? "UPGRADE" : "UNKNOWN",
		c->feature, c->code, c->version, c->file_order);
	if (c->type == CONFIG_PACKAGE && c->components)
	{
		puts("dumping package:");
		for (pkg = c->components; pkg; pkg = pkg->next)
			printf("\t0x%x: COMPONENT %s code %s\n", 
					pkg, pkg->feature, pkg->code);
	}
	if (c->package_mask & LM_LICENSE_PKG_SUITE)
		puts("\tPACKAGE SUITE parent");
	if (c->package_mask & LM_LICENSE_PKG_BUNDLE)
		puts("\tPACKAGE BUNDLE parent");
	if (c->package_mask & LM_LICENSE_PKG_ENABLE)
		puts("\tPACKAGE parent");
	if (c->package_mask & LM_LICENSE_PKG_COMPONENT)
		puts("\tCOMPONENT");
	if (c->parent_pkg)
	{
		printf("\tparent pkg is %s %s\n", 
			c->parent_pkg->feature, c->parent_pkg->code);
	}
	if (c->parent_feat)
	{
		printf("\tparent feat is %s %s\n", 
			c->parent_feat->feature, c->parent_feat->code);
	}
	for (k = c->lc_keylist; k; k = k->next)
	{
		printf("\tkeylist key %s, addrs: %x %x\n", k->key, k, k->key);
	}
	if (c->lc_issuer)
		printf("\tissuer %s addr %x\n", c->lc_issuer, c->lc_issuer);
}
#endif /* RELEASE_VERSION */
