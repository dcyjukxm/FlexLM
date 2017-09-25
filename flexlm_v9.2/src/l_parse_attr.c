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
/*	
 *	Module: $Id: l_parse_attr.c,v 1.46 2003/05/14 17:43:40 sluu Exp $
 *
 *	Function: 	l_parse_attr()
 *
 *	Description:  	parse name=value pairs in f/i/u line
 *
 *	Parameters:	job, conf, str
 *
 *	Return:		NULL = success, 
 *			ptr to first error in str;
 *
 *	D. Birns
 *	8/22/1995
 *
 *	Last changed:  10/24/98
 *
 */
#if DEBUG_ATTR
#define LM_INTERNAL
#include "lm_code.h"
#endif /* DEBUG_ATTR */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"

#define	T_VENDOR_STRING 0
#define	T_HOSTID_STRING 	1
#define	T_DUP_STRING		2
#define	T_OVERDRAFT_STRING 	3
#define	T_VENDOR_INFO		4
#define	T_USER_INFO		5
#define	T_ASSET_INFO		6
#define	T_ISSUER		7
#define	T_NOTICE		8
#define	T_LINGER_STRING 	9
#define	T_PREREQ 		10
#define	T_SUBLIC		11
#define	T_DIST_CONSTRAINT 	12
#define	T_WBINARY_STRING 	13
#define	T_WARGV0_STRING 	14
#define	T_WQUEUE_STRING 	15
#define	T_WTERMS_STRING		16
#define	T_WLOSS_STRING		17
#define T_CKSUM			18
#define T_DIST_INFO		19
#define T_OPTIONS		20
#define T_COMPONENT		21
#define T_SERIAL_STRING		22
#define	T_SUITE_DUP_STRING	23
#define	T_TYPE			24
#define T_SUITE			25
#define T_CAPACITY		26
#define T_USER_BASED		27
#define T_HOST_BASED		28
#define T_METER			29
#define T_SUPERSEDE		30
#define T_ISSUED		31
#define T_MINIMUM		32
#define T_POLICY		33
#define T_PLATFORMS		34
#define T_START			35
#define T_SORT			36
#define	T_SIGN			38
#define	T_BORROW		39
#define	T_FLOAT_OK		40
#define T_BUNDLE		41
#define T_TS_OK		42

typedef struct _names {
	const int type;
	char const * const name;
	int found; /* 0 -- not found, 1, found, -1 -- wrapper name (illegal) */
} NAME;

/*
 *	sluu
 *	These are the valid keywords that FLEXlm recongnizes.  Whenever a new
 *	keyword is added, you must add it to this list.
 */
static NAME names [] = 
{
	{T_VENDOR_STRING, LM_LICENSE_VENDOR_STRING, 0},
	{T_HOSTID_STRING, LM_LICENSE_HOSTID_STRING, 0},
	{T_DUP_STRING, LM_LICENSE_DUP_STRING, 0},
	{T_OVERDRAFT_STRING, LM_LICENSE_OVERDRAFT_STRING, 0},
	{T_VENDOR_INFO, LM_LICENSE_VENDOR_INFO, 0},
	{T_USER_INFO, LM_LICENSE_USER_INFO, 0},
	{T_ASSET_INFO, LM_LICENSE_ASSET_INFO, 0},
	{T_ISSUER, LM_LICENSE_ISSUER, 0},
	{T_NOTICE, LM_LICENSE_NOTICE, 0},
	{T_CKSUM, LM_LICENSE_CKSUM, 0},
	{T_DIST_INFO, LM_LICENSE_DIST_INFO, 0},
	{T_COMPONENT, LM_LICENSE_COMPONENT_STRING, 0},
	{T_SERIAL_STRING, LM_LICENSE_SERIAL_STRING, 0},
	{T_SUITE_DUP_STRING, LM_LICENSE_SUITE_DUP_STRING, 0},
	{T_OPTIONS, LM_LICENSE_OPTIONS_STRING, 0},
	{T_TYPE, LM_LICENSE_TYPE_STRING, 0},
	{T_SUITE, LM_LICENSE_SUITE_STRING, 0},
	{T_CAPACITY, LM_LICENSE_CAPACITY, 0},
	{T_USER_BASED, LM_LICENSE_USER_BASED, 0},
	{T_PLATFORMS, LM_LICENSE_PLATFORMS, 0},
	{T_HOST_BASED, LM_LICENSE_HOST_BASED, 0},
	{T_METER, LM_LICENSE_METER, 0},
	{T_SUPERSEDE, LM_LICENSE_SUPERSEDE, 0},
	{T_ISSUED, LM_LICENSE_ISSUED, 0},
	{T_LINGER_STRING, LM_LICENSE_LINGER_STRING, 0},
	{T_MINIMUM, LM_LICENSE_MINIMUM, 0},
	{T_PREREQ, LM_LICENSE_PREREQ, 0}, /* used by wrapper */
	{T_START, LM_LICENSE_START_DATE, 0},
	{T_SORT, LM_LICENSE_SORT, 0},
	{T_SIGN, LM_LICENSE_KEY, 0},
	{T_BORROW, LM_LICENSE_BORROW, 0},
	{T_FLOAT_OK, LM_LICENSE_FLOAT_OK, 0},
	{T_BUNDLE, LM_LICENSE_BUNDLE_STRING, 0},
	{T_TS_OK, LM_LICENSE_TS_OK, 0},
	{-1, (char *)0, 0}
};
static NAME wrapper_names[] =
{
	{T_WBINARY_STRING, LM_LICENSE_WBINARY_STRING, 0},
	{T_WARGV0_STRING, LM_LICENSE_WARGV0_STRING, 0},
	{T_WQUEUE_STRING, LM_LICENSE_WQUEUE_STRING, 0},
	{T_WTERMS_STRING, LM_LICENSE_WTERMS_STRING, 0},
	{T_WLOSS_STRING, LM_LICENSE_WLOSS_STRING, 0},
	{-1, (char *)0, 0}
};


static NAME * gettoken 	lm_args((LM_HANDLE *, unsigned char **, int *, char **));
static int addtoconf	lm_args((LM_HANDLE *, NAME *,char **, CONFIG *,
					int, char **));
static unsigned char * skip	lm_args((unsigned char *));
static int check_names	lm_args(( NAME **, NAME *, char **, char **));
static char **parse_strlist lm_args((LM_HANDLE *, unsigned char *));
static void l_strip_os_ver lm_args(( char *));

char * API_ENTRY
l_parse_attr(job, str, conf)
LM_HANDLE *job;
char *str;
CONFIG *conf;
{
  char buf[MAX_CONFIG_LINE];
  char *bufp = buf;
  char *err = (char *) 0;
  NAME * np;
  char *context;
  int n;

	strncpy(buf, str, MAX_CONFIG_LINE);

	for (np = names; np->name; np++)
		np->found = 0;
	for (np = wrapper_names; np->name; np++)
		np->found = 0;


	for (context = bufp; np = gettoken(job, (unsigned char **)&bufp, &n, (char **)&err); context = bufp)
	{
		if (!np->found)
		{
			LM_SET_ERROR(job, LM_FUTURE_FILE, 313, 0, context, LM_ERRMASK_ALL);
			conf->lc_future_minor = 313;
			continue;
		}
		if (addtoconf(job, np, &bufp, conf, n, &err))
			return err;
	}
	return err;
}
static
NAME *
gettoken(job, str, n, err)
LM_HANDLE *job;
unsigned char **str;
int *n;
char **err;
{
  unsigned char *cp = *str;
  NAME *np;
  int got = 0;
/*
 *	skip white space 
 */
	*n = -1;	/* memory threat */

	if (!*str || !**str)
		return (NAME *)0;
	while (isspace(*cp)) 
	{
		cp++;
		(*str)++;
	}
	if (got = check_names(&np, names, (char **)&cp, (char **)str))
		;
	else if (got = check_names(&np, wrapper_names, (char **)&cp, (char **)str))
	{
		if (l_getattr(job, WRAPPER) != WRAPPER_VAL)
			np->found = -1; /* illegal */
	}
	if ((str[0][-1] != '=') && isdigit(**str))
	{
		cp = *str;
		sscanf((char *)cp, "%d", n);	/* overrun checked */
		while (isdigit(*cp) || (*cp == '=')) cp++;
		*str = cp;
	}
	if (got) return np;
	if (err && !*err) 
	{
		*err = (char *)l_malloc(job, strlen((char *)cp) + 1);
		strcpy((char *)*err, (char *)cp);
	}
	while (isspace(*cp)) cp++;
	*str = cp;
	*str = skip((unsigned char *)*str);
	return np;
}
	
static
int
check_names(ret, n, cpp, str)
NAME **ret;
NAME *n;
char **cpp;
char **str;
{
  NAME *np;
  char *cp = *cpp;
  const char * ncp; 
  char *scp;
  char c1, c2;

	for (np = n; np->name; np++)
	{
		if (np->found) continue;
		for (ncp = np->name, scp = *str; *ncp && *scp; ncp++, scp++)
		{
			c1 = *ncp; c2 = *scp;
			if (toupper(c1) != toupper(c2)) break;
		}
		if (!*ncp && 
			((*scp == '=') || 
			(!*scp  || isspace(*scp) || isdigit(*scp))))
		{
			np->found = 1;
			if (*scp == '=')
				*str = ++scp;
			else
				*str = scp;
			*ret = np;
			*cpp = cp;
			return 1;
		}
	}
	*cpp = cp;
	*ret = np;
	return 0;
}

/*
 *	returns 0, success, <>0, lm_errno;
 */
static
int
addtoconf(job, np, str, conf, n, err)
LM_HANDLE *job;
NAME *np;	/* name pointer -- current name */
char **str;	/* Gets set to return value -- malloc'd */
CONFIG *conf;	
int n;
char **err;	/* gets set to error string, if necessary */
{
  unsigned char *ret = (unsigned char *)*str;
  char *m;
  unsigned char *cp1, *cp2;
  int i;
  int ret_errno = 0;

/*
 *	setup return pointer
 */
	cp2 = ret = skip((unsigned char *)ret);
	cp2--;
	while (isspace (*cp2))
		cp2--;
	if (*cp2 == '"')
		cp2--;
	cp2++;
	cp1 = (unsigned char *)*str;
	if (*cp1 == '"')
		cp1++;
#if 0	/* P6924 */
	if (*cp1 == '=')
		cp1++;
#endif
	i = (cp2 - cp1);
/*
 *	if found == -1, we return, don't put it in the license file,
 *	but we waited till now to get the arg eaten in the license line
 */
	if (np->found == -1) 
	{
		*str = (char *)ret;
		return ret_errno;
	}

	m = (char *)l_malloc(job, i+1);
	l_zcp(m, (char *)cp1, i);


	switch (np->type)
	{
	case T_VENDOR_STRING: 
		conf->lc_vendor_def = m;
		break;
	case T_HOSTID_STRING:
		if (l_get_id(job, &conf->idptr, m))
			ret_errno = job->lm_errno;
		free(m);
		break;
	case T_CKSUM:
		conf->lc_cksum = atoi(m);
		free(m);
	    	conf->lc_got_options |= LM_LICENSE_CKSUM_PRESENT;
		break;
	case T_DUP_STRING:
	case T_SUITE_DUP_STRING:
		{
		  char *newq = m;
		  int dup_group = 0;

			if (np->type == T_DUP_STRING)
				conf->lc_dup_group = 0;
			else
				conf->lc_suite_dup = 0;

			l_uppercase(m);
                        if (isdigit(*m))
                                dup_group = atoi(m);  /* Old style */
                        else if (l_keyword_eq(job, m, "SITE"))
                                dup_group = LM_DUP_SITE;
                        else if (l_keyword_eq(job, m, "NONE"))
                                dup_group = LM_DUP_NONE;
                        else
                        {
				newq = m;
                                while (*newq)
                                {
					if (*newq == 'U')
                                            dup_group |= LM_DUP_USER;
					else if (*newq == 'H')
                                            dup_group |= LM_DUP_HOST;
					else if (*newq == 'D')
					    dup_group |= LM_DUP_DISP;
					else if (*newq == 'V')
					    dup_group |= LM_DUP_VENDOR;
					newq++;
				}
				if (dup_group == 0)
					dup_group = LM_DUP_NONE;
			}

			if (np->type == T_DUP_STRING)
			{
				conf->lc_dup_group = dup_group;
				conf->lc_got_options |= LM_LICENSE_DUP_PRESENT;
			}
			else
			{
				conf->lc_suite_dup = dup_group;
				conf->lc_got_options |= 
						LM_LICENSE_SUITE_DUP_PRESENT;
			}

			free(m);
		}
		break;
	case T_OVERDRAFT_STRING:
		if (l_getattr(job, FULL_FLEXLM) == FULL_FLEXLM_VAL)
                {
			conf->lc_overdraft = atoi(m);
			conf->lc_got_options |= LM_LICENSE_OVERDRAFT_PRESENT;
		}
		free(m);
		break;
	case T_SERIAL_STRING:
		conf->lc_serial = m;
		break;
	case T_VENDOR_INFO:
		conf->lc_vendor_info = m;
		break;
	case T_DIST_INFO:
		conf->lc_dist_info = m;
		break;
	case T_USER_INFO:
		conf->lc_user_info = m;
		break;
	case T_SIGN:
		{
		  LM_KEYLIST *k, *ktmp;
		  char *m_tmp, *cp;
		  int ilen, len;
			if (l_keyword_eq_n(job, m, "strength:", 9))
			{
			  int bits = 0;
				sscanf(&m[9], "%d", 	/* overrun checked */
					&bits);
				switch(bits)
				{
				case 113:
					conf->strength_override =
					LM_STRENGTH_113BIT; break;
				case 163:
					conf->strength_override =
					LM_STRENGTH_163BIT; break;
				case 239:
					conf->strength_override =
					LM_STRENGTH_239BIT; break;
				case 1:
					conf->strength_override =
					LM_STRENGTH_DEFAULT; break;
				default:
					LM_SET_ERRNO(job, LM_BADFILE, 537, 0);
					ret_errno = LM_BADFILE;
				}
				cp = m + 9;
				m_tmp = l_malloc(job, strlen(cp) + 1);
				strcpy(m_tmp, cp);
				free(m);
				m = m_tmp;
			}
			np->found = 0; /* because you can have more than one */
			k = l_malloc(job, sizeof(LM_KEYLIST));
			k->key = m;
			if (n == -1) k->sign_level = 1;
			else k->sign_level = n;
			if ((n == 1) || (n == -1)) conf->lc_sign = k->key;
			if (!conf->strength_override && 
				(job->flags & LM_FLAG_IS_VD))
			{
				for (ilen = 0, len = 0; m[ilen]; ilen++)
					if (!isspace(m[ilen])) len++;
				if (len < L_PUBKEY113BIT_LEN)
					conf->strength_override = 
						LM_STRENGTH_DEFAULT; 
				else if (len == L_PUBKEY113BIT_LEN)
					conf->strength_override = 
						LM_STRENGTH_113BIT; 
				else if (len == L_PUBKEY163BIT_LEN)
					conf->strength_override = 
						LM_STRENGTH_163BIT; 
				else conf->strength_override = LM_STRENGTH_239BIT; 
			}
			
/*
 *			Now insert in numeric order:
 */
			if (conf->lc_keylist)
			{
				if (conf->lc_keylist->sign_level > k->sign_level)
				{
					k->next = conf->lc_keylist;
					conf->lc_keylist = k;
				}
				else
					for (ktmp = conf->lc_keylist; ktmp ; 
						ktmp = ktmp->next)
					{
						if ( (!(ktmp->next) ||	
							ktmp->next->sign_level > 
							k->sign_level))
						{
							k->next = ktmp->next;
							ktmp->next = k;
							break;
						}
					}
			}
			else conf->lc_keylist = k;
		}
		if (!*conf->code && !(job->flags & LM_FLAG_LIC_GEN))
		{
			l_zcp(conf->code, m, MAX_CRYPT_LEN);
		}
		else if (!conf->lc_sign) 
			conf->L_CONF_FLAGS |= L_CONF_FL_OLD_KEY;
		break;
	case T_ASSET_INFO:
		conf->lc_asset_info = m;
		break;
	case T_ISSUER:
		conf->lc_issuer = m;
		break;

	case T_ISSUED:
		conf->lc_issued = m;
		break;

	case T_NOTICE:
		conf->lc_notice = m;
		break;
	case T_LINGER_STRING:
		conf->lc_linger = atoi(m);
		conf->lc_got_options |= LM_LICENSE_LINGER_PRESENT;
		free(m);
		break;
	case T_PREREQ:
		conf->lc_prereq = m;
		break;
	case T_SUBLIC:
		conf->lc_sublic = m;
		break;
	case T_DIST_CONSTRAINT:
		conf->lc_dist_constraint = m;
		break;
	case T_OPTIONS:
	case T_TYPE:
	{
	  NAME *tnp;
	  char *cp = m;
	  int dofree = 1;
	  int unused;
		conf->lc_got_options |= LM_LICENSE_OPTIONS_PRESENT;
		while (tnp = gettoken(job, (unsigned char **)&cp, &unused, 
						err ? 0 : err))
		{
			switch(tnp->type)
			{
			case T_SUITE: conf->lc_options_mask |= LM_OPT_SUITE; 
					break;
			case T_BUNDLE: conf->lc_options_mask |= LM_OPT_BUNDLE; 
					break;
			default: 
				dofree = 0;
				if (err) *err = m;
				LM_SET_ERRNO(job, LM_FUTURE_FILE, 315, 0);
				ret_errno = LM_FUTURE_FILE;
				conf->lc_future_minor = 315;

			}
		}
		if (dofree) free(m);
	}
	break;
	case T_BUNDLE:
	case T_SUITE:
		conf->lc_got_options |= LM_LICENSE_OPTIONS_PRESENT;
		free(m);
		switch (np->type)
		{
		case T_SUITE: conf->lc_options_mask |= LM_OPT_SUITE; break;
		case T_BUNDLE: conf->lc_options_mask |= LM_OPT_BUNDLE; break;
		}
		break;
	case T_SUPERSEDE:
		conf->lc_got_options |= LM_LICENSE_OPTIONS_PRESENT;
		if (*m) 
		{
			if (conf->lc_supersede_list = parse_strlist(job, (unsigned char *)m))
				conf->lc_options_mask |= LM_OPT_SUPERSEDE; 
		}
		else 
			conf->lc_options_mask |= LM_OPT_SUPERSEDE; 
		free(m);
		break;
	case T_CAPACITY:
	case T_METER:
	case T_USER_BASED:
	case T_HOST_BASED:
	case T_MINIMUM:
	case T_FLOAT_OK:
	case T_PLATFORMS:
		conf->lc_got_options |= LM_LICENSE_TYPE_PRESENT;
		switch (np->type)
		{
		case T_CAPACITY: conf->lc_type_mask |= LM_TYPE_CAPACITY; break;
		case T_METER: conf->lc_type_mask |= LM_TYPE_METER; break;
		case T_HOST_BASED: 
			if (isdigit(*m)) conf->lc_host_based = atoi(m);
			else	conf->lc_host_based = conf->users;
			conf->lc_type_mask |= LM_TYPE_HOST_BASED; break;
		case T_USER_BASED: 
			if (isdigit(*m)) conf->lc_user_based = atoi(m);
			else	conf->lc_user_based = conf->users;
			conf->lc_type_mask |= LM_TYPE_USER_BASED; break;
		case T_MINIMUM: 
			conf->lc_minimum = atoi(m);
			conf->lc_type_mask |= LM_TYPE_MINIMUM; break;
		case T_PLATFORMS: 
			if (*m)
			{
				l_strip_os_ver(m);
				if (conf->lc_platforms = parse_strlist(job, (unsigned char *)m))
					conf->lc_type_mask |= LM_TYPE_PLATFORMS; 
			}
			else 
			{
				if (err) *err = m;
				LM_SET_ERRNO(job, LM_BADFILE, 290, 0);
				ret_errno = LM_BADFILE;
			}
			break;
		case T_FLOAT_OK: conf->lc_type_mask |= LM_TYPE_FLOAT_OK; 
			if (*m) l_get_id(job, &conf->floatid, m);
			break;
		}
		free(m);
		break;
	case T_COMPONENT:
	{
	  char *next_comp = m;
	  CONFIG *confp, *savconfp = (CONFIG *)0;

		conf->lc_vendor_def = next_comp;
		while (next_comp)
		{
			confp = 0;
			next_comp = l_parse_component( job, next_comp, &confp,
			conf->lc_options_mask & LM_OPT_SUITEBUNDLE);
			if (confp)
			{
				strcpy(confp->daemon, conf->daemon);
			}
/*
 *                      	link confp into package chain
 */
			if (!conf->components)  conf->components = confp;
			else                    savconfp->next = confp;
			savconfp = confp;
		}
	}
	break;
	case T_START: 
		l_zcp(conf->startdate, m, DATE_LEN);
		free(m);
		break;
	case T_WBINARY_STRING:
		conf->lc_w_binary = m;
		break;
	case T_WARGV0_STRING:
		conf->lc_w_argv0 = m;
		break;
	case T_WQUEUE_STRING:
		conf->lc_w_queue = atoi(m);
		conf->lc_got_options |= LM_LICENSE_WQUEUE_PRESENT;
		free(m);
		break;
	case T_SORT: 
		{
			if (l_keyword_eq(job, m, "first"))
				conf->lc_sort = 1;
			else if (l_keyword_eq(job, m, "last"))
				conf->lc_sort = 200;
			else 
			{
			  int j = -99;
				j = atoi(m);
				if ((j == -99) ||
					(j == 0)) /* atoi failed */
				{
					LM_SET_ERROR(job, LM_BADFILE, 463, 
					0, m, LM_ERRMASK_ALL);
				}
				else conf->lc_sort = j;
 			}
			free(m);
		}
		break;
	case T_BORROW:
		conf->lc_got_options |= LM_LICENSE_TYPE_PRESENT;
		conf->lc_type_mask |= LM_TYPE_BORROW;
		conf->lc_max_borrow_hours = (7*24);
		if (m)
		{
			if (*m ) 
			{
			  int j = -99;

				j = atoi(m);
				if ((j != -99) && (j != 0)) /* atoi failed */
					conf->lc_max_borrow_hours = j;
			}
			free(m);
		}
		break;
	case T_WTERMS_STRING:
		conf->lc_w_termsig = atoi(m);
		conf->lc_got_options |= LM_LICENSE_WTERMS_PRESENT;
		free(m);
		break;
	case T_WLOSS_STRING:
		conf->lc_w_loss = atoi(m);
		conf->lc_got_options |= LM_LICENSE_WLOSS_PRESENT;
		free(m);
		break;
	case T_TS_OK:
		conf->lc_got_options |= LM_LICENSE_TYPE_PRESENT;
		conf->lc_type_mask |= LM_TYPE_TS_OK;
		break;
	default:
		free(m);
		/* This should never happen here! */
		LM_SET_ERRNO(job, LM_FUTURE_FILE, 264, 0);
		ret_errno = LM_FUTURE_FILE;
		conf->lc_future_minor = 264;
		break;
	}
	*str = (char *)ret;


	return ret_errno;
	
}
/*
 *	skip to the next attribute (?)
 */
static
unsigned char *
skip(str)
unsigned char *str;
{
	if (*str == '"')
	{
		str++;
		while (*str && *str != '"')
			str++;
		if (*str == '"')
			str++;
	}
	else while (*str && !isspace(*str))
		str++;
	while (*str && isspace(*str))
		str++;
	return str;
}
static
char **
parse_strlist(job, str)
LM_HANDLE *job;
unsigned char *str;
{
   char **ret;
   unsigned char *cp, *cp2;
   int cnt = 0;
   char *buf, *bufp;


/*
 *	The return is essentially a null-terminated array
 */
	if (*str == '"')
		str++;
	for (cp = str; *cp; cp = skip((unsigned char *)cp))
		cnt++;
	
	ret = (char **)l_malloc(job, (cnt + 1) * sizeof(char *));
	bufp = buf = (char *)l_malloc(job, strlen((char *)str) + 1);

	memset((char *)ret, 0, (cnt + 1) * sizeof(char *));
	memset(buf, 0, strlen((char *)str) + 1);
	for (cnt = 0, cp = str; *cp; cp = skip((unsigned char *)cp), cnt++)
	{
		ret[cnt] = bufp;
		for (cp2 = cp; *cp2 && !isspace(*cp2); )
			*bufp++ = *cp2++;
		*bufp++ = 0;
	}
	return ret;
}
/*
 *	strip_os_ver
 *	Assume the string is of the format "plat_[a-z]*[0-9]*", like i86_rl2
 *	If it's this format, and if the number[s] are there, replace
 *	them with spaces.
 */
static
void
l_strip_os_ver(str)
char *str;
{
  char *cp;


	for (cp = str; *cp; )
	{
		if (*cp == '_')
		{
			cp++;
			while (*cp && isalpha(*cp)) cp++;
			while (*cp && isdigit(*cp)) *cp++ = ' ';
			continue;
		}
		cp++;
	}
}



#if DEBUG_ATTR
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
main()
{
	LM_HANDLE *job;
	char *err;
	CONFIG conf;
	lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &job);
	if (err = l_parse_attr(job, "HOSTID=\"12345 456789\" DUP_GROUP=HD laskdfj foobar", &conf))
		puts(err);

}
#endif /* DEBUG_ATTR */
