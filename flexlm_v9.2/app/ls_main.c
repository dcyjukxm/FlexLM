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
 *	Module: $Id: ls_main.c,v 1.43 2003/06/18 21:50:33 sluu Exp $
 *
 *	Function: ls_main(tcp_s, tcp_s, master_name)
 *
 *	Description: Main loop of the generic top_level servers
 *
 *	Parameters:	(LM_SOCKET) tcp_s - The socket to accept tcp connections
 *			(LM_SOCKET) tcp_s - The socket to accept spx connections
 *			(char *) master_name - name of the master server
 *
 *	Return:		None -- returns when the # of servers drops
 *				below a quorum.
 *
 *	M. Christiano
 *	2/28/88
 *
 *	Last changed:  12/29/98
 *
 */
#include "lmachdep.h"
#include <stdio.h>
#ifndef WINNT
#if !defined(NLM) && !defined(OS2)
#include <sys/time.h>
#endif /* !defined(NLM) && !defined(OS2) */
#else  /* WINNT */
#include <io.h> 
#endif /* WINNT */
#if defined( MOTO_88K) || defined(MIPS) || defined(apollo) || defined(sony_news)
#include <sys/types.h>
#endif
#ifdef PCRT
#include <sys/file.h>
#endif
#include <errno.h>
#ifndef NLM
#include <fcntl.h>		/* HP needs this */
#include <sys/stat.h>
#endif /* NLM */
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "lmselect.h"
#include "ls_aprot.h"
#include "lssignal.h"
#include "flexevent.h"

#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif


#ifndef RELEASE_VERSION
static char * debug = (char *)-1;
#define DEBUG(x) if (debug) printf x
#define DEBUG_INIT  if (debug == (char *)-1) debug = l_real_getenv("LS_MAIN")
#else
#define DEBUG(x)
#define DEBUG_INIT  
#endif

void ls_pooled_check lm_args((FEATURE_LIST *));

#ifdef PC
typedef void (LM_CALLBACK_TYPE * LS_CB_DAEMON_PERIODIC) (void);
#define LS_CB_DAEMON_PERIODIC_TYPE (LS_CB_DAEMON_PERIODIC)
#else

#define LS_CB_DAEMON_PERIODIC_TYPE 

#endif /* PC */

extern int ls_new_server;	/* New server is going to connect */
extern time_t ls_test_hang;
extern HOSTID *vendor_daemon_hostid;
extern int _ls_going_down;
extern int timer_expired;
extern int ls_got_reread;
#ifndef NO_REDUNDANT_SERVER
static void tell_them();
static void checklock_err lm_args((int));
#ifndef VMS
static void check_baddate lm_args((int));
#endif
#endif /* NO_REDUNDANT_SERVER */
static void ls_check_queue_timeout();
int ls_baddate_detected;		/* True if l_baddate() failed */

int ls_max_msgs_per_minute;
int ls_msg_cnt;
int ls_msg_cnt_this_minute;

static long ls_last_baddate_check;
extern void (*ls_daemon_periodic)();	/* Vendor-specified periodic call */
int dummy_fd[MAX_SERVERS][2];		/* File descriptors that hold a space
					   for servers that haven't connected
					   yet.  This is so clients will
					   not use up all the descriptors
					   in this server. */
static int first = 1;
static int wake;			/* DUMMY for ls_wakeup() */
static SELECT_MASK select_mask  = (SELECT_MASK)0;
					/* File descriptors to select on */
extern long ls_reread_time;
extern void ls_post_delayed_user_based(void);
extern int ls_user_based_reread_delay;

extern void (*ls_borrow_init)(char ** pszBorrowBuffer, int * piSize);
extern void (*ls_borrow_out)(char * szBorrowData, int iSize);

int
f_reserve_avail(
	FEATURE_LIST *	fl,		/*- The feature desired */
	CLIENT_DATA *	client,	/*- User making request */
	int				want,	/*- How many we want (if we are grabbing them) */
	OPTIONS **		list);	/*- The listhead (if specfied, grab them and link them here) */


static int retry_times=0;		/* for multiple tries for hostid */




static
int
sGetText(
	char *	pBuffer,
	char *	pSep)
{
	char *	pMark = NULL;

	pMark = strstr(pBuffer, pSep);

	if(pMark)
	{
		return pMark - pBuffer;
	}
	else
		return 0;
}

static
void
sAddBorrow(
	void *	pBuffer,
	int		size)
{
	CLIENT_DATA		client;
	FEATURE_LIST *	pFeature = NULL;
	USERLIST *		pTmp = NULL;
	USERLIST		user;
	USERLIST *		pUser = NULL;
	char *			pCurr = (char *)pBuffer;
	char *			pRange = pCurr + size;
	char			major_ver;
	char			minor_ver;
	char			borrow_ver;
	char			feature[MAX_FEATURE_LEN] = {'\0'};
	int				len = 0;
#define SEP					":"
#define VALID_RANGE()		if(pCurr > pRange) return
#define VALID_STRING()		if((pCurr + len) > pRange) return
#define MAX_VALUE(x)		(len > x ? x : len)


	memset(&client, 0, sizeof(CLIENT_DATA));
	memset(&user, 0, sizeof(USERLIST));

	/* Skip length field */
	pCurr += sizeof(int) + 1;
	VALID_RANGE();
	
	memcpy(&major_ver, pCurr++, 1);
	pCurr++;
	VALID_RANGE();

	memcpy(&minor_ver, pCurr++, 1);
	pCurr++;
	VALID_RANGE();

	memcpy(&borrow_ver, pCurr++, 1);
	pCurr++;
	VALID_RANGE();
	
	memcpy(&user.dup_select, pCurr, sizeof(user.dup_select));
	pCurr += sizeof(user.dup_select) + 1;
	VALID_RANGE();
	
	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(feature, pCurr, MAX_VALUE(sizeof(feature)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.name, pCurr, MAX_VALUE(sizeof(user.name)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.node, pCurr, MAX_VALUE(sizeof(user.node)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.display, pCurr, MAX_VALUE(sizeof(user.display)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.project, pCurr, MAX_VALUE(sizeof(user.project)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.vendor_def, pCurr, MAX_VALUE(sizeof(user.vendor_def)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.version, pCurr, MAX_VALUE(sizeof(user.version)));
	}
	pCurr += len + 1;

	len = sGetText(pCurr, SEP);
	if(len)
	{
		VALID_STRING();
		memcpy(user.code, pCurr, MAX_VALUE(sizeof(user.code)));
	}
	pCurr += len + 1;

	memcpy(&user.count, pCurr, sizeof(user.count));
	pCurr += sizeof(user.count) + 1;
	VALID_RANGE();

	memcpy(&user.linger, pCurr, sizeof(user.linger));
	pCurr += sizeof(user.linger) + 1;
	VALID_RANGE();

	memcpy(&user.endtime, pCurr, sizeof(user.endtime));
	pCurr += sizeof(user.endtime) + 1;
	VALID_RANGE();

	memcpy(&user.normal_count, pCurr, sizeof(user.normal_count));
	pCurr += sizeof(user.normal_count) + 1;
	VALID_RANGE();

	memcpy(&user.flags, pCurr, sizeof(user.flags));

	/*
	 *	Check to see if linger/borrow is still valid
	 */

	if(ls_currtime > (user.endtime + user.linger))
		return;

	l_zcp(client.name, user.name, MAX_USER_NAME);	/* LONGNAMES */
	l_zcp(client.node, user.node, MAX_SERVER_NAME);	/* LONGNAMES */
	l_zcp(client.display, user.display, MAX_DISPLAY_NAME);	/* LONGNAMES */
	l_zcp(client.vendor_def, user.vendor_def,MAX_VENDOR_CHECKOUT_DATA);/*LONGNAMES */
	l_zcp(client.project, user.project,MAX_PROJECT_LEN);
	client.flags |= CLIENT_FLAG_BORROW_INIT;
	client.use_vendor_def = 1;
	client.handle = user.handle;
	client.flexlm_ver = FLEXLM_VERSION;
	client.flexlm_rev = FLEXLM_REVISION;

	/*
	 *	make sure the feature is valid
	 */
	pFeature = f_get_feat(feature, user.code, 1);
	if(pFeature)
	{
		int		numusers = 0;
		int		numlic = 0;
		int		numres = 0;
		/*
		 *	Count the number of licenses available
		 */
		f_featcount(pFeature, &numusers, &numlic);
		numres = f_reserve_avail(pFeature, &client, user.count, NULL);
		if( ((pFeature->nlic - numlic) < user.count) &&
			(numres < user.count))
		{
			return;
		}
		else
		{
			/*
			 *	This is a hack.  If this were left at 0, then this user would be matched with 
			 *	an existing client.  By setting this to the number of current users, it makes
			 *	sure that the client doesn't match a previously added user for this feature.
			 *	Like I said, this is a hack.
			 */
			client.handle = numusers;
		}

		/*
		 *	Check to see if it's a BUNDLE component, if yes, reserve the features then proceed.
		 */

		/*
		 *	Determine how many reservations are available for this user.
		 */
		if(pFeature->flags & LM_FL_FLAG_BUNDLE_COMPONENT)
		{
			/*
			 *	Dynamic Reserve first
			 */
			FEATURE_LIST *	pParent = NULL;

			numres = f_reserve_avail(pFeature, &client, user.count, NULL);

			if((numres < user.count) && (user.count > 0))
			{
				/*
				 *	Get parent feature
				 */
				if(pFeature->flags & LM_FL_FLAG_BUNDLE_PARENT)
				{
					pParent = pFeature;
				}
				else
				{
					pParent = f_get_feat(pFeature->package->feature, pFeature->package->code, 1);
				}

				if(pParent)
				{

					/*
					 *	Checking out a BUNDLE, dynamically reserve all the components
					 *	then, checkout only the component I'm interested in.
					 */
					char	buffer[1024] = {'\0'};	/* OVERRUN */

					if(pParent->dup_select && pParent->suite_dup_group)
					{
						if(pParent->suite_dup_group & LM_DUP_USER)
						{
							sprintf(buffer, client.name);/*	OVERRUN */
						}
						strcat(buffer, ":");
						if(pParent->suite_dup_group & LM_DUP_HOST)
						{
							strcat(buffer, client.node);/* OVERRUN */
						}
						strcat(buffer, ":");
						if(pParent->suite_dup_group & LM_DUP_DISP)
						{
							strcat(buffer, client.display);/* OVERRUN */
						}
						strcat(buffer, ":");
						if(pParent->suite_dup_group & LM_DUP_VENDOR)
						{
							strcat(buffer, client.vendor_def);/* OVERRUN */
						}
					}
					else
					{
						/*
						 *	NO SUITE_DUP_GROUP specified, matches anything
						 */
						strcpy(buffer, "*:*:*:*");
					}
					f_dynamic_reserve(pParent, buffer, user.count - numres, 1);
				}
			}
		}

		f_add(&client, feature, user.count, user.version, user.dup_select, user.linger,
					user.code, user.serialno, 1, NULL);

		/*
		 *	Now set the endtime so linger will clean it up
		 */

		pUser = f_lookup(&client, feature, &pTmp, &pFeature, 0,
							user.code, 0, 0, user.dup_select);
		if(pUser)
		{
			pUser->endtime = user.endtime;
			pUser->flags = user.flags;
			pUser->flags |= LM_ULF_BORROW_INIT;
			pUser->handle = 0;
		}

		/*
		 *	call ls_borrow_out
		 */
		if(ls_borrow_out)
		{
			char	buffer[4096] = {'\0'};
			char *	pCurr = buffer;
			int		len = 0;
			long	flags = 0;

			memset(buffer, ':', sizeof(buffer));

			pCurr += sizeof(int) + 1;	/* skip first field */
			memset(pCurr++, major_ver, 1);	/* VERSION */
			pCurr++;
			memset(pCurr++, minor_ver, 1);/* REVISION */
			pCurr++;
			memset(pCurr++, borrow_ver, 1);	/* BORROW REVISION */
			pCurr++;
			memcpy(pCurr, &pUser->dup_select, sizeof(pUser->dup_select));
			pCurr += sizeof(pUser->dup_select) + 1;
			memcpy(pCurr, feature, strlen(feature));
			pCurr += strlen(feature) + 1;
			memcpy(pCurr, pUser->name, strlen(pUser->name));
			pCurr += strlen(pUser->name) + 1;
			memcpy(pCurr, pUser->node, strlen(pUser->node));
			pCurr += strlen(pUser->node) + 1;
			memcpy(pCurr, pUser->display, strlen(pUser->display));
			pCurr += strlen(pUser->display) + 1;
			memcpy(pCurr, pUser->project, strlen(pUser->project));
			pCurr += strlen(pUser->project) + 1;
			memcpy(pCurr, pUser->vendor_def, strlen(pUser->vendor_def));
			pCurr += strlen(pUser->vendor_def) + 1;
			memcpy(pCurr, pUser->version, strlen(pUser->version));
			pCurr += strlen(pUser->version) + 1;
			memcpy(pCurr, pUser->code, strlen(pUser->code));
			pCurr += strlen(pUser->code) + 1;
			memcpy(pCurr, &pUser->count, sizeof(pUser->count));
			pCurr += sizeof(pUser->count) + 1;
			memcpy(pCurr, &pUser->linger, sizeof(pUser->linger));
			pCurr += sizeof(pUser->linger) + 1;
			memcpy(pCurr, &pUser->endtime, sizeof(pUser->endtime));
			pCurr += sizeof(pUser->endtime) + 1;
			memcpy(pCurr, &pUser->normal_count, sizeof(pUser->normal_count));
			pCurr += sizeof(pUser->normal_count) + 1;
			if(pUser->flags & LM_ULF_BORROW_INIT)
			{
				flags = pUser->flags & (LM_ULF_BORROW_INIT ^ 0xffffffff);
				memcpy(pCurr, &flags, sizeof(flags));
			}
			else
			{
				memcpy(pCurr, &pUser->flags, sizeof(pUser->flags));
			}
			pCurr += sizeof(pUser->flags);
			len = pCurr - buffer;
			memcpy(buffer, &len, sizeof(len));

			/*
			 *	memset rest of buffer to 0
			 */
			memset(pCurr, 0, (sizeof(buffer)) - len);
			ls_borrow_out(buffer, len);
		}
	}
}

static
void
sProcessBorrowInfo(
	void *	pBuffer,
	int		size)
{
	char	entry[4096] = {'\0'};
	int		length = 0;
	char *	pCurr = (char *)pBuffer;
	
	ls_currtime = time(0);

	while(size)
	{
		/*
		 *	Determine how long the entry is
		 */
		memcpy(&length, pCurr, sizeof(length));
		if(length > size)
			break;
		memcpy(entry, pCurr, length);
		pCurr += length;
		size -= length;

		/*
		 *	Now process this entry
		 */
		sAddBorrow(entry, length);
	}
}

#ifdef EXIT_FILE_CHECK
/*
 *	Only used for debugging purposes.
 */
static
int
sPresent()
{
	if(access("c:\\temp\\exit.txt", 0) == 0)
	{
		return 1;
	}
	return 0;

}
#endif


void
ls_main(LM_SOCKET	tcp_s,		/*	Socket for tcp connections */
		LM_SOCKET	spx_s,		/*	Socket for spx connections */
		char *		master_name)/*	The main server node	*/
{
	SELECT_MASK ready_mask;
	SELECT_MASK write_mask;
	int havemaster = 0;
	int still_waiting;
	int nrdy;
	int i;
	LM_SERVER *ls;
	int havequorum = 0;	/* We don't start with a quorum */
	int tot_serv = 0;	/* Keep track of open files, 
			   so servers can always connect */
	int num_serv = 0;	/* tot_serv is total required, 
			   num_serv is current number in use */
	int currtime;/* used by ls_ck_udp_clients so we don't
			 * check too often
			 */
	int last_fired_at = 0;
	int rc;

	DEBUG_INIT;


	if(ls_borrow_init)
	{
		char *	pBuffer = NULL;
		int		size = 0;

		ls_borrow_init(&pBuffer, &size);

		if(pBuffer && size)
		{
			sProcessBorrowInfo(pBuffer, size);
			free(pBuffer);
			pBuffer = NULL;
		}

	}

	ls_currtime = currtime = time(0);
/*
 *	Flag all the dummy file descriptors as unused
 */
	if (first)
	{
		first = 0;
/*
 *		We need these 3 masks forever
 */
		MASK_CREATE(select_mask);
		MASK_CREATE(ready_mask);
		MASK_CREATE(write_mask);
		if (!select_mask || !ready_mask || !write_mask)
		{
			LOG(("Out of memory, exiting\n"));
			ls_go_down(EXIT_MALLOC);
		}

		for (i = 0; i < MAX_SERVERS; i++)
		{
			dummy_fd[i][0] = LM_BAD_SOCKET;
			dummy_fd[i][1] = LM_BAD_SOCKET;
		}
	}
/*
 *	Initialize the user structure
 */

	MASK_ZERO(select_mask);
	if (tcp_s != LM_BAD_SOCKET) /* LM_LOCAL */
	{
		MASK_SET(select_mask, tcp_s);	/* Always select the TCP and */
	}
	else
		LOG(("tcp_s is bad!!!\n"));
#ifdef SUPPORT_IPX
    if (spx_s != LM_BAD_SOCKET) /* LM_LOCAL */
    {
            MASK_SET(select_mask, spx_s);   /* Always select the TCP and */
    }
#endif /* SUPPORT_IPX */
	if (ls_udp_s != LM_BAD_SOCKET) 
		MASK_SET(select_mask, ls_udp_s);
	while (1)
	{
		ls_currtime = time(0);

#ifdef EXIT_FILE_CHECK
		if(sPresent())
			_ls_going_down = 1;
#endif

		if (_ls_going_down)
			ls_go_down(_ls_going_down);
		else if(ls_got_reread)
			ls_reread();

		if (ls_currtime - last_fired_at > TIMERSECS )
			timer_expired = 1;
		if (ls_currtime - last_fired_at > REPLOG_TIMESTAMP)
			ls_replog_timestamp();

		if (ls_test_hang && (ls_currtime > ls_test_hang))
		{
			ls_test_hang = 0;
			DLOG(("hanging for 4 minutes\n"));
			l_select_one(0, -1, 1000*60*4);
			DLOG(("back\n"));
		}

		if (ls_reread_time && 
			((ls_currtime - ls_reread_time) > 
						ls_user_based_reread_delay))
		{
			ls_post_delayed_user_based();
		}

				

		if (timer_expired)
		{
			DEBUG(("timer_expired\n"));
			
			last_fired_at = ls_currtime;
			f_checklinger(1);    /* Check all lingering licenses */
			if (ls_daemon_periodic) 
			{
				(* LS_CB_DAEMON_PERIODIC_TYPE 
					ls_daemon_periodic)();
			}
			ls_log_msg_cnt();

/* If we are on a ALPHA NT, we may need to check the dongle id
	more than once, P2662  */

			if (((rc = ls_host()) > 0)
#ifdef WINNT
				|| ((rc = ls_host()) > 0) || 
				((rc = ls_host()) > 0)
#endif
					)
			{
				retry_times=0   ; /*  OK */
			}
			else
			{
				if (!rc)
				{
					retry_times++; 
					/* P2836 give up to 3 minutes to
						 replace dongle */
					if (retry_times==3)
					{
						LOG((lmtext("No valid hostids, exiting\n")));
						ls_go_down(EXIT_WRONGHOST);
					}
				}
				else
				{
					LOG((lmtext("No features to serve, exiting\n")));
					ls_go_down(EXIT_NOFEATURES);
				}
			}
			ls_flush_replog();
			
		}
		MASK_ZERO(write_mask);
/*
 *		Continue until we get a quorum, then after we have one, 
 *		as long as the master is alive and we still have a quorum.
 */
#ifndef NO_REDUNDANT_SERVER		
/*
 *			First, time out any connections that haven't made it.
 */
		still_waiting = ls_serv_time(select_mask);
/*
 *			If we have no master node specified, get out.
 */
		if (!ls_s_havemaster())
		{
			LOG((lmtext("No master specified\n")));
			LOG_INFO((ERROR, "A vendor daemon was running \
				with no master node specified.  This \
				is an internal consistency error."));
			break;
		}
/*
 *			Find out if we are connecting, or running
 */
		if (!havequorum)
		{
		    if (!still_waiting)
		    {
				if (ls_s_havequorum())
				{
					havequorum = 1;
					if (ls_s_imaster())
						tell_them();
					if (ls_s_masterup())
						havemaster = 1;
				}
				else
				{
					  LOG((lmtext("No quorum established, exiting\n")));
					  break;	/* Go try again ... */
				}
		    }
		}
		else
		{
/*
 *				See if we lost quourm or the master
 *				first let lmgrd have a chance to kill us.
 */
			if ((!ls_s_masterup() && !ls_s_imaster()) ||
			     !ls_s_havequorum())  /* Lost quorum */
			{
				struct timeval t;
				int j;
				t.tv_sec = 1;
				t.tv_usec = 0;
/* 
 *					We do this 1 second/3 times
 *					because on PC select() is not
 *					interrupted by signals
 */
				for (j=0;j<3;j++)
				{
					(void) l_select(0, 0, 0, 0, &t);
/*
 *						if _ls_going_down is set,
 *						it means that ls_exit()
 *						was called
 */
					if (_ls_going_down)
						break;
				}
			}

			if (!_ls_going_down &&
			    ((!ls_s_masterup() && !ls_s_imaster()) ||
			     !ls_s_havequorum()))
			{
				LOG((lmtext("Lost quorum, exiting\n")));
				LOG_INFO((INFORM, "The vendor daemon \
					lost quorum, so it will \
					exit."));
				ls_lost(0);	/* Signal clients */
				break;   
			}
		}
/*
 *              	If we were waiting for a particular node to come
 *              	up, see if it is up now.
 */
		if (havequorum && !havemaster)
		{
			if (ls_s_masterup())
			{
				havemaster = 1;
			}
		 }
/*
 *			Next, build the masks for connections to other servers,
 *			and keep track of the dummy file descriptors.
 */
		tot_serv = num_serv = 0;
		for (i = 0, ls = ls_s_first(); ls; ls = ls->next, i++)
		{
			tot_serv += 2;
			if (ls->sflags & L_SFLAG_US)
				num_serv++;
			else if (ls->fd1 != LM_BAD_SOCKET) 
			{
				num_serv++;
				MASK_SET(select_mask, ls->fd1);
				if (dummy_fd[i][0] != LM_BAD_SOCKET)
				{
					(void) network_close(dummy_fd[i][0]);
					dummy_fd[i][0] = LM_BAD_SOCKET; 
				}
			}
			else if (!(ls->sflags & L_SFLAG_US) && 
				(dummy_fd[i][0]==LM_BAD_SOCKET) &&
					(ls->fd2 == LM_BAD_SOCKET) )
/*
 *				That's right, don't save this descriptor if
 *				fd2 is valid.  This is because in the case
 *				where another server has connected to us on
 *				fd2 and we are out of file descritors, if we
 *				save this one we won't be able to connect
 *				to him on fd1.
 */
			{
				dummy_fd[i][0] = dup(1);
			}

			if (ls->sflags & L_SFLAG_US)
				num_serv++;
			else if (ls->fd2 != LM_BAD_SOCKET) 
			{
				num_serv++;
				MASK_SET(select_mask, ls->fd2);
/*
 *					If fd2 is set and fd1 wasn't, we need
 *					to get rid of the first dummy descriptor
 *					so that we can finish the connection
 *					to this guy.
 */
				if (dummy_fd[i][0] != LM_BAD_SOCKET)
				{
					(void) network_close(dummy_fd[i][0]);
					dummy_fd[i][0] = LM_BAD_SOCKET;
				}
				if (dummy_fd[i][1] != LM_BAD_SOCKET)
				{
					(void) network_close(dummy_fd[i][1]);
					dummy_fd[i][1] = LM_BAD_SOCKET;
				}
			}
			else if (!(ls->sflags & L_SFLAG_US) && 
					dummy_fd[i][1] == LM_BAD_SOCKET)
			{
				dummy_fd[i][1] = dup(1);
			}
		}
/*
 *			If we are going to get a new server connection,
 *			close a pair of the dummy file descriptors.
 */
		if (ls_new_server)
		{
		    tot_serv -= 2;    /* Don't reserve these, either */
		    for (i = 0; i < MAX_SERVERS; i++)
		    {
				if (dummy_fd[i][0] != LM_BAD_SOCKET)
				{
					(void) network_close(dummy_fd[i][0]);
					dummy_fd[i][0] = LM_BAD_SOCKET;
					if (dummy_fd[i][1] != LM_BAD_SOCKET)
					{
						(void) network_close(dummy_fd[i][1]);
						dummy_fd[i][1] = LM_BAD_SOCKET;
					}
					break;
				}
		    }
		}
#endif /* NO_REDUNDANT_SERVER */			
/*
 *		Make sure we still hold the lock for this node.
 */
				
		if (timer_expired) /* once every TIMERSECS */
		{

			checklock_err (ls_checklock());
					
			if (l_midnight(currtime))
			{
				ls_pooled_check(ls_flist);
			}
/* #ifdef SUPPORT_UDP  ls_all_clients and ls_ck_udp_clients check tcp_timeout */
			ls_all_clients(ls_ck_udp_clients, &currtime);
			ls_ck_udp_clients(0,0); /*does actual delete*/
/* #endif  SUPPORT_UDP */
			f_remove_all_old(1);
		}
		currtime = ls_currtime;
#ifndef VMS
		check_baddate(currtime);
#endif



		MASK_COPY(ready_mask, select_mask);

		if (ANY_SET(ready_mask) == 0) 
		{
			LOG((lmtext("lm_server: lost all connections\n")));
			LOG_INFO((CONFIG, "This message is logged when all the \
				connections to a daemon are lost, which often \
				indicates a more general network problem."));
			(void) fflush(stderr);
			(void) lm_sleep(1);
			break;
		}
		timer_expired = 0;
		if (!_ls_going_down)	/*- Possible timing window: we could
					    take the signal between the time
					    we check _ls_going_down and when
					    the select call actually gets into 
					    the system call.  It's not fatal,
					    so we won't worry about it */
		{
			/*
			 *	Since there will not be timer signals which
			 *	can interrupt execution out of select(),
			 *	select() must timeout by itself.
			 */
			struct timeval one_minute;
#ifdef PC
			int lm_nofile=0;
#endif
			one_minute.tv_sec = TIMERSECS;
			one_minute.tv_usec = 0;		
		
			if (!(nrdy = l_select(lm_nofile, ready_mask, 
					write_mask,  0, &one_minute )))
			{
				continue ;
			}

			if (!_ls_going_down)
				ls_process(ready_mask, nrdy, select_mask, 
					    &tcp_s, &spx_s, master_name, 
						tot_serv - num_serv);
		}
	}
} 

#ifndef NO_REDUNDANT_SERVER
/*
 *	tell_them() - Tells the other servers we are the
 *		master.  If they don't agree, we exit, since
 *		this was decided at the master server level.
 */

static
void
tell_them()
{
	LM_SERVER *ls;
	char *hostname = ls_s_master();
	char *thismaster;

/*
 *	Make sure we can all agree on who is the master
 */

	for (ls = ls_s_first(); ls; ls = ls->next)
	{
	    if (!(ls->sflags & L_SFLAG_US) && (ls->state & C_CONNECTED))
	    {
			thismaster = ls_i_master(ls, hostname);
			if (thismaster && *thismaster)
			{
	/*
	 *			Bad news.  Someone else thought the master
	 *			wasn't us.  Time to exit and let the top-level
	 *			master figure this one out.
	 */
				LOG((lmtext("ERROR: %s thought %s was master\n"), 
								ls->name, thismaster));
				LOG_INFO((INFORM, "Another daemon (%s) thought that \
						%s was master, whereas we think that \
						we are.  This vendor daemon will exit  \
						and let the license server sort things \
						out."));
				ls_go_down(EXIT_BICKER);
			}
	    }
	}
}
#endif /* NO_REDUNDANT_SERVER */

void
ls_mask_clear(int fd)
{
	if ((fd != LM_BAD_SOCKET) && select_mask)
	{
		MASK_CLEAR(select_mask, (unsigned)fd);
	}
}

static
void
checklock_err(status)
int status;
{
	extern char *ls_user_lockfile;

	switch(status)
	{
	case 0:	return;
	case 1:
		LOG((lmtext("Error closing lock file, errno: %d\n"), errno));
		ls_go_down(EXIT_SERVERRUNNING);
		break;
	case 2:
		LOG((lmtext("Unable to re-open lock file (%s): %s\n"),
				ls_user_lockfile, SYS_ERRLIST(errno)));
		LOG_INFO((INFORM, "The vendor daemon cannot \
				re-open its lock file.  This \
				is either due to another copy of \
				the daemon running on the node, or \
				an incorrect \"umask\" for the user \
				who started the daemon."));
		ls_go_down(EXIT_SERVERRUNNING);
		break;
	case 3:
		LOG((lmtext("lost lock, exiting (errno: %d)\n"), errno));
		LOG_INFO((INFORM, "The vendor daemon cannot \
					flock its lock file.  This \
					is usually due to another copy of \
					the daemon running on the node.  \
					Locate the other daemon with \
					\"ps ax\" and kill \
					it with \"kill -9\"."));
		ls_go_down(EXIT_SERVERRUNNING);
		break;
	case 4:
		LOG((lmtext("lost semaphore lock, exiting (errno: %d)\n"), errno));
		ls_unlock(1);		/* Get rid of the lock file */
		ls_go_down(EXIT_SERVERRUNNING);
		break;
		
	}
}
/*
 *	check_baddate - returns 0 on success or <>0 on failure
 */
#ifndef VMS
static
void
check_baddate(currtime)
int currtime;
{
  static long called_baddate;
	if (ls_currtime - called_baddate > (60*60*24))
	{
		ls_baddate_detected = l_baddate(lm_job);
		called_baddate = ls_currtime;
	}
	else
	{
		if ((ls_currtime < ls_last_baddate_check) &&
			/* P2391 */
			(ls_last_baddate_check - ls_currtime > (60*60*24))) 
		{
			ls_baddate_detected = 1;
			ls_last_baddate_check = ls_currtime; /* reset */
			LOG((lmtext("System clock has been altered\n")));
			if(l_flexEventLogIsEnabled())
			{

				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LMGRD,
									MSG_FLEXLM_VENDOR_CLOCK_ALTERED,
									0,
									NULL,
									0,
									NULL);
			}
		}
/* 
 *		If the date WAS bad, and the date seems to
 *		have been set forward again, run l_baddate()
 *		to retest and reset if necessary
 */
		else if (ls_baddate_detected && 
			((ls_currtime - ls_last_baddate_check) 
					> 360))
		{
			ls_baddate_detected = l_baddate(lm_job);
			called_baddate = ls_currtime;
		}
	}
	ls_last_baddate_check = ls_currtime;
	if (ls_baddate_detected)
		LOG((lmtext("System clock setback detected\n")));
}
#endif
