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
 *	Module: $Id: ls_checkroot.c,v 1.5 2003/04/11 00:08:55 brolland Exp $
 *
 *	Function:	ls_checkroot()
 *
 *	Description: 	Checks that file system root hasn't been changed
 *
 *	Parameters:	None.
 *
 *	Return:		None. (Exits with a LOG message if root is changed)
 *
 *	M. Christiano
 *	11/1/91
 *
 *	Last changed:  08/07/97
 *
 */


#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_log.h"
#ifndef NLM
#include <sys/types.h>
#ifdef NO_DIR_DOT_H
#include <dirent.h>
#else
#ifdef DGUX
#include <sys/dir.h>
#define dirent direct
#else
#ifndef RS6000
#define dirent direct
#endif
#endif /* NLM */ 
#ifdef PC
#include <direct.h>
#else
#include <sys/dir.h>
#endif /* PC */
#endif
#endif

#ifndef PC  
extern DIR *opendir();
extern struct dirent *readdir();
#endif

int
ls_checkroot()
{
#if !defined(apollo) && !defined(PC) /*- This code doesn't work on apollo */

  DIR *dirp;
  struct dirent *dp;

  int dot = -1, dotdot = -1;

	dirp = opendir("/");
	if (dirp != NULL)
	{
		while ( (dp = readdir( dirp )) != NULL )	/* overrun checked */
		{
			if ( strcmp( dp->d_name, "." ) == 0 )
				dot = dp->d_ino;
			if ( strcmp( dp->d_name, ".." ) == 0 )
				dotdot = dp->d_ino;
		}
	}
	if (dot != dotdot) 
	{
		return 1;
	}
	(void) closedir(dirp);
#endif
	return 0;
}
