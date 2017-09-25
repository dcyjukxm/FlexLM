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
 *	Module: $Id: lock_tests.c,v 1.18 2003/01/13 22:55:14 kmaclean Exp $
 *
 *	Function:	lock_tests()
 *
 *	Description: 	Tests daemon file locking
 *
 *	Parameters:	None for standard test
 *			-r for "checkroot" test
 *
 *	Return:		0 Status -> failure
 *			Non-zero status -> success
 *
 *	M. Christiano
 *	10/2/91
 *
 *	Last changed:  12/31/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_code.h"
#include <errno.h>
#include <stdio.h>

#ifdef PC
#include <windows.h>
#endif

#ifdef OS2
#define INCL_DOSPROCESS
#include <os2.h>
#include <process.h>
#endif

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2,
	VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);


LM_HANDLE *lm_job;
#if 0
int ls_show_vendor_def = 0;
f_nousers() { }
ls_mask_clear() { }
int ls_i_am_lmgrd = 0;
ls_feat_dump() { }
ls_statfile_rm() {}
ls_get_info() {}
char *ls_flist;
int ls_dump_send_data;
int groups, hostgroups;
#endif
ls_mk_flexlm_dir() {}
char *ls_user_lockfile = "";
#ifdef PC
char *(*ls_user_lock)() = NULL;	/* callback to allow multiple semaphore names
					 * to be locked out at one time. 
					 */
#endif

int _ls_lock_desc;
FILE *ofp;

main(argc, argv)
int argc;
char *argv[];
{
  int pid;
  int i, arg;
#ifdef WINNT
  BOOL opOK;
  STARTUPINFO startupInfo;
  PROCESS_INFORMATION processInfo;
#endif /* WINNT */

	ofp = stdout;
	lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &lm_job);
	for (arg = 1; arg < argc; arg++)
	{

		if (!strcmp(argv[arg], "-r"))
		{
			ls_checkroot();		/* Make sure we're on the "real" root */
			exit(0);
		}
		else if (!strcmp(argv[arg], "-o"))
		{
			arg++;
			if (!(ofp = fopen(argv[arg], "w")))
			{
				fprintf(stderr, "Can't open %s\n", argv[arg]);
				exit(1);
			}
		}
		else if (!strcmp(argv[arg], "-s"))
		{
			fprintf(ofp, "Process %d setting lock\n", getpid());
			ls_setlock();
			for (i = 1; i < 10; i++)
			{
				fprintf(ofp, 
					"Process %d checking lock\n", getpid());
				ls_checklock();
#ifndef PC
				sleep(2);
#else
				Sleep( 2000 );
#endif /* WINNT */
			}
			ls_unlock();
			exit(i);
		}
	}

#ifndef PC
	pid = fork();
	if (pid == 0)
	{
		execl(argv[0], argv[0], "-o", "lock.out", "-s", 0);
		(void) fprintf(ofp, "execl of %s failed, errno: %d\n", argv[0],errno);
	}

	sleep(5);

#endif /* PC */

#ifdef WINNT
        GetStartupInfo( &startupInfo );
	opOK = CreateProcess(
		0,	// lpApplicationName
		"lock_tests.exe -o lock.out -s",	// lpCommandLine
		NULL,		// lpProcessAttributes
		NULL,		// lpThreadAttributes
		FALSE,		// bInheritHandles,
		DETACHED_PROCESS,	// dwCreationFlags,
		NULL,		// lpEnvironment,
		NULL,		// lpCurrentDirectory,
		&startupInfo,	// lplStartupInfo,
		&processInfo );	// lpProcessInformation

	if( !opOK )
	{
		fprintf(ofp, "CreateProcess of %s failed, error: %d\n",
			argv[0],
			GetLastError());
	}
	else
	{
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}
	
	Sleep( 5000 );

#endif /* WINNT */

#ifdef OS2
	pid = spawnl(P_NOWAIT, argv[0], argv[0], "-o", "lock.out", "-s", 0);

	if( 0 == pid )
		fprintf(ofp, "spawn of %s failed, errno=%d\n", argv[0], errno);

	Sleep( 5000 );

#endif /* OS2 */

	if (i = ls_setlock()) 
	{
		fprintf(ofp, "%s\n", i ? "lockset" : "notlocked");
		i =ls_setlock(); /* try it again */
	}
	if (!i) fprintf(ofp, "unlocked\n");
}
void *
ls_malloc(c)
{
	return (void *)malloc(c);
}
ls_log_prefix(){}
ls_log_asc_printf(){}
