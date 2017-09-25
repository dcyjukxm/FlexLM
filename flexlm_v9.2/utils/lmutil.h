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
 *	Module: $Id: lmutil.h,v 1.5 2003/01/13 21:58:49 kmaclean Exp $
 *
 *	Function:
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  10/25/98
 *
 */
#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif
#include "lmachdep.h"
#include <stdio.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "l_openf.h"
#include "utilscode.h"
#include "lm_attr.h"
#include <sys/types.h>
#ifdef PC
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <winsock.h>
#ifdef LMTOOLS
#undef fprintf
#define fprintf LogText
void LogText( FILE * tmp,char * fmt, ... );
#endif /* LMTOOLS */
#else
#ifndef apollo
#include <netinet/in.h>
#endif /* apollo */
#endif /* PC */
#include "lsmaster.h"
#ifdef MOTO_88K
#include "lm_select.h"
#endif
#include <signal.h>
/*#include <fcntl.h>*/
#include <time.h> 
#ifndef PC
#include <sys/time.h>
#endif
#ifndef  NO_UIO_H
#include <sys/uio.h>
#endif
/* #include <sys/socket.h> 
#include <netdb.h>  */
#include <errno.h>
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif
#if defined(HP) || defined(PCRT) || defined(MOTO_88K)
#include <sys/file.h>
#endif


extern LM_HANDLE *lm_job;
extern FILE *ofp;
extern int diag_interactive;
extern void (*dummy)();
extern char *myname;
extern int printheader;
extern int verbose;
extern char l_vendor[];
extern int exit_code;
void license_info lm_args(( int, CONFIG *));
lm_extern void lc_featspec 	lm_args((LM_HANDLE *, char *,char **, char **));
lm_extern CONFIG_PTR lc_findfeat lm_args(( LM_HANDLE *, char *feat, char *, 
							CONFIG *));
lm_extern LMGRD_STAT * l_select_lmgrd lm_args(( LMGRD_STAT **));
#define CMD_INVALID	0
#define	CMD_LMCKSUM	1
#define CMD_LMDOWN	2
#define CMD_LMHOSTID	3
#define CMD_LMREMOVE	4
#define CMD_LMREREAD	5
#define CMD_LMSWITCH	6
#define CMD_LMSWITCHR	7
#define CMD_LMSTAT	8
#define CMD_LMVER	9
#define CMD_LMBORROW	10
#define CMD_LMDIAG	11
#define CMD_LMNEWLOG	12

#define ALL_LMGRDS (LMGRD_STAT *)1
