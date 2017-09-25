/******************************************************************************

	    COPYRIGHT (c) 1995, 1996 by Macrovision Corporation.
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
 *	Module: $Id: lg_stubs.c,v 1.2 2003/01/13 22:22:33 kmaclean Exp $
 *
 *	Function:
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  6/17/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "lg_genvd.h"

void
lg_add_fl(str, vc, flist) 
char *str;
VENDORCODE *vc;
FEATURE_LIST *flist;
{;}

FEATURE_LIST *
lg_flist(client, msg)
CLIENT_DATA *client;
char *msg;
{return (FEATURE_LIST *) 0;}

void 
issue_outofdate_warning(){;}
