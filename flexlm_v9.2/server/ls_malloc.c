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
 *	Module: $Id: ls_malloc.c,v 1.4 2003/01/13 22:31:38 kmaclean Exp $
 *
 *	Function:	ls_malloc(size, line, file)
 *
 *	Description: 	mallocs (size) bytes, and exits on error.
 *
 *	M. Christiano
 *	5/10/90
 *
 *	Last changed:  9/8/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"

char *
ls_malloc(size, line, file)
unsigned size;
int line;
char *file;
{
  char *s;

#ifdef WINNT
	s = (char *) calloc(1, size);
#else
        s = (char *) l_malloc(lm_job, size);
#endif
	if (s == (char *) NULL)
	{
	    LOG((lmtext("malloc of %d bytes failed in module %s, line %d, exiting\n"),
					size, file, line));
	    ls_go_down(EXIT_MALLOC);
	    return(0);
	}
	else
	    return(s);
}
