/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_malloc.c,v 1.12 2003/06/12 16:57:17 sluu Exp $
 *
 *	Function:	l_malloc()
 *
 *	Description: 	Replacement for malloc
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  12/29/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "flex_file.h"
#include "l_prot.h"
#ifndef FLEXLM_ULTRALITE
#include <sys/types.h>
#endif	/* !FLEXLM_ULTRALITE */

#if !defined(RELEASE_VERSION) && !defined(FLEXLM_ULTRALITE)
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) {fprintf x; fflush(stderr);}
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif

/*
 *	l_malloc -- required for PC memory model problems
 *		    Useful to standardize malloc use.
 *		    Must memset memory to 0 -- use calloc for this
 *		    reason.
 */
void * API_ENTRY
l_malloc(
	LM_HANDLE *	job,
	size_t		length)
{
	void *	ret = NULL;
	static char *env = NULL;
	static FILE *fp;
	static int fail_num = 0;
	static int cnt = 0;
  	DEBUG_INIT
#if !defined(RELEASE_VERSION) && !defined(FLEXLM_ULTRALITE)
	if (env == NULL)
		env = l_real_getenv("FLEXLM_MALLOC_TEST");
	if (env && !fail_num)
	{
		if (fp = l_flexFopen(job, "malloc.tst", "r"))
		{
			fscanf(fp, "%d", &fail_num);	/* overrun checked */
			fail_num++;
			fclose(fp);
		}
		else
			fail_num = 1;
		
		if (fp = l_flexFopen(job, "malloc.tst", "w"))
		{
			fprintf(fp, "%d", fail_num);
			fclose(fp);
		}
		else
		{
			fprintf(stderr, "malloc test failed");
			fail_num = 0x7fffffff;
		}
	}
#endif /* RELEASE_VERSION */

	if (!length)
		return 0;

	if ( 
#if !defined(RELEASE_VERSION) && !defined(FLEXLM_ULTRALITE)
		(env && (fail_num == ++cnt)) ||
		(job && job->flags & LM_FLAG_MALLOC_FAIL) ||
#endif /* RELEASE_VERSION */
		!(ret =  (void *)calloc(1, length)))
	{
		if(job)
		{
			LM_SET_ERRNO(job, LM_CANTMALLOC, 0, 0);
			if (job->flags & LM_FLAG_TOOL_CATCH)
				longjmp(job->tool_catch, 1);
			if (job->flags & LM_FLAG_CATCH_SET)
				longjmp(job->jobcatch, 1);
		}
	}
	DEBUG((stderr, "malloc %d bytes: %x\n", length, ret));
	return ret;
}
#ifdef free
#undef free
#endif
/*
 *	For PC applications
 */
void
API_ENTRY
lc_free_mem(
	LM_HANDLE_PTR	job,	/* unused */
	LM_CHAR_PTR		mem)
{
	free(mem);
}

void
API_ENTRY
l_free(void * x)
{
  	DEBUG_INIT
	DEBUG((stderr, "free 0x%x\n", x));
	free(x);
}
