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

 *****************************************************************************/
/*
 *	Module: $Id: lmutil.c,v 1.63.2.5 2003/07/01 17:04:22 sluu Exp $
 *
 *	Function: lmutil: lmborrow/lmcksum/lmdiag/lmdown/lmremove/lmswitch/
 *			  lmswitchr/lmreread/lmhostid
 *
 *	Description: borrow licenses, checksum, shutdown, remove user,
 *		     switch report, switch log, reread, diagnostics
 *
 *	Parameters:	[-c license_file] [-v] [-k]
 *
 *	M. Christiano
 *	5/21/88
 *
 *	Last changed:  12/7/98
 *
 */

#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif
#include <signal.h>
#include "lmachdep.h"
#include <stdio.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "l_openf.h"
#include "utilscode.h"
#include "lm_attr.h"
#include "flex_file.h"
#include "flex_utils.h"
#include <sys/types.h>
#include <stdlib.h>
#ifdef PC
#include <string.h>
#include <io.h>
#include <winsock.h>
#else
#ifndef apollo
#include <netinet/in.h>
#endif /* apollo */
#endif /* PC */
#include "lsmaster.h"
#ifdef MOTO_88K
#include "lm_select.h"
#endif
/*#include <fcntl.h>*/
#if defined (MOTO_88K) || defined (sco) || defined(sinix) || defined(RS6000) || defined(PC)
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifndef  NO_UIO_H
#include <sys/uio.h>
#endif
/* #include <sys/socket.h>
#include <netdb.h>  */
#include <errno.h>
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif
#if defined(HP) || defined(PCRT) || defined(MOTO_88K)
#include <sys/file.h>
#endif


FILE *ofp;
int printheader = 1;
int printversion = 0;
void license_info lm_args((int, CONFIG *));
int lmborrow_main lm_args((int argc, char *argv[]));
int lmpath_main(int argc, char **argv);
int lmdiag_main lm_args((int argc, char *argv[]));
int lmhostid_main lm_args((int argc, char *argv[]));
int lmstat_main lm_args((int argc, char *argv[]));
int lmver_main lm_args(( char *path ));
int  get_first_char lm_args((FILE *file));
int  get_string lm_args((char *str, FILE *file, char **cpp, int max));
int  cmd_type lm_args(( char *cmd ));
int  lm_cksum lm_args((char *daemon));
void (*dummy)() = 0;
/*char *l_key_callback;
char *l_key_gen_callback;*/
char l_vendor[MAX_VENDOR_NAME + 1];
static void reject_extra_args lm_args(( int,  char *[]));

lm_extern int API_ENTRY lmutil_checkout lm_args((LM_HANDLE_PTR job,
			       LM_CHAR_PTR feature,
			       LM_CHAR_PTR  version, int nlic, int flag,
			       VENDORCODE_PTR key, int dup));
#ifndef OPENVMS
  FILE *fopen();
#endif

#ifdef PC
#define CHAR_SLASH	'\\'
#else
#define CHAR_SLASH  '/'
#endif

LM_DATA;			/* Declare lm_job, etc. */
static int process_args();
void usage();
#ifndef ANSI
char *strcpy();
char *strncpy();
#endif

int diag_interactive = 1;	/* Interactive lmdiag */

/*
 *	Order of arguments for lmremove
 */
int remove_handle = 0;	/* Default to using feature/user/host/disp */

#define CMD_INVALID	0
#define	CMD_LMCKSUM	1
#define CMD_LMDOWN	2
#define CMD_LMHOSTID	3
#define CMD_LMREMOVE	4
#define CMD_LMREREAD	5
#define CMD_LMSWITCH	6
#define CMD_LMSWITCHR	7
#define CMD_LMSTAT	8
#define CMD_LMVER	9
#define CMD_LMBORROW	10
#define CMD_LMDIAG	11
#define CMD_LMNEWLOG	12
#define CMD_LMINSTALL	13
#define CMD_LMPATH	14
#define ALL_LMGRDS (LMGRD_STAT *)1

#define MAXARG 15
char *copyright =  COPYRIGHT_STRING(1989)"\n";

int verbose = 0;
int lmdown_prompt = 0;
int do_all = 0;
static int got_dash_q;
int lmdown_force;
char *myname;
#ifndef ANSI
extern char *strrchr();
#endif

int exit_code = 0;

int
main(int argc, char * argv[])
{
	int i = 0, d = 0, cmd = 0;
	char msg[LM_MSG_LEN+1];
	char *feature = NULL;
	char *newargv[MAXARG];
	int newargc = 0;
	char *origname = NULL;
#ifdef VMS
	DAEMON *dl = NULL;
#endif
#if defined (PC) && !defined(WINNT) && !defined(OS2)
	struct _wsizeinfo wsize;
	char arg_buffer[256] = {'\0'};
	char *arg_vector[MAXARG+1];
restart:
#endif /* defined (PC) && !defined(WINNT) && !defined(OS2) */
	char **	flexArgv = NULL;
	char **	tmpArgv = NULL;
	int		flexArgc = 0;
	int		tmpArgc = 0;


#ifdef PC
	flexArgv = l_getUTF8Cmdline(NULL, &flexArgc);
	if(flexArgv == NULL)
	{
		fprintf(stderr, "Initialization error, exiting.\n");
		fflush(stderr);
		exit(0);
	}
	tmpArgv = flexArgv;
	tmpArgc = flexArgc;
#else /* !PC */
	tmpArgv = flexArgv = argv;
	tmpArgc = flexArgc = argc;
#endif

#ifdef VMS
	myname = strrchr(tmpArgv[0], ']');	/* Remove leading ddnn:[dir] */
	if (myname)
		myname++;
	else
		myname = tmpArgv[0];
	feature = strrchr(myname, '.');
	if (feature)
		*feature = '\0';	/* Remove trailing .exe;... */
#else
	myname = strrchr(tmpArgv[0], CHAR_SLASH );
	if (myname)
		myname++;
	else
		myname = tmpArgv[0];
	feature = strrchr(myname, '.');
	if (feature)
		*feature = '\0';	/* Remove trailing .exe */
#endif
	origname = myname;

#ifdef PC
	for ( i=0; myname[i]; i++ )
		myname[i] = tolower( myname[i] );
#if !defined(WINNT) && !defined(OS2)
	/*
	 * Set QuickWin window behavior customization.
	 */
	_wsetexit( _WINEXITPROMPT );
	_wabout( "FLEXlm Utilities version 3.1\n" COPYRIGHT_STRING(1988));
	wsize._version = _QWINVER;
	wsize._type = _WINSIZEMAX;
	_wsetsize( _wgetfocus(), &wsize );
#endif /* !defined(WINNT) && !defined(OS2) */
#endif /* PC */

	ofp = stdout;
	if (i = lc_init(0, "lmgrd", &code, (LM_HANDLE **) &lm_job))
	{
		printf(lmtext("lmutil: can't initialize: %s\n"),
					lmtext(lc_errstring(lm_job)));
		exit(i);
	}
	if (LM_TOOL_ERR_CATCH(lm_job))
	{
		fprintf(stdout, lmtext("Out of memory, exiting\n"));
		exit(lm_job->lm_errno);
	}
	l_text();
	if (!strncmp(myname, "lmutil", 6) && argc > 1)
	{
		tmpArgc--;
		tmpArgv++;
		if (!strcmp(tmpArgv[0], "-v"))
		{
			fprintf(ofp, "%s%s v%d.%d%s\n", copyright, myname,
				      lm_job->code.flexlm_version,
				      lm_job->code.flexlm_revision,
				      lm_job->code.flexlm_patch);
			exit(0);
		}
		myname = tmpArgv[0];
		if (!strcmp(myname, "borrow"))		myname = "lmborrow";
		else if (!strcmp(myname, "cksum"))	myname = "lmcksum";
		else if (!strcmp(myname, "diag"))	myname = "lmdiag";
		else if (!strcmp(myname, "down"))	myname = "lmdown";
		else if (!strcmp(myname, "hostid"))	myname = "lmhostid";
		else if (!strcmp(myname, "path"))	myname = "lmpath";
		else if (!strcmp(myname, "reread"))	myname = "lmreread";
		else if (!strcmp(myname, "stat"))	myname = "lmstat";
		else if (!strcmp(myname, "switchr"))    myname = "lmswitchr";
		else if (!strcmp(myname, "switch"))    myname = "lmswitch";
		else if (!strcmp(myname, "newlog"))	myname = "lmnewlog";
		else if (!strcmp(myname, "install"))    myname = "lminstall";
		else if (!strcmp(myname, "ver"))	myname = "lmver";
		else if (!strcmp(myname, "grd"))	myname = "lmgrd";
	}

	l_la_init(lm_job);

	cmd = cmd_type(myname);
	if ( exit_code = process_args(tmpArgc, tmpArgv, newargv, &newargc, cmd) )
		goto cmd_end;
	argc = newargc;
	argv = newargv;
	switch(cmd)
	{
	case CMD_LMREREAD:
	case CMD_LMDOWN:
		lm_job->flags |= LM_FLAG_LMUTIL_PRIV;
		break;
	}
	if ((cmd != CMD_LMHOSTID || printheader) && cmd != CMD_LMINSTALL)
		fprintf(ofp, "%s - %s", origname, copyright);
	if (!lm_job->options->config_file &&
		!l_getenv(lm_job, LM_DEFAULT_ENV_SPEC) &&
		cmd != CMD_LMDIAG )
	{
		char buf[LM_MAXPATHLEN + 1] = {'\0'};
		char szValue[MAX_PATH] = {'\0'};
		int doit = 1;

		if (*lm_job->alt_vendor)
		{
		  char vd_env_name[100];

			sprintf(vd_env_name, "%s_LICENSE_FILE",
							lm_job->alt_vendor);
			if (l_getEnvUTF8 (lm_job, vd_env_name, szValue, sizeof(szValue)))
				doit = 0;
			else
			{
				l_uppercase(vd_env_name);
				if (l_getEnvUTF8 (lm_job, vd_env_name, szValue, sizeof(szValue)))
					doit = 0;
			}
		}
		if (doit)
		{
			switch ( cmd )
			{
			case CMD_LMDIAG:
			case CMD_LMSTAT:
			case CMD_LMDOWN:
			case CMD_LMREMOVE:
			case CMD_LMSWITCH:
			case CMD_LMSWITCHR:
			case CMD_LMNEWLOG:
			case CMD_LMREREAD:
#ifdef UNIX
			sprintf(buf, "%s%c@%s%c.", LM_DEFAULT_LICENSE_FILE,
					PATHSEPARATOR,
					lc_hostname(lm_job, 1), PATHSEPARATOR);
#else
			sprintf(buf, "%s%c.", LM_DEFAULT_LICENSE_FILE,
					PATHSEPARATOR);
#endif
			lc_set_attr(lm_job, /*LM_A_LICENSE_FILE*/
				LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
			lm_job->options->disable_env = 0;
			}
		}
	}
	if (printversion)
	{
		fprintf(ofp, "%s v%d.%d%s\n", myname,
					      lm_job->code.flexlm_version,
					      lm_job->code.flexlm_revision,
					      lm_job->code.flexlm_patch);
		exit(0);
	}

	switch ( cmd )
	{
	case CMD_LMBORROW:
		exit (lmborrow_main(argc, argv));
		break;

	case CMD_LMCKSUM:
		{
		  char *daemon = "";

			if (newargc > 1)
				daemon = newargv[1];
		        else if (newargc > 2)
		        {
			        fprintf(ofp, "Unknown arguments %s ...\n",
                                                newargv[2]);
                                usage();
                                exit(1);
                        }


			exit_code = lm_cksum(daemon);
		}
		break;

	case CMD_LMDIAG:
		exit_code = lmdiag_main(argc, argv);
		break;

	case CMD_LMSTAT:
		exit_code = lmstat_main(argc, argv);
		break;

	case CMD_LMDOWN:
		verbose = 1;
		if (!got_dash_q) lmdown_prompt = 1;
		reject_extra_args(newargc, newargv);
        exit_code = lmutil_down();
		break;

	case CMD_LMHOSTID:
		exit_code = lmhostid_main(argc, argv); break;

	case CMD_LMPATH:
		exit_code = lmpath_main(newargc, newargv); break;

	case CMD_LMREMOVE:
		exit_code = lmutil_remove(newargc, newargv); break;

	case CMD_LMSWITCH:
	case CMD_LMSWITCHR:
	case CMD_LMNEWLOG:
		lmutil_switchr(newargc, newargv, cmd); break;

	case CMD_LMINSTALL:
		lmutil_install(newargc, newargv); break;

	case CMD_LMREREAD:
		reject_extra_args(newargc, newargv);
		lmutil_reread(newargc, newargv);
		break;

	case CMD_LMVER:
		{
			if (argc < 2)
			{
				fprintf(ofp,
				lmtext("No binary specified, exiting\n"));
				usage();
				exit(1);
			}
			else if (argc > 2)
			{
				fprintf(ofp, "Unknown arguments %s ...\n", argv[2]);
				usage();
				exit(1);
			}
			else if ((argc == 2) && (!strcmp(argv[1], "-help")))
			{
				usage();
				exit(1);
			}
			else
				exit_code = lmver_main( argv[1] );
		}
		break;

	default:
		usage();
		break;
	}

cmd_end:
#ifdef PC
	if(flexArgv && flexArgc)
	{
		l_freeUTF8Cmdline(flexArgc, flexArgv);
		flexArgv = NULL;
	}
#endif
	lm_free_job( lm_job );
	lm_job = 0;
#if defined (PC) && !defined(WINNT) && !defined(OS2)

	/*
	 * Allow QuickWin user to re-enter the command.  Otherwise,
	 * re-setting parameters for QuickWin apps can be very
	 * inconvenient to users.
	 */

	fprintf(ofp,  "\n\nEnter command or press ENTER to exit:\n" );
	(void) gets( arg_buffer );
	if ( strlen( arg_buffer ) )
	{
		char *cp = arg_buffer;
		argv = arg_vector;

		for ( argc=0; argc<MAXARG;  )
		{
			while ( (*cp==' ') || (*cp=='\t') )
				cp++;

			if (!*cp)
				break;
			*argv++ = cp;
            argc++;

			/* Find the end of this parameter. */
			while ( (*cp!=' ') && (*cp!='\t') && *cp)
				cp++;

			if ( !*cp )
				break;

			/* End this parameter with NULL */
			*cp++ = 0;
		}
		*argv = 0;
		argv = arg_vector;
		if ( argc )
			goto restart;
	}
#endif /* defined (PC) && !defined(WINNT) && !defined(OS2) */

#ifdef VMS
        if (exit_code == 0) exit_code = 1;      /* Make DCL happy */
#endif /* VMS */
	return(exit_code);
}



/*
 *
 *	Function: process_args(argc, argv, newargv, newargc)
 *
 *	Description: Processes the command-line arguments.
 *
 *	Parameters:	argc, argv.
 *
 *	Return:		Global variables are set.
 *
 *	M. Christiano
 *	2/12/88
 *
 *	Last changed:  12/7/98
 *
 */

static
process_args(argc, argv, newargv, newargc, cmd)
int argc;
char *argv[];
char *newargv[];
int *newargc;
int cmd;
{
  int movearg;

	*newargc = 0;
	newargv[*newargc] = argv[0];
	(*newargc)++;
	while (argc > 1)
	{
	  char *p = argv[1]+1;

		if (L_STREQ(argv[1], "-vendor") && argc > 2)
		{
			argc--; argv++;
			l_zcp(l_vendor, argv[1], MAX_VENDOR_NAME);
			l_zcp(lm_job->alt_vendor, argv[1], MAX_VENDOR_NAME);
			argc--; argv++;
			continue;
		}
		if (L_STREQ(argv[1], "-all"))
		{
			argc--; argv++;
			do_all = 1;
			continue;
		}
		if (L_STREQ(argv[1], "-force"))
		{
			lmdown_force = 1;
			argc--; argv++;
			continue;
		}
		if (L_STREQ(argv[1], "-verbose"))
		{
			argc--; argv++;
			lc_set_attr(lm_job, LM_A_LONG_ERRMSG,
						(LM_A_VAL_TYPE) 1);
			continue;
		}
		if (L_STREQ(argv[1], "-pre_v6"))
		{
			argc--; argv++;
			lc_set_attr(lm_job, LM_A_BEHAVIOR_VER,
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V5);
			continue;
		}
		movearg = 1;

		if (*argv[1] == '-' && !argv[1][2]) switch (*p)
		{
			case 'c':
				if (argc > 2 && *argv[2] != '-' )
				{
					argv++; argc--;
					lm_set_attr(LM_A_DISABLE_ENV,
							   (LM_A_VAL_TYPE)1);
					lm_set_attr(LM_A_LICENSE_FILE_PTR,
						      (LM_A_VAL_TYPE)argv[1]);
					movearg = 0;
				}
				break;

			case 'e':
				if (argc > 2 && *argv[2] != '-')
				{
					movearg = 0;
					argv++; argc--;
					if (!l_flexFreopen(lm_job, argv[1], "w", stderr))
					{
						fprintf(ofp, "Can't open %s: %s\n",
						argv[1], SYS_ERRLIST(errno));
						exit(1);
					}
				}
				break;


			case 'o':
				if (argc > 2 && *argv[2] != '-')
				{
					movearg = 0;
					argv++; argc--;
					if (!(ofp = l_flexFopen(lm_job, argv[1], "w")))
					{
						fprintf(ofp, "Can't open %s: %s\n",
						argv[1], SYS_ERRLIST(errno));

						exit(1);
					}
				}
				break;

			case 'h':
				if (cmd == CMD_LMREMOVE)
				{
					remove_handle = 1;
					movearg = 0;
				}
				break;

			case 'k':
				lm_set_attr(LM_A_CRYPT_CASE_SENSITIVE,
						   (LM_A_VAL_TYPE)1);
				movearg = 0;
				break;

			case 'n':
				diag_interactive = 0;
				printheader = 0;
				movearg = 0;
				break;

			case 'q':
				verbose = 0;
				got_dash_q = 1;
				movearg = 0;
				break;

			case 'v':

				if (!strcmp(p,"vsn"))
					printversion=0;
				else
					printversion = 1;
				break;

			default:
				break;
		}
		if (movearg)
		{
			if (*newargc > MAXARG)
			{
				usage();
				return 1;
			}
			newargv[*newargc] = argv[1];
			(*newargc)++;
		}
		argc--; argv++;
	}

	return 0;
}

void
usage()
{
  char name[30];

	l_lowercase(myname);
	if (*myname  != 'l') strcpy(myname, "lmutil");
	if (L_STREQ(myname, "lmutil"))
		strcpy(name, "lmutil ");
	else *name = 0;
	fprintf(ofp, lmtext("usage: "));
	if (L_STREQ(myname, "lmborrow") || L_STREQ(myname, "lmutil"))
		fprintf(ofp, "\
\t%slmborrow -status\n\
\t%slmborrow -clear\n\
\t%slmborrow {all|vendor} dd-mmm-yyyy:[time]\n\
\t%slmborrow -return [-c licfile] [-d display_name] feature\n", name, name, name, name);
#if 0 /* phasing out lmcksum */
	if (L_STREQ(myname, "lmcksum") || L_STREQ(myname, "lmutil"))
		fprintf(ofp, lmtext("\t%slmcksum [-c licfile] [-k] [-pre_v6]\n"), name);
#endif
	if (L_STREQ(myname, "lmdiag") || L_STREQ(myname, "lmutil"))
                fprintf(ofp, lmtext("\t%slmdiag [-c licfile] [-n]\n"), name);
	/* Adding detailed usage information for lmdown.
	if (L_STREQ(myname, "lmdown") || L_STREQ(myname, "lmutil")) */
    if (L_STREQ(myname, "lmutil"))
                fprintf(ofp, lmtext("\t%slmdown [-c licfile] [-q] [-all] [-vendor name] [-force] [-help]\n"), name);
    if (L_STREQ(myname, "lmdown"))
	{
		fprintf(ofp, "\tlmdown [-c licfile] [-q] [-all] [-vendor name] [-force] [-help]\n\n");
        fprintf(ofp, "\t[-c license_file_list]  Use the specified license file(s).\n");
        fprintf(ofp, "\tSpecifying -c license_file_list is always recommended with lmdown.\n");
        fprintf(ofp, "\t[-vendor vendor_daemon]  Shut down only this vendor daemon;\n");
		fprintf(ofp, "\t                         lmgrd continues running.\n");
        fprintf(ofp, "\t[-q]     Do not prompt for confirmation or print a header.\n");
        fprintf(ofp, "\t[-all]   If multiple servers are specified,\n");
		fprintf(ofp, "\t         automatically shuts down all of them.\n");
        fprintf(ofp, "\t         -q is implied with -all.\n");
        fprintf(ofp, "\t[-force] If licenses are borrowed, lmdown runs only from the machine \n");
        fprintf(ofp, "\t         where the license server is running, and then only if the \n");
        fprintf(ofp, "\t         user adds -force.\n");
		fprintf(ofp, "\t[-help]  Display usage information.\n\n");
	}
	if (L_STREQ(myname, "lmhostid") || L_STREQ(myname, "lmutil"))
#if defined (HP) || defined (OPENBSD)
                fprintf(ofp, "\
	%slmhostid [-ether|-internet|-user|-n|\n\
	          -display|-hostname|-string|-long]\n\n", name);
#else
#ifdef PC
		        fprintf(ofp, "\
	%slmhostid [-ether|-internet|-user|-n|-string\n\
	          -display|-hostname|-vsn|-flexid|-long|-utf8]\n", name);
#else
                fprintf(ofp, "\
	%slmhostid [-internet|-user|-display|-n|\n\
	          -hostname|-string|-long]\n\n", name);

#endif /* PC */
#endif /* HP || OPENBSD */

	if (L_STREQ_N(myname, "lminst", 6) || L_STREQ(myname, "lmutil"))
		fprintf(ofp, "\
	%slminstall [-i infile] [-o outfile]\n\
		[-overfmt {2, 3, 4, 5, 5.1, 6, 7.1, 8}]\n\
		[-odecimal] [-maxlen n]\n", name);
	if (L_STREQ(myname, "lmnewlog") || L_STREQ(myname, "lmutil"))
	{
                fprintf(ofp, "\t%slmnewlog [-c licfile] vendor new-file, or\n", name);
                fprintf(ofp, "\t%slmnewlog [-c licfile] feature new-file\n", name);
	}
	if (L_STREQ(myname, "lmpath") || L_STREQ(myname, "lmutil"))
	{
		fprintf(ofp, "\
	%slmpath -status\n\
	%slmpath -override {all | vendor } path\n\
	%slmpath -add {all | vendor } path\n",
		name, name, name);
	}
	if (L_STREQ(myname, "lmremove") || L_STREQ(myname, "lmutil"))
	{
                fprintf(ofp, lmtext("\t%slmremove [-c licfile] feature user host display\n"), name);
                fprintf(ofp, lmtext("\t%slmremove [-c licfile] -h feature host port handle\n"), name);
	}
	if (L_STREQ(myname, "lmreread") || L_STREQ(myname, "lmutil"))
                fprintf(ofp, "\t%slmreread [-c licfile] [-vendor name] [-all]\n", name);
	if (L_STREQ(myname, "lmswitchr") || L_STREQ(myname, "lmutil"))
	{
                fprintf(ofp, "\t%slmswitchr [-c licfile] vendor new-file, or\n", name);
                fprintf(ofp, "\t%slmswitchr [-c licfile] feature new-file\n", name);
	}
	if (L_STREQ(myname, "lmstat") || L_STREQ(myname, "lmutil"))
                fprintf(ofp, lmtext("\t%slmstat [-c licfile] [lmstat-args]\n"), name);
	if (L_STREQ(myname, "lmswitch") || L_STREQ(myname, "lmutil"))
	{
                fprintf(ofp, "\t%slmswitch [-c licfile] vendor new-file, or\n", name);
                fprintf(ofp, "\t%slmswitch [-c licfile] feature new-file\n", name);
	}
	if (L_STREQ(myname, "lmver") || L_STREQ(myname, "lmutil"))
		        fprintf(ofp, lmtext("\t%slmver flexlm_binary\n"), name);
	if (L_STREQ(myname, "lmutil"))
	{
                fprintf(ofp, lmtext("\t%s-help (prints this message)\n"), name);
				fprintf(ofp, "\t%sutility_name -help  (display detailed usage information)\n\n", name);
	}
}

cmd_type ( cmd )
char * cmd;
{
	if (!strcmp(cmd, "lmhostid"))
		return CMD_LMHOSTID;

	if (!strcmp(cmd, "lmstat"))
		return CMD_LMSTAT;

	if (!strcmp(cmd, "lmdiag"))
		return CMD_LMDIAG;

	if (!strcmp(cmd, "lmborrow"))
		return CMD_LMBORROW;

	if (!strcmp(cmd, "lmcksum"))
		return CMD_LMCKSUM;

	if (!strcmp(cmd, "lmdown"))
		return CMD_LMDOWN;

	if (!strcmp(cmd, "lmpath"))
		return CMD_LMPATH;

	if (!strcmp(cmd, "lmremove"))
		return CMD_LMREMOVE;

	if (!strcmp(cmd, "lmswitch"))
		return CMD_LMSWITCH;

	if (!strcmp(cmd, "lmnewlog"))
		return CMD_LMNEWLOG;

	if (!strcmp(cmd, "lminstall"))
		return CMD_LMINSTALL;
	if (!strncmp(cmd, "lminst~", 7))
		return CMD_LMINSTALL;

	if (!strcmp(cmd, "lmswitchr"))
		return CMD_LMSWITCHR;

	if (!strcmp(cmd, "lmswitch"))
		return CMD_LMSWITCH;

	if (!strcmp(cmd, "lmreread"))
		return CMD_LMREREAD;

	if (!strcmp(cmd, "lmver"))
		return CMD_LMVER;

	return CMD_INVALID;
}

LMGRD_STAT *
l_select_lmgrd(lmgrdp)
LMGRD_STAT **lmgrdp;
{
  LMGRD_STAT *lmgrd = *lmgrdp;

  int cnt, which = -1;
  LMGRD_STAT *lp, *this_lmgrd = 0, *last, *next;
  char buf[MAX_LONG_LEN + 1];


/*
 *	remove lmgrds that are down from list
 */
	for (last = 0, lp = lmgrd; lp; lp = next)
	{
		next = lp->next;
		if (!lp->up) /* remove it */
		{
			if (last) last->next = next;
			else lmgrd = next;
			lp->next = 0;
			lc_free_lmgrd_stat(lm_job, lp);
		}
		else last = lp;
	}
	*lmgrdp = lmgrd;


	fprintf(ofp, "\n   %-20s %s\n", "Port@Host",
		"Vendors");
	for (cnt = 1, lp = lmgrd; lp; lp = lp->next, cnt++)
	{
		fprintf(ofp, "%d) %-20s %s\n",
			cnt,
			lp->port_at_host,
			lp->vendor_daemons ? lp->vendor_daemons : "Unknown (pre-v6 lmgrd)");

	}
	if (do_all)
	{
	        fprintf(ofp, "\nAll lmgrds selected...\n");
                return ALL_LMGRDS;
        }
	cnt--;
	if (cnt > 1)
	{
		fprintf(ofp, "\nServer # [a=all, q=quit]?  ");
	}
	else if (lmdown_prompt)
	{
		fprintf(ofp, "\nAre you sure (y/n)?  ");
	}
	if (cnt > 1 || lmdown_prompt)
	{
		*buf = 0;
		fgets(buf, MAX_LONG_LEN, stdin);
		buf[MAX_LONG_LEN] = 0;
		l_lowercase(buf);
	}
	else strcpy(buf, "y");
	if (cnt > 1)
	{
	        if ((*buf == 'a') || (*buf == 'A'))
	                this_lmgrd = ALL_LMGRDS;
	        else
	        {
                        which = atoi(buf);
                        for (cnt = 1, lp = lmgrd; lp;
                                                lp = lp->next, cnt++)
                                if (which == cnt) this_lmgrd = lp;
                }
        }
	else if (*buf == 'y' || *buf == '1') this_lmgrd = lmgrd;
	return (this_lmgrd);
}
static
void
reject_extra_args(argc, argv)
int argc;
char *argv[];
{
  int gotone = 0;
  int i;
        for (i = 1;i < argc ;i++)
		  {
                gotone++;
				if (strcmp(argv[i],"-help"))   /*  for help/usage info.  */
                  fprintf(ofp, "Unknown argument: %s\n", argv[i]);

          }
        if (gotone)
	{
		usage();
		exit(1);
	}
}
