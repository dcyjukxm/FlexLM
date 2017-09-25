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
 *	Module: $Id: ls_new_log.c,v 1.8 2003/06/11 00:29:45 sluu Exp $
 *
 *	Function: ls_new_log(cmd)
 *
 *	Description:	Closes the old logfile and re-opens it.
 *
 *	Parameters:	(char *) cmd - The new logfile message
 *
 *	Return:		None - logfile is re-opened.
 *
 *	M. Christiano
 *	10/30/89
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "flex_file.h"
#ifdef sco
#include <sys/types.h>
#endif
#ifndef NLM
#include <sys/file.h>
#endif
#include <errno.h>

static char oldlog[LM_MSG_LEN+1] = { '\0' };

void
ls_new_log(cmd)
char *cmd;
{

	cmd[LM_MSG_LEN] = '\0';		/* Make sure its terminated */

	LOG((lmtext("Logfile switching to %s\n"), &cmd[MSG_DATA]));
	LOG_INFO((INFORM, "The user has requested that the current logfile \
			be closed and replaced with the named file."));
	if (l_flexFreopen(lm_job, &cmd[MSG_DATA], "w", stdout) == (FILE *) NULL)
	{
		(void) fprintf(stderr,
			lmtext("Cannot open \"%s\" as log file (errno: %d)\n"),
				&cmd[MSG_DATA], errno);
	}
	if (oldlog[0])
	{
		LOG((lmtext("Logfile switched from %s\n"), oldlog));
	}
	(void) strcpy(oldlog, &cmd[MSG_DATA]);
}
