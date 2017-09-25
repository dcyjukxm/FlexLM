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
 *	Module: $Id: ts_serv_supp.c,v 1.10 2003/01/13 22:55:19 kmaclean Exp $
 *
 *	Function:	run_server()
 *			daemon_sleep()
 *			run_server_c()
 *			kill_server()
 *			start_client()
 *			kill_clients()
 *			queue_client()
 *			signal_client()
 *			is_client()
 *			ts_sleep()
 *			svr4_system()
 *
 *	Description: 	Server/client spawning support routines
 *			modified from ts_serv_supp.c to support
 *			single process operations.
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	4/17/90
 *
 *	Last changed:  9/24/98
 *
 */
#include "lmachdep.h"

#if defined (MOTO_88K) || defined (sco)
#include <sys/unistd.h>				/* For X_OK */
#endif
#if defined( SVR4 ) && !defined(convex)
#include <unistd.h>
#endif
#ifndef PC
#include <sys/wait.h>
#endif /* PC */
#include "lmclient.h"
#include "lm_attr.h"
#include "lmhtype.h"
#include <sys/file.h>
#include "testsuite.h"
#include "lm_code.h"
#include "l_timers.h"
#include "l_prot.h"
#include <stdio.h>
#include <errno.h>
#ifndef PC
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>
#endif /* PC */
#include <sys/param.h>
#ifdef USE_WINSOCK	
#include <pcsock.h>	
#include <direct.h>	
#include <process.h>	
#include <stdlib.h>
#include <string.h> 
#else			
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* USE_WINSOCK */
LM_DATA_REF;

#ifdef VMS
int putenv(), setenv();
#endif /* VMS */
int do_dump = 0;
static char *delim =
     "---------------------------------------------------------------------\n";

extern int _lm_nerr;
extern char *_lm_errlist[];
LM_CODE(_encrypt, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, 
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

static int serverlist[20], serverpos = -1;
int ts_use_sleep = 0;

#ifndef VMS
run_new_server(name, string, log, dowait)
char *name;
char *string;
char *log;
int dowait;
{
    int pid;
    FILE *fp; 
    char a[20], host[200];
    char hostid[200];
    int port;

	setenv(LM_DEFAULT_ENV_SPEC, name);
	l_flush_config(lm_job);
	pid = start_daemon("", string, log);
	if (pid > 0)
	{
		serverlist[++serverpos] = pid;
	        if ((fp =  fopen(name, "r")) == NULL) {
		    perror("START_DAEMON fopen failed");
		    daemon_sleep();
		    return 0;
		}
		fscanf(fp, "%s%s%s%d", a, host, hostid, &port);
		fclose(fp);
		if (dowait) wait_until_listening(port);
	}
	else
	{
		fprintf(stdout, "DAEMON did not start - probably out of processes!\n");
	}
	return pid;
}
#endif /* VMS */

/*
 *	wait_until_listening -- returns 1 when it can connect to port, or 0
 *				if it times out
 */
void
f(i)
{
}
wait_until_listening(port)
int port;
{
  struct sockaddr_in sin;
  int s;
  long i;
  int err;
  int nl = 0;

	l_timer_add(lm_job, LM_REAL_TIMER, 0,(FP_PTRARG) f, LM_TIMER_SLEEP, 5000); 
	pause();

		/* let things settle first */
	(void) memset(&sin, 0, sizeof(sin));
	sin.sin_port = htons((short)port);
	sin.sin_family = AF_INET;
	i = htonl(0x7f000001); /* localhost */
	(void) memcpy(&sin.sin_addr, &i, 4);
	s = socket(AF_INET, SOCK_STREAM, 0);
	for (i=0;i<20;i++) /* try it 'n' times */
	{
		if (!connect(s, (struct sockaddr *)&sin,
					     sizeof(sin))) /* success */
		{
			if (nl) fputs("", stdout);
			shutdown(s,2);
			network_close(s);
			l_timer_add(lm_job, LM_REAL_TIMER, 0, (FP_PTRARG)f, 
				LM_TIMER_SLEEP, 10000); 
			pause();

			return 1; 
		}
		err = errno;
		network_close(s);
		s = socket(AF_INET, SOCK_STREAM, 0);
		fprintf(stdout, "*"); nl = 1;
		fflush(stdout);
		l_timer_add(lm_job, LM_REAL_TIMER, 0, (FP_PTRARG)f, 
				LM_TIMER_SLEEP, 5000); 
		pause();
	}
	errno = err;
	perror("delay in freeing lmgrd port");
	return 0;
}

#ifndef VMS
run_server_nowait(name, string, log)
char *name;
char *string;
char *log;
{
	return (run_new_server(name, string, log,0));
}
#endif /* VMS */

#ifndef VMS
/*returns pid*/
run_server(name, string, log)
char *name;
char *string;
char *log;
{
	return (run_new_server(name, string, log,1));
}
#endif /* VMS */

#if !defined (PC) && !defined(VMS)
static dowaitpid(pid) 
{
	int ret;
	extern int errno;
#ifdef TRUE_BSD
	
	while (1)
	{
		ret = wait(0);
		if ((ret == pid) || (ret == -1 && errno != EINTR))
			return;
	}
#else
	waitpid(pid, 0, 0);
#endif /* TRUE_BSD */
}
#endif /* !PC && !VMS */

void
daemon_sleep()
{
  int sleeptime = 20;
  HOSTTYPE *hosttype = lc_hosttype(lm_job, 0);

	if (hosttype->code == LM_SUN3_50) sleeptime += 5;
	/* Time for daemons to come up */
	if (ts_use_sleep) sleep(sleeptime); else lm_sleep(sleeptime);
}

#ifndef VMS
run_new_server_c(path, string, log)
char *path;
char *string;
char *log;
{
  int pid;

	pid = start_daemon(path, string, log);
	if (pid > 0)
	{
		serverlist[++serverpos] = pid;
		daemon_sleep();
	}
	else
	{
		fprintf(stdout, "DAEMON did not start - probably out of processes!\n");
	}
	return pid;
}
#endif /* !VMS */

#ifndef VMS
void
run_server_c(path, string, log)
char *path;
char *string;
char *log;
{
	run_new_server_c(path, string, log);
}
#endif /* !VMS */

#if !defined(PC) && !defined(VMS)
kill_server_pid(pid)
{
	int i;
	if (kill(pid, SIGTERM))
	{
		(void) fprintf(stdout, "kill of server (pid: %d) failed\n",
				pid);
		(void) perror("kill");
	}
	dowaitpid(pid);
	/*remove from serverlist*/
	for (i = 0; i <= serverpos; i++) 
	{
		if (serverlist[i] == pid)
		{
			serverlist[i] = 0;
		}
	}
}
#endif /* !PC && !VMS */

#ifndef VMS
void
kill_server()
{
  int i;

	for (i = 0; i <= serverpos; i++)
		if (serverlist[i] > 0)
		{
#ifdef PC   
#ifdef WINNT 
			TerminateProcess( (HANDLE)serverlist[i], (UINT)-1 );
#else /* WINNT */
			fprintf(stdout,  "FLEXlm does not support server capability on Windows yet!\n" );
#endif /* WINNT */
#else /* PC */ 
			if (kill(serverlist[i], SIGTERM))
			{
				(void) fprintf(stdout, "kill of server %d (pid: %d) failed\n",
						i, serverlist[i]);
				(void) perror("kill");
			}
			dowaitpid(serverlist[i]);
#endif /* PC */
		}
	serverpos = -1;
	if (ts_use_sleep) sleep(5); else lm_sleep(5);
}
#endif /* !VMS */

static char our_daemon[LM_MAXPATHLEN];
static int first = 1;

#ifndef VMS
start_daemon(path, string, log)
char *path;
char *string;
char *log;
{
  int pid;
  FILE *f, *fopen();
  int t;
  int retrycount = 10;	/* Retry the fork 10 times at 5 second intervals */
  char *lmgrd;
	if (!(lmgrd = getenv("LMGRD")))
#ifdef PC
		lmgrd = "lmgrd.exe";
#else 
		lmgrd = "lmgrd";
#endif		
	if (first)
	{
		(void) getwd(our_daemon, LM_MAXPATHLEN-2);
#ifdef VMS
		demopath[strlen(our_daemon)-1] = '\0';
#endif
		sprintf(our_daemon, "%s/%s", our_daemon, lmgrd);
	}

	t = access(our_daemon, X_OK);
	if (t) 
	{
		fprintf(stderr,"Daemon %s is not executable: ", our_daemon);
		perror("");
		exit(1);
	}
	f = fopen(log, "a");
	if (f != (FILE *) NULL) 
	{
		(void) fprintf(f, delim);
		(void) fprintf(f, delim);
		(void) fprintf(f, "Server for test %s\n", string);
		(void) fprintf(f, delim);
		(void) fprintf(f, delim);
		(void) fclose(f);
	}
	(void) fflush(stdout);
	pid = -1;
#ifdef PC
#ifdef WINNT
	{
		char cmdline[256];
		STARTUPINFO         startup_info;
		PROCESS_INFORMATION process_info;

		sprintf( cmdline, "%s -z -app -c %s license.dat", our_daemon, path);
		GetStartupInfo( &startup_info );
		if ( !CreateProcess( NULL,	/* module image	    */
				    cmdline,	/* module and args  */
				    NULL,	/* process security */
				    NULL,	/* thread security  */
				    TRUE,	/* inherit handles  */
				    DETACHED_PROCESS,
				    NULL,	/* environment ptr  */
				    NULL,	/* working dir	    */
				    &startup_info,
				    &process_info ) )
		{
			fprintf(stdout,  "License our_daemon startup failed: \n" );
			fprintf(stdout,  "\t%s\n", cmdline );
			fprintf(stdout,  "CreateProcess error code: 0x%x\n",
				GetLastError() );
			exit(-1);
		}
		/*
		 *	On NT, we use process handle as pid for the
		 *	vendor our_daemon process.
		 */
		pid = (int)process_info.hProcess;		
	
	}
#else /* WINNT */
	fprintf(stdout,  "FLEXlm does not support server capability on Windows yet!\n" );
#endif /* WINNT */

#else /* PC */
	while (pid < 0 && retrycount > 0)
	{
		pid = fork();
		retrycount--;
	}
	if (pid == -1) perror("EEKS: fork failed!");
	else if (pid == 0)
	{
		(void) freopen(log, "a", stdout);
		(void) freopen(log, "a", stderr);
		if (*path)
			execl(our_daemon, lmgrd, "-z", "-c", path, 0);
		else
			execl(our_daemon, lmgrd, "-z", 0);
		fprintf(stderr,"Exec of %s failed: ", our_daemon);
		perror("");
		exit(1);
	}
#endif /* PC */	
	return(pid);
}
#endif /* !VMS */

#if !defined(PC) && !defined(VMS)
good_client_udp(feature, ver, nlic, dup_mask, argv0, msg, linenum)
char *feature;
char *ver;
int nlic;
int dup_mask;
char *argv0;
char *msg;
int linenum;
{
  int rc;
        rc = start_client(feature, ver, nlic, 0, &_encrypt, dup_mask, argv0, LM_UDP);
        if (rc)
        {
          char string[100];

                if (msg == (char *) NULL)
                {
                        sprintf(string, "rc = %d checkout of %d %s license%s failed, linenum:%d",
                                    rc, nlic, feature, (nlic==1?"":"s"), linenum
);
                }
                else
                        sprintf(string, "%s, line %d, %d\n", msg,
                                                        linenum, _lm_errno);
                ts_error(lm_job, string);
                if (do_dump) ts_dump(feature);
        }
        return(rc);
}
#endif /* !PC && !VMS */

/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of start_client
 */

#ifndef VMS
good_client_udp_nc(feature, ver, nlic, vcode, dup_mask, argv0, msg, linenum)
char *feature;
char *ver;
int nlic;
VENDORCODE *vcode;
int dup_mask;
char *argv0;
char *msg;
int linenum;
{
  int rc;
	rc = start_client_nc(feature, ver, nlic, 0, vcode,dup_mask, 
				argv0, LM_UDP);
	if (rc)
	{
	  char string[100];

		if (msg == (char *) NULL)
		{
			sprintf(string, "rc = %d checkout of %d %s license%s failed, linenum:%d",
				    rc, nlic, feature, (nlic==1?"":"s"), linenum);
		}
		else 
			sprintf(string, "%s, line %d, %d\n", msg, 
							linenum, _lm_errno);
		ts_error(lm_job, string);
		if (do_dump) ts_dump(feature);
	}
	return(rc);
}
#endif /* !VMS */

#if !defined(PC) && !defined(VMS)
good_client(feature, ver, nlic, dup_mask, argv0, msg)
char *feature;
char *ver;
int nlic;
int dup_mask;
char *argv0;
char *msg;
{
        return good_client_line(feature, ver, nlic, dup_mask, argv0, msg, 0);

}
good_client_line(feature, ver, nlic, dup_mask, argv0, msg, line)
char *feature;
char * ver;
int nlic;
int dup_mask;
char *argv0;
char *msg;
int line;
{
  int rc;
        rc = start_client(feature, ver, nlic, 0, &_encrypt, dup_mask, argv0, LM_TCP);
        if (rc)
        {
          char string[100];

                if (msg == (char *) NULL)
                {
                        sprintf(string, "checkout of %d %s license%s failed",
                                        nlic, feature, (nlic==1?"":"s"));
                        if (line) sprintf(string, "%s line %d", string, line);
                        msg = string;
                }
                if (_lm_errno < 0) {
                        sprintf(string, "%s, line %d, %d", msg, line,_lm_errno);
                        ts_error(lm_job, string);
                }
                if (do_dump) ts_dump(feature);
        }
        return(rc);
}
#endif /* !PC && !VMS */


/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of good_client_line
 */
good_client_nc (feature, ver, nlic, vcode, dup_mask, argv0, msg)
char *feature;
char *ver;
int nlic;
int dup_mask;
VENDORCODE *vcode;
char *argv0;
char *msg;
{
	return good_client_line_nc (feature, ver, nlic, vcode, dup_mask, 
					argv0, msg, 0);

}
/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of start_client
 */
good_client_line_nc(feature, ver, nlic, vcode, dup_mask, argv0, msg, line)
char *feature;
char * ver;
int nlic;
int dup_mask;
VENDORCODE *vcode;
char *argv0;
char *msg;
int line;
{
  int rc;
	rc = start_client_nc (feature, ver, nlic, 0, vcode, dup_mask, 
				argv0, LM_TCP);
	if (rc)
	{
	  char string[100];

		if (msg == (char *) NULL)
		{
			sprintf(string, "checkout of %d %s license%s failed",
					nlic, feature, (nlic==1?"":"s"));
			if (line) sprintf(string, "%s line %d", string, line);
			msg = string;
		}
		if (_lm_errno < 0) {
			sprintf(string, "%s, line %d, %d", msg, line,_lm_errno);
			ts_error(lm_job, string);
		}
		if (do_dump) ts_dump(feature);
	}
	return(rc);
}

#if !defined(PC) && !defined(VMS)
queue_client(feature, ver, nlic, dup_group, argv0)
char *feature;
char * ver;
int nlic;
int dup_group;
char *argv0;
{
  int rc;

        rc = start_client(feature, ver, nlic, LM_CO_QUEUE,
                                        &_encrypt, dup_group, argv0, LM_TCP);
        return(rc);
}
#endif /* !PC && !VMS */


/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of start_client
 */
queue_client_nc (feature, ver, nlic, vcode, dup_group, argv0)
char *feature;
char * ver;
int nlic;
int dup_group;
VENDORCODE *vcode;
char *argv0;
{
  int rc;

	rc = start_client_nc(feature, ver, nlic, LM_CO_QUEUE, vcode,
					dup_group, argv0, LM_TCP);
	return(rc);
}

#if !defined(PC) && !defined(VMS)
break_client_udp(feature, ver, cnt, dup_group, argv0, msg, linenum)
char *feature;
char * ver;
int cnt;
int dup_group;
char *argv0;
char *msg;
int linenum;
{
  int rc;
/*
 *      Make sure this client breaks it.
 */
        rc = start_client(feature, ver, cnt, 0, &_encrypt,
                                                dup_group, argv0, LM_TCP);
        if (rc == 0)
        {
                if (msg == (char *) NULL)
                  fprintf(stdout, "checkout of 10th license succeeded (s/b only 9), LINE %d\n", linenum);
                else
                  fprintf(stdout, "s, LINE %d\n", msg, linenum);
                if (do_dump) ts_dump(feature);
        }
        kill_last_n_clients(1);
        lm_sleep(1);    /* Make sure they die before we continue */
        return(rc);
}
break_client(feature, ver, print, dup_group, argv0, msg)
char *feature;
char * ver;
int print;
int dup_group;
char *argv0;
char *msg;
{
        return break_client_line(feature, ver, print, dup_group, argv0, msg, 0);
}
break_client_line(feature, ver, print, dup_group, argv0, msg, line)
char *feature;
char * ver;
int print;
int dup_group;
char *argv0;
char *msg;
int line;
{
  int rc;
/*
 *      Make sure this client breaks it.
 */
        rc = start_client(feature, ver, 1, 0, &_encrypt, dup_group, argv0, LM_TCP);
        if (print && (rc == 0))
        {
                if (msg == (char *) NULL)
                  fprintf(stdout, "checkout of 10th license succeeded (s/b only 9)");
                else
                  fprintf(stdout, "s", msg);
                if (line) fprintf(stdout,  "line %d\n", line);
                else fputs("",stdout);
                if (do_dump) ts_dump(feature);
        }
        kill_last_n_clients(1);
        lm_sleep(1);    /* Make sure they die before we continue */
        return(rc);
}
#endif /* !PC && !VMS */

/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of start_client
 */
break_client_udp_nc (feature, ver, cnt, vcode, dup_group, argv0, msg, linenum)
char *feature;
char * ver;
int cnt;
int dup_group;
VENDORCODE *vcode;
char *argv0;
char *msg;
int linenum;
{
  int rc;
/* 
 *	Make sure this client breaks it.
 */
	rc = start_client_nc (feature, ver, cnt, 0, vcode,
						dup_group, argv0, LM_TCP);
	if (rc == 0)
	{
		if (msg == (char *) NULL)
		  fprintf(stdout, "checkout of 10th license succeeded (s/b only 9), LINE %d\n", linenum);
		else
		  fprintf(stdout, "%s, LINE %d\n", msg, linenum);
		if (do_dump) ts_dump(feature);
	}
	return(rc);
}
/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of break_client_line
 */
break_client_nc (feature, ver, print, vcode, dup_group, argv0, msg)
char *feature;
char * ver;
int print;
int dup_group;
VENDORCODE *vcode;
char *argv0;
char *msg;
{
	return break_client_line_nc (feature, ver, print, vcode, dup_group,
				argv0, msg, 0);
}

/*
 * This version takes a vendor code argument, and invokes the no-child-
 * process version of start_client
 */
break_client_line_nc (feature, ver, print, vcode, dup_group, argv0, msg, line)
char *feature;
char * ver;
int print;
int dup_group;
VENDORCODE *vcode;
char *argv0;
char *msg;
int line;
{
  int rc;
/* 
 *	Make sure this client breaks it.
 */
	rc = start_client_nc (feature, ver, 1, 0, vcode,dup_group, argv0, 
		LM_TCP);
	if (print && (rc == 0))
	{
		if (msg == (char *) NULL)
		  fprintf(stdout, "checkout of 10th license succeeded (s/b only 9)");
		else
		  fprintf(stdout, "s", msg);
		if (line) fprintf(stdout,  "line %d\n", line);
		else fputs("",stdout);
		if (do_dump) ts_dump(feature);
	}
	return(rc);
}

static int clientlist[20], clientpos = -1;

#if !defined(PC) && !defined(VMS) 
int     /* returns same status code as lm_checkout */
start_client(feature, ver, nlic, wait, code, dupmask, argv0, transport)
char *feature;
char * ver;
int nlic;
int wait;
VENDORCODE *code;
int dupmask;
char *argv0;
int transport;
{
  int pid;
  int rc = 0;
  char buf[100], msg[100];
  char *s;
  int fildes[2];
  FILE *in, *out, *fdopen();
  int l;
  int t;

        t = pipe(fildes);
        if (t) {
                perror("Pipe call failed");
                close(fildes[0]);
                close(fildes[1]);
                return -1;
        }
        (void) fflush(stdout);
        pid = fork();
        if (pid == -1)
        {
                perror("EEKS: fork failed!");
                return -1;
        }
        else if (pid)
        {
                in = fdopen(fildes[0],"r");
                close(fildes[1]);
                clientlist[++clientpos] = pid;
                buf[0] = 0;
                fgets(buf,sizeof(buf),in);      /* read one line */
                fclose(in);
                l = strlen(buf);
                if (l>0 && buf[l-1]=='\n') buf[--l]=0;
                if (strcmp(buf,"OK")==0) {
                        return 0;
                }
                t = sscanf(buf, "BAD %d %s", &rc, msg);
/*
 *              Sometimes we get an empty string back
 *              I don't know why yet, but it seems that we
 *              should ignore these.  So I added the test
 *              for *buf below
 */
                if (*buf && t!=2)
                {
                        fprintf(stderr,
                                "Bad format for error message from client");
                        fprintf(stderr,"Client message: \"%s\"\n",buf);
                        lc_set_errno(lm_job,  -1);
                        return -1;
                }
		lc_set_errno(lm_job,  rc);
                return rc;
        }
        else /*child process*/
        {
           char name[20];
           int i, j, k;

                out = fdopen(fildes[1],"w");
                j = fileno(out);
                (void) close(fildes[0]);
#if defined( MOTO_88K) || defined(SCO)
#undef NOFILE
#define NOFILE k
#ifdef SCO
                k = getdtablesize();
#else
                k = lu_getdtablesize();
#endif
#endif

                for (i=3; i<NOFILE; i++)
                {
                        if (i != j && i!= fildes[1])
                                (void) close(i);
                }
                i = strlen(argv0);
                bzero(argv0, strlen(argv0));
                sprintf(name, "client_%d", nlic);
                strncpy(argv0, name, i-1);
                lm_set_attr(LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)transport);
                rc = lm_checkout(feature, ver, nlic, wait, code, dupmask);
                if (rc==0)
                {
                        fprintf(out, "OK\n");
                }
                else
                {
                        if (_lm_errno == 0)
                        {
                                fprintf(stdout, "WHAT: rc: %d, _lm_errno: %d\n");
                        }
                        s = lc_errstring(lm_job);
                        fprintf(out, "BAD %d %s\n", _lm_errno, s);
                }
                fclose(out);
                while (1) pause();
        }
}
#endif /* !PC && !VMS */

int	/* returns same status code as lm_checkout */
start_client_nc (feature, ver, nlic, wait, code, dupmask, argv0, transport)
char *feature;
char * ver;
int nlic;
int wait;
VENDORCODE *code;
int dupmask;
char *argv0;
int transport;
{
  int rc = 0;
  char buf[100];
  char *s;

	{
		lm_set_attr(LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)transport);
		rc = lm_checkout(feature, ver, nlic, wait, code, dupmask);
		if (rc!=0) 
		{
			if (_lm_errno == 0) 
			{ 
				fprintf(stdout, "rc: %d, _lm_errno: %d\n", rc,
						_lm_errno); 
			}
			s = lc_errstring(lm_job);
		}
		return rc;
	}
}

#ifndef VMS
void
kill_clients() 
{
#ifdef PC
	fprintf(stdout,  "Error: kill_clients() used\n" );
#else 
	int ret;
	signal_clients(SIGKILL, 0);
	clientpos = -1;
#endif
}
#endif /* !VMS */

#if !defined(PC) && !defined (VMS)
void
kill_last_n_clients(n)
int n;
{
	signal_clients(SIGKILL, n);
	clientpos -= n;
	if (clientpos < 0) clientpos = -1;
}

void
signal_clients(signal, n)
int signal;
int n;
{
  int i;

	if (n == 0)
	{
		for (i = 0; i <= clientpos; i++)
		{
			signal_client(i, signal);
		}
	}
	else
	{
		for (i = clientpos; (i >= 0) && (n > 0); i--)
		{
			signal_client(i, signal);
			n--;
		}
	}
}

void
signal_client(num, signal)
int num;
int signal;
{
	if (clientlist[num] > 0)
	{
		if (kill(clientlist[num], signal))
		{
			(void) fprintf(stdout,  "kill of client %d (pid: %d) failed\n",
							num, clientlist[num]);
			(void) perror("kill");
		}
		if (signal == SIGKILL) dowaitpid(clientlist[num]);
	}
}

is_client()
{
  int i, found = 0, stat;

	for (i = 0; i <= clientpos; i++)
	{
		/* See if its there */
		dowaitpid(clientlist[i]);
		stat = kill(clientlist[i], 0);
		if ((stat == -1 && errno == ESRCH))
			clientlist[i] = 0;
		else
			found++;	
	}
	return(found);
}
#endif /* !PC && !VMS */

void
tsputenv(arg)
char *arg;
{
	
#if 0
	char str[100];
	char *cp;
	sprintf(str, "%s=%s", arg1, arg2);
	if (cp = getenv(arg1))
		free(cp);
	cp = (char *)malloc(strlen(str) + 1);
	strcpy(cp, str);
#endif
#if defined(TRUE_BSD) || defined(VMS) || defined (convex)
  char *arg1, *arg2, *cp, tmp[1000];
   
	strcpy(tmp, arg);
	arg1 = tmp;
	for (cp = tmp; *cp && *cp != '='; cp++) ;
	*cp = '\0';
	arg2 = cp + 1;
	setenv(arg1, arg2);
#else
	putenv(arg);
#endif /* TRUE_BSD */
}



#ifdef SVR4
svr4_system(cmd, output)
char *cmd;
char *output;
{
  int pid;

	pid = fork();
	if (pid == -1) perror("system: EEKS: fork failed!");
	else if (pid == 0)
	{
		if (output)
		{
			freopen(output, "w", stdout);
			dup2(fileno(stdout), fileno(stderr));
		}
		execl(cmd, cmd, 0);
		fprintf(stderr,"Exec of %s failed: ", cmd);
		perror("");
		exit(1);
	}
	lm_sleep(1);
}
#endif
ts_error(job, string)
LM_HANDLE *job;
char *string;
{
  extern FILE* ofp;
	fprintf(ofp, "%s: %s\n", string, lc_errstring(job));
}
