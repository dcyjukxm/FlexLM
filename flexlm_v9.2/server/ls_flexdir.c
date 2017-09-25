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
 *	Module: $Id: ls_flexdir.c,v 1.10 2003/05/15 23:28:55 sluu Exp $
 *
 *	Description: Open lockdir
 *
 *	D. Birns
 *	3/7/94
 *
 *	Last changed:  9/4/97
 *
 */

#include "lmachdep.h"
#ifdef WINNT
#include <io.h>
char * lm_flexlm_dir="c:\\flexlm\\";
#endif
#include <errno.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "flex_file.h"
#if defined (UNIX) && defined(ANSI)
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#if defined(UNIX) && !defined(S_ISLNK)
#define S_ISLNK(mode) ((mode&0xF000) == 0xa000)
#endif /* !def S_ISLNK */

#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif

#ifndef W_OK
/* 
 * I define these here, because the definitions are universal, but
 * the header files aren't.
 */
#define W_OK 02
#define X_OK 01
#define R_OK 04
#endif /* W_OK */

void
ls_mk_flexlm_dir()
{
#ifdef PC
char defaultflexdir[_MAX_PATH];
char originalpath[_MAX_PATH];
#endif 
char *commonfilebuffer;
int ireturn;

#if defined(WINNT) || defined(OS2)
#include <direct.h>				
#ifdef WINNT
#define mkdir(x,y) _mkdir(x)
#endif /* WINNT */
#endif /* WINNT || OS2 */
  struct stat st;
  int rc;
#if defined(TRUE_BSD) || defined(PC)
  int mask;
#else
  mode_t mask ;
#endif /* TRUE_BSD */

	rc = l_flexAccess(lm_job, LM_FLEXLM_DIR, W_OK | R_OK);
#ifndef PC
	if (!rc) /* perms are ok, but is it a directory */
	{
		if (!lstat(LM_FLEXLM_DIR, &st))
		{
/*
 *			This fixes the Sun security hole
 *			Note that S_ISLNK may not be terribly portable
 */
			if (!S_ISDIR(st.st_mode) || S_ISLNK(st.st_mode))
			{
				if (l_flexUnlink(lm_job, LM_FLEXLM_DIR))
				{
					LOG((lmtext("Can't remove %s, errno: %d(%s)\n"),
					LM_FLEXLM_DIR, errno, SYS_ERRLIST(errno)));
					return;
				}
			}
		}
		else 
		{
			LOG((lmtext("Can't stat %s, errno: %d(%s)\n"),
				LM_FLEXLM_DIR, errno, SYS_ERRLIST(errno)));
			return;
		}
	}
#endif /* PC */
	if (rc )
	{
#ifdef WINNT
			mask = umask(0);
/*			if ((mkdir(lm_flexlm_dir, 0777) == -1) && (errno != EEXIST) &&
					(errno != EACCES)) 
				LOG((lmtext("Can't make directory %s, errno: %d(%s)\n"),
					lm_flexlm_dir, errno, SYS_ERRLIST(errno)));
*/			umask(mask);
#else
			mask = umask(0);
			if ((mkdir(LM_FLEXLM_DIR, 0777) == -1) && (errno != EEXIST) &&
					(errno != EACCES)) 
				LOG((lmtext("Can't make directory %s, errno: %d(%s)\n"),
					LM_FLEXLM_DIR, errno, SYS_ERRLIST(errno)));
			umask(mask);
#endif

	}
}
