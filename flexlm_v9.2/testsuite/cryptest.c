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
 *	Module: $Id: cryptest.c,v 1.29 2003/06/12 16:57:18 sluu Exp $
 *
 *	Function: crypt_test()
 *
 *	Description: Test lc_crypt()
 *
 *	Parameters:	None
 *
 *	Return:		None - results printed to stdout
 *
 *	M. Christiano
 *	1/31/90
 *
 *	Last changed:  11/2/98
 *
 */


#include "lmprikey.h"
#include "lmclient.h"
#include "lm_code.h"
#undef ENCRYPTION_SEED1
#undef ENCRYPTION_SEED2
#undef ENCRYPTION_SEED3
#undef ENCRYPTION_SEED4
#undef LM_SIGN_LEVEL
#include "code.h"
#include "lm_attr.h"
#include <stdio.h>
#include "testsuite.h"
#include "l_prot.h"
FILE *ofp;
#ifdef ANSI
#include <stdlib.h>
#endif

void process_args lm_args((int,char **));
void version_3_test();
void version_3plus_test();
void version_4_test();
void version_5_test();
void version_6_test();
void version_7_test();
void ver6uniq_test();
int ver6uniq;
void v3_attr();
void v3_crypt lm_args((char *));
void v3_crypt1 lm_args((int));
void v4_cryptstr lm_args((int, char *));
void increment lm_args((lm_noargs));
void upgrade lm_args((lm_noargs));
void user_crypt_filter_gen lm_args(( LM_HANDLE *, char *, int ));
/*extern void stkeynew lm_args(( LM_HANDLE *, char *, int ));*/
/*extern void stkeynew_gen lm_args(( LM_HANDLE *, char *, int ));*/
int v7_1;
int v7_1_print;
int newkey;

LM_HANDLE *lm_job;

static LM_SERVER *shuffle();
#if 0
#ifdef LM_RESERVED_PACKAGE /* version 4 */
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2,
	VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5
	);
#else
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2,
	VENDOR_KEY3, VENDOR_KEY4);
#define lc_crypt(a, b, c, d) l_crypt(b, c, d)
#define VERSION3_ONLY
#endif
#endif

#define LICLIMIT 20
/*
 *	Dummy data for the tests
 */
static long codes[] = {0x12345678, 0xaabbccdd, 0x00aa00bb, 0};
static int fullyears[] = {89, 90, 91, 92, 0};
static int partyears[] = {90, 91, 0};
static int fulldays[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 
			   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
			   20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			   30, 31, 0};
static int partdays[] = {1, 10, 15, 20, 30, 31, 0};
static char *fullmonths[] = {"jan", "feb", "mar", "apr", "may", "jun", 
		    "jul", "aug", "sep", "oct", "nov", "dec", ""};
static char *partmonths[] = {"jan", "mar", "apr", "jun", 
		    "jul", "sep", "oct", "dec", ""};
static char *fullfeats[] = {"f1", "longfeaturename", "30characternameisthisonehere..", ""};
static char *partfeats[] = {"f1", "LongFeature", ""};
static int *years = fullyears;
static char **months = fullmonths;
static char **feats = fullfeats;
static int *days = fulldays;
static int verbose = 0;
static unsigned long numkeys = 1000000000;	/* How many keys to produce */
static int varycodes = 0;		/* Vary codes only */
static int varyorder = 0;		/* Vary the sort order of HOSTIDs */
static int varyhostids = 0;		/* Vary hostids only */
static int version_3 = 0;
static int version_3plus = 0;
static int version_4 = 0;
static int version_5 = 0;
static int version_6 = 0;
static int version_7 = 0;
static unsigned char eth[] = {
			 0, 8, 0x20, 0x0a, 0x34, 0xac ,
			 0, 8, 0x20, 0x5a, 0x34, 0xbc ,
			 0, 8, 0x2f, 0x01, 0x04, 0xcc ,
			 0, 8, 0x20, 0x0a, 0x30, 0xdc ,
			 0, 8, 0x20, 0x0c, 0x54, 0xec ,
			 0, 8, 0x20, 0x0b, 0x24, 0xfc ,
			 0, 8, 0x20, 0x0d, 0x34, 0xfc ,
			 0, 8, 0x20, 0x0e, 0x44, 0xfc ,
			 0, 8, 0x20, 0x1a, 0x54, 0xfc ,
			 0, 8, 0x20, 0x03, 0x34, 0x1c ,
			 0, 8, 0x20, 0x09, 0x34, 0xfc ,
			 0, 8, 0x20, 0x0a, 0x34, 0x3c ,
			 0, 8, 0x20, 0xaa, 0x34, 0xfc ,
			 0, 0, 0, 0, 0, 0 };
#define CRYPT	fprintf(ofp, "%s\n", lc_crypt(lm_job, &conf, startdate, &vcode)); 

VENDORCODE vcode;
HOSTID hid;
char *startdate = "ABCD";

LM_KEY_API

void
main(int argc, char * argv[])
{
  char **fp;
  long *cp;
  int *yp;
  char **mp;
  int i, numlic;
  int *day;
  char date[20];
  unsigned long tmpvcode;
  long keys = 0;
  CONFIG conf;
  int rc;

	ofp = stdout;
	if ((rc = lc_init(0, VENDOR_NAME, &code, &lm_job)) && rc != LM_DEMOKIT)
	{
		fprintf(ofp, "FLEXlm init failed with %d\n", rc);
		exit(1);
	}
	lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER,
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V5_1); 
	if (getenv("P3130"))
		lc_set_attr(lm_job, LM_A_INTERNAL1, (LM_A_VAL_TYPE)1);  
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);  

	x_flexlm_newid();
	(void) memset(&vcode, 0, sizeof (vcode));
	vcode.data[0] = codes[0];
	vcode.data[1] = 0xab673201;
	if (argc < 2)
	{
		fprintf(ofp, "usage: %s -f (full)\n", argv[0]);
		fprintf(ofp, "                  -s (short)\n");
		fprintf(ofp, "                  -h (test hostids)\n");
		fprintf(ofp, "                  -k (test keys)\n");
		fprintf(ofp, "                  -v (verbose)\n");
		fprintf(ofp, "                  -c nnn (produce only nnn keys)\n");
		fprintf(ofp, "                  -3 (v3.0 features only, with lc_crypt/l_crypt)\n");
		fprintf(ofp, "                  -3+ (v3.0 features only, with lc_cryptstr)\n");
		exit(1);
	}
	process_args(argc, argv);
	if (version_3 || version_4 | version_3plus | version_5 | version_6 |version_7|ver6uniq | v7_1) 
	{
		memset(&vcode, 0, sizeof(vcode));
		memcpy(&vcode, &code, sizeof(vcode));
#ifndef VERSION3_ONLY
		vcode.data[0] ^= VENDOR_KEY5;
		vcode.data[1] ^= VENDOR_KEY5;
#endif
		if (version_3) version_3_test(); 
		if (version_3plus) version_3plus_test(); 
		if (version_4) version_4_test(); 
		if (version_5) version_5_test(); 
		if (version_6) version_6_test(); 
		if (ver6uniq) ver6uniq_test(); 
		if (version_7) version_7_test(); 
		if (v7_1) v7_1test(); 
		exit(0);
	}
		
	(void)memset(&conf, 0, sizeof(conf));
	l_config(&conf, CONFIG_FEATURE, "feature", "1.0", "", "14-jul-92", 
					3, "vendor string goes here", NULL);
	memset(&hid, 0, sizeof(hid));
	conf.idptr = &hid;
	conf.idptr->type = HOSTID_LONG;
	conf.idptr->override = 0;
	conf.idptr->hostid_value = 0x1200ab46;
	if (varycodes)
	{
		for (tmpvcode = 0; tmpvcode < numkeys; tmpvcode++)
		{
			vcode.data[0] = tmpvcode;
			CRYPT;
			/*fprintf(ofp, "%s\n", lc_crypt(lm_job, &conf, startdate, &vcode));*/
		}
	}
	else if (varyhostids)
	{
	  unsigned char *e;
	  unsigned char *f;
	  unsigned char *g;
	  LM_SERVER hostlist[3];
	  HOSTID hida[3];

		memset(hostlist, 0, sizeof(hostlist));
		memset(&hida[0], 0, sizeof(hida));
		hostlist[0].idptr = &hida[0];
		hostlist[1].idptr = &hida[1];
		hostlist[2].idptr = &hida[2];
		hostlist[0].idptr->override = 0;
		hostlist[1].idptr->override = 0;
		hostlist[2].idptr->override = 0;
		hostlist[0].idptr->type = HOSTID_ETHER;
		hostlist[1].idptr->type = HOSTID_ETHER;
		hostlist[2].idptr->type = HOSTID_ETHER;
		hostlist[0].next = (LM_SERVER *) NULL;

		if (conf.lc_vendor_def) free(conf.lc_vendor_def);
		(void)memset(&conf, 0, sizeof(conf));

		l_config(&conf, CONFIG_FEATURE, "feature", "1.0", "", 
					"14-jul-92", 3, 
					"vendor string goes here", hostlist);
/*
 *		Single ethernet hostids
 */
		for (e = eth; 
			e[0] || e[1] || e[2] || e[3] || e[4] || e[5]; 
				e += ETHER_LEN)
		{
			for (i=0; i<ETHER_LEN; i++)
				hostlist[0].idptr->id.e[i] = e[i];
			CRYPT;
			/*fprintf(ofp, "%s\n", lc_crypt(lm_job, &conf, startdate, &vcode));*/
		}
/*
 *		Three ethernet hostids
 */
		shuffle(hostlist, 0, 1, 2);
		for (e = eth; 
			e[0] || e[1] || e[2] || e[3] || e[4] || e[5]; 
				e += ETHER_LEN)
		{
		  for (i=0; i<ETHER_LEN; i++)
				hostlist[0].idptr->id.e[i] = e[i];
		  for (f = e + ETHER_LEN; 
			f[0] || f[1] || f[2] || f[3] || f[4] || f[5]; 
				f += ETHER_LEN)
		   {
		      for (i=0; i<ETHER_LEN; i++)
				hostlist[1].idptr->id.e[i] = f[i];
		      for (g = f + ETHER_LEN; 
			g[0] || g[1] || g[2] || g[3] || g[4] || g[5]; 
				g += ETHER_LEN)
		      {
			for (i=0; i<ETHER_LEN; i++)
				hostlist[2].idptr->id.e[i] = g[i];
			CRYPT
			/*fprintf(ofp, "%s\n", lc_crypt(lm_job, &conf, startdate, &vcode));*/
		      }
		   }
		}
	}
	else if (varyorder)
	{
/*
 *		Vary the order of hostids to check encryption sorting
 */
	  unsigned char *e;
	  unsigned char *f;
	  unsigned char *g;
	  HOSTID hida[3];
	  LM_SERVER hostlist[3];

		if (conf.lc_vendor_def) free(conf.lc_vendor_def);
		(void)memset(&conf, 0, sizeof(conf));
		(void)memset(hostlist, 0, sizeof(hostlist));
		(void)memset(hida, 0, sizeof(hida));
		l_config(&conf, CONFIG_FEATURE, "feature", "1.0", "", "14-jul-92",
					3, "vendor string", NULL);
		memset(&hid, 0, sizeof(HOSTID));
		conf.idptr = &hid;
		conf.idptr->type = HOSTID_LONG;
		conf.idptr->override = 0;
		conf.idptr->hostid_value = 0x1200ab46;


		hostlist[0].idptr = &hida[0];
		hostlist[1].idptr = &hida[1];
		hostlist[2].idptr = &hida[2];
		hostlist[0].idptr->override = 0;
		hostlist[1].idptr->override = 0;
		hostlist[2].idptr->override = 0;
		hostlist[0].idptr->type = HOSTID_ETHER;
		hostlist[1].idptr->type = HOSTID_ETHER;
		hostlist[2].idptr->type = HOSTID_ETHER;
		hostlist[0].next = (LM_SERVER *) NULL;

/*
 *		Three ethernet hostids
 */
		conf.server = shuffle(hostlist, 0, 1, 2);

		e = eth; 
		for (i=0; i<ETHER_LEN; i++) hostlist[0].idptr->id.e[i] = e[i];
		f = e + ETHER_LEN; 
		for (i=0; i<ETHER_LEN; i++) hostlist[1].idptr->id.e[i] = f[i];
		g = f + ETHER_LEN; 
		for (i=0; i<ETHER_LEN; i++) hostlist[2].idptr->id.e[i] = g[i];
		CRYPT;			/* ethernet0, ethernet1, ethernet2 */
		conf.server = shuffle(hostlist, 0, 2, 1);
		CRYPT;			/* ethernet0, ethernet2, ethernet1 */
		conf.server = shuffle(hostlist, 1, 2, 0);
		CRYPT;			/* ethernet1, ethernet2, ethernet0 */
		hostlist[0].idptr->type = HOSTID_ETHER;
		hostlist[1].idptr->type = HOSTID_LONG;
		hostlist[2].idptr->type = HOSTID_ANY;
		hostlist[1].idptr->hostid_value = 0x12345678;
		CRYPT; 			/* Ethernet, long, any */
		conf.server = shuffle(hostlist, 1, 0, 2);
		CRYPT; 			/* long, Ethernet, any */
		conf.server = shuffle(hostlist, 1, 2, 0);
		CRYPT; 			/* long, any, ethernet */
		hostlist[0].idptr->type = HOSTID_LONG;
		hostlist[0].idptr->hostid_value = 0xf001234a;
		CRYPT; 			/* long1, any, long0 */
		conf.server = shuffle(hostlist, 0, 1, 2);
		CRYPT; 			/* long0, long1, any */
		conf.server = shuffle(hostlist, 2, 1, 0);
		CRYPT; 			/* any, long1, long0 */
		hostlist[0].idptr->type = HOSTID_USER;
		strcpy(hostlist[0].idptr->hostid_user, "joe");
		CRYPT; 			/* user, long, any */
		conf.server = shuffle(hostlist, 2, 1, 0);
		CRYPT; 			/* any, long, user */
		conf.server = shuffle(hostlist, 1, 2, 0);
		CRYPT; 			/* long, any, user */
		hostlist[2].idptr->type = HOSTID_DISPLAY;
		strcpy(hostlist[2].idptr->hostid_user, "globes");
		CRYPT; 			/* display, long, user */
		conf.server = shuffle(hostlist, 0, 1, 2);
		CRYPT; 			/* user, long, display */
		conf.server = shuffle(hostlist, 1, 0, 2);
		CRYPT; 			/* long, user, display */
	}
	else
	{
	  if (conf.lc_vendor_def) free(conf.lc_vendor_def);
	  (void)memset(&conf, 0, sizeof(conf));
	  (void)memset(&hid, 0, sizeof(hid));
	  l_config(&conf, CONFIG_FEATURE, "feature", "1.0", "", "14-jul-92",
								3, "xxx", NULL);
	  conf.idptr = &hid;
	  conf.idptr->type = HOSTID_LONG;
	  conf.idptr->override = 0;
	  conf.idptr->hostid_value = 0x1200ab46;
	  for (cp = codes; *cp; cp++)
	  {
	    vcode.data[0] = *cp;
	    if (verbose) fprintf(ofp, "CODE: %lx\n", *cp);
	    for (numlic = 0; numlic < LICLIMIT; numlic++)
	    {
		for (yp = years; *yp; yp++)
		{
		    for (mp = months; **mp; mp++)
		    {
			for (day = days; *day; day++)
			{
			    sprintf(date, "%d-%s-%d", *day, *mp, *yp);
		    	    for (fp = feats; **fp; fp++)
			    {
				(void) strncpy(conf.feature, *fp, MAX_FEATURE_LEN);
				conf.users = numlic;
				(void) strncpy(conf.date, date, DATE_LEN);
				if (verbose)
					fprintf(ofp, "%d, %d/%s/%d, %s: ", 
						numlic, *day, *mp, *yp, *fp);
				CRYPT;
				/*fprintf(ofp, "%s\n", lc_crypt(lm_job, &conf, startdate, 
								&vcode));*/
				if (keys++ > numkeys) exit(0);
			    }
			}
		    }
		}
	    }
	  }
	  ver6uniq_test();
	}
	if (conf.lc_vendor_def) free(conf.lc_vendor_def);

}

void
process_args(argc, argv)
int argc;
char *argv[];
{
	while (argc > 1)
	{
	  char *p = argv[1]+1;
		if (!strcmp(argv[1], "-outfile"))
		{
			if (!(ofp = fopen(argv[2], "w")))
			{
				perror("Can't open outfile");
				ofp = stdout;
			} 
			else
			{
				argc--; argv++;
				argc--; argv++;
				continue;
			}
		}
		else if (!strcmp(argv[1], "-verfmt6"))
		{
			startdate = 0;
			lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER,
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V6); 
			argc--; argv++;
			continue;
		}
		else if (!strcmp(argv[1], "-ver6uniq"))
		{
			ver6uniq = 1;
			argc--; argv++;
			continue;
		}
		else if (!strcmp(argv[1], "-filter"))
		{
			lc_set_attr(lm_job, LM_A_USER_CRYPT_FILTER_GEN, 
				(LM_A_VAL_TYPE)user_crypt_filter_gen);
			argc--; argv++;
			continue;
		}
#if 0
		else if (!strcmp(argv[1], "-newkey"))
		{
			newkey = 1;
			lc_set_attr(lm_job, LM_A_KEY_NEW_GEN, 
				(LM_A_VAL_TYPE)stkeynew_gen);
			argc--; argv++;
			continue;
		}
#endif
		else if (!strcmp(argv[1], "-7.1"))
		{
			v7_1 = 1;
			argc--; argv++;
			continue;
		}
		else if (!strcmp(argv[1], "-7.1p"))
		{
			v7_1_print = v7_1 = 1;
			argc--; argv++;
			continue;
		}
		while(*p)
		{
			switch (*p)
			{
				case 'c':
					numkeys = atoi(argv[2]);
					argv++; argc--;
					break;
				case 'f':
					years = fullyears;
					months = fullmonths;
					feats = fullfeats;
					days = fulldays;
					break;
				case 'h':
					varyhostids = 1;
					break;
				case 'k':
					varycodes = 1;
					break;
				case 'o':
					varyorder = 1;
					break;
				case 's':
					years = partyears;
					months = partmonths;
					feats = partfeats;
					days = partdays;
					break;
				case 'v':
					verbose = 1;
					break;
				case '3':
					version_3 = 1;
					break;
				case '4':
					version_4 = 1;
					break;
				case '5':
					version_5 = 1;
					break;
				case '6':
					version_6 = 1;
					break;
				case '7':
					version_7 = 1;
					break;
				case '+':
					if (version_3) 
					{
						version_3plus = 1;
						version_3 = 0;
					}
					break;
				default:
					fprintf(ofp, "Unknown switch %c\n", *p);
					break;
			}
		    p++;
		}
		argc--; argv++;
	}
}

/*
 *	shuffle() - Shuffle redundant host lists - for order tests
 */

static
LM_SERVER *
shuffle(list, first, second, third)
LM_SERVER *list;
int first, second, third;
{
	list[first].next = &list[second];
	list[second].next = &list[third];
	list[third].next = (LM_SERVER *) NULL;
	return(&list[first]);
}


LM_SERVER serv;
CONFIG conf;
char *sdate;

void
version_3_test()
{


	memset(&hid, 0, sizeof(hid));
	serv.idptr = &hid;
	serv.idptr->override = 0;
	serv.idptr->type = HOSTID_LONG;
	serv.idptr->hostid_value = 0x9999;
	serv.next = (LM_SERVER *) NULL;
	sdate = l_bin_date("1-jan-92");
	v3_attr();
}
void
v3_attr()
{
  HOSTID h;

	memset(&conf, 0, sizeof(conf));
	strcpy(conf.feature, "f1");
#ifdef VERSION3_ONLY
	conf.version = 1.05;
#else
	strcpy(conf.version, "1.050");
#endif
	strcpy(conf.daemon, "demo");
	strcpy(conf.date, "1-jan-0");
	conf.users = 0;
	conf.server = &serv;

	fprintf(ofp, "\tv3 tests\n");
	fprintf(ofp, "\tstart identical lines\n");

	conf.lc_vendor_info = "info1"; 
	v3_crypt1(__LINE__);
	conf.lc_vendor_info = "info2"; 
	v3_crypt1(__LINE__);
	conf.lc_vendor_info = (char *)0; 

	conf.lc_dist_info = "dist1";
	v3_crypt1(__LINE__);
	conf.lc_dist_info = "dist2";
	v3_crypt1(__LINE__);
	conf.lc_dist_info = (char *)0;

	conf.lc_user_info = "user1";
	v3_crypt1(__LINE__);
	conf.lc_user_info = "user2";
	v3_crypt1(__LINE__);
	conf.lc_user_info = (char *)0;

	conf.lc_asset_info = "asset1";
	v3_crypt1(__LINE__);
	conf.lc_asset_info = "asset2";
	v3_crypt1(__LINE__);
	conf.lc_asset_info = (char *)0;

	fprintf(ofp, "\tend identical lines\n");
	conf.users = 1;
	v3_crypt1(__LINE__);
	conf.users = 0;
	v3_crypt1(__LINE__);

	conf.lc_vendor_def = "string";
	v3_crypt1(__LINE__);
	conf.lc_vendor_def = "2str";
	v3_crypt1(__LINE__);
	conf.lc_vendor_def = (char *)0;

	memset(&hid, 0, sizeof(hid));
	conf.idptr = &hid;
	conf.idptr->type = HOSTID_LONG;
	conf.idptr->hostid_value = 0x6666;
	v3_crypt1(__LINE__);
	conf.idptr->hostid_value = 0x6667;
	v3_crypt1(__LINE__);
	conf.idptr = (HOSTID *)0;

	conf.lc_issuer = "issuer1";
	v3_crypt1(__LINE__);
	conf.lc_issuer = "issuer2";
	v3_crypt1(__LINE__);
	conf.lc_issuer = (char *)0;

	conf.lc_notice = "1st notice";
	v3_crypt1(__LINE__);
	conf.lc_notice = "2nd notice";
	v3_crypt1(__LINE__);
	conf.lc_notice = (char *)0;

	conf.lc_got_options = LM_LICENSE_LINGER_PRESENT;
	conf.lc_linger = 2;
	v3_crypt1(__LINE__);
	conf.lc_linger = 35;
	v3_crypt1(__LINE__);
	conf.lc_linger = 0;

	conf.lc_got_options = LM_LICENSE_DUP_PRESENT;
	conf.lc_dup_group = (LM_DUP_HOST | LM_DUP_DISP);
	v3_crypt1(__LINE__);
	conf.lc_dup_group |= LM_DUP_USER;
	v3_crypt1(__LINE__);
	conf.lc_dup_group = 0;
	conf.lc_got_options = 0;

	fprintf(ofp, "\tstart identical lines\n");
	sdate = l_bin_date("1-jan-2013");
#ifdef VERSION3_ONLY
	conf.version = 1.050;
#else
	strcpy(conf.version, "1.050");
#endif
	v3_crypt1(__LINE__);
#ifdef VERSION3_ONLY
	conf.version =  1.05;
#else
	strcpy(conf.version, "1.05");
#endif
	v3_crypt1(__LINE__);
	fprintf(ofp, "\tend identical lines\n");
}
void
increment()
{
	fprintf(ofp, "\tincrement tests\n");
	v4_cryptstr(__LINE__, "INCREMENT f1 demo 1.050 01-jan-0 0 start:1-jan-95");
	v4_cryptstr(__LINE__, "INCREMENT f1 demo 1.050 01-jan-0 1 start:1-jan-95");
	v4_cryptstr(__LINE__, "INCREMENT f1 demo 1.050 01-jan-2010 1 start:1-jan-95");
	v4_cryptstr(__LINE__, "INCREMENT f3 demo 1.050 01-jan-2010 1 start:1-jan-95");
	v4_cryptstr(__LINE__, "INCREMENT f3 demo 1.000 01-jan-2010 1 start:1-jan-95");
	v4_cryptstr(__LINE__, "INCREMENT f1 demo 1.050 01-jan-0 0 start:1-jan-95 DUP_GROUP=HD");
	fprintf(ofp, "\texpect bad line\n");
	v4_cryptstr(__LINE__, "INCREMENT f1 demo2 1.050 01-jan-0 0 start:1-jan-95 DUP_GROUP=HD");
}
void
upgrade()
{
	fprintf(ofp, "\tupgrade tests\n");
	v4_cryptstr(__LINE__, "UPGRADE f1 demo 1.05 2.000 01-jan-0 1 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f1 demo 2.0 3.000 01-jan-0 1 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.0 3.000 01-jan-0 1 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.1 3.000 01-jan-0 1 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.1 3.100 01-jan-0 1 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.1 3.100 01-jan-2010 1 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.1 3.100 01-jan-2010 5 start:1-jan-92 \"\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.1 3.100 01-jan-2010 6 start:1-jan-92 \"\"");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.100 3.100 01-jan-2010 6 start:1-jan-92 \"vendor str\"");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo 2.1 3.100 01-jan-2010 6 start:1-jan-92 \"vendor str\"");
	fprintf(ofp, "\tend identical lines\n");
	fprintf(ofp, "\texpect bad line\n");
	v4_cryptstr(__LINE__, "UPGRADE f2 demo2 2.1 3.100 01-jan-2010 6 start:1-jan-92 \"vendor str\"");
}
void
v3_crypt(line)
char * line;
{
	/*fprintf(ofp, "%s, %d\n", lc_crypt(lm_job, &conf, sdate, &code), line);*/

	startdate = sdate;
	/*fprintf(ofp, "%s\n", lc_crypt(lm_job, &conf, sdate, &vcode));*/
	CRYPT

}
void
v3_crypt1( line)
int line;
{
	startdate = sdate;
	CRYPT
 /*fprintf(ofp, "%s\n",lc_crypt(lm_job,&conf,sdate,&vcode));*/
}


#ifndef VERSION3_ONLY 
/* Use lc_crypstr, and produce only version 3.0+ features */

char str[MAX_CONFIG_LINE + 1];
char hostid[MAX_HOSTID_LEN + 1];
char header[MAX_CONFIG_LINE + 1];
char *errors;
void
v7_attr()
{
	fprintf(ofp, "\tv7 tests\n");

#if 0
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:100 0");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:fi:filename.met:100 0");
	v4_cryptstr(__LINE__, "SERVER this_host ANY\nVENDOR demo\nFEATURE f1 demo 1.0 permanent COUNT=METER:fi:filename.met:100 0");
	v4_cryptstr(__LINE__, "SERVER this_host 1234\nVENDOR demo\nFEATURE f1 demo 1.0 permanent COUNT=METER:fi:filename.met:100 0");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:99 0");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:101 0");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:102 0");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:1 0");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent COUNT=METER:2 0");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:100");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:fi:filename.met:100");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:fi:100");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=100");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:99");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:101");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:3");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=METER:2");
#endif
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=1234-5678");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=1234-5679");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=1234-5679-ABCD-EF12");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=1234-5679-ABCD-EF13");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=1234-5679-ABCD-EF13-1234-5678");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent uncounted 0 HOSTID=1234-5679-ABCD-EF13-1234-5679");
}
void
v6_attr()
{
#define LONGFEAT "SERVER this_host ID_STRING=testing123 3700\n\
VENDOR demo demo\n\
FEATURE f1 demo 1.0 permanent 3 start:1-jan-92 dup_group=uh \\\n\
host_based issued=12-jan-2010 issuer=\"This is a test\" \\\n\
notice=\"this is the notice\" overdraft=4 platforms=\"sun4_u sgi32_u \\\n\
alpha_u i86_n\" SN=1234 start=1-jan-1992 suite_dup_grouP=uH \\\n\
supersede=\"anotherFEAT\" VENDOR_STRING=\"this is the vs\" \\\n\
asset_info=asset ck=123 dist_info=\"dist info\" \\\n\
HOSTID=\"08002b32b161 1234 USER=joe display=tom hostNAME=speedy \\\n\
ID_STRING=abcdEF DISK_SERIAL_NUM=12345 DEMO any ID=12345 \\\n\
FLEXID=7-abDC1234 FLEXID=8-abcDEF1234 FLEXID=9-abCD1234 FLEXID=a-AB123\\\n\
interNET=123.456.111.111\""
  char longfeat[sizeof(LONGFEAT) + 1];

	strcpy(longfeat, LONGFEAT);
	fprintf(ofp, "\tv6 tests\n");
	fprintf(ofp, "\tstart identical lines\n");

	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=1-jan-90 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=1-jan-1990 HOSTID=ANY");

	fprintf(ofp, "\tend identical lines\n");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=01-JaN-2001 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=1-jan-2001 HOSTID=ANY");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=1-jan-2000 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=29-feb-2000 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=1-mar-2000 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 START=1-mar-3000 HOSTID=ANY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent 0 0 HOSTID=DOMAIN=globes.com");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent 0 0 HOSTID=DOMAIN=globes2.com");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent 0 0 HOSTID=\"DOMAIN=globes2.com 1234\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.0 permanent 0 0 HOSTID=\"DOMAIN=globes2.com DOMAIN=globes3.com 1234\"");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, longfeat);
	l_uppercase(longfeat);
	v4_cryptstr(__LINE__, longfeat);
	l_lowercase(longfeat);
	v4_cryptstr(__LINE__, longfeat);
	fprintf(ofp, "\tend identical lines\n");
}

void
v5_attr()
{
	fprintf(ofp, "\tv5 tests\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=123456789");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=1234");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=1235");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=\"EXAMPLE_HOSTID=1235 USER=joe\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=\"EXAMPLE_HOSTID=1235 12345678\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=\"EXAMPLE_HOSTID=1235 12345679\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=12345678");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=123456788");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 SN=3");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 SN=3a");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 SUITE_DUP_GROUP=UHD");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=UHD");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 SUITE_DUP_GROUP=UH");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 USER_BASED");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOST_BASED");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 METERED");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 CAPACITY");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 CAPACITY USER_BASED");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 CAPACITY USER_BASED=1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 CAPACITY USER_BASED=2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 CAPACITY HOST_BASED=1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 CAPACITY HOST_BASED=2");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "SERVER a ANY 1000\nDAEMON demo demo\nFEATURE f1 demo 1.050 01-jan-0 4 start:1-jan-92 CAPACITY METERED");
	v4_cryptstr(__LINE__, "SERVER a ANY 1000\nDAEMON demo demo\nFEATURE f1 demo 1.050 01-jan-0 4 start:1-jan-92 METERED CAPACITY");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "SERVER a ANY 1000\nDAEMON demo demo\nFEATURE f1 demo 1.050 01-jan-0 4 start:1-jan-92 USER_BASED=10");
	v4_cryptstr(__LINE__, "SERVER a ANY 1000\nDAEMON demo demo\nFEATURE f1 demo 1.050 01-jan-0 4 start:1-jan-92 USER_BASED=10 MINIMUM=3");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ISSUED=1-jan-1994");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ISSUED=1-feb-1994");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ISSUED=1-feb-1994 SUPERSEDE");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 SUPERSEDE=f1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 SUPERSEDE=\"f1 f2\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=123-456-789-1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=123-456-789-2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=323-456-789-2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=223-456-789-2");

	fprintf(ofp, "\tstart identical lines\n");

	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=7-10A9877");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=SENTINEL_KEY=10A9877");
	fprintf(ofp, "\tdifferent identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=123-456-789-0");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=12-34-5678-90");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID=1234567890");

	fprintf(ofp, "\tend identical lines\n");

	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=8-10A9877");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=9-10A9877");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=A-10A9877");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=A-10");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=7-10");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=FLEXID=7-11");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 PLATFORMS=sun4_u4");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 PLATFORMS=\"sun4_u4 alpha_u1\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 PLATFORMS=\"i86_w3\"");
}
v4_attr()
{
	fprintf(ofp, "\tv4 tests\n");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ck=1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ck=300");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 OVERDRAFT=10");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 OVERDRAFT=1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=SITE");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=NONE");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=UHD");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=DHU");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=D");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=UH");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=HD");
	fprintf(ofp, "\tvendor-defined hostids\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=1234");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=1235");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=EXAMPLE_HOSTID=vendor-def");
	/* ethernet tests */
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=08002b32b161");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=08002b32b162");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=#4000000000");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ee6b2800");
	fprintf(ofp, "\tend identical lines\n");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=#268435456");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=10000000");
	fprintf(ofp, "\tend identical lines\n");
	/* P2249 tests */
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=fff00000");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ff100000");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=fff0b000");
}

/*
 *	version_3plus_test -- same as version_3 
 *			but uses lc_cryptstr instead of lc_crypt
 */
void
version_3plus_test()

{
	strcpy(header, "SERVER speedy 9999 2937\n");
	strcat(header, "DAEMON demo /u/gsi/lmgr/testsuite/demo_noadd\n");
	fprintf(ofp, "\tv3 tests\n");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 vendor_info=info1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 vendor_info=info2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 dist_info=dist1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 dist_info=dist2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 user_info=user1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 user_info=user2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 asset_info=asset1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 asset_info=asset2");
	fprintf(ofp, "\tend identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 1 start:1-jan-92");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 VENDOR_STRING=\"string\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 VENDOR_STRING=\"2str\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=6666");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=6667");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ISSUER=issuer1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 ISSUER=issuer2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 NOTICE=\"1st notice\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 NOTICE=\"2nd notice\"");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 LINGER=2");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 LINGER=35");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=HD");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 DUP_GROUP=UHD");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID_STRING=string1");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-92 HOSTID=ID_STRING=string2");
	fprintf(ofp, "\tstart identical lines\n");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.050 01-jan-0 0 start:1-jan-2013");
	v4_cryptstr(__LINE__, "FEATURE f1 demo 1.05 01-jan-0 0 start:1-jan-2013");
	fprintf(ofp, "\tend identical lines\n");
}
package()
{
	fprintf(ofp, "\tpackage tests\n");
	v4_cryptstr(__LINE__, "PACKAGE f1 demo 1.0 0 COMPONENTS=\"comp1 comp2\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 1.0 0 COMPONENTS=\"comp1 comp2\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1 comp2\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1 comp2 comp3\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1:1.000 comp2 comp3\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1:1.000:3 comp2 comp3\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1:1.000:3 comp2:2.0 comp3\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1:1.000:3 comp2:2.0:4 comp3\"");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1:1.000 comp2:2.0 comp3\" OPTIONS=SUITE");
	v4_cryptstr(__LINE__, "PACKAGE f2 demo 2.0 0 COMPONENTS=\"comp1:1.000 comp2:2.0 comp3\"");
	increment();
	upgrade();
}
void
v4_cryptstr(line, x) 
int line ;
char *x;
{
  char *crypt_str = (char *)0, *stat;
  int flag = LM_CRYPT_ONLY;
	if (newkey) flag |= LM_CRYPT_KEY2_ONLY;
	else flag |= LM_CRYPT_ONLY;
	sprintf(str, "%s%s", header, x); 
	lc_cryptstr(lm_job, str, &crypt_str, &vcode, flag, 0, (char **)0); 
	if (!crypt_str) stat = "Bad";
	else stat = crypt_str;
	fprintf(ofp, "%s", stat); 
	if (crypt_str)free(crypt_str); 
	fprintf(ofp, "\n");
}
void
version_4_test()
{
	strcpy(header, "SERVER speedy 9999 2937\n");
	strcat(header, "DAEMON demo /u/gsi/lmgr/testsuite/demo_noadd\n");
	v4_attr();
	package();
}
void
version_5_test()
{
	v5_attr();
}
void
version_6_test()
{
	lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
		(LM_A_VAL_TYPE)LM_BEHAVIOR_V6); 
	v6_attr();
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1); 
	v6_attr();
}
void
version_7_test()
{
	lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
		(LM_A_VAL_TYPE)LM_BEHAVIOR_V7); 
	v7_attr();
}
#else
void
version_3plus_test()
{
}
void
version_4_test()
{
}
#endif /* version 4 */

void
ver6uniq_test()
{
  CONFIG **conf = &lm_job->line;
  int i, j, x;
  char c;
  HOSTID h;
#define CONF (*conf)
#define DEF_LIC "START_LICENSE\n\
		SERVER aaa 554066fa\n\
		FEATURE PKG6COMP1 demo 1.05 permanent 2 0"
#define PRTCRYPT(str, descr) fprintf(ofp, "%s\n", \
				lc_crypt(lm_job, CONF, 0, &vcode));  \
		/*fprintf(stderr,"%s %s\n",  descr, str);*/

#define PRTCRYPTI(i,str) \
		fprintf(ofp, "%s\n", lc_crypt(lm_job, CONF, 0, &vcode));  \
		 /* fprintf(stderr, "%s %x\n", str, (i));*/
	
#define PUTBACK lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)\
		DEF_LIC);


#if 0
 	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)0);
 	lc_set_attr(lm_job, LM_A_LKEY_START_DATE, (LM_A_VAL_TYPE)0);
#endif
 	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
 	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
		"START_LICENSE\n\
		SERVER aaa 554066fa\n\
		FEATURE PKG6COMP1 demo 1.05 permanent 2 0");
	PRTCRYPT("def lic", "");
/*
 *	Now change one thing at a time
 */
	strcpy(CONF->version, "1.06"); PRTCRYPT(CONF->version, "ver"); PUTBACK;
	strcpy(CONF->version, "1.0"); PRTCRYPT(CONF->version, "ver"); PUTBACK;
	for (i = 0; i < strlen(CONF->feature); i++)
	{
		for (c = '!'; c <= '~'; c++)
		{
		  char sav  = CONF->feature[i] ;
			if (islower(c)) continue;
			CONF->feature[i] = c;
			if (c != sav)
			{
				PRTCRYPT(CONF->feature, "1"); 
			}
			CONF->feature[i] = sav;
		}
	}
	strcpy(CONF->feature, "THISISALONGFEATURENAME");
	for (i = 0; i < strlen(CONF->feature); i++)
	{
		for (c = '!'; c <= '~'; c++)
		{
		  char sav  = CONF->feature[i] ;
			if (islower(c)) continue;
			CONF->feature[i] = c;
			if (c != sav)
			{
				PRTCRYPT(CONF->feature, "1"); 
			}
			CONF->feature[i] = sav;
		}
	}
	PUTBACK;
	strcpy(CONF->feature, "F1");
	for (i = 0; i < strlen(CONF->feature); i++)
	{
		for (c = '!'; c <= '~'; c++)
		{
		  char sav  = CONF->feature[i] ;
			if (islower(c)) continue;
			CONF->feature[i] = c;
			if (c != sav)
			{
				PRTCRYPT(CONF->feature, "1"); 
			}
			CONF->feature[i] = sav;
		}
	}
	PUTBACK;
	{

	  char *v = "12345.678";
	  char savver[20];

		strcpy(savver,CONF->version);
		strcpy(CONF->version, v);

		for (i = 0; i < strlen(CONF->version); i++)
		{
			for (c = '0'; c <= '9'; c++)
			{
			  char sav  = CONF->version[i] ;
				CONF->version[i] = c;
				if (c != sav)
				{
					PRTCRYPT(CONF->version, "vers"); 
				}
				CONF->version[i] = sav;
			}
		}
		strcpy(CONF->version,savver);
	}
	{

	  char v[100];

		strcpy(v, "THIS IS A LONG VENDOR-DEFINED STRING");

		CONF->lc_vendor_def = v;
		PRTCRYPT(CONF->lc_vendor_def, "vdef"); 
		for (i = 0; i < strlen(CONF->lc_vendor_def); i++)
		{
			for (c = '!'; c <= '~'; c++)
			{
			  char sav  = CONF->lc_vendor_def[i] ;
				if (islower(c)) continue;
				CONF->lc_vendor_def[i] = c;
				if (c != sav && sav != ' ')
				{
					PRTCRYPT(CONF->lc_vendor_def, "vdef"); 
				}
				CONF->lc_vendor_def[i] = sav;
			}
		}
		CONF->lc_vendor_def = 0;
	}
	for (i = 0; i < 8; i++)
	{
		for (j = 0; c <= 0xf; j++)
		{
		  int sav  = CONF->server->idptr->id.data;
			CONF->server->idptr->id.data &= ~(0xf << i);
			CONF->server->idptr->id.data |= (j << i);
			if (CONF->server->idptr->id.data != sav)
			{
				PRTCRYPTI(CONF->server->idptr->id.data, "servid");
			}
			CONF->server->idptr->id.data = sav;
		}
	}
	x = CONF->server->idptr->id.data;
	CONF->server->idptr->id.data = 0xffffffff;
	for (i=0; i < 32; i += 4)
	{
		for (j=0; j < 0xf; j++)
		{
		  int sav = CONF->server->idptr->id.data;
			CONF->server->idptr->id.data &= ~(0xf << i);
			CONF->server->idptr->id.data |= (j << i);
			if (CONF->server->idptr->id.data != sav)
			{
				PRTCRYPTI(CONF->server->idptr->id.data, "servid");
			}
			CONF->server->idptr->id.data = sav;
		}
	}
	memset(&h, 0, sizeof(h));
	CONF->idptr = &h;
	h.id.data = 0x554066fa;
	h.type = HOSTID_LONG;
	for (i=0; i < 32; i += 4)
	{
		for (j=0; j < 0xf; j++)
		{
		  int sav = CONF->idptr->id.data;

			CONF->idptr->id.data &= ~(0xf << i);
			CONF->idptr->id.data |= (j << i);
			if (CONF->idptr->id.data != sav)
			{
				PRTCRYPTI(CONF->idptr->id.data, "nl id");
			}
			CONF->idptr->id.data = sav;
		}
	}
	CONF->idptr = 0;
	{ 
	  int sav = CONF->users;
		for (i = 0; i < 999; i++)
		{
			CONF->users = i;
			if (i != sav)
			{
				PRTCRYPTI(CONF->users, "users");
			}
		}
		CONF->users = sav;
	}
	{ 
	  char sav[10];
	  int y, m,d;
		strcpy(sav, CONF->date);
		for (y = 1997; y < 2005; y++)
		{
			for (m = 1; m <=12; m++)

			{
				for (d = 1; d <=28;d++)
				{
					sprintf(CONF->date, "%d-%s-%d",
						d, 
						m == 1 ? "jan" :
						m == 2 ? "feb" :
						m == 3 ? "mar" :
						m == 4 ? "apr" :
						m == 5 ? "may" :
						m == 6 ? "jun" :
						m == 7 ? "jul" :
						m == 8 ? "aug" :
						m == 9 ? "sep" :
						m == 10 ? "oct" :
						m == 11 ? "nov" :
						"dec", y);
					if (strcmp(CONF->date, sav))
					{
						
						PRTCRYPT(CONF->date, 
								"date"); 
					}
				}
				strcpy(CONF->date, sav);
			}
		}
	}
}
changebit(c, bit)
char *c;
int bit;
{
	if (*c & 1<<bit) *c &= ~(1<<bit);
	else *c |= (1<<bit);
}
#if 1
v7_1test()
{
  VENDORCODE vc;
  int i;
  char *str;

	memcpy(&vc, &code, sizeof(vc));
	vc.data[0] ^= VENDOR_KEY5;
	vc.data[1] ^= VENDOR_KEY5;
	for (i = 0; i < l_priseedcnt; i++)
	{
	  int j;
		for (j =0; j < lm_prisize[i][j]; j++)
		{
			memcpy(&code.pubkeyinfo[i].pubkey[j], &lm_prikey[i][j], 
			sizeof(lm_prikey[i][j]));
		}
		code.pubkeyinfo[i].pubkey_fptr = l_pubkey_verify;
		code.pubkeyinfo[i].strength = LM_STRENGTH_PUBKEY;
		memcpy(&code.pubkeyinfo[i].pubkeysize, &lm_prisize, sizeof(lm_prisize));
	}
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)1);
	lc_cryptstr(lm_job, "FEATURE f1 demo 1.0 01-jan-0 0 KEY=0 HOSTID=ANY",
		&str, &vc, LM_CRYPT_ONLY, 0, (char **)0); 
	if (str)
	{
		puts(str);
		free(str);
	}

}
#else
v7_1test()
{
  char istr[10000];
  int i;
  VENDORCODE vc;
  int len;
#if 0

#define MODVAL ('~' - ' ')
	memcpy(&vc, &code, sizeof(vc));
	vc.data[0] ^= VENDOR_KEY5;
	vc.data[1] ^= VENDOR_KEY5;
/*	
 *	First, very short strings 
 */
	lc_set_attr(lm_job, LM_A_KEY_NEW_GEN, (LM_A_VAL_TYPE)stkeynew_gen);
	lc_set_attr(lm_job, LM_A_KEY_NEW, (LM_A_VAL_TYPE)stkeynew);
	memset(istr, 0, sizeof(istr));
	len = 0;
	while (i <= 26)
	{
		len++;
		i++;
		istr[len] = rand()%26 + 'A';
		if (!(len == 5))
		{
			testv71str(istr, len, &vc);
			len = 0;
		}
	}
	for (len = 0, i = ' '; i < '~'; i ++, len++)
	{
		istr[i++] = i;
	}
	testv71str(istr, len, &vc);
	for (len = 0, i = 0; i < 256; i ++, len++)
	{
		istr[i++] = i; /* 8bit */
	}
	testv71str(istr, len, &vc);
	while (1)
	{
		for (i = 0; i < 2000; i++)
			istr[i] = rand() % MODVAL + ' '; /* printable */
		testv71str(istr, rand() % 2000, &vc);
		for (i = 0; i < 2000; i++)
			istr[i++] = rand() % 256; /* 8bit */
		testv71str(istr, rand() % 2000, &vc);
	}
#endif
}
#endif
testv71str(istr, len, vc)
char *istr;
int len;
VENDORCODE *vc;
{
  char *ret1, *ret2;
  char buf1[MAX_CRYPT_LEN + 1], buf2[MAX_CRYPT_LEN + 1];
  int i, j;
  long d0 = vc->data[0], d1 = vc->data[1], savd1, savd2;

  extern unsigned char * crypt_string_key lm_args(( LM_HANDLE_PTR,
	unsigned char *, int, VENDORCODE *, unsigned long));
  extern unsigned char * crypt_string_app lm_args(( LM_HANDLE_PTR,
	unsigned char *, int, VENDORCODE *, unsigned long, char *));
	
	for (i = 0; i < 1000; i++)
	{
		savd1 = vc->data[0];
		savd2 = vc->data[1];
		vc->data[0] ^= (rand() << (rand()%32) & 0xffffffff);
		vc->data[1] ^= (rand() << (rand() % 32) & 0xffffffff);
		if ((savd1 == vc->data[0]) && (savd2 == vc->data[1]))
			continue;
		ret1 = crypt_string_key(lm_job, istr, len, vc, L_SECLEN_LONG); 
		strcpy(buf1, ret1);
		ret2 = crypt_string_app(lm_job, istr, len, vc, L_SECLEN_LONG, 
								buf1); 
		if (!ret2)
		{ 
			printf("failed with string: "); 
			for (j = 0; j < len; j++ )
				printf("%02x", istr[j]);
			puts("");
		}
		else 
		{
			if (ret2 = crypt_string_app(lm_job, istr, len, vc, 
					L_SECLEN_LONG, "0123456789ABCDEF"))
			{
				printf("Bad expected string, but crypt succeeded!");
				for (j = 0; j < len; j++ )\
					printf("%02x", istr[j]);\
				puts("");\
				printf("Changed exp string[%d] %s to %c\n", 
					len/2, buf1, ret1[len/2]);
				
			}
		}
		if (v7_1_print && (v7_1_print++ < 30000))
		{
#if 0
		  static FILE *fp;
			if (!fp) fp = fopen("v7.1strs.out", "w");
			fprintf(fp, "%s: istr ", buf1);
			for (j = 0; j < len; j++)
				fprintf(fp, "%02x", istr[j]);\
			fprintf(fp, " seeds: %x %x\n", 
					vc->data[0], vc->data[1]);
			fflush(fp);
#endif
			
			/* if (v7_1_print > 485)
				puts("here");*/
			puts(buf1);
		}
	}
	vc->data[0] = d0;
	vc->data[1] = d1;
}
