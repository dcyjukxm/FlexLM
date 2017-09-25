/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_diag.c,v 1.16.2.2 2003/07/01 17:04:21 sluu Exp $
 *
 *	Function: 	lmdiag_processing()
 *
 *	Description: 	lmdiag
 *
 *	Parameters:	feature name
 *
 *	Return:		void
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  11/13/98
 *
 */
#include "lmutil.h"
#include "flex_utils.h"
#include <sys/stat.h>

int
#ifdef PC
LM_CALLBACK_TYPE
#endif /* PC */
ckfilter lm_args(( CONFIG *));
static void lmdiag_processing lm_args((char *));
static int license_check lm_args((CONFIG *));
static make_conn lm_args((LM_HANDLE *, int,  char *));
static attempt_connects lm_args((char *, CONFIG *));
static int check_one_conf lm_args((CONFIG *));
static void lmdiag_parse lm_args(( LF_POINTER));
static unsigned short busy_ports[500];
static int num_busy_ports;
#ifdef PC
API_ENTRY
lmutil_checkout(LM_HANDLE *,char *,char * ,int ,int ,VENDORCODE *,int );
#endif


/*CONFIG *conf;*/
#define SEPARATOR "-----------------------------------------------------\n"

lmdiag_main(argc, argv)
int argc;
char *argv[];
{
  int d;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  long t;
  LF_POINTER lf;
  char *feat = (char *) NULL;

	if (lm_set_attr(LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE) 4))
		lc_perror(lm_job, "Set conn timeout failed"); 
						/* Default to 4 sec timeout */
	if (lm_set_attr(LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE) ckfilter))
		lc_perror(lm_job, "Install of checkout filter failed");
	if (lm_set_attr(LM_A_NO_TRAFFIC_ENCRYPT, (LM_A_VAL_TYPE) 1))
		lc_perror(lm_job, "Turning off traffic encryption failed");
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 1);
	lm_job->flags |= LM_FLAG_LMDIAG;
	if (argc > 1) feat = argv[1];
/* This is dead code and will never be executed 
 *	else if (argc > 2)
 *	{
 *              fprintf(ofp, "Unknown arguments %s ...\n", argv[2]);
 *              usage();
 *              return 1;
 *  }
*/	
	fprintf(ofp, lmtext("FLEXlm diagnostics on "));
#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &d, &d, &t, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &d, &d, &t);
#endif
	print_date(t);
	lf = l_open_file(lm_job, LFPTR_FIRST);
	while (lm_job->lm_errno != LM_ENDPATH)
	{
		if (lf)
		{
			fprintf(ofp, SEPARATOR);
			fprintf(ofp, lmtext("License file: %s\n"), 
				lm_job->lic_files[lm_job->lfptr]);
			fprintf(ofp, SEPARATOR);
			lmdiag_parse(lf);
			lmdiag_processing(feat);
		}
		lf = l_open_file(lm_job, LFPTR_NEXT);
	}
	lm_job->flags &= ~LM_FLAG_LMDIAG;
	return(0);
}
static 
void
lmdiag_parse(lf)
LF_POINTER lf;
{
  struct stat st;
  char *cp = (char *)0;
  char *cur, *ret = (char *)0, *err = (char *)0;
  int file = 0;

	if (lf->type == LF_STRING_PTR)
	{
		cp = lf->ptr.str.s;
	}
	else
	{
	  int n = 0, size, got = 0;
		file = 1;
		if (fstat(fileno(lf->ptr.f), &st))
			return; /* failed */
		size = st.st_size;
				
		if (!(cur = cp = (char *)malloc(size + 1)))
			return; /* failed */
		memset(cur, 0, size + 1);
		
/*
 *		NOTE: we don't use fread here (fileno instead) because
 *		of a windows bug
 */
		while ((n=read(fileno(lf->ptr.f), cur, size )) > 0)
		{
			cur += n;
			size -= n;
		}

	}	
	if (cp)
		lc_cryptstr(lm_job, cp, &ret, &code, 
			LM_CRYPT_IGNORE_FEATNAME_ERRS|LM_CRYPT_FOR_CKSUM_ONLY,
				lm_job->lic_files[lm_job->lfptr], &err);
	if (file) lc_free_mem(lm_job, cp);
	if (err)
	{
		fprintf(ofp, "ERRORS: %s\n", err);
		lc_free_mem(lm_job, err);
	}
	if (ret) lc_free_mem(lm_job, ret);
	return;
  
	
}
static
void 
lmdiag_processing(featspec)
char *featspec;
{
  CONFIG *conf = 0;
  char **features, **curfeature;
  CONFIG *nextconf = (CONFIG *) NULL;
  char *err;
  int gotone = 0;
  char *feat, *spec;

/*
 *	Display feature usage info 
 */
	if (featspec && *featspec)
	{

		lc_featspec(lm_job, featspec, &feat, &spec);
		
		while (conf = lc_findfeat(lm_job, feat, spec, conf))
		{
			if ((conf->package_mask & LM_LICENSE_PKG_ENABLE) && 
				!(conf->package_mask & LM_LICENSE_PKG_SUITEBUNDLE))
				continue;
			if (conf->lf == lm_job->lfptr)
			{
				if (gotone > 0 && diag_interactive)
				{
				  char x[100];

					fprintf(ofp, lmtext(
						"\nEnter <CR> to continue: "));
					fgets(x, sizeof(x), stdin);
				}
				gotone = check_one_conf(conf);
			}
		}
		if (!gotone)
		{
			fprintf(ofp, lmtext("No licenses for %s in this license file\n"), 
								feat);
		}
	}
	else
	{
		features = lm_feat_list(0, dummy);
		if (features)
		{
		  int gotone = 0;

			for (curfeature = features; *curfeature; 
							curfeature++)
			{

				nextconf = (CONFIG *) NULL;
				while ( conf = lc_next_conf(lm_job, 
						    *curfeature, &nextconf))
				{
				    if ((conf->package_mask & 
					LM_LICENSE_PKG_ENABLE) && 
					!(conf->package_mask & 
						LM_LICENSE_PKG_SUITEBUNDLE))
					continue;
				    if (conf->lf == lm_job->lfptr)
				    {
				        if (gotone && diag_interactive)
				        {
				          char x[100];

					    fprintf(ofp, lmtext(
						"\nEnter <CR> to continue: "));
					    gets(x);
				        }
					gotone = check_one_conf(conf);
				    }
				}
			}
		}
	}
}
static CONFIG *saveconfig;

static
int
check_one_conf(conf)
CONFIG *conf;
{
  char *err;
  int rc;


	if (!(rc = license_check(conf)))
		return rc;
	if (err = lc_chk_conf(lm_job, conf, 0))
	{
		fprintf(ofp, "%s\n", err);
		lc_free_mem(lm_job, err);
	}
	fprintf(ofp, SEPARATOR);
	return rc;

}


int
#ifdef PC
LM_CALLBACK_TYPE
#endif /* WINNT */
ckfilter(conf)
CONFIG *conf;
{
	if (conf == saveconfig) return(0);
	else return(1);
}

#if !defined(htons) && !defined(PC) && !defined (OSF) && !defined(BSDI)
#ifndef apollo
extern u_short htons();
#else
#define htons(x) (x)
#endif /* apollo */
#endif

/*
 *	Return 1 if license checked or 0 if not, -1 to ignore
 */
static
int
license_check(conf)
CONFIG *conf;
{
  int status;
  static int dup = LM_DUP_NONE;
  int tries  = 0 ;
#define NUM_DUP_GROUPS 16 /* 2^4 + 1 */

	saveconfig = conf;	/* For checkout filter */
	while (tries < NUM_DUP_GROUPS)
	{

		lc_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)
			"lmdiag");
		status = lmutil_checkout(lm_job, conf->feature, conf->version,
				1, LM_CO_NOWAIT, &code, dup);
		if (status == LM_LOCALFILTER) return 0;
		if (status != LM_BADFEATPARAM) 
			break;
		if (dup == LM_DUP_NONE)
			dup = 0;
		else
			dup++;
		if (dup > (NUM_DUP_GROUPS -1))
			dup = LM_DUP_NONE;
		tries++;
	}
	if (status == LM_POOL) return -1; /* ignore this one */
	license_info(0, conf);

	if (status == LM_NOSERVSUPP || status == LM_NOFEATURE) /* P4396 */
	{
	  CONFIG *c;
		for (c = lm_job->line; c; c = c->next)
		{
			if (l_keyword_eq(lm_job, c->feature, conf->feature) &&
				c->file_order)
			{
				status = 0;
				conf->file_order = 1;
				break;
			}
		}
	}
	if (status) 
	{
		fprintf(ofp, lmtext("This license cannot be checked out because:\n") );
		conf->file_order = 0; /* flag that it worked */
	}
	else
		conf->file_order = 1; /* flag that it worked */
	switch (status)
	{
	case 0:
		lc_checkin(lm_job, conf->feature, 0);
		if (conf->users) 
			fprintf(ofp, lmtext(
					"This license can be checked out\n"));
		else 
		{
			fprintf(ofp, lmtext(
	       "This is the correct node for this node-locked license\n"));
			}
		break;
		
	case LM_CANTCONNECT:
	{
	    char resp[100];
	    char *host = conf->server->name;
	    int i, stat;
	    int port;

		fprintf(ofp, lmtext("Cannot connect to license server\n"));
		fprintf(ofp, lmtext(
		"\n\t... I will try to determine what the problem is...\n\n"));
/*
 *		Try to connect to the telnet port first
 */
		port = 23;	/* Telnet port */
	        port = htons((unsigned short) port);
		i = make_conn(lm_job, port, host);
		if (i > 0)
		{
			fprintf(ofp, lmtext(
				"->Node %s is UP and the network is working\n"),
									host);
			lc_disconn(lm_job, 1);
		}
		else
		{
			fprintf(ofp, lmtext(
			 "\"telnet\" port (23) on node %s does not respond\n"),
									host);
			fprintf(ofp, lmtext("->if %s is a Unix system, it's probably down\n"), host);
			break;
		}
/*
 *		Next, Try to connect to the lmgrd
 */
		port = conf->server->port;	/* lmgrd port */
	        port = htons((unsigned short) port);
		i = make_conn(lm_job, port, host);
		if (i > 0)
		{
			fprintf(ofp, lmtext("->lmgrd on %s is listening\n"), 
									host);
			fprintf(ofp, lmtext(
	"\n    The most likely reason you cannnot check out your license\n"));
			fprintf(ofp, lmtext(
		"    is that the \"%s\" vendor daemon is not running.\n"), 
								conf->daemon);
			fprintf(ofp, lmtext(
		"    Examine the daemon log file on %s to determine why\n"), 
									host);
			fprintf(ofp, lmtext(
				    "    the \"%s\" daemon is not running.\n"), 
								conf->daemon);
			lc_disconn(lm_job, 1);
			break;
		}
		else
		{
		  char buf[100];
			if (conf->server->port == -1)
				sprintf(buf, "@%s", host);
			else
				sprintf(buf, "%d@%s", conf->server->port, host);
			fprintf(ofp, 
	lmtext("->the lmgrd for this license file (%s) does not respond\n"),
						buf);
			fprintf(ofp, lmtext("\n    This means that either the license servers were not\n"));
			fprintf(ofp, lmtext("    started, or that lmgrd is running on another port.  If\n"));
			fprintf(ofp, lmtext("    you would like to see if lmgrd is running on another port,\n"));
			if (!diag_interactive)
				fprintf(ofp, lmtext("    run lmdiag without the \"-n\" switch, and\n"));
			fprintf(ofp, lmtext("    choose the extended connection diagnostics.\n"));
		}
		while (diag_interactive)
		{
		    fprintf(ofp, lmtext("\n The extended connection diagnostics attempt to determine the\n"));
		    fprintf(ofp, lmtext(" reason you cannot connect to the license server.\n"));
		    fprintf(ofp, lmtext(" This process involves an attempt to connect to\n"));
		    fprintf(ofp, lmtext(" every port on the server node.  This operation can\n"));
		    fprintf(ofp, lmtext(" take a few minutes.\n\n"));
		    fprintf(ofp, lmtext("Do you want to perform the extended connection diagnostics?: [y/n]: "));
		    resp[0] = '\0';
		    gets(resp);
		    if (*resp == 'y' || *resp == 'Y')
		    {
			attempt_connects(host, conf);
			break;
		    }
		    break;
		}
	}
	break;
	case LM_NOTTHISHOST:
		{
		  HOSTID *idp;
		  int vd = 0;

			for (idp = conf->idptr; idp; idp = idp->next)
				if (idp->type == HOSTID_VENDOR)
					break;
			if (idp)
			{
			    fprintf(ofp, 
		    lmtext("Everything is okay except possibly the hostid,\n"));
			    fprintf(ofp, 
		    lmtext(" which is a non-standard, vendor-defined hostid,\n"));
			    fprintf(ofp, 
		    lmtext(" that lmdiag can't verify\n\n"));
			    break;
			}
		}
		/* fall through to default */
	default:
		fprintf(ofp, "%s\n", lc_errstring(lm_job));
		break;
	}
	return 1;
}

l_handshake(j) LM_HANDLE *j; { return 0;}


/*
 *	Routines to test connections to ports (for lmdiag)
 */
#include "l_timers.h"
#ifndef PC
#include <sys/ioctl.h>
#endif /* PC */
#include "l_socket.h"


#ifdef RELEASE_VERSION
#define DEBUG(x)
#else
#define DEBUG(x)	if (l_conn_debug) printf x
#endif

/*
 *	Timer handler
 */

static void install_timer(), put_timer_back(), pause_timer(), continue_timer();

static LM_PFV set_timer;
#ifndef VOID_SIGNAL
typedef int (*LM_PFI)();
typedef LM_PFI (*PFPFI)();
#endif

static 
void 
timerproc()
{
#ifdef VMS
	sys$setef(TIMER_EF);
	sys$setef(READ_EF);
#endif
}

static LM_TIMER connect_timer;
static int sigpipe_installed = 0;

/*ARGSUSED*/
static void _timer(sig) int sig;{ timerproc(); }/* The timer signal handler */
#ifndef PC
static SIGFUNCP _user_sigpipe = (SIGFUNCP)0;
#endif /* PC */
/*ARGSUSED*/
static void _lm_sigpipe(sig) int sig; 
{ 
	; /* NULL */
}

static struct sockaddr_in sock;
static char saved_hostname[MAX_HOSTNAME+1];
static int first = 1;
static int usetimers;
static int saved_user;
#ifndef PC
static SIGFUNCP _utimer;
#endif /* PC */
static struct itimerval timeout, remain_timeout, _utime;
static LM_SOCKET l_make_tcp_socket lm_args((LM_HANDLE *));

static
make_conn(job, port, node)
LM_HANDLE *job;		/* Current license job */
int port;		/* Port to connect to  -- short gets promoted to int */
char *node;		/* Node name to connect to */
{
  LM_SOCKET s;
  int goodhost = 1;
#ifndef RELEASE_VERSION
  char * l_conn_debug = l_real_getenv("MAKE_CONN_DEBUG"); 
#endif
#ifdef USE_WINSOCK
  long	non_blocking_mode;
#endif
  
	if (first == 1)
	{
		first = 0;
		saved_hostname[0] = '\0';
	}
	set_timer = job->options->setitimer;
#ifndef PC
  	_utimer = 0;	/* User's timer handler */
#endif /* PC */
	saved_user = 0;
/*
 *	If we have never set up a SIGPIPE handler in
 *	this process, do it now
 */
#if !defined (VMS) && !defined (PC)
	if (!sigpipe_installed)
	{
/*
 *		Set up the SIGPIPE handler
 */
		_user_sigpipe = l_timer_signal(job, SIGPIPE, _lm_sigpipe);
		if ((_user_sigpipe != SIG_IGN) && (_user_sigpipe !=SIG_DFL))
		{
/*
 *			There already was a SIGPIPE handler; put it back
 */
			l_timer_signal(job, SIGPIPE, _user_sigpipe);
		}
		sigpipe_installed = 1;
	}
#endif

/*
 *	First, set up the timers
 */
	if (job->options->conn_timeout > 0)
	{
		memset(&timeout, 0, sizeof(timeout));
		timeout.it_value.tv_sec = job->options->conn_timeout;
		usetimers = 1;
	}
	else
	{
		usetimers = 0;
	}
		
/*
 *	Create the socket, and connect
 */
	while (1)
	{
		install_timer(job);	 /* Time the whole operation out */

		if ((s = l_make_tcp_socket(job)) < 0) break;
		if (strcmp(saved_hostname, node))
		{
		  struct hostent *host;
		  long ipaddr;

			errno = 0;
			bzero((char *)&sock, sizeof(sock));
			if (ipaddr = l_ipaddr(node))
			{
				memcpy(&sock.sin_addr, &ipaddr, 4);
				sock.sin_family = AF_INET;
			}
			else
			{
				host = gethostbyname(node);
				if (host != NULL)
				{
					bcopy(host->h_addr, 
							(char *)&sock.sin_addr, 
							host->h_length);
					sock.sin_family = host->h_addrtype;
				}
				else
					goodhost = 0;
			}
		}
		if (goodhost == 0)
		{
			job->lm_errno = BADHOST;
			job->u_errno = net_errno;
			network_close(s);
			s = LM_BAD_SOCKET;
			break;
		}
		else
			l_zcp(saved_hostname, node, MAX_HOSTNAME);

		DEBUG(("Trying connection to %s port %d\n", node,
					       ntohs((unsigned short)port)));
		sock.sin_port = port;
		errno = 0;
#ifdef USE_WINSOCK	
		/*
		 *	On Windows and Windows NT platform, we will
		 *	temporarily switch to non-blocking mode connect()
		 *	so that we can control the timeout for connect().
		 */
		non_blocking_mode = 1;
		network_control(s, FIONBIO, &non_blocking_mode );

		if ( connect(s, (struct sockaddr *)&sock, sizeof(sock)) )
		{
			time_t start_time;
			fd_set fd_read, fd_write, fd_except;
			struct timeval sel_timeout;
			
			if ( (net_errno!=WSAEWOULDBLOCK)&&(net_errno!=WSAEINPROGRESS) )
			{
				/*
				 *	Connect() major error.
				 */				
				job->u_errno = net_errno;
				network_close(s);
				s = LM_BAD_SOCKET;
				job->lm_errno = CANTCONNECT;
				break;
			}

			/*
			 *	Socket is in waiting mode for connect().
			 *	Now use select() until connect() succeeds or
			 *	fails.
			 */
			start_time = time(NULL);
			sel_timeout.tv_sec = 0;
			sel_timeout.tv_usec = 100;
		
			while (1) 
			{
				FD_ZERO( &fd_write );
				FD_ZERO( &fd_read );
				FD_ZERO( &fd_except );
				FD_SET( s, &fd_write );
				FD_SET( s, &fd_read );
				FD_SET( s, &fd_except );
				
				select( 0, &fd_read, &fd_write, &fd_except, 
					 	&sel_timeout );

				if ( FD_ISSET(s, &fd_read) )
				{   
				  int i;
					/* Ready for read may mean connection 
					 * closed/reset. 
					 */
					if (recv(s, (char *)&i, 1, MSG_PEEK)
									<= 0)
					{ 
					    job->u_errno = NET_ECONNREFUSED;
					    network_close(s);
					    s = LM_BAD_SOCKET;
					    job->lm_errno = CANTCONNECT;      
					 } 
					 break;
				}

				if (FD_ISSET(s, &fd_write))
					break;

				if ( FD_ISSET( s, &fd_except ) )
				{
					/*
					 *	Connect() major error.
					 */				
					job->u_errno = NET_ECONNREFUSED;
					network_close(s);
					s = LM_BAD_SOCKET;
					job->lm_errno = CANTCONNECT;
					break;
				}
				
				if ( time(NULL) - start_time >
					     job->options->conn_timeout )
				{
					/*
					 *	Connect() timeout!
					 */
					job->u_errno = NET_ETIMEDOUT;		
					network_close(s);
					s = LM_BAD_SOCKET;
					job->lm_errno = CANTCONNECT;
					break;				
				}
#ifdef WINNT				
				Sleep(0);
#else
				Yield();
#endif				
			}

			if ( s == LM_BAD_SOCKET )
				break;

		}

		/*
		 *	switch back to blocking mode.
		 */
		non_blocking_mode = 0;
		network_control(s, FIONBIO, &non_blocking_mode );
		
#else /* USE_WINSOCK */
		if (connect(s, (struct sockaddr *) &sock, sizeof(sock)) != 0)
		{
			job->u_errno = net_errno;
			if (net_errno == NET_EINTR) job->u_errno = ETIMEDOUT;

#if !defined(apollo) && !defined(MOTO_88K) && !defined(USE_WINSOCK)
			/* Shut down the socket now */
			shutdown(s, 2);	
#endif
			network_close(s);
			s = LM_BAD_SOCKET;
			job->lm_errno = CANTCONNECT;
			break;
		}
#endif /* USE_WINSOCK */
		break;
		
	}
	put_timer_back(job);
	return(s);
}

static void
install_timer(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers && !saved_user)
	{
#ifndef PC
		_utimer = l_timer_signal(job, SIGALRM, _timer);
		(*set_timer)(
#ifdef VMS
				     job,
#endif /* VMS */
					ITIMER_REAL, &timeout, &_utime);
		saved_user = 1;
#endif		
	}
}

static void
pause_timer(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers)
	{
#ifndef PC
		l_timer_signal(job, SIGALRM, SIG_IGN);
		(*set_timer)(
#ifdef VMS
				     job,
#endif /* VMS */
					ITIMER_REAL, &timeout, &remain_timeout);
#endif
	}
}


static void
continue_timer(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers)
	{
#ifndef PC
                l_timer_signal(job, SIGALRM, _timer);
                (*set_timer)(
#ifdef VMS
				     job,
#endif /* VMS */
					ITIMER_REAL, &remain_timeout,
                                                        (struct itimerval *) 0);
#endif
	}
}

static void
put_timer_back(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers)
	{
#ifndef PC
		if (_utimer == SIG_IGN || _utimer == SIG_DFL)
			l_timer_signal(job, SIGALRM, SIG_IGN);
		else
			l_timer_signal(job, SIGALRM, _utimer);
		(*set_timer)(
#ifdef VMS
				     job,
#endif /* VMS */
					ITIMER_REAL, &_utime,
							(struct itimerval *) 0);
#endif
	}
}


static 
LM_SOCKET
l_make_tcp_socket(job)
LM_HANDLE *job;		/* Current license job */
{
  int optval = 1;
  LM_SOCKET s;

	errno = 0;
	if ( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		job->lm_errno = SOCKETFAIL;	/* Socket failure */
		job->u_errno = net_errno;
		return s;
	}
#ifndef PC
/*
*		Prevent STOPped programs from hanging the port
*/
#ifndef ALPHA
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *) &optval, 
						sizeof(int)) < 0) 
	{
		job->lm_errno = SETSOCKFAIL;
		job->u_errno = net_errno;
	}
#endif
#ifdef MOTO_88K
	{
	  int optval = 1;
	  struct linger ling;

		ling.l_onoff = 0;
		ling.l_linger = 0;
		if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ling, 
					sizeof(struct linger)) < 0)
			perror("setsockopt");
	}
#endif
#endif
	job->daemon->commtype = LM_TCP;
	return s;
}

static
attempt_connects(host, conf)
char *host;
CONFIG *conf;
{
  int num_lmgrd = 0;
  int num_ports = 0;
  int i, done = 0, stat;
  unsigned short port;
  COMM_ENDPOINT endpoint;

	fprintf(ofp, 
	lmtext("Running extended connection diagnostics to host %s\n\n"), host);
	for (i=1025; i < 32767; i++) 
	{
		port = htons((unsigned short) i);
		fprintf(ofp, lmtext("\ttrying port %s:%d ...\r"), host, i);
		if ((i % 100) == 0 || i == 1025) 
			fflush(stdout);
		stat = make_conn(lm_job, port, host);
		if (stat >= 0) 
		{
			num_ports++;
			endpoint.transport = LM_TCP;	
			endpoint.transport_addr.port =  port;
			stat = l_connect_endpoint(lm_job, &endpoint, host);
			if (stat >= 0)
			{
				fprintf(ofp, lmtext(
			      "\t There is an lmgrd running on port %d\n"), i);
				num_lmgrd++;
				lc_disconn(lm_job, 1);
				stat = l_connect_host_or_list(lm_job, 
						&endpoint, host, conf->server,
							conf->daemon, 1);
				if (stat >= 0)
				{
					fprintf(ofp, lmtext(
	"\nThe vendor daemon \"%s\" is running with the lmgrd at port %d\n"), 
						conf->daemon, i);
					fprintf(ofp, lmtext(
		"This means that the port number in your license file (%d)\n"), 
							conf->server->port);
					fprintf(ofp, lmtext(
				"is wrong, and should be changed to %d\n"), i);
					lc_disconn(lm_job, 1);
					done = 1;
					break;
				}
			}
			else
			{
				busy_ports[num_busy_ports++] = i;
			}
			lc_disconn(lm_job, 1);
		}
	}
	/* Clear the "trying port..." line */
	if (!done)
	{
		fprintf(ofp, lmtext(
		"The following ports are also listening:\n\t"));
		for (i = 0; i < num_busy_ports; i++)
			fprintf(ofp, "%d ", busy_ports[i]);
	}
	
	fprintf(ofp, "\n");
	fprintf(ofp, lmtext(
		"\n%d ports are listening, %d of them are lmgrd processes\n"),
							num_ports, num_lmgrd);
	fprintf(ofp, lmtext(
	"Extended diagnostics done - all ports found reported above\n"));
}
