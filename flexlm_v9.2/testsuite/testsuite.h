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
 *	Module: $Id: testsuite.h,v 1.19 2003/01/13 22:55:13 kmaclean Exp $
 *
 *	Description: Definitions for testsuite routines.
 *
 *	M. Christiano
 *	8/21/89
 *
 *	Last changed:  11/13/98
 *
 */

#include "lmachdep.h"
#ifdef getenv
#undef getenv
#endif

#ifdef PC
#define config "C:\\flexlm\\license.dat"
#else
#ifdef VMS
#define config "sys$common:[sysmgr]license.dat"
#else
#define config "/usr/local/flexlm/licenses/license.dat"
#endif /* VMS */
#endif /* PC */

#ifdef VMS
#define backup "sys$common:[sysmgr]license.datBAK"
#define DEMOD "demo"
#else
#ifdef PC 
#define backup "c:\\flexlm\\license.bak"
#else	
#define backup "/usr/local/flexlm/licenses/license.datBAK"
#endif /* PC */
#define DEMOD "/demo"
#define DAEMON "/lmgrd"
#endif

#define NUM_JOBS 20
#define tempconfig "tmp.dat"
#define CF2 "lic2.dat"
#define altfile "altlic.dat"


#define testdir "" 
#define good_daemon "demo"
/*#define CF "license.dat"*/
#define CF "."
#define FEATURE "f1"
#ifndef lm_args
#if defined(__STDC__) || defined (__ANSI_C__) 
#define  lm_noargs  void
#define  lm_args(args) args
#else	/* ! stdc || ansi */
#define  lm_noargs  /* void */
#define  lm_args(args) ()
#endif	/* stdc || ansi */
#endif

void ts_lm_init lm_args(( char *, VENDORCODE *, LM_HANDLE **, int));
void ts_lc_new_job lm_args(( VENDORCODE *, LM_HANDLE **, int));
lm_extern ts_lic_file lm_args((char *, char *, int, char *, char *, char *, 
	char *, char *, char *, int, char *, char *, char *, char *, char **));
lm_extern good_client lm_args((char *, char *, int, int, char *,char *));
lm_extern ts_lic_append  lm_args((char *file, char *feature, int type, 
				  char *daemon, char * ver, char * fromver,
				  int nlic, char *date, char *sdate, 
				  char *string, char *fhostid));
lm_extern ts_opt_file lm_args((char *filename, char *optstring));
lm_extern ts_opt_file_append lm_args((char *filename, char *optstring));
lm_extern ts_lic_pkg lm_args((char *file, char *pkg));
lm_extern ts_lic_append_vendor_opt lm_args((char *file, char *daemon, 
					    char *dpath, char *optfile));
lm_extern void ts_dump lm_args((char *feature));
lm_extern good_client_line_nc lm_args((char *feature, char * ver, int nlic, 
				       VENDORCODE *vcode, int dup_mask,
				       char *argv0, char *msg, int line));
lm_extern good_client_nc lm_args((char *feature, char *ver, int nlic,
				  VENDORCODE *vcode, int dup_mask,
				  char *argv0, char *msg));
lm_extern good_client_udp_nc lm_args((char *feature, char *ver, int nlic,
				      VENDORCODE *vcode, int dup_mask,
				      char *argv0, char *msg, int linenum));
lm_extern break_client_nc lm_args((char *feature, char * ver, int print, 
				   VENDORCODE *vcode, int dup_group, 
				   char *argv0, char *msg));
lm_extern break_client_line_nc lm_args((char *feature, char * ver, int print, 
					VENDORCODE *vcode, int dup_group,
					char *argv0, char *msg, int line));
lm_extern break_client_udp_nc lm_args((char *feature, char * ver, int cnt,
				       VENDORCODE *vcode, int dup_group,
				       char *argv0, char *msg, int linenum));
lm_extern void kill_clients lm_args((void));
lm_extern void kill_server lm_args((void));
lm_extern queue_client_nc lm_args((char *feature, char * ver, int nlic, 
				  VENDORCODE *vcode, int dup_group, 
				  char *argv0));
lm_extern lm_vd_msg lm_args((LM_HANDLE *job, char *feature));
lm_extern ts_lic_file_vcode lm_args((char *file, char *hostname, int type,
				     char *hostid, char *dpath, char *dopts,
				     char *feature, char *daemon, char * ver, 
				     int nlic, char *date, char *sdate, 
				     char *string, char *fhostid, 
				     char **morehosts, VENDORCODE *vcode));
lm_extern run_server lm_args((char *name, char *string, char *log));
lm_extern ts_lic_append_vcode lm_args((char *file, char *feature, int type,
				       char *daemon, char * ver,
				       char * fromver, int nlic, char *date,
				       char *sdate, char *string,
				       char *fhostid, VENDORCODE *vcode));
lm_extern ts_config lm_args((FILE **file, char *filename, char *feature,
			     int type, char *daemon, char *version, 
			     char *fromversion, int nusers, char *date, 
			     char *sdate, char *user_string, char *hostid));
lm_extern ts_flush_server (LM_HANDLE *, int line);
lm_extern ts_borrow_hours_is_seconds lm_args((LM_HANDLE *));
lm_extern start_daemon lm_args((char *path, char *string, char *log));
lm_extern void daemon_sleep lm_args((void));
lm_extern wait_until_listening lm_args((int port));
lm_extern pause lm_args((void));
lm_extern start_client lm_args((char *feature, char * ver, int nlic, int wait,
				VENDORCODE *code, int dupmask, 
				char *argv0, int transport));
lm_extern start_client_nc lm_args((char *feature, char * ver, int nlic,
				   int wait, VENDORCODE *code,
				   int dupmask, char *argv0, int transport));
lm_extern good_client_line lm_args((char *feature, char * ver, int nlic,
				    int dup_mask, char *argv0, char *msg,
				    int line));
lm_extern void kill_last_n_clients lm_args((int n));
lm_extern break_client_line lm_args((char *feature, char * ver, int print,
				     int dup_group, char *argv0, 
				     char *msg, int line));
lm_extern void signal_clients lm_args((int signal, int n));
lm_extern void signal_client lm_args((int num, int signal));
#ifdef PC
int API_ENTRY la_reread lm_args((LM_HANDLE_PTR job, LM_CHAR_PTR,LMGRD_STAT_PTR,
					LMGRD_STAT_PTR_PTR));
#endif
/*
 *	Everybody has getcwd, some don't have getwd
 */
#ifndef NO_GETCWD
#define getwd getcwd
#endif

#ifdef PC
#undef sleep
#define sleep(x)	lc_sleep(lm_job, x)
#define pause()		Sleep(10000)
#ifdef getwd
#undef getwd
#endif
#define getwd _getcwd
#ifdef OS2
#define rm(file)	unlink(file)
#define	_putenv(string)	putenv(string)
#define LM_CALLBACK_TYPE _System
#undef getwd
#define getwd getcwd
#else /* OS2 */
#define SIGTERM		15
#define SIGKILL		9
#define X_OK 0
#define rm(file)	    _unlink(file)
#if 0 /* def WINNT*/
#define LM_CALLBACK_TYPE 
#endif /* WINNT */
#ifdef _WINDOWS
#define LM_CALLBACK_TYPE _far _pascal
#endif /* _WINDOWS */
#endif /* OS2 */
#define cp(from, to)	    CopyFile(from, to, 0)

/*
 * DLL routines to manipulate the DLL's environment
 * variables.  (Note that with OS2_USE_LOCAL_ENV set
 * for OS/2, l_putenv is not used, but l_putenv_os2 is.)
 */

lm_extern int API_ENTRY l_putenv lm_args((char far *str));
#define DLL_PUTENV		l_putenv

#if defined(OS2) && defined(OS2_USE_LOCAL_ENV)
#undef DLL_PUTENV
lm_extern int API_ENTRY l_putenv_os2   lm_args((char far *str));
#define DLL_PUTENV		l_putenv_os2
#endif /* OS2 && OS2_USE_LOCAL_ENV */

/*
 * Overload "setenv" so that values get set in both the
 * application space and the DLL space.  Note we make
 * a permanent copy in Windows 3.1 because the environment
 * only takes the pointer and does not copy the data.
 */

#define setenv(var,value)	{	\
			char *string; \
			string = (char*) malloc(strlen(var) + strlen(value) + 5); \
			sprintf( string, "%s=%s",var,value);\
			_putenv( string );\
			DLL_PUTENV( string );\
			free(string);\
		}

/*
 * In OS/2, run unsetenv calls through the DLL if we're
 * using the locally managed environment calls.
 */

#if defined(OS2) && defined(OS2_USE_LOCAL_ENV)
lm_extern int API_ENTRY l_unsetenv_os2 lm_args((char far *str));
#define unsetenv(var)		l_unsetenv_os2(var)
#else
#define unsetenv(var)		setenv(var, "")
#endif /* OS2 */

#ifndef WINNT
#define l_timer_add(a, b, c, d, e, f)
lm_extern void Sleep lm_args((int mSeconds));
lm_extern void CopyFile lm_args((char *from, char *to, int ignored));
#endif /* WINNT */
#ifdef PC16
#define CB_LOAD_DATA __loadds
#else /* PC16 */
#define CB_LOAD_DATA
#endif /* PC16 */
#else /* PC */
#define CB_LOAD_DATA
#endif /* PC */

#if defined( convex) || defined(LINUX) || defined(cray) || defined(BSDI) || defined(FREEBSD) || defined(MAC10) 
#define setenv(var, value) setenv(var, value, 1)
#endif /* convex */
#ifdef RS64
#define unsetenv ts_unsetenv
#define setenv ts_setenv
#endif


#ifndef ANSI
extern char *strcpy(), *getwd(), *strrchr();
#endif
#ifdef VMS
int setenv();
#endif
void makelic lm_args((LM_HANDLE *,char *, char *, int, char *, char *, char *));
void st_rename lm_args((char *, char *, int));
int get_feat_info lm_args((char *, int , LM_VD_FEATURE_INFO *, LM_HANDLE *));
int get_feat_info_err(char * feat, int line, LM_VD_FEATURE_INFO *fi, LM_HANDLE *job, int report_errs, CONFIG *conf);
void serv_log lm_args((char *));
char * replace lm_args((char *, char *, char *));
char * get_line lm_args((char *, char *));
void st_set_attr lm_args(( LM_HANDLE *job, int attr, char * val, int line));
int LM_CALLBACK_TYPE checkout_filter lm_args((CONFIG *));
int CB_LOAD_DATA LM_CALLBACK_TYPE checkout_filter2 lm_args((CONFIG *));

void LM_CALLBACK_TYPE x_flexlm_newid();
int ts_list lm_args((char *));
void st_license lm_args(( LM_HANDLE *, char *, int));
void ts_reread lm_args(( char *, VENDORCODE *));
void last_test(); 
#ifdef VMS
void vms_daemon_line();
#endif /* VMS */

#if 1
#define TS_PV
#else
	/* this is bogus -- not the right solution -- 
	 * please don't use void * like this... it breaks builds
	 * on lots of platforms
	 */
#define TS_PV (void *)
#endif

#ifndef lm_sleep
#define lm_sleep(x) l_select_one(0, -1, 1000 * (x)); /* sleep x secs */
#endif 
lm_extern int ts_ds_app lm_args((char *, unsigned char *, unsigned int,
					unsigned char *, unsigned int));
lm_extern int ts_ds_gen lm_args(( char *, char *, unsigned int, 
				unsigned char *, unsigned int *));
lm_extern int st_cryptstr(char *, char** ,  int , int);
