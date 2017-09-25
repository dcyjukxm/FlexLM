/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_fdata.h,v 1.4 2003/01/13 22:13:12 kmaclean Exp $
 *
 *	D. Birns
 *	7/30/95
 *
 *	Last changed:  10/23/96
 *
 */

typedef struct _feature {
			struct _feature FAR *next;	/* Forward link */
			char f[MAX_FEATURE_LEN+1];
			char ver[MAX_VER_LEN+1];	/* Which version */
			int n;			/* How many licenses he has */
			LM_SOCKET s;		/* Socket it is on */
			int serialno;		/* Serial # of the socket */
			VENDORCODE key;		/* Vendor key */
			short int status;	/* Status of feature */
#define STAT_DIED 1				/* Server died */
#define STAT_NODELOCKED 2			/* Feature is nodelocked */
#define STAT_INQUEUE 4				/* We are in the queue */
#define STAT_BORROWED 8				/* We are in the queue */
			short int dup_select;	/* Duplicate criteria */
			CONFIG *conf;		/* License file line */
			char vendor_checkout_data[MAX_VENDOR_CHECKOUT_DATA+1];	
						/* vendor-defined data */
		      } FEATDATA;

#define JOB_FEATDATA ((FEATDATA *)job->featdata)
