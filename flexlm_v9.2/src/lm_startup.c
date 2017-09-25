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
 *	Module: $Id: lm_startup.c,v 1.2 2003/01/13 22:41:48 kmaclean Exp $
 *
 *	Function:	lc_startup(job, lmgrd_path, logfile, license_file)
 *
 *	Description: 	Starts lmgrd
 *
 *	Parameters:	(char *) lmgrd_path - Path to lmgrd
 *			(char *) logfile - Path to output log file (leading
 *					'+' to append to file).
 *			(char *) license_file - Path to license file
 *
 *	Return:	None - lmgrd is started
 *
 *	M. Christiano
 *	7/16/91
 *
 *	Last changed:  10/27/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

#define OVERHEAD 10	/*-  _ + -c_ + >_ + & + NULL + 2 spare */
API_ENTRY
lc_startup(job, lmgrd_path, logfile, license_file)
LM_HANDLE_PTR job; /* unused for now */
char *lmgrd_path;
char *logfile;
char *license_file;
{
  char startup[3*LM_MAXPATHLEN + OVERHEAD];

	l_clear_error(job);
	if (lmgrd_path && *lmgrd_path)
	{
		(void) strcpy(startup, lmgrd_path);
		(void) strcat(startup, " ");
		if (license_file && *license_file)
		{
			(void) strcat(startup, "-c ");
			(void) strcat(startup, license_file);
			(void) strcat(startup, " ");
		}
		if (logfile && *logfile)
		{
			if (*logfile == '+')
			{
				logfile++;
				(void) strcat(startup, ">");
			}
			(void) strcat(startup, "> ");
			(void) strcat(startup, logfile);
		}
		strcat(startup, "&");
		system(startup);
		return(1);
	}
	else
	{
		return(0);
	}
}
