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
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent 
 *
 *		to run on an unlicensed platform is a violation of your 
 *
 *		license agreement with Macrovision Corporation.
 *
 *
 *****************************************************************************/
/*	
 *	Module: $Id: l_mddl.h,v 1.4 2003/01/13 23:26:15 kmaclean Exp $
 *	Function: l_intelid(job)
 *
 *	Description: Performs de-obfuscation processing.
 *
 *
 */

#ifndef L_MDDL_H
#define L_MDDL_H
#ifdef PC

void l_gcdecode( unsigned char *buff, int size );
void l_demuddle( LM_HANDLE_PTR, char**, int* );

#endif /* PC */
#endif
