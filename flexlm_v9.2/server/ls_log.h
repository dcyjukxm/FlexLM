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
 *	Module: $Id: ls_log.h,v 1.10 2003/03/05 22:17:02 sluu Exp $
 *
 *	Description: Local definitions for log file functions
 *
 *	J. McBeath
 *	5/31/90
 *
 *	Last changed:  10/19/98
 *
 */

#ifndef _LS_LOG_H 
/* Log function constants */
/* where to output info */


#define LL_LOGTO_ASCII (1<<0)
#define LL_LOGTO_REPORT (1<<1)
#define LL_LOGTO_BOTH (LL_LOGTO_ASCII|LL_LOGTO_REPORT)

/* log file functions */
#define LL_USAGE 	0x1
#define LL_CHECKIN 	0x2
#define LL_USER		0x3
#define LL_DENIED 	0x4
#define LL_START 	0x5
#define LL_CONFIG	0x6
#define LL_DICTIONARY	0x7
#define LL_DEL_CLIENT	0x8
#define LL_CKSUM	0x9
#define LL_CURR_USE_LEVEL 0xa
#define LL_RES_USAGE 	0xb
#define LL_FEATURELIST	0xc
#define LL_POOL		0xe
#define LL_CLIENT	0xf
#define LL_END		0x10
#define LL_COMMENT 	0x11
#define LL_ERROR	0x12
#define LL_STAT		0x13
#define LL_TIMESTAMP	0x14
#define LL_SWITCHTO	0x16
#define LL_SWITCHFROM	0x17
#define LL_FLUSH	0x19
#define LL_BORROW 	0x1a
#define LL_UNBORROW 	0x1b
#define LL_BORROWFAIL   0x1c
#define LL_UNUSED1	0x1d
#define LL_UNUSED2	0x1e
#define LL_UNUSED3	0x1f
#define LL_SERVER	0x20
#define LL_UNSUPPORTED	0x22
#define LL_HOSTID	0x23
#define LL_CPU_USAGE	0x24
#define LL_PACKAGE	0x25
#define LL_PACKAGE_SUITE	0x26
#define LL_SERVER_PLATFORM	0x27
#define LL_SERVER_INFO	0x28
#define LL_BORROW_INCR   0x29
#define LL_MSG_VOL   0x30
#define LL_COUNTER   0x31
#define LL_ULTIMATE_DENIAL	0x32
/*
 *	Options file items
 */
#define LL_GROUP 	0x50
#define LL_HOST_GROUP 	0x51
#define LL_GROUP_MEMBER 0x52
#define LL_INCLUDE	0x53
#define LL_EXCLUDE	0x55
#define LL_RESERVE	0x58
#define LL_MAX		0x59
#define LL_DYNAMIC_RESERVE	0x5a

/* usage classes */
#define LL_USAGE_OUT 		0x1
#define LL_USAGE_IN 		0x2	/*-gets mapped to LL_CHECKIN */
#define LL_USAGE_QUEUED 	0x3
#define LL_USAGE_DEQUEUED 	0x4
#define LL_USAGE_DENIED 	0x5	/*-gets mapped to LL_DENIED */
#define LL_USAGE_UNSUPPORTED 	0x6	/*-gets mapped to LL_UNSUPPORTED */
#define LL_USAGE_UPGRADE 	0x7	/*- checkout x, later checkout x+y */
#define LL_USAGE_INUSE 		0x8	/*-for reread */
#define LL_USAGE_INQUEUE 	0x9
#define LL_USAGE_METERON 	0xa
#define LL_USAGE_USED 		0xb
#define LL_USAGE_METERFAIL 	0xc	/* Meter failure (see LL_REASON) */
#define LL_USAGE_METEROFF 	0xd
#define LL_USAGE_BORROW 	0xe
#define LL_USAGE_UNBORROW 	0xf
#define LL_USAGE_BORROWFAIL    0x10
#define LL_USAGE_BORROW_INCR   0x11

/* reason classes */
#define LL_REASON_INACTIVE 	0x1
#define LL_REASON_CLIENT_CRASH 	0x2
#define LL_REASON_CLIENT_REQUEST 0x3
#define LL_REASON_SHUTDOWN 	0x4
#define LL_REASON_USER_REMOVED 	0x5
#define LL_REASON_SERVER_CRASH 	0x6
#define LL_REASON_REQUEST_SERVICED 	0x7
#define LL_REASON_INITIAL_DEC 	0x8
#define LL_REASON_PERIODIC_DEC 	0x9
#define LL_REASON_NO_COUNT 	0xa		/* No count left in meter */
#define LL_REASON_CANT_READ 	0xb		/* Can't read from meter */
#define LL_REASON_PACKAGE	0xc
#define LL_REASON_BORROW	0xd

/* error classes */
#define LL_SYSTEM_ERROR 	0x1
#define LL_INTERNAL_ERROR 	0x2
#define LL_CLIENT_ERROR 	0x3
#define LL_FILE_ERROR 		0x4
#define LL_METER_ERROR 		0x5

#define REPLOG_SWITCH 1 		/* for ls_log_repopen_report */
#define REPLOG_NEWFILE 2
#define _LS_LOG_H
#endif /* _LS_LOG_H */

