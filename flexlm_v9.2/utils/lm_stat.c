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
 *	Module: $Id: lm_stat.c,v 1.33.4.3 2003/07/01 17:04:22 sluu Exp $
 *
 *	Description:  lmstat functions
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  12/10/98
 *
 */
#include <stdlib.h>
#include "lmutil.h"
#include "flex_utils.h"
#ifndef VMS
#define REPORT_LMGRD
#define MULTIPLE_DAEMONS
#ifndef NO_regular_expressions
char *re_comp();
#endif  /* !NO_regular_expressions */
#endif  /* !VMS */


int lmstat_main lm_args((int argc, char *argv[]));
void remove_dups lm_args(());
void lmstat_processing lm_args((LMGRD_STAT *));
void lmstat_usage lm_args(());
static void list lm_args((LMGRD_STAT *, char *, char *));
static int lmstat_process_args();
void print_date lm_args((long time));
void featinfo lm_args((char *f));
static char master_node[MAX_SERVER_NAME+1] = { 0 };
static int is_daemon lm_args(( char *));
static void do_serverinfo lm_args((LMGRD_STAT *));
#if 0
static int l_stat_conn lm_args((LM_HANDLE *, char *, COMM_ENDPOINT *, char *,
								char *));
#endif
/*
 *	Status request variables
 */
int active_only = 0;		/* Display only active licenses */
int license_pattern = 0;	/* license pattern specified */
int serverinfo = 0;		/* Display server info (DEFAULT) */
int oneserveronly = 0;		/* Display for only one server */
char *whichserver;		/* Which server to display */
int featureinfo = 0;		/* Display feature information */
int userinfo = 0;		/* Display user info */
int one_license_only = 0;	/* Display for one only */
char *which_license;		/* Which feature's licenses to display */
int daemoninfo = 0;		/* Display daemon info */
int onedaemononly = 0;		/* Display for one only */
char *whichdaemon;		/* Which daemon to display */


lmstat_main(argc, argv)
int argc;
char *argv[];
{
  int d;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  long t;
  LMGRD_STAT *lmgrds = 0; /* null terminated list of lmgrds */
  LMGRD_STAT *lp, *next;

	/*lm_set_attr(LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1); */
	lm_set_attr(LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)4); /* Default to 4 sec timeout */
	if ( lmstat_process_args(argc, argv) )
		return -1;

	fprintf(ofp, lmtext("Flexible License Manager status on "));
#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &d, &d, &t, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &d, &d, &t);
#endif
	print_date(t);

#ifdef PC
        if (!l_getenv(lm_job, "LM_TSUITE"))
        {
                fprintf(ofp, "[Detecting lmgrd processes...]\n");
                fflush(ofp);
        }
#endif
	lmgrds = l_lmgrds(lm_job, 0);
	if (!lmgrds)
	{
		fprintf(ofp, "Error getting status: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}

	for (lp = lmgrds; lp ; lp = next)
	{
		lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
		lc_set_attr(lm_job, LM_A_LICENSE_FILE,
					(LM_A_VAL_TYPE)lp->port_at_host);

		lmstat_processing(lp);
		next = lp->next;
		if (next)
		fprintf(ofp,
"\n----------------------------------------------------------------------------\n");
	}
	lc_free_lmgrd_stat(lm_job, lmgrds);
	return(0);
}

DAEMON *daemons;

void
lmstat_processing(lmgrd)
LMGRD_STAT *lmgrd;
{
  LM_SERVER *server = (LM_SERVER *) NULL;
  DAEMON *cur_daemon;
  CONFIG *conf;
  char **features, **curfeature;
	if (daemoninfo && !onedaemononly)    /* All DAEMONs == All features */
	{
		daemoninfo = 0;
		userinfo = 1;
		one_license_only = 0;
	}
/*
 *	Display license daemon status
 */

	server = lmgrd->server; /* l_master_list_from_job(lm_job); */

	if (server)
	{
		strncpy(master_node, server->name, MAX_SERVER_NAME);/* LONGNAMES */
	}
	if (serverinfo || !lmgrd->up) do_serverinfo(lmgrd);
	if (!lmgrd->up) return;
	remove_dups();
/*
 *	Display DAEMON info, in whatever flavor required:
 *
 *		- List of DAEMONs and their status if server info only
 *		- List of DAEMONs and their users if -S option
 */
	if (daemoninfo || serverinfo)
	{
#if 0
		if (server && server->commtype == LM_FILE_COMM)
		{
			if (!told_no_servers)
				fprintf(ofp, lmtext(
				"License file(s): %s (No license servers)\n"),
				lf->filename ? lf->filename :
				lic_files(lm_job));
				told_no_servers = 1;
		}
		else
#endif
		{
			daemons = l_cur_dlist(lm_job);
			if (daemoninfo && daemons)
			{
				fprintf(ofp, lmtext("DAEMONs in configuration file: "));
				for (cur_daemon = daemons; cur_daemon;
				cur_daemon = cur_daemon->next)
				{
					fprintf(ofp, "%s ", cur_daemon->name);
				}
				fprintf(ofp, "\n");
			}
			if (serverinfo && (!oneserveronly || !strcmp(master_node,
							whichserver)))
			{
				int s;
				int feat_idx;

				fprintf(ofp, lmtext("Vendor daemon status (on %s):\n\n"), master_node);
				features = lm_feat_list(LM_FLIST_ALL_FILES, dummy);

				if (features)
				{

					for (cur_daemon = daemons; cur_daemon;
					cur_daemon = cur_daemon->next)
					{
					  int rev;
					  int flexlmd = !strcmp(cur_daemon->name, "flexlmd");
					  CONFIG *pos;
					  int found_feature_for_daemon = 0;

						fprintf(ofp, "%10.10s", cur_daemon->name);
						fflush(stdout);
						for(feat_idx = 0; features[feat_idx]; feat_idx++)
						{
							pos = 0;
							conf = l_next_conf_or_marker(lm_job,
								features[feat_idx], &pos,
								0, cur_daemon->name);

							if (conf && ((flexlmd &&
								!is_daemon(conf->daemon)) ||
								!strcmp(cur_daemon->name,
								conf->daemon)))
							{
								found_feature_for_daemon = 1;
								s = l_connect(lm_job, server,
									cur_daemon->name,
									lm_job->daemon->commtype);
								rev = lm_job->daemon->comm_revision;
								if (rev <= 0) rev = 1;
								else if (rev < 3) rev = 2;
								else rev = 3;
								if (s >= 0)
								{
									fprintf(ofp,
										lmtext(": UP v%d.%d\n"),
										lm_job->daemon->ver,
										lm_job->daemon->rev);
										lm_disconn(0);
								}
								else
								{
									fprintf(ofp,
										": %s\n",
										lmtext(lc_errstring(lm_job)));
								}
								break;
							}
						}
						if (!found_feature_for_daemon)
						{
							fprintf(ofp, ": %s\n", lc_errstring(lm_job));
						}
					}
				}
				fprintf(ofp, "\n");
			}
			if (daemoninfo)
			{
				fprintf(ofp, lmtext("Users of features served by %s:\n"),
					whichdaemon);
				features = lm_feat_list(LM_FLIST_ALL_FILES, dummy);
				if (features)
				{
					for (curfeature = features; *curfeature; curfeature++)
					{
						conf = lm_get_config(*curfeature);
						if (conf && !strcmp(whichdaemon, conf->daemon))
							list(lmgrd, *curfeature, whichdaemon);
					}
				}
			}
		}
	}
/*
 *	Display feature usage info
 */
	if (userinfo)
	{
		if (one_license_only)
		{
			conf = lm_get_config(which_license);
			if (conf)
				list(lmgrd, which_license, conf->daemon);
			else
				fprintf(ofp, "Error getting feature, %s\n",
					lc_errstring(lm_job));
		}
		else
		{
			features = lm_feat_list(LM_FLIST_ALL_FILES, dummy);
			if (features)
			{
				fprintf(ofp, lmtext("Feature usage info:\n\n"));
				for (curfeature = features; *curfeature;
							curfeature++)
				{
/*
 *					If we aren't looking for a specified
 *					pattern, OR we are and this one matches,
 *					then list this feature.
 */
#ifndef NO_regular_expressions
					if (!license_pattern ||
						re_exec(*curfeature))
#endif
					{
						conf =
						     lm_get_config(*curfeature);
						if (conf)
							list(lmgrd, *curfeature,
								conf->daemon);
					}
				}
			}
		}
	}
	if (featureinfo)
	{

		fprintf(ofp,
	lmtext("NOTE: lmstat -i does not give information from the server,\n"));
		fprintf(ofp,
	lmtext("      but only reads the license file.  For this reason, \n"));
		fprintf(ofp,
	lmtext("      lmstat -a is recommended instead.\n\n"));
		fprintf(ofp, lmtext("Feature			Version	  # licenses    Expires		Vendor\n"));
		fprintf(ofp, lmtext("_______			_______	  __________    _______		______\n"));
		if (one_license_only)
			featinfo(which_license);
		else
		{
			features = lm_feat_list(LM_FLIST_ALL_FILES, dummy);
			if (features)
			{
				for (curfeature = features; *curfeature;
							curfeature++)
				{
					featinfo(*curfeature);
				}
			}
		}
	}
}

/*
 *	featinfo() - Print the information about a feature
 */
void featinfo(f)
char *f;
{
  CONFIG  *c, *pos = (CONFIG *)0;


	/*c = lc_get_config(lm_job, f);*/
	while (c = lc_next_conf(lm_job, f, &pos))
	{
/*
 *		skip enabling feature lines unless they're suites
 */
		if ((c->package_mask & LM_LICENSE_PKG_ENABLE) &&
				!(c->package_mask & LM_LICENSE_PKG_SUITEBUNDLE))
			continue;

		fprintf(ofp, "%-23.23s	%s	      %-4d	%s	%s\n",
			c->feature, c->version, c->users, c->date, c->daemon);
	}
}

/*
 *
 *	Function: list(feature)
 *
 *	Description: Performs the list command to the server.
 *
 *	Parameters:	(char *) feature
 *
 *	Return:		User list is displayed
 *
 *
 */

static char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"} ;
char sav_vendor[MAX_DAEMON_NAME + 1];

static
int
sCompareUsers(
	const void * pOne,
	const void * pTwo)
{
	LM_USERS * a = *((LM_USERS **)pOne);
	LM_USERS * b = *((LM_USERS **)pTwo);

	if(a && b)
	{
		if(a->opts == BUNDLERES && b->opts == BUNDLERES)
		{
			char	buffa[MAX_USER_NAME + MAX_SERVER_NAME + MAX_DISPLAY_NAME + 1] = {'\0'};/* LONGNAMES */
			char	buffb[MAX_USER_NAME + MAX_SERVER_NAME + MAX_DISPLAY_NAME + 1] = {'\0'};/* LONGNAMES */

			strcat(buffa, a->name);
			strcat(buffa, a->node);
			strcat(buffa, a->display);

			strcat(buffb, b->name);
			strcat(buffb, b->node);
			strcat(buffb, b->display);

			return strcmp(buffa, buffb);

		}
		else
		{
			if(a->opts == b->opts)
				return (strcmp(a->name, b->name));
			if(a->opts == BUNDLERES)
				return -1;
			if(b->opts == BUNDLERES)
				return 1;
			else
				return (strcmp(a->name, b->name));
		}
	}
	return 0;
}

static
LM_USERS *
sSortEntries(LM_USERS * pStart)
{
	int				iCount = 0;
	int				i = 0;
	LM_USERS *		pTemp = NULL;
	LM_USERS **		ppUsers = NULL;

	for(pTemp = pStart; pTemp; pTemp = pTemp->next)
	{
		iCount++;
	}

	/*
	 *	Allocate array to hold pointers and fill up array with data
	 */
	if(iCount)
	{
		ppUsers = (LM_USERS **)malloc(iCount * (sizeof(LM_USERS *)));
		if(ppUsers)
		{
			pTemp = pStart;
			for(i = 0; i < iCount; i++)
			{
				ppUsers[i] = pTemp;
				pTemp = pTemp->next;
			}
			/*
			 *	Now do the sort
			 */
			qsort((void *)(ppUsers), (size_t)iCount, sizeof(LM_USERS *), sCompareUsers);

			/*
			 *	Generate new list and return it.
			 */
			for(i = 0; i < (iCount - 1); i++)
			{
				(ppUsers[i])->next = ppUsers[i + 1];
			}
			(ppUsers[i])->next = NULL;
			pTemp = ppUsers[0];

			if(ppUsers)
			{
				free(ppUsers);
			}

			return pTemp;
		}

	}
	return NULL;
}

static
void
sOutputBundleResInfo(
	LM_USERS *	pUser,
	int			count,
	CONFIG *	pConf)
{
	char	szUHDV[10] = {'\0'};
	char	szName[MAX_USER_NAME + MAX_SERVER_NAME + MAX_DISPLAY_NAME + MAX_VENDOR_CHECKOUT_DATA + 1] = {'\0'};/* LONGNAMES */
	char	szPackage[MAX_USER_NAME + MAX_SERVER_NAME + MAX_DISPLAY_NAME + MAX_VENDOR_CHECKOUT_DATA + 1] = {'\0'};/* LONGNAMES */

	if(pUser)
	{

		/*
		 *	Set default package name
		 */
		strcpy(szPackage, "UNKNOWN");

		/*
		 *	Determine what we have here
		 */
		if(pUser->name[0] != '\0')
		{
			strcpy(szName, pUser->name);
			strcpy(szUHDV, "U");
		}
		if(pUser->node[0] != '\0')
		{
			if(szName[0] != '\0')
			{
				strcat(szName, ":");
			}
			strcat(szName, pUser->node);
			strcat(szUHDV, "H");
		}
		if(pUser->display[0] != '\0')
		{
			if(szName[0] != '\0')
			{
				strcat(szName, ":");
			}
			strcat(szName, pUser->display);
			strcat(szUHDV, "D");
		}
		if(pUser->vendor_def[0] != '\0')
		{
			if(szName[0] != '\0')
			{
				strcat(szName, ":");
			}
			strcat(szName, "*");	/*	Just put a wildcard to specify that its used */
			strcat(szUHDV, "V");
		}

		/*
		 *	Set package name.
		 */
		if(pConf)
		{
			if(pConf->package_mask & LM_LICENSE_PKG_BUNDLE && pConf->feature)
			{
				strcpy(szPackage, pConf->feature);
			}
			else if(pConf->package_mask & LM_LICENSE_PKG_COMPONENT &&
				pConf->parent_feat && pConf->parent_feat->package_mask & LM_LICENSE_PKG_BUNDLE &&
				pConf->parent_feat->feature)
			{
				strcpy(szPackage, pConf->parent_feat->feature);
			}
		}

		fprintf(ofp, "\t%d RESERVATION%s (%s) for PACKAGE %s, %s (%s/%d)\n",
				count, count > 1 ? "s" : "", szUHDV, szPackage, szName,
				pUser->ul_conf->server->name,
				pUser->ul_conf->server->port);
	}
}

static void
list(lmgrd, feature, vendor)
LMGRD_STAT *lmgrd;
char *feature;
char *vendor;
{
  char string[200];
  LM_USERS *users, *saveusers, savu;
  struct tm *t;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  CONFIG *header_conf = (CONFIG *) NULL, *conf;
  int printed_users = 0;
  int tot = 0;
  int usedlicenses = 0; /* P6719: enhancement to display used licenses */
  int printed = 0;
  char *hostname;
  int rescount;

	lm_job->lfptr = 0;
/*
 *	Get the list of people using this "feature"
 */

	conf = lc_get_config(lm_job, feature);
/*
 *	Correct for THIS_HOST
 */
	if ((conf->server->sflags & L_SFLAG_THIS_HOST) &&
		(hostname = lc_lic_where(lm_job)) &&
		(hostname = strchr(hostname, '@')))
	{
		strcpy(conf->server->name, hostname + 1);
	}
	if (!is_daemon(conf->daemon))
	{
		if (*sav_vendor)
		{
			if (!strcmp(sav_vendor, vendor))
				return; /* flexlmd doesn't have this daemon */
			*sav_vendor = '\0'; /* trying another daemon */
		}
	}
	if ((conf->package_mask & LM_LICENSE_PKG_ENABLE) &&
		!(conf->package_mask & LM_LICENSE_PKG_SUITEBUNDLE))
		return; /* it's an enabling feature, but not a suite */

	lm_job->lm_errno = 0;
	strcpy(lm_job->vendor, vendor);
	users = lc_userlist(lm_job, feature);
	if (lm_job->lm_errno == LM_NOSERVSUPP)
	{
		if (!*sav_vendor)
		{
			strcpy(sav_vendor, vendor);
			printf(
"flexlmd: Can't yet validate features for \"%s\" (including \"%s\")\n\n",
				vendor, feature);
		}
		lm_disconn(1);
		return;
	}
	sprintf(string, lmtext("Users of %s: "), feature ?
					feature : "?");
	if (users == 0)
	{
		if (lm_job->lm_errno == 0)
		{
			if (!active_only)
				fprintf(ofp, lmtext("%sNone\n"), string);
		}
		else
		{
			fprintf(ofp, lmtext("%sCannot get users of %s: %s\n"),
					string, feature ? feature : "?",
					lmtext(lc_errstring(lm_job)));
		}
		return;
	}

	saveusers = users;
	while (users)
	{
		if (*users->name == '\0' && users->opts != BUNDLERES)
		{
			tot += users->nlic;
		}

/* Enhancement P6719:  lmstat -a -c licfile now displays the number of used licenses in addition
 * to the number of issued licenses.
 */
		else
		  if (users->opts != INQUEUE) /* If the license is queued, do not include it in the usedlicenses count */
		     usedlicenses += users->nlic;

		users = users->next;
	}

	users = saveusers;
	memset(&savu, 0, sizeof(savu));
	rescount = 0;
	/*
	 *	Sort the entries
	 */
	users->next = sSortEntries(users->next);
	while (users)
	{
		if (users->ul_conf && (header_conf != users->ul_conf))
		{
			if (header_conf) fprintf(ofp, "\n");
			license_info(1, users->ul_conf);
			header_conf = users->ul_conf;
			printed_users = 1;
		}
		if (memcmp(&savu.name[0], &users->name[0], sizeof(savu) -
			((unsigned long)(&users->name[0]) - (unsigned long)(&users->next))))
		{
			if (*savu.name && lm_isres(savu.opts) || savu.opts == BUNDLERES)
			{
				if(savu.opts == BUNDLERES)
				{
					/*
					 *	Output message specific to this reservation.
					 */
					sOutputBundleResInfo(&savu, rescount, conf);
				}
				else
				{
					fprintf(ofp,
					"\t%d RESERVATION%s for %s %s (%s/%d)\n",
					rescount, (rescount > 1) ? "s" : "",
					savu.opts == USERRES ? lmtext("USER") :
					savu.opts == HOSTRES ? lmtext("HOST") :
					savu.opts == HOSTGROUPRES ? lmtext("HOST_GROUP") :
					savu.opts == DISPLAYRES ? lmtext("DISPLAY") :
					savu.opts == BORROWEDLIC ? lmtext("DAEMON on") :
					savu.opts == INTERNETRES ? lmtext("INTERNET") :
					savu.opts == PROJECTRES ? lmtext("PROJECTRES") :
					savu.opts == GROUPRES ? lmtext("PROJECTRES") :
								lmtext("UNKNOWN"),
					savu.name,
					savu.ul_conf->server->name,
					savu.ul_conf->server->port);
				}
			}
			memset(&savu, 0, sizeof(savu));
			rescount = 0;
		}
		if (!*users->name && users->opts != BUNDLERES) ;
		else if (lm_isres(users->opts))
		{
			if (!memcmp(&savu.name[0], &users->name[0], sizeof(savu) -
			((unsigned long)(&users->name[0]) - (unsigned long)(&users->next))))
			{
				rescount++;
			}
			else
			{
				memcpy(&savu, users, sizeof(savu));
				rescount = 1;
			}
		}
		else
		{
			time_t tt = users->time;

#ifdef THREAD_SAFE_TIME
			localtime_r(&tt, &tst);
			t = &tst;
#else /* !THREAD_SAFE_TIME */
			t = localtime(&tt);
#endif
			fprintf(ofp, "    %s %s",
				users->name, users->node);
			if (users->display[0])
				fprintf(ofp, " %s", users->display);
			if (users->vendor_def[0])
				fprintf(ofp, " %s", users->vendor_def);
			if (l_compare_version(lm_job, users->version,  "0.0"))
				fprintf(ofp, " (v%s)", users->version);
			if (users->ul_license_handle != 0)
				fprintf(ofp, " (%s/%d %d)",
						users->ul_conf->server->name,
						users->ul_conf->server->port,
						users->ul_license_handle);
			if (users->opts == INQUEUE)
			{
				fprintf(ofp, lmtext(" queued for %d license%s"),
					users->nlic, users->nlic > 1 ? "s":"");
			}
			else
			{
				fprintf(ofp, lmtext(", start %s %d/%d %d:%s%d"),
						days[t->tm_wday], t->tm_mon+1,
						t->tm_mday, /* t->tm_year, */
						t->tm_hour, t->tm_min < 10 ?
							"0" : "", t->tm_min);
				if (users->nlic > 1)
					fprintf(ofp, lmtext(", %d licenses"),
								users->nlic);
				if (users->linger > 0)
				{
					if (users->opts & LM_USERS_BORROW)
					{
						tt = users->time + users->linger;
#ifdef THREAD_SAFE_TIME
						localtime_r(&tt, &tst);
						t = &tst;
#else /* !THREAD_SAFE_TIME */
						t = localtime(&tt);
#endif

						fprintf(ofp,
					", borrow-return %s %d/%d %d:%s%d",
						days[t->tm_wday], t->tm_mon+1,
						t->tm_mday, /* t->tm_year, */
						t->tm_hour, t->tm_min < 10 ?
							"0" : "", t->tm_min);
					}
					else
					{
						fprintf(ofp, lmtext(" (linger: %ld)"),
								users->linger);
					}
				}
			}
			fprintf(ofp, "\n");
		}
		users = users->next;

		if (!printed && (!active_only || (users  && *users->name)))
		{
			printed = 1;
			if (tot == 0)
			{
				if (!conf->users)
					fprintf(ofp,
				"%s (Uncounted, node-locked)\n\n", string);
				else
					fprintf(ofp,
		"%s (Error: %d licenses, unsupported by licensed server)\n\n",
							string, conf->users);
			}
			else
				fprintf(ofp,
				lmtext(
				"%s (Total of %d %slicense%s issued;  Total of %d license%s in use)\n\n"),
						string,
						(tot < 0) ? 0 : tot,
#ifdef SUPPORT_METER_BORROW
				(conf->lc_type_mask & LM_TYPE_METER_BORROW) ?
							"borrowable " :
#endif /* SUPPORT_METER_BORROW */
							"",
							(tot == 1) ? "" : "s", usedlicenses, (usedlicenses == 1) ? "" : "s");


		}
	}
	if ((*savu.name && lm_isres(savu.opts)) || savu.opts == BUNDLERES)
	{
		if(savu.opts == BUNDLERES)
		{
			/*
			 *	Output message specific to this reservation.
			 */
			sOutputBundleResInfo(&savu, rescount, conf);
		}
		else
		{
			fprintf(ofp,
			"\t%d RESERVATION%s for %s %s (%s/%d)\n",
			rescount, (rescount > 1) ? "s" : "",
			savu.opts == USERRES ? lmtext("USER") :
			savu.opts == HOSTRES ? lmtext("HOST") :
			savu.opts == HOSTGROUPRES ? lmtext("HOST_GROUP") :
			savu.opts == DISPLAYRES ? lmtext("DISPLAY") :
			savu.opts == BORROWEDLIC ? lmtext("DAEMON on") :
			savu.opts == INTERNETRES ? lmtext("INTERNET") :
			savu.opts == GROUPRES ? lmtext("GROUP") :
			savu.opts == PROJECTRES ? lmtext("PROJECT") :
						lmtext("UNKNOWN"),
			savu.name,
			savu.ul_conf->server->name,
			savu.ul_conf->server->port);
		}
	}
	if (printed_users)
		fprintf(ofp, "\n");
}

void print_date(time)
long time;
{
  struct tm *t;
  time_t x = time;

	t = localtime(&x);
	fprintf(ofp, "%s %d/%d/%d %s%d:%s%d\n\n",
				days[t->tm_wday],
				t->tm_mon+1, t->tm_mday, t->tm_year + 1900,
				t->tm_hour < 10 ? "0" : "", t->tm_hour,
				t->tm_min < 10 ? "0" : "", t->tm_min);
}

/*
 *	lmstat_process_args(argc, argv)
 *
 *	Description: Processes the command-line arguments.
 *	RETURNS:		0 = success
 *
 */

static
lmstat_process_args(argc, argv)
int argc;
char *argv[];
{
  int noarg = 1;	/* Flag to indicate no option args are present */

	while (argc > 1)
	{
	  char *p = argv[1]+1;
		switch (*p)
		{
			case 'A':             /* 12/12/02: -A usage option is obsolete for all post v8.3c releases. */
				active_only = 1;
				/* Fall thru to 'a' case */
			case 'a':
				userinfo = 1;
				one_license_only = 0;
				serverinfo = 1;
				oneserveronly = 0;
				noarg = 0;	/* We have an arg */
				break;

			case 'c':
				if (argc > 2 && *argv[2] != '-')
				{
					argv++; argc--;

					lm_set_attr(LM_A_LICENSE_FILE,
						(LM_A_VAL_TYPE) argv[1]);
					lm_set_attr(LM_A_DISABLE_ENV,
							   (LM_A_VAL_TYPE)1);
				}
				break;

			case 'f':    /* Provides usage info about the specified (or all) feature(s). */
				userinfo = 1;
				if (argc > 2 && *argv[2] != '-')
				{
					argv++; argc--;
					one_license_only = 1;
					which_license = argv[1];
					noarg = 0;	/* We have an arg */
				}
				break;

			case 'i':      /*  Provides static feature-line info (version, # of licenses, 
						       expiration date, daemon name) from the license
						       file for the specified (or all) feature(s). */
				featureinfo = 1;
				noarg = 0;	/* We have an arg */
				if (argc > 2 && *argv[2] != '-')
				{
					argv++; argc--;
					one_license_only = 1;
					which_license = argv[1];
				}
				break;

#ifndef NO_regular_expressions

			case 'l':
				userinfo = 1;
				if (argc > 2 && *argv[2] != '-')
				{
				    char *errmsg;

				    argv++; argc--;
				    which_license = argv[1];
				    if (errmsg =
					(char *) re_comp(which_license))
				    {
					fprintf(ofp, lmtext("EXPRESSION ERROR: %s\n"),
									errmsg);
					return -1;
				    }
				    license_pattern = 1;
				    noarg = 0;	/* We have an arg */
				}
				break;
#endif

			case 's':	/* Provides status on all the license files listed
						   under VENDOR_LICENSE_FILE and LM_LICENSE_FILE
						   for the specified server. */

					/* VMS converts uppercase args to
					   lower!  But we don't use the
					   lowercase "s" option on VMS
					   anyway.  SO, just make sure it
					   stays before the Uppercase
					   "S" option, and we are OK */
#ifdef MULTIPLE_DAEMONS
				serverinfo = 1;
				if (argc > 2 && *argv[2] != '-')
				{
					argv++; argc--;
					oneserveronly = 1;
					whichserver = argv[1];
				}
				noarg = 0;	/* We have an arg */
				break;
#endif
			case 'S':      /* Provides usage information for features served by the 
						      specified vendor daemon. */
				daemoninfo = 1;
				if (argc > 2 && *argv[2] != '-')
				{
					argv++; argc--;
					onedaemononly = 1;
					whichdaemon = argv[1];
				}
				noarg = 0;	/* We have an arg */
				break;

			case 't':       /* Sets connection timeout to the timeout_value. */
				if (argc > 2)
				{
					argv++; argc--;
					lm_set_attr(LM_A_CONN_TIMEOUT,
						 (LM_A_VAL_TYPE)atoi(argv[1]));
				}
				break;

			case 'v':
				fprintf(ofp, "%s v%d.%d%s\n", myname,
					      lm_job->code.flexlm_version,
					      lm_job->code.flexlm_revision,
					      lm_job->code.flexlm_patch);
				break;


			default:
				lmstat_usage();
				return -1;
		}
		argc--; argv++;
	}
	if (noarg) serverinfo = 1;

	return 0;
}

void lmstat_usage()
{
	fprintf(ofp, lmtext("usage: lmstat\n"));
	fprintf(ofp, lmtext("\t[-a]                  (display everything)\n"));
	/* 12/12/02: -A usage option is obsolete for all post v8.3c releases. */
	/* fprintf(ofp, lmtext("\t[-A]               (list all active licenses)\n")); */
	fprintf(ofp, lmtext("\t[-c license_file]     (use \"license_file\" as license file)\n"));
	fprintf(ofp, lmtext("\t[-f [feature_name]]   (list usage info about specified (or all) feature(s))\n"));
	fprintf(ofp, lmtext("\t[-i [feature_name]]   (list info about specified (or all) feature(s) from\n"));
	fprintf(ofp, lmtext("\t                       the increment line in the license file)\n"));
#ifndef NO_regular_expressions
	fprintf(ofp, lmtext("\t[-l [regular expr.]]  (list users of matching licenses)\n"));
#endif
	fprintf(ofp, lmtext("\t[-S [DAEMON]]         (display all users of DAEMONs licenses)\n"));
#ifdef MULTIPLE_DAEMONS
	fprintf(ofp, lmtext("\t[-s [server_name]]    (display status of all license files on server node(s))\n"));
#endif
	fprintf(ofp, lmtext("\t[-t timeout_value]    (set connection timeout to \"timeout_value\")\n"));
	fprintf(ofp, lmtext("\t[-v]                  (display FLEXlm version, revision, and patch)\n"));
	fprintf(ofp, lmtext("\t[-help]               (prints this message)\n"));

}
/*
 *	remove_dups -- uses job's list -- remove duplicate
 *			codes IF they're not components
 */
void
remove_dups()
{
  CONFIG *conf, *next, *c, *n, *sav;
  int do_components = 1;

/*
 *	if we remove a parent_feat before a component, we have trouble,
 *	since the component refers to the parent_feat.  So, we remove
 *	only components first, then parents
 */

	while (do_components >= 0)
	{
		for (conf = lm_job->line; conf; conf = next)
		{
			next = conf->next;
			sav = conf;
			for (c = next; c; c = n)
			{
				n = c->next;
/*
 *			If it's a COMPONENT, check it's parent's code
 *			P1789
 */
				if ((!do_components &&
						!(c->package_mask &
						LM_LICENSE_PKG_COMPONENT) &&
					L_STREQ(c->code, conf->code) &&
					L_STREQ(c->feature, conf->feature))
						||
					((do_components &&
						c->package_mask &
						LM_LICENSE_PKG_COMPONENT) &&
						(conf->package_mask &
						LM_LICENSE_PKG_COMPONENT)
						&& c->parent_feat &&
							conf->parent_feat
						&& L_STREQ(c->feature,
							conf->feature)
						&& L_STREQ(c->parent_feat->code,
						conf->parent_feat->code)))
				{
/*
 *				It's a dup, remove it
 */
					sav->next = n;
					if (next == c) next = next->next;
					l_free_conf(lm_job, c);
				}
				else
					sav = c;
			}
		}
		do_components--;
	}
}
static
int
is_daemon(name)
char *name;
{
  DAEMON *d;
	for (d = daemons; d; d = d->next)
		if (!strcmp(d->name, name))
			return 1;
	return 0;
}

#if 0
static
char *
lic_files(job)
LM_HANDLE *job;
{
  static char buf [LM_MAXPATHLEN * 10];
  char *cp = buf;
  int i;
	for (i = 0; lm_job->lic_files[i]; i++)
	{
		sprintf(cp, " %s", lm_job->lic_files[i]);
		cp += strlen(cp);
		if (lm_job->lic_files[i+1])
		{
			sprintf(cp, "\n\t");
			cp += strlen(cp);
		}
	}

 	return buf;
}
#endif
static
void
do_serverinfo(lmgrd)
LMGRD_STAT *lmgrd;
{
  int s, c_rev;
  LM_SERVER *serv;
  char msg[LM_MSG_LEN+1];
  int t = lm_job->options->commtype == LM_UDP ? LM_TCP :
						lm_job->options->commtype;
#ifdef REPORT_LMGRD

	l_err_info_cp(lm_job, &lm_job->err_info, &lmgrd->e);
	if (!lmgrd->server)
	{
		fprintf(ofp, lmtext(
		"Error reading license file: %s\n%s\n\n"),
			lmgrd->license_paths, lmtext(lc_errstring(lm_job)));
		return;
	}

	fprintf(ofp, lmtext( "License server status: %s\n"),
		lmgrd->port_at_host);
	if (lmgrd->license_paths)
		fprintf(ofp, lmtext("    License file(s) on %s: %s:\n"),
			lmgrd->server->name,
			lmgrd->license_paths);
	fprintf(ofp, "\n");
	if (!lmgrd->up)
	{
		fprintf(ofp, "lmgrd is not running: %s\n", lc_errstring(lm_job));
		return;
	}

	for (serv = lmgrd->server; serv; serv = serv->next)
	{
	  COMM_ENDPOINT endpoint;

		if (oneserveronly && strcmp(whichserver, serv->name))
			continue;
		fprintf(ofp, "%10s", serv->name);
		fflush(stdout);
		c_rev = lm_job->daemon->our_comm_revision;
		lm_job->daemon->our_comm_revision = 0;
		l_conn_msg(lm_job, "", msg, t, 1);
		l_get_endpoint(lm_job, serv, "", t, &endpoint);

		lm_job->flags |= LM_FLAG_CONNECT_NO_THREAD;
		if ((s = l_basic_conn(lm_job, msg, &endpoint,

					serv->name, msg)) >= 0)
		{
			fprintf(ofp, lmtext(": license server UP"));
			if (msg[MSG_CMD] == LM_OK)
			{
				if (msg[MSG_OK_HOST])
				{
					strncpy(master_node,
						&msg[MSG_OK_HOST],
						MAX_SERVER_NAME);/* LONGNAMES */
				}
			}
			else
			{
				strncpy(master_node, serv->name,
						MAX_SERVER_NAME);/* LONGNAMES */
			}
			if (!strcmp(serv->name, master_node))
				fprintf(ofp, lmtext(" (MASTER)"));

			fprintf(ofp, lmtext(" v%d.%d\n"),
				lm_job->daemon->ver, lm_job->daemon->rev);
#ifndef PC
			shutdown(s, 2);
#endif
			network_close(s);
		}
		else
		{
			fprintf(ofp, ": %s\n", lmtext(lc_errstring(lm_job)));
		}
		lm_job->daemon->our_comm_revision = c_rev;
	}
	fprintf(ofp, "\n");
#endif
}

#if 0
static
int
l_stat_conn(job, msg, endpoint, hostname, resp)
LM_HANDLE *job;
char *msg;
COMM_ENDPOINT *endpoint;
char *hostname;
char *resp;
{
  unsigned short port = 0;
  int ret = -1;

        if ((endpoint->transport_addr.port & 0xffff) == 0xffff)
        {
                for (port = (unsigned short)LMGRD_PORT_START;
                                ret <0 && port <= job->port_end; port++)
                {
                        endpoint->transport_addr.port = ntohs(port);
                        ret = l_basic_conn(job, msg, endpoint, hostname, resp);
                }
        }
        else ret = l_basic_conn(job, msg, endpoint, hostname, resp);

	return ret;

}
#endif

