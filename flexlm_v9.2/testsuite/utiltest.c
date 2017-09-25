/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: utiltest.c,v 1.30 2003/06/12 16:57:18 sluu Exp $
 *
 *	Function: utiltest
 *
 *	Description: Test all command-line utilities
 *
 *	Instructions for adding a test to utiltest:
 *
 *		Edit utserv.cmd or utnoserv.cmd, or add a new .cmd file, 
 *		with new test.
 *			1) Add a DELIM line -- prints Delimiter, and
 *			   delimits test numbers, so you can run
 *			   tests with individual numbers
 *			2) Add PRINT lines where necessary.
 *			3) Add CMD line to execute any utility
 *				CMD 	<cmd which generate outfile>
 *			4) Add CHECK, to check output from CMD
 *				CHECK 	<outfile from CMD> <identifier> [<opt>]
 *				optional 3rd arg:  EXACT | REPLOG
 *			5) Other keywords:  SLEEP <n>, RM <file> 
 *			   FPRINT <file> <string>, 
 *			6) FLEXlm keywords: CHECKOUT, LMREMOVE
 *			7) vars:  $HOSTNAME, $DISPLAY, $USERNAME --
 *			   these come from the flexlm notion:
 *			   lc_username, etc.
 *
 *		Generate test output: 
 *			run "utiltest [ -f cmdfile ] -generate"
 *
 *		Run test
 *			run "utiltest [ -f cmdfile ]"
 *		
 *	NOTE: the cmdfile name in "-f cmdfile" should not have ".cmd"
 *	
 *	CHECK comparisons have built-in utilities for ignoring things
 *		that are likely to be different:
 *			o dates
 *			o hostnames
 *			o usernames
 *		plus, it knows about certain file formats ascii and report
 *		log formats
 *
 * 	Author: D. Birns
 *	3/21/95
 *
 *	Last changed:  12/2/98
 *
 */
#include <stdio.h>
#include <errno.h>
#ifdef _MSC_VER
#ifndef PC
#define PC
#endif
#include <pcsock.h>	// Gets "timeval"
#define FLEXLM
#endif /* PC */
#ifdef MIPS
extern int errno;
#endif /* MIPS */
#include <fcntl.h>
#if !defined( GLIBC) &&  !defined(ALPHA_LINUX) \
	&& !defined(VMLINUX64) && !defined(RHLINUX64) && !defined (MAC10)
extern char *sys_errlist[];
#endif
#if defined(SUNOS5) || defined(REDHAT)
#define SYS_ERRLIST(x) strerror(x)
#else
#define SYS_ERRLIST(x) sys_errlist[x]
#endif

#include <stdlib.h>
#include <string.h>
#define  lm_noargs  void
#define  lm_args(args) args
#include <time.h>
char *hostname = (char *)0;
char *username = (char *)0;
char *display = (char *)0;
int hostnamelen;
int usernamelen;
int displaylen;
int skip_until_endif;


#define EXPECT_KEYWORD "EXPECT_KEYWORD "
#define VARCHAR '$'

#ifdef VMS
#define unlink remove
#endif /* VMS */

void check 		lm_args((char *, char *, char *, char *));
int util_strcmp 	lm_args((char *, char *, char *));
void generate_exp 	lm_args((char *, char *, char *));
int is_errstr 		lm_args((char *));
int replog_strcmp 	lm_args((char *, char *));
int ishexnum		lm_args((char *));
char *replacevars	lm_args((char *));
void print_to_file	lm_args((char *));
void utsleep		lm_args((int));
int position		lm_args((FILE *, char *));
int cmdcnt;

#ifdef FLEXLM
extern char * ut_hostname lm_args((lm_noargs));
extern char * ut_display lm_args((lm_noargs));
extern char * ut_username lm_args((lm_noargs));
#else
char * ut_hostname (){return "";}
char * ut_display (){return "";}
char * ut_username (){return "";}
ut_checkout(str, i) {return 0;}
void ut_free_job(void) {}
ut_checkin(str, i) {return 0;}
#endif /* FLEXLM */
int generate = 0;
void utiltest lm_args((char *));

#define MAX_WHICH 30
#define UT_MAXWORD 250
int whichtest[MAX_WHICH];
int whichtest_cnt;
char *files[] = {
	"utflex",
	"utst",
	"utserv",
	"3serv",
	"utnoserv",
	"utcrypt",
	(char *)0};

char *delim =
     "---------------------------------------------------------------------\n";
#define UT_MAXLINE 2000 /* safe */
main(argc, argv)
char **argv;
{
  char *whichfile = (char *)0;
  int arg;
  char **file;

#ifdef sony
	setbuf(stdout);
	setbuf(stderr);
#else
        setvbuf(stdout, 0, _IONBF, 0);
        setvbuf(stderr, 0, _IONBF, 0);
#endif

	for (arg = 1; arg < argc; arg++)
	{
		if (!strcmp(argv[arg], "-f") && ++arg < argc)
			whichfile = argv[arg];
		else if (!strcmp(argv[arg], "-generate"))
			generate = 1; /* create the .exp files */
		else if (!strcmp(argv[arg], "-t") && ++arg < argc)
			sscanf(argv[arg], "%d", &whichtest[whichtest_cnt++]);
		else
		{
			printf("Unknown arg: %s\n", argv[arg]);
			usage();
		}
	}
	if (getenv("UTIL_GENERATE"))
		generate=1;

	if (whichfile)
		utiltest(whichfile);
	else
	{
		for (file = files; *file; file++)
			utiltest(*file);
	}
}
void
utiltest(filename)
char *filename;
{
   FILE *fp;
   char str[UT_MAXLINE + 1];
   char *cmdtype;
   char *cmd;
   char *cp1, *cp2;
   int cnt = 0;
   char cfile[UT_MAXLINE + 1];
   char cexpect[UT_MAXLINE + 1];
   char checkcmp[UT_MAXLINE + 1];
   int len;
   char cmdfile[512];
   int testcnt = 0;

	cmdcnt = 0;
	if (!hostname) 
	{
		hostname = ut_hostname();
		hostnamelen = strlen(hostname);
	}
	if (!username) 
	{
		username = ut_username();
		usernamelen = strlen(username);
	}
	if (!display) 
	{
		display = ut_display();
		displaylen = strlen(display);
	}

	if (generate)
	{
		printf(delim);
		printf("Generating (overwriting) %s.exp\n", cmdfile);
		printf(delim);
	}

	sprintf(cmdfile, "%s.cmd", filename);
	printf(delim);
	printf("\tT E S T I N G   F I L E :    %s\n", cmdfile);
	if (!(fp = fopen(cmdfile, "r")))
	{
		printf("Can't open command file %s, %s\n", cmdfile, 
							SYS_ERRLIST(errno));
	}
	if (generate)  /* truncate if already exists */
	{
	  char f[512];
	  int fd;
		sprintf(f, "%s.exp", filename);
		if ((fd = open(f, O_TRUNC, 777))>= 0)
			close(fd);
	}
	printf(delim);
	if (!fp) return;
	while (fgets(str, UT_MAXLINE, fp)){
		cp1 = str;
		str[strlen(str) - 1] = '\0';
		cmdtype = str;
		while (*cp1 && !isspace(*cp1))
			cp1++;
		*cp1 = '\0'; /* terminates cmdtype */
		cp1++;
		while (*cp1 && isspace(*cp1))
			cp1++;
		cmd = replacevars(cp1);
		len = strlen(cmdtype);
		if (!strcmp(cmdtype, "DELIM") )
		{
			cmdcnt++;
			++cnt;
			testcnt++;
		}
		if (skip_until_endif)
		{
			if( !strcmp(cmdtype, "ENDIF") ) skip_until_endif = 0;
			else printf("Skipping %d\n", testcnt);
			continue;
		}
		if (!*whichtest || in(whichtest, testcnt, MAX_WHICH))
		{
                  char *args[50];
                  int arg = 0;
                  char *cmdsav = cmd;
                  char argstr[100];
#ifdef PC
                  BOOL opOK;
                  STARTUPINFO startupInfo;
                  PROCESS_INFORMATION processInfo;
#endif /* PC */
			if (!strcmp(cmdtype, "CMD") )
			{
				printf("\t\"%s\"\n", cmd);
				system(cmd);
			}
			else if (!strcmp(cmdtype, "IFUNIX") )
			{
#ifdef PC
				skip_until_endif = 1;
#endif
			}
			else if (!strcmp(cmdtype, "IFWIN") )
			{
#ifndef PC
				skip_until_endif = 1;
#endif
			}
			else if (!strcmp(cmdtype, "BKG_CMD") )
			{
                                printf("\t\"%s\"\n", cmd);
#ifndef _MSC_VER
                                memset(args,0, sizeof(args));
                                *argstr = 0;
                                while (isspace(*cmd))
                                        cmd++;
                                args[arg] = cmd;
                                for (; *cmd; cmd++)
                                {
                                        if (isspace(*cmd))
                                        {
                                                *cmd++ = 0;
                                                arg++;
                                                args[arg] = cmd;
                                                if (!*argstr)
                                                        strcpy(argstr, cmd);
                                                continue;
                                        }
                                }
				if (fork() == 0)
				{
					execvp(args[0], args);
					fprintf(stderr, "Can't execute \"%s\"",
						args[0]);
					perror("");
				}
#else
                                startupInfo.cb = sizeof(startupInfo);
                                opOK = CreateProcess(
                                        0,	    // lpApplicationName
                                        cmd,	    // lpCommandLine
					NULL,		// lpProcessAttributes
                                        NULL,		// lpThreadAttributes
                                        FALSE,		// bInheritHandles,
                                        DETACHED_PROCESS |
                                        CREATE_NEW_PROCESS_GROUP,	// dwCreationFlags,
                                        NULL,		// lpEnvironment,
                                        NULL,		// lpCurrentDirectory,
                                        &startupInfo,	// lplStartupInfo,
                                        &processInfo );	// lpProcessInformation
                        
                                if( !opOK )
                                {
                                        fprintf(stderr, 
                                "CreateProcess of \"%s\" failed, error: %d\n",
                                                cmd,
                                                GetLastError());
                                }
                                else
                                {
                                        CloseHandle(processInfo.hThread);
                                        CloseHandle(processInfo.hProcess);
                                }


#endif
			}
			else if (!strcmp(cmdtype, "PRINT") )
			{
				printf("\t  => %s\n", cmd);
			}
			else if (!strcmp(cmdtype, "FPRINT") )
			{
				print_to_file(cmd);
			}
			else if (!strcmp(cmdtype, "RM") )
			{
				if (unlink(cmd))
					printf("Can't RM %s, %s\n", cmd, 
						SYS_ERRLIST(errno));
			}
			else if (!strcmp(cmdtype, "RM-F") )
			{
				unlink(cmd);
			}
			else if (!strcmp(cmdtype, "SETENV") )
			{
			  static char buf[10000]; /* up to 10k of setenv space*/
			  static char *bufp = buf;;
			  	printf("\tsetenv %s\n", cmd);
				strcpy(bufp, cmd);
#ifdef _MSC_VER /* PC */
				_putenv(bufp);
#else
				putenv(bufp);
				bufp += strlen(bufp) + 1;
#endif
			}
			else if (!strcmp(cmdtype, "DELIM") )
			{
				printf(delim);
				printf("Test %d\n", cnt);
			}
			else if (!strcmp(cmdtype, "CHECK") )
			{
				if (sscanf(cmd, "%s %s %s", cfile, cexpect,
						checkcmp) == 2)
					*checkcmp = '\0';
				printf("\tcheck %s\n", cmd);
				check(cfile, filename, cexpect, checkcmp);
			}
			else if (!strcmp(cmdtype, "SLEEP") )
			{
			  int i = atoi(cmd);
				
				printf("\tsleep %d...\n", i);
				utsleep(i);
			}
			else if (!strcmp(cmdtype, "LMREMOVE") )
			{
			  char lmremove[200];
				sprintf(lmremove, 
				"lmremove %s %s %s %s -o lmremove.out", 
					cmd, username, hostname, 
					display);
				printf("\t%s\n", lmremove);
				system(lmremove);
			}
			else if (!strcmp(cmdtype, "LMREMOVE_H") )
			{
			  char lmremove[200];
				sprintf(lmremove, 
				"lmremove -h %s %s 2837 101 -o lmremove.out", 
					cmd, hostname);
				printf("\t%s\n", lmremove);
				system(lmremove);
			}
			else if (!strcmp(cmdtype, "FREEJOB") )
				ut_free_job();
			else if (!strcmp(cmdtype, "CLEARBORROW") )
				ut_clear_borrow();
			else if (!strcmp(cmdtype, "CHECKOUT") )
			{
			  char feature[UT_MAXWORD];
			  char samejob[UT_MAXWORD];


				printf("\tCheckout %s\n", cmd);
				getword(&cmd, feature);
				if (getword(&cmd, samejob))
					ut_checkout(feature, 1);
				else
					ut_checkout(feature, 0);
			}
			else if (!strcmp(cmdtype, "CHECKIN") )
			{
				printf("\tCheckin %s\n", cmd);
				ut_checkin(cmd);
			}
			else
				/* comment */;
		}
	}
	fclose(fp);
		
}
void 
check(filename, testname, expect, cmp)
char *filename;
char *testname;
char *expect;
char *cmp;
{
  FILE *resultfp;
  FILE *efp;
   int linenum;
   char result_str[UT_MAXLINE + 1];
   char estr[UT_MAXLINE + 1];
   char last_result_str[UT_MAXLINE + 1];
   char last_estr[UT_MAXLINE + 1];
   char expectfile[512];
#ifdef PC
#define MAXBADLINES 15
#else
#define MAXBADLINES 4
#endif /* PC */
   int badcnt = 0;
   int skip_expected = 0;

	
	if (generate)
	{
		generate_exp(filename, testname, expect);
		return;
	}
	if (!(resultfp = fopen(filename, "r")))
	{
		printf("Can't open results file %s, %s\n", filename, 
						SYS_ERRLIST(errno));
		return;
	}
	sprintf(expectfile, "%s.exp", testname);
	if (!(efp = fopen(expectfile, "r")))
	{
		printf("Can't open .exp file %s, %s\n", 
				expectfile, SYS_ERRLIST(errno));
		goto err;
	}
	if (!position(efp, expect))
		goto err;
	linenum = 0;
	*last_estr = *last_result_str = 0;
	while (fgets(result_str, UT_MAXLINE, resultfp) && badcnt < MAXBADLINES)
	{
	        if (*result_str == '\n') 
	        {
	                linenum++;
                        continue;
                }
skip_result:
	        if (skip_expected)
		{
	                skip_expected = 0;
		}
		else 
		{
			*estr = 0;
                        while (fgets(estr, UT_MAXLINE, efp) && (*estr == '\n'))
				;
                }
		if (!*estr)
		{
			printf("more text than expected, EOF on %s\n", expectfile);
			printf("Got:     \t%s", result_str);
			goto err;
		}
		if (!strncmp(estr, EXPECT_KEYWORD, strlen(EXPECT_KEYWORD)))
		{
			printf("more text than expected in %s, line %d\n", filename, linenum+1);
			printf("%s Got:     \t%s", filename, result_str);
			goto err;
		}
		if (util_strcmp(result_str, &estr[0], cmp))
		{
#if 0
		        if (*last_estr && !util_strcmp(result_str, &last_estr[0], cmp))
		        {
		                skip_expected = 1;
		        }
		        else if (*last_result_str && !util_strcmp(last_result_str, estr, cmp))
		        {
		                goto skip_result;
		        }
		        else
#endif
		        {
                                printf("ERROR: File %s, line %d\n", filename, linenum+1);
                                printf("Expected:\t%s", estr);
                                printf("Got:     \t%s", result_str);
                                badcnt++;
                        }
		}
		strcpy(last_estr, estr);
		strcpy(last_result_str, result_str);
		linenum++;
	}
	if (badcnt >= MAXBADLINES)
		printf("\tmax bad lines exceeded, skipping other errors...\n");
	else 
	{
	  char * rc;

		while ((rc = fgets(estr, UT_MAXLINE, efp)) && (*estr != 'E') && (*estr == '\n')) ;
		if (rc && (*estr != 'E'))
		{
			printf("less text than expected in %s, line %d\n", filename, linenum);
			printf("Expected:\t%s", estr);
		}
	}
err:
	if (efp) fclose(efp);
	if (resultfp) fclose(resultfp);
}
/*
 *	generate the .exp files automatically 
 */
void
generate_exp(filename, testname, expect)
char *filename;
char *testname;
char *expect;
{
  FILE *fp;
  FILE *efp;
  char expectfile[512];
   char str[UT_MAXLINE + 1];
	if (!(fp = fopen(filename, "r")))
	{
		printf("Can't open results file %s, %s\n", filename, 
						SYS_ERRLIST(errno));
		exit(1);
	}
	sprintf(expectfile, "%s.exp", testname);
	if (!(efp = fopen(expectfile, "a")))
	{
		printf("Can't open .exp file %s, %s\n", 
				expectfile, SYS_ERRLIST(errno));
		exit(1);
	}
	fprintf(efp, "%s%s _______________________________________\n", 
		EXPECT_KEYWORD, expect);
	while (fgets(str, UT_MAXLINE, fp))
		fputs(str, efp);
	fclose(efp);
	fclose(fp);
}
int
position(fp, expect)
FILE *fp;
char *expect;
{
   char str[UT_MAXLINE + 1];
   char estr[80];
	sprintf(estr, "%s%s ", EXPECT_KEYWORD, expect);
	while (fgets(str, UT_MAXLINE, fp))
	{
		if (!strncmp(estr, str, strlen(estr)))
			return 1;
	}
	printf("Can't find position %s in .exp file, exiting\n", estr);
	return 0; /* failure */
}

/* 
 *	util_strcmp -- like strcmp, but tries to avoid obvious problems
 */
util_strcmp(str1, str2, cmp)
char *str1, *str2, *cmp;
{
  int cnt;
  int ret;
	if (!strcmp(cmp, "EXACT"))
		ret = strcmp(str1, str2);
	else if (!strcmp(cmp, "REPLOG"))
		ret = replog_strcmp(str1, str2);
	else
	{
		if (cnt = is_errstr(str1))
			ret = u_strncmp(str1, str2, cnt);
		else
			ret =  u_strncmp(str1, str2, 0);
	}
	return ret;
}
/* 
 *	is_errstr :  look for ".*:.*-#,.*"
 */
int
is_errstr(str)
char *str;
{
  char *cp = str;
  int state = 0;
	for(;*cp;cp++)
	{
                if ((state == 0) && (*cp == ':'))
                        state = 1;
                else if ((state == 1) && (*cp == '-'))
                        state = 2;
                else if ((state == 2) && (*cp == ','))
                        return (cp - str);
        }

	return 0;
}
is_time(str)
char *str;
{
        if (isdigit(str[0])
                && isdigit(str[1])
                && (str[2] == ':' )
                && isdigit(str[3])
                && isdigit(str[4])
                && (str[5] == ':' )
                && isdigit(str[6])
                && isdigit(str[7]))
                return 1;
        return 0;               
}
/*
 *	u_strncmp -- like strncmp -- but tries to ignore hostnames
 *			in both strings
 *	Also, ignore leading timestamps in log files
 */
u_strncmp(str, expected, len)
char *str;
char *expected;
int len; /* of str */
{
  char *cp = str, *ecp = expected;
  int cnt = 0;

/*
 *	If it's a log file, skip time
 */
	if (str[2] == ':' && ecp[2] == ':')
	{
		cp+=8;
		ecp+=8;
		if (len)
			len -= 8;
	}

	if (len)
	{
		if (!strncmp(cp, ecp, len)) /* easy */
			return 0;
	}
	else if (!strcmp(cp, ecp)) /* easy */
		return 0;

	if (!len) len = 100000; /* unlimited*/
	for (cnt = 0 ; *ecp && *cp && (cnt < len); cnt ++, cp++, ecp++)
	{
		while(*cp && (isspace(*cp) || *cp == '-'))
		{
			cp++;
			cnt++;
		}
		while(*ecp && (isspace(*ecp) || *ecp == '-'))
		{
			ecp++;
		}
/*
 *		if certain strings match, ignore rest of line
 */
		if (!strncmp("2700", cp, 4) && cp[4] >= '0' && cp[4] <= '9'
			&& (cp[5] < '0' || cp[5] > '9') ) /* default port */
		{
			return 0;
		}
			
		if (!strncmp(hostname, cp, strlen(hostname))) 
		{
			return 0;
		}
		if (!strncmp(username, cp, strlen(username) ))
			return 0;
		if (is_time(cp))
		{
		        cp += 8; ecp +=8;
		}
		if (!strncmp("RESERVATION for INTERNET", cp, 
					strlen("RESERVATION for INTERNET") ))
			return 0;
		if (isweekday(cp))  /* it's a date */
			return 0;
		if (strstr(cp, "200") ||
			strstr(cp, "199") ||
			strstr(cp, "198")) return 0;
/* 
 *		end of special string tests
 */
                if ((((*cp == '\\') || (*cp == '/')) &&
                        ((*ecp == '\\') || (*ecp == '/'))) ||
			(((*cp == ';') || (*cp == ':')) &&
				((*ecp == ';') || (*ecp == ':'))) )
				continue;
		if (*cp != *ecp)
			return (*cp - *ecp);
	}
	return 0;
}
in(arr, val, max)
int *arr;
{
  int i;
	for (i=0; i<max;i++)
		if (arr[i] == val)
			return 1;
	return 0;
}
char *weekdays[] = {
	"Sun ",
	"Mon ",
	"Tue ",
	"Wed ",
	"Thu ",
	"Fri ",
	"Sat ", (char *)0};
isweekday(str)
char *str;
{
  char **p;
	for (p = weekdays; *p; p++)
		if (!strncmp( *p, str, 4))
			return 1;
	return 0;
}
	
usage()
{
	printf("utiltest [ -f <file (without .cmd suffix)> ] [ -t testnum [ -t testnum ] ... ] \n");
	exit(1);
}
/*
 *	1) skip 2nd column
 *	2) ignore hostnames and usernames
 *	3) ignore strings starting with "0X"
 *	return 0 means equal, else not equal
 */
int
replog_strcmp(str1, str2)
char *str1;
char *str2;
{
#define LM_USAGE4 1
  char word1[UT_MAXWORD];
  char word2[UT_MAXWORD];
  int func = 0;
  int gotword1, gotword2;
  int ret = 0; /* assume equal */
  int column = 0;

	gotword1 = 1;
	while (gotword1)
	{
		column++;
		gotword1 = getword(&str1, word1); 
		gotword2 = getword(&str2, word2);
		if (gotword1 != gotword2)
			return 1;
		if (!gotword1)
			break;
		if (column == 2)
			continue;
		if ((column == 17 || column == 16 || column == 21) 
					&& func == LM_USAGE4)
			continue; 
			
		if (isuserhostdisplay(word1) || isuserhostdisplay(word2))
			continue;
		if (ishexnum(word1) && ishexnum(word2))
			continue;
		if ((column == 19 && func == LM_USAGE4))
		{
			if (isipaddr(word1) && isipaddr(word2))
				continue;
		}
		if (strcmp(word1, word2))
		{
			ret = 1;
			break;
		}
		if (column == 1)
		{
			if (!strcmp(word1, "lm_usage4"))
				func = LM_USAGE4;
		}
	}
	return ret;
	
}
int 
isuserhostdisplay(wrd)
char *wrd;
{
	if (*wrd == '"') wrd++;
	if (!strncmp(wrd, display, displaylen) 
		|| !strncmp(wrd, hostname, hostnamelen) 
		|| !strncmp(wrd, username, usernamelen) )
		return 1;
	return 0;
}
int 
getword(stream, wrd)
char **stream;
char *wrd;
{
  char *cp = *stream;
  char *cpw = wrd;

	while(*cp && isspace(*cp)) 	/* skip leading spaces */
		cp++;
	while(*cp && !isspace(*cp)) 	/* copy wrd */
		*cpw++ = *cp++;
	*stream = cp;
	*cpw = '\0';
	return *cp;
}
int 
ishexnum(str)
char *str;
{
	if (*str && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		return 1;
	else
		return 0;
}
int
isipaddr(str)
char *str;
{
  char *cp;
  int dotcnt = 0;

	for (cp = str; *cp; cp++)
	{
		if (isdigit(*cp))
			continue;
		else if (*cp == '.')
			dotcnt++;
		else if (*cp == '"')
			;
		else
			return 0;
	}
	if (dotcnt == 3)
		return 1;
	return 0;
}
		
void 
print_to_file(cmd)
char *cmd;
{
  char filename[UT_MAXWORD];
  FILE *fp;

	getword(&cmd, filename);

	if (!(fp = fopen(filename, "a")))
	{
		printf("Can't open %s, %s\n", filename, SYS_ERRLIST(errno));
		return;
	}
	while(*cmd && isspace(*cmd)) cmd++;
	fprintf(fp, "%s\n", cmd);
	fclose(fp);
}
char *
replacevars(str)
char *str;
{
  static char retstr[UT_MAXLINE + 1];
  char *savecp = str;
  char *cp;
  char *retcp = retstr;

	while (cp = strchr(savecp, VARCHAR))
	{
		if (!strncmp(cp + 1, "HOSTNAME", strlen("HOSTNAME")))
		{
			while (str < cp) *retcp++ = *str++;
			strcpy(retcp, hostname);
			retcp += hostnamelen;
			cp += (strlen("HOSTNAME") + 1);
			str += (strlen("HOSTNAME") + 1);
			savecp = cp;
		}
		else if (!strncmp(cp + 1, "USERNAME", strlen("USERNAME")))
		{
			while (str < cp) *retcp++ = *str++;
			strcpy(retcp, username);
			retcp += usernamelen;
			cp += (strlen("USERNAME") + 1);
			str += (strlen("USERNAME") + 1);
			savecp = cp;
		}
		else if (!strncmp(cp + 1, "DISPLAY", strlen("DISPLAY")))
		{
			while (str < cp) *retcp++ = *str++;
			strcpy(retcp, display);
			retcp += displaylen;
			cp += (strlen("DISPLAY") + 1);
			str += (strlen("DISPLAY") + 1);
			savecp = cp;
		}
		else if (!strncmp(cp + 1, "TODAY", strlen("TODAY")))
		{
                  char today[100];
		  time_t t = time(0);
		  struct tm *tm;
#ifdef THREAD_SAFE_TIME
		  struct tm tst;
#endif
		  int m;

#ifdef THREAD_SAFE_TIME
			localtime_r(&t, &tst);
			tm = &tst;
#else /* !THREAD_SAFE_TIME */
			tm = localtime(&t);
#endif
			m = tm->tm_mon + 1;

			sprintf(today, "%d-%s-%d",
						tm->tm_mday, 
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
						"dec", tm->tm_year + 1900);
                        while (str < cp) *retcp++ = *str++;

                        strcpy(retcp, today);
                        retcp += strlen(today);
                        cp += (strlen("TODAY") + 1);
                        str += (strlen("TODAY") + 1);
                        savecp = cp;
		}
		else if (!strncmp(cp + 1, "CMDCNT", strlen("CMDCNT")))
                {
                  char cmdcntbuf[100];
                        while (str < cp) *retcp++ = *str++;
                        sprintf(cmdcntbuf, "%d", cmdcnt);
                        strcpy(retcp, cmdcntbuf);
                        retcp += strlen(cmdcntbuf);
                        cp += (strlen("CMDCNT") + 1);
                        str += (strlen("CMDCNT") + 1);
                        savecp = cp;
                }
		else
		{
			while (str < (cp + 1)) *retcp++ = *str++;
			savecp = cp + 1;
		}
	}
	strcpy(retcp, savecp);
	return retstr;
}
#if !defined( OS2) && !defined(PC)
#include <sys/time.h>
#endif /* OS2 */
#include <sys/types.h>
void
utsleep(seconds)
{
#ifdef VMS	/* no select - use setitimr/waitfr */
  int interval[2];

	interval[0] = -10000000 * seconds;
	interval[0] = -1;

	sys$setimr (44, interval, 0, 0, 0);
	sys$waitfr (44);
#else
  long t, remaining;
  struct timeval tv;
	
	remaining = seconds;
	t = (long)time(0);


	while (remaining > 0)
	{
		tv.tv_sec = remaining;
		tv.tv_usec = 0;
		select(0,0,0,0, &tv);
		remaining = seconds - ((time(0) - t));
	}
#endif /* VMS */
}

