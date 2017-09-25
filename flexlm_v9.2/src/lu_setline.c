/******************************************************************************

	    COPYRIGHT (c) 1988, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lu_setline.c,v 1.4 2003/03/12 21:13:39 sluu Exp $
 *
 *	Function: lu_setlinebuf()
 *
 *	Description: 4.2bsd functions that are "missing" on various
 *		     FLEXlm platforms.
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"

#ifdef NO_setlinebuf

int lu_setlinebuf() { return 0; }

#endif
