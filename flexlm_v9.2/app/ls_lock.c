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
 *	Module: $Id: ls_lock.c,v 1.43 2003/05/15 23:28:35 sluu Exp $
 *
 *	Function: ls_setlock(), ls_checklock(), ls_unlock()
 *
 *	Description: Sets up and checks the lock for the application daemon.
 *
 *	Parameters:	None.
 *
 *	Return:		None.  Exits if lock cannot be set, or checks "bad".
 *
 *	M. Christiano
 *	2/2/88
 *	Adapted from the flock() code in ls_init() and ls_processing().
 *	For now, I've removed NLM, VMS for readability.  Refer to 1.1 of this
 *	file for info on that.
 *
 */

#include "lmachdep.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "flex_file.h"
#include <errno.h>
#include "ls_lock.h"
#ifdef UNIX
#include <utime.h>
#include <unistd.h>
#else
#include <sys/utime.h>
#include <sys/locking.h>
#include <io.h>
#endif /* UNIX */

#ifdef CRAY_NV1
#include <sys/file.h>
#endif /* CRAY_NV1 */

#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif

#ifdef WINNT
#include <windows.h>
#include "gsi_Accctrl.h"	/* copied from the original accctrl.h from platform SDK */
#include "Aclapi.h"			/* (get rid of this when we go to VC++ 6 or greater)    */
#endif

long ls_currtime;
static int l_lock_fd;
static int l_lock_inode;

static int lockfile_missing = 5;	/* How many times we will allow the
										lock file to be missing */
static void ls_update_lockfile_time();
static l_setlock(char *name);
static int check_old_locks();

#ifdef PC
static BOOL fSemaphoreAlreadyExists;
static int semacount, fin, finuser;
static void *hSemap[128];				/* Semaphore handle */
static BOOL fFoundAnOrphanedLockFile;
static BOOL fFoundAnOrphanedUserLockFile;
static BOOL fW2K, fXP, fNT4, fWin9x;	/* Windows OS flags */
static int ls_disable_default_lockfile_internal = 1;
typedef char * (LM_CALLBACK_TYPE * LS_USER_LOCK) ();
#define LS_USER_LOCK_TYPE (LS_USER_LOCK)

static int l_setsemaphore();
static int l_check_semaphore_defaultlockfile();
#endif /* PC */

#ifdef PC
BOOL l_determine_windows_os()
{
DWORD dwVersion, dwWindowsMajorVersion, dwWindowsMinorVersion;
DWORD dwBuild;
int iVersion;

	dwVersion = GetVersion();

	// Get the Windows version.

	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

	// Get the build number.

	if (dwVersion < 0x80000000)              // Windows NT/2000/XP
	    dwBuild = (DWORD)(HIWORD(dwVersion));
	else if (dwWindowsMajorVersion < 4)      // Win32s
	    dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
	else                                     // Windows 95/98/Me
	    dwBuild =  0;

//	printf("MajorVersion: %ld MinorVersion: %ld\n\n", dwWindowsMajorVersion, dwWindowsMinorVersion);

	if ((dwWindowsMajorVersion == 5) && (dwWindowsMinorVersion == 2))
		fXP = TRUE;			//printf("Windows .NET\n");
	if ((dwWindowsMajorVersion == 5) && (dwWindowsMinorVersion == 1))
		fXP = TRUE; 		//printf("Windows XP\n");
	if ((dwWindowsMajorVersion == 5) && (dwWindowsMinorVersion == 0))
		fW2K = TRUE; 		//printf("Windows 2000\n");
	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 0))
		fNT4 = TRUE; 		//printf("Windows NT 4.0\n");
	if ((dwWindowsMajorVersion == 3) && (dwWindowsMinorVersion == 0))
		fNT4 = TRUE; 		//printf("Windows NT 3.51\n");
	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 1))
		fWin9x = TRUE; 		//printf("Windows 95,98,Me\n");

	return TRUE;
}

int
l_check_semaphore_defaultlockfile()
{
  extern char* (*ls_user_lock)();	/* semaphore name callback */
  extern char *ls_user_lockfile;
  char name[MAX_LONGNAME_SIZE] = {'\0'};
  int iReturn, iCount;
  char *user_lock;
  char *usertok;
  DWORD dwError, dwWaitResult;
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  BOOL fReturn;

	/*LOG((lmtext("DEBUG: in l_check_semaphore_defaultlockfile.\n")));*/
	iReturn = 0;
 	if (ls_user_lock != NULL)
	{
	 	char *(*func)() = ls_user_lock;
		user_lock = (* LS_USER_LOCK_TYPE func)();
	}
	else
		user_lock = NULL;

	if (user_lock != NULL)
	{	/* multiple semaphore checks*/
		iCount = 0;
		usertok = strtok(user_lock, " ");
		while (usertok != NULL)
		{
			dwWaitResult = WaitForSingleObject(hSemap[iCount], 0L);
			switch (dwWaitResult)
			{
			    case WAIT_FAILED:
				iReturn = 4;	/* semaphore compromised! */
				break;
			    case WAIT_OBJECT_0:
			    case WAIT_TIMEOUT:
				if((ReleaseSemaphore(hSemap[iCount], 1, NULL)) == FALSE)
					iReturn = 4;	/* semaphore compromised! */
			        break;
			}	// end switch
			usertok = strtok(NULL, " ");
			iCount++;
		}
	}
	else
	{	/* single semaphore check */
		dwWaitResult = WaitForSingleObject(hSemap[semacount], 0L);
		switch (dwWaitResult)
		{
		    case WAIT_FAILED:
			iReturn = 4;	/* semaphore compromised! */
			break;
		    case WAIT_OBJECT_0:
		    case WAIT_TIMEOUT:
			if((ReleaseSemaphore(hSemap[0], 1, NULL)) == FALSE)
				iReturn = 4;	/* semaphore compromised! */
		        break;
		}	// end switch
	}

	/* check if default lockfile exists */
	/* in case someone else came in while we weren't looking, in semaphore mode only */
	if (ls_disable_default_lockfile_internal == 1)
	{
		/* check for default lockfile */
		sprintf(name, "C:\\FLEXLM\\%s", lm_job->vendor);	/* OVERRUN */

		if (fFoundAnOrphanedLockFile == TRUE)
		{
			// is OK then still orphan else error out.
			if((_locking(fin, _LK_UNLCK, 1)) < 0)
			{
				close(fin);
				LOG((lmtext("cannot unlock lockfile (%s): %s\n"), name, SYS_ERRLIST(errno)));
				iReturn = 3;
			}

			/* close handle */
			if (fin > 0)
				_close(fin);

			/* get handle */
			if (iReturn == 0)
				fin = l_flexOpen(lm_job, name, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
			if (fin < 0)
			{
				LOG((lmtext("cannot open lock file (%s): %s\n"), name, SYS_ERRLIST(errno)));
			    iReturn = 3;
			}
			/* lock it */
			if((_locking(fin, _LK_NBLCK, 1)) < 0)
			{
				close(fin);
				LOG((lmtext("cannot lock lockfile (%s): %s\n"), name, SYS_ERRLIST(errno)));
				iReturn = 3;
			}
		}

		if (fFoundAnOrphanedLockFile == FALSE)
		{
			hFind = FindFirstFile(name, &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				FindClose(hFind);	/* found it! */
				/* trick to see if file is locked or not by a vendor daemon */
				if ((l_flexRename(lm_job, name, name)) != 0)
				{
					iReturn = 3;	/* force an exit, since an old lockfile was found */
					LOG((lmtext("cannot open lock file (%s): %s\n"), name, SYS_ERRLIST(errno)));
				}
				else
				{
					fFoundAnOrphanedLockFile = TRUE;	/* clean up later */

					/* get handle */
					fin = l_flexOpen(lm_job, name, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
					if (fin < 0)
					{
						LOG((lmtext("cannot open lock file (%s): %s\n"), name, SYS_ERRLIST(errno)));
					    iReturn = 3;
					}

					/* lock it */
					if((_locking(fin, _LK_NBLCK, 1)) < 0)
					{
						close(fin);
						LOG((lmtext("cannot lock lockfile (%s): %s\n"), name, SYS_ERRLIST(errno)));
						iReturn = 3;
					}
				}
			}
		}

		/* check for orphaned ls_user_lockfile */
		if (ls_user_lockfile)
		{
			if (fFoundAnOrphanedUserLockFile == TRUE)
			{	/* unlock */
				if((_locking(finuser, _LK_UNLCK, 1)) < 0)
				{
					close(finuser);
					LOG((lmtext("cannot unlock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
					iReturn = 3;
				}
				/* close handle */
				if (finuser > 0)
					_close(finuser);

				/* lock orphan again */
				if (iReturn == 0)
					finuser = l_flexOpen(lm_job, ls_user_lockfile, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
				if (finuser < 0)
				{
					LOG((lmtext("cannot open lock file (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
				    iReturn = 3;
				}
				/* lock it here with _locking() */
				if((_locking(finuser, _LK_NBLCK, 1)) < 0)
				{
					close(finuser);
					LOG((lmtext("cannot lock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
					iReturn = 3;
				}
			}	/* end TRUE */

			if (fFoundAnOrphanedUserLockFile == FALSE)
			{
				hFind = FindFirstFile(ls_user_lockfile, &FindFileData);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					FindClose(hFind);	/* found it! */

					/* unlock */
					if((_locking(finuser, _LK_UNLCK, 1)) < 0)
					{
						close(finuser);
						LOG((lmtext("cannot unlock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
						iReturn = 3;
					}
					/* close handle */
					if (finuser > 0)
						_close(finuser);

					/* lock orphan again */
					finuser = l_flexOpen(lm_job, ls_user_lockfile, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
					if (finuser < 0)
					{
						LOG((lmtext("cannot open lock file (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
					    iReturn = 3;
					}
					/* lock it here with _locking() */
					if((_locking(finuser, _LK_NBLCK, 1)) < 0)
					{
						close(finuser);
						LOG((lmtext("cannot lock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
						iReturn = 3;
					}
				}
				else
				{
						LOG((lmtext("cannot find lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
						iReturn = 3;
				}
			}	/* end FALSE */
		}	/* end ls_user_lockfile */
	}	/* end ls_disable_default_lockfile_internal */
	return (iReturn);
}
#endif /* PC */

void
ls_block_lockfile(char *name)
{
  int i;
/*******************************************************************
	CAUTION: this code is meant to encrypt multiple lockfile
			names, it is not safe to simply hardcode for l_locks[0].
********************************************************************/

	for (i = 0; i < LS_MAX_LOCKFILES; i++)
		if (!l_locks[i]) break;

   	if (l_locks[i]) return; /* none left */

	l_locks[i] = (char *)LS_MALLOC(strlen(name) + 1);
	strcpy(l_locks[i], name);
}

#ifdef PC
int
l_setsemaphore()
{
  char name[200];
  char prefix[7];
  char suffix[15];
  char *user_lock;
  char *usertok;
  DWORD dwError, dwVersion, dwWindowsMajorVersion, dwWindowsMinorVersion;
  int cMin = 1;
  int cMax = 1;
  extern HANDLE hSemap[128];
  BOOL fSemaphoreExists = FALSE;
  extern char* (*ls_user_lock)();

	/* determine OS version */
	fNT4 = FALSE;
	fW2K = FALSE;
	fXP = FALSE;
	fWin9x = FALSE;

	if((l_determine_windows_os()) == FALSE)
		LOG((lmtext("could not determine Windows OS version\n")));

 	if (ls_user_lock != NULL)
	{
	 	char *(*func)() = ls_user_lock;
		user_lock = (* LS_USER_LOCK_TYPE func)();
	}
	else
		user_lock = NULL;

	semacount = 0;

	if (user_lock != NULL)
	{	/* custom semaphore name and multiple semaphore mode */
		usertok = strtok(user_lock, " ");
		while (usertok != NULL)
		{
			L_INIT_LOCKNAME(l_pc_locpre, prefix);	/* obfuscate string */
			L_INIT_LOCKNAME(l_pc_locpos, suffix);	/* obfuscate string */

			if ((fW2K) || (fXP))
				sprintf(name, "Global\\%s%s%s", prefix, usertok, suffix);
			else
				sprintf(name, "%s%s%s", prefix, usertok, suffix);

			ls_block_lockfile(name);		/* obfuscate string */
			hSemap[semacount] = CreateSemaphore( NULL,    // security attributes
                                                 cMin,   // initial count
                                                 cMax,   // maximum count
                                                 l_locks[semacount]);  // semaphore name
			if (hSemap[semacount] == NULL)
			{								/* fundamental semaphore create problem */
				fSemaphoreExists = TRUE;	/* fail vendor daemon start anyway */
			}
			else
			{
				dwError = GetLastError();
				if (dwError == ERROR_ALREADY_EXISTS)
				{
					fSemaphoreExists = TRUE;
				}
			}

			usertok = strtok(NULL, " ");
			semacount++;
		}
	}
	else
	{	/* default behavior, single semaphore */
		L_INIT_LOCKNAME(l_pc_locpre, prefix);	/* obfuscate string */
		L_INIT_LOCKNAME(l_pc_locpos, suffix);	/* obfuscate string */

		if ((fW2K) || (fXP))
			sprintf(name, "Global\\%s%s%s", prefix, lm_job->vendor, suffix);
		else
			sprintf(name, "%s%s%s", prefix, lm_job->vendor, suffix);

		ls_block_lockfile(name);	/* obfuscate string */

		/*LOG((lmtext("DEBUG: in l_setsemaphore - name(%s)(%s)\n"), name, l_locks[0]));*/

		hSemap[0] = CreateSemaphore( NULL,    // security attributes
                                     cMax,   // initial count
                                     cMax,   // maximum count
                                     l_locks[0]);  // semaphore name
		if (hSemap[0] == NULL)
		{								/* fundamental semaphore create problem */
			LOG((lmtext("cannot create semaphore lock (%s): %ld\n"), name, GetLastError()));
			fSemaphoreExists = TRUE;	/* fail vendor daemon start anyway */
		}
		else
		{
			dwError = GetLastError();
			if (dwError == ERROR_ALREADY_EXISTS)
			{
				fSemaphoreExists = TRUE;
				/*LOG((lmtext("DEBUG: in l_setsemaphore - semaphore already exists(%ld)\n"), (int)fSemaphoreExists));*/
			}
		}
	}
	return((int)fSemaphoreExists);
}
#endif

ls_setlock()
{
	char name[MAX_LONGNAME_SIZE] = {'\0'};
  static int done = 0;
  extern char *ls_user_lockfile;
#ifdef PC
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  int iReturnOldLocks;
  BOOL fReturn;
  int ilocalcount;
#endif

#ifdef PC
	fSemaphoreAlreadyExists = FALSE;
	fFoundAnOrphanedLockFile = FALSE;
	fFoundAnOrphanedUserLockFile = FALSE;
	/* create semaphore regardless of xp mode or not */
	if (l_setsemaphore())
		fSemaphoreAlreadyExists = TRUE;

	if (ls_disable_default_lockfile_internal) /* ==1 */
	{
		/* check if default lockfile exists */
		sprintf(name, "C:\\FLEXLM\\%s", lm_job->vendor);	/* OVERRUN */
		hFind = FindFirstFile(name, &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
			/* trick to see if file is locked or not by a vendor daemon */
			if ((l_flexRename(lm_job, name, name)) != 0)
			{
				return(-1);	/* force an exit, since an old lockfile was found */
			}
			else
			{
				fFoundAnOrphanedLockFile = TRUE;	/* clean up later */
				/* first run through go ahead and lock it */
				fin = l_flexOpen(lm_job, name, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
				if (fin < 0)
				{
					LOG((lmtext("cannot open lock file (%s): %s\n"), name, SYS_ERRLIST(errno)));
					return(-1);
				}
				/* lock it here with _locking() */
				if((_locking(fin, _LK_NBLCK, 1)) < 0)
				{
					close(fin);
					LOG((lmtext("cannot lock lockfile (%s): %s\n"), name, SYS_ERRLIST(errno)));
					return(1);
				}
			}
		}

		/* check if custom "ls_user_lockfile" exists */
		if (ls_user_lockfile)
		{

			/* check if ls_user_lockfile exists */
			hFind = FindFirstFile(ls_user_lockfile, &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				FindClose(hFind);
				/* trick to see if file is locked or not by a vendor daemon */
				if ((l_flexRename(lm_job, ls_user_lockfile, ls_user_lockfile)) != 0)
				{
					return(-1);	/* force an exit, since an old lockfile was found */
				}
				else
				{
					fFoundAnOrphanedUserLockFile = TRUE;	/* clean up later */
					/* found orphan go ahead and lock it */
					finuser = l_flexOpen(lm_job, ls_user_lockfile, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
					if (finuser < 0)
					{
						LOG((lmtext("cannot open lock file (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
						return(-1);
					}
					/* lock it here with _locking() */
					if((_locking(finuser, _LK_NBLCK, 1)) < 0)
					{
						close(finuser);
						LOG((lmtext("cannot lock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
						return(1);
					}
				}
			}
			else
			{
				finuser = open(ls_user_lockfile, O_RDWR | O_CREAT, _S_IREAD|_S_IWRITE );	/* LONGNAMES */
				if (finuser < 0)
				{
					LOG((lmtext("cannot open lock file (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
					return(-1);
				}
				/* lock it here with _locking() */
				if((_locking(finuser, _LK_NBLCK, 1)) < 0)
				{
					close(finuser);
					LOG((lmtext("cannot lock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
					return(1);
				}
			}
		}

		/* finally, check if semaphore exists or not */
		if (fSemaphoreAlreadyExists)
		{
			/*LOG((lmtext("DEBUG: in ls_setlock xp mode - sempahore already exists\n")));*/
			return(-1);
		}
		else
		{
			/*LOG((lmtext("DEBUG: in ls_setlock xp mode - sempahore OK\n")));
			LOG((lmtext("DEBUG: in ls_setlock xp mode - ls_disable_default_lockfile (%d)\n"), ls_disable_default_lockfile));*/
			return(0);
		}
	}
	else
#endif /* PC */
	{
		/*LOG((lmtext("DEBUG: in ls_setlock non xp mode - in old lockfile routine\n")));*/
		/* create lock file */
		if (!done)
		{
			if (ls_user_lockfile)
				ls_block_lockfile(ls_user_lockfile);
			else
			{
#ifdef UNIX
				L_INIT_LOCKNAME(l_var_tmp, name);
				sprintf(name, "%s/lock%s", name, lm_job->vendor);
				ls_block_lockfile(name);
				L_INIT_LOCKNAME(l_usr_tmp, name);
				sprintf(name, "%s/lock%s", name, lm_job->vendor);
				ls_block_lockfile(name);
				L_INIT_LOCKNAME(l_usr_tmp_flexlm, name);
				sprintf(name, "%s/.lock%s", name, lm_job->vendor);
				ls_block_lockfile(name);
#else
				L_INIT_LOCKNAME(l_pc_loc, name);
				sprintf(name, "%s\\%s", name, lm_job->vendor);
				ls_block_lockfile(name);
#endif
			}
			done = 1;
		}

		/*printf("Opening %s\n", l_locks[0]);*/
		l_lock_fd = l_flexOpen(lm_job, l_locks[0], O_RDWR | O_CREAT,
#ifdef PC
							  _S_IREAD|_S_IWRITE );
#else
							  0666);
#endif /* PC */


		if (l_lock_fd < 0)
		{
#ifdef PC
			if ((strnicmp(name,"c:\\flexlm", 9) == 0) && (fSemaphoreAlreadyExists == FALSE))
				return (0);	/* semaphore is OK so allow VDaemon to run even if default lockfile can be created */
			else
#endif
			{
				LOG((lmtext("cannot open lock file (%s): %s\n"), l_locks[0], SYS_ERRLIST(errno)));
				return(-1);
			}
		}
#ifdef LOCK_LOCKF
		if (lockf(l_lock_fd, F_TLOCK, 0))
#else
#ifdef LOCK_LOCKING
		if ( _locking(l_lock_fd, _LK_NBLCK, 1) )
#else
		if (flock(l_lock_fd, LOCK_EX | LOCK_NB))
#endif /* LOCK_LOCKING */
#endif /* LOCK_LOCKF */
		{
			close(l_lock_fd);
			LOG((lmtext("cannot open lock file (%s): %s\n"),
						l_locks[0], SYS_ERRLIST(errno)));
			return(1);
		}

#ifdef PC
	if (fSemaphoreAlreadyExists)
	{
		/*LOG((lmtext("DEBUG: in ls_setlock - semaphore already exists (-1)\n")));*/
		return(-1);
	}
	return(0);
#endif
		return check_old_locks();
	}	/* old behavior */

}

static
int
check_old_locks()
{
  int rc, i;

/*	LOG((lmtext("DEBUG: in check_old_locks\n")));*/

#ifdef UNIX
  struct stat st;
#define L_EXIST(s) stat((s), &st)
#else /* PC */
#define L_EXIST(s) l_flexAccess(NULL, (s), 0)	 /* use access, because errno is set for EACCES */
#endif /* UNIX */

#ifdef UNIX
	if (!l_lock_inode && !stat(l_locks[0], &st))
		l_lock_inode = st.st_ino;
#endif
	for (i = 1; l_locks[i] && i < LS_MAX_LOCKFILES; i++)
	{
		/*printf("Checking %s\n",  l_locks[i]);*/
		rc = L_EXIST(l_locks[i]);
		if (rc && (errno == EACCES))
		{
			LOG(("Permissions problem read/removing lockfile %s",
								l_locks[i]));
			return 1;
		}
		else if (!rc)
		{
#ifdef UNIX
			if (!stat(l_locks[i], &st) && (st.st_ino != l_lock_inode))
#endif
			{
				l_flexRemove(lm_job, l_locks[i]);
				if (!L_EXIST(l_locks[i]))
				{
					LOG(("Problem removing lockfile %s",
							l_locks[i]));
					return 1; /* remove failed */
				}
			}
			/*else printf("same inode %s %s\n", l_locks[0], l_locks[i]);*/
		}
	}
	return 0;
}

int
ls_checklock()
{
  extern char *ls_user_lockfile;
  int retries = 5;
#ifdef PC
  int iReturn;
#endif

/*
 *	Make sure no other servers have snuck in while
 *	we wern't looking.  It is necessary to close/open
 *	and re-flock the file, since if someone has
 *	removed it and started a new server, we won't
 *	be able to detect that without re-opening/flocking.
 */

#ifdef PC
	/*LOG(("DEBUG: in ls_checklock\n"));*/
	if ((ls_disable_default_lockfile_internal) != 1)
	{
		iReturn = 0;
		iReturn = l_check_semaphore_defaultlockfile();

#endif
#ifdef LOCK_FLOCK
		flock(l_lock_fd, LOCK_UN);	/* Unlock it */
#endif
		if (l_lock_fd >= 0)
		{
		    if (close(l_lock_fd))
		    {
			return 1;
		    }
		}
retry_open:
		ls_mk_flexlm_dir();
		l_lock_fd = l_flexOpen(lm_job, l_locks[0], O_RDWR, 0);
		if (l_lock_fd < 0)
		{
			if (errno == ENFILE && retries-- > 0)
			{
/*
 *			File table full - sleep 1 second and re-try
 */
				lm_sleep(1);
				goto retry_open;
			}
/*
 *		Only attempt to re-create it 5 times!!!
 */
			lockfile_missing--;
			if (lockfile_missing)
			{
				l_lock_fd = l_flexOpen(lm_job, l_locks[0], O_RDWR | O_CREAT,
#ifdef PC
							  _S_IREAD|_S_IWRITE	);
#else
							  0666);
#endif /* PC */

			}
			if (l_lock_fd < 0)
			{
				return 2;
			}
		}
#ifdef LOCK_LOCKF
		if (lockf(l_lock_fd, F_TLOCK, 0))
#else
#ifdef LOCK_LOCKING
		if (_locking(l_lock_fd, _LK_NBLCK, 1))
#else
		if (flock(l_lock_fd, LOCK_EX | LOCK_NB))
#endif /* LOCK_LOCKING */
#endif /* LOCK_LOCKF */
		{
			return 3;
		}
#ifdef PC
		if (iReturn != 0)
			return (iReturn);	/* generate semaphore error */
#endif
		ls_update_lockfile_time();
		return check_old_locks();
#ifdef PC
	}
	else
		iReturn = l_check_semaphore_defaultlockfile();
		return (iReturn);	/* xp mode */
#endif
}


/*
 *	Remove our lock file
 */

void
ls_unlock(remove)
int remove;
{
#ifdef PC
  extern HANDLE hSemap[128];
  extern char *ls_user_lockfile;
  int i;
  char name[255];
#endif

	if (l_lock_fd >= 0) close(l_lock_fd);
	if (remove)
	{
		l_flexUnlink(lm_job, l_locks[0]);
#ifdef PC
		/* clean up area for everything created above */
		if (semacount > 0)
		{
			for (i = 0; i < semacount; i++)
				if (hSemap[i] != NULL)
					CloseHandle(hSemap[i]);
		}
		else
		{
			if (hSemap[0] != NULL)
				CloseHandle(hSemap[0]);
		}
		/* clean up orphaned lockfile*/
		if (fFoundAnOrphanedLockFile == TRUE)
		{
			sprintf(name, "C:\\FLEXLM\\%s", lm_job->vendor);
			/* unlock it */
			if((_locking(fin, _LK_UNLCK, 1)) < 0)
			{
				close(fin);
				LOG((lmtext("cannot lock lockfile (%s): %s\n"), name, SYS_ERRLIST(errno)));
				return;
			}
			/* close handle */
			if (fin > 0)
				_close(fin);

			if((DeleteFile(name)) == 0)
				LOG(("Problem removing lockfile %s", name));
		}
		if (ls_user_lockfile)
		{
			/* unlock */
			if((_locking(finuser, _LK_UNLCK, 1)) < 0)
			{
				close(finuser);
				LOG((lmtext("cannot unlock lockfile (%s): %s\n"), ls_user_lockfile, SYS_ERRLIST(errno)));
				return;
			}
			/* close handle*/
			if (finuser > 0)
				_close(finuser);

			if((DeleteFile(ls_user_lockfile)) == 0)
				LOG(("Problem removing lockfile %s", ls_user_lockfile));
		}
#endif
	}
}


/*
 *	ls_update_lockfile_time()
 *	Every 6 hours we update the file system creation time for the
 *	lockfile.  This is to prevent deletion by scripts that
 *	remove old files from /usr/tmp.
 */


static
void
ls_update_lockfile_time()
{
 static long locktime = 0;
 struct utimbuf u;
 extern long ls_currtime;


#ifdef RELEASE_VERSION
#define LM_UPDATE_TIME 6 * 60 * 60 /*every six hours*/
#else
#define LM_UPDATE_TIME 60 /*every minute*/
#endif
	if (!locktime)
	{
		locktime= ls_currtime;
	}
	else
	{
		if ((ls_currtime - locktime) > LM_UPDATE_TIME)
		{
			u.actime = u.modtime = ls_currtime;
			utime(l_locks[0], &u); /* ignore errors -- it's
						   * worth it to detect them */
			locktime = ls_currtime;
		}

	}
}



