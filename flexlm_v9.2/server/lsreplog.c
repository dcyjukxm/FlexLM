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
 *	Module: $Id: lsreplog.c,v 1.23 2003/05/08 00:03:24 sluu Exp $
 *
 *	Description:	All report log functions here
 *
 *	D. Birns
 *	11/7/95
 *
 *	Last changed:  12/12/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "ls_log.h"
#include "lsreplog.h"
#include "lgetattr.h"
#include "l_strkey.h"
/*
 *	static prototypes
 */
static void add_str lm_args((char *, char **, int));
static void add_i lm_args((int, char **, int));
static void add_time lm_args((char **));
static void log_curr_usage lm_args((char *, char *));
static void add_key lm_args((int, char **));
static void dump_opts lm_args(( OPTIONS *, char *));
static void add_cpu_usages lm_args((CLIENT_DATA *, char **));

int ls_replog_ascii[] = {
		LL_CONFIG,
		LL_DICTIONARY,
		LL_FLUSH,
		LL_PACKAGE,
		LL_PACKAGE_SUITE,
		-1};
int ls_replog_clear[] = {
		LL_START,
		-1};

/*-
 *      ls_feat_info() - Dump all static feature info to report log file
 */
extern FEATURE_LIST *ls_flist;
void
ls_feat_info()
{
  CONFIG *c;
  FEATURE_LIST *f;
  char buf [MAX_CONFIG_LINE + 1];
  char buf2 [MAX_CONFIG_LINE + 10];
  char buf3 [MAX_REPFILE_LEN];
  char *cp;
  DICTKEY key;
  LS_POOLED *p;
  GROUP *g;
  extern GROUP *groups, *hostgroups;
  extern long ls_user_init1 ;
  extern long ls_user_init2 ;
  extern long ls_outfilter ;
  extern long ls_infilter ;
  extern long ls_incallback ;
  extern long ls_vendor_msg ;
  extern long ls_vendor_challenge ;
  extern char *ls_user_lockfile ;
  extern int ls_conn_timeout;
  extern int ls_enforce_startdate;
  extern int ls_tell_startdate;
  extern int ls_minimum_user_timeout;
  extern int ls_min_lmremove;
  extern int ls_use_featset ;
  extern int ls_do_checkroot;
  extern int ls_show_vendor_def;
  extern long ls_daemon_periodic;
  extern int ls_use_all_feature_lines ;
  extern int ls_compare_vendor_on_increment;
  extern int ls_compare_vendor_on_upgrade;
  extern char *ls_a_behavior_ver;
  extern int ls_a_check_baddate;
  extern int ls_a_lkey_start_date;
  extern int ls_a_lkey_long;
  extern int ls_a_license_case_sensitive;

/*-
 *	Dump this hostid
 */
	cp = buf;
	add_key(LL_HOSTID, &cp);
	add_str(l_asc_hostid(lm_job, l_gethostid(lm_job)), &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf);
/*-
 *	Dump this platform name
 */
	cp = buf;
	add_key(LL_SERVER_PLATFORM, &cp);
	add_str(l_platform_name(), &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf);

/*-
 *	Dump other server info
 */
	cp = buf;
	add_key(LL_SERVER_INFO, &cp);
	add_i(lm_job->code.flexlm_version, &cp, 0); /* FLEXlm-ver */
	add_i(lm_job->code.flexlm_revision, &cp, 0); /* FLEXlm-rev */
	add_i(ls_user_init1 ? 1 : 0, &cp, 0);
	add_i(ls_user_init2 ? 1 : 0, &cp, 0);
	add_i(ls_outfilter ? 1 : 0, &cp, 0);
	add_i(ls_infilter ? 1 : 0, &cp, 0);
	add_i(ls_incallback ? 1 : 0, &cp, 0);
	add_i(ls_vendor_msg ? 1 : 0, &cp, 0);
	add_i(ls_vendor_challenge ? 1 : 0, &cp, 0);
	add_str(ls_user_lockfile, &cp, 0);
	add_i(ls_conn_timeout, &cp, 0);
	add_i(ls_enforce_startdate, &cp, 0);
	add_i(ls_tell_startdate, &cp, 0);
	add_i(ls_minimum_user_timeout, &cp, 0);
	add_i(ls_min_lmremove, &cp, 0);
	add_i(ls_use_featset ? 1 : 0, &cp, 0);
	add_i(ls_do_checkroot, &cp, 0);
	add_i(ls_show_vendor_def, &cp, 0);
	add_i(ls_daemon_periodic ? 1 : 0, &cp, 0);
	add_i(ls_use_all_feature_lines, &cp, 0);
	add_i(ls_compare_vendor_on_increment, &cp, 0);
	add_i(ls_compare_vendor_on_upgrade, &cp, 0);
	add_str(ls_a_behavior_ver, &cp, 0);
	add_i(ls_a_check_baddate, &cp, 0);
	add_i(ls_a_lkey_start_date, &cp, 0);
	add_i(ls_a_lkey_long, &cp, 0);
	add_i(ls_a_license_case_sensitive, &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf);



/*-
 *	Dump SERVERs
 */
	if (lm_job->line)
	{
	  LM_SERVER *s;
		for (s = lm_job->line->server; s; s = s->next)
		{
			cp = buf;
			add_key(LL_SERVER, &cp);
			add_str("SERVER", &cp, 0);
			add_str(s->name, &cp, 0);
			add_str(l_asc_hostid(lm_job, s->idptr), &cp, 0);
			strcat(cp, "\n");
			ls_append_repfile(buf);
		}
	}
/*-
 *	Dump Package Parents
 */
	for (c = lm_job->line; c; c = c->next)
	{
		if (c->package_mask & LM_LICENSE_PKG_ENABLE)
		{
			l_print_config(lm_job, c, buf);
			sprintf(buf2, "%x %s\n", LL_CONFIG, buf);
			ls_append_repfile(buf2);
		}
	}

/*-
 *	Dump Configs
 */
	for (c = lm_job->line; c; c = c->next)
	{
		int type = LL_PACKAGE;

		if (c->package_mask & LM_LICENSE_PKG_ENABLE)
			continue;
		l_print_config(lm_job, c, buf);
		sprintf(buf2, "%x %s\n", LL_CONFIG, buf);
		ls_append_repfile(buf2);
		if (c->parent_feat)
		{
			if (c->parent_feat->package_mask & LM_LICENSE_PKG_BUNDLE)
			{
				type = LL_PACKAGE_SUITE;
			}
			sprintf(buf, "%x %s %s %s %s\n", type,
					c->feature, c->code, c->parent_feat->feature,
					c->parent_feat->code);
			ls_append_repfile(buf);
		}
	}
/*
 *	Dump Groups
 */
	for (g = groups; g; g = g->next)
	{
	  char name[50], *n, *cp2;
		cp = buf3;
		add_key(LL_GROUP, &cp);
		add_str(g->name, &cp, 0);
		strcat(cp, "\n");
		ls_append_repfile(buf3);
		cp2 = g->list;
		while (*cp2)
		{
			while (*cp2 == ' ') cp2++;
			n = name;
			while (*cp2 && *cp2 != ' ')
				*n++ = *cp2++;
			cp = buf3;
			*n = 0;
			if (!*name) break;
			add_key(LL_GROUP_MEMBER, &cp);
			add_str(name, &cp, 0);
			strcat(cp, "\n");
			ls_append_repfile(buf3);
			if (*cp2) cp2++;
		}
	}

/*
 *	Dump Host Groups
 */
	for (g = hostgroups; g; g = g->next)
	{
	  char name[50], *n, *cp2;
		cp = buf3;
		add_key(LL_HOST_GROUP, &cp);
		add_str(g->name, &cp, 0);
		strcat(cp, "\n");
		ls_append_repfile(buf3);
		cp2 = g->list;
		while (*cp2)
		{
			while (*cp2 == ' ') cp2++;
			n = name;
			while (*cp2 && *cp2 != ' ')
				*n++ = *cp2++;
			cp = buf3;
			*n = 0;
			if (!*name) break;
			add_key(LL_GROUP_MEMBER, &cp);
			add_str(name, &cp, 0);
			strcat(cp, "\n");
			ls_append_repfile(buf3);
			if (*cp2)
				cp2++;
		}
	}
/*-
 *	Dump FEATURE_LISTs
 */
	for (f = ls_flist; f; f = f->next)
	{
/*
 *	fixed args
 */
		if (!f->feature || !*f->feature) continue;
		cp = buf3;
		add_key(LL_FEATURELIST, &cp);
		add_str(f->feature, &cp, 0);
		add_str(f->code, &cp, 0);
		add_str(f->version, &cp, 0);
		add_str(f->expdate, &cp, 0);
		add_i(f->nlic, &cp, 0);
/*
 *	optional args
 */
		if (f->vendor_def && *f->vendor_def)
			add_str(f->vendor_def, &cp, LL_FL_VENDOR_DEF);
		if(f->overdraft)
			add_i(f->overdraft, &cp, LL_FL_OVERDRAFT);
		if(f->sticky_dup)
			add_i(f->dup_select, &cp, LL_FL_DUP);
		if(f->linger)
			add_i(f->linger, &cp, LL_FL_LINGER);
		if(f->flags & LM_FL_FLAG_UNCOUNTED)
			add_i(1, &cp, LL_FL_UNCOUNTED);
		if(f->lowwater)
			add_i(f->lowwater, &cp, LL_FL_LOWWATER);
		if(f->timeout)
			add_i(f->timeout, &cp, LL_FL_TIMEOUT);
		if(f->res)
			add_i(f->res, &cp, LL_FL_RES);
		if(f->type_mask)
			add_i(f->type_mask, &cp, LL_FL_TYPE_MASK);
		if(f->user_based)
			add_i(f->user_based, &cp, LL_FL_USER_BASED);
		if(f->host_based)
			add_i(f->host_based, &cp, LL_FL_HOST_BASED);
		if(f->minimum)
			add_i(f->minimum, &cp, LL_FL_MINIMUM);
		strcat(cp, "\n");
		ls_append_repfile(buf3);

		for (p = f->pooled; p; p = p->next)
		{
			if (!strcmp(p->code, f->code))
			{
				continue; /*  P6413 only log the pooled ones */
			}
			key = ls_dict(p->code);
			cp = buf3;
			add_key(LL_POOL, &cp);
			add_i(key, &cp, 0);
			strcat(cp, "\n");
			ls_append_repfile(buf3);
		}
		dump_opts(f->opt, f->code);
		dump_opts(f->include, f->code);
		dump_opts(f->exclude, f->code);
	}

/*
 *	Dump INCLUDE
 */

	ls_lf(0); /* flush */
	ls_append_repfile("# END HEADER\n");
}

static
void
add_key(key, cp)
int key;
char **cp;
{
  char *ptr = *cp;

#ifdef NLM
	ThreadSwitch();
#endif /* NLM */

	sprintf(ptr, "%x", key);
	*cp += strlen(ptr);
}

static
void
add_str(str, cp, key)
char *str;
char **cp;
int key;
{
  DICTKEY d;
  char *ptr = *cp;

#ifdef NLM
        ThreadSwitch();
#endif /* NLM */

	if (str)
		d = ls_dict(str);
	else
		d = 0;
	if (key)
		sprintf(ptr, " %x%c%x", key, LL_EQUAL, d);
	else
		sprintf(ptr, " %x", d);
	*cp += strlen(ptr);
}
static
void
add_i(i, cp, key)
int i;
char **cp;
int key;
{
#ifdef NLM
        ThreadSwitch();
#endif /* NLM */

	if (key)
		sprintf(*cp, " %x%c%x", key, LL_EQUAL, i);
	else
		sprintf(*cp, " %x", i);
	*cp += strlen(*cp);
}

void
ls_replog_usage(
long handle,    /*- a (semi)unique handle for this client */
long brhandle,  /*- handle of one of the brothers of this client */
int what,       /*- what kind of transaction: (LL_USAGE_xxx: IN, OUT,
                 *  DENIED, QUEUED, DEQUEUED, UNSUPPORTED, INUSE, INQUEUE,
                    v1.2: USED, METERFAIL) */
int why,        /*- why it happened: (LL_REASON_xxx: INACTIVE, CLIENT_CRASH,
                 *  CLIENT_REQUEST, SHUTDOWN, USER_REMOVED, SERVER_CRASH,
                    v1.2: FIRST_DEC, PERIODIC_DEC, NO_COUNT */
char *user,
char *node,
char *display,
char *vendor_def,
char *feature,
char *lickey,   /*- feature 20-char key */
LS_POOLED *pooled, /*- pooled data */
char * version,
int nlicreq,            /*- requested number of licenses in this transaction */
int nlicact,            /*- actual number of licenses this change affects */
int reftime,            /*- reference time (e.g. checkout time on checkin) */
CLIENT_DATA *who,
int group,
unsigned int userflags, /*- can indicate linger-borrow */
int linger_seconds) 	/*- if linger borrow, borrow time */
{
  char buf[MAX_REPFILE_LEN]; /* approx */
  char *cp = buf;
  DICTKEY ckey;
  CONFIG *conf = (CONFIG *)0;
  int type = LL_USAGE;


	if (what == LL_USAGE_IN)	type = LL_CHECKIN;
	else if (what == LL_USAGE_DENIED) type = LL_DENIED;
	else if (what == LL_USAGE_UNSUPPORTED) type = LL_UNSUPPORTED;
	else if (what == LL_ULTIMATE_DENIAL)
		type = LL_ULTIMATE_DENIAL;
	add_key(type, &cp);

	if (type != LL_CHECKIN)
	{
		if (!who)
		{
			DLOG(("INTERNAL ERROR: client is null %s %d %s\n",
				__FILE__, __LINE__,
					feature ? feature : "null feature"));
			return;
		}
		ckey = ls_log_client(who, 0);
	}
	if (!strcmp(lickey, CONFIG_PORT_HOST_PLUS_CODE))
	{
	  CONFIG *pos = 0;
		conf = l_next_conf_or_marker(lm_job, feature, &pos, 0, 0);
		if (conf) 	lickey = conf->code;
		else 		lickey = feature;
	}

/*
 *	It turns out to be easier on the report reader side
 *	if we log the current usage level changes BEFORE
 *	the usage event itself
 */
	if (l_getattr(lm_job, END_USER_OPTIONS)!=END_USER_OPTIONS_VAL)
	{
		if (what == LL_USAGE_OUT) log_curr_usage(feature, lickey);
		return;
	}


	if (what != LL_USAGE_DENIED && what != LL_USAGE_UNSUPPORTED &&
		what != LL_USAGE_INUSE && what != LL_USAGE_INQUEUE)
		log_curr_usage(feature, lickey);



	switch(what)
	{
	case LL_USAGE_IN:
		add_i((int)handle, &cp, 0);
/*
 *		optional args
 */
		add_time(&cp);
		break;
	case LL_USAGE_DENIED:
		add_str(feature, &cp, 0);
		add_str(lickey, &cp, 0);
		add_i(ckey, &cp, 0);

/*
 *	optional args
 */
		add_time(&cp);
		if (((conf || (conf = ls_lickey_to_conf(feature, lickey))))
			&& l_compare_version(lm_job, conf->version, version))
			add_str(version, &cp, LL_USE_VERSION);
		if (nlicreq != 1)
			add_i(nlicreq, &cp, LL_USE_NLICREQ);
		if (vendor_def && *vendor_def)
			add_str(vendor_def, &cp, LL_USE_VENDOR_DEF);
		if (who->ckout_sernum)
			add_i(who->ckout_sernum, &cp, LL_USE_REQNUM);
		if (who->sn)
			add_str(who->sn, &cp, LL_USE_SN);
		break;

	case LL_ULTIMATE_DENIAL:
		add_str(feature, &cp, 0);
		add_str(lickey, &cp, 0);
		add_i(ckey, &cp, 0);

/*
 *	optional args
 */
		add_time(&cp);
		if (((conf || (conf = ls_lickey_to_conf(feature, lickey))))
			&& l_compare_version(lm_job, conf->version, version))
			add_str(version, &cp, LL_USE_VERSION);
		if (nlicreq != 1)
			add_i(nlicreq, &cp, LL_USE_NLICREQ);
		if (vendor_def && *vendor_def)
			add_str(vendor_def, &cp, LL_USE_VENDOR_DEF);
		if (who->ckout_sernum)
			add_i(who->ckout_sernum, &cp, LL_USE_REQNUM);
		if (who->sn)
			add_str(who->sn, &cp, LL_USE_SN);
		break;

	default: /* checkout, usually */
		add_i((int)handle, &cp, 0);
		add_i(ckey, &cp, 0);
		add_str(feature, &cp, 0);
		add_str(lickey, &cp, 0); /* this could be a feature name
					  * if there's no such feature */
/*
 *	optional args
 */
		add_time(&cp);
		if (what != LL_USAGE_OUT)
			add_i(what, &cp, LL_USE_WHAT);
		if (brhandle)
			add_i((int)brhandle, &cp, LL_USE_BROTHER);
		if ((conf = ls_lickey_to_conf(feature, lickey))
			&& l_compare_version(lm_job, conf->version, version))
			add_str(version, &cp, LL_USE_VERSION);
		if (nlicreq != 1)
			add_i(nlicreq, &cp, LL_USE_NLICREQ);
		if (nlicact != 1)
			add_i(nlicact, &cp, LL_USE_NLICACT);
		if (vendor_def && *vendor_def)
			add_str(vendor_def, &cp, LL_USE_VENDOR_DEF);
		if (who->ckout_sernum)
			add_i(who->ckout_sernum, &cp, LL_USE_REQNUM);
		if (userflags)
			add_i(userflags, &cp, LL_USE_USERFLAGS);
		if (linger_seconds)
			add_i(linger_seconds, &cp, LL_USE_LINGER_SECS);
	}
	if (why != LL_REASON_CLIENT_REQUEST && why != LM_MAXUSERS)
	{
		if (why < 0)
			add_i(-why, &cp, LL_USE_ERRNO);
		else
			add_i(why, &cp, LL_USE_WHY);
	}
	if (group) add_i(group, &cp, LL_USE_GROUP);

	add_cpu_usages(who, &cp);

	strcat(cp, "\n");

	ls_append_repfile(buf);

}

static
void
log_curr_usage(feature, lickey)
char *feature;
char *lickey;
{
  int num_users;
  int float_lic_used;
  int tot_lic_used;
  int num_lic;
  int q_cnt;
  char buf[MAX_REPFILE_LEN];
  char *cp = buf;
/*-
 *	get current usage level
 */
	if (!ls_get_info(feature, lickey, &num_users, &float_lic_used,
				&tot_lic_used, &num_lic, &q_cnt))
		return; /* failed */


	add_key(LL_CURR_USE_LEVEL, &cp);
	add_str(feature, &cp, 0);
	add_str(lickey, &cp, 0);
	add_i(tot_lic_used, &cp, 0);
/*
 *	optional args
 */
	if (tot_lic_used != float_lic_used)
		add_i(float_lic_used, &cp, LL_CURR_USE_LEVEL_FLOAT);
	if (tot_lic_used != num_users)
		add_i(num_users, &cp, LL_CURR_USE_LEVEL_USERS);
	if (q_cnt)
		add_i(q_cnt, &cp, LL_CURR_USE_LEVEL_Q);
	strcat(cp, "\n");
	ls_append_repfile(buf);
}

void
ls_replog_delete_client(client)
CLIENT_DATA *client;
{
  DICTKEY ckey;
  char buf[50], *cp = buf;

	if (l_getattr(lm_job, END_USER_OPTIONS)!=END_USER_OPTIONS_VAL)
		return;
	if (!(ckey = ls_log_client(client, LS_CLIENT_LOOKUP_ONLY)))
		return;
	add_key(LL_DEL_CLIENT, &cp);
	add_i(ckey, &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf);
	ls_log_client(client, LS_CLIENT_REMOVE);
}

DICTKEY
ls_log_client(client, what)
CLIENT_DATA *client;
int what;	/* LS_CLIENT_REMOVE or LS_CLIENT_FLUSH */
{
  char buf[MAX_REPFILE_LEN];

  char *cp = buf;
  typedef struct _client_list {
	CLIENT_DATA *c;
	DICTKEY key;
	struct _client_list *next;
	} CLIENT_LIST;
  static DICTKEY idx;
  static CLIENT_LIST *clients, *free_clients;
  CLIENT_LIST *cl, *sav = (CLIENT_LIST *)0, *next;
  DICTKEY kdisp;
  DICTKEY userkey;

	if (l_getattr(lm_job, END_USER_OPTIONS)!=END_USER_OPTIONS_VAL)
		return 0;
	for (cl = clients; cl; cl = next)
	{
		next = cl->next;
		if (what == LS_CLIENT_FLUSH)
		{
			free(cl);
			continue;
		}
		if (client == cl->c)
		{
			if (what == LS_CLIENT_REMOVE)
			{
				if (sav) 	sav->next = cl->next;
				else 		clients = cl->next;
/*
 *				Add to free list
 */
				cl->next = free_clients;
				free_clients = cl;
				return 0;
			}
			return cl->key;
		}
		sav = cl;
	}
	if (what == LS_CLIENT_REMOVE || what == LS_CLIENT_LOOKUP_ONLY)
		return 0;
	if (what == LS_CLIENT_FLUSH)
	{
		clients = 0;
		return 0;
	}
/*
 *	take it from the free list if we can
 */
	if (free_clients)
	{
		cl = free_clients;
		free_clients = cl->next;
	}
	else
	{
		cl = (CLIENT_LIST *)LS_MALLOC(sizeof(CLIENT_LIST));
	}

	memset(cl, 0, sizeof(CLIENT_LIST));
	kdisp = ls_dict(client->display);
	cl->c = client;
	cl->key = ++idx;
	if (sav) 	sav->next = cl;
	else		clients = cl;
	add_key(LL_CLIENT, &cp);
	userkey = ls_user(client);
	add_i(cl->key, &cp, 0);
	add_i(userkey, &cp, 0);
	add_i((int)client->pid, &cp, 0);
	add_i(kdisp, &cp, 0);
	add_i(client->handle, &cp, 0);
/*
 *	optional args
 */
	if (client->comm_version != COMM_NUMVER ||
		client->comm_revision != COMM_NUMREV)
	{
		add_i(client->comm_version, &cp, LL_CLIENT_COMMVER);
		add_i(client->comm_version, &cp, LL_CLIENT_COMMREV);
	}
	if (client->flexlm_ver != lm_job->code.flexlm_version ||
		client->flexlm_rev != lm_job->code.flexlm_revision)
	{
		add_i(client->flexlm_ver, &cp, LL_CLIENT_FLEXVER);
		add_i(client->flexlm_rev, &cp, LL_CLIENT_FLEXREV);
	}
	if (client->addr.transport && (client->addr.transport != LM_TCP))
	{
		add_i(client->addr.transport, &cp, LL_CLIENT_TRANSPORT);
		add_i(client->udp_timeout, &cp, LL_CLIENT_UDPTIMEOUT);
	}
	if (client->project)
		add_str(client->project, &cp, LL_CLIENT_PROJECT);
	if (client->capacity > 1)
		add_i(client->capacity, &cp, LL_CLIENT_CAPACITY);
	if (client->platform)
		add_str(client->platform, &cp, LL_CLIENT_PLATFORM);
	if (client->group_id != NO_GROUP_ID)
		add_i(client->group_id, &cp, LL_CLIENT_GROUP_ID);
	strcat(cp, "\n");
	ls_append_repfile(buf);
	return cl->key;
}
void
ls_log_switchto(filename, client)
char *filename;
CLIENT_DATA *client;
{

  char buf[MAX_CONFIG_LINE];
  char *cp = buf;

	add_key(LL_SWITCHTO, &cp);
	if (!client)
		add_str(filename, &cp, LL_SWTO_FILENAME);
	else
	{
		add_str(client->name, &cp, LL_SWTO_CLIENTNAME);
		add_str(client->node, &cp, LL_SWTO_NODE);
		add_str(client->display, &cp, LL_SWTO_DISPLAY);
		add_i(client->handle, &cp, LL_SWTO_HANDLE);
	}
	strcat(cp, "\n");
	ls_append_repfile(buf);
	ls_log_close_report(1);
}
void
ls_log_switchfrom(name)
char *name;
{
  char buf[MAX_REPFILE_LEN], *cp = buf;

	add_key(LL_SWITCHFROM, &cp);
	cp += strlen(cp);
	add_str(name, &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf);
}

int ls_log_pass = 0;


void
ls_replog_res_usage(licgroup, handle, what, restype, nlic)
char *licgroup;
int handle, what, restype, nlic;
{
  char buf[MAX_REPFILE_LEN];
  char *cp = buf;

	if (l_getattr(lm_job, END_USER_OPTIONS)!=END_USER_OPTIONS_VAL)
		return;

	add_key(LL_RES_USAGE, &cp);
	add_i(handle, &cp, 0);
	add_i(what, &cp, 0);
	add_i(restype, &cp, 0);

/*
 *	optional args
 */
	add_time(&cp);
	if (nlic > 1)
		add_i(nlic, &cp, LL_RES_USE_NLIC);
	if (licgroup && *licgroup)
		add_str(licgroup, &cp, LL_RES_USE_LICGROUP);
	strcat(cp, "\n");
	ls_append_repfile(buf);
}

void
ls_replog_error(buf)
char *buf;
{
  char buf2[MAX_REPFILE_LEN], *cp = buf2;
  DICTKEY id;
  int len = strlen(buf);

	if (buf[len-1] == '\n')
	{
		buf[len-1] = '\0';
	}
	id = ls_dict(buf);
	add_key(LL_ERROR, &cp);
	add_i(id, &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf2);
}
/*
 *	lickey_to_conf -- look up feature name from 20-char key
 */
CONFIG *
ls_lickey_to_conf(feature, lickey)
char *feature;
char *lickey;
{
  CONFIG *conf;
	for (conf = lm_job->line; conf; conf = conf->next)
	{
#ifdef NLM
	        ThreadSwitch();
#endif /* NLM */

		if (!strcmp(conf->code, lickey) &&
			!strcmp(conf->feature, feature))
		{
			break;
		}
	}
	return conf;
}

static
void
add_time(cpp)
char **cpp;
{
#define PRINT_HOURS (PRINT_TIME_EVERY_N_HOURS * 60 * 60)
  static long lasttime;
  static long last_print_full_time;
  long t = time(0);
  int dtime;
  char *cp = *cpp;
#ifdef NLM
        ThreadSwitch();
#endif /* NLM */

	*cp = '\0'; /* to be safe */

	dtime = t - lasttime;
	if ((ls_log_pass > 0 && (t - last_print_full_time) < PRINT_HOURS) )
	{
		ls_log_pass--;
	}
	else
	{
		ls_log_pass = PRINT_TIME_EVERY_N_EVENTS;
		last_print_full_time =  dtime = t;
	}
	if (dtime)
	{
		cp += strlen(cp);
		sprintf(cp, " %x%c%x", LL_TIME_DIFF, LL_EQUAL, dtime);
	}
	lasttime = t;
	*cpp += strlen(cp);
}
ls_replog_comment(str)
char *str;
{
  char *buf2 = LS_MALLOC(strlen(str) + 15);
  char *cp = buf2;

  DICTKEY key;

	key = ls_dict(str);
	add_key(LL_COMMENT, &cp);
	add_i(key, &cp, 0);
	strcat(cp, "\n");
	ls_append_repfile(buf2);
	free(buf2);
}
void
ls_replog_timestamp()
{
  char buf[50];
  char *cp = buf;
  extern FILE *ls_log_repfile;

	if (!ls_log_repfile) return;
	add_key(LL_TIMESTAMP, &cp);
	add_time(&cp);
	strcat(cp, "\n");
	ls_append_repfile(buf);
}

void
ls_replog_end()
{
  char buf[50], *cp = buf;

	add_key(LL_END, &cp);
	add_time(&cp);
	strcat(cp, "\n");
	ls_append_repfile(buf);
}
void
ls_replog_cksum(cksumbuf)
char *cksumbuf;
{
  VENDORCODE vc;
  char buf[50], *cp = buf;
  unsigned char sav[SEEDS_XOR_NUM];
  static int counter = 0;

	if (!*cksumbuf)
		return;
#ifdef NLM
        ThreadSwitch();
#endif /* NLM */

/*-
 *	We have to flush here, otherwise the reader doesn't know the
 *	exact spot where the cksumbuf ends.
 */
#if 1 /*0*/
	{
	  char buf[50];
	  char *bcp = buf;
		add_key(LL_COUNTER, &bcp);
		add_i(++counter, &bcp, 0);
		strcat(bcp, "\n");
		ls_lf(buf);
	}
#endif
	ls_lf(0);
	vc.data[0] = REPCODE1;
	vc.data[1] = REPCODE2;
	memcpy(sav, lm_job->SEEDS_XOR, sizeof(sav));
	memset(lm_job->SEEDS_XOR, 0, sizeof(lm_job->SEEDS_XOR));
	add_key(LL_CKSUM, &cp);

	lm_job->flags |= LM_FLAG_MAKE_OLD_KEY;
	/*strcpy(cksumbuf, "1234567890123456789012345678901234567890");*/
	sprintf(cp, " %s\n", l_string_key(lm_job, (unsigned char *)cksumbuf,
			strlen(cksumbuf), &vc, 0));
	lm_job->flags &= ~LM_FLAG_MAKE_OLD_KEY;
	memcpy(lm_job->SEEDS_XOR, sav, sizeof(sav));
	ls_lf(buf);
/*-
 *	We have to flush here, otherwise the reader doesn't know the
 *	exact spot where the cksumbuf ends.
 */
	ls_lf(0);
}
static
void
dump_opts(opt, code)
OPTIONS *opt;
char *code;
{
  OPTIONS *o;
  char buf3[MAX_REPFILE_LEN], *cp;
	for (o = opt; o; o = o->next)
	{
		cp = buf3;
		switch (o->type)
		{
		case INCLUDE: add_key(LL_INCLUDE, &cp); break;
		case EXCLUDE: add_key(LL_EXCLUDE, &cp); break;
		case DYNAMIC_RESERVE:	add_key(LL_DYNAMIC_RESERVE, &cp); break;
		case RESERVE: add_key(LL_RESERVE, &cp); break;
		case OPT_MAX: add_key(LL_MAX, &cp); break;
		default: continue;
		}
		add_i(o->type2, &cp, 0);
		add_str(o->name, &cp, 0);
		if (o->type2 == OPT_INTERNET)
		{
			add_i(o->inet_addr[0], &cp, LL_OPT_INTERNET);
			add_i(o->inet_addr[1], &cp, LL_OPT_INTERNET);
			add_i(o->inet_addr[2], &cp, LL_OPT_INTERNET);
			add_i(o->inet_addr[3], &cp, LL_OPT_INTERNET);
		}
		if (o->count)
		{
			add_i(o->count, &cp, LL_OPT_COUNT);
		}
		if (code)
		{
			add_str(code, &cp, LL_OPT_CODE); /* not
							really optional*/
		}


		strcat(cp, "\n");
		ls_append_repfile(buf3);
	}
}

extern int ls_cpu_usage_delta;

void
ls_replog_cpu_usage(client)
CLIENT_DATA *client;
{
  char buf[MAX_REPFILE_LEN];
  char *cp = buf;
  int i;
  int added = 0;
  DICTKEY ckey;


	for (i=0;i<4;i++)
	{
/*
 * 		ONLY LOG ONES THAT HAVE CHANGE DELTA, AND DO WITH
 *		LL_CPU_XXX for each one.  All optional.
 *		SPECIAL KLUDGE NOTE:  i + 1 == LL_CPU_USAGE_n
 *		Also, even numbers are seconds, odd are milliseconds
 */


		if ((client->curr_cpu_usage[i]  -
					client->last_logged_cpu_usage[i])
						>= ls_cpu_usage_delta)
		{
			if (!added)
			{
				added = 1;

				if (!(ckey = ls_log_client(client,
							LS_CLIENT_LOOKUP_ONLY)))
					return;
				add_key(LL_CPU_USAGE, &cp);
				add_i((int)ckey, &cp, 0);
			}
			add_i(client->curr_cpu_usage[i], &cp, (i + 1));
			client->last_logged_cpu_usage[i] =
					client->curr_cpu_usage[i];
		}
	}
	if (added)
	{
		/*
		 *		optional args
		 */
		add_time(&cp);
		strcat(cp, "\n");
		ls_append_repfile(buf);
	}
}
static
void
add_cpu_usages(client, cp)
CLIENT_DATA *client;
char **cp;
{
  int i;

	if (!client) return; /* possible core dump */
	for (i=0;i<4;i++)
	{
		if (client->curr_cpu_usage[i] >= ls_cpu_usage_delta)
		{
			add_i(client->curr_cpu_usage[i], cp,
				LL_USE_CPU_USAGE_1 + i);
		}
	}
}
void
ls_replog_msg_vol(period)
int period;
{
  char buf [MAX_CONFIG_LINE + 1];
  char *cp = buf;
  extern int ls_max_msgs_per_minute;
  extern int ls_msg_cnt;
#define SIX_HOURS (60 * 60 * 6)

	add_key(LL_MSG_VOL, &cp);
	add_i(ls_max_msgs_per_minute, &cp, 0);
	add_i(ls_msg_cnt, &cp, 0);
	if (period != SIX_HOURS) add_i(period, &cp, LL_MSG_VOL_PERIOD);
	strcat(cp, "\n");
	ls_append_repfile(buf);
}
#include "../src/l_strkey.c"
