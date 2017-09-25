/******************************************************************************

	    COPYRIGHT (c) 1991, 2003 by Macrovision Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Inc.

 *****************************************************************************/
/*	
 *	Module:	 $Id: parent.h,v 1.2 2003/01/06 19:36:44 kmaclean Exp $ 
 *	Description: 	Definitions of child process parameters
 *
 *	M. Christiano
 *	12/17/90
 *
 *	Last changed:  %G%
 *
 */
#include "pnopen.h"

#define MAXBUF 100
typedef struct pnopen *CHILD_HANDLE;

CHILD_HANDLE start_child();
