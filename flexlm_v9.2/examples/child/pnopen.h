#ifndef PNOPEN_H
#define PNOPEN_H
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
 *	Module:	 $Id: pnopen.h,v 1.3 2003/01/14 22:38:18 kmaclean Exp $
 *
 *	Function:
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 * 	Author:  Jim McBeath, Macrovision Corporation
 *  	6.Aug.90  jimmc  Initial definition
 *
 *	Last changed:  %G%
 *
 */

#ifndef FILE
#include <stdio.h>
#endif

struct pnopen {
	int pid;
	int numfiles;
	FILE **files;
};

extern struct pnopen *pnopen();
extern int pnclose();

#endif PNOPEN_H
