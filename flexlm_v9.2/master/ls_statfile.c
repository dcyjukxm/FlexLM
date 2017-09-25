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
 *	Module: $Id: ls_statfile.c,v 1.31 2003/05/15 23:28:55 sluu Exp $
 *
 *	Function: ls_statfile();
 *
 *	Description: create and update ls_statfile
 *
 *	Parameters:	None.
 *
 *	Return:		None
 *
 *	D. Birns
 *	3/2/94
 *
 *	Last changed:  9/30/98
 *
 */


#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>
#include "lsmaster.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "flex_file.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#ifndef PC
#include <sys/time.h>
#include <errno.h>
#endif
#include <time.h>

/* directory stuff taken from lm_baddate.c */
#ifdef NO_DIR_DOT_H
#include <dirent.h>
#else
#ifdef PC
#include <io.h>
#include <process.h>
#include <direct.h>
static char lm_flexlm_dir2[LM_MAXPATHLEN];
#else
#include <sys/dir.h>
#define dirent direct
#endif /* WINNT */
#endif
#include <sys/stat.h>

#ifdef WINNT
#include <errno.h>
#include <windows.h>
#endif

#define UPDATE_INTERVAL (60 * 30) /* Every 30 minutes */
#ifdef X_OK
#undef X_OK
#endif /* X_OK */
#define X_OK 01

static char statfile[MAX_LONGNAME_SIZE];	/* LONGNAMES */
#ifdef WINNT
static char statfilepath[MAX_LONGNAME_SIZE];/* LONGNAMES */
#endif /* WINNT */
static char *argv0;
static char * ls_argv0_path lm_args((char *));
#if !defined (PC) && !defined (GLIBC) && !defined (VMLINUX64) && !defined(RHLINUX64) && !defined( MAC10)
extern char *sys_errlist[];
#endif /* PC */
static 	void clean_up_old	lm_args((lm_noargs));

/*
 *	Some systems (notably sun4os4) implement getcwd with a
 *	pipe to the pwd command.  Therefore, we have to ensure that
 *	cwd is only called once, and that early on, before SIGCHLD is
 *	trapped.  Therefore cwd is a static which gets set only once.
 */
static char cwd[LM_MAXPATHLEN] = "";

#ifdef WINNT

/*
 *	lDetermineSystemVersion -- 	get version information from Windows
 *
 *	Input:
 *		none.
 *
 *	Return:
 *		integer value indicating the version of the Windows OS.
 */

int l_DetermineSystemVersion()
{
int iOSVer;
DWORD dwVersion, dwWindowsMajorVersion, dwWindowsMinorVersion;

	dwVersion = GetVersion();

	/* Get the Windows version. */
	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

	/* Get the build number. */
	if (dwVersion < 0x80000000)				/* Windows NT/2000/XP */
	    iOSVer = OS_NT_WIN2K_XP;
	else if (dwWindowsMajorVersion < 4)		/* Win32s */
	    iOSVer = OS_WIN32S;
	else									/* Windows 95/98/Me */
	    iOSVer = OS_95_98_ME;

	return iOSVer;
}

/*
 *	ls_make_lmgrd_file -- based on Windows OS find or create proper statfile folder
 *	location.
 *
 *	Input:
 *		none
 *
 *	Return:
 *		none
 */

void ls_make_lmgrd_file()
{
char szDefaultFlexDir[255], szOriginalPath[255], szPath[128];
int iReturn, iMask;
L_SHGETFOLDERPATHA GSISHGetFolderPath;
HANDLE hSHFolder;
DWORD dwErr;

	iMask = umask(0);
	_getcwd(szOriginalPath, 255);	/* save original path */

	iReturn = l_DetermineSystemVersion();
	switch(iReturn)
	{
		case OS_NT_WIN2K_XP:
			hSHFolder = LoadLibrary("shfolder.dll");
			if (hSHFolder != NULL)
			{
				/* use Microsoft recommended method of finding
				"<driveletter>:\Documents and Settings\\all users\\application data" folder */
				GSISHGetFolderPath = (L_SHGETFOLDERPATHA)GetProcAddress(hSHFolder, "SHGetFolderPathA");
				if ((dwErr = (*GSISHGetFolderPath)(NULL, CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, szPath)) == S_OK)
				{
					/* try to create path */
					/* LOG((lmtext("DEBUG: in ls_make_lmgrd_file, call DLL OK\n"))); */
					sprintf(szDefaultFlexDir,"%s\\Macrovision\\FLEXlm", szPath);
					if ((_access(szDefaultFlexDir, 00) == -1))
					{	/* create level 1 */
						sprintf(szDefaultFlexDir, "%s", szPath );
						_chdir(szDefaultFlexDir);
						sprintf(szDefaultFlexDir, "%s\\Macrovision", szPath );
						if ((_mkdir(szDefaultFlexDir) == -1))
						{
							if (errno == EEXIST) /* 17 */
							{
								_chdir(szDefaultFlexDir);
								sprintf(szDefaultFlexDir, "%s\\Macrovision\\FLEXlm", szPath );
								if ((_mkdir(szDefaultFlexDir) == -1))
								{
									LOG((lmtext("Can't make directory %s, errno: %d(%s)\n"), szDefaultFlexDir, errno, SYS_ERRLIST(errno)));
									/* use old default */
									sprintf(szDefaultFlexDir, "c:\\flexlm");
									_mkdir(szDefaultFlexDir);
								}
							}
							else
							{
								LOG((lmtext("Can't make directory %s, errno: %d(%s)\n"), szDefaultFlexDir, errno, SYS_ERRLIST(errno)));
								/* use old default */
								sprintf(szDefaultFlexDir, "c:\\flexlm");
								_mkdir(szDefaultFlexDir);

							}
						}
						else
						{	/* create level 2 */
							_chdir(szDefaultFlexDir);
							sprintf(szDefaultFlexDir, "%s\\Macrovision\\FLEXlm", szPath );
							if ((_mkdir(szDefaultFlexDir) == -1))
							{
								LOG((lmtext("Can't make directory %s, errno: %d(%s)\n"), szDefaultFlexDir, errno, SYS_ERRLIST(errno)));
								/* use old default */
								sprintf(szDefaultFlexDir, "c:\\flexlm");
								_mkdir(szDefaultFlexDir);
							}
						}
					}
				}
				else
				{	/* write to c:\\flexlm */
					sprintf(szDefaultFlexDir, "c:\\flexlm");
					_mkdir(szDefaultFlexDir);
				}

				FreeLibrary(hSHFolder);
			}
			else
			{	/* write to c:\\flexlm */
				sprintf(szDefaultFlexDir, "c:\\flexlm");
				_mkdir(szDefaultFlexDir);
			}
			break;
		case OS_95_98_ME:
			/* write to c:\flexlm */
			sprintf(szDefaultFlexDir, "c:\\flexlm");
			_mkdir(szDefaultFlexDir);
			break;
		default:
			sprintf(szDefaultFlexDir, "c:\\flexlm");
			_mkdir(szDefaultFlexDir);
			break;
	}

	sprintf(statfilepath,"%s", szDefaultFlexDir); /* save statfile path*/

	_chdir(szOriginalPath);	/* restore original path */
	umask(iMask);
}
#endif

/*
 *	ls_statfile -- Statfile function for saving start-state information
 *	when a server is launched.
 *
 *	Input:
 *		none
 *
 *	Return:
 *		none
 */

void ls_statfile()
{
  static time_t lasttime = 0;
  FILE *fp;
  char license_path[LM_MAXPATHLEN];
#ifdef TRUE_BSD
  char *getwd();
#else
  char *getcwd();
#endif
  time_t t;
  static time_t initial_time;
  int i;
  LM_SERVER *master, *s;
  char *cp;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
  char szTimeBuf[1024] = {'\0'};
#endif

	t = time((time_t *)0);
	if (!lasttime)
	{
		initial_time = t;
		clean_up_old();
	}
	if (!*cwd)
#ifdef TRUE_BSD
		(void)getwd(cwd);
#else
		(void)getcwd(cwd, LM_MAXPATHLEN-1);
#endif

/*
 *	Make sure it's time to update the file
 */
	if (t - lasttime <= UPDATE_INTERVAL)
		return;

	lasttime = t;

#ifdef WINNT
	ls_make_lmgrd_file();
#endif

	ls_mk_flexlm_dir();
/*
 *	bug P1630 -- removed chmod, since this is done at create time
 *	the chmod here is a POTENTIAL security hole...  If we DIDN'T
 *	create it somehow, we may be opening permissions up to the
 *	wrong object!
 */
	/*chmod(LM_FLEXLM_DIR, S_IREAD|S_IWRITE); bug P1630 */
#ifdef WINNT
	sprintf(lm_flexlm_dir2,"%s\\", statfilepath);
	sprintf(statfile, "%slmgrd.%X", lm_flexlm_dir2, getpid());
#else
#ifdef OS2
	sprintf(statfile, "%s\\lmgrd.%X", LM_FLEXLM_DIR, getpid());
#else /* Unix and everyone else (?) */
	sprintf(statfile, "%s/lmgrd.%d", LM_FLEXLM_DIR, getpid());
#endif /* WINNT */
#endif /* PC32 */

#ifdef WINNT  /* fix for P7302 */
	SetFileAttributes(statfile, FILE_ATTRIBUTE_NORMAL);
#endif

	if (!(fp = l_flexFopen(lm_job, statfile, "w")))
	{
		for (i='a'; i<'m'; i++)  /* fallback in case original statfile can't be accessed */
		{
#ifdef WINNT
			sprintf(statfile, "%slmgrd%c.%X",	/* OVERRUN */
				lm_flexlm_dir2, i, getpid());
#else
#ifdef OS2
			sprintf(statfile, "%slmgrd%c.%d",	/* OVERRUN */
				LM_FLEXLM_DIR, i, getpid()%1000);
#else /* Unix and everyone else (?) */

			sprintf(statfile, "%s/lmgrd%c.%d",	/* OVERRUN */
					LM_FLEXLM_DIR, i, getpid());
#endif /* OS2 */
#endif /* WINNT */

#ifdef WINNT   /* fix for P7302 */
			SetFileAttributes(statfile, FILE_ATTRIBUTE_NORMAL);
#endif

			if (fp = l_flexFopen(lm_job, statfile, "w"))
				break;
		}
		if (!fp) /* outahere */
		{
#ifndef PC
			LOG((lmtext("Can't open %s, errno: %d\n"),
							statfile, errno));
#else
                        LOG((lmtext("Can't open %s, errno: %d\n"),
				statfile, GetLastError()));


#endif /* !PC */
			return;
		}
	}
	master = lc_master_list(lm_job);
	for (s = master;s; s = s->next)
	{
		if (ls_on_host(s->name))
			break;
	}
#ifdef PC
	fprintf(fp, "PID=%X\n", getpid());
#else
	fprintf(fp, "PID=%d\n", getpid());
#endif /* PC */

#ifdef THREAD_SAFE_TIME
	localtime_r(&initial_time, &tst);
#ifdef NO_BUFFER_SIZE
	asctime_r(&tst, szTimeBuf);
#else /* !NO_BUFFER_SIZE */
	asctime_r(&tst, szTimeBuf, sizeof(szTimeBuf));
#endif
	fprintf(fp, "STARTED=%s", szTimeBuf);
	localtime_r(&t, &tst);
#ifdef NO_BUFFER_SIZE
	asctime_r(&tst, szTimeBuf);
#else /* !NO_BUFFER_SIZE */
	asctime_r(&tst, szTimeBuf, sizeof(szTimeBuf));
#endif
	fprintf(fp, "STAMPED=%s", szTimeBuf);
#else /* !THREAD_SAFE_TIME */
	fprintf(fp, "STARTED=%s", asctime(localtime(&initial_time)));
	fprintf(fp, "STAMPED=%s", asctime(localtime(&t)));
#endif
	fprintf(fp, "LMGRD=%s\n", ls_argv0_path(argv0));
	cp = license_path;
	for (i = 0 ; lm_job->lic_files[i]; i++)
	{

		if ((cp + strlen(lm_job->lic_files[i])) >
					(license_path + LM_MAXPATHLEN))
			break;
#ifdef WINNT

		sprintf(cp, "%s", lm_job->lic_files[i]);

#else
		if (*lm_job->lic_files[i] != '/')/* relative path, prefix cwd */
			sprintf(cp, "%s/%s", cwd, lm_job->lic_files[i]);
		else
			strcpy(license_path, lm_job->lic_files[i]);
#endif
		cp += strlen(cp);
		*cp++ = ' ';
	}
	*cp = 0;
	(void) fprintf(fp, "LICENSE_FILE=%s\n", license_path);
	if (s) (void) fprintf(fp, "TCP_PORT=%d\n", s->port);
	else (void) fprintf(fp, "TCP_PORT=-1\n");
	(void) fclose(fp);

#ifdef WINNT
	SetFileAttributes(statfile, FILE_ATTRIBUTE_HIDDEN);
#endif
}

/*
 *	set argv0 here -- gets called from main().
 *
 *	Input:
 *		str - char string command-line paramater list.
 *
 *	Return:
 *		none
 */

void
ls_statfile_argv0(char *str)
{
	argv0 = str;
}

/*
 *	ls_argv0_path -- given argv[0], return the full path to the command.
 *
 *	Input:
 *		cmd - char string command-line paramater list.
 *
 *	Return:
 *		char string pointer
 */

static char * ls_argv0_path(char *cmd)
{
	static char ret[LM_MAXPATHLEN] = "";
	char path[LM_MAXPATHLEN*4];
	char *cp;
	char *cp2;

	if (*cmd == '/') 	/* it's already fully qualified */
	{
		(void)strcpy(ret, cmd);
		return ret;
	}
	if (strchr(cmd, '/')) 	/* it's a relative path */
	{
		(void)sprintf(ret, "%s/%s", cwd, cmd);
		return ret;
	}
	/*
	 *	We have to find cmd in our path
	 */
	if(l_getEnvUTF8(lm_job, "PATH", path, sizeof(path)))
		return ("Can't get PATH environment");
	for (cp = path; *cp; )
	{
		for (cp2 = cp; *cp2 && *cp2 != ':'; cp2++)
			;
		if (*cp2 == ':')
			*cp2 = '\0';
		if (strcmp(cp, "."))
			(void)sprintf(ret, "%s/%s", cp, cmd);
		else
			(void)sprintf(ret, "%s", cmd);

		if (l_flexAccess(lm_job, ret, X_OK) == 0) /* found it */
		{
			if (*ret != '/') /* it's still a relative path */
			{
				(void)strcpy(path, ret);
				(void)sprintf(ret, "%s/%s", cwd, path);
			}
			return ret;
		}
		cp += strlen(cp) + 1;
	}
#ifdef WINNT
        if (!GetCurrentDirectory(LM_MAXPATHLEN,ret) )
                return ("Can't find lmgrd in our path");
        else return(ret);
#else

	return ("Can't find lmgrd in our path");
#endif
}

/*
 *	ls_statfile_rm() -- remove the file upon exit.
 *
 *	Input:
 *		none.
 *
 *	Return:
 *		none.
 */

void
ls_statfile_rm()
{
	if (l_flexUnlink(lm_job, statfile) == -1)
#ifndef PC
		LOG((lmtext("Can't remove statfile %s: errno %s\n"),
			statfile, SYS_ERRLIST(errno)));
#else
                LOG((lmtext("Can't remove statfile %s: errno %s\n"),
                        statfile, sys_errlist[GetLastError()]));

#endif /* !PC */
}

/*
 *	clean_up_old
 *	remove /usr/tmp/.FLEXlm/lmgrd.* when it's older than 24 hours
 *
 *	Input:
 *		none.
 *
 *	Return:
 *		none.
 */

#ifdef WINNT
static
void clean_up_old()
{
#define HOURS_24 201 /* 24 hours */
  DWORD buflen=200;
  char current_directory[201];
  HANDLE h=0;
  int next=0;
  WIN32_FIND_DATA find_data;
  SYSTEMTIME systime ;
  FILETIME filetime;

	GetLocalTime(&systime);
	SystemTimeToFileTime(&systime,&filetime);

	if (GetCurrentDirectory(buflen,current_directory) &&
					SetCurrentDirectory(lm_flexlm_dir2) )
	{
                if((h=FindFirstFile("lmgrd.*",&find_data))!=
				INVALID_HANDLE_VALUE)  next = 1;


		while (next)
		{
			if ((filetime.dwHighDateTime - HOURS_24) >
				find_data.ftLastWriteTime.dwHighDateTime )
				DeleteFile(	find_data.cFileName);

			if (!FindNextFile(h,&find_data)) next=0;
		}
                if(h ) FindClose(h);
	}
	SetCurrentDirectory(current_directory);
}
#else /*non PC */
static
void clean_up_old()
{
#define HOURS_24 86400 /* 24 hours */
  DIR *dirp;
  struct dirent *dp;
  char buff[LM_MAXPATHLEN];
  struct stat stbuff;
  time_t systime = time((time_t *)0);
	dirp = opendir((char *)LM_FLEXLM_DIR);
	if( dirp == NULL )
		return; /* can't even open it! */

	while ( (dp = readdir( dirp )) != NULL ) 		/* overrun checked */
	{
		if ( strncmp( dp->d_name, "lmgrd.", 6 ) == 0 )
		{
			(void)sprintf(buff,"%s/%s",LM_FLEXLM_DIR,dp->d_name);
			if (!stat(buff,&stbuff))
			{
				if(( systime - stbuff.st_mtime) >  HOURS_24)
				{
					(void)l_flexUnlink(lm_job, buff);
				}
			}
		}
	}
	(void)closedir(dirp);
}
#endif /* WINNT */
