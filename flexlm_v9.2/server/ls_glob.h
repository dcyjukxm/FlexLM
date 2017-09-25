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
 *	Module: $Id: ls_glob.h,v 1.5 2003/01/13 22:31:36 kmaclean Exp $
 *
 *	Description: Global data from the license server.
 *
 *	M. Christiano
 *	3/9/88
 *
 *	Last changed:  10/23/96
 *
 */

extern int topdog;		/* Who is the top-level server */
				/* In CLIENT_DATA struct:	*/
#define DESC_READ 0		/* 	descriptor 0 is the read descriptor */
#define DESC_WRITE 1		/* 	and 1 is the write descriptor */
extern int dlog_on;		/* Debug logging ON/OFF flag */
extern int ls_conn_timeout;	/* How long to wait for another server 
								to come up*/
