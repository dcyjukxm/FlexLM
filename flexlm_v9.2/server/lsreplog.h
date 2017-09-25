/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lsreplog.h,v 1.7 2003/01/13 22:31:36 kmaclean Exp $
 *
 *	Description: Local definitions for replog file functions
 *
 *	D. Birns
 *	11/12/95
 *
 *	Last changed:  10/12/98
 *
 */

#ifndef _LS_REPLOG_H 
/* Log function constants */
/* where to output info */

/*-print whenver comes first: */
#define PRINT_TIME_EVERY_N_EVENTS 500
#define PRINT_TIME_EVERY_N_HOURS  1
/*-	flush whichever comes first: */
#define FLUSH_EVERY_N_DAYS	7
#define FLUSH_EVERY_N_EVENTS	20000

#define PRINT_CKSUM_EVERY_N_LINES 200

#define FILTER_LEN 70 /*-must be an even number */

#define LL_EQUAL	'g'

/*-
 *	Time is an optional arg common to many lines
 *	and must be zero!
 *	All other optional args start at 1
 */
#define LL_TIME_DIFF	0x0

/*-
 *	FEATURE_LIST -- optional args
 */

#define LL_FL_OVERDRAFT	0x1
#define LL_FL_DUP	0x2
#define LL_FL_LINGER	0x3
#define LL_FL_UNCOUNTED 0x4
#define LL_FL_LOWWATER 	0x5
#define LL_FL_TIMEOUT	0x6
#define LL_FL_RES	0x7
#define LL_FL_TYPE_MASK	0x9
#define LL_FL_USER_BASED 0xa
#define LL_FL_HOST_BASED 0xb
#define LL_FL_MINIMUM	0xc
#define LL_FL_VENDOR_DEF 0xd

/*-
 *    START -- optional args
 */

#define LL_START_RESTRICTED 0x1



/*-
 *	USAGE -- optional args
 */

#define LL_USE_NLICREQ		 0x1
#define LL_USE_NLICACT		 0x2
#define LL_USE_VENDOR_DEF 	 0x3
#define LL_USE_WHY 		 0x4
#define LL_USE_VERSION 		 0x5
#define LL_USE_BROTHER 		 0x6
#define LL_USE_WHAT 		 0x7
#define LL_USE_ERRNO 		 0x8
#define LL_USE_GROUP		 0x9
#define LL_USE_CPU_USAGE_1	 0xa
#define LL_USE_CPU_USAGE_2	 0xb
#define LL_USE_CPU_USAGE_3	 0xc
#define LL_USE_CPU_USAGE_4	 0xd
#define LL_USE_REQNUM		 0xe
#define LL_USE_SN		 0xf
#define LL_USE_USERFLAGS	0x10
#define LL_USE_LINGER_SECS	0x11


/*-
 *	LL_RES_USE -- optional args
 */
#define LL_RES_USE_LICGROUP 	0x1
#define LL_RES_USE_NLIC 	0x2
/*-
 *	LL_CLIENT -- optional args
 */

#define LL_CLIENT_PLATFORM 	0x2
#define LL_CLIENT_COMMVER 	0x3
#define LL_CLIENT_COMMREV 	0x4
#define LL_CLIENT_TRANSPORT 	0x5
#define LL_CLIENT_UDPTIMEOUT 	0x6
#define LL_CLIENT_PROJECT 	0x7
#define LL_CLIENT_CAPACITY 	0x8
#define LL_CLIENT_FLEXVER 	0x9
#define LL_CLIENT_FLEXREV 	0xa
#define LL_CLIENT_GROUP_ID 	0xb

/*-
 *	LL_SWITCHTO -- optional args
 */
#define LL_SWTO_FILENAME	0x1
#define LL_SWTO_CLIENTNAME	0x2
#define LL_SWTO_NODE		0x3
#define LL_SWTO_DISPLAY		0x4
#define LL_SWTO_HANDLE		0x5

/*-
 *	LL_CURR_USE_LEVEL -- optional args
 */

#define LL_CURR_USE_LEVEL_FLOAT 	0x1
#define LL_CURR_USE_LEVEL_USERS 	0x2
#define LL_CURR_USE_LEVEL_Q 		0x3

/*-
 *	INTERNET optional arg to options
 */
#define LL_OPT_INTERNET 		0x1
#define LL_OPT_COUNT 			0x2
#define LL_OPT_CODE 			0x3

/*-
 *	CPU_USAGE -- optional args
 */
#define LL_CPU_USAGE_1			0x1
#define LL_CPU_USAGE_2			0x2
#define LL_CPU_USAGE_3			0x3
#define LL_CPU_USAGE_4			0x4
/*-
 *	MSG_VOL -- optional args
 */
#define LL_MSG_VOL_PERIOD			0x1


#ifndef _LS_SERVPROT_H_
#ifndef DICTKEY_DEF
typedef unsigned int DICTKEY;
#define DICTKEY_DEF
#endif /* DICTKEY_DEF */
#endif

#define FILTER_START (' ' + 1)
#define FILTER_OBASE 94
#define MAX_HEX 8
/*-
 *	MAX_REPFILE_LEN applies only to base-20 lines, not dictionary 
 *	entries
 */
#define MAX_REPFILE_LEN ((LL_FL_VENDOR_DEF /*- highest optional args */ \
			   * (MAX_HEX + MAX_HEX + 2)) /*- space, num=num */ \
			   + \
			  (10 /*- highest fixed args */ *  \
				(MAX_HEX + 1) /*- space, num*/))

#define REPCODE1 0x875efc90
#define REPCODE2 0xd16592b1
#define CKSUMBUF_SIZE (PRINT_CKSUM_EVERY_N_LINES  * (FILTER_LEN + 15))

/*- for ls_log_client routine */
#define LS_CLIENT_REMOVE 1
#define LS_CLIENT_FLUSH  2
#define LS_CLIENT_LOOKUP_ONLY  3

#define _LS_REPLOG_H
#endif /* _LS_REPLOG_H */

