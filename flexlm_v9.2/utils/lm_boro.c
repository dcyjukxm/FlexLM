/******************************************************************************

	    COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_boro.c,v 1.5 2003/04/18 23:48:11 sluu Exp $
 *
 *	Description:  lmstat functions
 *
 *	D. Birns
 *	October, 1998
 *
 *	Last changed:  12/8/98
 *
 */
#include "lmutil.h"
#include "flex_file.h"

static int getlock_sw;
static int doincrement_sw;
static int serverinit_sw;
static int counter;
static char *device;
static int _howmany;
static char updatekey[100];
static char *vendor;
FILE *ifp;
static int prompt = 1;

lmborrow_main(argc, argv)
int argc;
char *argv[];
{
/**********************************************************************
 *	lmborrow -startupdate vendor counter [-device devname]
 *	lmborrow -update vendor counter _howmany update-key [-device devname]
 *	lmborrow -serverinit vendor [-device devname]
 *********************************************************************/

   int i;
   char *ifname;

	ifp = stdin;
 	for (i = 1; i < argc; i++)
	{
		if (l_keyword_eq(lm_job, argv[i], "-i"))
		{
			i++;
			if (i >= argc) break;
			if (L_STREQ(argv[i], "-"))
			{
				ifp = stdin;
				ifname = "stdin";
			}
			else if (!(ifp = l_flexFopen(job, argv[i], "r")))
			{
				fprintf(stderr, "Can't open %s: ", argv[i]);
				perror("");
				usage();
				exit(1);
			}
			else ifname = argv[i];
				
		}
		if (l_keyword_eq(lm_job, argv[i], "-q"))
			prompt = 0;
		else if (l_keyword_eq(lm_job, argv[i], "-startupdate"))
		{
			getlock_sw = 1;
			if (++i >= argc) { usage(); return(LM_BADPARAM); }
			vendor = argv[i];
			if (++i >= argc) { usage(); return(LM_BADPARAM); }
			sscanf(argv[i], "%d", &counter );
		}
		else if (l_keyword_eq(lm_job, argv[i], "-device"))
		{
			if (++i >= argc) { usage(); return(LM_BADPARAM); }
			device = argv[i];
		}
		else if (l_keyword_eq(lm_job, argv[i], "-serverinit"))
		{
			serverinit_sw = 1;
			if (++i >= argc) { usage(); return(LM_BADPARAM); }
			vendor = argv[i];
		}
		else if (l_keyword_eq(lm_job, argv[i], "-update"))
		{
		  char *up = updatekey;
		  	memset(updatekey, 0, sizeof(updatekey));
			doincrement_sw = 1;
			if (++i >= argc) { usage(); return LM_BADPARAM;}
			vendor = argv[i];
			if (++i >= argc) { usage(); return LM_BADPARAM;}
			sscanf(argv[i], "%d", &counter );
			if (++i >= argc) { usage(); return LM_BADPARAM;}
			sscanf(argv[i], "%d", &_howmany );
			if (++i >= argc) { usage(); return LM_BADPARAM;}
			for (;i < argc; i++)
			{
			  char *cp;

			  	for (cp = argv[i]; *cp; cp++)
					if (!isdigit(*cp)) break;
				if (*cp) break; /* non-numeric */
				strcpy(up, argv[i]);
				up += strlen(up);
				*up++ = ' ';
			}
			*--up = 0; /* remove last space */
		}
	}
	if ((doincrement_sw + getlock_sw + serverinit_sw) != 1)  
	{
		usage();
		return LM_BADPARAM;
	}
	if (doincrement_sw) 	return(doincrement());
	else if (getlock_sw)			return(getlock());
	else if (serverinit_sw)		return serverinit();
}

doincrement()
{

	if (l_borrow_update_server(lm_job, LM_BORROW_METER, device, vendor,
		counter, _howmany, updatekey))
	{
		fprintf(ofp, "lmborrow update failed: %s\n", 
			lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	fprintf(ofp, "lmborrow update succeeded, %d added to server\n", 
			_howmany);
	return 0;
}
	
getlock()
{
  char reply[100];
  char lock[100];
  char sn[100];

	if (prompt)
	{
		fprintf(ofp, 
	"Warning:  Once begun, the meter is locked, and other updates, and\n");
		fprintf(ofp, 
	"\tlicense returns prevented until the update is completed.\n");
		fprintf(ofp, 
	"\tAre you sure you want to do this (y/n)?  ");
		fgets(reply, 99, ifp);
		if ((*reply != 'y') && (*reply != 'Y'))
		{
			fprintf(stdout, "exiting...\n");
			exit(0);
		}
	}
	if (l_borrow_startupdate_server(lm_job, LM_BORROW_METER, device, 
			vendor, counter, lock, sn))
	{
		fprintf(ofp, "startupdate failed: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	fprintf(ofp, "Lock for Meter:%d %s is: %s\n", counter, sn, lock);
	fprintf(ofp, "\tGive this to your software provider\n");
	fprintf(ofp, "\tto obtain an update key. \n");
	fprintf(ofp, "\tUse \"lmborrow -update ...\" to update the server's meter\n");
	return 0;
}
serverinit()
{
	if (l_borrow_server_init(lm_job, LM_BORROW_METER, vendor, device))
	{
		fprintf(ofp, "lmborrow -serverinit failed: %s\n", 
			lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	return 0;
}
