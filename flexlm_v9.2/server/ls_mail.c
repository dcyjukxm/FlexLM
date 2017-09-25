/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

/*****************************************************************************/
/*
 *	Module: $Id: ls_mail.c,v 1.4 2003/01/13 22:31:38 kmaclean Exp $
 *
 *	Function: ls_mail(user, subj, msg)
 *
 *	Description: Sends a mail message "msg" to "user"
 *
 *	Parameters:	(char *) user - the username
 *			(char *) subj - The message subject
 *			(char *) msg - the messsage to be sent
 *
 *	Return:		None
 *
 *	M. Christiano
 *	2/22/88
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"

void
ls_mail(user, subj, msg)
char *user;
char *subj;
char *msg;
{
  char *header = "/usr/ucb/mail -s \"%s\" %s << EOF\n%s\nEOF";
  char *data;

	data = LS_MALLOC((unsigned) (strlen(header) + strlen(user) + 
			strlen(subj) + strlen(msg) + 1));
	(void) sprintf(data, header, subj, user, msg);
	(void) system(data);
	(void) free(data);
}
