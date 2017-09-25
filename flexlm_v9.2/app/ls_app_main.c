/******************************************************************************

	    COPYRIGHT (c) 1992, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_app_main.c,v 1.6.4.1 2003/06/25 21:25:25 sluu Exp $
 *
 *	Function: main() - main program for application daemons
 *
 *	Description:	The main program for application daemons.
 *
 *	Parameters:	None.
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	5/25/92
 *
 *	Last changed:  1/2/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "ls_sprot.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "ls_aprot.h"
#include "flex_utils.h"

void ls_i_fhostid lm_args((FEATURE_LIST *));
#if defined(PC) && (!defined(NLM))
extern char * lm_flexlm_dir;
#endif

#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

#if defined (PROFILE_WINNT)
#include <process.h>
  unsigned long thread_ptr;

void FlexMsgBox ()
{
        MessageBox(0,"Press OK when ready to exit","Flexible License Manager",
                        MB_OK | MB_TASKMODAL  );
        _exit(1);
}

#endif /* PROFILE WINNT */


int
main(int argc, char * argv[])
{
#ifdef USE_WINSOCK
	WSADATA wsadata;
#endif /* USE_WINSOCK */
#if defined( PC ) && (!defined(NLM))
	char	buf1[MAX_PATH] = {'\0'};
#endif
	char **	flexArgv = NULL;
	int		flexArgc = 0;


#if defined( PC ) && (!defined(NLM)) && (!defined(OS2))

/*
 *      Since lmgrd on Windows NT may be run as an application or as a
 *      system Service, and a Service program does not have stdout and stdin,
 *      we must redirect the ascii log to the right place.
 */


	GetWindowsDirectory(buf1, 200);

	if (buf1[0]=='A')
	{
		lm_flexlm_dir[0]=buf1[0];
	}
#endif


#ifdef USE_WINSOCK
	WSAStartup((WORD)WINSOCK_VERSION, &wsadata );
#endif

#ifdef PC
	flexArgv = l_getUTF8Cmdline(NULL, &flexArgc);
	if(flexArgv == NULL)
	{
		fprintf(stderr, "Initialization error, exiting.\n");
		fflush(stderr);
		return(1);
	}
#else /* !PC */
	flexArgv = argv;
	flexArgc = argc;
#endif 

	ls_daemon(flexArgc, flexArgv);

#ifdef USE_WINSOCK
	WSACleanup();
#endif
#ifdef PC
	if(flexArgv)
	{
		l_freeUTF8Cmdline(flexArgc, flexArgv);
		flexArgv = NULL;
	}
#endif 
	return(0);
}
