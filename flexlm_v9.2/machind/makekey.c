/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *
 *	Module: $Id: makekey.c,v 1.40 2003/05/05 15:58:04 sluu Exp $
 *
 *	Function: makekey
 *
 *	Description: Create a license file for the license manager
 *
 *	Parameters:	None.
 *
 *	Return:		None - license file is written
 *
 *	NOTE:		The salient function in this file is lc_cryptstr(),
 *			Everything else is I/O, checking, and setting up 
 *			the call to lc_cryptstr().  Vendors are permitted 
 *			to modify this source as needed.
 *
 *			Normally, lmcrypt is a better tool for making
 *			licenses.
 *
 *	M. Christiano
 *	2/25/88
 *
 *	Last changed:  12/10/98
 *
 */

#include "lmprikey.h"
#include "lmclient.h"
#include "lm_code.h"
#include "lmseeds.h"
#include "lm_attr.h"
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#ifdef ANSI
#include <stdlib.h>
#include <string.h>
#endif /* ANSI */
#if !defined(PC) && !defined(VMS)
#ifdef ANSI
#include <unistd.h>
#endif /* ANSI */
#endif /* UNIX */
#ifdef PC
#include <io.h>
#else /* PC */
#ifndef ANSI
extern char *getenv();
extern char *strchr();
extern char *strncpy();
#endif /* ANSI */
#endif

static int yes(), file_exists();
static void getinput lm_args((char *, FILE *));
#ifndef PC
extern int errno;
#endif /* PC */
char def_file[20];

#ifdef VMS
#define unlink delete
void unlink();
extern char *flm_vms_sys_error();
#undef SYS_ERRLIST
#define SYS_ERRLIST(x) flm_vms_sys_error(x)
#endif /* PC */

lm_extern int API_ENTRY
	l_validdate	 lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));

static void user_info_tag lm_args((char *out)); 
static void usage lm_args((lm_noargs));

void grep_server_daemon lm_args(( char *, char *));
static int add_optional_attrs lm_args(( char *, VENDORCODE *));
static void hostid_help lm_args((lm_noargs));
char default_file[LM_MAXPATHLEN];
char *license_fmt_ver;
char sav_errmsg[101];

char *months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul",
			"aug", "sep", "oct", "nov", "dec"};

LM_HANDLE *lm_job;



LM_CODE(site_code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
			VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);


#define MAX_USER_INPUT 500	/* 100 bytes of input per response */
#define MAX_FILE_SIZE 30000 /* 30k for now */
/*
 *	Defaults
 */
int port, dport, BASEPORT = 10001, port_used = -1;
#define RED_PORT 27009
int red_port = -1;
#ifdef VMS
char obj[MAX_USER_INPUT+1], *OBJECT = "200";  /* DECnet known object number */
#endif
FILE *ifp;
int record;
FILE *recordf;
FILE *ofp;
char our_daemon[MAX_DAEMON_NAME+1] = "";
char expdate[MAX_USER_INPUT+1] = "permanent";
char startdate[MAX_USER_INPUT+1]; /* defaults to today */
int nlic = 4;
int overdraft = 0;
char ipn[MAX_CONFIG_LINE+1];
char version[MAX_VER_LEN+1] = "1.0";
char fromversion[MAX_VER_LEN+1] = "1.0";
char lock_hostid[MAX_USER_INPUT+1] = "";
int crypt_flags = LM_CRYPT_NO_LINE_NUMBERS | LM_CRYPT_FORCE;
int have_dup_group = 0;
int dup_group = 0;
/*VENDORCODE clearcodes;*/
char *vendor_str = LM_RESERVED_PROG_ALIAS;
char resp[MAX_USER_INPUT+1];	/* Response from user */

int use_tcp =1;
int ipx_def_socket_num = 1234;
int ipx_socket_num = 1234;
char ipx_node_num[13] = "000000000001";
char ipx_network_num[9] = "00000000";

FILE *outfp;			/* File descriptor of output file */
char *outfile;
char outbuf[MAX_FILE_SIZE];
int FLOAT_OK = 0;


void
cleanup_exit(int sig)
{
	if (outfp && outfile && *outfile)
	{
		fprintf(ofp, "\nExiting... remove %s? (y/n):  ", outfile);
		getinput(resp, ifp);
		if (*resp == 'y' || *resp == 'Y')
		{
			fclose(outfp);
			fprintf(ofp, "Removing %s\n", outfile);
			unlink (outfile);
		}
		else
		{
			fprintf(outfp, outbuf);
			fclose(outfp);
		}
	}
	exit(1);
}

int
main(argc, argv)
int argc;
char *argv[];
{
  char outfilename[MAX_USER_INPUT+1];
  char one_feature[MAX_CONFIG_LINE + MAX_USER_INPUT];
  char *out = outbuf;
  char *outbufp = outbuf;
  char *return_str, *errors;
  char *start_of_feature;
  char *start_of_daemon = 0;
  char resp2[MAX_USER_INPUT+1];	/* Response from user */
  int arg;
  char hname[MAX_USER_INPUT+1], id[MAX_USER_INPUT+1];
  int numservers = 0, maxservers = 3;
  int type;			/* Type of license: FEATURE/INCREMENT/UPGRADE */
  int served = 0;		/* if user specified SERVER line */
  int counted = 0;		/* if counted feature line specified */
  int rc;
  int oldkey = 0;
  int newkey = 1;
#if defined (_WINDOWS) && !defined(WINNT)
  struct _wsizeinfo wsize;	

	/*
	 * Set QuickWin window behavior customization.
	 */
	_wsetexit( _WINEXITPERSIST );
	_wabout( "FLEXlm Create License Program\n" COPYRIGHT_STRING(1988) );
	wsize._version = _QWINVER;	
	wsize._type = _WINSIZEMAX;
	_wsetsize( _wgetfocus(), &wsize );	
#endif  
	ifp = stdin;
	ofp = stdout;
	*outbuf = 0;
	*ipn = 0;
	*hname = 0;

#if !defined(PC) && !defined(VMS)
	signal(SIGINT, cleanup_exit);
#endif /* UNIX */
	LM_CODE_GEN_INIT(&site_code);
	/*- P5245 */
	if (site_code.strength == LM_STRENGTH_LICENSE_KEY)
	{
		newkey = 0;
		oldkey=1;
	}
	if (lc_init((LM_HANDLE *)0, 
		VENDOR_NAME, &site_code, (LM_HANDLE **) &lm_job) )
        {
		lc_perror(lm_job, "lc_init failed");
                exit(1);
        }
	sprintf(def_file, "%s.lic", VENDOR_NAME);

	{
	  char *x = VENDOR_NAME;
	  int i;
	  long hash = 0;

/*
 *		The purpose of the following code is to derive a
 *		port number based on the VENDOR_NAME that's somewhere
 *		in the range 10000 < n < 32000, which is probably the
 *		most sparsely used range of port numbers.
 */
                for (i = 0; x[i]; i++)
                        hash += ((((i+1)*100) * x[i]) + x[i]);
                dport = (hash % 22000) + BASEPORT;

	}
#if 0
	memcpy((char *)&clearcodes, (char *)&site_code, 
							sizeof(clearcodes));
	clearcodes.data[0] ^= VENDOR_KEY5;
	clearcodes.data[1] ^= VENDOR_KEY5;
	memcpy(&clearcodes.pubkeysize[0], &lm_prisize[0], sizeof(lm_prisize));
	for (i = 0; i < LM_PUBKEYS; i++)
		memcpy(&clearcodes.pubkey[i][0], &lm_prikey[i][0], 
			lm_prisize[i]);
#endif
	if (lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0))
		lc_perror(lm_job, "lc_set_attr, LM_A_LONG_ERRMSG");

	for (arg = 1; arg < argc; arg++)
	{
		if (!strcmp(argv[arg], "-i") && ++arg < argc)
		{
			if (!(ifp = fopen(argv[arg], "r") ))
			{
                                fprintf(ofp, "Can't open input file %s\n",
							argv[arg]);
				cleanup_exit(0);
			}
		} 
		else if (!strcmp(argv[arg], "-o") && ++arg < argc)
			outfile = argv[arg];
		else if (!strcmp(argv[arg], "-oldkey"))
			oldkey = 1;
		else if (!strcmp(argv[arg], "-spx_port") && ++arg < argc)
		{
			outfile = argv[arg];
			use_tcp = 0;
			sscanf(argv[arg], "%d", &port);	/* overrun checked */
		}
		else if (!strcmp(argv[arg], "-java"))
		{
			lc_set_attr(lm_job, LM_A_JAVA_LIC_FMT, (LM_A_VAL_TYPE)1);
		}
		else if (!strcmp(argv[arg], "-record"))
		{
			fprintf(ofp, "record file is create.inp\n");
			record = 1;
		}
		else if (!strcmp(argv[arg], "-startdate") && ++arg < argc)
			strcpy(startdate, argv[arg]);
		else if (!strcmp(argv[arg], "-redirect"))
		{
			if (!(ofp = fopen("create.out", "w")))
			{
				perror("Can't open create.out");
				cleanup_exit(0);
			}
		}
		else if (!strcmp(argv[arg],"-longkey")) 
			lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
		else if (!strcmp(argv[arg],"-shortkey"))
			lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)0);
		else if (!strcmp(argv[arg],"-longerrs"))
			lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
		else if (!strcmp(argv[arg],"-decimal"))
			crypt_flags |= LM_CRYPT_DECIMAL_FMT;
		else if (!strcmp(argv[arg],"-maxlen"))
		{
		  int max = 0;

			arg++;
			if (arg >= argc)
				break;
			sscanf(argv[arg], "%d", &max);	/* overrun checked */
			if (!max)
				fprintf(stderr, 
				"Error: -maxlen %s Invalid line length\n", argv[arg]);
			else
				lc_set_attr(lm_job, LM_A_MAX_LICENSE_LEN, 
					(LM_A_VAL_TYPE)max);
		}
		else if (!strcmp(argv[arg],"-verfmt"))
		{
			arg++;
			if (arg >= argc)
				break;
			switch(*argv[arg])
			{
			case '2':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V2); break;
			case '3':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V3); break;
			case '4':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V4); break;
			case '5':
				if (!strncmp(argv[arg],"5.1", 3))
					lc_set_attr(lm_job, 
						LM_A_LICENSE_FMT_VER, 
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V5_1);
				else
					lc_set_attr(lm_job, 
						LM_A_LICENSE_FMT_VER, 
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V5); 
				break;
			case '6':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V6); break;
			case '7':
				if (!strncmp(argv[arg],"7.1", 3))
					lc_set_attr(lm_job, 
						LM_A_LICENSE_FMT_VER, 
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V7_1);
				else
					lc_set_attr(lm_job, 
						LM_A_LICENSE_FMT_VER, 
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V7); 
				break;
			default :
				usage();
			}
		}
		else
			outfile = argv[arg];
	}
	fprintf(ofp, "\nmakekey - \n");
	fprintf(ofp, COPYRIGHT_STRING(1988) "\n\n");
	lc_get_attr(lm_job, LM_A_LICENSE_FMT_VER, (short *)&license_fmt_ver);
	sprintf(default_file, "%s.lic", VENDOR_NAME);
	if (strcmp(license_fmt_ver, LM_BEHAVIOR_V6) < 0)
	{
		vendor_str = LM_RESERVED_PROG;
		strcpy(expdate, "1-jan-0");
	}


	if (!outfile)
	{
		outfile = def_file;
newfile:
		while ( !*outfile || file_exists(outfile))
		{
			if (*outfile)
			{
				fprintf(ofp, "%s exists, ", outfile);
				fprintf(ofp, "Overwrite? (y/n): ");
				if (yes())
					break;
			}
			fprintf(ofp, "please enter output filename: (default %s): ", default_file);
			getinput(resp, ifp);
			if (!*resp)
				outfile = default_file;
			else
				outfile = resp;
#ifdef VMS
			if (strchr(outfile, '.') == (char *) NULL) 
				strcat(outfile, ".dat");
#endif
		}
	}

	outfp = fopen(outfile, "w");
	if (outfp == (FILE *) NULL)
	{
		perror("open output file");
		outfile = def_file;
		goto newfile;
	}
	strcpy(outfilename, outfile);  /* Save it for later */
#ifdef VMS
	fclose(outfp);
#endif
	if (lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE) 1))
		lc_perror(lm_job, "lc_set_attr, LM_A_DISABLE_ENV");
	if (lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE) outfilename))
		lc_perror(lm_job, "lc_set_attr, LM_A_LICENSE_FILE");
#ifdef VMS
	outfp = fopen(outfilename, "a");
	if (outfp == (FILE *) NULL) perror("re-open output license file (3)");
#endif
	{
		if (maxservers == 1)
		{
			fprintf(ofp, 
			"\nPlease enter the name of the server node where you\n"
									);
			fprintf(ofp, "would like to run the FLEXlm daemons.\n");
		}
		else
		{
			fprintf(ofp, 
			"\nYou will be prompted for a list of server names and hostids.\n");
			fprintf(ofp, 
			"When the list is complete, type <CR> at the hostname prompt.\n");
		}
		fprintf(ofp, "\nNOTE: The server node name(s) and TCP/IP port numbers\n");
		fprintf(ofp, "      can be changed by the end-user of your product.\n");
		numservers = 0;
		while (1)
		{

			fprintf(ofp, "\nEnter hostname");
			if (numservers == 0)
			{
				fprintf(ofp, 
				" (RETURN for all uncounted/node-locked)");
			}
			else if (numservers > 0)
				fprintf(ofp, 
				" (for 3-server redundant server license,\n\tor RETURN for single server)");
			fprintf(ofp, ": ");
			getinput(hname, ifp);
			if (!*hname) 
			{
				if (numservers == 2)
				{
					fprintf(ofp, 
					  "You have entered %d server hosts. ", 
								numservers);
					fprintf(ofp, 
				     "Only 0, 1, or 3 servers are legal!\n");
					continue;
				}
				else
					break;
			}

			if ((numservers == 1)
			 && (strcmp(license_fmt_ver, LM_BEHAVIOR_V6) >= 0)) 
			{
/*
 *				2nd of redundant server set, use RED_PORT
 *				for a port number
 */
				port_used = red_port = RED_PORT;
				fprintf(ofp,
				"Redundant servers, using port number %d\n",
								red_port);
				out--; /* backup over newline */
				sprintf(out, " %d\n", red_port);
				out += strlen(out);
			}


			id[0] = '\0';
			while (id[0] == '\0')
			{
				fprintf(ofp, "Enter the hostid: ");
				getinput(id, ifp);
			}

#ifndef VMS_STYLE_LICENSE_FILE
			port = dport;
			if (use_tcp && 
                                strcmp(license_fmt_ver, LM_BEHAVIOR_V6) < 0) 
			{
				fprintf(ofp, "Enter the TCP/IP port number [%d]: ", port);
				getinput(resp, ifp);
				if (resp[0] != '\0')
				{
					port = atoi(resp);
					if (port == 0)
						port = dport;
				}
				port_used = dport = port;
				sprintf(out, "SERVER %s %s %d", hname, 
								id, port); 
			}
			else if (use_tcp)
                                sprintf(out, "SERVER %s %s", hname, id); 
                        else
                                sprintf(out, "SERVER %s %s SPX:%d", hname, id,
                                                port); 
			out += strlen(out);
			
#else
			sprintf(out, "SERVER %s %s", hname, id); 
			out += strlen(out);
#endif
			if (red_port >= 0 )
			{
				sprintf(out, " %d", red_port); 
				out += strlen(out);
			}
			sprintf(out, "\n"); 
			out++;
				
			
			if (++numservers >= maxservers) break;
		}
		if (numservers)
		{
			strcpy(our_daemon, VENDOR_NAME);
			strcpy(resp, VENDOR_NAME);
			fprintf(ofp, "\n");
		}
		if (*outbuf)
			fprintf(ofp, "==>%s\n", outbuf);
		start_of_daemon = out;
/*
 *		If counted and pre-v6
 */
#ifndef VMS_STYLE_LICENSE_FILE
		if (numservers)
		{
			if (strcmp(license_fmt_ver, LM_BEHAVIOR_V6) < 0)
#endif
			{
				fprintf(ofp, "Enter %s \"%s\" path: ", vendor_str, resp);
				getinput(resp2, ifp);
#ifdef VMS_STYLE_LICENSE_FILE
				fprintf(ofp, "Enter the DECnet known object number [%s]: ",
								OBJECT);
				getinput(obj, ifp);
				if (obj[0] == '\0')
				{
					strcpy(obj, OBJECT);
				}
#endif /* VMS_STYLE_LICENSE_FILE */
				sprintf(out, "%s %s %s %s\n", vendor_str,
					resp, resp2,
#ifdef VMS_STYLE_LICENSE_FILE
							obj
#else
							""
#endif /* VMS_STYLE_LICENSE_FILE */
				);
			}
			else
			{
				sprintf(out, "%s %s\n", vendor_str, 
					VENDOR_NAME);
			}
			out += strlen(out);
		}
/*
 *		Close and re-open so the shell commands will work
 */
#ifndef VMS
		fclose(outfp);
		outfp = fopen(outfilename, "a");
#endif
	}
	if (numservers)
		served = 1;
	if (start_of_daemon)
		fprintf(ofp, "\n==>%s\n", start_of_daemon);
	if (!*our_daemon)
		strcpy(our_daemon, VENDOR_NAME); /* Need name here */
	if (!served)
	{
		fprintf(ofp, 
	   "\nNo SERVER name, all licenses will be node-locked, uncounted\n\n");
	}
 
	if (served) 
	{
		strcpy(out, "USE_SERVER\n");
		fprintf(ofp, "==>%s", out);
		out += strlen(out);
	}

	fprintf(ofp, "\nYou will be prompted for feature names.\n");
	fprintf(ofp, " When the list is complete, just type <CR> to the Feature name prompt.\n");
	fprintf(ofp, " To get the default answer to any question, type a <CR>.\n");
	fprintf(ofp, "To re-start specifying the feature, just type \"-\" to any of the prompts.\n");
	strcpy(one_feature, outbuf);
	outbufp = out;
	start_of_feature = &one_feature[strlen(one_feature)];
	while (1)
	{
	  char feat[MAX_USER_INPUT+1];
	  int validdate = 0;

		*sav_errmsg = 0;
		out = start_of_feature;

		fprintf(ofp, "\nFeature name: ");
		getinput(feat, ifp);
		if (feat[0] == '\0')
			break;
		if ((feat[0] != '_') && !isalnum(feat[0]))
		{
			fprintf(ofp, "Legal feature names must start with '_', a letter\n");
			fprintf(ofp, "or a number.  This feature name may not operate\n");
			fprintf(ofp, "correctly with future versions of FLEXlm\n");
			continue;
		}

		if (strcmp(license_fmt_ver, LM_BEHAVIOR_V2) > 0)
		{
			fprintf(ofp, 
		"FEATURE/INCREMENT/UPGRADE line? [f/i/u, default: f]: ");
			getinput(resp, ifp);
			if (resp[0] == '-')
				continue;
			if (*resp == 'i' || *resp == 'I')
				type = CONFIG_INCREMENT;
			else if (*resp == 'u' || *resp == 'U')
				type = CONFIG_UPGRADE;
			else
				type = CONFIG_FEATURE;

			if (type == CONFIG_UPGRADE)
			{
				fprintf(ofp, "UPGRADING FROM which version of %s [default %s]: ", 
								feat, fromversion);
				getinput(resp, ifp);
				if (resp[0] == '-')
					continue;
				if (resp[0] != '\0') 
				{
					strncpy(fromversion, resp, MAX_VER_LEN);
					fromversion[MAX_VER_LEN] = '\0';
				}
				fprintf(ofp, "New ");	/* Goes with "version ... */
			}
		}
		else type = CONFIG_FEATURE;

		fprintf(ofp, "Version of %s [default %s]: ", feat, version);
		getinput(resp, ifp);
		if (resp[0] == '-')
			continue;
		if (resp[0] != '\0') 
		{
			strncpy(version, resp, MAX_VER_LEN);
			version[MAX_VER_LEN] = '\0';
		}

		{
			strcpy(our_daemon, VENDOR_NAME);
		}

numlic:
		if (!served)
			nlic = 0;
		else
		{
			fprintf(ofp, "Number of licenses [default: %d]: ", nlic);
			getinput(resp, ifp);
			if (resp[0] == '-' && resp[1] == '\0')
				continue;
			if (resp[0] != '\0') 
			{
				nlic = atoi(resp);
			}
		}
		if (nlic == 0) 
		{
			if (type == CONFIG_UPGRADE)
			{
				fprintf(ofp, "UPGRADE not allowed for node-locked, uncounted licenses\n");
				fprintf(ofp, "Simply issue a new FEATURE line\n");
				continue;
			}
			fprintf(ofp, 
			"You have specified node-locked, uncounted license.\n");
			*resp = 0;
			while (!*resp || *resp == '?')
			{
				fprintf(ofp,
				"HOSTID(s) (required [? for help], or RETURN to enter new count)\n\t: ");
				getinput(resp, ifp);
				if (!*resp)
					goto numlic;
				if (*resp == '?')
				{
					hostid_help();
				}
			}

			nlic = 0;
			strcpy(lock_hostid, resp);
		}
		else 
		{
			counted = 1;
			if (!served)
			{
				fprintf(ofp, "Error:  hostname required for ");
				fprintf(ofp, "counted licenses, skipping ");
				fprintf(ofp, "this feature\n");
				continue;
			}

		}

/*
 *		Get the expiration date
 */
		validdate = 0;
		while (!validdate)
		{
			fprintf(ofp, "Expiration date [dd-mmm-yyyy, default: %s]: ",
								expdate);
			getinput(resp, ifp);
			if (resp[0] == '\0') 
			{
				strcpy(resp, expdate);
			}
			validdate = 1;
		}
		strcpy(expdate, resp);

		{
			if (!*startdate)
			{
			  int day, month, year; 
#ifdef THREAD_SAFE_TIME
			  struct tm tst;
#endif
			  struct tm *t;
			  time_t x;
				x  = (long) time(0);
#ifdef THREAD_SAFE_TIME
				localtime_r(&x, &tst);
				t = &tst;
#else /* !THREAD_SAFE_TIME */
				t = localtime(&x);
#endif
				day = t->tm_mday;
				month = t->tm_mon;
				year = t->tm_year;
				sprintf(startdate, "%d-%s-%d",
					day, months[month], year + 1900);
			}
		}
#ifdef PROMPT_FOR_START_DATE 
/*
 *		Get the start date
 */
		validdate = 0;
		while (!validdate)
		{
		  int status;
			fprintf(ofp, "Start date [dd-mmm-yyyy, default: %s]: ",
								startdate);
			getinput(resp, ifp);
			if (resp[0] == '\0') 
			{
				strcpy(resp, startdate);
			}
			status = l_validdate(lm_job, resp);
			if (status != 0 && status != LM_LONGGONE)
			{
			    fprintf(ofp, "%s is not a valid date.  Please enter as dd-mmm-yyyy\n",
									resp);
			    fprintf(ofp, "for example: 12-feb-91\n");
			    if (status == LM_DATE_TOOBIG)
			    {
				fprintf(ofp, "Maximum startdate year is %d\n",
						MAX_BINDATE_YEAR);
			    }
			    if (status == LM_BADDATE)
			    {
			      fprintf(ofp, " (If a 2-digit year is entered, 1900 is added to the year)\n");
			    }
			}
			else
			{
				validdate = 1;
			}
		}
		strcpy(startdate, resp);
#endif
		
#ifdef VMS
		fclose(outfp);
		outfp = fopen(outfilename, "a");
		if (outfp == (FILE *) NULL) perror("open output license file");
#endif
		
		strcpy(out, 
				type == CONFIG_FEATURE ? LM_RESERVED_FEATURE :
				type == CONFIG_UPGRADE ? LM_RESERVED_UPGRADE :
				LM_RESERVED_INCREMENT ); out += strlen(out);
		sprintf(out, " %s %s", feat, our_daemon); out += strlen(out);
		if (type == CONFIG_UPGRADE) 
		{
			sprintf(out, " %s" , fromversion); out += strlen(out);
		}
		sprintf(out, " %s %s %d ", version, expdate, nlic); 
		out += strlen(out);
		if (oldkey) 
		{
			sprintf(out, " start:%s", startdate); 
			out += strlen(out);
		}
		if(strcmp(license_fmt_ver, LM_BEHAVIOR_V2) == 0)
		{
			fprintf(ofp, 
			"Your vendor-defined string or RETURN for none: ");
			getinput(resp, ifp);
			sprintf(out, " \"%s\"", resp);
			out += strlen(out);
			if (!*lock_hostid && 
				(strcmp(license_fmt_ver, LM_BEHAVIOR_V2) == 0))
			{
				fprintf(ofp, "Host id (if locking application to a single host): ");
				getinput(resp, ifp);
				if (*resp) strcpy(lock_hostid, resp);
			}
			if (lock_hostid)
			{
				sprintf(out, " %s", lock_hostid);
				out += strlen(out);
			}
		}
		else if (*lock_hostid)
		{
			sprintf(out, " HOSTID=%s", lock_hostid);
			out += strlen(out);
		}
		if (newkey)
		{
			sprintf(out, " SIGN=0");
			out += strlen(out);
		}
		rc = lc_cryptstr(lm_job, one_feature, &return_str, /*&clearcodes*/ &site_code, 
			crypt_flags, 0, &errors);
		
		if ((!errors || (errors && strcmp(errors, sav_errmsg))) &&
			(rc || !strcmp(one_feature, return_str)))
		{
			if (errors)
				strncpy(sav_errmsg, errors, 100);
			fprintf(ofp, "Error ");
			if (errors)
				fprintf(ofp, errors);
			fprintf(ofp, 
			"\nDo you want to keep this line anyway (y/n)? ");
			lc_free_mem(lm_job, errors);
			getinput(resp, ifp);
			if (*resp != 'y' && *resp != 'Y')
			{
				fprintf(ofp, "Skipping this line...\n");
				lc_free_mem(lm_job, return_str);
				continue;
			}
		}
		if (errors)
			lc_free_mem(lm_job, errors); 
		lc_free_mem(lm_job, return_str); 
		if (strcmp(license_fmt_ver, LM_BEHAVIOR_V3) >= 0)
		{
			while (add_optional_attrs(one_feature, 
							/*&clearcodes*/&site_code))
				;
		}
		rc = lc_cryptstr(lm_job, one_feature, &return_str, /*&clearcodes*/&site_code, 
			LM_CRYPT_NO_LINE_NUMBERS|crypt_flags, 0, &errors);
		if ((!errors || (errors && strcmp(errors, sav_errmsg))) &&
			(rc || !strcmp(one_feature, return_str)))
		{
			if (errors)
				strncpy(sav_errmsg, errors, 100);
			fprintf(ofp, "Error ");
			if (errors)
				fprintf(ofp, errors);
			fprintf(ofp, 
			"\nDo you want to keep this line anyway (y/n)? ");
			lc_free_mem(lm_job, errors);
			errors = NULL;
			getinput(resp, ifp);
			if (*resp != 'y' && *resp != 'Y')
			{
				fprintf(ofp, "Skipping this line...\n");
				lc_free_mem(lm_job, return_str);
				continue;
			}
		}
		if (errors) lc_free_mem(lm_job, errors);
		{
		  char *cp = &return_str[strlen(return_str) - 1];

			if (*cp == '\n')
				cp--;
			while ((cp >= return_str) && 
				((*cp != '\n') || (*cp == '\n' &&
					cp > return_str && cp[-1] == '\\'))) 
				cp--;
			cp++;

			fprintf(ofp, "==>%s", cp);
			strcat(outbufp, cp);
			outbufp += strlen(outbufp);
			if (errors) 
				lc_free_mem(lm_job, errors); 
			lc_free_mem(lm_job, return_str); 
		}
	}
	out = outbufp;
	user_info_tag(out);
	if (counted || FLOAT_OK) fprintf(outfp, outbuf);
	else 
	{
		if (*hname)
		{
			fprintf(ofp, "All licenses are uncounted, ");
			fprintf(ofp, "SERVER/%s lines not needed\n", vendor_str);
		}
		fprintf(outfp, &outbuf[start_of_feature - one_feature]);
	}
	fclose(outfp);
#ifdef VMS
	{
	  char systembuff[MAX_USER_INPUT+1];

		sprintf(systembuff, "purge %s", outfilename);
		system(systembuff);
	}
#endif
	fprintf(ofp, "License is %s\n", outfile);
	exit(0);
	return 0;
}


static
int
yes()
{
  char resp[MAX_USER_INPUT+1];

	getinput(resp, ifp); 
	if (!strcmp(resp, "Y") || !strcmp(resp, "y"))
		return(1);
	else
		return(0);
}


static
void
user_info_tag(out)
char *out;
{
	sprintf(out, 
	"\n#NOTE: You can edit the hostname on the server line (1st arg).\n"); 
	out += strlen(out);
	sprintf(out,
	"#      The (optional) daemon-path on the VENDOR line (2nd arg).\n"); 
	out += strlen(out);
	if (port_used > 0 )
	{
		sprintf(out,
		"#      And the port number (%d) on the SERVER line.\n",
			port_used);
		out += strlen(out);
	}
	sprintf(out,
	"#      Most other changes will invalidate this license.\n"); 
	out += strlen(out);
}

void 
grep_server_daemon( src_file, dest )
char *src_file;
char *dest;
{
	FILE *src_fp;
	char linebuff[MAX_CONFIG_LINE];

	if ( !(src_fp=fopen( src_file, "r" )) )
	{
		fprintf(ofp,  "Can't open %s for read.", src_file );
		return;
	}


	while( fgets( linebuff,  MAX_CONFIG_LINE, src_fp ) )	/* overrun checked */
	{
		if ( !strncmp(linebuff, "SERVER", 6) )
		{
			fprintf(ofp, "==>%s", linebuff);
			strcat( dest, linebuff );
			continue;
		}
		if ( !strncmp(linebuff, vendor_str, 6) )
		{
			fprintf(ofp, "==>%s", linebuff);
			strcat( dest, linebuff );
			continue;
		}
	}
	fclose( src_fp );
}

/*
 * Use fopen() for compatibility
 */
static
int file_exists (filename)
char *filename;
{
        int exists = 0;         /* assume success */
	FILE *f;

        if ((f = fopen (filename, "r")))
        {
                fclose (f);
                exists = 1;
        }
        return (exists);
}

static 
void
getinput(str, stream)
char *str;
FILE *stream;
{
	if (!fgets(str, MAX_USER_INPUT, stream))	/* overrun checked */
	{
		perror("Error reading input");
		exit(1);
	}
	if (record) 
	{
		if (!recordf)
		{
			if (!(recordf = fopen("create.inp", "w")))
			{
				perror("Can't open create.inp");
				exit(1);
			}
		}
		fputs(str, recordf);
	}
	str[strlen(str) - 1] = '\0'; /* strip newline */
}

static
int
add_optional_attrs(feat_str, codeptr)
char *feat_str;
VENDORCODE *codeptr;
{
  char newattr[MAX_USER_INPUT];	/* overrun checked */
  int rc;
  char feature[MAX_CONFIG_LINE];
  char *return_str = NULL, *errors = NULL;

	strcpy(feature, feat_str);
	fprintf(ofp, "Enter an optional attribute [? to list]: ");
	getinput(newattr, ifp);
	if (!*newattr)
		return 0;
	if (*newattr == '?')
	{
		fprintf(ofp, "\tDUP_GROUP=NONE|SITE|[UHDV]\n");
		fprintf(ofp, "\tHOSTID=\"hostid_1 [... hostid_n]\"\n");
		fprintf(ofp, "\tISSUER=\"...\"\n");
		fprintf(ofp, "\tNOTICE=\"...\"\n");
		fprintf(ofp, "\tVENDOR_STRING=\"...\"\n");
		fprintf(ofp, "\tasset_info=\"...\"\n");
		fprintf(ofp, "\tdist_info=\"...\"\n");
		fprintf(ofp, "\tuser_info=\"...\"\n");
		fprintf(ofp, "\tvendor_info=\"...\"\n");
		if (strcmp(license_fmt_ver, LM_BEHAVIOR_V4) >= 0)
		{
			fprintf(ofp, "\tOVERDRAFT=n\n");
		}
		if (strcmp(license_fmt_ver, LM_BEHAVIOR_V5) >= 0)
		{
			fprintf(ofp, "\tCAPACITY\n");
			fprintf(ofp, "\tHOST_BASED|USER_BASED[=n]\n");
			fprintf(ofp, "\tISSUED=dd-mmm-yyy\n");
			fprintf(ofp, "\tMINIMUM=n\n");
			fprintf(ofp, "\tSN=serial-number\n");
			fprintf(ofp, "\tSUITE_DUP_GROUP=NONE|SITE|[UHDV]\n");
			if (strcmp(license_fmt_ver, LM_BEHAVIOR_V5_1) >= 0)
				fprintf(ofp, 
			     "\tSUPERSEDE[=\"feat_1 [... feat_n]\"|*ALL*]\n");
			else
				fprintf(ofp, "\tSUPERSEDE\n");
		}
		if (strcmp(license_fmt_ver, LM_BEHAVIOR_V5_1) >= 0)
		{
			fprintf(ofp, "\tPLATFORMS=\"...\"\n");
			fprintf(ofp, "\tID=n\n");
		}
		if (strcmp(license_fmt_ver, LM_BEHAVIOR_V6) >= 0)
		{
			fprintf(ofp, "\tSTART=\"...\"\n");
		}
		fprintf(ofp, "\t    NOTE: quotes (\"\") are required if =\"...\" contains spaces\n");
		return -1;
	}
	if(!strcmp (newattr,"FLOAT_OK"))
	{
		FLOAT_OK=1;
	}
	sprintf(feature, "%s %s", feat_str, newattr);
	rc = lc_cryptstr(lm_job, feature, &return_str, codeptr, 
		LM_CRYPT_NO_LINE_NUMBERS|crypt_flags, 0, &errors);
	if ((!errors || (errors && strcmp(errors, sav_errmsg))) && rc)
	{
		if (errors)
			strncpy(sav_errmsg, errors, 100);
		fprintf(ofp, "Error ");
		if (errors)
			fprintf(ofp, errors);
		fprintf(ofp, 
		"\nDo you want to keep this attribute anyway (y/n)? ");
		lc_free_mem(lm_job, errors);
		errors = NULL;
		getinput(resp, ifp);
		if (*resp != 'y' && *resp != 'Y')
		{
			fprintf(ofp, "Skipping this attribute...\n");
			lc_free_mem(lm_job, return_str);
			return -1;
		}
	}
	if (errors)
		lc_free_mem(lm_job, errors);
	sprintf(feat_str, "%s %s", feat_str, newattr);
	if (errors)
		lc_free_mem(lm_job, errors); 
	lc_free_mem(lm_job, return_str); 
	return 1;
}

static
void
hostid_help()
{
	fprintf(ofp, "\tHostid(s) are obtained from ");
	fprintf(ofp, "lmhostid, or see manual for other methods\n");
	fprintf(ofp, 
	"\tThis can be a list of hostids (\"id1 ... idn\")\n");
	fprintf(ofp, "\tAlternatives include (explained in manual):\n");
	fprintf(ofp, "\t\tDEMO (runs anywhere)\n");
	fprintf(ofp, "\t\tANY (runs anywhere)\n");
	fprintf(ofp, "\t\tID=n (runs anywhere)\n");
	fprintf(ofp, "\t\tHOSTNAME=string\n");
	fprintf(ofp, "\t\tUSER=string\n");
	fprintf(ofp, "\t\tDISPLAY=string\n");
	fprintf(ofp, "\t\tINTERNET=nnn.nnn.nnn.nnn ('*' wildcard allowed)\n");
}
static
void
usage()
{
fprintf(stderr,"usage: makekey [-verfmt ver] [-decimal]\n");
fprintf(stderr,"   -verfmt prints in old format and prints errors when \n");
fprintf(stderr,"   is new than specified. Version formats are 2, 3, 4, 5, 5.1 and 6\n");
}
