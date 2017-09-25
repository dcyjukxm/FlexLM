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
 *	Module: $Id: lm_comm.h,v 1.28 2003/03/05 22:17:00 sluu Exp $
 *
 *	Description:	Definitions for the license manager client-server
 *				and server-server communications
 *
 *	M. Christiano
 *	2/13/88
 *
 *	Last changed:  11/16/98
 *
 */
/*
 *	The format of the messages is as follows:
 *
 *			MESSAGE_CODE "Parameter"
 *		where:
 *
 *			MESSAGE_CODE is the (char) code, and
 *			Parameter is the (message-dependent) data
 *
 *	handshake ----------- HANDSHAKE crypt_flag date
 *	hello --------------- HELLO COMM_VERSION user host display
 *	checkin ------------- CHECKIN feature code
 *	list ---------------- LIST feature
 *	tellme -------------- TELLME feature
 *	checkout ------------ CHECKOUT nnnn wait feature dup_select linger code
 *							vendor_defined
 *					- n copies of feature, wait flag "wait"
 *	server_hello -------- SHELLO COMM_VERSION host daemon
 *	vendor_daemon hello - VHELLO COMM_VERSION host daemon transport address
 *	borrow -------------- BORROW feature nnn version exp-date h1 h2 h3 h4 h5
 *	are you master------- YOU_MASTER
 *	I am master   ------- I_MASTER
 *	Master is ready ----- MASTER_READY
 *	Is he master -------- IS_MASTER host
 *	dump database ------- F_DUMP {F|Q|S} (F:feature, Q:quorum, S:daemon)
 *	slave server pid ---- PID pid
 *	daemon port # ------- DAEMON name port
 *	restart client ------ RESTART fd
 *	heartbeat  ---------- HEARTBEAT
 *	server connecting --- NEWSERVER
 *	server shutdown ----- SHUTDOWN
 *	remove user --------- REMOVE feature user host display
 *	remove user (handle)- REMOVEH feature server port handle
 *	detach socket ------- DETACH fd
 *	switch logfile ------ SWITCH newname
 *	log message --------- LOG message
 *	server order -------- ORDER nnnn host
 *	clock setting ------- CLOCK xxxx
 *	reread license file - REREAD NAME HOST DAEMON
 *	switch log file ----- SWITCH
 *	switch report file -- SWITCH_REPORT
 *	heard since d? ------ SINCE d	(returns LM_OK | LM_NO_SUCH_FEATURE)
 *	send license file --- SEND_LF COMM_VERSION user host display vendor
 *	vendor defined ------ VENDOR_MSG
 *	challenge ----------- CHALLENGE (like vendor defined)
 *	get vd info --------- VENDOR_DAEMON_INFO (subparameters 
 *					GENERIC_INFO for ls_vendor.c info
 *					FEAT_BUNDLE_1 for FEATURE_LIST info
 *
 * Responses:
 *	try_another --------- TRY_ANOTHER master port
 *	nusers -------------- NUSERS n
 *	username ------------ USERNAME user host time (+ much more)
 *	username2 ----------- USERNAME2 VENDOR_CHECKOUT_DATA
 *	OK ------------------ OK [VER REV PATCH COMM_VER COMM_REV]
 *	SERVER HELLO OK ----- SHELLO_OK
 *	WHAT? --------------- WHAT
 *	feature_available --- FEATURE_AVAILABLE
 *	no_such_feature ----- NO_SUCH_FEATURE feature
 *	no_config_file ------ NO_CONFIG_FILE
 *	server busy --------- BUSY (-> SERVBUSY)
 *	new server connecting BUSY_NEW (-> BUSYNEWSERV)
 *	heartbeat respoonse - HEARTBEAT_RESP
 *	You are in the queue- QUEUE
 *	License file data --- LF_DATA remaining_bytes license_file_data
 *	vendor response ----- VENDOR_RESP
 *	Not administrator --- NOT_ADMIN
 *	feature ------------- FEATURE <feature line info>
 *	vd ls_vendor.c info-- VD_GENERIC_INFO
 *	vd feature info ----- VD_FEAT_BUNDLE_1
 *
 */

/*
 *	Communications version/revision (for backward compatiblity
 *		between clients and servers)
 *	History:
 *		v0.9 - First release comm version.
 *		v1.0 - Updated in FLEXlm v1.07, for heartbeat msgs.
 *		v1.1 - Updated in v2.0 - many changes:
 *			Added: HANDSHAKE, LOG, ORDER, CLOCK, REREAD,
 *					SWITCH, and SWITCH_REPORT messages
 *			Added: QUEUE response
 *			HELLO message: NEW FIELD for COMMv1.1: MSG_HEL_DISPLAY	
 *			CHECKOUT MSG: NEW FIELD for COMMv1.1: MSG_CO_DUP_SEL
 *			NUSERS message: NEW fields for COMMv1.1: MSG_NUSERS_TOT,
 *							MSG_NUSERS_TIME	
 *			USERNAME response:
 *				MSG_U_NAME_v1_0	- Username@Hostname
 *				MSG_U_NUM_v1_0	- Same as v1.1 
 *				MSG_U_TIME_v1_0	- Same as v1.1
 *		v1.2 - Added LINGER and CODE to checkout message. (v2.22)
 *		v1.3 - Updated for version 3 (first changed at v2.4)
 *			Added CODE to checkin, list, and remove messages. (v2.4)
 *			Added MSG_U_HANDLE (license handle), MSG_U_LINGER
 *			Added VER/REV/PATCH/COMM_VER/COMM_REV to OK msg in
 *				response to HELLO message.
 *			Added Requested COMM to HELLO Message and 
 *						LM_COMM_UNAVAILABLE
 *			Added MSG_TRY_HOST2 (for 2nd 32 bytes of hostname)
 *			Added REMOVEH message (for lmremove with handle)
 *			Added VENDOR_MSG and VENDOR_RESP (free-format)
 *			Added CHALLENGE and CHALLENGE_RESP (free-format)
 *			Added NOT_ADMIN, TOOSOON, and NO_USER responses 
 *			Added VENDOR_DEF to CHECKOUT msg
 *			Added USERNAME2 message
 *			Added MSG_OK_HOST for return from lmgrd (for lmstat)
 *			REREAD message now uses HEL_NAME, HEL_HOST and
 *				HEL_DAEMON fields
 *		(was v1.4 in v2.51 - changed back to v1.3 in 2.52)
 *				- 2nd update for v3.0:
 *			MSG_IN_USEVENDOR/MSG_IN_VENDOR added to CHECKIN msg
 *			BORROW/BORROW2/FEATURE/FEATURE2 messages added
 */

#ifndef _LM_COMM_INCLUDED
#define	_LM_COMM_INCLUDED

#define COMM_NUMVER 1		/* Number of current version */
#define COMM_VERSION '1'	/* MUST BE the SAME as COMM_NUMVER */
#define COMM_NUMREV 3		/* Number of current revision */
#define COMM_REVISION '3'	/* MUST BE the SAME as COMM_NUMREV */
/*
 *	No COMM version < LAST_COMPATIBLE_VER.LAST_COMPATIBLE_REV or
 *			> COMM_VERSION.COMM_REVISION
 *		will work with the current server ver.rev
 */
#define LAST_COMPATIBLE_VER '1'		/* Last compatible version */
#define LAST_COMPATIBLE_REV '0'		/* Last compatible rev */

/*
 *	Historical definitions
 */
/*
 *	Comm v1.0
 */
#define LM_MSG_LEN_v1_0	88			/* V1.0 msgs are 88 bytes */
#define MSG_U_NAME_v1_0	MSG_DATA		/* Username@Hostname */
#define MSG_U_NUM_v1_0	MSG_U_NAME_v1_0 + MAX_USER_NAME+1+MAX_SERVER_NAME+1
						/* Same as v1.1 */
#define MSG_U_TIME_v1_0	MSG_U_NUM_v1_0 + 11 /* Same as v1.1 */
/*
 *	Comm v1.1
 */
#define LM_MSG_LEN_v1_1	125			/* V1.1 msgs are 125 bytes */
/*
 *	Comm v1.2
 */
#define LM_MSG_LEN_v1_2	125			/* V1.2 msgs are 125 bytes */
/*
 *	Comm v1.3
 */
#define LM_MSG_LEN_v1_3	147			/* V1.3 msgs are 147 bytes */
/*
 *	Minimum historical message length
 */
#define LM_MSG_LEN_MIN LM_MSG_LEN_v1_0

#ifndef VMS
extern int _lm_msg_size[COMM_NUMREV+2];
#endif /* !VMS */

/*
 *	End historical definitions
 */

/*
 *	Message Offsets
 */
#define MSG_CMD 	0	/* Command field */
#define MSG_CHECKSUM	1	/* Checksum byte */
/* 				   As of 3.0, for UDP clients, this
 * 				   field is packed: 4 bits each sernum/cksum
 *				 
 *					[....][....]
 *					  ^     ^
 *			    sernum  ------'      `------ checksum
 */
#define MSG_DATA	2	/* Data part of message */
#define MAX_XLONG 8
#define MAX_XLONG_LEN 8
#define MSG_QUEUED_PACKAGE	MSG_DATA+MAX_FEATURE_LEN+1
#define MSG_REPORT_LOG_ENABLED	MSG_QUEUED_PACKAGE+1

/*
 *	Data field offsets - Messages with only one data item put
 *	the data @MSG_DATA
 */

/* BORROW message */
#define MSG_BORROW_TYPE    MSG_DATA
#define LM_METER_BORROW 1
#if 0
#define MSG_BORROW_N       MSG_DATA + 1
#define MSG_BORROW_2ND     MSG_BORROW_N+6       /* Max of 99999 licenses */
#define MSG_BORROW_FEATURE MSG_BORROW_2ND+1     /* 1 byte of "2nd msg" flag */
#define MSG_BORROW_VERSION MSG_BORROW_FEATURE + MAX_FEATURE_LEN + 1
#define MSG_BORROW_EXP     MSG_BORROW_VERSION + MAX_VER_LEN + 1
#define MSG_BORROW_H1      MSG_BORROW_EXP + DATE_LEN + 1
#define MSG_BORROW_H2      MSG_BORROW_H1 + MAX_HOSTID_LEN + 1
#define _BORROW_LEN       (MSG_BORROW_H2 + MAX_HOSTID_LEN + 1)
#endif /* old borrow msg */

/* METER_BORROW message */
#define MSG_METER_BORROW_FEAT   MSG_BORROW_TYPE + 1
#define MSG_METER_BORROW_VER    MSG_BORROW_FEATURE + MAX_FEATURE_LEN
#define _METER_BORROW_LEN       MSG_BORROW_MTER_VERSION + MAX_VER_LEN + 1


/* BORROW2 message */
#define MSG_BORROW_H3 	   MSG_DATA	
#define MSG_BORROW_H4	   MSG_BORROW_H3 + MAX_HOSTID_LEN + 1
#define MSG_BORROW_H5	   MSG_BORROW_H4 + MAX_HOSTID_LEN + 1
#define _BORROW2_LEN	  (MSG_BORROW_H5 + MAX_HOSTID_LEN + 1)

/* CLOCKSETTING	message */
#define MSG_CLOCK_VALUE	MSG_DATA
#define MSG_CLOCK_DIFF	(MSG_CLOCK_VALUE + MAX_LONG_LEN + 1)
#define MSG_CLOCK_PROJECT (MSG_CLOCK_DIFF + MAX_LONG_LEN + 1)
#define MSG_CLOCK_CAPACITY (MSG_CLOCK_PROJECT + MAX_PROJECT_LEN + 1)
#define _CLOCK_LEN	(MSG_CLOCK_CAPACITY + MAX_LONG_LEN + 1)


/* CHECKOUT message */
#define MSG_CO_N 	MSG_DATA	
#define MSG_CO_WAIT 	MSG_CO_N+5		/* 4 bytes of NNNN plus null */
/* values for MSG_CO_WAIT: */
#define MSG_CO_WAIT_NO_WAIT 	'0'
#define MSG_CO_WAIT_QUEUE 	'1'
#define MSG_CO_WAIT_LOCALTEST 	'2'
/*
 *	The high 2 bits of this byte are reserved for flags 
 */
#define MSG_CO_WAIT_FLAG_PORT_HOST_PLUS (0x1 << 6) /* P6047 */
#define MSG_CO_WAIT_FLAG_UNUSED (0x2 << 6)
#define MSG_CO_WAIT_FLAGS (MSG_CO_WAIT_FLAG_PORT_HOST_PLUS |  \
			MSG_CO_WAIT_FLAG_UNUSED)
/* end values for MSG_CO_WAIT */
#define MSG_CO_FEATURE	MSG_CO_WAIT+1		/* 1 byte of wait */
#define MSG_CO_VERSION	MSG_CO_FEATURE + MAX_FEATURE_LEN + 1
#define MSG_CO_DUP_SEL	MSG_CO_VERSION + MAX_VER_LEN + 1
#define MSG_CO_LINGER	MSG_CO_DUP_SEL + MAX_LONG_LEN + 1
#define MSG_CO_CODE	MSG_CO_LINGER + MAX_LONG_LEN + 1
#define MSG_CO_VENDOR_DEF MSG_CO_CODE + MAX_CRYPT_LEN + 1
#define MSG_CO_HOSTID   MSG_CO_VENDOR_DEF + MAX_VENDOR_CHECKOUT_DATA + 1
#define MSG_CO_SERNUM   MSG_CO_HOSTID + (ETHER_LEN * 2) + 1 
#define MSG_CO_FLAGS     MSG_CO_SERNUM + 2 /* 1 byte */
#define MSG_CO_ULTIMATE_DENIAL_ON_FAILURE	MSG_CO_FLAGS + 2	/* 1 byte */
/*-
 *	The high order 7 bits are reserved for SIGNn= numbers.
 *	It's the SIGNn= that this client requires.
 *	This allows 2^7 keys, or 128 key types per ISV.
 */
#define MSG_CO_HS_CB 0x1	/*- handshake callback required */
#define MSG_CO_KEY_REQ_MASK 0xfe
#define MSG_CO_FLAGS2 MSG_CO_FLAGS + 1 /* 1 byte in binary format */
#define MSG_CO_FLAGS2_BORROW 1 /* bit 0 */

#define _CO_LEN		MSG_CO_FLAGS2 + 1

/* 
 *	FEAT_BUNDLE_1 response to VD_INFO message  -- all int's 
 */
#define MSG_FEAT_BUNDLE1_REV MSG_DATA 		
#define MSG_FEAT_BUNDLE1_TIMEOUT MSG_FEAT_BUNDLE1_REV + MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_LINGER MSG_FEAT_BUNDLE1_TIMEOUT + MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_DUP_SELECT MSG_FEAT_BUNDLE1_LINGER + MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_RES MSG_FEAT_BUNDLE1_DUP_SELECT + MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_TOT_LIC_IN_USE MSG_FEAT_BUNDLE1_RES + MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_FLOAT_IN_USE MSG_FEAT_BUNDLE1_TOT_LIC_IN_USE + \
					MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_USER_CNT MSG_FEAT_BUNDLE1_FLOAT_IN_USE + \
					MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_NUM_LIC MSG_FEAT_BUNDLE1_USER_CNT + \
					MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_QUEUE_CNT MSG_FEAT_BUNDLE1_NUM_LIC + \
					MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_OVERDRAFT MSG_FEAT_BUNDLE1_QUEUE_CNT + \
					MAX_LONG_LEN + 1
#define MSG_FEAT_BUNDLE1_BORROWED MSG_FEAT_BUNDLE1_OVERDRAFT + \
					MAX_LONG_LEN + 1
#define _FEAT_BUNDLE1_LEN MSG_FEAT_BUNDLE1_BORROWED  + 2 /* 2 bytes binary */


/* FEATURE message */
#define MSG_FEATURE_CONT	MSG_DATA		/* Name of DAEMON */
#define MSG_FEATURE_DATA	MSG_FEATURE_CONT + 1
#define _FEATURE_LEN		(MSG_FEATURE_DATA + MAX_FEATURE_LEN + 1 + MAX_DAEMON_NAME + 1 + MAX_LONG_LEN + 1 + DATE_LEN + 1 + MAX_LONG_LEN + MAX_CRYPT_LEN + 1)

/* FEATURE2 message */
#define MSG_FEATURE2_DATA	MSG_DATA		/* Name of DAEMON */
#define _FEATURE2_LEN		(MSG_FEATURE_DATA + MAX_VENDOR_DEFINED + 1)


#define LM_COMM_TCP	'T'
#define LM_COMM_UDP	'U'
#define LM_COMM_SPX	'S'
#define LM_COMM_LOCAL	'L'

/* responses to VD_INFO messages */

/* GENERIC_INFO response to VD_INFO message */
#define MSG_G_USER_INIT1 MSG_DATA 			/* flag */
#define MSG_G_USER_INIT2 MSG_G_USER_INIT1 + 1	/* flag */
#define MSG_G_OUTFILTER MSG_G_USER_INIT2 + 1	/* flag */
#define MSG_G_INFILTER MSG_G_OUTFILTER + 1		/* flag */
#define MSG_G_CALLBACK MSG_G_INFILTER + 1		/* flag */
#define MSG_G_VENDOR_MSG MSG_G_CALLBACK + 1		/* flag */
#define MSG_G_VENDOR_CHALLENGE MSG_G_VENDOR_MSG + 1	/* flag */
#define MSG_G_LOCKFILE MSG_G_VENDOR_CHALLENGE + 1   /* flag  */
#define MSG_G_READ_WAIT MSG_G_LOCKFILE + 1		/* int */
#define MSG_G_DUMP_SEND_DATA MSG_G_READ_WAIT \
				+ MAX_LONG_LEN + 1 		/* flag */
#define MSG_G_NORMAL_HOSTID MSG_G_DUMP_SEND_DATA + 1 /* flag */
#define MSG_G_CONN_TIMEOUT MSG_G_NORMAL_HOSTID + 1 	/* int */
#define MSG_G_ENFORCE_STARTDATE MSG_G_CONN_TIMEOUT \
					+ MAX_LONG_LEN + 1 	/* flag */
#define MSG_G_TELL_STARTDATE MSG_G_ENFORCE_STARTDATE + 1 /* flag */
#define MSG_G_MIN_USER_TIMEOUT MSG_G_TELL_STARTDATE + 1 /* int */
#define MSG_G_MIN_LMREMOVE MSG_G_MIN_USER_TIMEOUT \
					+ MAX_LONG_LEN + 1 	/* int */
#define MSG_G_USE_FEATSET MSG_G_MIN_LMREMOVE \
					+ MAX_LONG_LEN + 1 	/* flag */
#define MSG_G_DUP_SEL MSG_G_USE_FEATSET + 1		/* int */
#define MSG_G_USE_ALL_FEATURE_LINES MSG_G_DUP_SEL \
					+ MAX_LONG_LEN + 1	/* flag */
#define MSG_G_DO_CHECKROOT MSG_G_USE_ALL_FEATURE_LINES \
					+ 1			/* flag */
#define MSG_G_SHOW_VENDOR_DEF MSG_G_DO_CHECKROOT + 1/* flag */
#define MSG_G_ALLOW_BORROW MSG_G_SHOW_VENDOR_DEF + 1/* flag */
#define MSG_G_HOSTID_REDIRECT_VERIFY MSG_G_ALLOW_BORROW + 1/* flag */
#define MSG_G_DAEMON_PERIODIC MSG_G_HOSTID_REDIRECT_VERIFY + 1 /* flag */
#define MSG_G_COMPARE_ON_INCREMENT MSG_G_DAEMON_PERIODIC + 1 /*flag*/
#define MSG_G_COMPARE_ON_UPGRADE \
				MSG_G_COMPARE_ON_INCREMENT + 1 /* flag */
#define MSG_G_VERSION MSG_G_COMPARE_ON_UPGRADE + 1 /* int */
#define MSG_G_REVISION MSG_G_VERSION + MAX_LONG_LEN + 1 /* int */
#define MSG_G_LITE MSG_G_REVISION + MAX_LONG_LEN + 1 /* flag */
#define MSG_G_LMGRD_START MSG_G_LITE + 1 /* int */
#define MSG_G_VD_START MSG_G_LMGRD_START + MAX_XLONG_LEN + 1 /* int */
#define MSG_G_VD_TIME MSG_G_VD_START + MAX_XLONG_LEN + 1 /* int */
#define _GEN_INFO_LEN MSG_G_VD_TIME + MAX_XLONG_LEN + 1

/* HELLO messages (SHELLO, HELLO) */
/*	NOTE: The LF message uses the HELLO offsets for the first 5
 *		parameters (VER, NAME, HOST, DAEMON, and DISPLAY)
 *	DON'T CHANGE THESE (either here or in the LF message).
 *	The finder code DEPENDS on the HELLO offsets, since it uses the
 *	l_conn_message() function to build the LF message.
 */
#define MSG_HEL_VER	MSG_DATA  	/* Version comes first in HELLO msg */
#define MSG_HEL_NAME	MSG_HEL_VER+2	/* User name comes next */
#define MSG_HEL_HOST	MSG_HEL_NAME + MAX_USER_NAME + 1
#define MSG_HEL_DAEMON	MSG_HEL_HOST + MAX_SERVER_NAME + 1
#define MSG_HEL_DISPLAY	MSG_HEL_DAEMON + MAX_DAEMON_NAME + 1
#define MSG_HEL_REQ_COMM MSG_HEL_DISPLAY + MAX_DISPLAY_NAME + 1
#define MSG_HEL_UDP_TIMEOUT MSG_HEL_REQ_COMM + 2
#define MSG_HEL_PID MSG_HEL_UDP_TIMEOUT + MAX_LONG_LEN + 1
#define MSG_HEL_PLATFORM MSG_HEL_PID + MAX_LONG_LEN + 1
#define MSG_HEL_FLEX_VER MSG_HEL_PLATFORM + MAX_PLATFORM_NAME + 1
#define MSG_HEL_FLEX_REV MSG_HEL_FLEX_VER + 1
#define MSG_HEL_TCP_TIMEOUT MSG_HEL_FLEX_REV + 1 /*- in hex minutes, max 0xff,
						     4 hours, 15 minutes */
#define _HEL_LEN	MSG_HEL_TCP_TIMEOUT + 3


/* HANDSHAKE message */
#define MSG_HAND_CRYPT MSG_DATA			/* Encrytion used flag */
#define MSG_HAND_DATA  MSG_HAND_CRYPT + 2	/* Put date in message */
#define MAX_HAND_DATA_LEN 8
#define MSG_HAND_TIME	(MSG_HAND_DATA + MAX_HAND_DATA_LEN + 1)
#define MSG_HAND_DATA2 (MSG_HAND_TIME + MAX_XLONG_LEN + 1)
#define MSG_HAND_DATA3 (MSG_HAND_DATA2 + MAX_HAND_DATA_LEN + 1)
#define MSG_HAND_DATA4 (MSG_HAND_DATA3 + MAX_HAND_DATA_LEN + 1)
/*
 * 	Group-id is stored here because MSG_HEL_ is *almost* full 
 *	sent as a packed 32-bit value -- 4 bytes
 */
#define MSG_HAND_GROUP_ID (MSG_HAND_DATA4 + MAX_HAND_DATA_LEN + 1) 
#define MSG_HAND_CB_FLAG (MSG_HAND_GROUP_ID + 4)
#define _HANDSHAKE_LEN (MSG_HAND_CB_FLAG + 2)

/* HOSTID message */
#define MSG_HOSTID 		MSG_DATA
#define MSG_HOSTID_TYPE 	MSG_HOSTID + MAX_HOSTID_LEN + 1
#define _HOSTID_LEN 		MSG_HOSTID_TYPE + MAX_LONG_LEN + 1

/* HOSTID message */
#define MSG_DOWN_VER	MSG_DATA  	/* Version comes first in DOWNLO msg */
#define MSG_DOWN_NAME	MSG_DOWN_VER+2	/* User name comes next */
#define MSG_DOWN_HOST	MSG_DOWN_NAME + MAX_USER_NAME + 1
#define MSG_DOWN_DAEMON MSG_DOWN_HOST + MAX_HOSTNAME + 1
#define MSG_DOWN_DISPLAY MSG_DOWN_DAEMON + MAX_DAEMON_NAME + 1
#define MSG_DOWN_IPADDR MSG_DOWN_DISPLAY + MAX_DISPLAY_NAME + 1
#define MSG_DOWN_FLAGS MSG_DOWN_IPADDR + MAX_XLONG + 1

/* CHECKIN message */
#define MSG_IN_FEATURE	MSG_DATA		/* Feature name first */
#define MSG_IN_CODE	MSG_IN_FEATURE + MAX_FEATURE_LEN + 1
#define MSG_IN_USE_VENDOR MSG_IN_CODE + MAX_CRYPT_LEN + 1
#define MSG_IN_VENDOR	MSG_IN_USE_VENDOR + 1
#define _IN_LEN		(MSG_IN_VENDOR + MAX_VENDOR_CHECKOUT_DATA + 1)
			
/* LF message (send license file or license finder) */
/*
 *	NOTE: This message uses the HELLO offsets for the first 5
 *		parameters (VER, NAME, HOST, DAEMON, and DISPLAY)
 *	DON'T CHANGE THESE (either here or in the HELLO message).
 *	This code DEPENDS on the HELLO offsets, since it uses the
 *	l_conn_message() function to build the LF message.
 */
#define MSG_LF_VER	MSG_HEL_VER  	/* Version comes first in SEND_LF msg */
#define MSG_LF_NAME	MSG_HEL_NAME	/* User name comes next */
#define MSG_LF_HOST	MSG_HEL_HOST
#define MSG_LF_DAEMON	MSG_HEL_DAEMON
#define MSG_LF_DISPLAY	MSG_HEL_DISPLAY
#define MSG_LF_FINDER_TYPE MSG_LF_DISPLAY+MAX_DISPLAY_NAME
#define _LF_LEN (MSG_LF_FINDER_TYPE + MAX_FINDER_TYPE + 1)

/* LF_DATA message */
#define MSG_LF_REMAIN 	MSG_DATA		/* # remaining bytes */
#define MSG_LF_DATA	(MSG_LF_REMAIN + MAX_LONG_LEN + 1)
/* #define _LF_DATA_LEN	Unused - This message fills the available space */
#define MAX_LF_DATA	(LM_MSG_LEN - MSG_LF_DATA)	
	/* Actually (LM_MSG_LEN - 1) - MSG_LF_DATA + 1 */

/* LIST message */
#define MSG_LIST_FEATURE MSG_DATA		/* Feature name first */
#define MSG_LIST_CODE	MSG_LIST_FEATURE + MAX_FEATURE_LEN + 1
#define MSG_LIST_BFLAG	MSG_LIST_CODE + MAX_CRYPT_LEN + 1
#define _LIST_LEN	(MSG_LIST_BFLAG + 1)

/* NEED_HOSTID message */
#define MSG_NEED_HOSTID_TYPE 	MSG_DATA
#define _NEED_HOSTID_LEN 	MSG_NEED_HOSTID_TYPE + MAX_LONG_LEN + 1


/* NUSERS message */
#define MSG_NUSERS_N	MSG_DATA
#define MSG_NUSERS_TOT	(MSG_NUSERS_N + MAX_LONG_LEN + 1)
#define MSG_NUSERS_TIME	(MSG_NUSERS_TOT + MAX_LONG_LEN + 1)
#define _NUSERS_LEN	(MSG_NUSERS_TIME + MAX_LONG_LEN + 1)

/* OK response to HELLO message */
#define MSG_OK_VER 	MSG_DATA
#define MSG_OK_REV	MSG_OK_VER + MAX_LONG_LEN + 1
#define MSG_OK_PATCH	MSG_OK_REV + MAX_LONG_LEN + 1
#define MSG_OK_COMM_VER	MSG_OK_PATCH + 1
#define MSG_OK_COMM_REV	MSG_OK_COMM_VER + MAX_LONG_LEN + 1
#define MSG_OK_HOST	MSG_OK_COMM_REV + MAX_LONG_LEN + 1
#define MSG_OK_DAEMON	MSG_OK_HOST + MAX_HOSTNAME + 1
#define _OK_LEN		MSG_OK_DAEMON + MAX_DAEMON_NAME + 1

/* ORDER message */
#define MSG_ORDER_N	MSG_DATA
#define MSG_ORDER_HOST	(MSG_ORDER_N + MAX_LONG_LEN + 1)
#define _ORDER_LEN	(MSG_ORDER_HOST + MAX_SERVER_NAME + 1)

/* REMOVE message */
#define MSG_REMOVE_FEAT	MSG_DATA
#define MSG_REMOVE_USER (MSG_REMOVE_FEAT + MAX_FEATURE_LEN + 1)
#define MSG_REMOVE_HOST (MSG_REMOVE_USER + MAX_USER_NAME + 1)
#define MSG_REMOVE_DISP	(MSG_REMOVE_HOST + MAX_SERVER_NAME + 1)
#define MSG_REMOVE_CODE	(MSG_REMOVE_DISP + MAX_DISPLAY_NAME + 1)	/* DO NOT USE */
#define MSG_REMOVE_FORCE (MSG_REMOVE_CODE + 1)
#define _REMOVE_LEN (MSG_REMOVE_FORCE + MAX_CRYPT_LEN + 1)

/* REMOVEH message - SERVER and PORT are unused */
#define MSG_REMOVEH_FEAT	MSG_DATA
#define MSG_REMOVEH_SERVER	(MSG_REMOVEH_FEAT + MAX_FEATURE_LEN + 1)
#define MSG_REMOVEH_PORT 	(MSG_REMOVEH_SERVER + MAX_SERVER_NAME + 1)
#define MSG_REMOVEH_HANDLE	(MSG_REMOVEH_PORT + MAX_LONG_LEN + 1)
#define MSG_REMOVEH_FORCE	(MSG_REMOVEH_HANDLE + MAX_LONG_LEN + 1)
#define _REMOVEH_LEN		(MSG_REMOVEH_FORCE + MAX_LONG_LEN + 1)

/* DAEMON port message */
#define MSG_SPP_NAME	MSG_DATA		/* Name of DAEMON */
#define MSG_SPP_PORT	MSG_SPP_NAME + MAX_DAEMON_NAME + 1
#define MSG_SPP_UDP_PORT	MSG_SPP_PORT + MAX_LONG_LEN + 1
#define MSG_SPP_SPX_PORT	MSG_SPP_UDP_PORT + MAX_LONG_LEN + 1 
#define _DAEMON_LEN	(MSG_SPP_SPX_PORT + MAX_LONG_LEN + 1)	    

/* TRY ANOTHER message */
#define MSG_TRY_HOST		MSG_DATA		/* Where to try */
#define MSG_TRY_TCP_PORT	MSG_TRY_HOST + MAX_SERVER_NAME + 1
#define MSG_TRY_HOST2		MSG_TRY_TCP_PORT + MAX_LONG_LEN + 1
#define MSG_TRY_UDP_PORT MSG_TRY_HOST2 + (MAX_HOSTNAME - MAX_SERVER_NAME) + 1
#define MSG_TRY_TRANSPORT 	MSG_TRY_UDP_PORT + MAX_LONG_LEN + 1
#define MSG_TRY_TRANSPORT_REASON	MSG_TRY_TRANSPORT  + 1
#define LM_COMM_REASON_USER 	'U'
#define LM_COMM_REASON_DAEMON 	'D'
#define _TRY_LEN		(MSG_TRY_TRANSPORT_REASON + 1)

/* USERNAME response */
#define MSG_U_NAME	MSG_DATA		/* Username comes first */
#define MSG_U_NODE	MSG_U_NAME + MAX_USER_NAME + 1
#define MSG_U_DISP	MSG_U_NODE + MAX_SERVER_NAME + 1
#define MSG_U_VER	MSG_U_DISP + MAX_DISPLAY_NAME + 1
#define MSG_U_NUM	MSG_U_VER + MAX_VER_LEN + 1
#define MSG_U_FLAGS	MSG_U_NUM + MAX_LONG_LEN + 1 
#define MSG_U_TIME	MSG_U_FLAGS + 3 /* 3 bytes of flags for historic reasons */
#define MSG_U_LINGER	MSG_U_TIME + MAX_LONG_LEN + 1
#define MSG_U_HANDLE	MSG_U_LINGER + MAX_LONG_LEN + 1
#define _U_LEN		(MSG_U_HANDLE + MAX_LONG_LEN + 1)



/* VHELLO message */
#define MSG_VHEL_VER	MSG_DATA  	/* Version comes first in HELLO msg */
#define MSG_VHEL_HOST	MSG_VHEL_VER + 2
#define MSG_VHEL_DAEMON	MSG_VHEL_HOST + MAX_SERVER_NAME + 1
#define MSG_VHEL_TRANSPORT 		MSG_VHEL_DAEMON + MAX_DAEMON_NAME + 1
#define MSG_VHEL_ADDR_HOST 		MSG_VHEL_TRANSPORT + 1
#define MSG_VHEL_ADDR_PORT 		MSG_VHEL_ADDR_HOST + MAX_SERVER_NAME + 1
#define MSG_VHEL_REC_TRANSPORT 		MSG_VHEL_ADDR_PORT + MAX_LONG_LEN + 1
#define MSG_VHEL_TRANSPORT_REASON 	MSG_VHEL_REC_TRANSPORT + 1
#define MSG_VHEL_FLEX_VER		MSG_VHEL_TRANSPORT_REASON + 1
#define MSG_VHEL_FLEX_REV		MSG_VHEL_FLEX_VER + MAX_LONG_LEN + 1
#define _VHELLO_LEN 			MSG_VHEL_TRANSPORT_REASON + MAX_LONG_LEN + 1

/* VENDOR_DAEMON_INFO */
/* (we may want to add more info at the end later) */
#define MSG_VD_INFO_PARAM 	MSG_DATA  		
#define MSG_VD_INFO_FEATURE 	MSG_VD_INFO_PARAM + 1
#define MSG_VD_INFO_CODE	MSG_VD_INFO_FEATURE + MAX_FEATURE_LEN + 1

/* when used for borrowing */
#define MSG_VD_INFO_BMSG_LEN 	MSG_VD_INFO_PARAM + 1
#define MSG_VD_INFO_BMSG_SALT 	MSG_VD_INFO_BMSG_LEN + MAX_SHORT_LEN + 1
#define MSG_VD_INFO_BMSG 	MSG_VD_INFO_BMSG_SALT + 2 /* 2 bytes */

#define _VD_INFO_LEN		MSG_VD_INFO_CODE + MAX_CRYPT_LEN + 1

#ifdef LM_GENERIC_VD
#define LM_NEED_SEEDS		's'
#endif /* GENERIC */

/* HEARTBEAT */
#define MSG_HB_CPU_USAGE_1	MSG_DATA
#define MSG_HB_CPU_USAGE_2	MSG_HB_CPU_USAGE_1 + MAX_XLONG + 1
#define MSG_HB_CPU_USAGE_3	MSG_HB_CPU_USAGE_2 + MAX_XLONG + 1
#define MSG_HB_CPU_USAGE_4	MSG_HB_CPU_USAGE_3 + MAX_XLONG + 1
#define _HB_LEN 		MSG_HB_CPU_USAGE_4 + MAX_XLONG + 1

/*
 *	Compute the Message length - longest of any of these messages
 */

#define LM_MSG_LEN _HEL_LEN

#if (_BORROW_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _BORROW_LEN
#endif

#if (_BORROW2_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _BORROW2_LEN
#endif

#if (_CO_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _CO_LEN
#endif

#if (_CLOCK_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _CLOCK_LEN
#endif

#if (_FEATURE_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _FEATURE_LEN
#endif

#if (_FEATURE2_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _FEATURE2_LEN
#endif

#if (_DAEMON_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _DAEMON_LEN
#endif

#if (_HANDSHAKE_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _HANDSHAKE_LEN
#endif

#if (_LF_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _LF_LEN
#endif

#if (_NUSERS_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _NUSERS_LEN
#endif

#if (_ORDER_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _ORDER_LEN
#endif

#if (_REMOVE_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _REMOVE_LEN
#endif

#if (_REMOVEH_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _REMOVEH_LEN
#endif

#if (_TRY_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _TRY_LEN
#endif

#if (_U_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _U_LEN
#endif

#if (_IN_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _IN_LEN
#endif

#if (_LIST_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _LIST_LEN
#endif

#if (_OK_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _OK_LEN
#endif

#if (_GENINFO_LEN > LM_MSG_LEN)
#undef LM_MSG_LEN
#define LM_MSG_LEN _GENINFO_LEN
#endif

#define LM_LOG_MAX_LEN (LM_MSG_LEN - MSG_DATA)


/* Requests */

#define LM_HANDSHAKE	'a'
#define LM_DAEMON	'b'	/* SERVER ONLY request (data) */
#define LM_DISOWN	'c'	/* SERVER ONLY request */
#define LM_DUMP		'd'
#define LM_IS_MASTER	'e'	/* SERVER ONLY request */
#define LM_DETACH	'f'	/* SERVER ONLY request */
#define LM_HEARTBEAT	'g'
#define LM_HELLO	'h'
#define LM_CHECKIN	'i'
#define LM_LOG		'j'
#define LM_RESTART	'k'	/* SERVER ONLY request */
#define LM_LIST 	'l'
#define LM_I_MASTER	'm'	/* SERVER ONLY request */
#define LM_NEWSERVER	'n'	/* SERVER ONLY request */
#define LM_CHECKOUT	'o'
#define LM_PID		'p'	/* SERVER ONLY request */
#define LM_SHUTDOWN	'q'
#define LM_MASTER_READY	'r'	/* SERVER ONLY request */
#define LM_SHELLO	's'	/* SERVER ONLY request */
#define LM_TELLME	't'
#define LM_REMOVE	'u'
#define LM_ORDER	'v'	/* SERVER ONLY request */
#define LM_SWITCH	'w'
#define LM_DOTELL	'x'	/* SERVER ONLY request */
#define LM_U_MASTER	'y'	/* SERVER ONLY request */
#define LM_DOREMOVEALL	'z'	/* SERVER ONLY request */
#define LM_CLOCKSETTING	'*'
#define LM_REREAD	'&'
#define LM_SWITCH_REPORT '^'
#define LM_NEWREPLOG    '{'
#define LM_SWITCH_REPORT_CLIENT "CLIENT"
#define LM_SINCE	'%'
#define LM_SEND_LF	'$'
#define LM_REMOVEH	'#'
#define LM_VENDOR_MSG	'@'
#define LM_BORROW	'!'
#define LM_BORROW2	'('
#define LM_VHELLO	'+'	/* SERVER ONLY request */
#define LM_CHALLENGE	')'
#define LM_VENDOR_DAEMON_INFO	'='
#define LM_HOSTID	'~'
#define LM_REPFILE	'_' 	/* underscore */
#define LM_HELLO_THIS_VD '`'
#ifdef LM_GENERIC_VD
#define LM_SEND_SEEDS		'-' 	/* dash */
#endif /* LM_GENERIC_VD */


/* Responses */

#define	LM_TRY_ANOTHER		'A'
#define LM_BUSY			'B'
#define	LM_NO_CONFIG_FILE	'C'
#define LM_BUSY_NEW		'D'
#define LM_COMM_UNAVAILABLE	'E'
#define	LM_FEATURE_AVAILABLE	'F'
#define LM_HEARTBEAT_RESP	'G'
#define LM_NOT_ADMIN		'H'	/* Resp. to lmremove/lmdown/lmreread */
#define LM_TOO_SOON		'I'	/* Response to lmremove */
#define LM_NO_USER		'J'	/* Response to lmremove */
#define LM_NO_MESSAGE		'K'	/* NO MESSAGE - Used as a flag */
#define LM_LF_DATA		'L'
#define LM_VD_FEAT_BUNDLE_1	'M' 
#define	LM_NUSERS		'N'
#define	LM_OK			'O'
#define LM_VD_GEN_INFO		'P' 
#define LM_QUEUED		'Q'
#define LM_CHALLENGE_RESP	'R'
#define	LM_SHELLO_OK		'S'
#define	LM_NEED_HOSTID		'T'
#define	LM_USERNAME		'U'
#define	LM_VENDOR_RESP		'V'
#define	LM_WHAT			'W'
#define	LM_USERNAME2		'X'
#define	LM_FEATURE_LINE		'Y'
#define	LM_FEATURE_LINE2	'Z'
#define	LM_NO_SUCH_FEATURE	'?'
#define LM_NO_RETURN_EARLY	'['

#endif 	/* _LM_COMM_INCLUDED */
