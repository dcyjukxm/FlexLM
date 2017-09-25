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
 *	Module: $Id: ls_adaem.h,v 1.2 2003/01/13 22:22:33 kmaclean Exp $
 *
 *	Description: Local definitions for license manager application daemons.
 *
 *	M. Christiano
 *	3/13/89
 *
 *	Last changed:  12/10/98
 *
 */

#define CLIENT_CHECK 60		/* Client socket check interval (in secs.) */
#define CLIENT_SILENT 3600	/* Allow a client to be silent for an hour
					before sending a msg to him */

extern FEATURE_LIST *ls_flist;	/* Which feature names we serve */
extern VENDORCODE *vendorkeys;/* Table of possible encryption codes */
extern char *(*ls_user_crypt)(); /* Vendor-supplied encryption */
extern char *prog;		/* Our program path */
extern int *server_tab;		/* Table of server PIDs */
extern int ls_use_featset;	/* Use FEATURESET line from license file */
extern char *ls_dup_sel;	/* Duplicate selection criteria to emulate
					count_duplicates for pre-v2 clients */
#define BORROW_3	"mp/.bo"
#define BORROW_2	"r/t"
#define BORROW_1	"/us"
#include "ls_aprot.h"
