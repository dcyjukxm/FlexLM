/******************************************************************************

	    COPYRIGHT (c) 1991, 1996 by Macrovision Corporation.
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
 *	Module: $Id: lm_isadmin.c,v 1.3 2002/11/11 21:45:30 kmaclean Exp $
 *
 *	Function: lc_isadmin
 *
 *	Description: returns true if the current user is part of the
 *		lmadmin group.
 *
 *	Jim McBeath
 *	3/25/91
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"		/*- To get strncpy declared right */
#include "lmclient.h"

#if !defined(UNCONDITIONAL_LMADMIN) && !defined(VXWORKS)
#include <grp.h>
#include <pwd.h>
#endif

#define ADMINNAME "lmadmin"
#define MAXUNAMELEN 256

/*- typical calling sequence in client: lc_isadmin(lc_username(job, 0)) */

int API_ENTRY		/* 1 if he's a good guy */
lc_isadmin(job, username)
LM_HANDLE *job; /* unused for now */
char *username;
{
#ifdef VXWORKS
	return 1;	 
#elif defined(UNCONDITIONAL_LMADMIN)
	/* Everyone is an NOT administrator on VMS/PC/NT */
	return(0);		
#else
  struct group *g;
  char name[MAXUNAMELEN+1];
  struct passwd *pw;
  char **p;
  int n;

	(void) strncpy(name,username,MAXUNAMELEN);
		/* copy it in case it is the return from lm_username,
		 * which is the return from getgruid, which gets
		 * overwritten when we call getpwnam. */
	pw = getpwnam(name);
	g = getgrnam(ADMINNAME);	/* get flexlm admin group */
	if (g) {
		/* lmadmin group is defined, only allow people in that group */
		if (pw && pw->pw_gid==g->gr_gid)
			return 1;	/* his group ID is lmadmin */
		for (p=g->gr_mem; *p; p++) {	/* see if he's in this group */
			if (strcmp(*p,name)==0)
				return 1;	/* found him in the list */
		}
		return 0;	/* not in lmadmin group, so not allowed */
	}
	/* No lmadmin group, try groups 0 and 1 */
	for (n=0; n<=1; n++) {
		g = getgrgid(n);
		if (g) {
			if (pw && pw->pw_gid==g->gr_gid)
				return 1;	/* his group ID is ok */
			for (p=g->gr_mem; *p; p++) {	/* see if in this grp */
				if (strcmp(*p,name)==0)
					return 1;	/* found him */
			}
		}
	}
	return 0;	/* no lmadmin, not in group 0 or 1 */
#endif
}
