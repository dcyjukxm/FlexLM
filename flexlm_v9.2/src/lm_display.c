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
 *	Module: $Id: lm_display.c,v 1.9 2003/04/18 23:48:06 sluu Exp $
 *
 *	Function: lc_display(job, flag)
 *
 *	Description: 	Returns the current display name
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) flag - 0 for host system's idea of name
 *				   - <> 0 for FLEXlm's idea of name
 *
 *	Return:		(char *) display terminal name
 *
 *	M. Christiano
 *	3/16/90
 *
 *	Last changed:  1/5/99
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#ifdef PC
#include "l_wtsapi32.h"
#endif
#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

static char display[MAX_DISPLAY_NAME+1];

#ifdef VXWORKS
static char *ttyname(int fd)
{
	return NULL;
}	
#elif defined(PC)
typedef BOOL (WINAPI * WTSQUERYSESSIONINFORMATION)(HANDLE, 
						    DWORD, 
						    WTS_INFO_CLASS, 
						    LPTSTR *, 
						    DWORD *);


static char hostname[MAX_DISPLAY_NAME+1];

static char *ttyname(LM_HANDLE_PTR  job, int fd)
{
  WTSQUERYSESSIONINFORMATION GSIWTSQuerySessionInformation;
  LPTSTR szReturn;
  DWORD dwBytesReturn;
  int i, iReturn;
  char szClientName[255];
  HANDLE hWtsApi = 0;
  static char *nonet = (char *)-1;

	dwBytesReturn = 0;	

	if (nonet == (char *)-1)
		nonet = getenv( "LM_NO_NETWORK");	/* overrun checked */
		
	if (nonet)
	{
		return("console");
	}
	else
	{
	// if on terminal server set display name display name to remotes host name
		iReturn = isTSOK();
		if (iReturn == 1)
		{				
			hWtsApi = LoadLibrary("wtsapi32.dll");
			if (hWtsApi)
			{
				GSIWTSQuerySessionInformation = (WTSQUERYSESSIONINFORMATION)GetProcAddress(hWtsApi,"WTSQuerySessionInformationA");

				((* GSIWTSQuerySessionInformation )( WTS_CURRENT_SERVER_HANDLE,
									WTS_CURRENT_SESSION,
									WTSClientName,
									&szReturn,
									&dwBytesReturn));

/*				WTSQuerySessionInformation( WTS_CURRENT_SERVER_HANDLE,
  								WTS_CURRENT_SESSION,
								WTSClientName,
  								&szReturn,
								&dwBytesReturn ); */
				if (dwBytesReturn > 0)
				{
					strcpy(szClientName, "");
					for (i=0;i < (int)dwBytesReturn;i++)
						szClientName[i]=(char)szReturn[i];

					strcpy(hostname, szClientName);
				}
				else
				{	/* provide default hostname, should NEVER get here */
					(void) gethostname(hostname, MAX_DISPLAY_NAME);/* LONGNAMES */
					FreeLibrary(hWtsApi);
		                	return hostname;
				}
				FreeLibrary(hWtsApi);
				return hostname;

			}	/*end dll check*/
			/* assume on a machine not compliant with TS */
			(void) gethostname(hostname, MAX_DISPLAY_NAME);/* LONGNAMES */
                	return hostname  ;

		}	/*end if remote session*/
		else	/*local session*/
		{
			(void) gethostname(hostname, MAX_DISPLAY_NAME);/* LONGNAMES */
                	return hostname  ;
		}
        }
}
#endif

char * API_ENTRY
lc_display(job, flag)
LM_HANDLE *job;		/* Current license job */
int flag;
{
  char *tty, *ttyname();

	/* doesn't set errors */
	if (flag && job->options->display_override[0])
		return(job->options->display_override);
	else
	{
#ifdef PC
		tty = ttyname(job,0);
#else
		tty = ttyname(0);
#endif
		if (tty)
		{
			(void)l_zcp(display, tty, MAX_DISPLAY_NAME);/* LONGNAMES */
		}
		else
			(void)strcpy(display, "/dev/tty");
#ifdef VMS
/*
 *		Put the display into upper case so that DCL commands
 *		like "lmremove" will work properly
 */
		l_uppercase(display);
#endif
		if (!job->options->display_override[0])
		{
			l_zcp(job->options->display_override, display, 
							MAX_DISPLAY_NAME);/* LONGNAMES */
		}
		return(display);
	}
}
