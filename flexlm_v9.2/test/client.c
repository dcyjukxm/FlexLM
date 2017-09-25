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

/*****************************************************************************/
/*	
 *	Module: $Id: client.c,v 1.4 2003/01/13 22:46:12 kmaclean Exp $
 *
 *	Description:	This is a sample application program, to illustrate 
 *			the use of the Flexible License Manager.
 *			This is a more full-featured example.
 *
 *	M. Christiano
 *	2/18/88
 *	Last changed:  5/13/97
 *
 */

#include "lm_client.h" 
#include "lm_code.h"
#include "lm_attr.h"
#ifndef sun
#include <stdlib.h>
#endif
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#ifdef WIN32	 
#include <windows.h>  
#include <stdlib.h> 
#include <string.h>
#include <malloc.h>
#include <io.h>
#else
extern int errno;
#endif /* WIN32 */

#ifndef PC
#define LM_CALLBACK_TYPE
#endif

void LM_CALLBACK_TYPE quit();
void LM_CALLBACK_TYPE reconn(), LM_CALLBACK_TYPE r_done();

void usage();
void exit();
void wait_for_string();

#ifdef VMS
char *strcpy();
char *getenv();
#define time_t long
#endif

#define FEATURE "f1"
#define VERSION "1.0"

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2, 
				VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

static char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
				"Thursday", "Friday", "Saturday"} ;


LM_HANDLE *lm_job;

main(argc, argv)
int argc;
char *argv[];
{
  int rc;
  char string[200];
  LM_USERS *users;	/* The userlist data, if we are denied */
  CONFIG *conf;
  char feature[200];
  char * version = VERSION;
  int checkout_cnt = 1;
  int arg;
  int do_udp = 0;
  int do_queue = 0;
  int check_flag = LM_CO_NOWAIT;
  char *new_lf_path = (char *)NULL;
  char *old_lf_path;
	
	if (rc = lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &lm_job))
	{
		lc_perror(lm_job, "lc_init failed");
		exit(rc);
	}
	for (arg = 1; arg<argc; arg++)
	{
		if (!strcmp(argv[arg], "-c"))
		{
			arg++;
			if (lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, 
					(LM_A_VAL_TYPE) argv[arg]))
				lc_perror(lm_job, 
				"Error setting new license file");
			continue;
		}
		else if (isdigit(*argv[arg])) /*number of licenses to checkout*/
		{
			if (sscanf(argv[arg], "%d", &checkout_cnt) <=0)
				checkout_cnt = 1;
		}
		else if (!strcmp(argv[arg], "-udp"))
			do_udp = 1;
		
		else if (!strcmp(argv[arg], "-queue"))
		{
			do_queue = 1;
		}
		else usage();
	}
/*
 *	License manager parameter intialization (optional)
 */

	/*
	 *	For an example of FLEXlm callback routine on Windows and NT,
	 *	please refer to WINTEVW.CPP in WINTEST directory.
	 */
	lc_set_attr(lm_job, LM_A_USER_EXITCALL, (LM_A_VAL_TYPE) quit);
	lc_set_attr(lm_job, LM_A_USER_RECONNECT, (LM_A_VAL_TYPE) reconn);
	lc_set_attr(lm_job, LM_A_USER_RECONNECT_DONE, (LM_A_VAL_TYPE) 
							r_done);	
	if (do_udp) 
	{
		lc_set_attr(lm_job, LM_A_COMM_TRANSPORT, 
						(LM_A_VAL_TYPE)LM_UDP);
	}

/*
 *	Find out which feature to test
 */
	printf("Enter feature to checkout [default: \"%s\"]: ", FEATURE);
	gets(feature);
	if (*feature == '\0') strcpy(feature, FEATURE);
/*
 *	Try to check out a copy of this feature - note that
 *	the fifth parameter is our vendor-private encryption
 *	code.  This code can be assigned on a per-feature
 *	basis, if desired, or a single code can be assigned
 *	to all features.  The third parameter is the number
 *	of licenses to checkout, which can be used
 *	to charge more for usage on a more powerful CPU.
 *	The fourth parameter is a flag to indicate whether
 *	to wait for an available license.
 */
	rc = lc_checkout(lm_job, feature, version, checkout_cnt, 
				check_flag, &code, LM_DUP_NONE);

	if (rc == LM_MAXUSERS ||
		rc == LM_USERSQUEUED ||
	    	rc == LM_FEATQUEUE)
	{
	  struct tm *t;
#ifdef THREAD_SAFE_TIME
	  struct tm tst;
#endif

		printf("Maximum # users for \"%s\" reached.\n", feature);
		printf("Users are:\n");
/*
 *		Get the list of people using this feature and display
 */
		users = lc_userlist(lm_job, feature);
		if (!users) lc_perror(lm_job, "lm_userlist failed");
		else while (users)
		{
		    if (lm_isres(users->opts))
		    {
			printf("%s %s %s\n",
				    users->opts == BORROWEDLIC ?
					"License BORROWED by" :
					users->opts == UNKNOWNRES ?
					    "UNKNOWN RESERVATION TYPE for" :
					    "RESERVATION for",
				    users->opts == BORROWEDLIC ? "DAEMON on" :
				    users->opts == USERRES ? "USER" :
				    users->opts == HOSTRES ? "HOST" :
				    users->opts == DISPLAYRES ? "DISPLAY" :
				    users->opts == INTERNETRES ? "INTERNET":
				    "GROUP", users->name);
		    }
		    else if (*users->name == '\0')
		    {
/*
 *			This structure contains the total number of users
 *			and the server time -- ignore it.
 */
		    }
		    else
		    {
		      time_t x = users->time;

#ifdef THREAD_SAFE_TIME
			localtime_r(&x, &tst);
			t = &tst;
#else /* !THREAD_SAFE_TIME */
			t = localtime(&x);
#endif
			printf("%s at %s on %s (v%s), started on %s %d/%d at %d:%s%d"
				, users->name, users->node, users->display,
				users->version, days[t->tm_wday],
				t->tm_mon+1, t->tm_mday, t->tm_hour, 
				t->tm_min < 10 ? "0" : "", t->tm_min);
				if (users->opts == INQUEUE) 
						printf(" IN QUEUE");
				if (users->nlic > 1) 
					printf(", %d licenses\n", 
								users->nlic);
				else
					printf("\n");
		    }
		    users = users->next;
		}
		printf("\n");
		if (do_queue) 
		{
			printf("Waiting for feature to be available...\n");
			if (rc = lc_checkout(lm_job, feature, version, 
				checkout_cnt, LM_CO_WAIT, &code, LM_DUP_NONE))
			{
				lc_perror(lm_job, "client");	
				lc_free_job(lm_job);
				exit(rc);
			}
		}
	} 
	else if (rc)
	{
		printf("Checkout of \"%s\" failed\n", feature);
		lc_perror(lm_job, "client");	
				/* Print license manager error code */
		lc_free_job(lm_job);
		exit(rc);
	}


/*
 *	We got a copy of the feature for our use
 */
	printf("\n%s feature locked .... \n", feature);
/* 
 *	Actual applications code, to be licensed per-user 
 *
 *		.
 *		.
 *		.
 */
	printf("\n<cr> to exit\n");
	printf("E to exit without lc_checkin(): ");
	
	wait_for_string(string);

	/* system 5 signals interrupt this read */
	if (*string != 'E' && *string != 'e')
/*
 *		This call is optional for TCP, but it is recommended that
 *		you include it, since a license which uses UDP will not
 *		be returned until the daemon times it out if lc_checkin()
 *		is not called.
 */

		if (lc_checkin(lm_job, feature, 0))
			lc_perror(lm_job, "lm_checkin failed");

	lc_free_job(lm_job);
	return(0);
}


void
wait_for_string(char * string)
{
#ifndef WIN32  /* Unix Style Wait */
	errno = 0;	
	gets(string);
	while (errno == EINTR) 
	{
		errno = 0;
		gets(string);
	}
#else   /* Pc style Wait */
	while (!_kbhit())
	{
		lc_timer(lm_job);
	} 	

	gets(string);


#endif /* WIN32 */
}

void LM_CALLBACK_TYPE
reconn(f, num, total, interval)
char *f;
int num, total, interval;
{
	printf("\nLost connection for %s, reconnection attempt %d of %d."
							, f, num, total);
	printf(" (interval: %d secs.)\n", interval);
	printf("status for %s is %d\n", f, lc_status(lm_job, f));
}


void LM_CALLBACK_TYPE
r_done(f, num, total, interval)
char *f;
int num, total, interval;
{
	printf("\nRe-connected to server for %s after %d of %d retries\n"
							, f, num, total);
	printf(" (interval: %d secs.)\n", interval);
	printf("status for %s is %d\n", f, lc_status(lm_job, f));
}


void LM_CALLBACK_TYPE
quit(f)
char *f;
{
	printf("\nLost connection to server for %s, exiting...\n", f);
}


void
usage()
{
	printf("usage: %s [-c <license>] [#] [-udp] [-queue]\n", 
							"lmclient");
	printf("\t-c license file -- [default $LM_LICENSE_FILE or /usr/local/flexlm/licenses/license.dat\n");
	printf("\t                      or C:\\flexlm\\license.dat] \n");
	printf("\t# -- number of licenses to checkout [default 1]\n");
	printf("\t-udp -- Use UDP sockets [default TCP]\n");
	printf("\t-queue -- Queue this client if no licenses available\n");
	exit(1);
}

