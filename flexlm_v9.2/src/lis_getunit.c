/******************************************************************************

	    COPYRIGHT (c) 1992, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lis_getunit.c,v 1.2 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function: lis_getunitid()
 *
 *	Description: "get unit id" replacement for gethostid
 *
 *	Parameters:	(long) provider_length - length of provider string
 *			(long) serial_length - length of serial # string
 *
 *	Return:		(char *) provider - hardware manufacturer
 *			(char *) serial - serial number of machine
 *
 *	M. Christiano
 *	11/27/91
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"

#ifdef SUNOS5
#include <sys/utsname.h>
#include <sys/systeminfo.h>
#endif

lis_getunitid(provider, provider_length, serial, serial_length)
char *provider;		/* Hardware manufacturer */
long provider_length;	/* max length of provider */
char *serial;		/* Serial # of system */
long serial_length;	/* Max length of serial # */
{
#ifdef SUNOS5
	*provider = '\0';
	sysinfo(SI_HW_SERIAL, serial, serial_length);
	sysinfo(SI_HW_PROVIDER, provider, provider_length);
	return(0);
#else
	return(-1);
#endif
}
