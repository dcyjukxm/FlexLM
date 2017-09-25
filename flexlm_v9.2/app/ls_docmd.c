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
 *	Module: $Id: ls_docmd.c,v 1.65 2003/06/11 00:29:44 sluu Exp $
 *
 *	Function: ls_docmd(cmd, thisuser, select_mask)
 *
 *	Description: Processes the command that was just received by the server.
 *
 *	Parameters:	(char *) cmd - The command data
 *			(CLIENT_DATA *) thisuser - The user structures
 *			(SELECT_MASK) select_mask - daemon fd select mask
 *
 *	Return:		response is returned directly to the client.
 *			(SELECT_MASK *) select_mask - updated for open/close
 *
 *	Side effects:	server state is updated for any changes
 *			in the server connections.
 *
 *	M. Christiano
 *	2/27/88
 *
 *	Last changed:  9/30/98
 *
 */

#include <stdio.h>
#include <errno.h>
#ifdef WINNT
#include <io.h>
#endif
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "lmselect.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "ls_comm.h"	/* Server-server comm */
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "lssignal.h"
#include "ls_log.h"
#include "flexevent.h"

extern char *(*ls_vendor_msg)();
extern char *(*ls_vendor_challenge)();
extern GROUP *groups;
extern GROUP *hostgroups;
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif

extern FILE *ls_log_repfile;
extern int ls_allow_lmremove;
extern int ls_borrow_return_early;
extern int ls_got_opt_max;
int ls_borrow_minutes_is_seconds; /* used to make testing go faster */
static CONFIG * get_config 	lm_args((char *, char *));
static void clocksetting 	lm_args((char *, char *, CLIENT_DATA *));
static void add_groups 		lm_args((CLIENT_DATA *));
static void add_1_group 	lm_args(( GROUP **, char *name));
static void port_host_plus_msg	lm_args((char *, char *, char *, CLIENT_DATA *));
static void strip_line_continuation lm_args((char *));
int ls_new_server = 0;	/* New server is going to connect */
int unpriv_utils = 1;	/* Unpriviliged utilities */
static int vermismatch_notreported = 1;	/* Only report version mismatch once */

#ifdef PC

typedef void (LM_CALLBACK_TYPE * LS_CB_USER_INIT1) (void);
#define LS_CB_USER_INIT1_TYPE (LS_CB_USER_INIT1)

typedef void (LM_CALLBACK_TYPE * LS_CB_USER_INIT2) (void);
#define LS_CB_USER_INIT2_TYPE (LS_CB_USER_INIT2)


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

#define  LS_CB_USER_INIT1_TYPE
#define  LS_CB_USER_INIT2_TYPE
#define	 LS_CB_OUTFILTER_TYPE
#define	 LS_CB_INCALLBACK_TYPE
#define	 LS_CB_VENDOR_MSG_TYPE
#define	 LS_CB_VENDOR_CHALLENGE_TYPE
#define	 LS_CB_DAEMON_PERIODIC_TYPE
#define	 LS_CB_HOSTID_REDIR_VERIFY_TYPE
#endif /* PC */

#ifdef NO_LMGRD
char * getdlist(void);
char * getlf(char *what);
#endif

/*
 *	Variables for do_borrow
 */

extern int dummy_fd[MAX_SERVERS][2];	/* File descriptors that hold a space
					   for servers that haven't connected
					   yet.  We close these when
					   calling ls_hookup(), so that
					   the hookup process can complete. */

#ifdef WINNT
#pragma optimize("",off)
#endif

void
ls_docmd(char *			cmd,
		 CLIENT_DATA *	thisuser,
		 SELECT_MASK	select_mask)
{
	int rc;
	char msg[LM_MSG_LEN+1];	/* For ls_client_send */
	extern VENDORCODE *ls_keys;
	extern int ls_flexlmd;
	int pre_v3 = 0;


	memset(msg, 0, sizeof(msg));
/*
 *	If we are a top-level vendor daemon and we either don't have a
 *	quorum, OR we are trying to accept a connection from another
 *	vendor daemon, don't process any commands from clients.
 */
	if ((ls_new_server|| !ls_s_havequorum()) &&
			((cmd[MSG_CMD] == LM_HELLO) ||
			 (cmd[MSG_CMD] == LM_CHECKOUT) ||
			 (cmd[MSG_CMD] == LM_CHECKIN) ||
			 (cmd[MSG_CMD] == LM_LIST) ||
			 (cmd[MSG_CMD] == LM_VENDOR_DAEMON_INFO) ||
			 (cmd[MSG_CMD] == LM_HEARTBEAT) ||
			 (cmd[MSG_CMD] == LM_TELLME) ) )
	{
		if (ls_new_server)
		{
			ls_client_send(thisuser, LM_BUSY_NEW, msg);
		}
		else
		{
			ls_client_send(thisuser, LM_BUSY, msg);
		}
	}
	else
	{
		switch(cmd[MSG_CMD])
		{
			case LM_CHECKIN:
				rc = ls_checkin(thisuser, cmd);
				break;

			case LM_CHECKOUT:
				{
					char *dup_sel = &cmd[MSG_CO_DUP_SEL];
					char *linger = "0";
					char code[MAX_CRYPT_LEN + 1];
					char *vendor_data = &cmd[MSG_CO_VENDOR_DEF];
					char *hostid_asc = &cmd[MSG_CO_HOSTID];
					HOSTID *hostid = 0;
					int port_at_host_plus;
					char borrow_seconds_buf[MAX_LONG_LEN + 1];
					int msg_flags = (cmd[MSG_CO_WAIT] & MSG_CO_WAIT_FLAGS);
					int usevendor = 1;


					int	bLogUltimateDenial = 0;
					if(cmd[MSG_CO_ULTIMATE_DENIAL_ON_FAILURE])
					{
						bLogUltimateDenial = 1;
					}
					if (!(cmd[MSG_CO_FLAGS2] & MSG_CO_FLAGS2_BORROW))
						linger = &cmd[MSG_CO_LINGER];

					cmd[MSG_CO_WAIT] &= ~MSG_CO_WAIT_FLAGS;
					if (cmd[MSG_CO_FLAGS2] & MSG_CO_FLAGS2_BORROW)
					{
						DLOG(("Linger-based Borrowing\n"));
						l_decode_int(&cmd[MSG_CO_LINGER],
							(int *)&thisuser->borrow_seconds);
					}
					if (thisuser->borrow_seconds)  /* linger-based */
					{
						extern int (*L_NEW_JOB)();
							/* not supported by this vendor */
						if ((lm_job->attrs[REAL_KEYS] == REAL_KEYS_VAL) &&
									!((*L_NEW_JOB)(0, 0, 2, 0, 0, 0)))
						{
							LM_SET_ERRNO(lm_job, LM_NOBORROWSUPP, 562, 0);

							LOG(("%s@%s: %s\n", thisuser->name,
								thisuser->node,
								lc_errstring(lm_job)));

							l_encode_int(&msg[MSG_DATA], LM_NOBORROWSUPP);
							ls_client_send(thisuser, LM_WHAT, msg);
							break;
						}
		/*
		 *				Note:  ls_borrow_minutes_is_seconds
		 *				is used to make the testsuite run
		 *				in a reasonable timeframe
		 */
						if (!L_STREQ(lm_job->vendor, "demo"))
							ls_borrow_minutes_is_seconds = 0;
						if (ls_borrow_minutes_is_seconds)
							thisuser->borrow_seconds /= 60;

						sprintf(borrow_seconds_buf, "%d",
								thisuser->borrow_seconds);
						linger = borrow_seconds_buf;
						/* Now it behaves just like a linger */
					}

					if (thisuser->flexlm_ver >= 7)
						l_decode_16bit_packed(&cmd[MSG_CO_SERNUM],
								&thisuser->ckout_sernum);

					l_zcp(code, &cmd[MSG_CO_CODE], MAX_CRYPT_LEN);

					port_at_host_plus = (msg_flags &  /* P6047 */
						MSG_CO_WAIT_FLAG_PORT_HOST_PLUS) ||
						!strcmp(code, CONFIG_PORT_HOST_PLUS_CODE);

					if (thisuser->comm_version < 2)
					{
						if (thisuser->comm_revision < 1)
						{
							dup_sel = ls_dup_sel;
						}
						if (thisuser->comm_revision < 2)
						{
							linger = "0";
							*code = 0;
						}
						if (thisuser->comm_revision < 3)
						{
							vendor_data = "";
							usevendor = 0;
						}
					}

		/*
		 *			convert the hostid from the checkout message to
		 *			hostid struct
		 */
					if (!thisuser->hostids &&
						l_get_id(lm_job, &hostid, hostid_asc)
							&& !*thisuser->platform)
					{
						thisuser->flexlm_ver = 0;
						pre_v3 = 1;
					}

		/*
		 *			Init CLIENT_DATA's list of hostids with this
		 *			hostid
		 */
					if (hostid)
						ls_client_add_hostid(thisuser, hostid);
		  			if (cmd[MSG_CO_FLAGS])
					{
						thisuser->L_CLIENT_KEY_REQ =
								cmd[MSG_CO_FLAGS] >> 1;
						/*DLOG(("user %s requires key %d\n",
							thisuser->name,
								thisuser->L_CLIENT_KEY_REQ));*/

						if (cmd[MSG_CO_FLAGS] & MSG_CO_HS_CB)
							thisuser->flags |= L_CLIENT_FLAG_HS_CB;
					}



					(void) strncpy(thisuser->vendor_def,
							vendor_data,
							MAX_VENDOR_CHECKOUT_DATA);
					thisuser->vendor_def[MAX_VENDOR_CHECKOUT_DATA]
									= '\0';
					rc = ls_checkout(&cmd[MSG_CO_FEATURE],
								&cmd[MSG_CO_N],
								&cmd[MSG_CO_WAIT],
								thisuser,
								&cmd[MSG_CO_VERSION],
								dup_sel, linger,
								code, 0, 1);

		/*
		 *			rc == 0 => checkout performed
		 *			rc < 0 => no checkout
		 *			rc > 0 => process is in queue
		 */
					if (rc == 0)
					{
						if (port_at_host_plus)
						{
							port_host_plus_msg(msg, cmd, code, thisuser);
						}
						/* for development purposes only */
						if (thisuser->borrow_seconds)
							DLOG(("%s@%s borrow %s for %d seconds\n",
							thisuser->name, thisuser->node,
							&cmd[MSG_CO_FEATURE],
							thisuser->borrow_seconds));
						ls_client_send(thisuser, LM_OK, msg);
					}
					else if (rc < 0 && rc != LM_HOSTID_REQUESTED)
					{
						l_encode_int(&msg[MSG_DATA], rc);
						if(ls_log_repfile)
						{
							msg[MSG_REPORT_LOG_ENABLED] = 1;
						}
						ls_client_send(thisuser, LM_WHAT, msg);
						if(bLogUltimateDenial)
						{
							ls_log_usage(LL_LOGTO_REPORT,
									(long)0,
									(long)0,
									LL_ULTIMATE_DENIAL,
									rc,
									thisuser->name,
									thisuser->node,
									thisuser->display,
									thisuser->vendor_def,
									&cmd[MSG_CO_FEATURE],
									code,
									(LS_POOLED *)0,
									&cmd[MSG_CO_VERSION],
									atoi(&cmd[MSG_CO_N]),	/* requested num of licenses	*/
									0,	/* actual num of license	*/
									0,	/* reference time		*/
									thisuser,
									0,	/* group			*/
									0,	/* userflags			*/
									0);	/* linger seconds		*/

						}
					}
					else if (rc > 0)
					{
						strcpy(&msg[MSG_DATA], code);
						ls_client_send(thisuser, LM_QUEUED, msg);
					}
				}
				break;

			case LM_CLOCKSETTING:
				clocksetting(cmd, msg, thisuser);
				break;

			case LM_HANDSHAKE:

#ifdef RELEASE_VERSION
				if (thisuser->flexlm_ver >= 5)
#endif
				{
					long client_time;
					long diff;
					l_decode_long_hex(&cmd[MSG_HAND_TIME],
						&client_time);
					if (client_time)
					{
						diff = client_time - ls_currtime;
						/* absolute value */
						if (diff < 0)
							diff = -diff;
						if (diff > 24*60*60) /* one day */
						{
							thisuser->flags |=
							CLIENT_FLAG_BADDATE_DETECTED;
						}
					}
				}
				(void) ls_shakehand(cmd, &vendorkeys[0], thisuser);
				break;

			case LM_HELLO:
			case LM_HELLO_THIS_VD:
			{
				char *s = &cmd[MSG_HEL_DAEMON];
				char v, r;
				int badver = 0;



				/* Grab the COMM version and store it here */

				thisuser->comm_version = (int) (cmd[MSG_HEL_VER] - '0');
				thisuser->comm_revision = (int) (cmd[MSG_HEL_VER+1]
										- '0');
				thisuser->flexlm_ver = cmd[MSG_HEL_FLEX_VER];
	/*
	 *			IMPORTANT:  prior to v3, this part of the
	 *			message was not initialized by the client.  Therefore
	 *			this value could be garbage.  To protect against this,
	 *			we look for a flexlm_ver in a small range of values.
	 */
				if (thisuser->flexlm_ver != FLEXLM_DEV_VER &&
					(thisuser->flexlm_ver < 5 ||
							thisuser->flexlm_ver > 15 ))
				{
					thisuser->flexlm_ver = 0;
					pre_v3 = 1;
				}
				else
					thisuser->flexlm_rev = cmd[MSG_HEL_FLEX_REV];

	/*
	 *			default to max timeout for pre v5.1 clients
	 */
				thisuser->tcp_timeout = 15300; /* maximum timeout */

				if (thisuser->addr.transport == LM_UDP)
					l_decode_int(&cmd[MSG_HEL_UDP_TIMEOUT],
							&thisuser->udp_timeout);
				else
					if (!pre_v3 && (thisuser->flexlm_ver >=6 ||
						(thisuser->flexlm_ver ==5 &&
						thisuser->flexlm_rev >= 1 )))
				{
					/* added v5.1 */
					sscanf(&cmd[MSG_HEL_TCP_TIMEOUT], "%x",	/* overrun checked */
							&thisuser->tcp_timeout);
	/*
	 *				Message was sent in 3-minute increments
	 */
					if (L_STREQ(lm_job->vendor, "demo") &&
						(thisuser->tcp_timeout == 1))
						; /* for testing purposes */
					else
						thisuser->tcp_timeout *=
							LM_TCP_TIMEOUT_INCREMENT;
				}

				r = cmd[MSG_HEL_VER+1];
	   			if ((v = cmd[MSG_HEL_VER]) != COMM_VERSION ||
			    				r != COMM_REVISION)
				{
					/*
					 *	sluu: Not very descriptive but what do you want
					 *	in the time given to do this?
					 */
					char	szV[2] = {'\0'};
					char	szR[2] = {'\0'};
					char	szCommVer[2] = {'\0'};
					char	szCommRev[2] = {'\0'};
					char *	ppszInStr[20] = {NULL};

					if (vermismatch_notreported)
					{
						vermismatch_notreported = 0;
						DLOG((lmtext("WARNING: Client/server comm version mismatch (Client: %c.%c, server: %c.%c)\n"),
							v, r, COMM_VERSION, COMM_REVISION));
						LOG_INFO((INFORM, "The client and server \
							programs are running (potentially) \
							incompatible versions of FLEXlm comm \
							software.  It is best to get \
							compatible versions installed."));
						if(l_flexEventLogIsEnabled())
						{
							sprintf(szV, "%c", v);
							sprintf(szR, "%c", r);
							sprintf(szCommVer, "%c", COMM_VERSION);
							sprintf(szCommRev, "%c", COMM_REVISION);
							ppszInStr[0] = szV;
							ppszInStr[1] = szR;
							ppszInStr[2] = szCommVer;
							ppszInStr[3] = szCommRev;
							
							l_flexEventLogWrite(lm_job,
												FLEXEVENT_WARN, 
												CAT_FLEXLM_NETWORK_COMM,
												MSG_FLEXLM_LMGRD_VERSION_ERROR,
												4,
												ppszInStr,
												0,
												NULL);

						}
					}
					if ((v > COMM_VERSION) ||
						(v == COMM_VERSION && r > COMM_REVISION) ||
						(v < LAST_COMPATIBLE_VER) ||
						(v == LAST_COMPATIBLE_VER &&
						r < LAST_COMPATIBLE_REV))
					{
						DLOG((lmtext("WARNING: Client/server comm version mismatch (Client: %c.%c, server: %c.%c)\n"),
							v, r, COMM_VERSION, COMM_REVISION));

						if(l_flexEventLogIsEnabled())
						{
							sprintf(szV, "%c", v);
							sprintf(szR, "%c", r);
							sprintf(szCommVer, "%c", COMM_VERSION);
							sprintf(szCommRev, "%c", COMM_REVISION);
							ppszInStr[0] = szV;
							ppszInStr[1] = szR;
							ppszInStr[2] = szCommVer;
							ppszInStr[3] = szCommRev;
								
							l_flexEventLogWrite(lm_job,
												FLEXEVENT_WARN,
												CAT_FLEXLM_NETWORK_COMM,
												MSG_FLEXLM_LMGRD_VERSION_ERROR,
												4,
												ppszInStr,
												0,
												NULL);
						}
						badver = 1;
					}
				}
	#ifdef NLM
				/*
 				 *	The fact that NLM does not have lmgrd causes
				 *	the client attempt to shutdown or lmstat to
				 * 	connect directly to the vendor daemon.  In
				 *	those connect LM_HELLP msgs, MSG_HEL_DAEMON
				 *	field has "" as vendor name.
				 */
				if (!badver)
	#else
				if (!badver && (ls_flexlmd ||
						!strcmp(s, lm_job->vendor) ))
	#endif
				{
					l_encode_int(&msg[MSG_OK_VER],
						lm_job->code.flexlm_version);
					l_encode_int(&msg[MSG_OK_REV],
						lm_job->code.flexlm_revision);
					(void) memcpy(&msg[MSG_OK_PATCH],
						lm_job->code.flexlm_patch, 1);
					msg[MSG_OK_COMM_VER] = COMM_VERSION;
					msg[MSG_OK_COMM_VER+1] = '\0';
					msg[MSG_OK_COMM_REV] = COMM_REVISION;
					msg[MSG_OK_COMM_REV+1] = '\0';
					strcpy(&msg[MSG_OK_DAEMON], ls_flexlmd ?
							"flexlmd" : lm_job->vendor);
					ls_client_send(thisuser, LM_OK, msg);
					(void) strncpy(thisuser->name,
							&cmd[MSG_HEL_NAME], MAX_USER_NAME);	/* LONGNAMES */
					thisuser->name[MAX_USER_NAME] = '\0';		/* LONGNAMES */
					(void) strncpy(thisuser->node,
							&cmd[MSG_HEL_HOST],
								MAX_SERVER_NAME);			/* LONGNAMES */
					thisuser->node[MAX_SERVER_NAME] = '\0';/* LONGNAMES */
					(void) strncpy(thisuser->platform,
							&cmd[MSG_HEL_PLATFORM],
								MAX_PLATFORM_NAME);
					thisuser->platform[MAX_PLATFORM_NAME] = '\0';

					if (thisuser->comm_revision == 0)
					{
						(void) strcpy(thisuser->display, "");
					}
					else
					{
						(void) strncpy(thisuser->display,
							&cmd[MSG_HEL_DISPLAY],
								MAX_DISPLAY_NAME);	/* LONGNAMES */
						thisuser->display[MAX_DISPLAY_NAME] =
									'\0';
					}
	/*
	 *				If client is pre v3.1, this will be 0
	 */
					l_decode_long(&cmd[MSG_HEL_PID], &thisuser->pid);
					if (ls_got_opt_max) add_groups(thisuser);
				}
				else
				{
					ls_client_send(thisuser, LM_WHAT, msg);
				}
				break;
			}

			case LM_HOSTID:
				{
					char hostid_asc[MAX_HOSTID_LEN + 1];
					HOSTID *hostid = NULL;
					long type;
					l_zcp(hostid_asc, &cmd[MSG_HOSTID],
								MAX_HOSTID_LEN);
					l_decode_long(&cmd[MSG_HOSTID_TYPE], &type);
					if (L_STREQ(hostid_asc, "NOMORE"))
					{
						hostid = l_new_hostid();
						hostid->type = type;
						strcpy(hostid->id.string, "NOMORE");
					}
					else
						l_get_id(lm_job, &hostid, hostid_asc);
					if (!hostid)
					{
						hostid = l_new_hostid();
						hostid->type = type;
					}
					else if (type != hostid->type)
					{
						hostid->type = type;
					}
					ls_client_add_hostid(thisuser, hostid);

					ls_client_send(thisuser, LM_OK, msg);
				}

				break;

			case LM_REMOVE:
	/*
	 *			Remove a user/host pair from the specified feature.
	 */
				if ((unpriv_utils || lm_isadmin(&thisuser->name[0])) &&
						ls_allow_lmremove)
				{
					CLIENT_DATA junk;
					int			iForce = 0;

					(void) bzero((char *) &junk,
							sizeof(CLIENT_DATA));
					(void) strncpy(junk.name, &cmd[MSG_REMOVE_USER],
								MAX_USER_NAME);	/* LONGNAMES */
					(void) strncpy(junk.node, &cmd[MSG_REMOVE_HOST],
								MAX_SERVER_NAME);/* LONGNAMES */
					(void) strncpy(junk.display,
							&cmd[MSG_REMOVE_DISP],
								MAX_DISPLAY_NAME);/* LONGNAMES */
					iForce = cmd[MSG_REMOVE_FORCE];
					junk.handle = LM_LINGERED_HANDLE_VALUE;
					rc = f_user_remove(
							&cmd[MSG_REMOVE_FEAT],
							&junk, 1, iForce);
					junk.handle = 0;
					if (rc == 1)
					{
						ls_client_send(thisuser, LM_OK, msg);
					}
					else if (rc == -1)
					{	ls_client_send(thisuser, LM_TOO_SOON,
										msg);
					}
					else if (rc == -2 && thisuser->flexlm_ver >= 6)
					{
	/*
	 *					It's not queued, but this code,
	 *					in response to remove, means
	 *					LM_REMOVE_LINGER
	 */
						ls_client_send(thisuser, LM_QUEUED,
										msg);
					}
					else if (rc == -3)
					{
						ls_client_send(thisuser, LM_NO_RETURN_EARLY, msg);
					}
					else
					{	ls_client_send(thisuser, LM_NO_USER,
									msg);
					}
				}
				else if (!ls_allow_lmremove)
				{
					rc = 0;
					LOG((lmtext("LMREMOVE not allowed\n")));
					ls_client_send(thisuser, LM_NO_SUCH_FEATURE, msg);
				}
				else
				{
					rc = 0;
					LOG((lmtext("UNAUTHORIZED lmremove request from %s at node %s\n"),
							&thisuser->name[0],
							&thisuser->node[0]));
					ls_client_send(thisuser, LM_NOT_ADMIN, msg);
				}
				break;

			case LM_REMOVEH:
	/*
	 *			Remove a user/host pair from the specified feature.
	 */

		/* Fix for P6639: lmgrd -x lmremove will now work when running lmremove with handle */
				if ((unpriv_utils || lm_isadmin(&thisuser->name[0])) &&
						ls_allow_lmremove)
				{
					int		iForce = 0;

					iForce = cmd[MSG_REMOVEH_FORCE];

					rc = f_user_remove_handle(
						   &cmd[MSG_REMOVEH_FEAT],
						   &cmd[MSG_REMOVEH_HANDLE], 1, iForce);
					if(rc == 1)
					{
						ls_client_send(thisuser, LM_OK, msg);
					}
					else if(rc == -1)
					{
						ls_client_send(thisuser, LM_TOO_SOON, msg);
					}
					else if(rc == -2)
					{
						ls_client_send(thisuser, LM_QUEUED, msg);
					}
					else
					{
						ls_client_send(thisuser, LM_NO_USER, msg);
					}
				}
				else
				{
					rc = 0;
					LOG((lmtext("UNAUTHORIZED lmremove request from %s at node %s\n"),
							&thisuser->name[0],
							&thisuser->node[0]));
					ls_client_send(thisuser, LM_NOT_ADMIN, msg);
				}
				break;


			case LM_HEARTBEAT:
				{
					int i[8], j;
					extern int ls_cpu_usage_interval;
					extern int ls_cpu_usage_last_logged;
					if (thisuser->flexlm_ver >= 5)
					{
						sscanf(&cmd[MSG_HB_CPU_USAGE_1], "%x", /* overrun checked */
							&i[0]);
						sscanf(&cmd[MSG_HB_CPU_USAGE_2], "%x", /* overrun checked */
							&i[1]);
						sscanf(&cmd[MSG_HB_CPU_USAGE_3], "%x", /* overrun checked */
							&i[2]);
						sscanf(&cmd[MSG_HB_CPU_USAGE_4], "%x", /* overrun checked */
							&i[3]);
						for (j=0;j<4;j++)
							thisuser->curr_cpu_usage[j] = i[j];
	/*
	 *					ls_cpu_usage_interval is in
	 *					minutes, so we * 60.
	 */
						if ((ls_currtime -
					 		   ls_cpu_usage_last_logged) >=
							(ls_cpu_usage_interval * 60))
						{
							ls_log_cpu_usage(thisuser);
							ls_cpu_usage_last_logged =
									ls_currtime;
						}
					}
					ls_client_send(thisuser,
						LM_HEARTBEAT_RESP, msg);
				}
				break;

			case LM_LIST:

				ls_list(cmd, thisuser);
				break;

			case LM_SHUTDOWN:
	/*
	 *			NOTE:  MSG_HEL_VER is used a flag
	 *			To indicate that -force was not used
	 *			in lmdown -- so we enforce the borrowing lmdown
	 *			restrictions.
	 */
				if (cmd[MSG_HEL_VER] == 1)
				{

					DLOG(("checking borrowed\n"));
					if (f_borrowed())
					{
	/*
	 *					Extra restrictions on lmdown if
	 *					licenses are currently linger-borrowed
	 */
						ls_client_send(thisuser, LM_BUSY, msg);
						return;
					}
				}
				else
				{
					DLOG(("NOT checking borrowed\n"));
				}

				ls_client_send(thisuser, LM_OK, msg);
							ls_down(&(thisuser->addr.addr.fd), "shutdown");
				ls_exit(SHUTDOWN_SIGNAL);
				break;		/* Never get here */

			case LM_REREAD:
				ls_client_send(thisuser, LM_OK, msg);
				ls_reread();
				break;

			case LM_SWITCH:

				ls_log_reopen_ascii(&cmd[MSG_DATA]);
				LOG(("Debug log switched to (%s).\n",
							&cmd[MSG_DATA]));
				ls_print_feats();
				{
					extern FILE *ls_log_ascfile;
			  		if (ls_log_ascfile)
						ls_client_send( thisuser, LM_OK, msg);
					else
						ls_client_send( thisuser, LM_WHAT, msg);
				}
				break;

			case LM_SWITCH_REPORT:
			case LM_NEWREPLOG:

				{
					int retval = 0;
					CLIENT_DATA *toclient = (CLIENT_DATA*)0;


					if (!strcmp(&cmd[MSG_DATA],
						LM_SWITCH_REPORT_CLIENT))
						toclient = thisuser;

					/* nprakash 5/2/03: This code processes the lmswitchr command */
					if (cmd[MSG_CMD] == LM_SWITCH_REPORT)
					{
						retval = ls_log_reopen_report(
							&cmd[MSG_DATA],		/* LONGNAMES */
							toclient,
							REPLOG_SWITCH);
					/* P5671 -- send status back */
					 /* 5/2/03 nprakash P7214 bug fix: The conditional now handles the
					  * correct return value from ls_log_reopen_report().
					  */
						if (retval)
						{
							LOG(("Switched REPORTLOG to %s\n", &cmd[MSG_DATA]));
							ls_client_send(
								thisuser,
								LM_OK, msg);
						}
						else
						{
							msg[MSG_DATA] = 0;
							if (errno)
								strcpy(&msg[MSG_DATA],
								SYS_ERRLIST(errno));
							ls_client_send(
								thisuser,
								LM_WHAT, msg);
						}
					}
					/* nprakash 5/2/03: This code processes the lmnewlog command */
					else
					{
						retval = ls_log_reopen_report(
							&cmd[MSG_DATA],		/* LONGNAMES */
							toclient,
							REPLOG_NEWFILE );
						if (retval < 1)
						{
							l_encode_int(
								&msg[MSG_DATA],
								retval);
							ls_client_send(
								thisuser,
								LM_WHAT, msg);
						}
						else
							ls_client_send(
								thisuser,
								LM_OK, msg);
					}
				}
				break;

#ifdef INCLUDE_DUMP_CMD
			case LM_DUMP:
				{
					extern int ls_dump_send_data;

					switch(cmd[MSG_DATA])
					{
					case 'D':	/* DLOG on */
						LOG(("DEBUG logging turned ON, was %s\n"
							, dlog_on ? "ON" : "OFF"));
						dlog_on = 1;
						break;
					case 'd':	/* DLOG off */
						LOG(("DEBUG logging turned OFF, was %s\n",
							dlog_on ? "ON" : "OFF"));
						dlog_on = 0;
						break;
					case 'Q':
						ls_s_dump();
						break;
					case 'R':		/* Report log dump */
						ls_feat_dump();
						break;
					case 'S':	/* DUMP SEND DATA on */
						LOG(("DUMP send data turned ON, was %s\n"
							, ls_dump_send_data ? "ON" :
										"OFF"));
						ls_dump_send_data = 1;
						break;
					case 's':	/* DUMP SEND DATA off */
						LOG(("DUMP send data turned OFF, was %s\n"
							, ls_dump_send_data ? "ON" :
										"OFF"));
						ls_dump_send_data = 0;
						break;
					default:
						f_dump(stdout);
						break;
					}
				}
				break;
#endif	/* INCLUDE_DUMP_CMD */
			case LM_WHAT:
	/*
	 *		Nothing good here.  Just let this one drop on the floor, to
	 *		prevent the daemons sending WHAT! messages back and fourth.
	 */
				DLOG(("Unsolicited WHAT! message\n"));
				break;

			case LM_LOG:
				if (l_getattr(lm_job, LOG_SUPPORT) == LOG_SUPPORT_VAL)
				{
					LOG(("%s", &cmd[MSG_DATA]));
				}
				break;

			case LM_VENDOR_DAEMON_INFO:
				ls_vd_info(cmd, thisuser);
				break;
#ifdef NO_LMGRD

			case LM_SEND_LF:
			{
				char *p = (char *) NULL;
				if (L_STREQ(&cmd[MSG_LF_FINDER_TYPE],
									LM_FINDER_GET_DLIST))
					p = getdlist();
				else
					p = getlf(&cmd[MSG_LF_FINDER_TYPE]);

				if (p)
				{
					ls_client_send_str(thisuser, p);
				}
				else
					ls_client_send(thisuser, LM_WHAT, msg);

				if (p) free (p);

			}
			break;
#endif /* NO_LMGRD */


			case LM_VENDOR_MSG:
			case LM_CHALLENGE:
				if ((l_getattr(lm_job, FILTERS) == FILTERS_VAL))
				{
					char resp = LM_VENDOR_RESP;
					char *(*func)() = ls_vendor_msg;

					if (cmd[MSG_CMD] == LM_CHALLENGE)
					{
						resp = LM_CHALLENGE_RESP;
						func = ls_vendor_challenge;
					}
					if (func)
					{
						char *ret;

						ret = (* LS_CB_VENDOR_MSG_TYPE
								func)(&cmd[MSG_DATA]);
							(void) strncpy(&msg[MSG_DATA], ret,
								LM_MSG_LEN - MSG_DATA);
					}
					ls_client_send(thisuser, resp, msg);

				}
				else
				{
					ls_client_send(thisuser, LM_WHAT, msg);
				}
				break;

			 default:
				{
					char *master = "";

	/*
	 *				If we are not the topdog server, then
	 *				tell the other server he has to try again,
	 *				close this file descriptor, tell topdog that
	 *				another server is trying to connect, and
	 *				pass control back up to him...
	 */

	#ifndef NO_REDUNDANT_SERVER
					{
						ls_new_server = 0;
						master = ls_s_master();
	/*
	 *				    If this is an SHELLO message, find the
	 *				    server and close down it's file
	 *				    descriptors.
	 */
						if (cmd[MSG_CMD] == LM_SHELLO)
						{
							LM_SERVER *ls;
							int i, j;

							for (i = 0, ls = ls_s_first(); ls;
									ls = ls->next, i++)
							{
								if (!strcmp(ls->name,
									&cmd[MSG_HEL_HOST]))
								{
								for (j=0; j<2; j++)
								{
									if (dummy_fd[i][j] !=
										LM_BAD_SOCKET)
									{
									(void)
									  network_close(
										dummy_fd[i][j]);
									dummy_fd[i][j] =
										LM_BAD_SOCKET;
									}
								}
								if (ls->state & C_CONNECTED)
								{
		/*
		 *						An old connection hanging
		 *						around - break it.
		 */
									ls_down(&ls->fd1,
								 "breaking server connection1");
									ls_down(&ls->fd2,
								 "breaking server connection2");
									ls->state = 0;
								}
								break;
								}
							}
						}
						if (ls_hookup(cmd, &thisuser->addr,
							master, thisuser, select_mask))
						{
						LOG((lmtext("SHELLO for wrong DAEMON, we are %s, request for %s\n"),
							   lm_job->vendor,
								&cmd[MSG_HEL_DAEMON]));
						LOG_INFO((ERROR, "This vendor daemon \
							was sent a 'SERVER HELLO' \
							message that was destined for \
							another vendor daemon."));
						ls_client_send(thisuser, LM_WHAT, msg);
						}
					}
	#endif /* NO_REDUNDANT_SERVER */
				}
				break;
			}
	}
}
#ifdef WINNT
#pragma optimize("",on)
#endif

/*
 *	add_groups -- add list of group names that this
 *			user belongs to
 */
static
void
add_groups(CLIENT_DATA * client)
{
	GROUP *g;

	for (g = groups; g; g = g->next)
	{
		if (ls_ingroup(OPT_GROUP, client, g->name))
		{
			add_1_group(&client->groups, g->name);
		}
	}
	for (g = hostgroups; g; g = g->next)
	{
		if (ls_ingroup(OPT_HOST_GROUP, client, g->name))
		{
			add_1_group(&client->hostgroups, g->name);
		}
	}
}

/*
 *	Malloc and add a group to this client
 *	NOTE:  The GROUP.list member is unused here
 */
static
void
add_1_group(GROUP **head, char *name)
{
	GROUP *g;

	g = (GROUP *)LS_MALLOC(sizeof (GROUP));
	memset(g, 0, sizeof(GROUP));
	g->name = (char *)LS_MALLOC(strlen(name) + 1);
	strcpy(g->name, name);
	g->next = *head;
	*head = g;
}

/*
 *	get_config  -- version of get_config that matches on license-key
 */
static
CONFIG *
get_config(code, feat)
char *code, *feat;
{
	CONFIG *pos = (CONFIG *)0, *ret;

	while (ret = l_next_conf_or_marker(lm_job, feat, &pos, 0, (char *)0))
	{
		if (l_keyword_eq(lm_job, code, ret->code)) /* found it */
			break;
	}
	if (!ret)
		DLOG(("INTERNAL ERROR %s %d\n", __FILE__, __LINE__));
	return ret;
}

static
void
clocksetting(cmd, msg, thisuser)
char *cmd, *msg;
CLIENT_DATA *thisuser;
{

	int x, diff, capacity;
#ifdef THREAD_SAFE_TIME
	struct tm tst;
#endif
	long t, client_t;
	int len;


	if(cmd[MSG_CLOCK_PROJECT])
	{
		thisuser->project = LS_MALLOC(
				len = (strlen(&cmd[MSG_CLOCK_PROJECT]) + 1));
		l_zcp(thisuser->project, &cmd[MSG_CLOCK_PROJECT], len - 1);
	}
	if(cmd[MSG_CLOCK_CAPACITY])
	{
		l_decode_int(&cmd[MSG_CLOCK_CAPACITY], &capacity);
		thisuser->capacity = (short)capacity;
	}
	l_decode_long(&cmd[MSG_CLOCK_VALUE], &client_t);
	/*
	 * It it's zero, we're setting other things, not timediff
	 */
	if (client_t)
	{
		l_decode_int(&cmd[MSG_CLOCK_DIFF], &diff);
#ifdef THREAD_SAFE_TIME
		l_get_date(&x, &x, &x, &t, &tst);
#else /* !THREAD_SAFE_TIME */
		l_get_date(&x, &x, &x, &t);
#endif
		t -= client_t;	/* Difference in seconds */
		t /= 60;	/* Make it minutes */
		if (t > diff || t < -diff)
		{
			LOG((lmtext(
			"Client connection from node %s denied.\n")
						, thisuser->node));
			LOG((lmtext(
				"Node %s clock setting off by more than %d minutes.\n")
					, thisuser->node, diff));
			LOG_INFO((INFORM, "The node that the specified \
			client is running on has its clock set \
			to a value inconsistent with the \
			server node."));
			ls_client_send(thisuser, LM_WHAT, msg);
		}
		else
		{
			ls_client_send(thisuser, LM_OK, msg);
		}
	}
	else
		ls_client_send(thisuser, LM_OK, msg);
}

static
void
port_host_plus_msg(
	char *msg,
	char *cmd,
	char *code,
	CLIENT_DATA *c)

{
	CONFIG *conf;
	char buf [MAX_CONFIG_LINE + 1];
	char buf2 [MAX_CONFIG_LINE + 1];
	int truncate_flag;
	int flag;

/*
 *
 *	We now send the feature line back
 *	to the client, as the "param" to the
 *	LM_OK msg.
 *	[code (license-key) has been reset
 *	in ls_checkout()]
 */
	flag = LM_FLAG_PRT_CONF_OLDEST_FMT;
	/* P5276 */
	if (c->flexlm_ver <7 || (c->flexlm_ver == 7 && c->flexlm_rev == 0))
		flag = LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN;
	memset(msg, 0, LM_MSG_LEN + 1);

/*
 *	Send FEATURE back in ascii format
 */
	*buf = 0;
	conf = get_config(code, &cmd[MSG_CO_FEATURE]);

	if (conf)
	{
		char sign[MAX_CONFIG_LINE];
		extern int ls_use_all_feature_lines;
		int type = conf->type;
		/* P5278 */
#if 0
		if (ls_use_all_feature_lines)
		{

			if ((conf->type == CONFIG_INCREMENT)
                                && (conf->lc_options_mask & LM_OPT_ISFEAT))
			{
				conf->type = CONFIG_FEATURE;
				printf("setting to feature\n");
			}
			/* P6220 */
			else if ((conf->type == CONFIG_FEATURE)
                                && !(conf->lc_options_mask & LM_OPT_ISFEAT))
			{
				conf->type = CONFIG_INCREMENT;
			}
		}
#endif

		if (conf->lc_sign)
		{
			strcpy(sign, conf->lc_sign);
		}
		lm_job->flags |= flag;
		l_print_config(lm_job, conf, buf2);
		lm_job->flags &= ~flag;
		if (conf->lc_sign)
			strcpy(conf->lc_sign, sign);
		conf->type = type;
	}
	else
		*buf2 = '\0';

	strip_line_continuation(buf2);

/*
 *	If it won't fit in LM_OK msg,
 *	truncate it, but mark it so the
 *	client knows it's only partial
 */
	truncate_flag = strlen(buf2);
	if (truncate_flag > (LM_LOG_MAX_LEN - ((MSG_DATA) + 2)))
	{
		int cnt;
		char *cp = buf2;
		char *next;
/*
 *		We truncate after the license-
 *		key -- it turns out it's always
 *		parseable FEATURE line (though
 *		it's missing stuff)
 *		-- turn the 7th space into
 *		null -- truncates to 6 fields
 */
		while (*cp == ' ')
			cp++;
		for (next = cp;  *next && ((next - &buf2[0]) < LM_MSG_LEN - MSG_DATA); )
		{
			cp = next;
			/* kmaclean Jan 15, 2003
			 * P6852
			 * The below while was: while (*next > ' ')
			 * Since char is signed on some systems this will cause the for()
			 * to loop forever if the string contains international chars. */
			while (*next != ' ')   /* step over the next work */
				next++;
			while (*next == ' ')  /* skip spaces */
				next++;
		}
		*cp = '\0';
/*
 * 		truncate_flag == 0 means FEATURE is truncated
 */
		truncate_flag = 0;

	}
	else
	{
/*
 *		Bug 1545 -- truncate_flag is used as a
 *		flag, not a real len
 */
		truncate_flag = 1;
	}
	sprintf(&msg[MSG_DATA], "%d %s", truncate_flag, buf2);
}

static
void
strip_line_continuation(char * buf)
{
	char *cp, *cp2;

	for (cp = buf; cp && *cp; cp++)
	{
		if (*cp == '\\' || *cp == '\n' || *cp == '\t' || *cp == '\r')
			*cp = ' ';
	}
	for (cp = buf; cp && *cp; cp++)
	{
		while ( (cp > buf) &&
			(cp[0] == ' ') &&
			(cp[-1] == ' '))
		{
			for (cp2 = cp; *cp2 ; cp2++)
				cp2[0] = cp2[1];
		}

	}
}
