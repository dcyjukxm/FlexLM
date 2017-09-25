/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ts_utils.c,v 1.3 2003/01/13 22:55:19 kmaclean Exp $
 *
 *	Function: rm(file), cp(file1, file2)
 *
 *	Description: Program interface to system utilities.
 *
 *	Parameters:	(char *) file* - File names
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	8/21/89
 *
 *	Last changed:  10/22/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef VMS
#define RM "del"
#define CP "copy"
#define XTRA ";*"
#else
#define RM "rm -f"
#define CP "cp"
#define XTRA ""
#endif

static char cmd[LM_MAXPATHLEN];

rm(file)
char *file;
{
	unlink(file);
}

cp(file1, file2)
char *file1, *file2;
{
  FILE *ifp;
  FILE *ofp;
  struct stat st;
  char *cp;

	stat(file1, &st);
	cp = (char *)malloc(st.st_size + 1);
	ifp = fopen(file1, "r");
	ofp = fopen(file2, "w");
	if (!ifp || !ofp)
	{
		/*fprintf(stderr, "%s to %s failed", file1, file2);*/
		/*perror("cp failed");*/
		return;
	}
	fread (cp, st.st_size, 1, ifp);
	fwrite(cp, st.st_size, 1, ofp);
	fclose(ifp);
	fclose(ofp);
	free(cp);
}
