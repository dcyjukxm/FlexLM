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
 *	Module: $Id: ls_app_init.c,v 1.84.2.6 2003/07/01 17:04:20 sluu Exp $
 *
 *	Function: ls_app_init(argc, argv, list, master_name, tcp_s)
 *		  ls_checkhost()
 *
 *	Description: Server initialization - checks machine parameters
 *
 *	Parameters:	data is read from the configuration file.
 *			argc, argv - The command line parameters
 *			(LM_SERVER **) list - list of servers - filled in.
 *			(LM_SOCKET *) tcp_s -- tcp socket fd - filled in.
 *
 *	Return:		none (void)
 *			Program exits if any checks fail.
 *			(char **) master_name - The name of the "master" node
 *			(LM_SOCKET *) tcp_s is filled in
 *
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#ifndef PC
#include <sys/time.h>
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "l_timers.h"
#include "lmselect.h"
#include "lm_attr.h"
#include "lgetattr.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_log.h"
#include "ls_comm.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "flex_file.h"
#include "flex_utils.h"
#include "flexevent.h"
#ifdef HP
#define FIRST_CLOSE_DESCRIPTOR 1
#else
#define FIRST_CLOSE_DESCRIPTOR 0
#endif
#include <sys/types.h>
#ifdef ANSI
#include <stdlib.h>
#ifdef UNIX
#include <unistd.h>
#endif /* UNIX */
#endif /* ANSI */

#ifndef NLM
#ifdef FIONBIO_IN_FILIO_H
#include <sys/filio.h>
#endif
#ifdef FIONBIO_IN_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/file.h>
#include <fcntl.h>  /* Need this for HP, even though file.h includes it
on the SUN */
#endif /* NLM */

#include "lssignal.h"
#ifndef PC
#include <sys/ioctl.h>
#endif
#include <stdio.h>
#include <errno.h>

#if defined(VMS) || defined(NLM) || !defined(RELEASE_VERSION)
#define ALLOW_STARTUP_FROM_COMMAND_LINE
#endif /* VMS | NLM | !RELEASE_VERSION */

#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif
#ifdef VMS
#include <ssdef.h>
#endif
#include <locale.h>

#ifdef UNLIMIT_DESCRIPTORS
#include <sys/resource.h> /* for getrlimit and setrlimit */
#endif /* UNLIMIT_DESCRIPTORS */

#ifdef PC
#define setlinebuf(x)
void  issue_outofdate_warning(void);
#endif /* PC */

/* The signal handlers */
#ifdef VOID_SIGNAL
void ls_control();
#else
int ls_control();
#endif
#ifdef VOID_SIGNAL
void
#endif
ls_exit lm_args((int sig));
#ifdef PC

typedef void (LM_CALLBACK_TYPE * LS_CB_USER_INIT1) (void);
#define LS_CB_USER_INIT1_TYPE (LS_CB_USER_INIT1)

typedef void (LM_CALLBACK_TYPE * LS_CB_USER_INIT2) (void);
#define LS_CB_USER_INIT2_TYPE (LS_CB_USER_INIT2)

typedef int (LM_CALLBACK_TYPE * LS_CB_OUTFILTER) (void);
#define LS_CB_OUTFILTER_TYPE (LS_CB_OUTFILTER)

typedef int (LM_CALLBACK_TYPE * LS_CB_INFILTER) (void);
#define LS_CB_INFILTER_TYPE (LS_CB_INFILTER)

typedef int (LM_CALLBACK_TYPE * LS_CB_INCALLBACK) (void);
#define LS_CB_INCALLBACK_TYPE (LS_CB_INCALLBACK)

typedef char * (LM_CALLBACK_TYPE * LS_CB_VENDOR_MSG) (char * i);
#define LS_CB_VENDOR_MSG_TYPE (LS_CB_VENDOR_MSG)

typedef char *  (LM_CALLBACK_TYPE * LS_CB_VENDOR_CHALLENGE) (char * i);
#define LS_CB_VENDOR_CHALLENGE_TYPE (LS_CB_VENDOR_CHALLENGE)

typedef int (LM_CALLBACK_TYPE * LS_CB_HOSTID_REDIRECT_VERIFY) (void);
#define LS_CB_HOSTID_REDIR_VERIFY_TYPE (LS_CB_HOSTID_REDIRECT_VERIFY)

typedef void (LM_CALLBACK_TYPE * LS_CB_DAEMON_PERIODIC) (void);
#define LS_CB_DAEMON_PERIODIC_TYPE (LS_CB_DAEMON_PERIODIC)


#else

#define  LS_CB_OUTFILTER_TYPE
#define  LS_CB_INFILTER_TYPE
#define  LS_CB_INCALLBACK_TYPE
#define  LS_CB_VENDOR_MSG_TYPE
#define  LS_CB_VENDOR_CHALLENGE_TYPE
#define  LS_CB_DAEMON_PERIODIC_TYPE
#define  LS_CB_HOSTID_REDIR_VERIFY_TYPE
#define  LS_CB_USER_INIT1_TYPE
#define  LS_CB_USER_INIT2_TYPE

#endif /* PC */

extern void (*ls_user_init1)();   /* User routine for prior-to-init */
extern void (*ls_user_init2)();   /* User routine for end-of-init */
extern void l_set_user_init3 lm_args((void (*)()));/* Set user routine for
                                                   licfile read */
extern void (*ls_user_init3)();   /* User routine for licfile read */
extern int (*ls_outfilter)();   /* Checkout filter */
extern int (*ls_infilter)();    /* Checkin filter */
extern int (*ls_incallback)();    /* Checkin callback */
extern int ls_use_all_feature_lines;  /* Use all feature lines flag */
extern int ls_normal_hostid;    /* Normal/extended hostid checking */
extern int ls_crypt_case_sensitive; /* Used for vendor-supplied encryption
                                    routines that are case-sensitive */
extern int ls_tell_startdate;
extern int ls_enforce_startdate;
extern int ls_do_checkroot;   /* Perform check for filesystem root */
extern int unpriv_utils;    /* Unpriviliged LMREREAD */
extern char *ls_user_lockfile;
extern char ls_our_hostname[];
extern int (*ls_hostid_redirect_verify)();
extern char *(*ls_vendor_msg)();
extern char *(*ls_vendor_challenge)();
extern int ls_min_lmremove;
extern int ls_use_featset;
extern int ls_show_vendor_def;
extern int ls_allow_borrow;
extern int (*ls_hostid_redirect_verify)();
extern void (*ls_daemon_periodic)();
extern int ls_allow_lmremove;
extern int ls_a_check_baddate;
extern char * ls_a_behavior_ver;
extern int ls_a_lkey_start_date;
extern int ls_a_lkey_long;
extern int ls_a_license_case_sensitive;
extern void (*ls_a_user_crypt_filter)();
extern void (*ls_a_phase2_app)();

int ls_lmgrd_tcp_port = -1;
time_t ls_test_hang;
time_t ls_start_time;
time_t ls_lmgrd_start = 0;

extern FEATURE_LIST fnull;
#ifdef VMS
extern char lockname[];
#endif
char ls_options[MAX_LONGNAME_SIZE + 1]; /* Our options file, if specified */ /* LONGNAMES */
extern int ls_fflush_ok;

VENDORCODE *vendorkeys;

/*
*      Note: Keep the copyright less than 47 characters wide.
*      also, because some C compilers (sunos4) can't initialize
*      an array string, I had to use the less attractive solution below
*/

char *copyright1 =
COPYRIGHT_STRING(1988) ". This software has been \
provided pursuant to a License Agreement \
containing restrictions on its use.  This \
software contains confidential information \
of Macrovision Corporation and is \
protected by copyright and other law.  It may \
not be copied or distributed in any form or \
medium, disclosed to third parties, reverse \
engineered or used in any manner not provided \
for in said License Agreement except with the \
prior written authorization from Macrovision \
Corporation.";

int ls_flexlmd = 0;
extern VENDORCODE *ls_keys;



HOSTID *vendor_daemon_hostid; /* Our host id (for periodic check) */
#ifdef DEFAULT_LIC_FILE_SPEC
extern char *default_license_file;
#endif
#define MAX_FD_FOR_PORT 1024

/*
*  List of signals to ignore, and ones to kill children with
*/
#if !defined(SIGNAL_NOT_AVAILABLE)

static int debug_signals[] = {  SIGQUIT, SIGBUS, SIGSEGV, SIGFPE, SIGILL
#ifndef RELEASE_VERSION
, SIGTSTP,
#ifndef VMS
SIGCONT,
#endif /* VMS */
SIGINT, SIGQUIT
#endif
};
/* DEBUG signals reset
to default action */
static int kill_signals[] = {   SIGIOT, SIGEMT, SIGTRAP,
SIGEMT };
#endif /* SIGNAL_NOT_AVAILABLE */

static void checklock();
void find_default_options_file lm_args((char *));
static void set_signals lm_args((lm_noargs));
static void file_to_dirname lm_args((char *, char *));



#ifdef PC
static int lm_nofile = 0;   /* Winsock has socket id to the whole int space. */
#endif
int timer_expired = 0;

#ifdef SUPPORT_UDP
static LM_SOCKET ls_setup_udp_socket
lm_args((LM_SERVER *master_list, int port));
static void ls_create_port_msg
lm_args((char *msg, int port, char *hostname, int commtype));
static unsigned short ls_get_lmgrd_endpoint
lm_args((LM_SERVER *master_list, char *hostname, int commtype,
        COMM_ENDPOINT *endpoint));
#endif  /* SUPPORT_UDP */

#ifndef NO_LMGRD
static void ls_send_port lm_args((LM_SERVER *master_list, char *hostname,
                                 int port, int commtype));
#endif /* NO_LMGRD */

#ifndef PC
lm_extern
#ifdef VOID_SIGNAL
void
#else
int
#endif /* VOID_SIGNAL */
ls_reread_sig lm_args((int));
#endif /* PC */
int ls_got_reread;
int ls_cpu_usage_delta = CPU_USAGE_DEFAULT_DELTA;
int ls_cpu_usage_interval = CPU_USAGE_DEFAULT_INTERVAL;
int ls_cpu_usage_last_logged = 0;

#if !defined( RELEASE_VERSION) && !defined(SIGNAL_NOT_AVAILABLE)
void
_hang(i)
{
  int minutes = 4;

  DLOG(("hanging for %d minutes\n", minutes));

  signal(SIGUSR1, _hang);
  l_select_one(0, -1, 1000*60*minutes);
  DLOG(("back\n"));
}
#endif /* RELEASE_VERSION */

#if !defined(SIGNAL_NOT_AVAILABLE)

void ls_shutdown(sig) int sig; {extern int _ls_going_down;
signal(sig, ls_shutdown);
/*DLOG(("Got shutdown signal %d\n", sig));*/
_ls_going_down = sig; }
#endif /* signal not available*/
int keysize;

void
ls_app_init(argc, argv, list, master_name, tcp_s, spx_s)
int argc;
char *argv[];
LM_SERVER **list; /* gets set to the list of 1 or 3 servers */
char **master_name;
LM_SOCKET *tcp_s;
LM_SOCKET *spx_s;
{

  LM_SERVER *master_list; /* gets set to the "current server" from *list */
  LM_SOCKET s;    /* The socket file descriptor opened by lmgrd --
                  *  to be assigned to tcp_s*/
  int v2args = 0;
  int i, j;
  int ret;
  int port = 0;   /* Internet port number to use */
  int ONE = 1;
  char vendor_name[MAX_DAEMON_NAME+1];
  unsigned char buf[MAX_CONFIG_LINE];
  int ver = -1, rev = -1;
  VENDORCODE vcode;
  extern int (*L_NEW_JOB)();
  char *	ppszInStr[20] = {NULL};


  ls_start_time = time(0);
  *ls_options = '\0'; /*initialize*/
  *tcp_s = *spx_s = LM_BAD_SOCKET;

  /*setlocale(LC_ALL, "en");*/ /* so the behavior is always the same */
  set_signals();
  keysize = (*L_NEW_JOB)(vendor_name, &vcode, 0, 0, 0, 0);
  if (!(vendorkeys =
    (VENDORCODE *)calloc(1, keysize * (sizeof(VENDORCODE) + 1))))
  {
    printf("Out of memory, exiting\n");
    ls_go_down(EXIT_MALLOC);
  }

  /*
   *	Init event logging, if it fails, start anyway
   */
  l_flexEventLogInit(NULL, vendor_name);

  memcpy(vendorkeys, &vcode, sizeof(VENDORCODE));
  /*
  *  Fill in remaining vendorkeys
  */
  for (i = 1; i < keysize;i++)
    (*L_NEW_JOB)(vendor_name, &vendorkeys[i], 0, 0, 0, 0);


  {
    LM_OPTIONS cur_options;
    LM_HANDLE dummy_job;

    /*
    *              Set up to disable the license finder
    */

    (void) memset((char *)&dummy_job, 0, sizeof(dummy_job));
    (void) memset((char *)&cur_options, 0, sizeof(cur_options));
    dummy_job.type = LM_JOB_HANDLE_DSPECIAL_TYPE;
    dummy_job.options = &cur_options;
    cur_options.disable_finder = 1;
    ls_keys = &vendorkeys[0];

    l_set_user_init3 (ls_user_init3);
#ifdef NO_LMGRD
    cur_options.cache_file=1;
#endif

    if (i = lc_init(&dummy_job, vendor_name, ls_keys, &lm_job)
      == LM_CANTMALLOC)
    {
      printf("Out of memory, exiting\n");
      ls_go_down(EXIT_MALLOC);
    }

    lm_job->flags |= LM_FLAG_IS_VD;
    if (LM_TOOL_ERR_CATCH(lm_job))
    {
      printf("Out of memory, exiting\n");
      ls_go_down(EXIT_MALLOC);
    }
    /*-
    *    This indicates that this job used L_NEW_JOB to
    *    for seeds, not l_svk
    */
    if(ls_use_all_feature_lines)
    {
      lm_job->flags |= LM_OPTFLAG_USE_ALL_FEATURE_LINES;
    }
    lm_job->options->flags |= LM_OPTFLAG_CUSTOM_KEY5;
    lm_job->options->flags &= ~ LM_OPTFLAG_CKOUT_INSTALL_LIC;
    if (ls_a_user_crypt_filter)
      lc_set_attr(lm_job, LM_A_USER_CRYPT_FILTER,
      (LM_A_VAL_TYPE)ls_a_user_crypt_filter);
    if (ls_a_phase2_app)
      lc_set_attr(lm_job, LM_A_PHASE2_APP,
      (LM_A_VAL_TYPE)ls_a_phase2_app);
    lc_set_attr(lm_job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE)0);
    lc_set_attr(lm_job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE)0);
    /*
    *    P4069 -- have to set in init1 if they need
    *    lc_set_attr(lm_job, LM_A_SUPPORT_HP_IDMODULE, (LM_A_VAL_TYPE)1);
    */
    lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)0);
    if (ls_a_behavior_ver && L_STREQ(lm_job->options->behavior_ver,
      LM_BEHAVIOR_CURRENT))
      lc_set_attr(lm_job, LM_A_BEHAVIOR_VER,
      (LM_A_VAL_TYPE)ls_a_behavior_ver);
    if (ls_a_lkey_start_date)
      lc_set_attr(lm_job, LM_A_LKEY_START_DATE,
      (LM_A_VAL_TYPE)ls_a_lkey_start_date);
    if (ls_a_lkey_long)
      lc_set_attr(lm_job, LM_A_LKEY_LONG,
      (LM_A_VAL_TYPE)ls_a_lkey_long);
    if (ls_a_check_baddate)
      lc_set_attr(lm_job, LM_A_CHECK_BADDATE,
      (LM_A_VAL_TYPE)ls_a_check_baddate);
    if (ls_a_license_case_sensitive)
      lc_set_attr(lm_job, LM_A_LICENSE_CASE_SENSITIVE,
      (LM_A_VAL_TYPE)ls_a_license_case_sensitive);

#ifdef LM_GENERIC_VD
    if (!strcmp(lm_job->vendor, "flexlmd"))
    {
      ls_flexlmd = 1;
    }
#endif
    if (lm_job) lm_job->first_job = lm_job; /* Fix up */

    lm_job->attrs[CHECKOUT] = CHECKOUT_VAL;
  }
  ls_s_init();      /* Initialize the server database */
#ifdef VMS
  ls_vms_call(&argc, &argv);
#endif
  l_text();     /* Bind text domain */

#ifdef WINNT
  {
    char buf[100];
    sprintf(buf, "%s:  FLEXlm vendor daemon", lm_job->vendor);
    SetConsoleTitle(buf);
  }

#endif
  ls_log_open_ascii("stdout"); /* Get the log file started off right */ /* FILENAME - HARDCODED */
#ifdef LM_GENERIC_VD
  issue_outofdate_warning();
  if (l_getattr(lm_job, END_USER_OPTIONS) != END_USER_OPTIONS_VAL)
    ls_upgrade_vkeys(argc, argv, vendor_name);
#endif /* LM_GENERIC_VD */
  if (LM_TOOL_ERR_CATCH(lm_job))
  {
    LOG(("Out of memory, exiting\n"));
    ls_go_down(EXIT_MALLOC);
  }
  {
    short v, r;
    char *p;
    lc_get_attr(lm_job, LM_A_VERSION, (short *) &v);
    lc_get_attr(lm_job, LM_A_REVISION, (short *) &r);
    lc_get_attr(lm_job, LM_A_RELEASE_PATCH, (short *) &p);
    LOG(("FLEXlm version %d.%d%s\n", v, r, p));
  }
  if (i && (i != LM_DEFAULT_SEEDS))
  {
    if (i == EXPIREDKEYS)
    {
		LOG(("\n"));
		LOG((
		lmtext("This DEMO version of FLEXlm has expired.  Please contact\n")));
		LOG((lmtext("your sales person at Macrovision.\n")));
		LOG(("\n"));
    }
    else
    {
		LOG((lmtext("Daemon initialization error: %s\n"),
		lmtext(lc_errstring(lm_job))));



		if(l_flexEventLogIsEnabled())
		{
			l_flexEventLogWrite(lm_job, FLEXEVENT_ERROR, CAT_FLEXLM_LMGRD,
				MSG_FLEXLM_DAEMON_INIT_ERROR, 0, NULL, 0, NULL);
		}
    }
    if (i == BADKEYDATA)
    {
      LOG((lmtext("Keys: 0x%x 0x%x 0x%x 0x%x 0x%x\n"),
        vendorkeys[0].keys[0],
        vendorkeys[0].keys[1],
        vendorkeys[0].keys[2],
        vendorkeys[0].keys[3],
        vendorkeys[0].keys[4]));
      LOG((lmtext("Please contact Macrovision Corporation at info@macrovision.com\n")));
      LOG((lmtext("in order to get updated vendor keys.\n")));
    }
    ls_go_down(EXIT_EXPIRED);
  }
  if (argc < 4)
  {
    if ((argc > 1) && (!strcmp(argv[1], "-v") ||
      !strcmp(argv[1], "-V")))
    {
      (void) printf("%s v%d.%d%s - "COPYRIGHT_STRING(1988) "\n",
        lm_job->vendor,
        lm_job->code.flexlm_version,
        lm_job->code.flexlm_revision,
        lm_job->code.flexlm_patch
        );
      exit(0);
    }
    else
    {
      LOG((lmtext("Vendor daemons must be run by lmgrd\n")));
      exit(EXIT_BADCALL);
    }
  }
  if ((l_getattr(lm_job, LM_RUN_DAEMON) != LM_RUN_DAEMON_VAL)
    && strcmp(vendor_name, "flexlmd"))
  {
    LOG((lmtext(
      "The vendor keys don't support vendor daemon, exiting\n")));
    LOG((lmtext("Vendor keys are:\n0x%x\n0x%x\n0x%x\n0x%x\n"),
      vendorkeys[0].keys[0],
      vendorkeys[0].keys[1],
      vendorkeys[0].keys[2],
      vendorkeys[0].keys[3]));
    exit(EXIT_NO_SERVERS);
  }


  if (l_getattr(lm_job, FULL_FLEXLM) != FULL_FLEXLM_VAL)
  {
    /*
    *    FLEXlm LITE
    */
    ls_user_crypt = 0;
    ls_user_init1 = 0;
    ls_user_init2 = 0;
    ls_crypt_case_sensitive = 0;
    ls_user_lockfile = (char *)NULL;
    ls_normal_hostid = 1;
    ls_conn_timeout = MASTER_WAIT;
    ls_enforce_startdate = 0;
    ls_tell_startdate = 0;
    ls_use_featset = 0;
    ls_use_all_feature_lines = 0;
    ls_dup_sel = LM_NO_COUNT_DUP_STRING;
    ls_outfilter = 0;
    ls_infilter = 0;
    ls_incallback = 0;
    ls_vendor_msg = 0;
    ls_vendor_challenge = 0;
    ls_min_lmremove = 120;
    ls_use_featset = 0;
    ls_show_vendor_def = 1;
    ls_allow_borrow = 0;
    ls_hostid_redirect_verify = 0;
    ls_daemon_periodic = 0;
#ifdef VMS
    default_license_file = (char *) NULL;
#endif
  }

  lm_set_attr(LM_A_NORMAL_HOSTID, (LM_A_VAL_TYPE)ls_normal_hostid);
  lm_set_attr(LM_A_USER_CRYPT, (LM_A_VAL_TYPE) ls_user_crypt);
  lm_set_attr(LM_A_CRYPT_CASE_SENSITIVE, (LM_A_VAL_TYPE)ls_crypt_case_sensitive);
#ifdef DEFAULT_LIC_FILE_SPEC
  if (default_license_file != (char *) NULL)
  {
    lm_set_attr(LM_A_LICENSE_FILE,
      (LM_A_VAL_TYPE) default_license_file);
  }
#endif
  if (ls_hostid_redirect_verify)
  {
    (void) lm_set_attr(LM_A_REDIRECT_VERIFY,
      (LM_A_VAL_TYPE)ls_hostid_redirect_verify);
  }
  /*
  *  If the platform allows it, get as many file descriptors for
  *  the vendor daemons as possible, to avoid having to start
  *  another copy as long as possible
  */

#ifdef UNLIMIT_DESCRIPTORS
  {
#include <sys/resource.h>

    struct rlimit limit;

    errno = 0;
    getrlimit(RLIMIT_NOFILE, &limit);
    if (errno == 0)
    {
      limit.rlim_cur = limit.rlim_max;
      if (limit.rlim_cur > 4000)
        limit.rlim_cur = limit.rlim_max = 65000;

      setrlimit(RLIMIT_NOFILE, &limit);
#ifdef FD_SET_LIMITED
      lm_nofile = sizeof(fd_set) * 8;
#else
      lm_nofile = limit.rlim_cur;
#endif
      lm_bpi = sizeof(int) * 8;
      lm_max_masks = lm_nofile / lm_bpi;
      if ((lm_max_masks * lm_bpi) < lm_nofile)
        lm_max_masks++;
    }
  }
#endif
  l_flush_config(lm_job);
  argv++; argc--;   /* "-T" */
#if defined(UNIX) && !defined( TRUE_BSD)
  (void) setsid(); /* We are a new P.G. */
#endif /* UNIX */
  *master_name = argv[1];
  argv++; argc--;   /* master_name */
  {

    sscanf(argv[1], "%d.%d", &ver, &rev); /* overrun checked */
    if (ver >= 0 && rev>=0)
    {
      argv++; argc--;   /* version */
      v2args = 1;
    }
  }
#ifdef VMS
  ls_log_comment(LL_LOGTO_BOTH, lmtext("%s version %d.%d%s"),
    lm_job->vendor, FLEXLM_VERSION, FLEXLM_REVISION, FLEXLM_PATCH);
#else
  if (v2args)
  {
    if ((ver != FLEXLM_VERSION) || (rev != FLEXLM_REVISION))
    {
      ls_log_comment(LL_LOGTO_BOTH,
        lmtext("lmgrd version %d.%d, %s version %d.%d\n"),
        ver,rev, lm_job->vendor,
        FLEXLM_VERSION, FLEXLM_REVISION);
      LOG_INFO((INFORM, "The vendor daemon is running a \
                        different version of FLEXlm from \
                        lmgrd."));
    }
  }
#endif /* VMS */
  if (argc >= 2)
  {
    port = atoi(argv[1]);
    argv++; argc--;   /* port */
  }
  else
  {
    LOG((lmtext("No internet port specified\n")));
    LOG_INFO((CONFIG, "An application daemon was started \
                      without an internet port number.  This \
                      is an internal consistency error."));
    ls_go_down(EXIT_BADCONFIG);
  }
  {
#if defined(ACCEPT_BUG) && defined(USE_TLI)
    s = (LM_SOCKET) 1;
#else
#ifdef ALLOW_STARTUP_FROM_COMMAND_LINE
    if (port > MAX_FD_FOR_PORT)
      s = (LM_SOCKET) 1;/* IF we have a port #,
                        just dummy this descriptor */
    else
#endif /* ALLOW_STARTUP */
      s = (LM_SOCKET) port;
#endif /* ACCEPT_BUG */
  }
  /*
  *   Get rid of everything but stdout and stderr (and the
  *     port, if so specified).
  */
  if (ls_fflush_ok)
    setlinebuf(stdout);
  if (ls_fflush_ok)
    setlinebuf(stderr);
  /*
  *   If we were passed the config file or log file location - use it.
  */
  while (argc > 1)
  {
    if (L_STREQ(argv[1], "-nfs_log"))
      ls_fflush_ok = 0;
    else if (L_STREQ(argv[1], "-lmgrd_port"))
    {
      argv++; argc--;
      sscanf(argv[1], "%x", &ls_lmgrd_tcp_port);  /* overrun checked */
    }
    else if (L_STREQ(argv[1], "-hang"))
    {
      ls_test_hang = time(0) + 15;
    }
    else if (L_STREQ(argv[1], "-max_connections"))
    {
      extern long max_connections;
      argv++; argc--;
      sscanf(argv[1], "%ld", &max_connections); /* overrun checked */
      LOG(("Max client connections to set %d\n",
        max_connections));
    }
#ifdef WINNT
    else if (L_STREQ(argv[1], "-daemon_port"))
    {
      argv++; argc--;
      sscanf(argv[1], "%x", &port); /* overrun checked */
    }


#endif

    else if (L_STREQ(argv[1], "-path"))
    {
      argv++; argc--;
      if (argc > 1)
        LOG(("%s: %s\n",
        lmtext("Path used"), argv[1]));
    }
    else if (L_STREQ(argv[1], "--lmgrd_start"))
    {
      argv++; argc--;
      sscanf(argv[1], "%lx", &ls_lmgrd_start); /* P6465 */  /* overrun checked */
    }
    else if (*argv[1] == '-')
    {
      switch (argv[1][1])
      {
      case 'c':
        if ( argv[2] )
        {
          lm_job->options->disable_env  = 1;
          l_set_license_path(lm_job, argv[2],
            LM_A_LICENSE_FILE);
          argv++; argc--;
        }
        break;
      case 'l':
#if defined(VMS) || defined (PC )
        if ( argv[2] )
          ls_log_open_ascii(argv[2]); /* FILENAME - SYSTEM VALUE */
#endif
#if /*!defined(PC) && */ !defined(VMS) && !defined(PC)

				if (l_flexFreopen(lm_job, argv[2], "a+", stdout) ==
						(FILE *) NULL)
				{
					(void) fprintf(stderr,
				      "Cannot open \"%s\" as log file\n"
							, argv[2]);
					perror("freopen");
				}
#endif
				argv++; argc--;
				break;
			case 'x':
			case 'X':
				if ((argc > 2) &&
					(!strcmp("lmremove", argv[2])))
				{
					LOG((lmtext(
					"lmremove disabled\n")));
					ls_allow_lmremove = 0;
				}
				else
				{
					LOG((lmtext(
				"-x  has invalid arg, ignored\n")));
					if (argc>2)
					{
						argv++; argc--;
					}
				}
				break;

			case 'p':		/* privilged */
				unpriv_utils = 0;
				break;
			case 'Z':		/* UNUSED - filler */
			default:
				break;
		    }
		}
		argv++; argc--;
	}
/*
 *		Set up the locking for this daemon, so only one copy can run.
 */
#ifndef VMS
  if (ls_do_checkroot && ls_checkroot())
  {
    LOG((lmtext("Cannot open daemon lock file\n")));

    DLOG(("%s\n", ls_user_lockfile));

    ls_go_down(EXIT_SERVERRUNNING);
  }
#endif
#ifndef RELEASE_VERSION
  /*for debugging redundant servers*/
  {
    if (l_real_getenv("DEBUG_LOCKFILE"))
      ls_user_lockfile = l_real_getenv("DEBUG_LOCKFILE");
  }
#endif
  checklock();
  /*
  *    Check the FEATURESET line, if so requested
  */
  if (ls_ck_feats() == 0)
  {
    LOG(("Exiting\n"));
    ls_go_down(EXIT_FEATSET_ERROR);
  }


  if (ls_user_init1 && (l_getattr(lm_job, DAEMON_INIT) == DAEMON_INIT_VAL))
    (void) (* LS_CB_USER_INIT1_TYPE ls_user_init1)();
  /*
  *  Create the descriptors to talk to our eventual children -
  *  After we exec our child, we will close child_desc[2] and
  *  child_desc[3]
  */
  *list = master_list = l_master_list_from_job(lm_job);

  if (!master_list)
  {

    if (_lm_errno == NOCONFFILE)
    {
      LOG((lmtext("Cannot open license file %s\n"),
        lm_lic_where()));
      LOG_INFO((INFORM, "The vendor daemon cannot find its \
                        license file."));
    }
    else
    {
      char errmsg[200];
      (void) sprintf(errmsg, "%s: can't initialize",
        lm_job->vendor);
      lm_perror(errmsg);
    }
    ls_go_down(EXIT_NO_LICENSE_FILE);
  }
  if (master_list->sflags & L_SFLAG_THIS_HOST)
  {
    char hostname[MAX_HOSTNAME + 1];

    gethostname(hostname, MAX_HOSTNAME);
    strcpy(master_list->name, hostname);
    if (lm_job->line->server)
      strcpy(lm_job->line->server->name, hostname);
  }

#ifdef FLEX_BETA
  LOG(("For Beta Test Use Only\n"));
#endif

  if ((l_getattr(lm_job, NO_EXPIRE) != NO_EXPIRE_VAL))
  {
    LOG(("\n"));
    LOG(("\n"));
    LOG((lmtext("\t\tFLEXIBLE LICENSE MANAGER DEMO VERSION\n")));
    LOG(("\n"));
    if (l_date(lm_job, l_getexp(), L_DATE_EXPIRED))
    {
      LOG(("\n"));
      LOG((
        lmtext("This DEMO version of FLEXlm has expired.  Please contact\n")));
      LOG((
        lmtext("your sales person at Macrovision.\n")));
      LOG(("\n"));
      ls_go_down(EXIT_EXPIRED);
    }
    {
      extern char *copyright1;
      int k, j;
      char buf[80];
      for (k = 0, j = 0; copyright1[k]; k++)
      {
        buf[j++] = copyright1[k];
        if (j >= 50 && buf[j-1] == ' ')
        {
          buf[j++] = '\n';
          buf[j] = '\0';
          j = 0;
          LOG((buf));
        }
      }
      buf[j++] = '\n';
      buf[j++] = '\n';
      buf[j] = '\0';
      LOG((buf));
    }
  }
#ifndef NLM
  if (l_getattr(lm_job, DUP_SERVER) != DUP_SERVER_VAL)
#else
  if ( TRUE )  /*  Never allow Redundant servers on Novell   P4248 */
#endif

  {
    if (master_list->next)
    {
      LOG((lmtext("This FLEXlm version supports only one SERVER host!\n")));
      LOG_INFO((INFORM, "An attempt was made to configure redundant \
                        servers in a version of the software that does not support \
                        this feature."));
      ls_go_down(EXIT_BADCONFIG);
    }
  }
#ifdef VMS
  if (master_list->next)
  {
    LOG((lmtext("VMS FLEXlm does not support redundant SERVER hosts!\n")));
    LOG_INFO((INFORM, "An attempt was made to configure redundant \
                      servers in a version of the software that does not support \
                      this feature."));
    ls_go_down(EXIT_BADCONFIG);
  }
#endif
#ifdef NO_LMGRD
  {
    char arch[MAX_HOSTTYPE_NAME+5];
    HOSTTYPE *x;
#ifdef THREAD_SAFE_TIME
    struct tm *ls_gettime(struct tm * ptst), *t;
    struct tm tst;
#else /* !THREAD_SAFE_TIME */
    struct tm *ls_gettime(), *t;
#endif

    x = lm_hosttype(0);
    if (x && x->name)
    {
      (void) sprintf(arch, " (%s)", x->name);
    }
    else
      arch[0] = '\0';

#ifdef THREAD_SAFE_TIME
    ls_gettime(&tst);
    t = &tst;
#else /* !THREAD_SAFE_TIME */
    t = ls_gettime();
#endif
    {
      int v, r;
      char *p;

      lc_get_attr(lm_job, LM_A_VERSION, (short *) &v);
      lc_get_attr(lm_job, LM_A_REVISION, (short *) &r);
      LOG((lmtext(
        "FLEXlm (v%d.%d%s) started on %s%s (%d/%d/%d)\n"),
        lm_job->code.flexlm_version,
        lm_job->code.flexlm_revision,
        lm_job->code.flexlm_patch,
		ls_our_hostname,
        arch, t->tm_mon+1, t->tm_mday,
        t->tm_year + 1900));

		if(pszOurHostname)
		{
			l_free(pszOurHostname);
			pszOurHostname = NULL;
		}
    }

    LOG(("FLEXlm "COPYRIGHT_STRING(1988)"\n"));
    LOG((lmtext("World Wide Web:  http://www.macrovision.com\n")));
    LOG_INFO((INFORM, "The license server logs its start time \
                      along with the node name whenever it starts up."));

    LOG((lmtext("License file(s):")));
    {
      int i;

      for (i = 0; lm_job->lic_files[i]; i++)
        _LOG((" %s", lm_job->lic_files[i]));
    }
    _LOG(("\n"));

    LOG_INFO((INFORM, "The license server logs the current \
                      active license file."));
  }



#endif
  prog = lm_daemon(lm_job->vendor, ls_options, &i);
  if (!*ls_options)
    find_default_options_file(ls_options);
  if (*ls_options)
  {
		LOG((lmtext("Using options file: \"%s\"\n"), ls_options));
		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = ls_options;

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_VENDOR_OPTIONS_FILE,
								1,
								ppszInStr,
								0,
								NULL);

		}
  }
  master_list = ls_checkhost(master_list);
  /*
  *   Build our feature table
  */
  ls_init_feat(*list);
  ls_host();
  /*
  *   Find out if we have an options file specified, and if so,
  *   read it and fill in the info in "ls_flist".
  */
  /*  P6394 */
  if (ls_flist && ls_flist->next)
    ls_get_opts(ls_options, *master_name, 0, 0);
  if (!(ls_options[0] && ls_flist && ls_flist->next))
    ls_user_based(ls_options, vendor_name);

  {
#ifdef VD_MAKES_TCP_SOCKET
    int dynamic = 1;
    COMM_ENDPOINT endpoint;

    char *str = l_asc_hostid(lm_job, master_list->idptr);

    /*
    *    Now, save the hostid for later and fire off the timer
    *    as of v5.0 can't use memcpy for hostids
    */

    l_get_id(lm_job, &vendor_daemon_hostid, str);


    /*
    *  On some platforms, the vendor daemon cannot inherit the TCP/IP
    *  endpoint as on BSD-type systems.  On these systems, we create
    *  the TCP socket here and pass it back to lmgrd
    */
#ifdef NLM
    /*
    *  NLM should pick up the TCP port number from
    *  the license file where "TCP:xxxx" is specified.
    */
    if ( master_list->port && master_list->port != -1 )
    {
      port = master_list->port;
#endif /* NLM */

      if (port > 0) dynamic = 0;
      endpoint.transport = LM_TCP;
      endpoint.transport_addr.port =
        (int) ntohs((unsigned short)port);

      s = ls_socket(ls_our_hostname, &endpoint, dynamic);
#ifndef NO_LMGRD
      if ( dynamic )
        ls_send_port(master_list, ls_our_hostname,
        (int)ntohs((unsigned short)
        endpoint.transport_addr.port),
        LM_COMM_TCP);
#endif /* NO_LMGRD */
#ifdef NLM
    }
    else
    {
      s = LM_BAD_SOCKET;
    }
#endif /* NLM */

#ifdef SUPPORT_IPX
    if ( master_list->spx_addr.sa_family == AF_NS )
    {
      dynamic = 1;
      if (port > 0) dynamic = 0;
      endpoint.transport = LM_SPX;
      endpoint.transport_addr.spx_addr=master_list->spx_addr;

#ifndef NLM
      endpoint.transport_addr.spx_addr.sa_socket =
        ntohs((unsigned short)port);
#endif /* NLM */

      *spx_s = ls_socket(ls_our_hostname, &endpoint, dynamic);
#ifndef NO_LMGRD
      if ( dynamic )
        ls_send_port(master_list, ls_our_hostname,
        (int)ntohs((unsigned short)endpoint.transport_addr.spx_addr.sa_socket),
        LM_COMM_SPX);
#endif /* NO_LMGRD */
    }
#endif /* SUPPORT_IPX */

#else /* VD_MAKES_TCP_SOCKET */
#if !defined(VMS) && !defined(USE_TLI)
    if (port <= MAX_FD_FOR_PORT)
      s = (LM_SOCKET) port;
    else        /* DEBUGGING */
#endif /* VMS */
    {
#ifdef ALLOW_STARTUP_FROM_COMMAND_LINE
      COMM_ENDPOINT endpoint;

      endpoint.transport = LM_TCP;
      endpoint.transport_addr.port=(int)ntohs((unsigned short)port);

      s = ls_socket(ls_our_hostname, &endpoint, 0);
#endif /* ALLOW_STARTUP */
    }

#ifdef ACCEPT_BUG
    if (listen(s, LISTEN_BACKLOG) < 0)
    {
      char errmsg[200];

      (void) sprintf(errmsg, "%s: listen",
        lm_job->vendor);
      perror(errmsg);
      ls_go_down(EXIT_COMM);
    }
#endif
#endif /* VD_MAKES_TCP_SOCKET */

    (void) ls_store_master_list(*list);
#ifndef SIGNAL_NOT_AVAILABLE
    (void) signal(REREAD_SIGNAL, ls_reread_sig);
#endif
    /* re-read license file */
  }
#if !defined(SIGNAL_NOT_AVAILABLE)

  (void) signal(SHUTDOWN_SIGNAL, ls_shutdown);/* Exit signal from above */
#endif /* SIGNAL_NOT_AVAILABLE */

  /*
  *  Create UDP/TCP sockets and communicate that back to lmgrd
  */

#ifdef SUPPORT_UDP

#ifndef NLM
  if ((ls_udp_s = ls_setup_udp_socket(master_list, port)) ==
    LM_BAD_SOCKET)
  {
#ifdef PC
    if (port < 0)
#endif
    {

      LOG((lmtext("Cannot create UDP for server communication\n")));
      ls_go_down(EXIT_BADCALL);
    }
  }
  else
#endif  /* NLM */
    ls_udp_s = LM_BAD_SOCKET;

#endif  /* SUPPORT_UDP */


  if (ls_user_init2 && (l_getattr(lm_job, DAEMON_INIT) == DAEMON_INIT_VAL))
    (void) (* LS_CB_USER_INIT2_TYPE ls_user_init2)();
  if ((l_getattr(lm_job, ADDITIVE) != ADDITIVE_VAL))
  {
    ls_use_all_feature_lines = 0;
  }
  if ((l_getattr(lm_job, FILTERS) != FILTERS_VAL))
  {
    ls_outfilter = 0;
    ls_infilter = 0;
    ls_incallback = 0;
  }

  *tcp_s = s; /*return value*/
  (void) ls_store_socket(*tcp_s, ls_udp_s, *spx_s);

}

extern char ls_vname0, ls_vname1, ls_vname2, ls_vname3, ls_vname4, ls_vname5,
ls_vname6, ls_vname7, ls_vname8, ls_vname9;


/*
*  Check that we are on the right hostname/hostid
*/

LM_SERVER *
ls_checkhost(master_list)
LM_SERVER *master_list;
{
  char hostname[MAX_HOSTNAME + 1];
  /*
  *  Make sure we are a valid host
  */
  while (master_list && !ls_on_host(master_list->name))
  {

    master_list = master_list->next;
  }
  if (master_list == NULL)
  {
    /*
    *    We are not one of the hosts in the master list
    */
    (void) gethostname(hostname, MAX_HOSTNAME);
    LOG((lmtext("%s: Not a valid server host."),
      hostname));
    LOG_INFO((CONFIG,
      "The daemon was started on a node that is \
      not one of the server hosts specified in the \
      configuration file.  This condition should \
      normally be caught by the license daemon."));
    if (!(getenv("FLEXLM_ANYHOSTNAME")))
    {
      _LOG((lmtext(" Exiting.\n")));
      ls_go_down(EXIT_BADCONFIG);
    }
    _LOG(("FLEXLM_ANYHOSTNAME set, not exiting\n"));
  }
  (void) strncpy(ls_our_hostname, master_list ? master_list->name : "localhost", MAX_HOSTNAME);
  ls_our_hostname[MAX_HOSTNAME] = '\0';
  /*
  *  pre-v6 we used to check hostid here.  Now this is done in ls_host()
  */
  return(master_list);
}

#ifdef SUPPORT_UDP
/*
*  Create udp socket, and send message to lmgrd with port address
*  for udp socket
*/

static LM_SOCKET
ls_setup_udp_socket(master_list, port)
LM_SERVER *master_list; /*used to get lmgrd port*/
int port; /*If we're debugging from vd directory,
          * then we bind to this port address also */
{
  int udp_sock;
  char *hostname = lm_hostname(1);
#ifdef ALLOW_STARTUP_FROM_COMMAND_LINE
#ifdef PC
  int talk_to_lmgrd = port < 0;
#else
  int talk_to_lmgrd = port < MAX_FD_FOR_PORT;
#endif /* PC */
#else /* !ALLOW_STARTUP */
  int talk_to_lmgrd = 1;
#endif /* !ALLOW_STARTUP */
  COMM_ENDPOINT endpoint;

  endpoint.transport = LM_UDP;

  /*
  *  Assume success for ls_socket -- it will exit otherwise
  */
  if (!talk_to_lmgrd)
  {
    endpoint.transport_addr.port = (int)ntohs((unsigned short)port);
    udp_sock = ls_socket(hostname, &endpoint, 0);

  }
  else
  {
    endpoint.transport_addr.port = 0;
    udp_sock = ls_socket(hostname, &endpoint, 1);
    ls_send_port(master_list, hostname,
      ntohs((unsigned short)endpoint.transport_addr.port),
      LM_COMM_UDP);
  }

  return udp_sock;
}

static
void
ls_send_port(master_list, hostname, port, commtype)
LM_SERVER *master_list; /*used to get lmgrd port*/
char *hostname;
int port;
int commtype;
{
  char msg[LM_MSG_LEN+1], resp[LM_MSG_LEN+1];
  LM_SOCKET lmgrd_s = LM_BAD_SOCKET;    /* lmgrd tcp socket */
  COMM_ENDPOINT endpoint;

  ls_get_lmgrd_endpoint(master_list, hostname, commtype, &endpoint);
  ls_create_port_msg(msg, port, hostname, commtype); /*sets up msg*/
  l_msg_cksum(msg, COMM_NUMREV, LM_TCP);

  /*
  *  Send VHELLO to lmgrd -- giving the UDP port address
  */
  if (ls_lmgrd_tcp_port != -1)
    endpoint.transport_addr.port =
    ntohs((unsigned short)ls_lmgrd_tcp_port);

  lm_job->flags |=  LM_FLAG_CONNECT_NO_THREAD;
  lmgrd_s = l_basic_conn(lm_job, msg, &endpoint, hostname, resp);
  lm_job->flags &=  ~LM_FLAG_CONNECT_NO_THREAD;

  if (lmgrd_s == LM_BAD_SOCKET)
  {
    if(getenv("FLEXLM_ANYHOSTNAME"))
    {
      lm_job->flags |=  LM_FLAG_CONNECT_NO_THREAD;
      lmgrd_s = l_basic_conn(lm_job, msg, &endpoint, "localhost", resp);
      lm_job->flags &=  ~LM_FLAG_CONNECT_NO_THREAD;
    }
    if(lmgrd_s == LM_BAD_SOCKET)
    {
      LOG(("Vendor daemon can't talk to lmgrd (%s)\n",
        lmtext(lc_errstring(lm_job))));
      ls_go_down(EXIT_COMM);
    }
  }
  ls_down((LM_SOCKET *)&lmgrd_s, "lmgrd socket");
}

/*
*    Create the VHELLO msg
*/
static void
ls_create_port_msg(msg, port, hostname, commtype) /*Create the VHELLO msg*/
char *msg;
int port;
char *hostname;
int commtype;
{

  (void)memset(msg, 0, LM_MSG_LEN + 1);
  msg[MSG_CMD] = LM_VHELLO;
  msg[MSG_VHEL_VER] = '0' + COMM_NUMVER;
  msg[MSG_HEL_VER+1] = '0' + COMM_NUMREV;
  (void) strncpy(&msg[MSG_VHEL_HOST], hostname, MAX_SERVER_NAME);/* LONGNAMES */
  msg[MSG_VHEL_HOST + MAX_SERVER_NAME] = '\0';/* LONGNAMES */
  (void) strncpy(&msg[MSG_VHEL_DAEMON], lm_job->vendor, MAX_DAEMON_NAME);
  msg[MSG_VHEL_DAEMON + MAX_DAEMON_NAME] = '\0';
  msg[MSG_VHEL_TRANSPORT] = commtype;
  (void) strncpy(&msg[MSG_VHEL_ADDR_HOST], hostname, MAX_SERVER_NAME);/* LONGNAMES */
  msg[MSG_VHEL_ADDR_HOST + MAX_SERVER_NAME] = '\0';/* LONGNAMES */
  (void) sprintf(&msg[MSG_VHEL_ADDR_PORT], "%d", port);
  msg[MSG_VHEL_ADDR_PORT + MAX_LONG_LEN] = '\0';
  if (lm_job->options->transport_reset == LM_RESET_BY_USER)
    msg[MSG_VHEL_TRANSPORT_REASON] = LM_COMM_REASON_USER;
  else
    msg[MSG_VHEL_TRANSPORT_REASON] = ' ';
  msg[MSG_VHEL_REC_TRANSPORT] = (lm_job->options->commtype == LM_UDP ?
LM_COMM_UDP : LM_COMM_TCP);
  l_encode_int(&msg[MSG_VHEL_FLEX_VER], lm_job->code.flexlm_version);
  l_encode_int(&msg[MSG_VHEL_FLEX_REV], lm_job->code.flexlm_revision);
}

static unsigned short
ls_get_lmgrd_endpoint(master_list, hostname, commtype, endpoint)
LM_SERVER *master_list;
char *hostname;
int commtype; /* LM_COMM_TCP, LM_COMM_SPX, and etc */
COMM_ENDPOINT *endpoint;
{
  LM_SERVER *s;
  s = master_list;
  while (s && !ls_on_host(s->name))
    s = s->next;
  if (!s) {
    LOG((lmtext("Wrong hostname -- this host is not in the license file")));
    ls_go_down(EXIT_WRONGHOST);
  }
#ifdef SUPPORT_IPX
  if (commtype == LM_COMM_SPX)
    commtype = LM_SPX;
  else
#endif /* SUPPORT_IPX */
    commtype = LM_TCP;

  return (l_get_endpoint(lm_job, s, "", commtype, endpoint));
}

#endif  /* SUPPORT_UDP */

#ifndef NO_LMGRD
/*
*  A hack to make vendor daemon link -- this function is linked from
*  ls_signals.c in server directory, but isn't used in vendor
*  daemon.
*/
void
ls_statfile_rm()
{
}
#endif /* NO_LMGRD */

void
static
set_signals()
{
  int i;
  /*
  *  Set up the defaults for our signals (some will be overridden later)
  *  Start by ignoring everything
  */
#ifdef MOTO_88K
  for (i=1; i <= NSIG; i++)
  {
    switch (i)
    {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGILL:
    case SIGTRAP:
    case SIGABRT:
    case SIGFPE:
    case SIGKILL:
    case SIGBUS:
    case SIGSEGV:
    case SIGSYS:
    case SIGPIPE:
    case SIGALRM:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
    case SIGCLD:
    case SIGWINCH:
    case SIGSTOP:
    case SIGTSTP:
    case SIGCONT:
    case SIGTTIN:
    case SIGTTOU:
    case SIGURG:
    case SIGIO:
    case SIGVTALRM:
    case SIGPROF:
      (void) signal(i, SIG_IGN);
      break;
    default:
      break;
    }
  }
#endif
#if !defined(SIGNAL_NOT_AVAILABLE)
  for (i=1; i < NSIG; i++)
    (void) signal(i, SIG_IGN);


  for (i=0; i < sizeof(kill_signals) / sizeof(kill_signals[0]); i++)
  {
    (void) signal(kill_signals[i], ls_shutdown);
  }
  for (i=0; i < sizeof(debug_signals) / sizeof(debug_signals[0]); i++)
    (void) signal(debug_signals[i], SIG_DFL);
#if !defined( RELEASE_VERSION)  && !defined(sco)
  signal(SIGURG, _hang);
#endif /* RELEASE_VERSION */
#endif /* SIGNAL_NOT_AVAILABLE */
}

/*
*  Check the lock file.
*/
static
void
checklock()
{
  int i;

  i = ls_setlock();
  if (i < 0)
  {
    ls_go_down(EXIT_SERVERRUNNING);
  }
  else if (i > 0)
  {
#ifdef VMS
    if (i == SS$_NOSYSLCK)
    {
      LOG((lmtext("Cannot set lock.  Need SYSLCK priv.")));
      LOG_INFO((INFORM,
        "The vendor daemon could not establish a lock.  It needs SYSLCK priv."));
    }
    else
    {
      LOG((lmtext("Cannot set process name lock (%s) (%d)\n"),
        lockname, i));
      LOG_INFO((INFORM, "The vendor daemon could not set the \              process name."));
    }
    ls_go_down(EXIT_SERVERRUNNING);
#else
    LOG_INFO((INFORM, "The vendor daemon cannot flock its lock \
                      file.  This is usually due to another copy of \
                      the daemon running on the node.  Locate the \
                      other daemon with \"ps ax\" and kill \
                      it with \"kill -9\"."));
    ls_go_down(EXIT_SERVERRUNNING);
#endif
  }
}

void
find_default_options_file(options)
char *options;
{
  CONFIG *c;
  char last_dir[LM_MAXPATHLEN + 1];
  char this_dir[LM_MAXPATHLEN + 1];
  char default_name[MAX_VENDOR_NAME + 5];
  char tmp [LM_MAXPATHLEN + 1];


  *options = *last_dir = 0;
  sprintf(default_name, "%s.opt", lm_job->vendor);
  for (c = lm_job->line; c; c = c->next)
  {
    *this_dir = 0;
    file_to_dirname(lm_job->lic_files[c->lf], this_dir);
    if (strcmp(this_dir, last_dir))
    {
      strcpy(last_dir, this_dir);
      l_files_in_dir(lm_job, this_dir, tmp, default_name);
      if (*tmp)
      {
        strcpy(options, tmp);
        return;
      }
    }
  }
}
static
void
file_to_dirname(file, buf)
char *file;
char *buf;
{
  int len = strlen(file);

  for (len--; len >= 0; len--)
  {
    if (file[len] == PATHTERMINATOR)
      break;
  }
  if (len > 0)
    l_zcp(buf, file, len);
  else
    strcpy(buf, ".");
}
