/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_text.c,v 1.3 2003/04/18 23:48:05 sluu Exp $
 *
 *	Function:	l_text()
 *
 *	Description: 	Initializes locale and text domain information (Solaris)
 *
 *	Parameters:	None
 *
 *	Return:		None.  Locale and domain set to "" and "FLEXlm"
 *
 *	M. Christiano
 *	2/22/93
 *
 *	Last changed:  11/16/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifdef USE_GETTEXT
#include <locale.h>
#endif

void API_ENTRY
l_text()
{
#ifdef USE_GETTEXT
  char *x, y[LM_MAXPATHLEN+1];

	(void) setlocale(LC_MESSAGES, "");
	(void) textdomain(LM_TEXTDOMAIN);
	x = l_real_getenv("FLEXLM_TEXT");
	if (x)
	{
		(void) strcpy(y, x);	/* OVERRUN */
		(void) strcat(y, "/locale");
		bindtextdomain(LM_TEXTDOMAIN, y);
	}
#endif
}
