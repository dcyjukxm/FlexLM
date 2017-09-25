/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_privat.h,v 1.91 2003/04/23 23:56:16 sluu Exp $
 */
/***********************************************************/
/**   @file l_privat.h
 *    @brief Private (internal) header info common to all files
 *    @version $Revision: 1.91 $
 *
  ************************************************************/

#include <stdio.h>
#include <stdlib.h>

#ifndef _L_PRIVAT_H_
#define _L_PRIVAT_H_

#ifndef FLEXLM_ULTRALITE
#include <setjmp.h>
#include "lm_comm.h"
#include "l_ctype.h"
#endif 

/* 
 *	job->options->flags
 */
#define LM_OPTFLAG_UNUSED1		0x1 /* True if ok, default False */
#define LM_OPTFLAG_PORT_HOST_PLUS	0x2 /* True if ok, default True */
#define LM_OPTFLAG_DIAGS_ENABLED	0x4 /* True if ok, default True */
#define LM_OPTFLAG_TRY_OLD_COMM		0x8 /* True if ok, default False */
#define LM_OPTFLAG_RETRY_CHECKOUT      0x10 /* True if retry, default False */
#define LM_OPTFLAG_SUPPORT_HP_IDMODULE 0x20 /* default False */
#define LM_OPTFLAG_CHECK_BADDATE       0x40 /* default False */
#define LM_OPTFLAG_USE_START_DATE      0x80 /* default False */
#define LM_OPTFLAG_LKEY_START_DATE    0x100 /* default False */
#define LM_OPTFLAG_NO_HEARTBEAT       0x200 /* default False */
#define LM_OPTFLAG_LONG_ERRMSG        0x400 /* default True */
#define LM_OPTFLAG_STRINGS_CASE_SENSITIVE 0x1000  /* default false */
#define LM_OPTFLAG_PC_PROMPT_FOR_FILE 0x2000  /* default true */
#define LM_OPTFLAG_INTERNAL1 	      0x4000  
#define LM_OPTFLAG_INTERNAL2 	      0x8000  
#define LM_OPTFLAG_CKOUT_INSTALL_LIC 0x10000  /* default true */
#define LM_OPTFLAG_PERROR_MSGBOX     0x20000  /* default true */
#define LM_OPTFLAG_INTERNAL3         0x40000  /* default true */
#define LM_OPTFLAG_LCM               0x80000  /* default is true */
#define LM_OPTFLAG_FLEXLOCK         0x100000
#define LM_OPTFLAG_V7_0_HANDSHAKE   0x200000  /* default is false */
#define LM_OPTFLAG_REPORT_FEATNAME_ERRS  0x400000  /* default is false */
#define LM_OPTFLAG_MT_HEARTBEAT	   0x80000000 /* multi-threaded heartbeat */
/*
 *	-------------------------------------------------------------
 *	Error processing 
 */


#define LM_ERRMASK_ALL 0xff /* 1 byte which things to present to user */
#define LM_ERRMASK_FEAT	0x1
#define LM_ERRMASK_PATH	0x2
#define LM_ERRMASK_SYS	0x4
#define LM_ERRMASK_FROM_SERVER 0x100 /* indicates this error set by server */

/*****************************************************************/
/** @section errorNote	IMPORTANT NOTE about errors:
		-# If an lc_xxx function can set an error, it should
		   call l_clear_error at the top of the function.
		-# If a lc_xxx or l_xxxx function calls an lc_xxx
		   function, it must do this carefully, knowing that
		   the lc_xxx function may clear or reset the current
		   error.  For example, it may:
			- save the current error with 
				- LM_ERR_INFO e;

				- e = job->err_info;
				- lc_xxx()
				- job->err_info = e;
			- it may allow the lc_xxx function to reset the
			  error:
				- e = job->err_info;
				- if (lc_xxx() == success)
					- job->err_info = e;
			- it may know a priori that lc_xxx doesnt reset
			  the flexlm errno (although this could break
			  when lc_xxx is modified).
*****************************************************************/

/**	Set an error for a thread. 
 * We set the major errno here to prevent spoofing by changing
 *	l_set_error().  But we don't reset the major_errno in some cases...
 * @ref errorNote
 */
#define LM_SET_ERROR_THREAD(job, this_lmerrno, minor, this_u_errno, str, mask, thread) \
	{ \
		job->err_info.maj_errno =  \
		(job->err_info.maj_errno && \
			((this_lmerrno == LM_CANTCONNECT) || \
			(this_lmerrno == LM_NOSERVER) || \
			(this_lmerrno == LM_VENDOR_DOWN) || \
			(this_lmerrno == LM_POOL))) ? \
		job->err_info.maj_errno : this_lmerrno; \
		l_set_error(job, this_lmerrno, minor, this_u_errno,  \
		(char *)str, mask, thread) ; \
	}

/**	Macro to set an errno for a job.
 * @ref errorNote
 */
#define LM_SET_ERRNO(job, this_lmerrno, minor, this_u_errno) \
	LM_SET_ERROR_THREAD(job, this_lmerrno, minor, this_u_errno, 0, LM_ERRMASK_ALL, 0) 

/**	Macro to set an error for a job.
 * @ref errorNote
 */
#define LM_SET_ERROR(job, this_lmerrno, minor, this_u_errno, str, mask) \
        LM_SET_ERROR_THREAD(job, this_lmerrno, minor, this_u_errno, str,mask, 0) 

#ifndef RELEASE_VERSION
#define LM_SET_DEBUG_ERROR(job,err, var) LM_SET_ERROR(job,err, __LINE__, 0,__FILE__ ":" #var, LM_ERRMASK_ALL)
#else
#define LM_SET_DEBUG_ERROR(job, err, var) LM_SET_ERROR(job,err, __LINE__, 0,NULL, LM_ERRMASK_ALL)
#endif 

/** Macro that sets the errno then returns */
#define RETURN_SET_ERRNO(job, err)	\
{\
	LM_SET_DEBUG_ERROR(job,err,"")\
	return job->lm_errno;\
}
/** Macro that checks a pointer for NULL then sets an error and returns if it is NULL */
#define ERROR_RETURN_NULL_CHECK(job, ptr)	\
	{\
	if ( ptr == NULL ) {\
		LM_SET_DEBUG_ERROR(job,LM_NULLPOINTER,ptr)\
		return job->lm_errno;\
	}\
}
/** Macro that checks a number for zero then sets an error and returns if it is */
#define ERROR_RETURN_ZERO_CHECK(job, num)	\
	{\
	if ( num == 0) {\
		LM_SET_DEBUG_ERROR(job,LM_BADPARAM,num)\
		return job->lm_errno;\
	}\
}

/** Macro that checks a job pointer for NULL then returns an error if it is NULL */
#define ERROR_RETURN_JOB_CHECK(job) \
{\
	if ( job == NULL ) \
		return LM_NULLJOBHANDLE; \
}
	

/* 
 *	example usage:  if (LM_API_ERR_CATCH) return 0;
 */
#ifdef FLEXLM_ULTRALITE
#define LM_API_ERR_CATCH 0    /*- Daniel says only used for memory errors */ 
#else /* FLEXLM_ULTRALITE */
#define LM_API_ERR_CATCH l_clear_error(job), \
			job->flags |= LM_FLAG_CATCH_SET,\
			l_mt_lock(job, __FILE__, __LINE__), \
			setjmp(job->jobcatch)

#endif	/* FLEXLM_ULTRALITE */
#define LM_API_RETURN(t, x) {t y; y = (x); job->flags &= ~LM_FLAG_CATCH_SET; l_mt_unlock(job, __FILE__, __LINE__); return y;}
#define LM_API_RETURN_VOID() {job->flags &= ~LM_FLAG_CATCH_SET, l_mt_unlock(job, __FILE__, __LINE__); return;}
#define LM_TOOL_ERR_CATCH(job) job->flags |= LM_FLAG_TOOL_CATCH,\
			setjmp(job->tool_catch)

		


/*
 *	-------------------------------------------------------------
 *	General macros -- strcmp, opendir, etc.
 */

#define L_STREQ(s1, s2) (((s1) && (s2)) && (strcmp((s1), (s2)) == 0))
#define L_STREQ_N(s1, s2, n) (((s1) && (s2)) && (strncmp((s1), (s2), (n)) == 0))
/*
 *	Macro for doing string case in-sensitive comparison
 */
#ifdef HAS_STRICMP
#define L_STREQ_I(s1, s2) (((s1) && (s2)) && (stricmp((s1), (s2)) == 0))
#else
#define L_STREQ_I(s1, s2) (((s1) && (s2)) && (strcasecmp((s1), (s2)) == 0))
#endif
#define CHILDPID(pid) (((pid) != 0) && ((pid) != -1))
#define BADPID(pid) ((pid) == -1)

/*-
 *	-------------------------------------------------------------
 *	license-key-length stuff
 */
#define LM_OPTFLAG_LKEY_LONG        0x800 /* default False */
#define L_SECLEN_SHORT 0x66D8B337
#define L_SECLEN_LONG 0x289BEB8A
#define L_SECLEN_SET_LONG job->options->flags |= LM_OPTFLAG_LKEY_LONG; \
			job->options->sf = L_SECLEN_LONG;
#define L_SECLEN_SET_SHORT job->options->flags &= ~LM_OPTFLAG_LKEY_LONG; \
			job->options->sf = L_SECLEN_SHORT;
#define L_SECLEN_OK (((job->options->flags & LM_OPTFLAG_LKEY_LONG) && \
			(job->options->sf == L_SECLEN_LONG)) || \
			(job->options->sf == L_SECLEN_SHORT))

/*-
 *	How SECLEN stuff works.
 *	We want to make sure that the license-key length is not
 *	simply a bit that gets toggled -- otherwise it's too 
 *	easy to change.
 *	So we have a bit and a long value that have to match.
 *	If they don't match, tampering is detected.
 *
 *	The length is set by sticking a correct magic number in
 *	job->options->sf.  it must be either L_SECLEN_SHORT or L_SECLEN_LONG.
 *	Long keys are specified by having setting the
 *	LM_OPTFLAG_LKEY_LONG bit in job->options->flags and also setting
 *	job->options->sf to L_SECLEN_LONG.  
 *	Similarly, short keys are specified by having the
 *	LM_OPTFLAG_LKEY_LONG bit in job->options->flags turned off,
 *	and setting job->options->sf to L_SECLEN_SHORT.  
 *	If 
 *		o job->options->sf is not one of these values, or 
 *		o if sf is set to one value, but the 
 *		  LM_OPTFLAG_LKEY_LONG bit is set to the wrong value,
 *		  L_SECLEN_OK() will fail.
 */

#if defined ( NLM ) || !defined (PC)
#define L_S_ISDIR(x) S_ISDIR((x))
#define L_OPENDIR(x) opendir((x))
#define L_READDIR(x) readdir((x))
#define L_CLOSEDIR(x) closedir((x))
#endif

/*- 
 * so oldest format gets sent from server to client 
 * this takes priority over fmt_ver and behavior_ver. 
 */
#define LM_FLAG_PRT_CONF_OLDEST_FMT LM_FLAG_INTERNAL_USE4  
#define LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN LM_FLAG_INTERNAL_USE7 
#define LM_FLAG_MALLOC_FAIL LM_FLAG_INTERNAL_USE5  
#define LM_FLAG_CATCH_SET LM_FLAG_INTERNAL_USE6

/*
 *	-------------------------------------------------------------
 *	License finder 
 */
#define LM_FINDER_FILE "license_file"	/* License file data (port@host) */
#define LM_FINDER_PATH "lfpath"		/* Get license path */
#define LM_FINDER_GET_DLIST    "dlist"	/* Get daemon list */
#define LM_FINDER_GET_PATHS    "getpaths" /* get list of licenses server 
						is using*/

#define LMGRD_PORT_START 27000	/* FLEXlm assigned port number */
#define LMGRD_PORT_END   27009	/* up to 10 lmgrds on one system */
#define L_DEFAULT_PORTS ((job->port_end - LMGRD_PORT_START) + 1)      

/*
 *	-------------------------------------------------------------
 *	LM_A_XXX 
 */
#define LM_VD_GET_CONFIG_TYPE		107	
#define LM_VD_GET_FLIST_TYPE		108	
#define LM_VD_GET_UPDATE_KEY		109	
#define LM_OPTFLAG_CUSTOM_KEY5		LM_OPTFLAG_INTERNAL2
#define LM_OPTFLAG_BORROW_OK		LM_OPTFLAG_INTERNAL3
#define LM_VD_VERIFY_KEY		116
#define LM_VD_BORROW			117

/*
 *	-------------------------------------------------------------
 *	PORT@HOST +
 */
#define LM_PORT_HOST_PLUS_SIGN	'+'
#define LM_OLD_PORT_HOST_SIGN	'-'
/*
 *	-------------------------------------------------------------
 *	License file strings
 */
#define LM_LICENSE_VENDOR_STRING 	"VENDOR_STRING"
#define LM_LICENSE_HOSTID_STRING 	"HOSTID"
#define LM_LICENSE_COMPONENT_STRING 	"COMPONENTS"
#define LM_LICENSE_OPTIONS_STRING 	"OPTIONS"
#define LM_LICENSE_TYPE_STRING 		"TYPE"
#define LM_LICENSE_VENDOR_INFO 		"vendor_info"
#define LM_LICENSE_DIST_INFO 		"dist_info"
#define LM_LICENSE_USER_INFO 		"user_info"
#define LM_LICENSE_ASSET_INFO 		"asset_info"
#define LM_LICENSE_CKSUM 		"ck"
#define LM_LICENSE_ISSUER 		"ISSUER"
#define LM_LICENSE_NOTICE 		"NOTICE"
#define LM_LICENSE_SERIAL_STRING 	"SN"
#define LM_LICENSE_OVERDRAFT_STRING 	"OVERDRAFT"
#define LM_LICENSE_SUITE_STRING 	"SUITE"
#define LM_LICENSE_BUNDLE_STRING 	"SUITE_RESERVED"
#define LM_LICENSE_CAPACITY 		"CAPACITY"
#define LM_LICENSE_USER_BASED 		"USER_BASED"
#define LM_LICENSE_PLATFORMS 		"PLATFORMS"
#define LM_LICENSE_MINIMUM 		"MINIMUM"
#define LM_LICENSE_HOST_BASED 		"HOST_BASED"
#define LM_LICENSE_METER 		"METERED"
#define LM_LICENSE_SUPERSEDE 		"SUPERSEDE"
#define LM_LICENSE_ISSUED 		"ISSUED"
#define LM_LICENSE_DUP_STRING 		"DUP_GROUP"
#define LM_LICENSE_SUITE_DUP_STRING 	"SUITE_DUP_GROUP"
#define LM_LICENSE_TS_OK		"TS_OK"
#if 0 /* unused */
#define LM_LICENSE_POLICY		"POLICY"
#define LM_LICENSE_RESTRICTIVE		"RESTRICTIVE"
#define LM_LICENSE_LENIENT		"LENIENT"
#define LM_LICENSE_FAILSAFE		"FAILSAFE"
#endif
#define LM_LICENSE_START		"START_LICENSE"
#define LM_LICENSE_END			"END_LICENSE"
#define LM_LICENSE_START_DATE		"START"
#define LM_LICENSE_SORT			"sort"
#define LM_LICENSE_KEY 			"SIGN"
#define LM_LICENSE_BORROW		"BORROW"
#define LM_LICENSE_FLOAT_OK 		"FLOAT_OK"
/*
 *	For future use - DO NOT USE
 */
#define LM_LICENSE_LINGER_STRING "LINGER"
#define LM_LICENSE_PREREQ "PREREQ"
#define LM_LICENSE_SUBLIC "SUBLIC"
#define LM_LICENSE_DIST_CONSTRAINT "DIST_CONSTRAINT"
#define LM_LICENSE_COUNT 	"COUNT"

#define HOSTID_USER_STRING 		"USER="
#define HOSTID_HOSTNAME_STRING 		"HOSTNAME="
#define HOSTID_DISPLAY_STRING 		"DISPLAY="
#define HOSTID_STRING_STRING 		"ID_STRING="
#define HOSTID_SERNUM_ID_STRING 	"ID="
#define HOSTID_INTERNET_STRING 		"INTERNET="
#define HOSTID_FLEXID6_KEY_STRING 	"FLEXID=6-"
#define HOSTID_FLEXID1_KEY_STRING 	"FLEXID=7-"
#define HOSTID_FLEXID2_KEY_STRING 	"FLEXID=8-"
#define HOSTID_FLEXID3_KEY_STRING 	"FLEXID=9-"
#define HOSTID_FLEXID4_KEY_STRING 	"FLEXID=A-"
#define HOSTID_FLEXID_FILE_KEY_STRING 	"FLEXID=FILE-"
#define HOSTID_DISK_SERIAL_NUM_STRING 	"DISK_SERIAL_NUM="
#define HOSTID_DOMAIN_STRING 		"DOMAIN="
#define HOSTID_CPU_STRING               "CPU="
#define HOSTID_DISK_GEOMETRY_STRING	"DRIVE_GEOMETRY="
#define HOSTID_BIOS_STRING              "BIOS="
#define HOSTID_COMPOSITE_STRING         "COMPOSITE="

#ifdef DEFAULT_HOSTID_ETHER
#define L_DEFAULT_HOSTID HOSTID_ETHER
#else
#ifdef DEFAULT_HOSTID_STRING
#define L_DEFAULT_HOSTID HOSTID_STRING
#else
#define L_DEFAULT_HOSTID HOSTID_LONG
#endif /* VMS || ULTRIX */
#endif /* sco & sinix */

#define SHORTHOSTID_STRING_STRING 	"IDS=" 		/* len ?? */
#define SHORTHOSTID_DISK_SERIAL_NUM_STR "VSN=" 	/* len 12 */
#define SHORTHOSTID_FLEXID1_KEY_STRING	"FLX="		/* len 12 */
#define SHORTHOSTID_FLEXID6_KEY_STRING	"FL6="		/* len 12 */


#define MAX_CRYPT_LEN 20	/* use 8 bytes of encrypted return string to */
/*
 *	Wrapper 
 */
#define LM_LICENSE_WBINARY_STRING "w_binary"
#define LM_LICENSE_WARGV0_STRING "w_argv"
#define LM_LICENSE_WQUEUE_STRING "w_queue"
#define LM_LICENSE_WTERMS_STRING "w_term_signal"
#define LM_LICENSE_WLOSS_STRING "W_LIC_LOSS"

#define LM_FINDER_FILE "license_file"	/* License file data (port@host) */
	
/*
 *	-------------------------------------------------------------
 *	Generic Daemon
 */
#define UPGRADE_SERVER_FEATURE "flexlmd-upgrade"
#define GENERIC_SERVER_FEATURE "flexlmd-runtime"

/*
 *	-------------------------------------------------------------
 *	Development version
 */
#define FLEXLM_DEV_VER 99

/*
 *	-------------------------------------------------------------
 *	Decimal version
 */
#define L_DECIMAL_DELIM '-'
#define L_DECIMAL_ALT1_DELIM '#'
#define L_DECIMAL_ALT2_DELIM '*'
#define LM_FLAG_CKSUM_ONLY LM_FLAG_INTERNAL_USE3

/*- internal strncmp to avoid security problems with strncmp() */
#define STRNCMP(s1, s2, n, result) \
{ \
	int i; \
        result = 0; \
        if(n) { \
                if(!s1 || !*s1) { \
                        if(s2 && *s2) result = -1; \
                } else if(!s2 || !*s2) { \
                        result = 1; \
                } else { \
                        i = 0; \
                        do { \
                                if(result = (s1[i] - s2[i])) break; \
                        } while(s1[i] && s2[i] && (++i < n)); \
                } \
        } \
}

#define CONFIG_EOF (CONFIG *) -1
/*
 *	Security issues
 */
#define L_NEW_JOB_STR "l_n36_buf"
#define L_UNIQ_KEY5_FUNC l_n36_buff
#define L_UNIQ_KEY5_FUNC_STR "l_n36_buff"
#define L_SET_KEY5_FUNC l_x77_buf
#define L_SET_KEY5_FUNC_STR "l_x77_buf"
#define SEEDS_XOR mem_ptr2_bytes /*- where seeds are hidden in job */
#define SEEDS_XOR_NUM 12

#define L_SIGN_LEVEL 			env1 /* field in LM_HANDLE */
#define L_STRENGTH_OVERRIDE 		env3 /* field in LM_HANDLE */
#define L_CONF_STRENGTH_OVERRIDE 	internal1
#define L_CLIENT_KEY_REQ 		internal1
#define L_CLIENT_FLAG_HS_CB 		CLIENT_FLAG_INTERNAL1
#define L_BORROW_STRING 		env4 /* field in LM_HANDLE */
/*
 *	-------------------------------------------------------------
 *	l_date values
 */
#define L_DATE_EXPIRED 0
#define L_DATE_START_OK 1


/*-
 *	-------------------------------------------------------------
 *	P3130
 */
#define LM_OPTFLAG_P3130 0x4000 /* LM_OPTFLAG_INTERNAL1 */

/*
 *	-------------------------------------------------------------
 *			CONF-FLAG-VALUES 
 */
#define L_CONF_FLAGS		reg1 /* obfuscated name */
#define L_CONF_FL_VERIFIED	0x1			
#define L_CONF_FL_MAKE_KEY_OLD	0x2 /* used by lc_cryptstr */
#define L_CONF_FL_OLD_KEY	0x4			
#define L_CONF_FL_REJECT	0x8			
/*
 *	-------------------------------------------------------------
 *	job->flags bit values
 */

#define LM_FLAG_INRECEIVE			0x00000001		
#define LM_FLAG_RECONNECTING		0x00000002
#define LM_FLAG_IS_VD				0x00000004
#define LM_FLAG_GENERIC_SERVER		0x00000008
#define LM_FLAG_LMUTIL				0x00000010
#define LM_FLAG_LMGRD				0x00000020
#define LM_FLAG_CHECKOUT			0x00000040
#define LM_FLAG_INTERNAL_USE1		0x00000080 /*- 2 flags for debugging */
#define LM_FLAG_INTERNAL_USE2		0x00000100 /*- l_baddate.c */
#define LM_FLAG_INTERNAL_USE3		0x00000200 /*- l_cksum */
#define LM_FLAG_IN_CONNECT	        0x00000400 /*- l_basic_conn */
#define LM_FLAG_FEAT_FOUND	        0x00000800 /*- lc_next_conf */
#define LM_FLAG_INTERNAL_USE4		0x00001000 /*- LM_FLAG_PRT_CONF_OLDEST_FMT */
#define LM_FLAG_INTERNAL_USE5		0x00002000 /*- LM_FLAG_MALLOC_FAIL */
#define LM_FLAG_INTERNAL_USE6		0x00004000 /*- LM_FLAG_CATCH_SET */
#define LM_FLAG_TOOL_CATCH			0x00008000 /* longjmp if fatal error */
#define LM_FLAG_LMDIAG				0x00010000 /* diag's checkout */
#define LM_FLAG_BORROWED_CHECKOUT	0x00020000 /*	Set when the last checkout resulted in borrow init */
#define LM_FLAG_LIC_GEN				0x00040000 /*- This program is a license 
				 		generator */
#define LM_FLAG_MAKE_OLD_KEY		0x00100000 /*- set only when we're gen KEY2*/
#define LM_FLAG_MAKE_PHASE3_KEY		0x00200000 /*- set only when we're gen KEY2*/
#define LM_FLAG_MAKE_PUBLIC_KEY		0x00400000 /*- set only when we're gen KEY2*/
#define LM_FLAG_CLEAR_VKEYS			0x00800000 
#define LM_FLAG_SLOW_VERIFY         0x01000000 /*- recheck key for same conf */
#define LM_FLAG_LMUTIL_PRIV			0x02000000 /*- privileged commands
				 		lmreread/lmdown */
#define LM_FLAG_INTERNAL_USE7       0x04000000 /*- LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN */
#define LM_FLAG_NEED_REREAD			0x08000000 /*- need to read license file(s) */
#define LM_FLAG_CONNECT_NO_HARVEST	0x10000000 /* l_conn_harvest -- do nothing
						 This is so we can connect
						 to the default ports once,
						 and keep the connections
						 alive through several 
						 l_connect() calls -- this
						 makes it much more efficient
						 in certain cases */
#define LM_FLAG_CONNECT_NO_THREAD	0x20000000 /* connect 27000, not default */
#define LM_FLAG_LOG_UD_ON_FAILURE	0x40000000	/* Ultimate denial */

/*
 *	-------------------------------------------------------------
 *	rc_vars 
 */

typedef struct _l_key_filter {
	char *app_filter;
#define LM_KF_RESULT_SUCCESS 	0  /* >=0 means success */
#define LM_KF_RESULT_FAIL 	-1 /* < 0 means failure * */
	char *gen_filter;
	int pubkeysize[LM_PUBKEYS];
	unsigned char pubkey[LM_PUBKEYS][LM_MAXPUBKEYSIZ];
	int pubkeyinfo1;
	LM_VENDORCODE_PUBKEYINFO *pubkeyinfo;
	int sign_level;
	int flags;
#define LM_KF_PREHASH 	0x1 /* If set, raw variable string given to filter */
#define LM_KF_ASYMMETRIC 0x2 /* If set, app_filter reverses key_filter */
#define LM_KF_BYTE 	0x4 /* If set, filter one byte at a time, 
				must be !PREHASH */
#define LM_KF_DIG_SIG 	0x8 /* Digital Signature:
				implies that gen_filter returns
				0 (success) or errno */
	struct _l_key_filter *next;
} L_KEY_FILTER;

#define L_KEY_BYTE_TO_STRLEN(bytes) ((bytes * 2) + (bytes/2) + 1)
#define L_MAXPUBKEYSIZ 256


#ifdef UNIX
#define LM_REGISTRY_FILE ".flexlmrc"
#define LM_BORROW_FILE ".flexlmborrow"
#endif

#ifndef FLEXLM_ULTRALITE	/* Ultralite has it's own free */
#define free(x) l_free(x)
#endif	/* !FLEXLM_ULTRALITE */

#if 0 /* def PC */
typedef struct _lm_conninfo {
	int sock; 
	unsigned short portnum;
	long thread;
	int thread_idx; /* if we're in a thread, then conninfo[0].thread_idx
	                    tells us which thread we're in */
        LM_HANDLE *job; 
        char msg[LM_MSG_LEN + 1]; 
        COMM_ENDPOINT endpoint; 
        char *hostname;
        char resp[LM_MSG_LEN + 1];
        LM_ERR_INFO err_info;
        int idx; /* which one are we */
} LM_CONNINFO;
#endif

#define LM_A_V7_0_HANDSHAKE LM_A_INTERNAL2 /* to turn off enhanced handshake 
					      allows client to talk to 
					      different vendor daemons with
					      same seeds
					      */
#define L_PUBKEY79BIT_LEN 40
#define L_PUBKEY97BIT_LEN 50
#define L_PUBKEY113BIT_LEN 60
#define L_PUBKEY163BIT_LEN 84
#define L_PUBKEY239BIT_LEN 120


#ifndef PC
/* 
 * 	use our own gethostname, because the header files are different 
 *	across unixes and this was breaking our builds.
 */
#define gethostname(x, y) l_gethostname(x, y) 
#endif /* PC */

/* lmdown flags argument values */
#define LM_DOWN_PRINT 0x1
#define LM_DOWN_FORCE 0x2

#define LM_ULF_BORROWED	0x1

#endif /* _L_PRIVAT_H_ */
