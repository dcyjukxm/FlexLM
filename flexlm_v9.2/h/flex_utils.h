/****************************************************************************
	COPYRIGHT (c) 2003 by Macrovision Corporation.	
	This software has been provided pursuant to a License Agreement	
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Corporation and is protected by law.
	It may 	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Corporation.
*****************************************************************************/
/*	$Id: flex_utils.h,v 1.4.2.1 2003/06/27 18:41:21 sluu Exp $	*/

/**	@file		flex_utils.h
 *	@brief		Header file for utility functions.
 *	@version	$Revision: 1.4.2.1 $
 *
 *	This header file contains the protototypes for utility functions such
 *	as string conversion routines, etc....
 ****************************************************************************/

#ifndef INCLUDE_FLEX_UTILS_H
#define INCLUDE_FLEX_UTILS_H

#ifdef WINNT
#include <windows.h>
#endif /* WINNT */
#include "lmclient.h"
#include "l_prot.h"

#ifdef WINNT

int
l_getOSFamily();


wchar_t *
l_convertStringUTF8ToWC(
	LM_HANDLE *		job,
	const char *	pszInput,
	int *			piSize);


char *
l_convertStringWCToUTF8(
	LM_HANDLE *		job,
	const wchar_t *	pwszInput,
	int *			piSize);

char **
l_getUTF8Cmdline(
	LM_HANDLE *		job,
	int *			pArgc);

void
l_freeUTF8Cmdline(
	int				argc,
	char **			argv);

char *
l_regQueryValue(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int *	pdwType,
	int *			piBufSize);

int
l_regSetValue(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int	dwType,
	void *			pData,
	int 			iDataSize);

int
l_getUserName(
	LM_HANDLE *		job,
	char *			szBuffer,
	int				iBufferSize);

int
l_getHostname(
	LM_HANDLE *		job, 
	char *			szHostname,
	int				iSize);

#endif /* WINNT */

char *
l_convertStringMBToUTF8(
	LM_HANDLE *		job,
	const char *	pszInput,
	int *			piSize);

char *
l_convertStringUTF8ToMB(
	LM_HANDLE *		job,
	const char *	pszInput,
	int *			piSize);

int
l_getEnvironmentVariable(
	LM_HANDLE *		job,
	char *			szName,
	char *			szBuffer,
	int				iSize);


#endif /* INCLUDE_FLEX_UTILS_H */
