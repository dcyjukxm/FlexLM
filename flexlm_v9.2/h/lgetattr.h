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
/*-	
 *	Module: $Id: lgetattr.h,v 1.24 2003/05/31 03:13:10 jonl Exp $
 *
 *	Description: 	Key/value pairs for l_getattr() call
 *
 *	M. Christiano
 *	8/29/90
 *
 *	Last changed:  8/17/98
 *
 */

/*-
 *	Keys (MUST be in same order as table in l_getattr.c)
 */
#define DUP_SERVER	1	/*- Redundant server support */
#define EXTENDED_HOSTID 2	/*- -> ADVANCED_HOSTIDS - all but ANY, dongle */
#define ADVANCED_HOSTIDS 2	/*- -> ADVANCED_HOSTIDS - all but ANY, dongle */
#define CLOCK_SETTING	3
#define SWITCH_LOGFILE	4	/*- -> UPGRADE_INCREMENT - enables INCREMENT */
#define UPGRADE_INCREMENT	4
				/*-			and UPGRADE */
#define LOG_SUPPORT	5
#define LF_PATH		6	/*- -> LICENSE_SEEDS: enables your own */
				/* 		license seeds */
#define LICENSE_SEEDS		6	
#if 0
#define USERNAME_SUPPORT 7
#define LM_HOSTNAME	8
#define LM_DISPLAY	9
#define LM_AUTH_DATA	11
#endif
#define LM_DONGLE1 	7
#define LM_DONGLE2 	8
#define LM_DONGLE3 	9
#define LM_HOSTTYPE	10
#define LM_DONGLE4 	11
#define DAEMON_INIT	12
#define END_USER_OPTIONS 13	/*- All server options file stuff, except */
				/*- REORTLOG; reduced REPORTLOG functions */
#define LMADMIN_API	14
#define LM_RUN_DAEMON	15
#define REREAD		16
#define START_DATE	17
#define GROUP_DUP	18
#define FEATURESET	19
#define NO_EXPIRE	20
#define REAL_KEYS	NO_EXPIRE /* otherwise, eval keys */
#define ULIST		21
#define FULL_FLEXLM	22
#define CHECKOUT	23
#define FILTERS		24
#define LINGERING	25
#define ADDITIVE	26
#define WRAPPER		27
#define LM_SET_ATTR	28
#define MULTIPLE_JOBS	29
#define FLEXCRYPT	30
#define LM_PUBKEY_ATTR	FLEXCRYPT	/*-reuse FLEXCRYPT bit  */
#define LM_PHASE2_ATTR	LM_PUBKEY_ATTR
#define FLEXlm_SOURCE	31	/*- Source licensee: allows builds on
					unsupported platforms */

/* 
 *	Old definitions which are equivalent to END_USER_OPTIONS
 */

#define OPT_INCLUDE	END_USER_OPTIONS
#define OPT_EXCLUDE	END_USER_OPTIONS
#define OPT_TIMEOUT	END_USER_OPTIONS
/*-
 *	Key values (Should be random, MUST be non-zero, short)
 */
#define DUP_SERVER_VAL 		32 		/* 1 */
#define EXTENDED_HOSTID_VAL 	0x3e1 		/* 2 */
#define ADVANCED_HOSTIDS_VAL 	0x3e1 		/* 2 */
#define CLOCK_SETTING_VAL	0xabcd 		/* 3 */
#define SWITCH_LOGFILE_VAL	84 		/* 4 */
#define UPGRADE_INCREMENT_VAL	84 		/* 4 */
#define LOG_SUPPORT_VAL		94 		/* 5 */
#define LF_PATH_VAL		127 		/* 6 */
#define LICENSE_SEEDS_VAL	127 		/* 6 */
#if 0
#define USERNAME_SUPPORT_VAL	20123 		/* 7 */
#define LM_HOSTNAME_VAL		199 		/* 8 */
#define LM_DISPLAY_VAL		5432 		/* 9 */
#define LM_AUTH_DATA_VAL	12 		/* 11 */
#endif
#define LM_DONGLE1_VAL		20123 		/* 7 */
#define LM_DONGLE2_VAL		199 		/* 8 */
#define LM_DONGLE3_VAL		5432 		/* 9 */
#define LM_DONGLE4_VAL		12 		/* 11 */
#define LM_HOSTTYPE_VAL		9912 		/* 10 */
#define DAEMON_INIT_VAL		974 		/* 12 */
#define END_USER_OPTIONS_VAL	555 		/* 13 */
#define LMADMIN_API_VAL		0xdead 		/* 14 */
#define LM_RUN_DAEMON_VAL	0xfeef 		/* 15 */
#define REREAD_VAL		70 		/* 16 */
#define START_DATE_VAL		221 		/* 17 */
#define GROUP_DUP_VAL		112 		/* 18 */
#define FEATURESET_VAL		5542 		/* 19 */
#define NO_EXPIRE_VAL		9813 		/* 20 */
#define REAL_KEYS_VAL		NO_EXPIRE_VAL 	/* 20 */
#define ULIST_VAL		1297 		/* 21 */
#define FULL_FLEXLM_VAL		0x1204 		/* 22 */
#define CHECKOUT_VAL		0x2a34 		/* 23 */
#define FILTERS_VAL		0x12ef 		/* 24 */
#define LINGERING_VAL		0x9320 		/* 25 */
#define ADDITIVE_VAL		0x7032 		/* 26 */
#define WRAPPER_VAL		0x836a 		/* 27 */
#define LM_SET_ATTR_VAL		0x336 		/* 28 */
#define MULTIPLE_JOBS_VAL	0x9969 		/* 29 */
#define FLEXCRYPT_VAL		0xd30f 		/* 30 */
#define LM_PUBKEY_ATTR_VAL  	FLEXCRYPT_VAL	/* 30 */
#define LM_PHASE2_ATTR_VAL 	LM_PUBKEY_ATTR_VAL	/* 30 */

#define OPT_INCLUDE_VAL	END_USER_OPTIONS_VAL
#define OPT_EXCLUDE_VAL	END_USER_OPTIONS_VAL
#define OPT_TIMEOUT_VAL	END_USER_OPTIONS_VAL

/*-
 *	Platform support (Update PLAT_ALL when making any changes)
 */
/* These first set of bits on the top are all platform 1 in lmvkey.c and 
 *  keys[1] in l_getattr.c
 */
#define PLAT_it64_hp     ((long) 1<<16)
#define PLAT_it64_lr	((long) 1<<17)
#define PLAT_it64_n	((long) 1<<18)
#define PLAT_it32_hp	((long) 1<<19)
#define PLAT_it32_lr	((long) 1<<20)
#define PLAT_ppc_lynx	((long) 1<<21)  /* was motorola */
#define PLAT_netware	((long) 1<<22)
#define PLAT_it32_n	((long) 1<<23)
#define PLAT_rs6000	((long) 1<<24)
#define PLAT_sgi	((long) 1<<25)
#define PLAT_sun	((long) 1<<26)
#define PLAT_it64_a	((long) 1<<27)
#define PLAT_vms	((long) 1<<28)
#define PLAT_it32_a	((long) 1<<29)
#define PLAT_sco	((long) 1<<30)
#define PLAT_it64_lv	((long) 1<<31)
/*- Bottom 16 bits available (expdate moved to third key in FLEXlm v2.2 */
#define PLAT_hp700	((long) 1<<15)
#define PLAT_sony_news	((long) 1<<14)
#define PLAT_it32_lv 	((long) 1<<13)
#define PLAT_rs64       ((long) 1<<12)
#define PLAT_ncr	((long) 1<<11)
#define PLAT_sunx86	((long) 1<<10)
#define PLAT_alpha_vms	((long) 1<<9)
#define PLAT_alpha_osf	((long) 1<<8)
/*- Word 3: more platform bits: moved in v2.7 */
/* These second set of bits on the are all platform 2 in lmvkey.c and 
 *  keys[2] in l_getattr.c
 */
#define PLAT_winnt_alpha ((long) 1<<31)
#define PLAT_winnt_intel ((long) 1<<30)
#define PLAT_winnt_mips	((long) 1<<29)
#define PLAT_ppc_mac ((long) 1<<28)
#define PLAT_siemens_mips ((long) 1<<27)
/*#define PLAT_icl_intel ((long) 1<<26)*/
#define PLAT_sun64 	((long) 1<<26)
/*#define PLAT_unused8 	((long) 1<<25)*/
#define PLAT_sun32		((long) 1<<25)	/* 32 bit Solaris built on Solaris 7 or higher */
#define PLAT_dg_intel 	((long) 1<<24)
#define PLAT_nec_ews 	((long) 1<<23)
#define PLAT_windows 	((long) 1<<22)
#define PLAT_unixware 	((long) 1<<21)
#define PLAT_sgi_r8000 	((long) 1<<20)
#define PLAT_vxworks 	((long) 1<<19) /* VxWorks for the simulator */
#define PLAT_unused10 	((long) 1<<18)
#define PLAT_linux 	((long) 1<<17) 
#define PLAT_winnt_ppc 	((long) 1<<16)
#define PLAT_aix_ppc 	((long) 1<<15)
#define PLAT_cray_ymp 	((long) 1<<14) /* left for dead */
#define PLAT_openbsd 	((long) 1<<14) /* Custom Port for Ridgeway */
#define PLAT_cray_t90 	((long) 1<<13)
#define PLAT_cray_c90 	((long) 1<<12)
#define PLAT_java 	((long) 1<<11)
#define PLAT_os2 	((long) 1<<10)
#define PLAT_bsdi 	((long) 1<<9)
#define PLAT_unused11 	((long) 1<<8)
#define PLAT_alpha_linux ((long) 1<<7)
#define PLAT_cray_t3e 	((long) 1<<6) /* left for dead */
#define PLAT_montavista	((long) 1<<6)
#define PLAT_cray_j90 	((long) 1<<5) /* left for dead */
#define PLAT_cray_nv1 	((long) 1<<5)
#define PLAT_intel_lynx ((long) 1<<4)
#define PLAT_hp64 	((long) 1<<3)
#define PLAT_necsx4 	((long) 1<<2)
#define PLAT_freebsd 	((long) 1<<1)
/* shift 0 I believe doesn't work -- Daniel */


	/*- Word3 platform bits */
#define PLAT_ALL_2  0xffffffff

	/*- Word2 platform bits */
#define PLAT_ALL    0xffffff80

#define JUNK 0xa3ef0000	/*- Just uses the top 16-bits */
