/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_finder.c,v 1.8 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	l_finder() - Find the "license finder" and return path
 *
 *	Description: 	l_finder() locates the FLEXlm license finder and
 *			returns the path to the license file for this user.
 *
 *	Parameters:	(LM_HANDLE *) job - current job.
 *
 *	Return:		(char *) - license file path for this job.
 *
 *	M. Christiano
 *	7/23/94
 *
 *	Last changed:  11/13/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_socket.h"
#ifdef USE_WINSOCK
#include <pcsock.h>
#ifndef NLM		
#include "stderr.h"
#endif	/* NLM */
#endif /* USE_WINSOCK */
#include <sys/types.h>
#ifndef PC
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#endif
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif /* NO_UIO_H */
#ifdef ANSI
#include <stdlib.h>
#endif

#if !defined (htons) && !defined(i386) && !defined(OSF)	&& !defined(PC) && !defined(RHLINUX64)
			/*- Sun systems make this a macro! */
extern u_short htons();
#endif

unsigned short
API_ENTRY
l_finder_port()
{
	struct servent *	serv = NULL;
	unsigned short		port = 0;

	serv = getservbyname("FLEXlm", "tcp");
	if (serv)
		port = ntohs(serv->s_port);
	else
		port = (unsigned short) FLEXLM_PORT;

	return(port);
}

char *
API_ENTRY
l_finder(LM_HANDLE * job)
{
	char			path[1000] = {'\0'};
	char *			lfpath = NULL;
	unsigned short	port = 0;
	int				oldtimeout = 0, t = 2;
	static char *	timeout = (char *)-1;
	static char *	diag = (char *)-1;
	COMM_ENDPOINT	e;

	if (job->options->finder_path == (char *) NULL)
	{
/*
 *		The following statement is for early detection
 *		of the non-existence of FLEXlm finder host.
 *		This line improves performance a great deal on
 *		Windows and NT.
 */
		if (!gethostbyname(FLEXLM_FINDER_HOST))
			return 0;

		port = l_finder_port();
		(void) sprintf(path, "%d@%s,%d@%s,%d@%s",
			port, FLEXLM_FINDER_HOST,
			port, FLEXLM_FINDER_HOST_BACKUP,
			port, FLEXLM_FINDER_HOST_BACKUP2);
/*
 *		Temporarily set the conn timeout to 2 seconds
 */
		oldtimeout = job->options->conn_timeout;
		if (timeout == (char *)-1)
			timeout = getenv("FLEXLM_FINDER_TIMEOUT");	/* overrun checked */
		if (timeout)
			t = atoi(timeout);
		job->options->conn_timeout = t;

		if (diag == (char *)-1)
			diag = getenv("FLEXLM_DIAGNOSTICS");	/* overrun checked */
		if (diag)
			fprintf(stderr, "Using  FLEXlm finder: %s\n", path);

		lfpath = l_get_lfile(job, path, LM_FINDER_PATH, &e);
		job->options->conn_timeout = oldtimeout;
		if (lfpath)
		{
			job->options->finder_path = lfpath;
		}
	}
	lfpath = job->options->finder_path;
	return(lfpath);
}
