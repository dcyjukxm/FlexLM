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
 *	Module: $Id: l_read_lfile.c,v 1.5 2003/04/18 23:48:04 sluu Exp $
 *
 *	Function:	l_read_lfile(job, file)
 *
 *	Description: 	Reads the license file data into memory
 *
 *	Parameters:	(char *) file - The file name
 *
 *	Return:		(char *) license file data (in malloced memory)
 *
 *	M. Christiano
 *	6/15/92
 *
 *	Last changed:  11/16/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "flex_file.h"
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>

char *
l_read_lfile(
	LM_HANDLE *	job,
	char *		file)	/* Current license job */
{
  char *ret = (char *) NULL;
  FILE *f;
#if !defined(OPENVMS)
  FILE *fopen();
#endif

	f = l_flexFopen(job, file, "r");
	if (f)
	{
	  long size;

		(void) fseek(f, 0L, 2);	/* Seek to end */
		size = ftell(f);
		(void) fseek(f, 0L, 0);	/* Seek to beginning */
/*
 *				+1 for null terminator
 */
		ret = malloc((unsigned) size + 1); 
		if (ret)
		{
		  unsigned char *in, *out;
/*
 *	memset to 0 for potential bug
 *
 */
			memset(ret,0,(unsigned) size + 1);
			(void) fread(ret, (size_t)size, 1, f);	/* overrun checked */
			(void) fclose(f);
			ret[size] = '\0';
/*
 *			Now, walk the whole "file", and remove continuation
 *			characters ....
 */
			in = out = (unsigned char *)ret;
			while (*in)
			{
				if ((*in == '\\') && (*(in+1) == '\n'))
				{
					in += 2;
				}
				else
				{
					/* smartquotes */
					if ((*in == 147) || (*in == 148))
						*in = '"';
					*out = *in;
					out++; in++;
				}
			}
			*out = '\0';
			
		}
	}
	else
	{
		switch (errno)
		{
#ifndef NLM
			case EPERM:
#endif
			case EACCES:
				LM_SET_ERROR(job, LM_NOREADLIC, 91, errno, file, LM_ERRMASK_ALL);
				break;
			default:
				LM_SET_ERROR(job, LM_NOCONFFILE, 92, errno, file, LM_ERRMASK_ALL);
				break;
		}
	}
	return(ret);
}
