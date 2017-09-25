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
 *	Module: $Id: l_baddate.c,v 1.9 2003/04/18 23:48:01 sluu Exp $
 *
 *	Function: l_baddate()
 *		  l_chkdir()
 *
 *	Description: l_baddate detects if the system's clock has been set back 
 *		     l_chkdir() checks a specified directory
 *
 *	Parameters:  l_baddate() - None.
 *		     l_chkdir():  (char *) dir - Directory to be checked
 *
 *	Return:		(int) - 0 - OK, date is OK
 *				1 - Found at least one file whose
 *				creation date is in the future more than
 *                              24 hours.
 *
 *	J. Zelaya
 *	9/23/91
 *
 *	Last changed:  11/20/98
 *
 */

#ifdef OS2
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#endif

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <sys/types.h>
#include <time.h>
#ifdef NO_DIR_DOT_H
#include <dirent.h>
#else

#ifdef PC
#include <dos.h>
#include <io.h>
#else /* PC */
#include <sys/dir.h>
#define dirent direct 
#endif /* PC */
#endif /* NO_DIR_DOT_H */
#include <sys/stat.h>

#define L_CHECK_DIR 0
#define L_CHECK_FILE 1
static int l_chkdir lm_args(( LM_HANDLE *, char *, int));


/* Maximum tolerance level for file and system date time discrepancy */

#define LM_MAXCLKDIFF 86400 /* 24 hours */



#ifndef NLM
static
int
l_chkdir(job, name, flag)
LM_HANDLE *job;		/* Current license job */
char *name;
int flag;
{
  int rc = 0; /* Assume success */
#ifdef PC
  struct _stat stbuff;
#else
  struct stat stbuff;
#endif
  char buff[LM_MAXPATHLEN];
  int day, month, year;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  long systime;
#ifdef PC
  int no_more;
#ifdef WINNT
  struct _finddata_t fileinfo;
  long   find_handle;
#else
  struct _find_t fileinfo;
#endif /* WINNT */ 
#else
  DIR *dirp;
  struct dirent *dp;
#endif /* PC */

/* 
 *	get the system time in seconds
 */
#ifdef THREAD_SAFE_TIME
	(void) l_get_date(&day, &month, &year, &systime, &tst);
#else /* !THREAD_SAFE_TIME */
	(void) l_get_date(&day, &month, &year, &systime);
#endif

  
#ifdef PC
	if( strcmp(name,"\\") )	
		sprintf( buff, "%s\\*.*", name );
	else
		sprintf( buff, "%s*.*", name );

#ifdef WINNT
	for ( find_handle=_findfirst(buff, &fileinfo), no_more=FALSE;
#ifdef _ia64_
		!no_more && (!(find_handle < 0));	/* fix for P7054 */
#else
		!no_more && (find_handle!=-1);
#endif
		 no_more=_findnext(find_handle, &fileinfo) )  
#else
	for ( no_more=_dos_findfirst(buff, _A_NORMAL|_A_HIDDEN, &fileinfo);
		!no_more; no_more=_dos_findnext(&fileinfo) )
#endif /* WINNT */
	{
		if( strcmp(name,"\\") )
			(void)sprintf(buff,"%s\\%s",name,fileinfo.name);
		else
			(void)sprintf(buff,"%s%s",name,fileinfo.name);

		if (_stat(buff,&stbuff)) continue;

		if( (long)(stbuff.st_mtime - LM_MAXCLKDIFF) > systime ) 
							/* Nailed him ! */
		{
#ifndef RELEASE_VERSION
			if (l_real_getenv("BADDATE_DEBUG"))	/* overrun checked */
				(void) printf("bad date file is %s\n", buff);
#endif
				rc = 1;
			break;	
		}
	}
#ifdef WINNT
	_findclose(find_handle);
#endif
#else /* PC */
	  
	if (flag == L_CHECK_FILE)
	{
		if (stat(name,&stbuff)) return 0; /* can't even read it */
		if (stbuff.st_mtime - systime > LM_MAXCLKDIFF) return 1;
		return 0;
	}

	if (!(dirp = opendir(name)))
		rc = 0;
	else
	{
		while ( (dp = readdir( dirp )) != NULL )	/* overrun checked */
		{
			if ( strcmp( dp->d_name, "." ) == 0 )
				continue;
			if ( strcmp( dp->d_name, ".." ) == 0 )
				continue;

			if( strcmp(name,"/") == 0 )
				(void)sprintf(buff,"%s%s",name,dp->d_name);
			else
				(void)sprintf(buff,"%s/%s",name,dp->d_name);

			if (stat(buff,&stbuff)) continue;

			if( stbuff.st_mtime - systime > LM_MAXCLKDIFF ) 
							/* Nailed him ! */
			{
#ifndef RELEASE_VERSION
				if ((int) l_real_getenv("BADDATE_DEBUG"))
				{
					(void) printf("bad date file is %s\n", buff);
				}
#endif
				closedir(dirp);
				rc = 1;
				break;
			}
		}
		if( rc == 0 )
			closedir(dirp);
	}
#endif /* _WINDOWS */
	
	if (rc)
	{
		LM_SET_ERRNO(job, LM_BADSYSDATE, 309, 0);
		rc = LM_BADSYSDATE;
	}
	return(rc);
}
#endif /* NLM */

API_ENTRY
l_baddate(LM_HANDLE * job)
{
  char buffer[LM_MAXPATHLEN];
  int rc =0; /* Assume success */

	if (!(job->options->flags & LM_OPTFLAG_CHECK_BADDATE))
		return 0;

#ifndef NLM	
	if (!(job->flags & LM_FLAG_INTERNAL_USE1))
	{
	  char buf[200];

		job->flags |= LM_FLAG_INTERNAL_USE1;
		sprintf(buf, "FLEXLM_%c%c%sADDATE", 'D', 'E', "BUG_B");
		if ((int)getenv(buf))	/* overrun checked */
			job->flags |= LM_FLAG_INTERNAL_USE2;
	}
	if (job->flags & LM_FLAG_INTERNAL_USE2) 
	{
		LM_SET_ERRNO(job, LM_BADSYSDATE, 312, 0);
		return LM_BADSYSDATE;
	}

/* 
 *	We check / . We don't want to use strings "/" or "/etc" since
 *	the user may use strings to attack these scheme 
 */
#if defined(PC) && !defined(NLM) 
	buffer[0] = '\\';
#else
	buffer[0] = '/';
#endif /* PC  */

	buffer[1] = '\0';

#ifndef UNIX /* P2181 */
	if (rc = l_chkdir(job, buffer, L_CHECK_DIR))  return rc;
#endif

	
#if defined ( PC) && !defined(NLM)
 
	GetWindowsDirectory(buffer, LM_MAXPATHLEN);

#else
#ifdef OS2
	{
	ULONG bootDrive;
	APIRET rc;

	rc = DosQuerySysInfo(
		QSV_BOOT_DRIVE,
		QSV_BOOT_DRIVE,
		(PVOID) &bootDrive,
		sizeof(bootDrive) );

/*
 *	On error, just assume that the "C" drive is the boot
 *	drive.
 */

	if( NO_ERROR != rc )
		bootDrive = 3;

/*
 *	Decrement bootDrive so that 0 means drive A,
 *	then make a string to point to the OS2 directory.
 */

	--bootDrive;
	buffer[3] = 'O';
	buffer[1] = ':';
	buffer[2] = '\\';
	buffer[0] = 'A' + bootDrive;
	buffer[4] = 'S';
	buffer[6] = '\0';
	buffer[5] = '2';
	}
#else

/* 
 *	We check /etc . We don't want to use strings "/" or "/etc" since
 *	the user may use strings to attack these scheme 
 */
	buffer[0] = '/';
	buffer[2] = 't';
	buffer[3] = 'c';
	buffer[1] = 'e';
	buffer[4] = '\0';
#endif /* OS2 */
#endif /* _WINDOWS */

	if (rc = l_chkdir(job, buffer, L_CHECK_DIR))  return rc;
	buffer[0] = '/';
	buffer[2] = 'a';
	buffer[3] = 'r';
	buffer[1] = 'v';
	buffer[4] = '\0';
	if (rc = l_chkdir(job, buffer, L_CHECK_DIR))  return rc;
	buffer[0] = '/';
	buffer[4] = '\0';
	buffer[2] = 'm';
	buffer[1] = 't';
	buffer[3] = 'p';
	if (rc = l_chkdir(job, buffer, L_CHECK_DIR))  return rc;
#endif /* NLM */

	return(rc);
}


