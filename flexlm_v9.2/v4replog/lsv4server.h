/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation Software Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation Software Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation Software Inc.

 *****************************************************************************/
/*
 *	Module:	lsv4server.h v1.3.0.0
 *
 *	v4 report.log compatibility package
 *	Description: Local definitions for license manager servers
 *
 *	D. Birns
 *	4/5/96
 *
 *	Last changed:  4/5/96
 *
 */

#ifndef _LS_SERVER_H_
#define _LS_SERVER_H_

#include "lm_comm.h"
#include <stdio.h>
#include <sys/types.h>
#ifdef USE_WINSOCK
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

/*
 *	BASE_EXIT must be a number higher than the signals available
 *	on the o/s, and low enough that BASE_SIG + <num> is < 255
 *	which is the highest possible exit value on unix.
 */
#ifndef NSIG
#include <signal.h>
#endif
#ifndef NSIG
#define BASE_EXIT 200 /*200 allows 55 exit values*/
#else
#define BASE_EXIT NSIG  /* for compatibility with older versions */
#endif

/*
 *	Error exit codes - These are used by the master to tell
 *		what went wrong with the vendor daemon
 */

#define EXIT_BADCONFIG	    BASE_EXIT + 1	/* Bad configuration data */
#define EXIT_WRONGHOST	    BASE_EXIT + 2	/* Invalid host */
#define EXIT_PORT_IN_USE    BASE_EXIT + 3	/* Internet "ADDRESS ALREADY IN USE" */
#define EXIT_NOFEATURES	    BASE_EXIT + 4	/* No features to serve */
#define EXIT_COMM	    BASE_EXIT + 5	/* Communications error */
#define EXIT_DESC	    BASE_EXIT + 6	/* Not enough descriptors to */
							/* re-create pipes */
#define EXIT_NOMASTER	    BASE_EXIT + 7	/* Couldn't find a master */
#define EXIT_SIGNAL	    BASE_EXIT + 8	/* Exited due to some signal */
#define EXIT_SERVERRUNNING  BASE_EXIT + 9	/* Exited because another server */
							    /* was running */
#define EXIT_MALLOC	    BASE_EXIT + 10	/* malloc() failure */
#define EXIT_BICKER	    BASE_EXIT + 11	/* Servers can't agree on who is */
							     /* the master */
#define EXIT_BADDAEMON	    BASE_EXIT + 12	/* DAEMON name doesn't agree between */
						/* daemon and license file */
#define EXIT_CANTEXEC	    BASE_EXIT + 13	/* Child cannot exec requested server */
#define EXIT_REQUESTED	    BASE_EXIT + 14	/* lmgrd requested vendor daemon down */
#define EXIT_EXPIRED	    BASE_EXIT + 15	/* demo version has expired */
#define EXIT_BADCALL	    BASE_EXIT + 16	/* vendor daemon started incorrectly */
#define EXIT_INCONSISTENT   BASE_EXIT + 17	/* vendor daemon consistency error */
#define EXIT_FEATSET_ERROR  BASE_EXIT + 18	/* Feature-set inconsistent */
#define EXIT_BORROW_ERROR   BASE_EXIT + 19	/* Borrow database corrupted */
#define EXIT_NO_LICENSE_FILE BASE_EXIT + 20	/* No license file */
#define EXIT_NO_SERVERS     BASE_EXIT + 20	/* Vendor keys don't support */


typedef unsigned char SERVERNUM;
#define MAX_SERVER_NUM 256		/* Maximum # of servers in a "chain" */

#ifdef MULTIPLE_VD_SPAWN
extern SERVERNUM snum;	/* Our server number */
#endif

#define TRUE 1
#define FALSE 0
#define INCLUDE_DUMP_CMD	    /* Define to include (debugging) dump command */
#define MASTER_TIMEOUT 10	    /* # of seconds to wait for master to respond */
#define TIMERSECS 60                /* Timer to go off to send data to another lmgrd */
#define MASTER_ALIVE (2*TIMERSECS)  /* # seconds to wait for any data from */
				    /* another lmgrd - if we don't get it, 
				      declare the other lmgrd down */

#define CLIENT_FD   0
#define CLIENT_SOCK 1
typedef int HANDLE_T;

#ifdef SUPPORT_FIFO
typedef struct _ls_local_addr {
	int server_write; /* write fd */
	char *name;	/* there are actually two files: with
				 * "cr" or "cw" appended for client read
				 * or client write -- destroyer of
				 * CLIENT_ADDR is responsible for freeing
				 * malloc's memory.  This is the name
				 * that's sent in the original message from
				 * client.
				 */
} LS_LOCAL_DATA;
#endif

typedef union _addr_id  {
			LM_SOCKET	fd;		/* if is_fd */
			struct sockaddr_in sin; 	/* if LM_UDP */
		} ADDR_ID;
	
typedef struct _client_addr {
		ADDR_ID addr; 		/* enough to uniquely identify */
		int is_fd; 		/*boolean -- TRUE if fd address*/
		int transport;          /*LM_TCP, LM_UDP, LM_LOCAL*/
#ifdef SUPPORT_FIFO
		LS_LOCAL_DATA local;
#endif
	} CLIENT_ADDR;

#define MAX_CLIENT_HANDLE sizeof(HANDLE_T) /*(32 bit int)*/

typedef struct client_data {
		CLIENT_ADDR addr;		/* fd / sockaddr*/
		HANDLE_T handle;
		char name[MAX_USER_NAME +1];		/* Username *//* LONGNAMES */
		char node[MAX_SERVER_NAME + 1];		/* Node *//* LONGNAMES */
		char display[MAX_DISPLAY_NAME + 1];	/* Display *//* LONGNAMES */
		char vendor_def[MAX_VENDOR_CHECKOUT_DATA + 1]; /* Changes with
								each checkout */
		short use_vendor_def;		/* Use vendor_def data */
		short inet_addr[4];		/* Internet addr */
		long time;	/* Time started */
		char platform[MAX_PLATFORM_NAME + 1];	/* e.g., sun4_u4 */
		unsigned int lastcomm;	/* Last communications 
					 * received (or heartbeat sent) 
					 * if -1, marked for later deletion
					 */
#define	LS_DELETE_THIS_CLIENT 0xffffffff
		long encryption; /* Comm encryption code */
		int comm_version; /* Client's communication version */
		int comm_revision; /* Client's communication revision */
		int udp_sernum;	 /*For lost or duplicate UDP messages*/
		int udp_timeout;
		char *udp_last_resp; 	/* For lost/duplicate *UDP messages*/
					/* Must be malloc'd -- only for
					 * UDP clients*/
		long pid;		/* client's pid */
		HOSTID *hostids;       /* null-terminated array of
					 * client's hostids */
		char *project;		/* $LM_PROJECT for reportlog */
		short capacity;		/* Multiplier for checkout cnt */
		unsigned char flexlm_ver; /* FLEXlm ver (>= v5) */
		unsigned char flexlm_rev; /* FLEXlm revision (>= v5) */
#ifdef LM_GENERIC_VD
		char *flist;		/* Gets cast to a FEATURE_LIST * by
					   vendor daemon */
		char *gen_sav_msg;		/* used by generic server */
#endif /* GENERIC */
	} CLIENT_DATA;


typedef struct lm_quorum {
				int count;      /* How many active */
				int quorum;     /* How many required */
				int master;	/* Index into list of master */
				int n_order;	/* How many order messages we
						   need before we start */
				int alpha_order; /* Use alphabetical master
						   selection algorithm */
				LM_SERVER *list; /* Who they are */
				int debug;	/* Debug flag */
			  } LM_QUORUM;


typedef struct ls_pooled {
			struct ls_pooled *next;
			char date[DATE_LEN+1]; 
			char code[MAX_CRYPT_LEN+1];
			char *lc_vendor_def;
			LM_CHAR_PTR lc_dist_info;
			LM_CHAR_PTR lc_user_info;
			LM_CHAR_PTR lc_asset_info;      
			LM_CHAR_PTR lc_issuer;
			LM_CHAR_PTR lc_notice;  
			int users; 	/* How many expire at that date */
} LS_POOLED;
			
/*
 *	States involved in connecting to other servers (bitmap)
 */
#define C_NONE 0	/* Not started */
#define C_SOCK 1	/* Socket to other server connected */
#define C_SENT 2	/* (fd1) Socket connected, Msg sent to other master */
#define C_CONNECT1 4	/* (fd1) Connection to other server complete */
#define C_CONNECT2 8	/* (fd2) Other server has established connection */
#define C_CONNECTED 16	/* Completely connected */
#define C_MASTER_READY 32 /* Connection complete, master elected and ready */
#define C_TIMEDOUT 64	/* Connection timed out - don't retry */
#define C_NOHOST 128	/* This host doesn't exist */

/*
 *	Output to error log file
 *
 *	The LOG_INFO macros are used to generate the documentation
 *	on all the LOG calls.  They are always ignored, as far as
 *	the code is concerned.
 */

#define LOGERR(x) \
	{\
		ls_log_prefix(LL_LOGTO_BOTH, LL_ERROR); \
		ls_log_error x;\
	}
#define _LOGERR(x) ls_log_error x ;
#define LOG(x) {ls_log_prefix(LL_LOGTO_ASCII, 0); (void) ls_log_asc_printf x;}
#define LOG_INFO(x)

/*
 *	If LOG_TIME_AT_START is defined, all daemon logging is
 *	done with timestamps at the beginning of the lines, rather
 *	than just certain (important) lines being timestamped at the
 *	end.  LOG_TIME_AT_START became the default in v1.3.
 */
#define LOG_TIME_AT_START	/* Log time @ start of log lines */

/* 
 *	Default directory for lock file and lmgrd info file
 */

#ifdef WINNT 
#define LS_LOCKDIR "C:\\flexlm"
#else
#define LS_LOCKDIR "/usr/tmp/.flexlm"
#endif /* WINNT */

/*
 *	Output to log without header (for building strings)
 */
#define _LOG(x) { (void) ls_log_asc_printf x; }


extern int dlog_on;
extern LM_SOCKET ls_udp_s;		/* Global UDP socket for reading */
extern int _ls_fifo_fd;		/* Global fifo fd for reading */
extern LM_HANDLE *lm_job;	/* Server's one (and only) job */

#define DLOG(x) { if (dlog_on) \
	  { ls_log_prefix(LL_LOGTO_ASCII, -1); (void) ls_log_asc_printf x; } }
/*
 *	Output to log without header (for building strings)
 */
#define _DLOG(x) { if (dlog_on) { (void) ls_log_asc_printf x; } }

/*
 *	Exchange descriptors so that parent-child communication is happy
 */

#define XCHANGE(_p) { int _t = _p[3]; _p[3] = _p[1]; _p[1] = _t; }

/*
 *	Server's malloc - simply logs an error if malloc fails, and exits
 */

extern char *ls_malloc();
#define LS_MALLOC(x)	ls_malloc((x), __LINE__, __FILE__)

/*
 *	listen() backlog - number of pending connections
 */
#define LISTEN_BACKLOG 50

/*
 *	Get the time 
 */
/* 
 *	Sun/Vax uses _TIME_ to detect <sys/time.h>, Apollo uses the 
 *	same include file for both <time.h> and <sys/time.h>
 */

#ifndef ITIMER_REAL	
#include <time.h>
#endif
#ifdef THREAD_SAFE_TIME
struct tm *ls_gettime(struct tm * ptst);
#else /* !THREAD_SAFE_TIME */
struct tm *ls_gettime();
#endif

#include "lsv4_sprot.h"

#define LS_READ_TIMEOUT 60000 /* 60 seconds */
#endif 	/* _LS_SERVER_H_ */
