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
 *	Module: $Id: l_pack.h,v 1.9 2003/01/13 22:13:12 kmaclean Exp $
 *
 *	D. Birns
 *	7/14/97
 *
 *	Last changed:  9/30/98
 *
 */

unsigned long l_unpack	lm_args((unsigned char *, int *, int));
void l_pack		lm_args((unsigned char *, int *, int, long));
void l_pack_print	lm_args((unsigned char *, int, char *));
int l_pack_unprint	lm_args((LM_HANDLE *job, 
				unsigned char *istr, unsigned char *));

/*
 *	Storing integers
 */
#define LM_CDLEN_NUM_BITS 3	
#define LM_CD_NUM_1BIT	0 	/* good for 0 and flags */
#define LM_CD_NUM_4BIT	1
#define LM_CD_NUM_8BIT	2
#define LM_CD_NUM_12BIT	3
#define LM_CD_NUM_16BIT	4
#define LM_CD_NUM_24BIT	5
#define LM_CD_NUM_31BIT	6	/* for signed positive 32-bit its */
#define LM_CD_NUM_32BIT	7

/*
 *	Length of each config element
 */
/*
 *	--------------------------------------------------------------
 *	Fixed Section #1
 */
#define LM_CDLEN_FMTVER		3
#define LM_CDLEN_CONFTYPE	2
#define LM_CDLEN_KEYLEN		2
#define LM_CDLEN_EXPFLAG	1
#define LM_CDLEN_CNTDFLAG	1
#define LM_CDLEN_HOSTIDFLAGS	2
#define LM_CDLEN_GOT_OPTS_FLAG	1

/* 
 *	--------------------------------------------------------------
 *	values for Section #1 
 */
/*
 *	Possible license key lengths
 */
#define LM_CDKEYLEN_0		0
#define LM_CDKEYLEN_40		1
#define LM_CDKEYLEN_48		2
#define LM_CDKEYLEN_64		3

/*
 *	HOSTID FLAGS
 */
#define LM_CD_NOHOSTID		0
#define LM_CD_DEMOHOSTID	1
#define LM_CD_THISHOSTID	2
#define LM_CD_REALHOSTID	3

/*
 *	--------------------------------------------------------------
 *	Section 3, variable
 */
#define LM_DEFAULT_START_DATE	"C201" /* 1-jan-1997 */

/*
 *	--------------------------------------------------------------
 *	Section 4, name=value pair info, start at 1, so 0 is the
 *	terminator...
 */
#define LM_CDLEN_ID		5		/* Length of the id # */

/*
 *	Start-Date
 */
#define LM_CDID_START_DATE 	1		/* id #1 */
#define LM_CDLEN_START_DATE 	16

#define LM_CDID_VERSION 	2		/* id #2 */
#define LM_CDLEN_VER_DECPT	4		/* decimal point offset */
/*
 *      IMPORTANT NOTE:  It's possible to have version string that
 *                      won't fit.  This has to be reported as a
 *                      conversion error when converting TO decimal.
 *                      10 digits with no decimal is a legal version, but
 *                      is not 32-bit int.
 */

#define LM_CDID_VENDOR_STRING 	3		/* id #3 */

#define LM_CDLEN_STRING_STORE	2	
#define LM_CDLEN_STORE_HEX	1	
#define LM_CDLEN_STORE_UPPER	1	
#define LM_STRING_STORE_STRING	0
#define LM_STRING_STORE_BYTE	1
#define LM_STRING_STORE_16BIT	2
#define LM_STRING_STORE_32BIT	3
#define LM_STRING_STORE_DEC	0

#define LM_CDLEN_STRING_LEN	9	/* 9-bits == max 2^9 = 512
						   len in bytes.  */

/*
 *	For hostid lists: dump them one at a time, and make
 *	a list upon parse...
 */
#define LM_CDID_HOSTID 		4		/* id #4 */
#define LM_CD_HOSTID_VENDOR	35
/*
 *	IMPORTANT NOTE:  Vendor hostids start at 35, to save
 *	space, instead of 1000
 */
#define LM_CDLEN_HOSTID_TYPE	6	/* 0-63 hostid types, vdef start
					   at 35, leaving 29 vdef 
					   hostid types */
#define LM_CDLEN_HOSTID_LEN	6	/* MAX_HOSTID_LEN is ~41 bytes */

/*
 *	lc_got_options
 */
#define LM_CDID_GOT_OPTIONS 	5		/* id #5  */
#define LM_CDLEN_GOT_OPTIONS	12	/* v6 -- only 10 options -- 
					   2 to grow on*/
#define LM_CD_DUP_NONE		0x10	
#define LM_CDLEN_CKSUM		8
#define LM_CDLEN_OPTIONS	4	/* defined as char, but only 4 vals */

#define LM_CDLEN_TYPE		8	/* unsigned char */

#define LM_CDID_VENDOR_INFO 	6		/* id #6 */
#define LM_CDID_DIST_INFO 	7		/* id #7 */
#define LM_CDID_USER_INFO 	8		/* id #8 */
#define LM_CDID_ASSET_INFO 	9		/* id #9 */
#define LM_CDID_ISSUER 		10		/* id #10 */
#define LM_CDID_NOTICE 		11		/* id #11 */
#define LM_CDID_PLATFORMS	12		/* id #12 */
#define LM_CDLEN_NUM_PLATFORMS	8		/* max 256 platforms */

#define LM_CDID_SERIAL		13		/* id #13 */
#define LM_CDID_ISSUED		14		/* id #14 */
#define LM_CDID_USER_BASED	15		/* id #15 */
#define LM_CDLEN_USER_BASED	10		/* 2^10 = 1024 */

#define LM_CDID_MINIMUM		16		/* id #16 */
#define LM_CDLEN_MINIMUM	13		/* 2^13 = 8192 */

#define LM_CDID_HOST_BASED	17		/* id #17 */
#define LM_CDLEN_HOST_BASED	16		/* 2^10 = 1024 */

#define LM_CDID_SUPERSEDE_LIST	18		/* id #18 */
#define LM_CDLEN_NUM_SUPERSEDE	8		/* max 256 superseded feats */

#define LM_CDID_SERVER_HOSTID	19		/* id #19 */

#define LM_CDID_SERVER_PORT	20		/* id #20 */
#define LM_CDLEN_SERVER_PORT	16		/* short */

#define LM_CDID_FROMVERSION 	21		/* id #21 */
#define LM_CDID_STARTDATE_ATTR 	22		/* id #22 */
#define LM_CDID_METER_BORROW_INFO 23		/* id #23 */
#define LM_CDID_KEY2 		24		/* id #24 */
#define LM_CDID_BORROW 		25		/* id #25 */
#define LM_CDLEN_KEY2 		16		/* short */
