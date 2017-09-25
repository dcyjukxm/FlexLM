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
 *	Module: $Id: ls_comm.h,v 1.4 2003/01/13 22:31:36 kmaclean Exp $
 *
 *	Description: Definition of the server's parent-child comm protocol.
 *
 *	M. Christiano
 *	3/9/88
 *
 *	Last changed:  10/18/95
 *
 */
/*
 *	The message:
 *			destination	(1 byte) 		SCOMM_DEST
 *			msgtype		(1 byte) 		SCOMM_TYPE
 *			data		(MSGSIZE-3 bytes) 	SCOMM_DATA
 *
 *	All data fields look like the following:
 *
 *   "handle\0user\0host\0NNNN\0wait\0feature_name\0version_number\0dup_sel\0"
 *		
 *				handle 	(MAX_CLIENT_HANDLE )	SCOMM_USER
 *				:
 *				user	(MAX_USER_NAME + 1)
 *				host	(MAX_SERVER_NAME + 1)
 *				N	(4 bytes + null)	SCOMM_N
 *				wait	(1 byte + null)		SCOMM_WAIT
 *				feature	(MAX_FEATURE_LEN + 1)	SCOMM_FEAT
 *				version (MAX_VER_LEN + 1)	SCOMM_VER
 *				dup_sel (MAX_LONG_LEN + 1)	SCOMM_DUP
 *
 *	The messages are:
 *						msgtype		data
 *						-------		----
 *		List of users:			LM_LIST		feature
 *		Diagnostic dump:		LM_DUMP
 *		A single user:			LM_USERNAME	user@host@time
 *		Add this user to tellme list	LM_TELLME	user@host:feat
 *		Process tellme list		LM_DOTELL	feature
 *		Remove all traces of a user	LM_DOREMOVEALL	user@host
 *		Remove all traces of a server	LM_DISOWN	(NONE)
 *		OK (message was fine)		LM_OK		variable
 *		ERROR				LM_WHAT		variable
 *		Switch logfile			LM_SWITCH	filename
 *		Switch report logfile		LM_SWITCH_REPORT filename
 *
 *		The "tellme" list is maintained at the top-level server,
 *		so all tellme requests go there.
 *
 *	Message codes are defined in "lm_comm.h"
 */


#define MSG_UP (unsigned char) 0	/* Message is going to "topdog" */
#define MSG_DOWN 'd'	/* Message is going down (to our child) */

#define MSG_ANY 1	/* Receive messages for ANYONE */
#define MSG_ME	2	/* Receive messages for ourselves only (pass others) */

/*
 *	Message field offsets
 */
#define SCOMM_DEST 0
#define SCOMM_TYPE 1
#define SCOMM_DATA 2
#define SCOMM_CHANDLE 0		/* Data field offsets from start of data */
#define SCOMM_USER (SCOMM_CHANDLE + MAX_LONG_LEN + 1)
#define SCOMM_NODE (SCOMM_USER + MAX_USER_NAME + 1)
#define SCOMM_DISP (SCOMM_NODE + MAX_SERVER_NAME + 1)
#define SCOMM_N (SCOMM_DISP + MAX_DISPLAY_NAME + 1)
#define SCOMM_N_SIZE	5	/* 4 bytes + null */
#define SCOMM_WAIT (SCOMM_N + SCOMM_N_SIZE)

#define SCOMM_TIME SCOMM_WAIT	/* Time goes here for USERNAME message */
#define SCOMM_TIME_SIZE 21	/* long int as a string + NULL */

#define SCOMM_FEAT (SCOMM_WAIT + 2)
#define SCOMM_VER  (SCOMM_FEAT + MAX_FEATURE_LEN + 1)
#define SCOMM_DUP  (SCOMM_VER + MAX_VER_LEN + 1)
#define SCOMM_LINGER  (SCOMM_DUP + MAX_LONG_LEN + 1)
#define SCOMM_HANDLE  (SCOMM_LINGER + MAX_LONG_LEN + 1)
#define SCOMM_CODE  (SCOMM_HANDLE + MAX_LONG_LEN + 1)
#define SCOMM_INET  (SCOMM_CODE + MAX_CRYPT_LEN + 1)
#define SCOMM_VENDORDEF  (SCOMM_INET + MAX_INET + 1)
#define SCOMM_USEVENDORDEF  (SCOMM_VENDORDEF + MAX_VENDOR_CHECKOUT_DATA + 1)
#define SCOMM_HOSTID  (SCOMM_USEVENDORDEF + 2)

#define SCOMM_MSGSIZE ((SCOMM_DATA + 1) + (SCOMM_HOSTID + (MAX_SHORTHOSTID_LEN * 5) + 6))

#if (SCOMM_TIME_SIZE > (MAX_FEATURE_LEN + MAX_VER_LEN + MAX_LONG_LEN + 1))
#undef SCOMM_MSGSIZE
#define SCOMM_MSGSIZE ((SCOMM_DATA + 1) + (SCOMM_TIME + SCOMM_TIME_SIZE) + 1)
#endif

#define SCOMM_MAXDATA (SCOMM_MSGSIZE - SCOMM_DATA - 1)

/*
 *	Macros to get fields of the message
 */

#define MSGTYPE(_ptr)	 (_ptr[SCOMM_TYPE])
#define MSGDATA(_ptr)	 (&_ptr[SCOMM_DATA])
#define MSGNNNN(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_N])
#define MSGTIME(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_TIME])
#define MSGWAIT(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_WAIT])
#define MSGFEATURE(_ptr) (&_ptr[SCOMM_DATA+SCOMM_FEAT])
#define MSGCHANDLE(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_CHANDLE])
#define MSGUSER(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_USER])
#define MSGNODE(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_NODE])
#define MSGDISP(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_DISP])
#define MSGVER(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_VER])
#define MSGDUP(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_DUP])
#define MSGLINGER(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_LINGER])
#define MSGCODE(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_CODE])
#define MSGHANDLE(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_HANDLE])
#define MSGINET(_ptr)	 (&_ptr[SCOMM_DATA+SCOMM_INET])
#define MSGVENDORDEF(_ptr) (&_ptr[SCOMM_DATA+SCOMM_VENDORDEF])
#define MSGUSEVENDORDEF(_ptr) (_ptr[SCOMM_DATA+SCOMM_USEVENDORDEF])
#define MSGHOSTID(_ptr) (&_ptr[SCOMM_DATA+SCOMM_HOSTID])
