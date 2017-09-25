/*****************************************************************************
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
/*	$Id: flexevent.c,v 1.2.2.1 2003/06/23 17:06:21 sluu Exp $	*/

/**	@file 		flexevent.c
 *	@brief		A set of rountines for writing to the event log (syslog
 *				possible someday...)
 *	@version	$Revision: 1.2.2.1 $
 *
*****************************************************************************/

#ifdef WINNT

#include "flex_utils.h"
#include "flexevent.h"
#include "lmclient.h"
#include "l_prot.h"

/*
 *	File static data
 */
static HANDLE	s_hEventLog = NULL;		/* event log handle */
static int		s_EventLogEnabled = 0;	/* state of event logging */


/****************************************************************************/
/**	@brief	Used to determine if event logging is enabled
 *
 *	@param	szVendorName	Name of vendor
 *
 *	@return	nonzero if enabled, else zero.
 ****************************************************************************/
static
int
sFlexEventLogIsEnabled(
	char *	szVendorName)
{
	HKEY	hKey = NULL;
	/* Vendor names limited to 8 chars max (really 10), so no overflow problems */
	char	buffer[MAX_PATH] = {'\0'};
	char	szValue[MAX_PATH] = {'\0'};
	int		type = 0;
	int		len = sizeof(szValue);
	int		rv = 0;

	if(szVendorName == NULL)
		goto done;

	sprintf(buffer, EVENTLOG_MODE, szVendorName);

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, EVENTLOG_SUBKEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if(RegQueryValueEx(hKey, buffer, NULL, &type, szValue, &len) == ERROR_SUCCESS)
		{
			if(stricmp(szValue, EVENTLOG_MODE_ENABLED) == 0)
				rv = 1;
		}
	}

done:
	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return rv;
}

/****************************************************************************/
/**	@brief	Used to determine if event logging is enabled
 *
 *	@param	szVendorName	Name of vendor
 *
 *	@return	nonzero if enabled, else zero.
 ****************************************************************************/
int
l_flexEventLogIsEnabled()
{
	return s_EventLogEnabled;
}


/****************************************************************************/
/**	@brief	Initialize event logging support
 *
 *	@param	szComputerName	Name of computer, if NULL then local computer
 *	@param	szVendorName	Name of vendor
 *
 *	@return	LM_NOERROR on success, else a FLEXlm error code.
 ****************************************************************************/
FLEX_ERROR
l_flexEventLogInit(
	char *	szComputerName,
	char *	szVendorName)
{
	int		err = LM_EVENTLOG_INIT_ERR;
	/* Vendor names limited to 8 chars max (really 10), so no overflow problems */
	char	buffer[MAX_PATH] = {'\0'};

	if(szVendorName == NULL)
		goto done;

	/*
	 *	Find out if event logging is enabled or not
	 */
	s_EventLogEnabled = sFlexEventLogIsEnabled(szVendorName);

	if(s_EventLogEnabled)
	{
		sprintf(buffer, EVENTLOG_NAME, szVendorName);

		if(s_hEventLog)
		{
			(void)DeregisterEventSource(s_hEventLog);
			s_hEventLog = NULL;
		}

		s_hEventLog = RegisterEventSource(szComputerName, buffer);
		if(s_hEventLog)
		{
			err = LM_NOERROR;
		}
		else
		{
			s_EventLogEnabled = 0;	/* turn off event logging */
		}
	}
	else
	{
		err = LM_EVENTLOG_DISABLED;
	}
done:
	return err;
}

/****************************************************************************/
/**	@brief	Cleanup routine for event logging
 *
 *	@param	NONE
 *
 *	@return	NONE
 ****************************************************************************/
void
l_flexEventLogCleanup()
{
	if(s_hEventLog)
	{
		(void)DeregisterEventSource(s_hEventLog);
		s_hEventLog = NULL;
	}
}

/****************************************************************************/
/**	@brief	Used to add an insertion string to event log message.
 *
 *	@param	job				FLEXlm job handle
 *	@param	szInsertString	String to add to insertion list for logging.
 *
 *	@return	LM_NOERROR if successful, else a FLEXlm error code.
 *
 *	@note	Storage for the string is allocated, the user must call
 *			l_flexEventLogFlushStrings() to free up this memory.
 ****************************************************************************/
FLEX_ERROR
l_flexEventLogAddString(
	LM_HANDLE *		job,
	char *			szInsertString)
{
	return LM_NOERROR;
}

/****************************************************************************/
/**	@brief	Used to free up resources allocted for insertion strings.
 *
 *	@param	job		FLEXlm job handle
 *
 *	@return	NONE
 ****************************************************************************/
void
l_flexEventLogFlushStrings(
	LM_HANDLE *		job)
{
	return;
}

/****************************************************************************/
/**	@brief	Write a message to event log.  Assumes input is in UTF-8 format
 *			and does a conversion to MB before writing the event.
 *
 *	@param	job				FLEXlm job handle
 *	@param	type			Message type (informational, error, warning,...)
 *	@param	cat				Category
 *	@param	id				ID of message to write
 *	@param	numStrings		Number of insertion strings in ppszInsertStr
 *	@param	ppszInsertStr	Array of insertion strings
 *	@param	rawDataSize		Size of raw data
 *	@param	pRawData		Raw data of size rawDataSize
 *
 *	@return	LM_NOERROR on success, else FLEXlm error code.
 ****************************************************************************/
FLEX_ERROR
l_flexEventLogWrite(
	LM_HANDLE *			job,
	unsigned short		type,
	unsigned short		cat,
	unsigned int		id,
	unsigned short		numStrings,
	const char **		ppszInsertionStrings,
	unsigned int		rawDataSize,
	void *				pRawData)
{
	FLEX_ERROR	err = LM_EVENTLOG_WRITE_ERR;
	int			i = 0;
	int			size = 0;
	char **		ppszStrings = NULL;
	char		szPID[MAX_PATH] = {'\0'};

	if( (s_hEventLog == NULL) || (numStrings && ppszInsertionStrings == NULL) )
		goto done;

	/*
	 *	Generate a new array and insert the PID as the first entry.
	 */
	ppszStrings = l_malloc(job, (sizeof(char *) * (++numStrings)));
	if(ppszStrings)
	{
		/*
		 *	Get PID
		 */
		sprintf(szPID, "%d", _getpid());
		ppszStrings[0] = szPID;
		for(i = 1; i < numStrings; i++)
		{
			/*
			 *	Convert from UTF-8 to MB
			 */
			ppszStrings[i] = l_convertStringUTF8ToMB(job,
													(char *)ppszInsertionStrings[i-1],
													&size);
			if(ppszStrings[i] == NULL)
			{
				/*
				 *	Couldn't do conversion, cleanup and error out
				 */
				numStrings = i;
				goto done;
			}
		}

		if(ReportEvent(s_hEventLog, type, cat, id, NULL, numStrings, rawDataSize,
				ppszStrings, pRawData))
		{
			err = LM_NOERROR;
		}
	}
	else
	{
		err = LM_CANTMALLOC;
	}

done:
	if(ppszStrings)
	{
		for(i = 1; i < numStrings; i++)
		{
			if(ppszStrings[i])
			{
				l_free(ppszStrings[i]);
				ppszStrings[i] = NULL;
			}
		}

		l_free(ppszStrings);
		ppszStrings = NULL;
	}

	return err;
}
#endif /* WINNT */