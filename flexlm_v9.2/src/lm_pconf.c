/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_pconf.c,v 1.50 2003/02/12 19:41:05 sluu Exp $
 *
 *	Function:	l_print_config
 *
 *	Description: 	Prints a configuration (feature) line to a string.
 *
 *	M. Christiano
 *	4/19/90
 *
 *	Last changed:  10/27/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_pack.h"
#include "l_prot.h"
#include "lgetattr.h"

#define MAX_LICENSE_LEN 70

static void add lm_args((char *s, char *d, char *o));
static void addi lm_args((int i, char *d, char *o));
static char * ver_3dec lm_args(( LM_HANDLE *, char *));
static void do_dup_group lm_args(( LM_HANDLE *, char *, int, char *));
static void add_fixed_param lm_args((char *, char *));
static int l_print_config_asc lm_args(( LM_HANDLE *, CONFIG *, char *));
static char * getnextword lm_args(( char **, int *));
static void add_nl_to_result lm_args(( LM_HANDLE *, char *));
static void add_users lm_args(( LM_HANDLE *, CONFIG *, char *));
static void add_signature(LM_HANDLE *job, CONFIG *config, char *ptr);

#define NOSTRING ((char *)-1)


int API_ENTRY
lc_print_config(job, config, buf)
LM_HANDLE *job;
CONFIG *config;
char *buf;
{
	l_clear_error(job);
	return l_print_config(job, config, buf);
}

int 
API_ENTRY
l_print_config(job, config, buf)
LM_HANDLE *job;
CONFIG *config;
char *buf;
{
	if (config->decimal_fmt) 
		return l_print_conf_dec(job, config, buf);
	else 
		return l_print_config_asc(job, config, buf);
}

static 
int
l_print_config_asc(job, config, buf) /* gen printable string (no newline) */
LM_HANDLE *job;		/* Current license job */
CONFIG *config;	/* same info returned by lm_get_config */
char *buf;	/* must be at least MAX_CONFIG_LINE+1 long */
{
  char *ptr=buf;
  char *aschid;
  char *keyword = LM_RESERVED_FEATURE;
  int oldstyle = 0;
  int oldflag = (job->flags & (LM_FLAG_PRT_CONF_OLDEST_FMT | 
			LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN));
  int conftype;

/*
 *	Now, figure out if we can do it as an old-style license
 */
	conftype = config->type;
	if ((conftype == CONFIG_INCREMENT) &&  /* P6220 */
				(config->lc_options_mask & LM_OPT_ISFEAT)) 
	{
		conftype = CONFIG_FEATURE;
	}
	if (((strcmp(job->options->behavior_ver, LM_BEHAVIOR_V6) < 0 )
		&& strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V6) >= 0 )
	 	|| (L_STREQ(job->options->license_fmt_ver, LM_BEHAVIOR_V2))
	 	|| oldflag
	 	|| (job->options->flags & LM_OPTFLAG_JAVA_LIC_FMT))
	{
		oldstyle = 1;
		if (config->lc_got_options || config->lc_w_binary || 
			config->lc_w_argv0
			|| config->lc_vendor_info || config->lc_dist_info
			|| config->lc_user_info || config->lc_asset_info
			|| config->lc_keylist
			|| config->lc_issuer || config->lc_notice
			|| config->lc_serial 
			|| config->lc_prereq || config->lc_sublic
			|| config->lc_overdraft || config->lc_cksum
			|| config->lc_issued || config->lc_type_mask
			|| conftype == CONFIG_PACKAGE 
			|| (config->idptr && config->idptr->next) /*id list*/
			|| config->lc_dist_constraint)
			oldstyle = 0;
	}
	if (config->server && (config->server->sflags & L_SFLAG_PARSED_DEC))
	{
	  LM_SERVER *s;
	  char portbuf[MAX_SHORT_LEN + 1];

		for (s = config->server; s; s = s->next)
		{

			if (s->port > 0)
				sprintf(portbuf, "%d", s->port);
			else
				*portbuf = 0;
			sprintf(ptr, "%s %s %s %s\n", LM_RESERVED_SERVER, 
				s->name[0] ? s->name : LM_RESERVED_THIS_HOST, 
				l_asc_hostid(job, s->idptr), 
				portbuf);
			ptr += strlen(ptr);
		}
		if ((strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V6) >= 0)
		 && !oldflag)
			sprintf(ptr, "%s %s\n", LM_RESERVED_PROG_ALIAS, 
							config->daemon);
		else
			sprintf(ptr, "%s %s\n", LM_RESERVED_PROG, 
							config->daemon);
		ptr += strlen(ptr);
		config->server->sflags &= ~L_SFLAG_PARSED_DEC;
	}
		

	if (conftype == CONFIG_UPGRADE)
	{
		sprintf(ptr, "%s", LM_RESERVED_UPGRADE);
		ptr += strlen(ptr);
		add_fixed_param(config->feature, ptr);
		add_fixed_param(config->daemon, ptr);
		add_fixed_param(ver_3dec(job, config->fromversion), ptr);
		add_fixed_param(ver_3dec(job, config->version), 
							ptr);
		l_lowercase(config->date);
		if (L_STREQ(config->date, "1-jan-0") &&
				(strcmp(job->options->license_fmt_ver, 
				LM_BEHAVIOR_V6) >= 0) &&
		                !oldflag)
			add_fixed_param(LM_RESERVED_UNEXPIRING, ptr);
		else 	
			add_fixed_param(config->date, ptr);
		{
			add_users(job, config, ptr);
		}
		/*if (config->code && !strchr(config->code, ' '))*/
		if ((config->code[0]) && 
				(config->L_CONF_FLAGS & L_CONF_FL_OLD_KEY))
			add_fixed_param(config->code, ptr);
	}
	else if (conftype == CONFIG_PACKAGE)
	{
		if (config->lc_sign && 
			!(config->L_CONF_FLAGS & L_CONF_FL_OLD_KEY)) /* P5540*/
			sprintf(ptr, "%s %s %s %s", 
			LM_RESERVED_PACKAGE, config->feature, config->daemon,
			ver_3dec(job, config->version));
		else
			sprintf(ptr, "%s %s %s %s %s", 
			LM_RESERVED_PACKAGE, config->feature, config->daemon,
			ver_3dec(job, config->version), config->code);
	}
	else
	{
		if (conftype == CONFIG_INCREMENT)
			keyword = LM_RESERVED_INCREMENT;
		strcpy(ptr, keyword); /* use strcpy for 1st one */
		add_fixed_param(config->feature, ptr);
		add_fixed_param(config->daemon, ptr);
		add_fixed_param(ver_3dec(job, config->version), ptr);
		l_lowercase(config->date);
		if (L_STREQ(config->date, "1-jan-0") &&
				(strcmp(job->options->license_fmt_ver, 
				LM_BEHAVIOR_V6) >= 0) &&
				 !oldflag)
			add_fixed_param(LM_RESERVED_UNEXPIRING, ptr);
		else 	
			add_fixed_param(config->date, ptr);
		{
			add_users(job, config, ptr);
		}
		/*if (*config->code && !strchr(config->code, ' '))*/
		if ((config->code[0]) && 
				(config->L_CONF_FLAGS & L_CONF_FL_OLD_KEY))
			add_fixed_param(config->code, ptr);
	}

	aschid = l_asc_hostid(job, config->idptr);

	if (oldstyle)
	{
		ptr += strlen(ptr);
		*ptr++ = ' ';
		*ptr++ = '"';
		if (config->lc_vendor_def)
		{
			strcpy(ptr, config->lc_vendor_def);
			ptr += strlen(config->lc_vendor_def);
		}
		*ptr++ = '"';
		*ptr = '\0';
		if (aschid && *aschid) 
		{
			*ptr++ = ' ';	/* add the separating space */
			strcpy(ptr, aschid);
		}
	}
	else
	{
	 	if (oldflag)
			add_signature(job, config, ptr);
		if (conftype == CONFIG_PACKAGE)
		{
			add(config->lc_vendor_def, 
					LM_LICENSE_COMPONENT_STRING, ptr);
		}
		else
		{
			add(config->lc_vendor_def, 
					LM_LICENSE_VENDOR_STRING, ptr);
		}
		add(aschid, LM_LICENSE_HOSTID_STRING, ptr);
		if (config->lc_got_options & LM_LICENSE_OVERDRAFT_PRESENT)
		    addi(config->lc_overdraft, 
					LM_LICENSE_OVERDRAFT_STRING, ptr);
		if (config->lc_sort)
		{
			if (config->lc_sort == 200)
			    add("last", LM_LICENSE_SORT, ptr);
			else if (config->lc_sort == 1)
			    add("first", LM_LICENSE_SORT, ptr);
			else addi(config->lc_sort, LM_LICENSE_SORT, ptr);
		}
		if (config->lc_got_options & LM_LICENSE_TYPE_PRESENT)
		{

			if (config->lc_type_mask & LM_TYPE_CAPACITY)
				add(NOSTRING, LM_LICENSE_CAPACITY, ptr);
			if (config->lc_type_mask & LM_TYPE_USER_BASED)
			{
				if (config->users != config->lc_user_based)
					addi(config->lc_user_based, 
					LM_LICENSE_USER_BASED, ptr);
				else
					add(NOSTRING, 
						LM_LICENSE_USER_BASED, ptr);
			}
			if (config->lc_type_mask & LM_TYPE_HOST_BASED)
			{
				if (config->users != config->lc_host_based)
					addi(config->lc_host_based, 
					LM_LICENSE_HOST_BASED, ptr);
				else
					add(NOSTRING, 
						LM_LICENSE_HOST_BASED, ptr);
			}
			if (config->lc_type_mask & LM_TYPE_METER)
				add(NOSTRING, LM_LICENSE_METER, ptr);
			if (config->lc_type_mask & LM_TYPE_MINIMUM)
			{
				addi(config->lc_minimum, 
							LM_LICENSE_MINIMUM, ptr);
			}
			if (config->lc_type_mask & LM_TYPE_PLATFORMS)
			{
			  char platlist[MAX_CONFIG_LINE + 1];
			  char **cpp, *cp;
				for (cp = platlist, 
					cpp = config->lc_platforms;
							*cpp; cpp++)
				{
					strcpy(cp, *cpp);
					cp += strlen(cp);
					*cp++ = ' ';
				}
				cp--;
				*cp = 0;
				add(platlist, LM_LICENSE_PLATFORMS, ptr);
			}
			if (config->lc_type_mask & LM_TYPE_FLOAT_OK)
				add(config->floatid ? 
				l_asc_hostid(job, config->floatid) : NOSTRING, 
					LM_LICENSE_FLOAT_OK, ptr);
			if (config->lc_type_mask & LM_TYPE_BORROW)
			{
				if (config->lc_max_borrow_hours != (7*24))
					addi(config->lc_max_borrow_hours, 
						LM_LICENSE_BORROW, ptr);
				else
					add(NOSTRING, 
						LM_LICENSE_BORROW, ptr);
			}
		}
		if (config->lc_got_options & LM_LICENSE_OPTIONS_PRESENT)
		{
			if (config->lc_options_mask & LM_OPT_SUITE)
				add(LM_LICENSE_SUITE_STRING, 
					LM_LICENSE_OPTIONS_STRING, ptr);
			if (config->lc_options_mask & LM_OPT_BUNDLE)
				add(LM_LICENSE_BUNDLE_STRING, 
					LM_LICENSE_OPTIONS_STRING, ptr);
			if (config->lc_options_mask & LM_OPT_SUPERSEDE)
			{
			  char featlist[MAX_CONFIG_LINE + 1];
			  char **cpp, *cp;
				if (!config->lc_supersede_list)
					add(NOSTRING, 
						LM_LICENSE_SUPERSEDE, ptr);
				else
				{
					for (cp = featlist, 
						cpp = config->lc_supersede_list;
								*cpp; cpp++)
					{
						strcpy(cp, *cpp);
						cp += strlen(cp);
						*cp++ = ' ';
					}
					cp--;
					*cp = 0;
					add(featlist, 
						LM_LICENSE_SUPERSEDE, ptr);
				}
			}
		}
		if (config->lc_got_options & LM_LICENSE_LINGER_PRESENT)
		    addi(config->lc_linger, LM_LICENSE_LINGER_STRING, ptr);
		if (config->lc_got_options & LM_LICENSE_DUP_PRESENT)
			do_dup_group(job, LM_LICENSE_DUP_STRING, 
					config->lc_dup_group, ptr);
		if (config->lc_got_options & LM_LICENSE_SUITE_DUP_PRESENT)
			do_dup_group(job, LM_LICENSE_SUITE_DUP_STRING,
					config->lc_suite_dup, ptr);

		if (l_getattr(job, WRAPPER) == WRAPPER_VAL)
		{
			add(config->lc_w_binary, 
				LM_LICENSE_WBINARY_STRING, ptr);
			add(config->lc_w_argv0, LM_LICENSE_WARGV0_STRING, 
								ptr);
			if (config->lc_got_options & LM_LICENSE_WQUEUE_PRESENT)
				addi(config->lc_w_queue, 
					LM_LICENSE_WQUEUE_STRING, ptr);
			if (config->lc_got_options & LM_LICENSE_WTERMS_PRESENT)
				addi(config->lc_w_termsig, 
					LM_LICENSE_WTERMS_STRING, ptr);
			if (config->lc_got_options & LM_LICENSE_WLOSS_PRESENT)
				addi(config->lc_w_loss, LM_LICENSE_WLOSS_STRING,
								ptr);
		}
		add(config->lc_vendor_info, LM_LICENSE_VENDOR_INFO, ptr);
		add(config->lc_dist_info, LM_LICENSE_DIST_INFO, ptr);
		add(config->lc_user_info, LM_LICENSE_USER_INFO, ptr);
		add(config->lc_asset_info, LM_LICENSE_ASSET_INFO, ptr);
		add(config->lc_issuer, LM_LICENSE_ISSUER, ptr);
		add(config->lc_issued, LM_LICENSE_ISSUED, ptr);
		add(config->lc_notice, LM_LICENSE_NOTICE, ptr);
		add(config->lc_prereq, LM_LICENSE_PREREQ, ptr);
		add(config->lc_sublic, LM_LICENSE_SUBLIC, ptr);
		add(config->lc_dist_constraint, 
					LM_LICENSE_DIST_CONSTRAINT, ptr);
		if (config->lc_got_options & LM_LICENSE_CKSUM_PRESENT)
		    addi(config->lc_cksum, LM_LICENSE_CKSUM, ptr);
		add(config->lc_serial, LM_LICENSE_SERIAL_STRING, ptr);
		if (*config->startdate)
			add(config->startdate, LM_LICENSE_START_DATE, 
								ptr);

/* Terminal server notation */
		if (config->lc_type_mask & LM_TYPE_TS_OK)
			add(NOSTRING, LM_LICENSE_TS_OK, ptr);
							
	 	if (!oldflag)
			add_signature(job, config, ptr);
#ifdef SUPPORT_METER_BORROW
		add(config->lc_meter_borrow_info, LM_LICENSE_METER_BORROW_INFO, ptr);
#endif /* SUPPORT_METER_BORROW */
		add_nl_to_result(job, buf);
	}
	/* TBD - check for buffer overflow? */
	return 0;
}

static
void
add_fixed_param(str, out)
char *str;
char *out;
{
	strcat(out, " ");
	strcat(out, str);
}


static void
add(str, desc, out)
char *str;
char *desc;
char *out;
{
  char x[MAX_CONFIG_LINE + 1];
  char newstr[MAX_CONFIG_LINE + 1];
  int quote = 0;

	*newstr = 0;
	if (str != NOSTRING && (!str || !*str))
		return;
	if (str == NOSTRING)
		sprintf(x, " %s", desc);
	else
	{

		strcpy(newstr, str);

		if (*newstr != '"') /* already quoted */
		{
		  char *c = 0;
			c = strchr(newstr, ' ');
			if (c) quote = 1;
			c = strchr(newstr, '	');
			if (c) quote = 1;
		}
		if (quote)
			sprintf(x, " %s=\"%s\"", desc, newstr);
		else
			sprintf(x, " %s=%s", desc, newstr);
	}
	strcat(out, x);
}

static void
addi(d, desc, out)
int d;
char *desc;
char *out;
{
  char x[MAX_CONFIG_LINE];

	sprintf(x, " %s=%d", desc, d);
	strcat(out, x);
}
/*-
 *	ver_3dec  make sure version has 3 decimal points -- P571 buf fix
 *		assumes ver is MAX_VER_LEN + 1 long
 */
static 
char *
ver_3dec(job, ver)
LM_HANDLE *job;
char *ver;

{
  char *cp;
  int got_dec = 0, cnt = 0, i, len;

	if (!strcmp(ver, LM_PKG_ANY_VERSION))
		return "";
	if (strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V3) > 0)
		return ver; /* doesn't apply after v3 */
	for (len = 0, cp = ver; *cp; cp++, len++)
	{
		if (got_dec) cnt++;
		if (*cp == '.') 	got_dec = 1;
	}
	if (!got_dec && len < MAX_VER_LEN)
	{
		*cp++ = '.';
		len++;
	}
	for (i = 0; len < MAX_VER_LEN && i < (3 - cnt); i++, len++)
		*cp++ = '0';
	*cp = '\0';
	return ver;
}
static 
void
do_dup_group(job, name, dup_group, ptr)
LM_HANDLE *job;
char *name;
int dup_group;
char *ptr;
{
  char string[20];


	if (dup_group == 0) /* LM_DUP_SITE is 0 */
		strcpy(string, "SITE");
	else if (dup_group & LM_DUP_NONE)
		strcpy(string, "NONE");
	else
	{
		string[0] = '\0';
		if (dup_group & LM_DUP_USER) 
			strcat(string, "U");
		if (dup_group & LM_DUP_HOST) 
			strcat(string, "H");
		if (dup_group & LM_DUP_DISP) 
			strcat(string, "D");
		if (dup_group & LM_DUP_VENDOR) 
			strcat(string, "V");
	}
	add(string, name, ptr);
}
static
void
add_nl_to_result(job, inbuf)
LM_HANDLE *job;
char *inbuf;
{
  char outbuf [MAX_CONFIG_LINE + 1];
  char *icp = inbuf;
  char *ocp = outbuf;
  char *nextword;
  int linelen = 0;
  int totlen = 0;
  int wordlen;
  int maxline;
  int thisword;
  int i;
  int firstword = 1;
  int thisline = 0;

	if (job->options->max_license_len < 0) return; /* leave it alone */
	if (job->options->max_license_len)
		maxline = job->options->max_license_len ;
	else
		maxline = MAX_LICENSE_LEN;

	while (nextword = getnextword(&icp, &wordlen))
	{
		linelen += wordlen + 1;
		totlen += wordlen + 1;
		if ((totlen) > MAX_CONFIG_LINE)
		{
			/* truncate and return */
			*ocp = 0; return;
		}
		if (wordlen > maxline) thisword = wordlen;
		else thisword = 0;
		if ((linelen > maxline) && !firstword)
		{
			sprintf(ocp, "\\\n\t");
			linelen = wordlen + 8;
			thisline = 8;
			totlen += 7;
			ocp += 3;
			if (thisword) 
				thisword += 8;
		}
		if ((totlen) > MAX_CONFIG_LINE)
		{
			/* truncate and return */
			*ocp = 0; return;
		}
		for (i = 0; i < wordlen; i++)
		{
			if (nextword[i] == '\n') linelen = 0;
			thisline++;
			*ocp++ = nextword[i];
		}
		if (firstword) firstword = 0;
		*ocp++ = ' ';
		*ocp = 0;
	}
	ocp[-1] = 0;
	strcpy(inbuf, outbuf);
}
static
char *
getnextword(str, len)
char **str;
int *len;
{
  unsigned char *cp = *(unsigned char **)str;
  unsigned char * ret;

#define ISSPACE(x) (isspace((unsigned char)(x)) && ((x) != '\n'))

	*len = 0;
	while (ISSPACE(*cp)) cp++;
	if (ret = (*cp ? cp : 0))
	{
		while (*cp && (!ISSPACE(*cp)) )
		{
			(*len)++;
			cp++;
		}
		while (*cp && ISSPACE(*cp)) cp++;
		*str = (char *)cp;
	}
	return (char *)ret;
}
static
void
add_users(job, conf, out)
LM_HANDLE *job;
CONFIG *conf;
char *out;
{
  char nusers[MAX_CONFIG_LINE + 1];
  int oldflag = (job->flags & (LM_FLAG_PRT_CONF_OLDEST_FMT | 
			LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN));

	if (!conf->users && 
		(strcmp(job->options->license_fmt_ver, 
		LM_BEHAVIOR_V6) >= 0) &&
		!oldflag)
	{
		add_fixed_param(LM_RESERVED_UNCOUNTED, out);
	}
#ifdef SUPPORT_METER_BORROW
	else if (conf->users == LM_COUNT_FROM_METER)
	{
		if (conf->meter_borrow_device)
			sprintf(nusers, "%s=%s:%s:%d", 
				LM_LICENSE_COUNT,
				LM_LICENSE_METER_BORROW,
				conf->meter_borrow_device,
				conf->meter_borrow_counter);
		else 
			sprintf(nusers, "%s=%s:%d", 
				LM_LICENSE_COUNT,
				LM_LICENSE_METER_BORROW,
				conf->meter_borrow_counter);
		add_fixed_param(nusers, out);
	}
#endif /* SUPPORT_METER_BORROW */
	else
	{
		if(conf->type == CONFIG_UPGRADE &&
			conf->upgrade_count != conf->users)
		{
			sprintf(nusers, "%d", conf->upgrade_count);
		}
		else
		{
			sprintf(nusers, "%d", conf->users);
		}
		add_fixed_param(nusers, out);
	}
}
static
void
add_signature(LM_HANDLE *job, CONFIG *config, char *ptr)
{
   LM_KEYLIST *k;
   char buf[sizeof(LM_LICENSE_KEY) + MAX_LONG_LEN + 1];
	if (job->flags & LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN)
		return;
	for (k = config->lc_keylist; k; k = k->next)
	{
		
		if (k->sign_level > 1)
			sprintf(buf, "%s%d", LM_LICENSE_KEY,
				k->sign_level);
		else
			sprintf(buf, "%s", LM_LICENSE_KEY);
		add(k->key, buf, ptr);
	}
}
