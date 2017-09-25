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
 *	Module: $Id: lssignal.h,v 1.5 2003/01/13 22:31:36 kmaclean Exp $
 *
 *	Description: Definitions for signals used in the server processes.
 *
 *	M. Christiano
 *	4/5/88
 *
 *	Last changed:  12/10/98
 *
 */

/*
 *	Everything that includes this file has already included
 *	lsserver.h, but just in case...
 */

#ifndef SIGUSR1
#include <signal.h>
#endif


#define LOST_CONNECTIONS SIGUSR2	/* Lost connection to other servers */
#define SHUTDOWN_SIGNAL SIGTRAP		/* SHUTDOWN this server */
#define REREAD_SIGNAL SIGVTALRM		/* REREAD license file */
#ifdef MOTOSVR4
#undef REREAD_SIGNAL
#define REREAD_SIGNAL 37		/* header file is screwed up!  */
#endif
#ifdef PC
#define SIGTRAP (NSIG+1)
#else /* PC */
#include <sys/signal.h>
#endif /* PC */
#ifndef NSIG /* some ANSI systems don't define this, so I do it by hand */
#ifdef SGI
#define NSIG 33
#endif
#if defined(SUNOS5) && defined(GCC)
#define NSIG 34
#endif
#ifdef NLM		
#define NSIG _SIGLAST
#endif /* NLM */
#ifdef OS2 
#define NSIG _SIGMAX
#endif /* OS2 */
#endif

/*
 *	When we need more signals, we should use: SIGQUIT
 */
