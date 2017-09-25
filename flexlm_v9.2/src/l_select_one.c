/******************************************************************************

	    COPYRIGHT (c) 1991, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_select_one.c,v 1.14 2003/01/14 21:47:05 kmaclean Exp $
 *
 *	Function:	l_select_one(fd, read, timeout)
 *
 *	Description: 	Selects a single file descriptor
 *
 *	Parameters:	(int) fd - File descriptor to select
 *			(int) read - 1 -> select on read, 0 -> select on write
 *			(int) timeout - Select timeout value in msec
 *				> 0 == timeout or L_SELECT_BLOCK or 
 *					L_SELECT_NONBLOCK (0)
 *
 *	Return:		(int) return from select
 *
 *	M. Christiano
 *	9/23/91
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifdef USE_WINSOCK
#include <pcsock.h>
#endif
#include "lmselect.h"
#include "l_time.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif
static void set_timeout lm_args((int, struct timeval *));

static SELECT_MASK j;
static int first = 1;	/* Only create the select mask the first time */

#ifdef PC 
static fd_set k1;
#endif
int
API_ENTRY
l_select_one(fd, readflag, timeout)
int fd;
int readflag; 	/* if -1, use select to sleep */
int timeout;
{
  struct timeval *t, tt;
  int i = 0;
  long now, now2;
  int saved_errno;
#ifdef getenv
#undef getenv
#endif
	DEBUG_INIT
	DEBUG(("lmnofile %d timeout %d %s time %d\n", lm_nofile, timeout, readflag ? "read" : "write",
		 time(0) ));
	if (first)
	{
#ifdef PC
		j=(int *) &k1;
#else
		MASK_CREATE(j);
#endif
		first = 0;
	}
	if (timeout == L_SELECT_BLOCK)
		t = (struct timeval *) NULL;
	else
		t = &tt;
	MASK_INIT(j, fd);
	i=0;
	saved_errno = NET_EINTR;
	now = l_now();
	while ((readflag == -1 && timeout >= 0) ||
		((saved_errno == NET_EINTR)  && (timeout >= 0 ||
		((timeout == L_SELECT_BLOCK) && !i))))
	{
	    saved_errno = 0;
	    
	    set_timeout(timeout, &tt);
	    switch (readflag)
	    {
	    case 1:
		i = l_select(lm_nofile, (int *)j, (int *)0, (int *)0, t);
		break;
	    case 0:
		i = l_select(lm_nofile, (int *) 0, (int *)j, (int *) 0, t);
		break;
	    case -1:
#if 1 		/* v7 -- this seems needless, so I *considered* removing it */
		t->tv_usec = 500;
		t->tv_sec = 0;
#endif
		/*printf("timeout is %d\n", timeout);*/
		i = l_select(0, (int *) 0, (int *)0, (int *) 0, t);
	    }
	    if (i < 0) saved_errno = net_errno; /* only reset upon error */
	    now2 = l_now();
	    timeout -= (int)(now2 - now);
	    now = now2;
	}
/* ******************************************************************/
/*
 * 	SPECIAL WINDOWS 95 KLUDGE 
 *	Select is sometimes returning the wrong status
 *	It is saying it is timing out when it really isnt,
 *	so if it stops when it didnt timeout, we say the data is ready
 */
#ifdef PC

#define FUDGE_FACTOR 10 /* milliseconds */
        if (i==0)
	{         /* special case for win95 */
		if (timeout >=FUDGE_FACTOR )       return 1;
	}

/* ******************************************************************/
#endif /* PC */

	if (ANY_SET((int *)j) == 0) i = 0;	/* Nothing set => none ready */

#ifdef PC16
	/*
	 *	Trying to get around Wollongong PathWay WinSock DLL select()
	 *	problem.  A bug report has been filed with Wollongong.  Once
	 *	PathWay 3.0 become obselete, this patch can then be removed.
	 */

	if ( getenv("PATHWAY_PATCH") )
		return 1;
#endif
	if (i==0 && readflag != -1) 
	{
		DEBUG(("select returning 0 %d %s time %d\n", timeout, readflag ? "read" : "write", time(0)));

	}
	{
	  extern int errno;
		DEBUG(("select returning %d %d\n", i, errno));
	}
  
	
	return(i);
}
static
void
set_timeout(mlsec, tval)
int mlsec;
struct timeval *tval;
{
	tval->tv_sec = mlsec / 1000;
	tval->tv_usec = 1000 * (mlsec % 1000);

#ifdef PC16
	/*
	 *	Trying to get around Wollongong PathWay WinSock DLL select()
	 *	problem.  A bug report has been filed with Wollongong.  Once
	 *	PathWay 3.0 become obselete, this patch can then be removed.
	 */
	if ( getenv("PATHWAY_PATCH") )
	{
		tval->tv_sec = 1;
		tval->tv_usec = 0;		
	}
#endif
}

