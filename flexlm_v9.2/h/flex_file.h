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
/*	$Id: flex_file.h,v 1.10 2003/05/15 22:22:17 sluu Exp $	*/

/**	@file 		flex_file.h
 *	@brief		Wrapper function for file routines.
 *	@version	$Revision: 1.10 $
 *
 *	Wrapper functions for fopen/fclose, open/close.  Support UTF-8 on windows.
*****************************************************************************/

#ifndef INCLUDE_FLEX_FILE_H
#define INCLUDE_FLEX_FILE_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WINNT
#include <unistd.h>
#endif /* !WINNT */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"



FILE *
l_flexFopen(
	LM_HANDLE *		job,
	const char *	szFilename,
	const char *	szMode);

FILE *
l_flexFreopen(
	LM_HANDLE *		job,
	const char *	szFilename,
	const char *	szMode,
	FILE *			pFile);

int
l_flexFclose(FILE * fp);

int
l_flexOpen(
	LM_HANDLE *		job,
	const char *	szFilename,
	int				iOpenFlag,
	int				iPermission);

int
l_flexClose(int fd);

int
l_flexStat(
	LM_HANDLE *		job,
	char *			szFilename,
	struct stat *	pStatBuffer);

int
l_flexRename(
	LM_HANDLE *		job,
	char *			szFromFile,
	char *			szToFile);

int
l_flexAccess(
	LM_HANDLE *		job,
	char *			szFilename,
	int				iMode);

int
l_flexRemove(
	LM_HANDLE *		job,
	char *			szFilename);

int
l_flexUnlink(
	LM_HANDLE *		job,
	char *			szFilename);


#endif /* INCLUDE_FLEX_FILE_H */
