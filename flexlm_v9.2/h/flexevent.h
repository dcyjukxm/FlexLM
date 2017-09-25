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
/*	$Id: flexevent.h,v 1.2 2003/06/11 00:29:45 sluu Exp $	*/

/**	@file 		flexevent.h
 *	@brief		Prototypes for event log functions
 *	@version	$Revision: 1.2 $
 *
*****************************************************************************/

#ifndef INCLUDE_FLEXEVENT_H
#define INCLUDE_FLEXEVENT_H

#ifdef WINNT

#include <stdio.h>
#include <windows.h>	/* For event log calls and defines */
#include "lmclient.h"

#define		FLEXEVENT_ERROR			EVENTLOG_ERROR_TYPE
#define		FLEXEVENT_WARN			EVENTLOG_WARNING_TYPE
#define		FLEXEVENT_INFO			EVENTLOG_INFORMATION_TYPE
#define		FLEXEVENT_AUDIT_SUCCES	EVENTLOG_AUDIT_SUCCESS
#define		FLEXEVENT_AUDIT_FAILURE	EVENTLOG_AUDIT_FAILURE

#define		EVENTLOG_SUBKEY				"SOFTWARE\\Macrovision Corporation\\FLEXlm License Manager"
#define		EVENTLOG_MODE				"%s_EventLoggingMode"
#define		EVENTLOG_NAME				"%s_LicenseServer"
#define		EVENTLOG_MODE_ENABLED		"ENABLE"
#define		EVENTLOG_MODE_DISABLED		"DISABLE"


int
l_flexEventLogIsEnabled();

FLEX_ERROR
l_flexEventLogInit(
	char *	szComputerName,
	char *	szVendorName);

void
l_flexEventLogCleanup();

FLEX_ERROR
l_flexEventLogAddString(
	LM_HANDLE *		job,
	char *			szInsertString);

void
l_flexEventLogFlushStrings(
	LM_HANDLE *		job);

FLEX_ERROR
l_flexEventLogWrite(
	LM_HANDLE *			job,
	unsigned short		type,
	unsigned short		cat,
	unsigned int		id,
	unsigned short		numStrings,
	const char **		ppszInsertionStrings,
	unsigned int		rawDataSize,
	void *				pRawData);


#endif /* WINNT */
#endif /* INCLUDE_FLEXEVENT_H */
