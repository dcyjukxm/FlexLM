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
 *	Module: $Id: demo.c,v 1.44 2003/05/15 23:09:22 jwong Exp $
 *
 *	Function:  None
 *
 *	Description: Vendor customization data for server.
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  9/24/98
 *
 */

#include <stdio.h>
#include "lmclient.h"
#include "code.h"
#include "testsuite.h"
#include "l_privat.h"

VENDORCODE vendorkeys[] = {		/* Possible keys for vendor daemons */
		{ VENDORCODE_6, 
		ENCRYPTION_SEED1^VENDOR_KEY5, ENCRYPTION_SEED2^VENDOR_KEY5,
		VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, 
			FLEXLM_VERSION, FLEXLM_REVISION,
			FLEXLM_PATCH, LM_BEHAVIOR_CURRENT, {CRO_KEY1, CRO_KEY2},
		  LM_STRENGTH, LM_SIGN_LEVEL, 0
			},
		    };	/* End of vendor keys */

int keysize = sizeof(vendorkeys)/sizeof(vendorkeys[0]);
int ls_do_checkroot = 0;

/* Vendor encryption routine */

char *(*ls_user_crypt)() = 0;

/* Vendor initialization routines */

void (*ls_user_init1)() = TS_PV x_flexlm_newid;
void (*ls_user_init2)() = 0;
void (*ls_user_init3)() = 0;

/* Checkout, checkin filters, checkin callback */

extern int outfilter();
int (*ls_outfilter)() = outfilter;
int (*ls_infilter)() = 0;
int (*ls_incallback)() = 0;

/* vendor message, challenge */

extern char *ts_vmsg();
char *(*ls_vendor_msg)() = ts_vmsg;
char *(*ls_vendor_challenge)() = 0;

int ls_crypt_case_sensitive = 0; /* Is encryption code case-sensitive? -
					Only used for vendor-supplied 
					encryption routines. */

char *ls_user_lockfile = (char *)NULL;
				/* Lock file to prevent multiple copies of 
				   the daemon - default: 
					/usr/tmp/lock"DAEMON" (non-apollo)
					/sys/node_data/tmp/lock"DAEMON" (apollo)
				*/
#ifdef PC
#ifdef RED_SERVER /* for redundant server testsuite: 3serv.cmd, red_s_tests.exe*/
char * customcallback()
{
	return (getenv("DEBUG_LOCKFILE"));
}
char *(*ls_user_lock)() = customcallback;
#else
char *(*ls_user_lock)() = NULL;	/* callback to allow multiple semaphore names
					 * to be locked out at one time. 
					*/
#endif
#endif 

int ls_read_wait = 10;		/* How long to wait for solicited reads */
int ls_dump_send_data = 0;	/* Set to non-zero value for debug output */
int ls_normal_hostid = 1;	/* <> 0 -> normal hostid call, 0 -> extended */
int ls_conn_timeout = MASTER_WAIT;  /* How long to wait for a connection */
int ls_enforce_startdate = 1;	/* Enforce start date in features */
int ls_tell_startdate = 1;	/* Tell the user if it is earlier than start
								date */
int ls_minimum_user_timeout = 10; /* Minimum user inactivity timeout (seconds) 
					<= 0 -> activity timeout disabled */
int ls_min_lmremove = 5;	/* Minimum amount of time (seconds) that a
				   client must be connected to the daemon before
				   an "lmremove" command will work. */
int ls_use_featset = 0;		/* Use the FEATURESET line from the license
									file */
char *ls_dup_sel = LM_COUNT_DUP_STRING;  /* Duplicate selection criteria
						to emulate count_duplicates for 
						pre-v2 clients */
int ls_use_all_feature_lines = 0; /* Use ALL copies of feature lines that are
				     unique if non-zero (ADDITIVE licenses) */
int ls_show_vendor_def = 1;
int ls_allow_borrow = 0;        /* Allow "Borrowing" of licenses by another
				   server */
int (*ls_hostid_redirect_verify)() = 0;	/* Hostid redirection verification */

void (*ls_daemon_periodic)() = 0;	/* Periodic routine in daemon */
void (*ls_child_exited)() = 0;


int ls_compare_vendor_on_increment = 0;	/* Compare vendor-defined fields to
					 declare matches for INCREMENT lines? */
int ls_compare_vendor_on_upgrade = 0;	/* Compare vendor-defined fields to
					   declare matches for UPGRADE lines? */

char * ls_a_behavior_ver = 0;/* like LM_A_BEHAVIOR_VER */
int ls_a_check_baddate;			/* like LM_A_CHECK_BADDATE */
int ls_a_lkey_start_date;			
int ls_a_lkey_long;			
int ls_a_license_case_sensitive;

int ls_hud_hostid_case_sensitive = 0; 	/* Forces hostid checks for hostname,     */
					/* user, or display to be case sensitive. */

void (*ls_a_user_crypt_filter)() = 0;	/*- like LM_A_USER_CRYPT_FILTER */
L_KEY_FILTER kf =
	{
		(char *)ts_ds_app,
		0,
		2, 
		0, 
		0
	};


char *ls_a_key_filter = 0; /*(char *) &kf;*/

#ifdef VMS
char *default_license_file = (char *) NULL;	
					/* Default location for license file */
#endif
#ifdef OS2
extern void main();
void (*drag_in_main_from_lib)()=main;
#endif

void (*ls_a_phase2_app)() = 0;
int ls_user_based_reread_delay = 20 ; /* 20 seconds for testing */

int ls_borrow_return_early = 1;

void (*ls_borrow_out)(char * szBorrowData, int iSize)  = NULL;
void (*ls_borrow_in)(char * szBorrowData, int iSize) = NULL;
void (*ls_borrow_init)(char ** pszBorrowBuffer, int * piSize) = NULL;
