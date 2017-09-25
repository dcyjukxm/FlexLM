/*****************************************************************************
 *
 *	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
 *	This software has been provided pursuant to a License Agreement
 *	containing restrictions on its use.  This software contains
 *	valuable trade secrets and proprietary information of 
 *	Macrovision Corporation. and is protected by law.  It may 
 *	not be copied or distributed in any form or medium, disclosed 
 *	to third parties, reverse engineered or used in any manner not 
 *	provided for in said License Agreement except with the prior 
 *	written authorization from Macrovision Corporation.
 *
 *****************************************************************************/
/*
 *
 *	Module: $Id: basic_tests.c,v 1.19 2003/03/12 18:34:31 sluu Exp $
 *
 *	Test suite for testing utilities and basic library functions
 *
 *	Tests performed:
 *
 *	1	Create license files
 *	2	Location of license file (thru environment or program)
 *	3	get_daemon - get DAEMON lines
 *	4	host - test hostid routines
 *	5	check_server - check server line from license file
 *	6	test_date - test expiration dates
 *	7	get/set attributes 
 *	8	test_misc - test miscellaneous functions
 *
 *	M. Christiano
 *	3/21/88
 *
 *	8/16/89 - Converted from C-shell to C.
 *	
 *	Last changed:  12/31/98
 *
 */
#include "lmachdep.h"
#ifdef PC
#include <winsock.h>
#include <pcsock.h>
#endif /* PC */
#include "lmclient.h"
#include "lm_attr.h"
#include "l_timers.h"
#include <stdio.h>
#include "code.h"
#include "testsuite.h"
#include "l_prot.h"

#undef CF
#define CF "license.dat"
#ifndef ANSI
extern char *strcpy(), *getwd();
#endif
static test_date();
static char * get_config lm_args((char *));
#ifdef VMS
extern void rm();
extern void unsetenv();
#endif /* VMS */
int get_daemon(), host();
void check_server(), test_attr();
LM_DATA;
extern cp lm_args(( char *, char *));
static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", 
						NULL};
static int monthlen[] = { 31,    28,    31,    30,    31,    30,
		    31,    31,    30,    31,    30,    31 };
		   


char *delim = 
    "---------------------------------------------------------------------\n";
FILE *ofp;

main(argc, argv)
int argc;
char *argv[];
{
  char gsipath[LM_MAXPATHLEN], opts[LM_MAXPATHLEN];
  long junk;
  char hostname[101];
  char *hostid, *chp;
  HOSTID *tid, *id;
  char *ptr;
  int day, month, year;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  char date[20];	/* xx-xxx-xx */
  char buf [MAX_CONFIG_LINE + 1]; 
  char *featstr;
  char *errors;
  FILE *fp;

	

	ofp = stdout;
	argv++;
	argc--;
#ifndef _MSC_VER /* UNIX */
	{
		char buf[200];
		sprintf(buf, "%s/.flexlmrc", getenv("HOME"));
		unlink(buf);
	}
#endif /* UNIX */
	for (; argc > 0; argc--, argv++)
	{
		if (!strcmp(*argv, "-o"))
		{
			argc--;
			argv++;
			if (!(ofp = fopen(*argv, "w")))
			{
				fprintf(stderr, "%s: ", *argv);
				perror("Can't open for writing");
				exit(1);
			}
			
		}
	}
		
	(void) lm_init("demo", &code, (LM_HANDLE **) &lm_job);
	lc_set_registry(lm_job, "LM_LICENSE_FILE", 0);
#ifdef THREAD_SAFE_TIME
	l_get_date(&day, &month, &year, &junk, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&day, &month, &year, &junk);
#endif
	year += 1900;
	lc_set_attr(lm_job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_PERROR_MSGBOX, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_PROMPT_FOR_FILE, (LM_A_VAL_TYPE)0);
	gethostname(hostname, 100);
	id = lc_gethostid(lm_job);
	hostid = l_asc_hostid(lm_job, id);
	chp = (char *)malloc(strlen(hostid)+1);
	strcpy(chp, hostid);
	hostid = chp;

/*
 * -----------------------------------------------------------------------------
 *  1 - Create license files
 */
	fprintf(ofp, delim);
	fprintf(ofp, "1. - Creating and testing license files\n");
	fprintf(ofp, delim);
	(void) getwd(gsipath, LM_MAXPATHLEN-2);
#ifdef VMS
	gsipath[strlen(gsipath)-1] = '\0';
#endif
	(void) strcat(gsipath, DEMOD);
	fprintf(ofp, "Creating license files for host id %s on %s\n", 
							hostid, hostname);
	/*cp(config, backup);*/
	sprintf(buf, "SERVER %s %s 2837\n\
DAEMON demo ./demo\n\
FEATURE f1 demo 1.0 01-jan-2010 9 0\n", hostname, hostid);
	if (lc_cryptstr(lm_job, buf, &featstr, &code, 0, 0, &errors))
	{
		fprintf(ofp, "cryptstr failed %s\n", errors ? errors : "");
		exit(1);
	}
	if (!(fp = fopen(CF, "w")))
	{
		perror(CF);
		exit(1);
	}
	fprintf(fp, featstr);
	fclose(fp);

	if (!(fp = fopen(CF2, "w")))
	{
		perror(CF);
		exit(1);
	}
	sprintf(buf, "SERVER %s %s 2837\n\
DAEMON demo ./demo\n\
FEATURE f1 demo 1.0 01-jan-2010 uncounted 0 HOSTID=08002b32b161 ISSUER=GSI\n",
			hostname, hostid);
	if (lc_cryptstr(lm_job, buf, &featstr, &code, 0, 0, &errors))
	{
		fprintf(ofp, "cryptstr failed %s\n", errors ? errors : "");
		exit(1);
	}
	fprintf(fp, featstr);
	fclose(fp);
	cp(CF, config);
	lc_free_job(lm_job);
	lc_init(0, "demo", &code, (LM_HANDLE **) &lm_job);
	lc_set_attr(lm_job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_PROMPT_FOR_FILE, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_PERROR_MSGBOX, (LM_A_VAL_TYPE)0);
/*
 *-----------------------------------------------------------------------------
 *	2	Location of license file (thru environment or program)
 */
	fprintf(ofp, delim);
	fprintf(ofp,  "2. - Testing license file location options.\n");
	fprintf(ofp, delim);

#if 0
	if (ptr = get_config(FEATURE))
	{
		fprintf(ofp, "lm_get_config FAILED to find standard config file\n");
		lm_perror(ptr);
	}
#endif
        lc_set_registry(lm_job, "LM_LICENSE_FILE", 0);
        lc_set_registry(lm_job, "DEMO_LICENSE_FILE", 0);

	rm(tempconfig);
	setenv("LM_LICENSE_FILE", tempconfig);
	l_flush_config(lm_job);
	if ((ptr = get_config(FEATURE)) == (char *) NULL)
	{
		fprintf(ofp, "lm_get_config FAILED (found ENV file when not there): %s\n", lc_errstring(lm_job));
	}

	cp(config, tempconfig);
	rm(config);
#if 0
	if (ptr = get_config(FEATURE))
	{
		fprintf(ofp, "lm_get_config FAILED to find file from environment  line %d\n", __LINE__);
		lm_perror(ptr);
	}
#endif
/*
 *	Make sure we can't find it if we disable LM_LICENSE_FILE
 */
	lm_set_attr(LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	if ((ptr = get_config(FEATURE)) == (char *) NULL)
	{
		fprintf(ofp, "lm_set_attr FAILED (found ENV file when DISABLED)\n");
		lm_perror("lm_get_config");
	}
	lm_set_attr(LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)0);

	unsetenv("LM_LICENSE_FILE");
	l_flush_config(lm_job);
	rm(config);
	rm(tempconfig);
	if ((ptr = get_config(FEATURE)) == (char *) NULL)
	{
		fprintf(ofp, "lm_get_config FAILED: No file anywhere, but it found one\n");
		lm_perror("");
	}

	cp(CF, altfile);
	l_flush_config(lm_job);
	lc_set_errno(lm_job, 0);
	lm_set_attr(LM_A_LICENSE_FILE, (LM_A_VAL_TYPE) altfile);
#if 0
	if (ptr = get_config(FEATURE))
	{
		fprintf(ofp, "lm_get_config FAILED: with lm_setup specified file line %d\n", __LINE__);
		lm_perror(ptr);
	}
#endif

	cp(altfile, config);
	rm(altfile);
	lm_set_attr(LM_A_LICENSE_FILE, (LM_A_VAL_TYPE) config); 
					/* Reset lm_setup() variables */

#if 0
/*
 *-----------------------------------------------------------------------------
 *	3	get_daemon - get DAEMON lines
 */
	fprintf(ofp, delim);
	fprintf(ofp,  "3. - get_daemon - get DAEMON lines\n");
	fprintf(ofp, delim);

	lm_set_attr(LM_A_LICENSE_FILE, (LM_A_VAL_TYPE) altfile); 
	if (get_daemon(good_daemon, (char *) NULL, (char *) NULL))
	{
		fprintf(ofp, "get_daemon returns error on good daemon line (%s)\n",
							good_daemon);
	}
	(void) strcpy(opts, "");
	if (get_daemon(good_daemon, "./demo", opts))
	{
		fprintf(ofp, "get_daemon returns error on good daemon line (%s)\n",
							good_daemon);
	}
	{
	  static char *list[] = {"x", "y", "z", "bad_daemon_line", NULL};
	  char **p;
	
		for (p = list; *p; p++)
		if (!get_daemon(*p, (char *) NULL, (char *) NULL))
		{
		    fprintf(ofp, "get_daemon returns success on unknown DAEMON %s\n",
									*p);
		}
	}
#endif
	lm_set_attr(LM_A_LICENSE_FILE, (LM_A_VAL_TYPE) config); 

/*
 *-----------------------------------------------------------------------------
 *	4	host - test hostid routines
 */
	fprintf(ofp, delim);
	fprintf(ofp,  "4. - host - test hostid routines\n");
	fprintf(ofp, delim);

	/*we have to re-get this info because lm_flush_config loses it*/
	id = lc_gethostid(lm_job);
	free(hostid);
	hostid = l_asc_hostid(lm_job, id);
	chp = (char *)malloc(strlen(hostid)+1);
	strcpy(chp, hostid);
	hostid = chp;

	{
	  char **p;
	  static char *list[] = { "123", "17001234", "15001324", 
				"12345678", "99887766", "0", "-1", NULL};

		for (p = list; *p; p++) (void) host(*p, 0);
	}
/*
 *	Test the alternate hostids on HP and VMS
 */
#ifdef VMS
	{
	  char *hid;
	  HOSTID *i;

		i = lm_getid_type(HOSTID_LONG);
		hid = l_asc_hostid(lm_job, i);
		host(hid, 1);
		i = lc_gethostid(lm_job);	/* Put back the correct type */
		/*(void) l_asc_hostid(lm_job, i);*/
	}
#endif
#ifdef HP
	{
	  char *hid;
	  HOSTID *i;

		i = lm_getid_type(HOSTID_ID_MODULE);
		hid = l_asc_hostid(lm_job, i);
		host(hid, 1);
		i = lc_gethostid(lm_job);	/* Put back the correct type */
		(void) l_asc_hostid(lm_job, i);
		i = lm_getid_type(HOSTID_ETHER);
		hid = l_asc_hostid(lm_job, i);
		host(hid, 1);
		i = lc_gethostid(lm_job);	/* Put back the correct type */
		(void) l_asc_hostid(lm_job, i);
	}
#endif

/*
 *	Now, test the correct one
 */
	(void) host(hostid, 1);
/*
 *-----------------------------------------------------------------------------
 *	5	check_server - get master list from config file
 */
	fprintf(ofp, delim);
	fprintf(ofp,  "5. - check_server - check server line from license file\n");
	fprintf(ofp, delim);

	id = lc_gethostid(lm_job);
	check_server(hostname, id);

/*
 * -----------------------------------------------------------------------------
 *	6	test_date - test expiration dates
 */
	fprintf(ofp, delim);
	fprintf(ofp, "6. - test_date - test expiration dates\n");
	fprintf(ofp, delim);
	{
	  int yesterday_month, tomorrow_month;	/* Don't run on 12-31 or 1-1 */
	  int yesterday, tomorrow, lastyear, nextyear;

		yesterday_month = tomorrow_month = month;
		yesterday = day - 1; 
		if (yesterday < 1) { yesterday_month--; 
					yesterday = monthlen[yesterday_month]; }
		tomorrow = day + 1; 
		if (tomorrow > monthlen[month]) { tomorrow = 1; 
							tomorrow_month++; }
		lastyear = year - 1; 
		nextyear = year + 1; 

/*
 *		Try some bad dates -- test_date should return 1
 */
		sprintf(date, "%d-%s-%d", yesterday, months[yesterday_month],
						    year);
		test_date(date, 1);
		sprintf(date, "%d-%s-%d", day, months[month], lastyear);
		test_date(date, 1);
/*
 *		Now some good dates -- test_date should return 0
 */
		sprintf(date, "%d-%s-%d", day, months[month], year);
		test_date(date, 0);
		sprintf(date, "%d-%s-%d", tomorrow, months[tomorrow_month],
						    		year);
		test_date(date, 0);
		sprintf(date, "%d-%s-%d", day, months[month], nextyear);
		test_date(date, 0);
	}

/*
 * -----------------------------------------------------------------------------
 *	7	set/get attributes
 */
	fprintf(ofp, delim);
	fprintf(ofp, "7. test get/set attributes (lm_get_attr/lm_set_attr)\n");
	fprintf(ofp, delim);

	test_attr();
/*
 * -----------------------------------------------------------------------------
 *	8	test_misc - test miscellaenous routines
 */
	fprintf(ofp, delim);
	fprintf(ofp, "8. - test_misc - test miscellaenous routines\n");
	fprintf(ofp, delim);
	{
	  CONFIG conf;
	  char featstr[200];
	  char newme[30];
	  char *whoiam, *getenv(); 
#if defined(TRUE_BSD) || defined(PC) || defined(BSDI) || defined(FREEBSD) || defined(MAC10)
	  char *getenv();
#else
	  char *cuserid();
#endif /* TRUE_BSD */
	  HOSTTYPE *ht, *lc_hosttype();
	  int i;

/*
 *		lm_username() test
 */
#if defined(TRUE_BSD) || defined(BSDI) || defined(FREEBSD) || defined(MAC10)
		whoiam = getenv("USER");
#else
#ifdef PC
		whoiam = getenv("USERNAME");
#else
		whoiam = cuserid(0);
#endif /* PC */
#endif /* TRUE_BSD */

		if (whoiam == (char *) NULL)
		{
#if 0
			fprintf(ofp, "NO getenv: lm_username(0) returns %s\n", 
								lm_username(0));
#endif
			whoiam = "NoName";
		}
		else if (strcmp(whoiam, lm_username(0)))
		{
			fprintf(ofp, "ERROR: lm_username(0) returns %s, we are %s\n",
					lm_username(0), whoiam);
		}
		strcpy(newme, whoiam);
		newme[2] = 'Q';
		newme[1] = 'X';
		lm_set_attr(LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)newme);
		if (strcmp(newme, lm_username(1)))
		{
			fprintf(ofp, "ERROR: lm_username(1) returns %s, we set %s\n",
					lm_username(1), newme);
		}
#ifndef VMS
/*
 *		lm_hostname() test
 */
		if (strcmp(hostname, lm_hostname(0)))
		{
			fprintf(ofp, "ERROR: lm_hostname(0) returns %s, we are %s\n",
					lm_hostname(0), hostname);
		}
		strcpy(newme, hostname);
		newme[2] = 'Q';
		newme[1] = 'X';
		lm_set_attr(LM_A_HOST_OVERRIDE, (LM_A_VAL_TYPE)newme);
		if (strcmp(newme, lm_hostname(1)))
		{
			fprintf(ofp, "ERROR: lm_hostname(1) returns %s, we set %s\n",
					lm_hostname(1), newme);
		}
#endif
		ht = lm_hosttype(1);
		fprintf(ofp, "lm_hosttype returns %d (%s), speed: %d/%d\n", 
			ht->code, ht->name, ht->flexlm_speed, ht->vendor_speed);
		
#ifndef VMS
/*
 *		Test lm_baddate() function
 */
		if (lm_baddate())
		{
			fprintf(ofp, "EEKS -- System clock set back!\n");
		}
#endif
		lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)0);
		if (strcmp(lc_errtext(lm_job, LM_NOCONFFILE),
				"Cannot find license file"))
			fprintf(ofp, "lc_errtext failed line %d\n", __LINE__);
		lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
		lc_set_attr(lm_job, LM_A_PROMPT_FOR_FILE, (LM_A_VAL_TYPE)0);
		if (lc_checkout(lm_job, "f1", "1.0", 0, LM_CO_NOWAIT, &code, 
					LM_DUP_NONE) != LM_BADPARAM)
			fprintf(ofp, "lc_checkout cnt=0 error %s line %d\n", lc_errstring(lm_job), __LINE__);
#if 0
		sprintf(date, "%d-%s-%d", day+2, months[month], year);
		sprintf(featstr, 
		"FEATURE f1 demo 1.0 %s 0 12345678901234567890 HOSTID=DEMO",
					date);
		if (!l_parse_feature_line(lm_job, featstr, &conf, 0)
#ifdef NO_MKTIME
			|| lc_expire_days(lm_job, &conf) < 0
			|| lc_expire_days(lm_job, &conf) > 2
#else
			|| lc_expire_days(lm_job, &conf) != 2
#endif
			)
			fprintf(ofp, "error lc_expire_days line %d\n", __LINE__);
		sprintf(date, "%d-%s-%d", day, months[month], year);
		sprintf(featstr, 
		"FEATURE f1 demo 1.0 %s 0 12345678901234567890 HOSTID=DEMO",
					date);
		if (!l_parse_feature_line(lm_job, featstr, &conf, 0)
#ifdef NO_MKTIME
			|| lc_expire_days(lm_job, &conf) > 0
#else
			|| lc_expire_days(lm_job, &conf) != 0
#endif
			)
			fprintf(ofp, "error lc_expire_days line %d got %d exp %d\n", 
				__LINE__,
				lc_expire_days(lm_job, &conf), 0);
#endif
	}
/*
 * -----------------------------------------------------------------------------
 *	Put things back to "normal"
 */
	fprintf(ofp, delim);
	fprintf(ofp, "Basic tests cleaning up -- all errors reported above\n");
	fprintf(ofp, delim);
	/*cp(backup, config);*/
	rm(CF);
	rm(CF2);
	/*rm(backup);*/
	exit(0);
}

/*
 * -----------------------------------------------------------------------------
 * -----------------------------------------------------------------------------
 * -----------------------------------------------------------------------------
 *
 *	Support routines
 *
 * -----------------------------------------------------------------------------
 * -----------------------------------------------------------------------------
 * -----------------------------------------------------------------------------
 */




static
char *
get_config(feature)
char *feature;
{
  CONFIG *x, *lc_get_config();

	x = lm_get_config(feature);
	if (x != (CONFIG *)NULL)
	{
		return(0);
	}
	else
	{
		return("lm_get_config");
	}
}
/*	
 *
 *	Function: get_daemon
 *
 *	Description: Tests lm_daemon.
 *
 *	Paramaters:	(char *) DAEMON name to retrieve.
 *
 *	Return:		0 - OK
 *			<> 0 - error.
 *
 *	M. Christiano
 *	3/21/88
 *
 *
 */

int
get_daemon(daemon, desired_path, desired_opts)
char *daemon;
char *desired_path;
char *desired_opts;
{
  char *lc_daemon();
  char opts[512];
  char *line;
  int port;

	line = lc_daemon(lm_job, daemon, opts, &port);
	if (desired_path && line && strcmp(desired_path, line))
	{
		fprintf(ofp, "DAEMON path for %s is %s, should be %s\n",
					daemon, line, desired_path);
	}
	if (desired_opts && line && strcmp(desired_opts, opts))
	{
		fprintf(ofp, "DAEMON options for %s is %s, should be %s\n",
					daemon, opts, desired_opts);
	}
	return (line == (char *) NULL);
}

/*
 *	host() - Check that we are on the correct host ID
 */
int
host(hostid, good)
char *hostid;
int good;
{
  HOSTID *id, *tid;
  int status;

	l_get_id(lm_job, &id, hostid);
	status = l_host(lm_job, id);
	if (good && status)
		fprintf(ofp, "l_host returns error on correct hostid (%s)\n", hostid);
	else if (!good && !status)
		fprintf(ofp, "l_host returns success on bad hostid (%s)\n", hostid);
	lc_free_hostid(lm_job, id);
	return(status);
}

/*
 *	check_server() - check the first server node in the license file
 */

void
check_server(hostname, hostid)
char *hostname;
HOSTID *hostid;
{
  LM_SERVER *x, *lc_master_list();
  int i;
  HOSTID *id;

	id = lc_copy_hostid(lm_job, hostid);
	x = lc_master_list(lm_job);
	if (x)
	{
		if (strcmp(hostname, x->name))
		{
			fprintf(ofp, "lm_master_list returns host %s, should be %s\n",
					x->name, hostname);
		}
		if (x->idptr->type == HOSTID_LONG && id->type == HOSTID_LONG &&
				id->hostid_value != x->idptr->hostid_value)
		{
			fprintf(ofp, "lm_master_list returns hostid %x, should be %x\n"
				, x->idptr->hostid_value, id->hostid_value);
		}
		else if (x->idptr->type == HOSTID_ETHER && 
			id->type == HOSTID_ETHER)
		{
			for (i = 0; i < ETHER_LEN; i++)
			{
				if (x->idptr->hostid_eth[i] != id->hostid_eth[i])
					break;
			}
			if (i < ETHER_LEN)
			{
				fprintf(ofp, "lm_master_list returns bad hostid:\n");
				fprintf(ofp, "return  should be (read downward)\n");
				for (i = 0; i < ETHER_LEN; i++)
				    fprintf(ofp, "%s%x	%s%x\n", 
					x->idptr->hostid_eth[i] < 16 ? "0" : "", 
					x->idptr->hostid_eth[i] & 0x0ff,
					id->hostid_eth[i] < 16 ? "0" : "", 
					id->hostid_eth[i] & 0x0ff);
			}
		}
		else if (x->idptr->type != id->type)
		{
			fprintf(ofp, "Host ID type mismatch: should be: %d, is: %d\n",
					id->type, x->idptr->type);
		}
	}
	lc_free_hostid(lm_job, id);
}


static
test_date(date, expected) 
char *date;
int expected;	/* Expected return from l_date() */
{ 
  int i;

	i = l_date(lm_job, date, L_DATE_EXPIRED);
	if (i && !expected)
	{
		fprintf(ofp, "%s is EXPIRED, but it shouldn't be\n", date);
	}
	else if (!i && expected)
	{
		fprintf(ofp, "%s is NOT EXPIRED, but it should be\n", date);
	}
}
/*
 *	Function:	test_attr()
 *
 *	Description:	Test lm_set_attr/lm_get_attr
 *
 *	M. Christiano
 *	5/3/90
 *
 */

typedef int (*PFI)();

#ifndef PC
#include <sys/time.h>
#endif /* OS2 */
#include "lm_attr.h"

#define S(t) \
  if (k) fprintf(ofp, "lm_set_attr(t..) call failed, returned %d (line %d)\n", k, __LINE__)
#define G(t) \
  if (k) fprintf(ofp, "lm_get_attr(t..) call failed, returned %d (line %d)\n", k, __LINE__)

#define test_vs_expected(x, a, t, expected) { int k; \
			k = lm_set_attr(x, (LM_A_VAL_TYPE) a); S(x); \
			k = lm_get_attr(x, (short *) &t); G(x); \
			if (t != expected) \
			fprintf(ofp, "lm_set_attr(x..) failed, set %d, got %d (line %d)\n", \
					a, t, __LINE__); }

#define test(x, a, t) { int k;\
			k = lm_set_attr(x, (LM_A_VAL_TYPE) a); S(x); \
			k = lm_get_attr(x, (short *) &t); G(x); \
			if (t != a) \
			fprintf(ofp, "lm_set_attr(x..) failed, set %d, got %d (line %d)\n", \
					a, t, __LINE__); }

#define stringtest(x, a, b, t) { int k; \
			k = lm_set_attr(x, (LM_A_VAL_TYPE) a); S(x); \
			k = lm_get_attr(x, (short *) t); G(x); \
			if (strcmp(t, a)) \
			fprintf(ofp, "lm_set_attr(x..) failed, set %s, got %s (line %d)\n", \
					a, t, __LINE__); \
			k = lm_set_attr(x, (LM_A_VAL_TYPE) b); S(x); \
			k = lm_get_attr(x, (short *) t); G(x); \
			if (strcmp(t,  b)) \
			fprintf(ofp, "lm_set_attr(x..) failed, set %s, got %s (line %d)\n", \
					b, t, __LINE__); }
void
test_attr()
{
  int i;
  short s;
  char c[100];
  PFI x, a = (PFI) 100, b = (PFI) 200;

#if 0
	test(LM_A_DECRYPT_FLAG, 1, s)
	test(LM_A_DECRYPT_FLAG, 0, s)
#endif
	test(LM_A_DISABLE_ENV, 0, s)
	test(LM_A_DISABLE_ENV, 1, s)
#ifdef VMS
	stringtest(LM_A_LICENSE_FILE, "sys$system:startup.com", "sys$manager:syconfig.com", c)
#endif
#ifdef PC
	stringtest(LM_A_LICENSE_FILE, "c:\\autoexec.bat", "c:\\config.sys", c)
#endif
#if !defined(PC) && !defined(VMS)
	stringtest(LM_A_LICENSE_FILE, "/etc/hosts", "/etc/passwd", c)
#endif
	test(LM_A_CRYPT_CASE_SENSITIVE, 0, s)
	test(LM_A_COMM_TRANSPORT, LM_UDP, s)
	test(LM_A_COMM_TRANSPORT, LM_TCP, s)
	test(LM_A_CRYPT_CASE_SENSITIVE, 1, s)
	test(LM_A_CHECK_INTERVAL, -1, i)
	test(LM_A_CHECK_INTERVAL, 100, i)
	test(LM_A_RETRY_INTERVAL, -1, i)
	test(LM_A_RETRY_INTERVAL, 100, i)
	test(LM_A_RETRY_COUNT, 0, i)
	test(LM_A_RETRY_COUNT, 1, i)
#ifndef PC
	test_vs_expected(LM_A_TIMER_TYPE, ITIMER_REAL, i, LM_REAL_TIMER)
	test_vs_expected(LM_A_TIMER_TYPE, ITIMER_VIRTUAL, i, LM_VIRTUAL_TIMER)
	test(LM_A_TIMER_TYPE, LM_REAL_TIMER, i)
	test(LM_A_TIMER_TYPE, LM_VIRTUAL_TIMER, i)
#endif
	test(LM_A_CONN_TIMEOUT, 0, i)
	test(LM_A_CONN_TIMEOUT, 1, i)
	test(LM_A_NORMAL_HOSTID, 0, s)
	test(LM_A_NORMAL_HOSTID, 1, s)
	test(LM_A_USER_EXITCALL, a, x)
	test(LM_A_USER_EXITCALL, b, x)
	test(LM_A_USER_RECONNECT, a, x)
	test(LM_A_USER_RECONNECT, b, x)
	test(LM_A_USER_RECONNECT_DONE, a, x)
	test(LM_A_USER_RECONNECT_DONE, b, x)
	test(LM_A_USER_CRYPT, a, x)
	test(LM_A_USER_CRYPT, b, x)
	stringtest(LM_A_USER_OVERRIDE, "foouser", "", c)
	stringtest(LM_A_HOST_OVERRIDE, "foohost", "", c)
#ifndef PC
	test(LM_A_PERIODIC_CALL, a, x)
	test(LM_A_PERIODIC_CALL, b, x)
	test(LM_A_PERIODIC_COUNT, 0, i)
	test(LM_A_PERIODIC_COUNT, 1, i)
	lm_set_attr(LM_A_PERIODIC_CALL, (LM_A_VAL_TYPE) 0);
#endif
	test(LM_A_NO_TRAFFIC_ENCRYPT, 0, s)
	test(LM_A_NO_TRAFFIC_ENCRYPT, 1, s)
	test(LM_A_USE_START_DATE, 0, s)
	test(LM_A_USE_START_DATE, 1, s)
	test(LM_A_MAX_TIMEDIFF, 1, i)
	test(LM_A_MAX_TIMEDIFF, 0, i)
	test(LM_A_ALLOW_SET_TRANSPORT, 0, i)
	test(LM_A_ALLOW_SET_TRANSPORT, 1, i)
	test(LM_A_CHECKOUTFILTER, a, x)
	test(LM_A_CHECKOUTFILTER, b, x)
	test(LM_A_CHECK_BADDATE, 0, s);
	test(LM_A_CHECK_BADDATE, 1, s);
	test(LM_A_LKEY_START_DATE, 0, i);
	test(LM_A_LKEY_START_DATE, 1, i);
	test(LM_A_LKEY_LONG, 0, i);
	test(LM_A_LKEY_LONG, 1, i);
	test(LM_A_LONG_ERRMSG, 0, i);
	test(LM_A_LONG_ERRMSG, 1, i);
	test( LM_A_LICENSE_CASE_SENSITIVE, 0, i);
	test( LM_A_LICENSE_CASE_SENSITIVE, 1, i);
	test( LM_A_PC_PROMPT_FOR_FILE, 0, i);
	test( LM_A_PC_PROMPT_FOR_FILE, 1, i);
#ifdef VMS
	test(LM_A_EF_1, 0, i)
	test(LM_A_EF_1, 1, i)
	test(LM_A_EF_2, 0, i)
	test(LM_A_EF_2, 1, i)
	test(LM_A_EF_3, 0, i)
	test(LM_A_EF_3, 1, i)
#endif
}
