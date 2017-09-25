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
 *	Module: $Id: ts_list.c,v 1.5 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Function: ts_list(feature)
 *
 *	Description: Performs the list command to the server.
 *
 *	Paramaters:	(char *)feature - the feature name to list
 *
 *	Return:		
 *
 *	M. Christiano
 *	3/14/88
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

LM_DATA_REF;

static char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
				"Thursday", "Friday", "Saturday"} ;

ts_list(feature)
char *feature;
{
  int rc;
  char string[200];
  LM_USERS *users;
  struct tm *t;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  int nusers = 0;
	
/*
 *	Get the list of people using this "feature"
 */
	users = lm_userlist(feature);
	if (users == 0 && _lm_errno != 0) 
	{
		lm_perror("lm_userlist");
	}
	while (users)
	{
	    if (*users->name == '\0')
	    {
		/* avail = users->nlic; 	 Total # of licenses */
	    }
	    else if (users->time == 0)
	    {
		printf("RESERVATION for %s %s\n",
					users->name[0] ? "USER" : "NODE",
					users->name[0] ? users->name : 
							users->node);
	    }
	    else
	    {
	        nusers++;
	        if (nusers == 1) printf("Users are:\n");
		{
		  time_t tt = users->time;

#ifdef THREAD_SAFE_TIME
			localtime_r(&tt, &tst);
			t = &tst;
#else /* !THREAD_SAFE_TIME */
			t = localtime(&tt);
#endif
		}
		printf("%s at %s", users->name, users->node);
		if (users->opts == INQUEUE)
		{
			printf(" queued for %d license%s",
				users->nlic, users->nlic > 1 ? "s":"");
		}
		else
		{
			if (users->nlic > 1)
				printf(", %d licenses", users->nlic);
			printf(", started"); 
		}
		printf(" on %s %d/%d/%d at %d:%s%d", days[t->tm_wday],
				t->tm_mon+1, t->tm_mday, t->tm_year,
				t->tm_hour, 
				t->tm_min < 10 ? "0" : "", t->tm_min);
		printf("\n");
	    }
	    users = users->next;
	}
	if (nusers > 0) printf("\n");
	return(nusers);
}
