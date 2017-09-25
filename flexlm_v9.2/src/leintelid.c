/******************************************************************************

	    COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
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
 *	Module: $Id: leintelid.c,v 1.5 2003/01/13 23:18:46 kmaclean Exp $
 *	Function: l_intelid(job)
 *
 *	Description: Encoded ("muddled") cpu id functions.
 *
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_mddl.h"
#include "l_intelid.h"
#include "l_eintelid.h"

/*
 * This is the name of the character array that holds the encoded ("muddled")
 * cpu id sub routine.
 */

extern unsigned char l_gcspim[];

/************************************************************************
 *
 * load_cpuid_func  -  Recovers the encoded ("muddled") cpuid function
 *                     and loads it into memory.
 *
 ***********************************************************************/

static LM_GET_PSN_PROC
load_cpuid_func( LM_HANDLE_PTR job )
{
  void *base_region;
  unsigned char *buff;
  int size;

	buff = (unsigned char*) l_gcspim;
	l_demuddle( job, &buff, &size );
	l_gcdecode( buff, size );

	/*
	 * Compute the size necessary, then allocate this
	 * amount of memory.
	 */

	base_region = VirtualAlloc(
			NULL,
			size + 10,
			MEM_COMMIT,
			PAGE_EXECUTE_READWRITE );

	if( NULL == base_region )
		return NULL;

	/*
	 * Copy over the subroutine.
	 */

	memcpy( base_region, buff, size );
	memset( buff, 0, size );
	free( buff );

	return (LM_GET_PSN_PROC) base_region;
}

/************************************************************************
 *
 * release_cpuid_func  -  Releases the executable memory used for the
 *                        decoded cpuid function.
 *
 ***********************************************************************/

static void
release_cpuid_func( LM_HANDLE_PTR job, LM_GET_PSN_PROC base_region )
{
	VirtualFree(
		base_region,
		0,
		MEM_RELEASE );
}

/************************************************************************
 *
 * l_eintelid  -  Builds the decoded version of the cpu id function
 *                and uses it to build a host id list.
 *
 ***********************************************************************/

HOSTID*
l_eintelid( LM_HANDLE_PTR job)
{
	LM_GET_PSN_PROC proc;
	HOSTID *idptr;

	proc = load_cpuid_func( job );
	if( !proc )
		return NULL;
	idptr = l_intel_all( job, proc );
	release_cpuid_func( job, proc );

	return idptr;
}


