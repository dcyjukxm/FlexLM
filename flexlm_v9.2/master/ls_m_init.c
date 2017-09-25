/******************************************************************************

      COPYRIGHT (c) 1988, 2003 by Macrovision Inc.
  This software has been provided pursuant to a License Agreement
  containing restrictions on its use.  This software contains
  valuable trade secrets and proprietary information of
  Macrovision Inc and is protected by law.  It may
  not be copied or distributed in any form or medium, disclosed
  to third parties, reverse engineered or used in any manner not
  provided for in said License Agreement except with the prior
  written authorization from Macrovision Inc.

 *****************************************************************************/
/*
 *  Module: $Id: ls_m_init.c,v 1.78.2.6 2003/07/01 17:04:21 sluu Exp $
 *
 *  Function: ls_m_init(argc, argv, list, tcp_s, spx_x)
 *
 *  Description:  Master Server initialization - checks machine parameters
 *
 *  Parameters: data is read from the configuration file.
 *      (int) argc - # of command arguments
 *      (char **) argv - the arguments
 *      (LM_SERVER **) list - list of servers - filled in
 *      (LM_SOCKET *) TCP socket that lmgrd is listening on
 *      (LM_SOCKET *) SPX socket that lmgrd is listening on
 *
 *  Return:   function return - socket descriptor
 *      Program exits if any checks fail.
 *
 *  M. Christiano
 *  2/15/88
 *
 */

#include "lmachdep.h"
#ifndef PC
#include <sys/time.h>
#else
#include <fcntl.h>

#endif /* PC */
#include "lmclient.h"
#include "lgetattr.h"
#include "l_timers.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lm_attr.h"
#include "lsmaster.h"
#include "lsserver.h"
#include "ls_glob.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#include <sys/types.h>
#include "lssignal.h"
#include "ls_sprot.h"
#include "flex_file.h"
#include "flex_utils.h"
#include "flexevent.h"
#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/param.h>
#include <locale.h>

#ifdef WINNT
#include <windows.h>
#endif

#define TIMESTAMP_DEFAULT 60*60*6 /* Timestamp every 6 hours */

extern int unpriv_utils;    /* Utilities don't require priv */
extern char ls_our_hostname[];
extern int ls_allow_lmdown;
extern int ls_allow_lmremove;
extern char *ls_license_finder;
extern char **argv_sav;
extern DAEMON *master_daemons;
extern int argc_sav;
extern int ls_fflush_ok;
extern int ls_lmgrd_tcp_port;
#ifdef WINNT
char * ls_log_specified = 0;
extern int run_lmgrd_as_service;
extern int run_lmgrd_as_applet;
char ServiceName[128];
char lmgrd_log_path[MAX_LONGNAME_SIZE + 1];
SERVICE_STATUS          GSIServiceStatus;
SERVICE_STATUS_HANDLE   GSIServiceStatusHandle;
extern VOID Process_Service_Message(DWORD dwCtrlCode);

#endif
char * ls_lmgrd_argv0;
int max_vd_connections;
int ls_local;
time_t ls_test_hang;
time_t ls_test_exit;
time_t ls_start_time;
int ls_force_shutdown;


static void usage();
static void check_already_running lm_args((lm_noargs));

/* The signal handlers */
#ifdef VOID_SIGNAL

void ls_sigusr1 lm_args((int sig)), ls_sigusr2 lm_args((int sig));
#else
int ls_sigusr1(), ls_sigusr2(), ls_kill_chld();
#endif

#define LM_VER_BEHAVIOR LM_BEHAVIOR_CURRENT
#ifndef lint
static junk[] = {0x3e15ee26, 0xac826896, 0x5f4e64c5, 0xa410b77c };
#endif
#define LM_STRENGTH LM_STRENGTH_LICENSE_KEY
#define CRO_KEY1 0
#define CRO_KEY2 0
LM_CODE (code, 0x8c441d58, 0x58aef652,
/*- FLEXlm vendor keys here: */
/*-
 *  Generate this code with:
 *
 *    lmvkey -v lmgrd -d 1-jan-90 -p ALL -c ALL
 */
  0xb1de5081, 0x72f9b7d2, 0xb8e4a32a, 0x1b221d41, 0x671e57ea);

#ifndef lint
static junk2[] = {0xc215d132, 0x657a2ea0, 0x60f99768 };
#endif
static void process_args();

/*
 *  Signal handler to interrupt the select() in ls_m_main()
 */

int ls_timer_expired = 0;

static
#ifdef VOID_SIGNAL
void
#endif
_timer(sig)     /* interrupt the select in ls_m_main() */
int sig;
{
  ls_timer_expired = 1;
}

#if !defined( RELEASE_VERSION) && !defined(SIGNAL_NOT_AVAILABLE)
void
_hang(i)
{
  DLOG(("hanging for 4 minutes\n"));
  signal(SIGURG, _hang);
  l_select_one(0, -1, 1000*60*4);
  DLOG(("back\n"));

}
#endif /* RELEASE_VERSION */

extern int _lm_timestamp_interval;  /* Interval for master log timestamping */
static char *myname = "license manager";

/*
 *  List of signals to ignore, and ones to kill children with
 */
#ifndef SIGNAL_NOT_AVAILABLE
static int normal_signals[] = {  SIGTSTP, SIGCONT
#ifdef apollo
            /*- APOLLO does it again.
               SIGCHLD doesn't work unless
               SIGCLD is "normal" */
        , SIGCLD
#endif
        ,SIGBUS, SIGSEGV };

#ifdef RELEASE_VERSION
static int kill_signals[] =   { SIGTERM, SIGILL, SIGIOT, SIGFPE,
#if !defined(PCRT) && !defined(DGUX) && !defined(MOTO_88K)
        SIGEMT,
#endif
#ifdef apollo   /*- Apollo sends SIGQUIT or SIGHUP on node shutdown */
        SIGQUIT, SIGHUP,
#endif
        };
#endif /* RELEASE_VERSION */
#endif /* SIGNAL_NOT_AVAILABLE */

static
#ifdef VOID_SIGNAL
void
#endif
ls_reread_static lm_args((int));
int ls_foreground = 0;
static void background lm_args((lm_noargs));

extern LM_SOCKET tcp_s, spx_s;

int ls_m_init( int argc, char *argv[], LM_SERVER **list, int reread)
{
  char errmsg[100];
  LM_SERVER *master_list;
  int i;
  int ret;
  char hostname[MAX_HOSTNAME+1];
#ifdef WINNT
  extern char * lm_flexlm_dir;
  char buf[200];
  extern int is_lmgrd_a_service;
#endif
  char *	ppszInStr[20] = {NULL};

  if (!reread)
  {
    ls_lmgrd_argv0 = argv[0];
    ls_start_time = time(0);
    {
      LM_OPTIONS cur_options;
      LM_HANDLE dummy_job;
#ifdef USE_WINSOCK
      WSADATA wsadata;
      WSAStartup( (WORD)WINSOCK_VERSION, &wsadata );
#endif /* USE_WINSOCK */

/*
 *    Set up to "cache" the license file for clients that use
 *    port@host
 */
      memset((char *)&dummy_job, 0, sizeof(dummy_job));
      dummy_job.options = &cur_options;
      dummy_job.type = LM_JOB_HANDLE_DSPECIAL_TYPE;
      cur_options.cache_file = 1;
      cur_options.disable_finder = 1;
      if (lc_init(&dummy_job, "lmgrd", &code, &lm_job))
      {
        if (lm_job->lm_errno == LM_CANTMALLOC)
        {
          printf("Out of memory, exiting\n");
          exit(lm_job->lm_errno);
        }
        else
        {
          lc_perror(lm_job, "FLEXlm initialize failed\n");
          exit(lm_job->lm_errno);
        }

      }


      if (LM_TOOL_ERR_CATCH(lm_job))
      {
        printf("Out of memory, exiting\n");
        exit(lm_job->lm_errno);
      }
      l_set_attr(lm_job, LM_A_PERROR_MSGBOX, (LM_A_VAL_TYPE) 0);
      if (lm_job) lm_job->first_job = lm_job; /* Fix up */
      lm_job->vendor[0] = '\0';       /* lmgrd's name was always "" */
      lm_job->flags |= LM_FLAG_LMGRD;
    }
    ls_s_init();  /* Initialize the server database */
    argv_sav = argv;
    argc_sav = argc;
    l_text();
/*
 *  Process the input args
 */
#ifdef WINNT
/*
 *  Since lmgrd on Windows NT may be run as an application or as a
 *  system Service, and a Service program does not have stdout and stdin,
 *  we must redirect the ascii log to the right place.
 */
    GetWindowsDirectory(buf, 200);

    if (buf[0]==65)
    {
      lm_flexlm_dir[0]=buf[0];

    }

    if ( run_lmgrd_as_service )
    {
      char file_ptr[MAX_PATH + 1] = {'\0'}, * ff = NULL;  /* LONGNAMES */
      char szLogFile[MAX_PATH + 1] = {'\0'};        /* LONGNAMES */
      is_lmgrd_a_service = run_lmgrd_as_service;  /* used during shutdown to prevent
                               lmgrd from shutting down too early */

      /* initialize service control manager callback routine */
      /* NOTE: we currently do nothing when PAUSE or RESUME is request from SCM. */
      GSIServiceStatus.dwServiceType        = SERVICE_WIN32;
      GSIServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
      GSIServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP |
                          SERVICE_ACCEPT_SHUTDOWN |
                          SERVICE_ACCEPT_PAUSE_CONTINUE;
      GSIServiceStatus.dwWin32ExitCode      = 0;
      GSIServiceStatus.dwServiceSpecificExitCode = 0;
      GSIServiceStatus.dwCheckPoint         = 0;
      GSIServiceStatus.dwWaitHint           = 0;

      GSIServiceStatusHandle = RegisterServiceCtrlHandler( ServiceName,
                                (LPHANDLER_FUNCTION)Process_Service_Message
                                );

/*      char file_ptr[ MAX_PATH + 1 ], *ff;*/
      /*
       *  This instance of lmgrd.exe is run as a service.
       *  There is no 'stdout' for Windows NT Service,
       *  use the generice log file lmgrd.log until the
       *  -L option is processed.  See below for -L.
       */

      getenv_service( ServiceName,"LMGRD_LOG_FILE", file_ptr, MAX_PATH);
      if (file_ptr[0])
      {
		ls_log_open_ascii(file_ptr);
      }
      else
      {
        ff = l_getEnvUTF8(lm_job, "LMGRD_LOG_FILE", szLogFile,
          sizeof(szLogFile));
        if (ff)
          ls_log_open_ascii(ff);
        else
          ls_log_open_ascii("LMGRD.LOG");
      }
    } else
#endif
    ls_log_open_ascii("stdout");  /* Start log off right */
    if (LM_TOOL_ERR_CATCH(lm_job))
    {
      LOG(("Out of memory, exiting\n"));
      exit(lm_job->lm_errno);
    }
    _lm_timestamp_interval = TIMESTAMP_DEFAULT;
    process_args(argc, argv);
#ifdef FLEX_BETA
    LOG(("For Beta Test Use Only\n"));
#endif
#ifdef UNIX
    if (getuid() == 0)
    {
      LOG((lmtext("lmgrd running as root:\n")));
      LOG((lmtext("\tThis is a potential security problem\n")));
      LOG((lmtext("\tAnd is not recommended\n")));
    }
#endif /* UNIX */
    if ( !ls_foreground
#ifdef WINNT
      && !run_lmgrd_as_service
#endif
        )
      background();
#ifdef WINNT
    else
      SetConsoleTitle("lmgrd:  FLEXlm license server");
#endif /* WINNT */



#ifndef PC
/*
 *    Get rid of everything but stdout and stderr.
 */
    for (i = 0; i < lm_nofile; i++)
      if (i != 1 && i != 2)
          close(i);
    if (ls_fflush_ok) setlinebuf(stdout);
    if (ls_fflush_ok) setlinebuf(stderr);
#endif /* PC */
    /* Time out connects in 5 sec */
    lm_job->options->conn_timeout = 5;


    ls_mk_flexlm_dir();
/*
 *  set up statfile
 */
    ls_statfile_argv0(argv[0]);
    ls_statfile();
  }



  if (ls_license_finder)
  {
    tcp_s = LM_BAD_SOCKET;  /* We will open this later */
  }
  else
  {
    master_list = l_master_list_from_job(lm_job);
    if (!master_list)
    {
      sprintf(errmsg, "%s: can't initialize", myname);
      lm_perror(errmsg);
      if (_lm_errno == NOCONFFILE)
      {
		LOG((lmtext("Using license file \"%s\"\n"), lm_lic_where()));
		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = lm_lic_where();


			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_LMGRD_LICENSE_FILE,
								1,
								ppszInStr,
								0,
								NULL);			
		}
      }
      ls_statfile_rm();
      exit(_lm_errno);
    }
/*
 *    Make sure there's no port number conflicts in license files
 */
    if (!reread)
    {
      int port = -99;
      CONFIG *c;
      for (c = lm_job->line; c; c = c->next)
      {
        if (!c->server) continue;
        if (c->server->port)
        {
          if (port != -99 &&
            c->server->port != port)
          {
            char **cpp;
            LOG((lmtext(
    "ERROR: license files have conflicting port numbers\n")));
            LOG(("   %d and %d\n",
          port, c->server->port));
            LOG((
    "    Please edit the license files so the port numbers\n"));
            LOG((
    "    on the SERVER line are identical.\n"));
            LOG((
    "    License files include:"));
            for (cpp = lm_job->lic_files;
              *cpp; cpp++)
              _LOG((" %s", *cpp));
            _LOG(("\n" ));
            LOG((lmtext("Exiting\n")));
            exit(2);
          }

        }
        port = c->server->port;
      }
    }

/*
 *        Now, fill in the data for the connection message
 *        Make sure we are a valid host
 */
    *list = master_list;
    i = 0;
    for (master_list = *list; master_list;
      master_list = master_list->next)
    i++;
    if ((i % 2) == 0)
    {
      LOG(("\n"));
      LOG((lmtext("WARNING: %d servers in license file.\n"),
                i));
      if (i > 1)
      {
        LOG((lmtext("(an even number of servers is probably a mistake.)\n")));
        LOG((lmtext("You may have duplicated a server line by mistake\n")));
      }
      LOG(("\n"));
    }
    master_list = *list;
    while (master_list && !ls_on_host(master_list->name))
    {
      if (master_list->sflags & L_SFLAG_THIS_HOST)
      {
        gethostname(hostname, MAX_HOSTNAME);
        strcpy(master_list->name, hostname);
        break;
      }

      master_list = master_list->next;
    }
    if (master_list == NULL)
    {
/*
 *      We are not one of the hosts in the master list
 */
      gethostname(hostname, MAX_HOSTNAME);
      LOG((lmtext(
          "\"%s\": Not a valid server hostname, exiting.\n"),
                hostname));
      LOG((lmtext("Valid server hosts are: ")));
      for (master_list = *list; master_list;
        master_list = master_list->next)
      {
		
			_LOG(("\"%s\" ", master_list->name));
      }
      _LOG(("\n"));
      LOG((lmtext("Using license file \"%s\"\n"),
              lm_lic_where()));
      ls_statfile_rm();
      exit(2);
    }
    if (!reread)
    {
      master_daemons = l_get_dlist(lm_job);
    }
    l_zcp(ls_our_hostname, master_list->name, MAX_HOSTNAME);
    if (!reread)
    {
      int ret = -1;

      if (master_list->port == -1 )
      {
        /* if a server is already running and serving our vendor daemon
        * then this will cause an exit().  */
        check_already_running();

        /* port is not set so use a default port */
        for (master_list->port = LMGRD_PORT_START;
          (master_list->port <= lm_job->port_end) && (ret < 0);  )
        {
          /* loop through all the default ports until we find
           * one we can bind to. (it's not already in use) */
          if ((ret = ls_bind_socket(&tcp_s, &spx_s, master_list)) < 0)
          {
            master_list->port ++;
          }
        }
        if ( ret == -1 )
          LOG((lmtext("Failed to open any default TCP port.\n")))
      }
      else
      {
        /* use the specified port */
        /* P6771  Kmaclean 10/25/02
         * save the return value and actually check it for an error */
        ret = ls_bind_socket(&tcp_s, &spx_s, master_list);
        /*
         *      setting ls_lmgrd_tcp_port to -1
         *      means that the port number was
         *      specified in the file.
         */
        ls_lmgrd_tcp_port = -1;
        if ( ret == -1 )
		{
          LOG((lmtext("Failed to open the TCP port number in the license.\n")))
			if(l_flexEventLogIsEnabled())
			{
				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_NETWORK_COMM,
									MSG_FLEXLM_LMGRD_PORT_OPEN_FAILED,
									0,
									ppszInStr,
									0,
									NULL);
			}
		}
      }
      if ( ret == -1)
      {
        /* P6771  -  Kmaclean 10/25/02
         * we failed to open the port so we can't start a daemon.
         * Quit. */
        /* clean-up first.
         * I'd think we should do more than ls_statfile_rm() to clean-up
         * but this is all the other exits do here. */
        ls_statfile_rm();

        exit(EXIT_PORT_IN_USE);
      }
    }
  }
/*
 *  Set up our signals
 */
  if (!reread)
#ifndef PC
  {
    for (i=1; (i <= NSIG); i++)
    {
      switch (i)
      {
        case SIGHUP:
        case SIGINT:
        case SIGPIPE:
        case SIGALRM:
        case SIGTERM:
        case SIGUSR1:
        case SIGUSR2:
#ifndef MAC10
        case SIGCLD:
#endif
        /*case SIGWINCH:*/
        case SIGSTOP:
        case SIGTSTP:
        case SIGCONT:
        case SIGTTIN:
        case SIGTTOU:
#if !defined( SCO) && !defined(cray) && !defined(NECSX4)
        case SIGURG:
        case SIGIO:
        case SIGVTALRM:
        case SIGPROF:
#endif
        signal(i, SIG_IGN);
        break;
        default:
        break;
      }
    }
#else
    for (i=1; (i < NSIG); i++)
      signal(i, SIG_IGN);
    signal(SIGINT, (void(*)(int))ls_kill_chld);
#endif
#if defined(RELEASE_VERSION) && !defined(SIGNAL_NOT_AVAILABLE)
    for (i=0; i < sizeof(kill_signals) / sizeof(kill_signals[0]); i++)
      signal(kill_signals[i], ls_kill_chld);
#else
      signal(SIGTERM, (void(*)(int))ls_kill_chld);
#endif

#ifndef PC
    for (i=0; i < sizeof(normal_signals) / sizeof(normal_signals[0]); i++)
      signal(normal_signals[i], SIG_DFL);

    signal(SIGCHLD, ls_chld_died);
#if !defined( RELEASE_VERSION) && !defined(SIGNAL_NOT_AVAILABLE)
    signal(SIGURG, _hang);
#endif /* RELEASE_VERSION */
#endif /* PC */

#ifndef SIGNAL_NOT_AVAILABLE
    signal(REREAD_SIGNAL, ls_reread_static);
#endif /* SIGNAL_NOT_AVAILABLE */
/*
 *  Set up the timer and its signal handler, so that we
 *  are assured of being waked up at least once during
 *  each _lm_timestamp_interval.  We set this timer to 1
 *  minute so that the daemon will wake up every minute
 *  and check for stuff like timeouts from other servers, etc.
 *
 *  Note: No timer for license_finder daemons
 */
#ifndef PC /* PC does this by select() timeout */
    if (!ls_license_finder)
    {
      if (!(l_timer_add(lm_job, LM_REAL_TIMER, 60000,
          (FP_PTRARG)_timer, LM_TIMER_LMGRD, 60000)))
        return(_lm_errno);
    }
  }
  else
#endif /* WINNT */
    LOG(("Done rereading\n"));


  return(tcp_s);
}

/*
 *  Process the input arguments
 */

extern int use_v1_spawn;
#ifndef NO_GETDOMAINNAME_CALL
extern int add_domain_name;
#endif

static
void
process_args(argc, argv)
int argc;
char *argv[];
{
#ifdef WINNT
	if (run_lmgrd_as_service)  /* This is a service, read from the registry
*/
	{
		char  file_ptr[MAX_PATH+1] = {'\0'};
		char * token;
		getenv_service( ServiceName,"LMGRD_LOG_FILE", lmgrd_log_path, MAX_PATH);
		if (lmgrd_log_path[0])
		{
			if(lmgrd_log_path[0] != '+')
				_unlink(lmgrd_log_path);
			ls_log_reopen_ascii(lmgrd_log_path);
			ls_log_specified = lmgrd_log_path;
		}
		else
		{
			ls_log_open_ascii("LMGRD.LOG");
			ls_log_specified = "+LMGRD.LOG";
		}
		getenv_service( ServiceName, "License", file_ptr, MAX_PATH);
		if (file_ptr)
		{
		  lm_set_attr(LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
		  lm_set_attr(LM_A_LICENSE_FILE_PTR,
													(LM_A_VAL_TYPE)file_ptr);
		}

		getenv_service(ServiceName,"LMGRD_LMUTIL",file_ptr, MAX_PATH );
		if (file_ptr)
		{
			if (!strcmp(file_ptr,"lmdown"))
				ls_allow_lmdown=0;
			if (!strcmp(file_ptr,"lmremove"))
				ls_allow_lmremove=0;
		}
		/*
		 *  -local, -x lmdown, -x lmremove settings for advanced settings in lmtools
		 *	utility. Values are read from registry. Setting and and removing of options
		 *  only should be done within lmtools.
		 */
		getenv_service(ServiceName,"cmdlineparams",file_ptr, MAX_PATH );

		token = strtok(file_ptr, " ");

		while(token != NULL)
		{
			if (strcmp(token, "-local") == 0)
			{
				ls_local = 1;
			}
			if (strcmp(token, "-x") == 0)
			{
				token = strtok(NULL, " ");
				if (strcmp(token, "lmdown") == 0)
				{
					ls_allow_lmdown = 0;
					LOG((lmtext("lmdown requests disabled\n")));
				}

				/* We want to disable lmremove NOT lmreread. */
				/* if (strcmp(token, "lmreread") == 0) */
				if (strcmp(token, "lmremove") == 0)
				{
					ls_allow_lmremove=0;
				}
			}
			token = strtok(NULL, " ");
		} /* end while */
		return;
	}

#endif /* WINNT */

  while (argc > 1)
  {
	if (!strcmp(argv[1], "-nfs_log"))
    {
      ls_fflush_ok = 0;
    }
    else if (!strcmp(argv[1], "-max_connections"))
    {
      argv++; argc--;
      sscanf(argv[1], "%d", &max_vd_connections);   /* overrun threat */
    }
    else if (!strcmp(argv[1], "-hang"))   /* For ESD testsuite only */
    {
            ls_test_hang = time(0) + 20;
    }
    else if (!strcmp(argv[1], "-exit"))   /* For ESD testsuite only */
    {
      argv++; argc--;
      sscanf(argv[1], "%d", &ls_test_exit);   /* overrun threat */
            ls_test_exit += time(0);
    }
#if 0
    else if (!strcmp(argv[1], "-vendor"))
    {
      l_zcp(lm_job->alt_vendor, argv[2], MAX_VENDOR_NAME);
      argc--; argv++;
    }
#endif
    else if (*argv[1] == '-')
    {
      char *p = argv[1]+1;

      switch(*p)
      {
        case '2':
        use_v1_spawn = 0;
        break;

#ifdef WINNT
        case 'a':
        case 'A':
        /*
         * '-app' command line arguement will make
         * lmgrd run as an application instead of
         * a Windows NT Service.  This flag is also
         * set in service.c.
         */
        run_lmgrd_as_service = FALSE;
        break;
#endif /* WINNT */
        case 'b':
        case 'B':
        use_v1_spawn = 1;
        break;

        case 'c':
        case 'C':
        if (argc > 2)
        {
          lm_set_attr(LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
          lm_set_attr(LM_A_LICENSE_FILE,
                (LM_A_VAL_TYPE)argv[2]);
          argc--; argv++;
        }
        break;

#if 0
#ifndef NO_GETDOMAINNAME_CALL
        case 'd':
        case 'D':
        add_domain_name = 1;
        break;
#endif
#endif /* Bug P1941 */

        case 'f':    /* Will not be exposed to customer */
        case 'F':
        ls_license_finder = argv[2];
        argc--; argv++;
        break;

        case 'h':
        case 'H':
        usage();
        break;

        case 'l':
        case 'L':
        if (!strcmp(p, "local"))
        {
        ls_local = 1;
        }
                          else if (argc > 2)
                          {
                            char *mode = "w+";

              if (*argv[2] == '+')
        {
          mode = "a+";
          argv[2]++;
        }
        else
        {
          l_flexUnlink(lm_job, argv[2]);
        }
        errno = 0;
        if (!l_flexFreopen(lm_job, argv[2], "a+", stdout))
        {
          fprintf(stderr,
           "Cannot open \"%s\" as log file (%d)\n"
            , argv[2], fileno(stdout));
          perror("freopen");
          exit(errno);
                                }
#ifdef WINNT
        ls_log_specified = argv[2];
#endif
        argc--; argv++;
                          }
        break;

        case 'p':
        case 'P':
        unpriv_utils = 0;
        break;

        case 's':                /* Will not be exposed to customer */
        case 'S':
        if (argc > 2)
        {
            _lm_timestamp_interval = 60 * atoi(argv[2]);
            argc--; argv++;
        }
        break;

        case 't':
        case 'T':
        if (argc > 2)
        {
            lm_job->options->conn_timeout =
                atoi(argv[2]);
            argc--; argv++;
        }
        break;

        case 'v':
        case 'V':
/*
 *        Print the version/copyright and exit
 */
        fprintf(stdout,
  "lmgrd v%d.%d%s - "COPYRIGHT_STRING(1988)"\n",
            lm_job->code.flexlm_version,
            lm_job->code.flexlm_revision,
            lm_job->code.flexlm_patch
            );
        exit(0);
        break;

        case 'x':
        case 'X':
        if (argc > 2)
        {
          if (!strcmp(argv[2], "lmdown"))
          {
            ls_allow_lmdown = 0;
            LOG((lmtext(
            "lmdown requests disabled\n")));

          }
          else if (!strcmp(argv[2], "lmremove"))
          {
            ls_allow_lmremove = 0;
          }
          else
          {
            LOG((lmtext(
        "-x option missing arg lmdown or lmremove")));
            usage();
          }
          argc--; argv++;
        }
        else
        {
          LOG((lmtext(
        "-x option missing arg lmdown or lmremove")));
          usage();
        }
        break;
#ifdef WINNT
                          case 'w':
                          case 'W':
                               run_lmgrd_as_applet=TRUE;
                               break ;
#endif /* WINNT */

        case 'z':
        case 'Z':
        ls_foreground = 1;
        /* we're already running in the backround */
        if (p[1] == '2') ls_foreground = 2;
#ifdef WINNT
                                if (getpid() & 0xffff0000)
                                {
                                        LOG(("pid %X\n", getpid()));
                                }
                                else
                                {
                                        LOG(("pid %d\n", getpid()));
                                }

#endif
        break;

        default:
        LOG((lmtext("Unrecognized command-line switch \"%c\"\n")
              , *p));
        usage();
        break;
      }
    }
    argc--; argv++;
  }

  if (use_v1_spawn && !unpriv_utils)
  {
    LOG(("\n"));
    LOG((lmtext("WARNING: -p ineffective with the V1\n")));
    LOG((lmtext("vendor daemon startup protocol.\n")));
    LOG((lmtext("lmgrd will still require a license administrator\n")));
    LOG((lmtext("for the use of \"lmdown\" and \"lmreread\"\n")));
    LOG((lmtext("but the vendor daemons will not enforce\n")));
    LOG((lmtext("this restriction for \"lmremove\"\n")));
    LOG((lmtext("If you do not have any pre-V2 vendor daemons\n")));
    LOG((lmtext("use the \"-2\" switch in addition to \"-p\"\n")));
    LOG(("\n"));
  }
}
/*
 *  ls_reread_static() - re-read the configuration files
 */

static
#ifdef VOID_SIGNAL
void
#endif
ls_reread_static(i)
int i;
{
#ifndef SIGNAL_NOT_AVAILABLE
  signal(REREAD_SIGNAL, ls_reread_static);
#endif
}

/*
 *  Stub for ls_unlock()
 */
/* ARGSUSED */
void
ls_unlock(remove)
int remove;
{
}

/*
 *  Bind the socket.  Used after initialization only on apollo.
 */
ls_bind_socket(tcp_s, spx_s, master )
LM_SOCKET *tcp_s;
LM_SOCKET *spx_s;
LM_SERVER *master;
{
  COMM_ENDPOINT endpoint;

  endpoint.transport =
#ifdef FIFO
    lm_job->options->commtype == LM_LOCAL ? LM_LOCAL :
#endif /* FIFO */
    LM_TCP;
  endpoint.transport_addr.port = ntohs((unsigned short)master->port);
  *tcp_s = ls_socket(ls_our_hostname, &endpoint, 0);

#ifdef ACCEPT_BUG
#ifdef FIFO
  if (lm_job->options->commtype != LM_LOCAL)
#endif /* FIFO */
    if (listen(*tcp_s, LISTEN_BACKLOG) < 0)
    {
      char errmsg[100];

      sprintf(errmsg, "%s: listen", myname);
      perror(errmsg);
      exit(EXIT_COMM);
    }

#endif
  /* Give the signal handlers the socket f.d.  */
  ls_store_socket(*tcp_s, LM_BAD_SOCKET, *spx_s);


#ifdef SUPPORT_IPX
  if (master->spx_addr.sa_family == AF_NS)
  {
    endpoint.transport = LM_SPX;
    endpoint.transport_addr.spx_addr = master->spx_addr;
    *spx_s = ls_socket(ls_our_hostname, &endpoint, 0);
  }
  else
#endif
    *spx_s = LM_BAD_SOCKET;

  return(*tcp_s);
}
static
void
usage()
{
  LOG(("\n"));
  LOG((lmtext("usage: lmgrd [-2 -p] [-z] [-c license_file_list]\n")));
  LOG((lmtext("             [-v] [-l [+]debug_log_path][-local]\n")));
  LOG((lmtext("             [-x lmdown|lmremove] [-help]\n")));
  LOG(("\n"));
  exit(1);
}

/*
 *  background -- run lmgrd as background process
 */
static
void
background()
{
#ifndef PC /* PC server works in the foreground. */
  int pid;
#if !defined(GLIBC) && !defined(ALPHA_LINUX) && !defined(VMLINUX64) \
      && !defined(RHLINUX64) && !defined(MAC10)
  extern char *sys_errlist[];
#endif
  extern int sys_nerr;

  errno = 0;
  pid = fork();
  if (BADPID(pid))
  {
    LOG((lmtext(
  "Error (%s) putting lmgrd into background, running in foreground\n"),
        SYS_ERRLIST(errno<SYS_NERR?errno:0)));
  }
  else if (CHILDPID(pid))
  {
    exit(0); /* parent process */
  }
#else  /* PC */
	BOOL opOK;
	PROCESS_INFORMATION processInfo;
	char buf[MAX_PATH * 2] = {'\0'};
	char * pszTemp = NULL;
	wchar_t wbuf[MAX_PATH * 2] = {0};
	wchar_t * pwszTemp = NULL;
	int iOSFamily = l_getOSFamily();
	/* Run as an application. */
	if ( run_lmgrd_as_service)
		return;

	LOG(("Running lmgrd in dedicated windows ...\n"));
	LOG(("\tUse -z to run in foreground in this window\n"));

	if(iOSFamily == OS_95_98_ME)
	{
		STARTUPINFOA startupInfo;
		char	buf[MAX_PATH * 2] = {'\0'};
		int i = 0;

		strcpy(buf, GetCommandLine());
		strcat(buf, " -z2");
		memset(&startupInfo, 0, sizeof(startupInfo));

		startupInfo.cb = sizeof(startupInfo);
		opOK = CreateProcessA(
							0,	    // lpApplicationName
							buf,	    // lpCommandLine
							NULL,		// lpProcessAttributes
							NULL,		// lpThreadAttributes
							FALSE,		// bInheritHandles,
							(ls_log_specified ? DETACHED_PROCESS : CREATE_NEW_CONSOLE)
							/*| CREATE_NEW_PROCESS_GROUP*/,	// dwCreationFlags,
							NULL,		// lpEnvironment,
							NULL,		// lpCurrentDirectory,
							&startupInfo,	// lplStartupInfo,
							&processInfo );	// lpProcessInformation

	}
	else
	{
		STARTUPINFOW startupInfo;
		wchar_t	*pTemp = NULL;
		int i = 0;

		wcscpy(wbuf, GetCommandLineW());
		pwszTemp = l_convertStringUTF8ToWC(NULL, " -z2", &i);
		if(pwszTemp == NULL)
		{
			return;
		}
		wcscat(wbuf, pwszTemp);
		l_free(pwszTemp);
		pwszTemp = NULL;

		/*
		 *	Convert it to UTF8 just in case an error occurs so we can output it.
		 */
		pszTemp = l_convertStringWCToUTF8(NULL, wbuf, &i);
		if(pszTemp)
		{
			if(i > sizeof(buf))
			{
				i = sizeof(buf) - 1;
			}
			strncpy(buf, pszTemp, i);
			buf[i] = '\0';
			l_free(pszTemp);
			pszTemp = NULL;
		}
		else
		{
			buf[0] = '\0';
		}
		memset(&startupInfo, 0, sizeof(startupInfo));

		startupInfo.cb = sizeof(startupInfo);
		opOK = CreateProcessW(
							0,	    // lpApplicationName
							wbuf,	    // lpCommandLine
							NULL,		// lpProcessAttributes
							NULL,		// lpThreadAttributes
							FALSE,		// bInheritHandles,
							(ls_log_specified ? DETACHED_PROCESS : CREATE_NEW_CONSOLE)
							/*| CREATE_NEW_PROCESS_GROUP*/,	// dwCreationFlags,
							NULL,		// lpEnvironment,
							NULL,		// lpCurrentDirectory,
							&startupInfo,	// lplStartupInfo,
							&processInfo );	// lpProcessInformation

	}
	if(!opOK)
	{
		fprintf(stderr,
		"CreateProcess of \"%s\" failed, error: %d\n",
				buf, GetLastError());
	}
	else
	{
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		exit(0);
	}
#endif /* WINNT */
}
/*
 *  If it's running on a default port, and there's an lmgrd
 *  for every daemon, exit
 */
static
void
check_already_running()
{
  DAEMON *dp;
  LMGRD_STAT *lps, *lp = 0;
  int ok = 0;
  char dname[MAX_VENDOR_NAME + 1];
  char *cp;
  char *cdp;
  int found;

#ifdef PC
    LOG(("Detecting other lmgrd processes...\n"));
#endif
  lps = l_lmgrds(lm_job, 0);
  for (dp = master_daemons; dp; dp = dp->next)
  {
    for (lp = lps; lp; lp = lp->next)
    {
/*
 *      Find out if dp->name is in lp->vendor_daemons
 */
      found = 0;
      cp = lp->vendor_daemons;
      while (cp && *cp)
      {
        for (cdp = dname; *cp && *cp != ' '; )
          *cdp++ = *cp++;
        *cdp = 0;
        if (L_STREQ(dname, dp->name))
        {
          found = 1;
          break;
        }
        if (*cp) cp++;

      }

      if (!found) continue; /* this lmgrd doesn't support
            dp->name */

      if (!lp->up || !lp->vendor_daemons)
        continue;
      else
      {
        LOG(("%s already running %s\n", dp->name,
              lp->port_at_host));
        break;
      }
    }
    if (!found || !lp || !lp->up || !lp->vendor_daemons)
      ok = 1;
  }
  if (lps) lc_free_lmgrd_stat(lm_job, lps);
  if (master_daemons && !ok)
  {
    LOG(("lmgrd already serving all vendors, exiting\n"));
    lc_free_job(lm_job);
    exit(2);
  }
}
