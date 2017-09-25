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
 *	Module: $Id: ls_data.c,v 1.24 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function:  None
 *
 *	Description: Global data for the license manager servers.
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  8/2/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"


#ifdef RELEASE_VERSION
int dlog_on = 0;		/* Debug logging , off for release*/
#else
int dlog_on = 1;		/* Debug logging , on for development */
#endif
char *ls_serv_down_notify = (char *) NULL;
				/* Whom to notify on "SERVER DOWN" conditions */
char *ls_new_master_notify = (char *) NULL;
				/* Whom to notify on "NEW MASTER" conditions */
char ls_our_hostname[MAX_HOSTNAME+1];	/* Our hostname, from license file */
LM_SOCKET ls_udp_s = LM_BAD_SOCKET;	/* UDP socket */
int _ls_fifo_fd;			/* FIFO fd */
LM_HANDLE *lm_job;		/* Server's one (and only) job */

/*
 *	Variables that are used to control the multiple-vendor-daemon hierarchy
 */
