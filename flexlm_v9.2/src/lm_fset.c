/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_fset.c,v 1.12 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	lc_feat_set(job, daemon, code, &codes)
 *
 *	Description:	Returns a string of daemon's feature codes (encrypted).
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) daemon - Name of daemon.
 *			(VENDORCODE *) code - Vendor's code
 *			(char **) codes - "Raw" codes string
 *
 *	Return:		(char *) - The encrypted concatenated code 
 *
 *	Notes:		The strings are	malloc'ed.  Each subsequent call will 
 *			free the old string and re-malloc.
 *
 *	M. Christiano
 *	6/11/90
 *
 *	Last changed:  9/9/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"
#include "l_strkey.h"
#include <stdio.h>
#include <sys/types.h>

#if !defined(MOTO_88K) && !defined(PC)

#ifndef VXWORKS
#include <sys/file.h>
#else 
#include "iolib.h"
#endif

#endif
#include <errno.h>

static char *_strings = (char *) NULL;
static char *lc_feat_set_lfp lm_args((LM_HANDLE *, LICENSE_FILE *, char *daemon));

char * API_ENTRY
lc_feat_set(job, daemon, code, codes)
LM_HANDLE *job;		/* Current license job */
char *daemon;		/* Daemon to match on */
VENDORCODE *code;
char **codes;		/* Returned "raw" codes */
{
  LICENSE_FILE *lf;
  char *p;
  VENDORCODE tmpcode;
  unsigned long sig;
  int xor_arr[XOR_SEEDS_ARRAY_SIZ][4];
  int idx = (*job->vendor) % XOR_SEEDS_ARRAY_SIZ;
  XOR_SEEDS_INIT_ARRAY(xor_arr)

	if (LM_API_ERR_CATCH) return 0;

	if (_strings != (char *) NULL)		/* Free the old stuff */
	{
		(void) free(_strings);
		_strings = NULL;
	}
	if ((lf = l_open_file(job, LFPTR_CURRENT)) == (LICENSE_FILE *)NULL)
	{
		LM_API_RETURN(char *,_strings)
	}
	else
	{
		p = lc_feat_set_lfp(job, lf, daemon);
		l_lfclose(lf);
		if (codes) *codes = p;		/* Return raw codes */
/*
 *		Fix codes for signature
 */
		(void) memcpy((char *)&tmpcode, (char *)code, sizeof(tmpcode));
		if (! (job->flags & LM_FLAG_CLEAR_VKEYS))
			l_xorname(job->vendor, &tmpcode);
		l_sg(job, job->vendor, &tmpcode);
#if 0
		if (! (job->flags & LM_FLAG_CLEAR_VKEYS))
		{
			tmpcode.data[0] ^=
				(((long)(job->SEEDS_XOR[xor_arr[idx][0]])<<0)
				^((long)(job->SEEDS_XOR[xor_arr[idx][1]])<<8) 
				^((long)(job->SEEDS_XOR[xor_arr[idx][2]])<<16)
				^((long)(job->SEEDS_XOR[xor_arr[idx][3]])<<24));
			tmpcode.data[1] ^=
				(((long)(job->SEEDS_XOR[xor_arr[idx][0]])<<0)
				^((long)(job->SEEDS_XOR[xor_arr[idx][1]])<<8) 
				^((long)(job->SEEDS_XOR[xor_arr[idx][2]])<<16)
				^((long)(job->SEEDS_XOR[xor_arr[idx][3]])<<24));
		}
#endif
		if (p)
		{
		  char *ret;
			job->flags |= LM_FLAG_MAKE_OLD_KEY;
			ret =((char *) l_string_key(job, (unsigned char *)p, 
				strlen(p), &tmpcode, (unsigned long)
				job->options->sf));
			job->flags &= ~LM_FLAG_MAKE_OLD_KEY;
			LM_API_RETURN(char *, ret)
			
		}
		else
			LM_API_RETURN(char *, NULL)
	}
}

static
char *
lc_feat_set_lfp(job, lf, daemon)
LM_HANDLE *job;		/* Current license job */
LICENSE_FILE *lf;
char *daemon;		/* Daemon to match on */
{
  char line[MAX_CONFIG_LINE+1];
  char *f1, *f2, *f3, *f4, *f5, *f6, *f7;
  int nflds;
  int numfeat = 0, howbig = 0;

	f1 = (char *)l_malloc(job, (MAX_CONFIG_LINE+1)*7);	/* memory threat */
	f2 = f1+MAX_CONFIG_LINE+1;
	f3 = f2+MAX_CONFIG_LINE+1;
	f4 = f3+MAX_CONFIG_LINE+1;
	f5 = f4+MAX_CONFIG_LINE+1;
	f6 = f5+MAX_CONFIG_LINE+1;
	f7 = f6+MAX_CONFIG_LINE+1;

	while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
	{
	    nflds = sscanf(line, "%s %s %s %s %s %s %s", f1, f2, f3,	/* overrun checked */
						f4, f5, f6, f7);
	    if (nflds >= 7 && 
		(l_keyword_eq(job, f1, LM_RESERVED_FEATURE)  ||
		l_keyword_eq(job, f1, LM_RESERVED_INCREMENT)  ||
		l_keyword_eq(job, f1, LM_RESERVED_UPGRADE) ) && 
		l_keyword_eq(job, f3, daemon))
	    {				/* Found one */
		numfeat++;
		howbig += MAX_CRYPT_LEN;
	    }
	}
	if (numfeat > 0)
	{
	    (void) l_lfseek(lf, (long) 0, L_SET);
	    _strings = (char *) l_malloc(job, (unsigned) howbig + 1);	/* memory threat */
	    (void)strcpy(_strings, "");	/* Initialize it */
	    while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
	    {
		nflds = sscanf(line, "%s %s %s %s %s %s %s", f1, f2, f3,	/* overrun checked */
						f4, f5, f6, f7);
	        if (nflds >= 7 && 
			(l_keyword_eq(job, f1, LM_RESERVED_FEATURE)  ||
			l_keyword_eq(job, f1, LM_RESERVED_INCREMENT)  ||
			l_keyword_eq(job, f1, LM_RESERVED_UPGRADE) ) && 
				!strcmp(f3, daemon))
		{				/* Found one */
			strcat(_strings, f7);	/* Tack on the new code */
		}
	    }
	}

	free( f1 );
	return(_strings);
}
#include "l_strkey.c"
