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
 *	Module: $Id: ls_startup.c,v 1.26 2003/06/18 21:52:32 sluu Exp $
 *
 *      Function: ls_startup(daemon, hostname, master_name)
 *
 *      Description:    Start an application server, open its socket, pass fd.
 *
 *      Parameters:     (DAEMON *) daemon - pointer to DAEMON struct
 *                      (char *) hostname - Our hostname
 *                      (char *) master_name - Name of master node
 *
 *      Return:         (int) pid of child (-1 for error)
 *                      (port number updated)
 *
 *
 *      M. Christiano
 *      2/27/88
 *
 *	Last changed:  10/11/98
 *
 */
 
#include "lmachdep.h"
#include <errno.h>
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#include "flex_utils.h"
#include "flexevent.h"
#ifdef PC 
#include <stdlib.h>
extern char * ls_log_specified;
extern int	run_lmgrd_as_service;
extern int	run_lmgrd_as_applet;
extern char	ServiceName[128] ;
char QuotedLogPath[128];
#endif /* PC */

extern int use_v1_spawn;
extern time_t ls_test_hang;
extern time_t ls_test_exit;
extern int unpriv_utils;
extern int ls_allow_lmremove;
extern int ls_fflush_ok;
extern int ls_lmgrd_tcp_port;
extern char * ls_lmgrd_argv0;
extern int max_vd_connections;
extern int ls_foreground;
#ifdef WINNT
static int NT_Startup(char *, DAEMON *, char ** , char ** , int );
#endif /* WINNT */

#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif /* RELEASE_VERSION */

/*
 *      pre-v7 this returned the pid.  On PC there's 2 pids, so we
 *      return it anymore, but set it in daemon
 */
void
ls_startup(daemon, hostname, master_name)
DAEMON *daemon;
char *hostname;
char *master_name;
{
  int _child;
  int s;
  char *argv[20];
  int arg;
#ifndef PC
  char path[LM_MAXPATHLEN+1];
#endif /* PC */
  long t;
  int patharg = 0;
  char up_secs[9];
  extern time_t ls_start_time;
  char *	ppszInStr[20] = {NULL};

	DEBUG_INIT
	daemon->pid = daemon->print_pid -1; /* assume failure */
#ifdef PC /* P5155 */
	if (daemon->file_tcp_port)
	{
		LOG(("Starting vendor daemon at port %d\n", 
			daemon->file_tcp_port));
	}
#endif

	if (!master_name || *master_name == '\0')
	{
		LOG((lmtext("INTERNAL error: Please report the following to your vendor\n")));
		LOG((lmtext("ATTEMPT TO START VENDOR DAEMON %s with NO MASTER\n"),
			daemon->name));
		LOG_INFO((ERROR, "A vendor daemon was started with no master \
			selected.  This is an internal consistency error \
			in the daemons."));
		return;
	}
	if (!strcmp(daemon->name, "lmgrd"))
	{
		LOG((lmtext("INTERNAL error: Please report the following to your vendor\n")));
		LOG((lmtext("Attempt to start a vendor daemon named \"lmgrd\"\n")));
		LOG_INFO((ERROR, "A vendor attempted to name his daemon \
			\"lmgrd\".  This is not a valid vendor daemon name."));
		return;
	}
	if (daemon->file_tcp_port)
	{
		LOG(("Using vendor daemon port %d specified in license file\n",
			daemon->file_tcp_port));
	}
/*
 *	Start out by creating the socket for the application server
 */
#ifdef VD_MAKES_TCP_SOCKET
	s = -1;
/*	daemon->tcp_port = 0; */
#else
	{
		COMM_ENDPOINT endpoint;
		endpoint.transport = LM_TCP;
		endpoint.transport_addr.port = ntohs((unsigned short)daemon->tcp_port);
		s = ls_socket(hostname, &endpoint, 1);
		daemon->tcp_port = ntohs((unsigned int)endpoint.transport_addr.port);
	}
#endif

#ifndef PC	
	_child = fork();
	if (_child == 0)
#endif /* PC */
	{
	  char port_desc[LM_MAXPATHLEN];
	  char version[10];
	  char *cf;
/*
 *		In the child - First, set up SIGTRAP so that we will
 *		die if the master daemon starts us then wants to
 *		shut us down right away.
 */
#if !defined(SIGNAL_NOT_AVAILABLE) 
		(void) signal(SIGTRAP, SIG_IGN);
#endif /* SIGNAL_NOT_AVAILABLE */
/*
 *		Next, exec the server
 */

		(void) sprintf(version, "%d.%d", lm_job->code.flexlm_version, 
						lm_job->code.flexlm_revision);
		{
#if defined(ACCEPT_BUG) && defined(USE_TLI)
			(void) sprintf(port_desc, "%d", daemon->tcp_port);
#else
			(void) sprintf(port_desc, "%d", s);
#endif
		}
		argv[0] = daemon->name;
		argv[1] = "-T";
		argv[2] = master_name;
		if (!use_v1_spawn)
		{
			argv[3] = version;
			arg = 4;
		}
		else
			arg = 3;
		argv[arg++] = port_desc;
#ifdef WINNT
                if (run_lmgrd_as_service)
                {
                        char  tempstring[128];
                        tempstring[0]='0';
                        getenv_service(ServiceName,"License",tempstring,127 );
                        if (tempstring) lm_set_attr(LM_A_LICENSE_FILE_PTR,
                                                (LM_A_VAL_TYPE)tempstring);


                }


#endif /* WINNT */

		lm_get_attr(LM_A_LICENSE_FILE_PTR, (short *)&cf);
		if (cf)
		{
			argv[arg++] = "-c";
			argv[arg++] = cf;
		}
		else
			cf = "";	/* Dummy for debug output below */
		if (!use_v1_spawn)
		{
			if (unpriv_utils == 0) argv[arg++] = "-p";
			if (arg == 6) argv[arg++] = "-Z";
		}
		if ((ls_lmgrd_tcp_port >= LMGRD_PORT_START) &&
			(ls_lmgrd_tcp_port <= lm_job->port_end))
		{
		  static char ls_lmgrd_tcp_port_buf[MAX_LONG_LEN + 1];

			argv[arg++] = "-lmgrd_port";
			sprintf(ls_lmgrd_tcp_port_buf, "%x", ls_lmgrd_tcp_port);
			argv[arg++] = ls_lmgrd_tcp_port_buf;
			DEBUG(("-lmgrd_port %x\n", ls_lmgrd_tcp_port));
		}

		if (!ls_allow_lmremove)
		{
			argv[arg++] = "-x";
			argv[arg++] = "lmremove";
		}
		if (ls_test_hang) argv[arg++] = "-hang";
		if (!ls_fflush_ok)
		{
			argv[arg++] = "-nfs_log";
		}
		if (max_vd_connections)
		{
		  static char max_conn_str[MAX_LONG_LEN + 1];	
			argv[arg++] = "-max_connections";
			sprintf(max_conn_str, "%d", max_vd_connections);
			argv[arg++] = max_conn_str;

		}
#ifdef WINNT
		if (daemon->tcp_port)
		{
		  static char ls_daemon_port_buf[MAX_LONG_LEN + 1];	
			argv[arg++] = "-daemon_port";
			sprintf(ls_daemon_port_buf, "%x", daemon->tcp_port);
			argv[arg++] = ls_daemon_port_buf;
			DEBUG(("-daemon_port %x\n", daemon->tcp_port));

		}
#endif
		argv[arg++] = "--lmgrd_start";
		sprintf(up_secs, "%x", ls_start_time);
		argv[arg++] = up_secs;
		argv[arg] = 0;

		argv[arg] = (char *) NULL;
#if 0 
	{ int i; printf("L:%d ",__LINE__); for (i = 0; i < arg; i++) printf("%s ", argv[i]); puts(""); }
#endif
#ifdef WINNT 
                {
                        /* build command line from argv and arg */
                        /* build command line from argv and arg */
                  char pathinfo[256];
                  char * cmdline=0;
                  int error;
                  int started=FALSE;
                        daemon->pid = _child=-1;
                        if (*(daemon->path))
                        {
                            strcpy(pathinfo,daemon->path);
                            if (!NT_Startup(pathinfo, daemon, argv,&cmdline,arg))
                            { 
								LOG(( "License daemon startup failed: \n" ));
								if ( cmdline && *cmdline )
									LOG( ("\t%s\n", cmdline ));
								error=GetLastError();
								if (error==2)
								{
									LOG(( "File not found, %s\n",daemon->path ));
									if(l_flexEventLogIsEnabled())
									{
										ppszInStr[0] = daemon->path;

										l_flexEventLogWrite(lm_job,
															FLEXEVENT_ERROR,
															CAT_FLEXLM_LMGRD,
															MSG_FLEXLM_LMGRD_STARTUP_FAILED,
															1,
															ppszInStr,
															0,
															NULL);
									}
								}
								else
								{
									LOG(("CreateProcess error code: 0x%x File= %s\n",
											error, daemon->path ));

								}
    
                            }
                            else
							started=TRUE;
                        }
                        if (!started)
                        {
                            sprintf(pathinfo,"%s.exe",daemon->name);
                            if (!NT_Startup(pathinfo, daemon, argv, &cmdline, arg))
                            {
								LOG(( "License daemon startup failed: \n" ));
								if ( cmdline && *cmdline )
									LOG( ("\t%s\n", cmdline ));
								error=GetLastError();
								if (error==2)
								{
									LOG(( "File not found, %s\n",pathinfo ));
									if(l_flexEventLogIsEnabled())
									{
										ppszInStr[0] = pathinfo;

										l_flexEventLogWrite(lm_job,
											FLEXEVENT_ERROR,
											CAT_FLEXLM_LMGRD,
											MSG_FLEXLM_LMGRD_STARTUP_FAILED,
											1,
											ppszInStr,
											0,
											NULL);
									}
								}
								else
								{
									LOG(( "CreateProcess error code: 0x%x   File= %s\n",
											error, pathinfo ));
								}

                            }
                            else
								started=TRUE;
                        }
        /*
         *              Try lmgrd's argv[0], if applicable
         */
                        if (!started && run_lmgrd_as_service )
                        {
							char *cp;
							getenv_service(ServiceName,"Lmgrd",pathinfo,127 ); 
							cp = &pathinfo[strlen(pathinfo) - 1];
							while ((cp > pathinfo) && (*cp != PATHTERMINATOR) && (*cp != ':'))
							{
								cp--;
							}
							if (cp > pathinfo)
							{
								cp++;
								*cp = 0;
								strcpy(cp, daemon->name);
								if (!NT_Startup(pathinfo, daemon, argv,
												&cmdline, arg))
								{
									LOG(("License daemon startup failed: \n"));
									if ( cmdline && *cmdline )
									{
										LOG( ("\t%s\n", cmdline ));
									}
									error=GetLastError();
									if (error==2)
									{
										LOG(( "File not found, %s\n",pathinfo ));
										if(l_flexEventLogIsEnabled())
										{
											ppszInStr[0] = pathinfo;

											l_flexEventLogWrite(lm_job,
																FLEXEVENT_ERROR,
																CAT_FLEXLM_LMGRD,
																MSG_FLEXLM_LMGRD_STARTUP_FAILED,
																1,
																ppszInStr,
																0,
																NULL);
										}
									}
									else
									{
										LOG(( "CreateProcess error code: 0x%x   File= %s\n",
													error, pathinfo ));

									}

								}
								else
									started=TRUE;
							}
                        }
        
                        _child = daemon->pid;
                }
#endif /* WINNT */
#if defined(OS2)
		{
		  char cmdline[256];
		  char *pstart = (char *)cmdline;
		  RESULTCODES rcodes;
		  int i, rc;
		  char objname[64];
		  char daemonPath[200];

			if ( ls_log_specified )
			{
				argv[arg++] = "-l";
				argv[arg++] = ls_log_specified;
				argv[arg] = (char *) NULL;
			}
			
			strcpy(daemonPath, daemon->path);
			strupr(daemonPath);
			if ( !strstr(daemonPath, ".EXE") )
				strcat(daemonPath, ".EXE" );

			strupr(daemonPath);
			if ( !strstr(daemonPath, ".EXE") )
				strcat(daemonPath, ".EXE" );

			/*
			 * DosExecPgm requires NULL separated arguments and
			 * doulbe-null to terminate it.
			 */
			pstart += sprintf( cmdline, "%s", daemonPath );
			pstart++;
			for ( i=1; i<arg; i++ )
			{
				pstart += sprintf( pstart, "%s ", argv[i] );
			}
			*(++pstart) = 0;

			if (rc=DosExecPgm( objname, 64, EXEC_ASYNCRESULT,
					   cmdline, 0, &rcodes, daemonPath))
			{
				printf( "License daemon startup failed: \n" );
				printf( "\t%s %s\n", daemonPath, cmdline );
				printf( "DosExecPgm error code: 0x%x\n", rc );
				exit(EXIT_CANTEXEC);
			}

			/*
			 *	DosExecPgm returns process id in the
			 *	TerminateCode field.
			 */
			_child = (int)rcodes.codeTerminate; 
		}
#endif /* OS2 */
#if !defined(WINNT) && !defined(OS2)
		if (!daemon->path) /* P3203 */
		{
			argv[arg++] = "-path";
			patharg = arg++;
			argv[patharg] = daemon->path;

			argv[arg] = 0;
		}
#if 0 
	{ int i; printf("L:%d ",__LINE__); for (i = 0; i < arg; i++) printf("%s ", argv[i]); puts(""); }
#endif
		(void) execv(daemon->path, argv);
		DEBUG(("-path %s failed\n", daemon->path));
/*
 *		Failed: try it again, with the path tacked on
 */
		if (daemon->path && *daemon->path)
		{
			(void) strcpy(path, daemon->path);
			(void) strcat(path, "/");
			(void) strcat(path, daemon->name);
			if (patharg) /* P3203 */
				argv[patharg] = path;
#if 0 
	{ int i; printf("L:%d ",__LINE__); for (i = 0; i < arg; i++) printf("%s ", argv[i]); puts(""); }
#endif
			(void) execv(path, argv);
		}
		DEBUG(("-path %s failed L:%d\n", path, __LINE__));
/*
 *		try ./daemon
 */
		sprintf(path, "./%s", daemon->name);
		if (patharg) /* P3203 */
			argv[patharg] = path;
#if 0 
	{ int i; printf("L:%d ",__LINE__); for (i = 0; i < arg; i++) printf("%s ", argv[i]); puts(""); }
#endif
		execv(path, argv);
		DEBUG(("-path %s failed\n", path));
/*
 *		Try lmgrd's argv[0], if applicable
 */
		{
		  char *cp;

			l_zcp(path, ls_lmgrd_argv0, LM_MAXPATHLEN);
			cp = &path[strlen(path) - 1];
			while ((cp > path) && (*cp != PATHTERMINATOR))
				cp--;
			if (cp > path)
			{
				cp++;
				*cp = 0;
				strcpy(cp, daemon->name);
				/*argv[arg - 1] = path;*/
#if 0 
	{ int i; printf("L:%d ",__LINE__); for (i = 0; i < arg; i++) printf("%s ", argv[i]); puts(""); }
#endif
				execv(path, argv);
				DEBUG(("-path %s failed\n", path));
			}
		}
/*
 * 		Now try using $PATH 
 */
		argv[arg - 1] = "$PATH";
#if 0 
	{ int i; printf("L:%d ",__LINE__); for (i = 0; i < arg; i++) printf("%s ", argv[i]); puts(""); }
#endif
		execvp(daemon->name, argv);
		DEBUG(("execvp failed: %s\n", getenv("PATH")));
		LOG(( lmtext (
		"license daemon: execute process failed: (%s%s%s) -T %s %s %s -c %s\n"),
			daemon->path && *daemon->path ? daemon->path : "",
			daemon->path && *daemon->path ? " or " : "",
			path, master_name, version, port_desc, cf));
		LOG ((lmtext ("license daemon: system error code: %s\n"), 
							SYS_ERRLIST(errno)));
		exit(EXIT_CANTEXEC);
#endif /* !WINNT  && !OS2 */
	}

#ifndef PC	
	else if (_child < 0)
	{
		LOG((lmtext("INTERNAL error: Please report the following to your vendor\n")));
		LOG((lmtext("Vendor daemon fork failed! (errno: %d: %s)\n"), 
						errno, SYS_ERRLIST(errno)));
		LOG_INFO((INFORM, "The fork() system call failed."));
	}
/*
 *	The child never gets here
 */
	(void) close(s);
#endif /* PC */
	t = time(0);
	if ((t - daemon->time_started) > 120)
		daemon->num_recent_restarts = 0;
	daemon->time_started = t;
	daemon->num_recent_restarts++;
	daemon->pid = _child;
}



#ifdef WINNT

static
int
NT_Startup(char *vendors_name,
               DAEMON *daemon,
               char ** argv,
               char ** cmdline1,
                int args)
{
	PROCESS_INFORMATION process_info;
	char				cmdline[MAX_PATH] = {'\0'};
	wchar_t *			pwszCmdline = NULL;
	char *				pszCmdline = NULL;
	int					iSize = 0;
	char *				pstart = (char *)cmdline;
	int					i = 0;
	int					status = 0;
	BOOL				LastParmCFlag = FALSE;

	daemon->pid = daemon->print_pid  = -1; /* assume failure */
	pstart += sprintf( cmdline, "%s ",vendors_name );
	for ( i=1; i<args; i++ )
	{
		if (!strcmp(argv[i],(const char *)"-c") || !strcmp(argv[i],(const char *)"-C"))
		{
			LastParmCFlag = TRUE;
			pstart += sprintf( pstart, "%s ", argv[i] );
		}
		else if (LastParmCFlag==TRUE)
		{
			LastParmCFlag = FALSE;
			pstart += sprintf( pstart, "\"%s\" ", argv[i] );
		}
		else
			pstart += sprintf( pstart, "%s ", argv[i] );
	}
	if ( ls_log_specified)
	{
			strcat(cmdline, " -l \"");
			strcat(cmdline, ls_log_specified);
			strcat(cmdline, "\"");
	}

	/*
	 *      When lmgrd is run as a service, it must
	 *      start vendor daemon as DETACHED_PROCESS
	 *      so that no console window will be created
	 *      for vendor daemon.
	 */
	if(l_getOSFamily() == OS_95_98_ME)
	{
		STARTUPINFOA        startup_info;

		GetStartupInfoA( &startup_info );

		pszCmdline = l_convertStringUTF8ToMB(NULL, cmdline, &iSize);
		if(pszCmdline == NULL)
			return FALSE;

		status = CreateProcessA( NULL,      /* module image     */
							pszCmdline,    /* module and args  */
							NULL,       /* process security */
							NULL,       /* thread security  */
							TRUE,       /* inherit handles  */
							(ls_foreground == 1) ? 0 :
							((!ls_log_specified) && (ls_foreground == 2) )?
							CREATE_NEW_CONSOLE : DETACHED_PROCESS,
							NULL,       /* environment ptr  */
							NULL,       /* working dir      */
							&startup_info,
							&process_info );
		if(pszCmdline)
		{
			l_free(pszCmdline);
			pszCmdline = NULL;
		}

	}
	else
	{
		STARTUPINFOW	startup_info;

		GetStartupInfoW( &startup_info );

		pwszCmdline = l_convertStringUTF8ToWC(NULL, cmdline, &iSize);
		if(pwszCmdline == NULL)
			return FALSE;

		status = CreateProcessW( NULL,      /* module image     */
							pwszCmdline,    /* module and args  */
							NULL,       /* process security */
							NULL,       /* thread security  */
							TRUE,       /* inherit handles  */
							(ls_foreground == 1) ? 0 :
							((!ls_log_specified) && (ls_foreground == 2) )?
							CREATE_NEW_CONSOLE : DETACHED_PROCESS,
							NULL,       /* environment ptr  */
							NULL,       /* working dir      */
							&startup_info,
							&process_info );
		if(pwszCmdline)
		{
			l_free(pwszCmdline);
			pwszCmdline = NULL;
		}
	}
	if(status == 0)
	{
		return FALSE;
	}

	daemon->print_pid = (int)process_info.dwProcessId;
	daemon->pid = (int)process_info.hProcess;
	*cmdline1=cmdline;
	return TRUE;

}


#endif

