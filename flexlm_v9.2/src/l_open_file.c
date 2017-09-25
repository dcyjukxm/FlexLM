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
 *	Module: $Id: l_open_file.c,v 1.7 2003/04/29 21:46:26 sluu Exp $
 *
 *	Function: l_open_file(job, which)
 *
 *	Description: Opens the license manager license file.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) which -   LFPTR_CURRENT -> open current file
 *					LFPTR_NEXT -> open next file in path
 *					LFPTR_FIRST -> open first file in path
 *
 *	Return:		(LICENSE_FILE *) license file pointer, or NULL, if file
 *					can't be opened or string is unavailable
 *
 *	M. Christiano
 *	2/13/88
 *
 *	Last changed:  12/14/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"
#include "flex_file.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

LICENSE_FILE * API_ENTRY
l_open_file(job, which)
LM_HANDLE *job;		/* Current license job */
int which;
{
  LICENSE_FILE *lf = (LICENSE_FILE *) NULL;
  char *file = NULL;
  char *s = (char *)NULL;
  struct stat st;
  int start_len = sizeof(LM_LICENSE_START) - 1;
  char *file_txt = NULL;

/*
 *	Get the location of the license file
 */

#ifdef NLM
	ThreadSwitch();
#endif
	if (job->lic_files == (char **) NULL) l_init_file(job);

	if (which == LFPTR_NEXT) job->lfptr++;
	else if (which == LFPTR_FIRST) job->lfptr = LFPTR_FILE1;
	/*else if (which == LFPTR_CURRENT)  do nothing  ;*/

	if (job->lfptr >= job->lm_numlf)
	{
/*
 *		We are beyond the end of the license file names.  Return
 *		an error.
 */
		LM_SET_ERRNO(job, LM_ENDPATH, 70, -1);
		return((LICENSE_FILE *) NULL);
	}
	lf = &job->license_file_pointers[job->lfptr];


	if (lf->type != LF_NO_PTR)
	{
		if (lf->type != LF_PORT_HOST_PLUS)
			l_lfseek(lf, (long) 0, 0);
		return(lf);
	}

	l_set_error_path(job);

	file = lc_lic_where(job);

	if (file == (char *) NULL)
	{
		LM_SET_ERRNO(job, LM_NOCONFFILE, 71, 0);
		return((LICENSE_FILE *) NULL);
	}

#ifdef PORT_AT_HOST_SUPPORT
	if (L_STREQ_N(file, LM_LICENSE_START, start_len))
	{
		
		s = lf->ptr.str.s = (char *)l_malloc(job, strlen(file) - 
				(start_len));
		strcpy(lf->ptr.str.s, &file[start_len + 1]);
		lf->ptr.str.cur = s;	/* Reset cur ptr */
		lf->type = LF_STRING_PTR;


	}
	else if (strchr(file, '@') != (char *) NULL)
	{
		if (*file == LM_OLD_PORT_HOST_SIGN) 
		{
			file++;
		}
		else if (*file == LM_PORT_HOST_PLUS_SIGN || isdigit(*file)
			|| *file == '@') 
		{
			if (*file == LM_PORT_HOST_PLUS_SIGN)
				file++;
			lf->ptr.commspec = file;
			if (job->options->flags & LM_OPTFLAG_PORT_HOST_PLUS)
			{
				lf->type = LF_PORT_HOST_PLUS;
				return(lf);
			}
		} 
/* 
 *		This "filename" (with an embedded '@') indicates that we 
 *		are to get the license file data from the server.  
 *		If we have already read it, return it now.
 */
		if (lf->type == LF_STRING_PTR && lf->ptr.str.s)
		{
			lf->ptr.str.cur = lf->ptr.str.s;  /* Reset read ptr */
		}
#ifndef NO_LMGRD
		else
		{
			s = l_get_lfile(job, file, "", &lf->endpoint);
			if (s)
			{
				lf->ptr.str.s = s;
				lf->ptr.str.cur = s;	/* Reset cur ptr */
				lf->type = LF_STRING_PTR;
			}
			else 
			{
				return 0;
			}
		}
#endif /* NO_LMGRD */
	}
#endif	/* PORT_AT_HOST_SUPPORT */
	if (file && !s)
	{
		file_txt = (char *)l_malloc(job, strlen(file) + 5);
		sprintf(file_txt, "%s.txt", file);
#ifndef PC
		if ((l_flexStat(job, file, &st) || !S_ISREG((st.st_mode )))
		 && (l_flexStat(job, file_txt, &st) || !S_ISREG((st.st_mode ))))
#else
#ifdef OS2
	this is a very wrong sintax!
#endif /* OS2 */
		if ((l_flexStat(job, file, &st) || !( S_IFREG & st.st_mode ))
		 && (l_flexStat(job, file_txt, &st) || !( S_IFREG & st.st_mode )))
#endif /* !PC */

		{
			LM_SET_ERROR(job, LM_NOCONFFILE, 359, errno, file, LM_ERRMASK_ALL);
			if (file_txt)
				free(file_txt);
			return 0;
		}
		lf->type = LF_FILE_PTR;
		if (job->options->cache_file)
		{
		  char *sp;

			sp = l_read_lfile(job, file);
			if (sp)
			{
				lf->type = LF_STRING_PTR;
				lf->ptr.str.s = sp;
				lf->ptr.str.cur = sp;
			}
			else
			{
				lf->type = LF_NO_PTR;
				lf = (LICENSE_FILE *) NULL;
			}
		}
		else if (
#ifdef PC
			!(lf->ptr.f = l_flexFopen(job, file, "rb")) &&
			!(lf->ptr.f = l_flexFopen(job, file_txt, "rb")))
#else
			!(lf->ptr.f = l_flexFopen(job, file, "r")) &&
			!(lf->ptr.f = l_flexFopen(job, file_txt, "r")))
#endif /* PC */
		{
			lf->type = LF_NO_PTR;
			lf = (LICENSE_FILE *) NULL;
			switch (errno)
			{
#ifndef NLM
				case EPERM:
#endif
				case EACCES:
					LM_SET_ERROR(job, LM_NOREADLIC, 72, errno, file, LM_ERRMASK_ALL);
					break;
				default:
					LM_SET_ERROR(job, LM_NOCONFFILE, 73, errno, file, LM_ERRMASK_ALL);
					break;
			}
		}
		else
		{
			lf->flags = LM_LF_FLAG_LINE_START;
		}
	}
	if (file_txt) free(file_txt);

	return(lf);
}
