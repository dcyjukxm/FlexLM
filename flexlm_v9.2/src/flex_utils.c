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
/*	$Id: flex_utils.c,v 1.6.2.1 2003/06/27 18:41:22 sluu Exp $	*/

/**	@file 	flex_utils.c
 *	@brief	Various utility functions, such as string conversion rountines
 *	@version $Revision: 1.6.2.1 $
 *
 *	Utility functions used by FLEXlm, such as UTF-8 to Unicode conversion.
*****************************************************************************/

#include "flex_utils.h"

#ifdef WINNT


/****************************************************************************/
/**	@brief	Converts a multi-byte string to a wide character string.
 *			Can be	used to determine the buffer size needed to hold
 *			converted string.
 *
 *	@param	pszInput	multi-byte character string, NULL terminated.
 *	@param	pwszOutput	wchar_t buffer to hold result.
 *	@param	iSize		size of pwszOutput, if 0, pwszOutput is ignored and
 *							the	required buffer size is computed.
 *
 *	@return	0 on error.  If iSize is 0, the number of wchar_t's needed to
 *				hold result, else the number of wchar_t's written to
 *				pwszOutput.
 ****************************************************************************/
static
int
sConvertMBToWC(
	const char *	pszInput,
	wchar_t *		pwszOutput,
	int				iSize)
{
	if(!pszInput)
		return 0;
	/*
	 *	Do conversion IF iSize != 0, else return number of wchar_t's needed.
	 */
	return MultiByteToWideChar(CP_ACP,			/* code page */
								MB_ERR_INVALID_CHARS,	/* flag */
								pszInput,		/* name to convert */
								-1,				/* string is NULL terminated */
								pwszOutput,		/* ignored if iSize = 0 */
								iSize);			/* if iSize is 0, calc */
}

/****************************************************************************/
/**	@brief	Converts a multi-byte UTF-8 string to a wide character string.
 *			Can be	used to determine the buffer size needed to hold
 *			converted string.
 *
 *	@param	pszInput	multi-byte character string, NULL terminated.
 *	@param	pwszOutput	wchar_t buffer to hold result.
 *	@param	iSize		size of pwszOutput, if 0, pwszOutput is ignored and
 *							the	required buffer size is computed.
 *
 *	@return	0 on error.  If iSize is 0, the number of wchar_t's needed to
 *				hold result, else the number of wchar_t's written to
 *				pwszOutput.
 ****************************************************************************/
static
int
sConvertUTF8ToWC(
	const char *	pszInput,
	wchar_t *		pwszOutput,
	int				iSize)
{
	/* Byte Order Mask for UTF-8 */
	unsigned char	BOM[] = {0xEF, 0xBB, 0xBF};
	char *			pData = NULL;

	if(!pszInput)
		return 0;
	/*
	 *	Check to see if BOM is prepended, if yes, skip it
	 */
	if(strlen(pszInput) > sizeof(BOM))
	{
		if(memcmp(pszInput, BOM, sizeof(BOM)) == 0)
			pData = (char *)(pszInput + sizeof(BOM));
		else
			pData = (char *)pszInput;
	}
	else
		pData = (char *)pszInput;

	/*
	 *	Do conversion IF iSize != 0, else return number of wchar_t's needed.
	 */
	return MultiByteToWideChar(CP_UTF8,			/* code page */
								0,				/* flag */
								pData,			/* name to convert */
								-1,				/* string is NULL terminated */
								pwszOutput,		/* ignored if iSize = 0 */
								iSize);			/* if iSize is 0, calc */
}

/****************************************************************************/
/**	@brief	Converts a wide character string to a multi-byte UTF-8 character
 *			string.  Can be used to determine the buffer size needed to
 *			hold converted string.
 *
 *	@param	pwszInput	wide character string, NULL terminated.
 *	@param	pszOutput	char buffer to hold result.
 *	@param	iSize		size of pszOutput, if 0, pszOutput is ignored and
 *							the	required buffer size is computed.
 *
 *	@return	0 on error.  If iSize is 0, the number of char's needed to
 *				hold result, else the number of char's written to
 *				pszOutput.
 ****************************************************************************/
static
int
sConvertWCToUTF8(
	const wchar_t *	pwszInput,
	char *			pszOutput,
	int				iSize)
{
	if(!pwszInput)
		return 0;
	/*
	 *	Do conversion IF iSize != 0, else return number of wchar_t's needed.
	 */
	return WideCharToMultiByte(CP_UTF8,		/* code page */
								0,			/* required to be zero for CP_UTF8 */
								pwszInput,	/* name to convert */
								-1,			/* string is NULL terminated */
								pszOutput,	/* ignored if iSize = 0 */
								iSize,		/* if iSize is 0, calc */
								NULL,		/* use system default for unknown chars */
								NULL);		/* don't care if defaut char used */

}


/****************************************************************************/
/**	@brief	Converts a wide character string to a multi-byte character
 *			string.  Can be used to determine the buffer size needed to
 *			hold converted string.
 *
 *	@param	pwszInput	wide character string, NULL terminated.
 *	@param	pszOutput	char buffer to hold result.
 *	@param	iSize		size of pszOutput, if 0, pszOutput is ignored and
 *							the	required buffer size is computed.
 *
 *	@return	0 on error.  If iSize is 0, the number of char's needed to
 *				hold result, else the number of char's written to
 *				pszOutput.
 ****************************************************************************/
static
int
sConvertWCToMB(
	const wchar_t *	pwszInput,
	char *			pszOutput,
	int				iSize)
{
	if(!pwszInput)
		return 0;
	/*
	 *	Do conversion IF iSize != 0, else return number of wchar_t's needed.
	 */
	return WideCharToMultiByte(CP_ACP,		/* code page */
								0,	/* do the best we can */
								pwszInput,	/* name to convert */
								-1,			/* string is NULL terminated */
								pszOutput,	/* ignored if iSize = 0 */
								iSize,		/* if iSize is 0, calc */
								NULL,		/* use system default for unknown chars */
								NULL);		/* don't care if defaut char used */

}


/****************************************************************************/
/**	@brief	Utility to determine which version of windows your runningo on
 *
 *	@return	An int that specifies the windows OS family running on.
 ****************************************************************************/
int
l_getOSFamily()
{
	/*
	 *	Since this function is used to determine what OS your running on and the 
	 *	chances of that changing are NILL, ignore the race condition that could
	 *	occur here.  The assumption of course being that multiple threads would
	 *	get the same result when determining the OS family.  Once 'family' is set,
	 *	just use it.
	 */
	OSVERSIONINFO	os;
	static int		family = 0;
	static int		initialized = 0;

	if(!initialized)
	{
		memset(&os, 0, sizeof(OSVERSIONINFO));
		os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if(GetVersionEx(&os))
		{
			if(os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				/*
				 *	95/98/ME, don't really need to know exactly which of
				 *	the three it is.
				 */
				family = OS_95_98_ME;
			}
			else if(os.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				/*
				 *	NT/W2K/XP, don't really need to know exactly which of
				 *	the three it is.
				 */
				family = OS_NT_WIN2K_XP;
			}
			else if(os.dwPlatformId == VER_PLATFORM_WIN32s)
			{
				/*
				 *	Win32s on Windows 3.1, ouch!  Fortunately, we don't run on
				 *	Windows 3.1 anymore.
				 */
				family = OS_WIN32S;
			}
		}
		initialized = 1;
	}

	return family;
}

#endif /* WINNT */

/****************************************************************************/
/**	@brief	Utility function for converting UTF-8 a MB string.
 *
 *	@param	job			FLEXlm job handle
 *	@param	pszInput	multi-byte UTF-8 character string, NULL terminated.
 *	@param	piSize		pointer to int that will receive size of buffer
 *							returned, in wide characters.
 *
 *	@return	Pointer to a char buffer that contains MB
 *				representation of input string, or NULL on error.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
char *
l_convertStringUTF8ToMB(
	LM_HANDLE *		job,
	const char *	pszInput,
	int *			piSize)
{
#ifdef WINNT
	char *		pszOutput = NULL;
	wchar_t *	pwszTemp = NULL;
	int			status = 0;

	if( (pszInput == NULL) || (piSize == NULL) )
		goto done;

	/*
	 *	Initialize stuff
	 */
	*piSize = 0;

	/*
	 *	Determine size
	 */
	*piSize = sConvertUTF8ToWC(pszInput, NULL, 0);
	if(*piSize)
	{
		/*
		 *	Allocate buffer and do conversion.
		 */
		pwszTemp = l_malloc(job, sizeof(wchar_t) * ((*piSize) + 1));
		if(pwszTemp)
		{
			status = sConvertUTF8ToWC(pszInput, pwszTemp, *piSize + 1);
			if(status)
			{
				/*
				 *	Convert from WC to MB
				 */
				*piSize = sConvertWCToMB(pwszTemp, NULL, 0);
				if(*piSize)
				{
					pszOutput = l_malloc(job, sizeof(char) * ((*piSize) + 1));
					if(pszOutput)
					{
						status = sConvertWCToMB(pwszTemp, pszOutput, *piSize + 1);
					}
				}
			}
		}
	}

done:

	if(pwszTemp)
	{
		l_free(pwszTemp);
	}
	if(!status)
	{
		if(pszOutput)
		{
			l_free(pszOutput);
			pszOutput = NULL;
		}
		*piSize = 0;
	}

	return pszOutput;
#else
	return NULL;
#endif
}



#ifdef WINNT
/****************************************************************************/
/**	@brief	Utility function for converting MB UTF-8 a Wide Character string.
 *
 *	@param	job			FLEXlm job handle
 *	@param	pszInput	multi-byte UTF-8 character string, NULL terminated.
 *	@param	piSize		pointer to int that will receive size of buffer
 *							returned, in wide characters.
 *
 *	@return	Pointer to a wchar_t buffer that contains Wide Character
 *				representation of input string, or NULL on error.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
wchar_t *
l_convertStringUTF8ToWC(
	LM_HANDLE *		job,
	const char *	pszInput,
	int *			piSize)
{
	wchar_t	*	pwszOutput = NULL;
	int			status = 0;

	if( (pszInput == NULL) || (piSize == NULL) )
		goto done;

	/*
	 *	Initialize stuff
	 */
	*piSize = 0;

	/*
	 *	Determine size
	 */
	*piSize = sConvertUTF8ToWC(pszInput, NULL, 0);
	if(*piSize)
	{
		/*
		 *	Allocate buffer and do conversion.
		 */
		pwszOutput = l_malloc(job, sizeof(wchar_t) * ((*piSize) + 1));
		if(pwszOutput)
		{
			status = sConvertUTF8ToWC(pszInput, pwszOutput, *piSize + 1);
		}
	}

done:

	if(!status)
	{
		if(pwszOutput)
		{
			l_free(pwszOutput);
		}
		pwszOutput = NULL;
	}

	return pwszOutput;
}

/****************************************************************************/
/**	@brief	Utility function for converting Wide Character string to
 *			UTF-8 string.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	pwszInput	wide character string
 *	@param	piSize		pointer to int that will receive size of buffer
 *							returned.
 *
 *	@return	Pointer to a char buffer that contains a MB UTF-8 representation
 *				of the input string, else NULL.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
char *
l_convertStringWCToUTF8(
	LM_HANDLE *		job,
	const wchar_t *	pwszInput,
	int *			piSize)
{
	char *	pszOutput = NULL;
	int		status = 0;

	if( (pwszInput == NULL) || (piSize == NULL) )
		goto done;

	/*
	 *	Initialize stuff
	 */
	*piSize = 0;

	/*
	 *	Determine size
	 */
	*piSize = sConvertWCToUTF8(pwszInput, NULL, 0);
	if(*piSize)
	{
		/*
		 *	Allocate buffer and do conversion.
		 */
		pszOutput = l_malloc(job, sizeof(char) * ((*piSize) + 1));
		if(pszOutput)
		{
			status = sConvertWCToUTF8(pwszInput, pszOutput, *piSize + 1);
		}
	}

done:

	if(!status)
	{
		if(pszOutput)
		{
			l_free(pszOutput);
		}
		pszOutput = NULL;
	}

	return pszOutput;
}
#endif /* WINNT */

/****************************************************************************/
/**	@brief	Utility function for converting MB character string to a 
 *			UTF-8 string.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	pszInput	multi-byte character string, NULL terminated.
 *	@param	piSize		pointer to int that will receive size of buffer
 *							returned, in char's.
 *
 *	@return	Pointer to a char buffer that contains UTF-8 representation of
 *			input string, or NULL on error.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
char *
l_convertStringMBToUTF8(
	LM_HANDLE *		job,
	const char *	pszInput,
	int *			piSize)
{
#ifdef WINNT
	char *		pszOutput = NULL;
	wchar_t *	pwszTemp = NULL;
	int			status = 0;
	int			size = 0;

	if( (pszInput == NULL) || (piSize == NULL) )
		goto done;

	/*
	 *	Initialize stuff
	 */
	*piSize = 0;

	/*
	 *	Determine size
	 */
	size = sConvertMBToWC(pszInput, NULL, 0);
	if(size) 
	{
		/*
		 *	Allocate buffer and do conversion to wide characters.
		 */
		pwszTemp = l_malloc(job, sizeof(wchar_t) * (size + 1));
		if(pwszTemp)
		{
			status = sConvertMBToWC(pszInput, pwszTemp, size + 1);
			if(status)
			{
				/*
				 *	Convert from wide character to UTF-8
				 */
				pszOutput = l_convertStringWCToUTF8(job, pwszTemp, piSize);
			}
		}
	}

done:

	if(pwszTemp)
	{
		l_free(pwszTemp);
		pwszTemp = NULL;
	}

	return pszOutput;
#else
	return NULL;
#endif
}

#ifdef WINNT
/****************************************************************************/
/**	@brief	Utility for parsing the command line parameters.
 *
 *	@param	pszInput	Input string to parse
 *	@param	pszOutput	Buffer to put token into.
 *	@param	iSize		Size of pszOutput
 *
 *	@return	Remaining string after the token, else NULL if no more tokens.
 ****************************************************************************/
static
char *
sGetToken(char * pszInput, char * pszOutput, int iSize)
{
	char *	pszToken = NULL;

	if( (pszInput == NULL) || (pszOutput == NULL) || (iSize == 0) )
		goto done;

	memset(pszOutput, 0, iSize);
	/*
	 *	Eat white space characters
	 */
	while(*pszInput == ' ')
		pszInput++;

	if(pszInput[0] == '"')
	{
		/*
		 *	Quoted string
		 */
		pszInput++;
		pszToken = strstr(pszInput, "\"");
	}
	else
	{
		pszToken = strstr(pszInput, " ");
	}

	if(pszToken)
	{
		memcpy(pszOutput, pszInput, (pszToken - pszInput) > iSize ? 
										iSize : (pszToken - pszInput));
		if(pszToken[0] == '\"')
			pszToken++;
	}
	else
	{
		strncpy(pszOutput, pszInput, iSize);
	}
done:
	return pszToken;
}

/****************************************************************************/
/**	@brief	Utility for counting number of arguments in command line
 *
 *	@param	pszInput	UTF-8 command line string
 *
 *	@return	Number of arguments on command line.
 ****************************************************************************/
static
int
sCountCmdlineParameters(char * pszInput)
{
	char	szArgument[MAX_PATH] = {'\0'};
	int		argc = 0;

	if(pszInput == NULL)
		goto done;

	do
	{
		memset(szArgument, 0, sizeof(szArgument));
		pszInput = sGetToken(pszInput, szArgument, sizeof(szArgument));
		if(szArgument[0] != '\0')
			argc++;

	} while(pszInput);

done:

	return argc;
}

/****************************************************************************/
/**	@brief	Utility for generating a UTF-8 version of argv
 *
 *	@param	job			FLEXlm job handle
 *	@param	pszInput	UTF-8 command line string
 *	@param	pArgc		Pointer to int that will receive number of arguments.
 *
 *	@return	Array of pointers to command line arguments, else NULL on error.
 *	@note	User is responsible for freeing memory returned via
 *			l_freeUTF8Cmdline.
 ****************************************************************************/
static
char **
sGetCmdline(
	LM_HANDLE *		job,
	char *			pszInput,
	int *			pArgc)
{
	char	szArgument[MAX_PATH] = {'\0'};
	char **	ppszArgv = NULL;
	int		i = 0;
	int		j = 0;

	if(pszInput && pArgc)
	{
		*pArgc = 0;

		/*
		 *	Count arguments
		 */
		*pArgc = sCountCmdlineParameters(pszInput);

		if(*pArgc)
		{
			ppszArgv = l_malloc(job, sizeof(char *) * (*pArgc));
			if(ppszArgv)
			{
				for(i = 0; i < *pArgc && pszInput; i++)
				{
					pszInput = sGetToken(pszInput, szArgument, sizeof(szArgument));
					if(szArgument[0] != '\0')
					{
						ppszArgv[i] = l_malloc(job,
										sizeof(char) * (strlen(szArgument) + 1));
						if(ppszArgv[i])
						{
							memset(ppszArgv[i], 0, strlen(szArgument) + 1);
							strcpy(ppszArgv[i], szArgument);
						}
						else
						{
							for(j = 0; j < i; j++)
							{
								if(ppszArgv[j])
								{
									l_free(ppszArgv[j]);
									ppszArgv[j] = NULL;
								}
							}
							*pArgc = 0;
							l_free(ppszArgv);
							ppszArgv = NULL;
							break;
						}
					}
					else
					{
						/*
						 *	Error getting command line args.
						 */
						for(j = 0; j < i; j++)
						{
							if(ppszArgv[j])
							{
								l_free(ppszArgv[j]);
								ppszArgv[j] = NULL;
							}
						}
						*pArgc = 0;
						l_free(ppszArgv);
						ppszArgv = NULL;
						break;
					}
				}
			}
		}
	}
	return ppszArgv;
}

/****************************************************************************/
/**	@brief	Utility for generating new argv from wide character version.  New
 *			argv is returned in UTF8 format.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	argc		Number of entries in argv
 *	@param	argv		Wide character array of command line arguments.  Size
 *						of array is argc entries.
 *
 *	@return	pointer to a new array of command line parameters in UTF-8 format
 *	@note	The user if responsible for calling l_freeUTF8Cmdline() when done.
 ****************************************************************************/
char **
l_getUTF8Cmdline(
	LM_HANDLE *		job,
	int *			pArgc)
{
	wchar_t *	pwszCmdline = NULL;
	char **		ppszArgv = NULL;
	char *		pszUTF8Cmdline = NULL;
	int			iSize = 0;
	int			i = 0;
	int			j = 0;

	if(pArgc == NULL)
		goto done;

	*pArgc = 0;

	/*
	 *	Get wide character version of command line
	 */
	pwszCmdline = GetCommandLineW();	/* available on all windows platforms */

	if(pwszCmdline)
	{
		/*
		 *	Convert to UTF-8
		 */
		pszUTF8Cmdline = l_convertStringWCToUTF8(job, pwszCmdline, &iSize);
		if(pszUTF8Cmdline)
		{
			ppszArgv = sGetCmdline(job, pszUTF8Cmdline, pArgc);
		}
	}

done:

	if(pszUTF8Cmdline)
	{
		l_free(pszUTF8Cmdline);
		pszUTF8Cmdline = NULL;
	}

	return ppszArgv;
}


/****************************************************************************/
/**	@brief	Utility for freeing data returned by call to l_convertArgvWCToUTF8
 *
 *	@param	argc		Number of entries in argv
 *	@param	argv		Dynamically allocated array of strings to free, number
 *						of entries in array is argc. *
 *	@return	NOTHING
 ****************************************************************************/
void
l_freeUTF8Cmdline(
	int				argc,
	char **			argv)
{
	int i = 0;

	if( (argc == 0) || (argv == NULL) )
		goto done;

	for(i = 0; i < argc; i++)
	{
		if(argv[i])
		{
			l_free(argv[i]);
			argv[i] = NULL;
		}
	}
	l_free(argv);

done:
	return;
}

/****************************************************************************/
/**	@brief	Utility for reading data from registry in multi-byte.
 *			If data is of type	REG_SZ, data is converted to UTF-8.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	hKey		Handle to registry key
 *	@param	szValue		Value to query
 *	@param	pdwType		Pointer to unsigned int that will receive type
 *	@param	piBufSize	Pointer to int that will receive size of buffer
 *						returned.
 *
 *	@return	Pointer to a buffer that contains data at specified key/value
 *			on success, else NULL.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
static
char *
sFlexRegQueryValueA(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int *	pdwType,
	int *			piBufSize)
{
	char *		pszValue = NULL;
	char *		pszBuffer = NULL;
	char *		pszData = NULL;

	int			iSize = 0;
	int			err = 0;

	if( (hKey == NULL) || (szValue == NULL) || (pdwType == NULL) || (piBufSize == NULL) )
		goto done;

	*piBufSize = 0;
	*pdwType = 0;

	/*
	 *	Convert input to MB
	 */
	pszValue = l_convertStringUTF8ToMB(job, szValue, &iSize);
	
	if(pszValue && iSize)
	{
		/*
		 *	Determine the size of buffer needed to hold data.
		 */
		err = RegQueryValueExA(hKey, pszValue, NULL, pdwType, NULL, &iSize);
		if(err == ERROR_SUCCESS)
		{
			pszBuffer = l_malloc(job, sizeof(char) * (iSize + 1));
			if(pszBuffer)
			{
				err = RegQueryValueEx(hKey, pszValue, NULL, pdwType, pszBuffer, &iSize);
				if(err == ERROR_SUCCESS)
				{
					if(*pdwType == REG_SZ || *pdwType == REG_EXPAND_SZ)
					{
						/*
						 *	Do conversion to MB to UTF-8
						 */
						pszData = l_convertStringMBToUTF8(job, pszBuffer, piBufSize);
					}
					else
					{
						pszData = pszBuffer;
						pszBuffer = NULL;	/* Caller will free it */
						*piBufSize = iSize;
					}
				}
			}
		}
	}

done:

	if(pszValue)
	{
		l_free(pszValue);
		pszValue = NULL;
	}
	if(pszBuffer)
	{
		l_free(pszBuffer);
		pszBuffer = NULL;
	}

	return pszData;

}


/****************************************************************************/
/**	@brief	Utility for reading data from registry in wide characters.
 *			If data is of type	REG_SZ, data is converted to UTF-8.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	hKey		Handle to registry key
 *	@param	szValue		Value to query
 *	@param	pdwType		Pointer to unsigned int that will receive type
 *	@param	piBufSize	Pointer to int that will receive size of buffer
 *						returned.
 *
 *	@return	Pointer to a buffer that contains data at specified key/value
 *			on success, else NULL.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
static
char *
sFlexRegQueryValueW(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int *	pdwType,
	int *			piBufSize)
{
	wchar_t	*		pwszValue = NULL;
	char *			pszBuffer = NULL;
	char *			pszData = NULL;

	int				iSize = 0;
	int				err = 0;

	if( (hKey == NULL) || (szValue == NULL) || (pdwType == NULL) || (piBufSize == NULL) )
		goto done;

	*piBufSize = 0;
	*pdwType = 0;

	/*
	 *	Convert input to WC
	 */
	pwszValue = l_convertStringUTF8ToWC(job, szValue, &iSize);
	
	if(pwszValue && iSize)
	{
		/*
		 *	Determine the size of buffer needed to hold data.
		 */
		err = RegQueryValueExW(hKey, pwszValue, NULL, pdwType, NULL, &iSize);
		if(err == ERROR_SUCCESS)
		{
			pszBuffer = l_malloc(job, sizeof(char) * (iSize + 1));
			if(pszBuffer)
			{
				err = RegQueryValueExW(hKey, pwszValue, NULL, pdwType, pszBuffer, &iSize);
				if(err == ERROR_SUCCESS)
				{
					if(*pdwType == REG_SZ || *pdwType == REG_EXPAND_SZ)
					{
						/*
						 *	Do conversion to UTF-8 from WC
						 */
						pszData = l_convertStringWCToUTF8(job, (wchar_t *)pszBuffer, piBufSize);
					}
					else
					{
						pszData = pszBuffer;
						pszBuffer = NULL;	/* Caller will free it */
						*piBufSize = iSize;
					}
				}
			}
		}
	}

done:

	if(pwszValue)
	{
		l_free(pwszValue);
		pwszValue = NULL;
	}
	if(pszBuffer)
	{
		l_free(pszBuffer);
		pszBuffer = NULL;
	}

	return pszData;

}

/****************************************************************************/
/**	@brief	Utility for reading data from registry, if data is of type
 *			REG_SZ, data is converted to UTF-8.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	hKey		Handle to registry key
 *	@param	szValue		Value to query
 *	@param	pdwType		Pointer to unsigned int that will receive type
 *	@param	piBufSize	Pointer to int that will receive size of buffer
 *						returned.
 *
 *	@return	Pointer to a buffer that contains data at specified key/value
 *			on success, else NULL.
 *	@note	The user is responsible for freeing the buffer when finished.
 ****************************************************************************/
char *
l_regQueryValue(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int *	pdwType,
	int *			piBufSize)
{
	char *	pszData = NULL;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		pszData = sFlexRegQueryValueA(job, hKey, szValue, pdwType, piBufSize);
	}
	else
	{
		pszData = sFlexRegQueryValueW(job, hKey, szValue, pdwType, piBufSize);
	}
	return pszData;
}

/****************************************************************************/
/**	@brief	Utility for writing data to the registry multi-byte format.
 *			If data is of type	REG_SZ, data is converted to multi-byte..
 *
 *	@param	job			FLEXlm job handle.
 *	@param	hKey		Handle to registry key
 *	@param	szValue		Value to query
 *	@param	dwType		Type that pData is.
 *	@param	pData		The data to write to the registry.  UTF-8 if REG_SZ.
 *	@param	iDataSize	Size of pData, in bytes.
 *
 *	@return	0 on success, else non zero.
 ****************************************************************************/
static
int
sFlexRegSetValueA(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int	dwType,
	void *			pData,
	int				iDataSize)
{
	char *		pszValue = NULL;
	char *		pszData = NULL;
	int			iSize = 0;
	int			err = 0;

	if( (hKey == NULL) || (szValue == NULL) || (pData == NULL) || (iDataSize == 0) )
		goto done;

	if(dwType == REG_SZ || dwType == REG_EXPAND_SZ)
	{
		/*
		 *	Convert input to MB
		 */
		pszValue = l_convertStringUTF8ToMB(job, szValue, &iSize);
		if(pszValue == NULL)
			goto done;
		pszData = (char *)l_convertStringUTF8ToMB(job, pData, &iSize);
		if(pszData == NULL)
			goto done;
		iSize;
		err = RegSetValueExA(hKey, pszValue, 0, dwType, pszData, iSize);
	}
	else
	{
		err = RegSetValueExA(hKey, szValue, 0, dwType, (const char *)pData, iDataSize);
	}

done:

	if(pszValue)
	{
		l_free(pszValue);
		pszValue = NULL;
	}
	if(pszData)
	{
		l_free(pszData);
		pszData = NULL;
	}

	return err;

}


/****************************************************************************/
/**	@brief	Utility for writing data to the registry in wide character.
 *			If data is of type REG_SZ, data is converted to wide character.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	hKey		Handle to registry key
 *	@param	szValue		Value to query
 *	@param	dwType		Type that pData is.
 *	@param	pData		The data to write to the registry.	UTF-8 if REG_SZ
 *	@param	iDataSize	Size of pData, in bytes.
 *
 *	@return	0 on success, else non zero.
 ****************************************************************************/
static
int
sFlexRegSetValueW(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int	dwType,
	void *			pData,
	int				iDataSize)
{
	wchar_t	*		pwszValue = NULL;
	char *			pszData = NULL;
	int				iSize = 0;
	int				err = 0;

	if( (hKey == NULL) || (szValue == NULL) || (pData == NULL) || (iDataSize == 0) )
		goto done;

	if(dwType == REG_SZ || dwType == REG_EXPAND_SZ)
	{
		/*
		 *	Convert input to WC
		 */
		pwszValue = l_convertStringUTF8ToWC(job, szValue, &iSize);
		if(pwszValue == NULL)
			goto done;
		pszData = (char *)l_convertStringUTF8ToWC(job, pData, &iSize);
		if(pszData == NULL)
			goto done;
		iSize *= 2;	/* size returned is in terms of wide characters, need to change to bytes */
		err = RegSetValueExW(hKey, pwszValue, 0, dwType, pszData, iSize);
	}
	else
	{
		err = RegSetValueExA(hKey, szValue, 0, dwType, (const char *)pData, iDataSize);
	}

done:

	if(pwszValue)
	{
		l_free(pwszValue);
		pwszValue = NULL;
	}
	if(pszData)
	{
		l_free(pszData);
		pszData = NULL;
	}

	return err;

}

/****************************************************************************/
/**	@brief	Utility for writing data to the registry, if data is of type
 *			REG_SZ, data is converted to wide character.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	hKey		Handle to registry key
 *	@param	szValue		Value to query
 *	@param	dwType		Type that pData is.
 *	@param	pData		The data to write to the registry
 *	@param	iDataSize	Size of pData, in bytes.
 *
 *	@return	0 on success, else non zero.
 ****************************************************************************/
int
l_regSetValue(
	LM_HANDLE *		job,
	HKEY			hKey,
	char *			szValue,
	unsigned int	dwType,
	void *			pData,
	int 			iDataSize)
{
	int				err = 1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		err = sFlexRegSetValueA(job, hKey, szValue, dwType, pData, iDataSize);
	}
	else
	{
		err = sFlexRegSetValueW(job, hKey, szValue, dwType, pData, iDataSize);
	}

	return err;
}

/****************************************************************************/
/**	@brief	Utility for obtaining the username in UTF-8 format.  Data is
 *			read in multi-byte format then converted.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szBuffer	Pointer to char buffer that will receive the username
 *						in UTF-8 format.
 *	@param	iBufferSize	Size, in bytes of szBuffer.  If 0, szBuffer is
 *						ignored and the number of bytes required to hold the
 *						username is returned, including terminating NULL.
 *
 *	@return	Number of bytes copied into szBuffer, up to iBufferSize - 1, else
 *			the number of bytes required to hold username.  Returns 0 on
 *			error.
 ****************************************************************************/
static
int
sFlexGetUserNameA(
	LM_HANDLE *		job,
	char *			szBuffer,
	int				iBufferSize)
{
	int			len = 0;
	int			rv = 0;
	int			status = 0;
	char *		pszBuffer = NULL;
	char *		pszUTF8Buffer = NULL;

	if( (iBufferSize && szBuffer == NULL) )
		goto done;

	/*
	 *	Calculate buffer size needed
	 */
	(void)GetUserNameA(szBuffer, (unsigned int *)&len);

	if(len)
	{
		pszBuffer = l_malloc(job, (sizeof(char) * len));
		if(pszBuffer)
		{
			status = GetUserNameA(pszBuffer, (unsigned int *)&len);
			if(status)
			{
				/*
				 *	Convert from MB to UTF-8
				 */
				pszUTF8Buffer = l_convertStringMBToUTF8(job, pszBuffer, &len);
				if(pszUTF8Buffer)
				{
					if(iBufferSize != 0)
					{
						/*
						 *	Copy upto iBufferSize - 1 bytes
						 */
						if(len > iBufferSize)
							len = iBufferSize - 1;
						memset(szBuffer, 0, iBufferSize);
						memcpy(szBuffer, pszUTF8Buffer, len);
					}
					rv = len;
				}
			}
		}
	}


done:
	if(pszBuffer)
	{
		l_free(pszBuffer);
		pszBuffer = NULL;
	}
	if(pszBuffer)
	{
		l_free(pszBuffer);
		pszBuffer = NULL;
	}
	return rv;

}


/****************************************************************************/
/**	@brief	Utility for obtaining the username in UTF-8 format.  Data is
 *			read in wide character format then converted.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szBuffer	Pointer to char buffer that will receive the username
 *						in UTF-8 format.
 *	@param	iBufferSize	Size, in bytes of szBuffer.  If 0, szBuffer is
 *						ignored and the number of bytes required to hold the
 *						username is returned, including terminating NULL.
 *
 *	@return	Number of bytes copied into szBuffer, up to iBufferSize - 1, else
 *			the number of bytes required to hold username.  Returns 0 on
 *			error.
 ****************************************************************************/
static
int
sFlexGetUserNameW(
	LM_HANDLE *		job,
	char *			szBuffer,
	int				iBufferSize)
{
	int				len = 0;
	int				rv = 0;
	int				status = 0;
	wchar_t *		pwszBuffer = NULL;
	char *			pszBuffer = NULL;

	if( (iBufferSize && szBuffer == NULL) )
		goto done;

	/*
	 *	Calculate buffer size needed
	 */
	(void)GetUserNameW((wchar_t *)szBuffer, (unsigned int *)&len);

	if(len)
	{
		pwszBuffer = l_malloc(job, (sizeof(wchar_t) * len));
		if(pwszBuffer)
		{
			status = GetUserNameW(pwszBuffer, (unsigned int *)&len);
			if(status)
			{
				/*
				 *	Convert from wchar_t to char
				 */
				pszBuffer = l_convertStringWCToUTF8(job, pwszBuffer, &len);
				if(pszBuffer)
				{
					if(iBufferSize != 0)
					{
						/*
						 *	Copy upto iBufferSize - 1 bytes
						 */
						if(len > iBufferSize)
							len = iBufferSize - 1;
						memset(szBuffer, 0, iBufferSize);
						memcpy(szBuffer, pszBuffer, len);
					}
					rv = len;
				}
			}
		}
	}


done:
	if(pwszBuffer)
	{
		l_free(pwszBuffer);
		pwszBuffer = NULL;
	}
	if(pszBuffer)
	{
		l_free(pszBuffer);
		pszBuffer = NULL;
	}
	return rv;

}

/****************************************************************************/
/**	@brief	Utility for obtaining the username in UTF-8 format.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szBuffer	Pointer to char buffer that will receive the username
 *						in UTF-8 format.
 *	@param	iBufferSize	Size, in bytes of szBuffer.  If 0, szBuffer is
 *						ignored and the number of bytes required to hold the
 *						username is returned, including terminating NULL.
 *
 *	@return	Number of bytes copied into szBuffer, up to iBufferSize - 1, else
 *			the number of bytes required to hold username.  Returns 0 on
 *			error.
 ****************************************************************************/
int
l_getUserName(
	LM_HANDLE *		job,
	char *			szBuffer,
	int				iBufferSize)
{
	int				rv = 0;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		rv = sFlexGetUserNameA(job, szBuffer, iBufferSize);
	}
	else
	{
		rv = sFlexGetUserNameW(job, szBuffer, iBufferSize);
	}

	return rv;
}

/****************************************************************************/
/**	@brief	Utility for obtaining the hostname in UTF-8 format.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szHostname	Buffer to hold hostname	in UTF-8 format.
 *	@param	iSize		Size, in bytes of szHostname.
 *
 *	@return	0 on success, else SOCKET_ERROR (-1) on error.
 ****************************************************************************/
int
l_getHostname(
	LM_HANDLE *		job, 
	char *			szHostname,
	int				iSize)
{
#ifdef WINNT
	int			err = SOCKET_ERROR;
	int			len = 0;
	char *		pszUTF8 = NULL;

	if( (szHostname == NULL) || iSize == 0 )
		goto done;

	err = gethostname(szHostname, iSize);
	if(!err)
	{
		/*
		 *	According to Citrix, this will give me a multi-byte string, which
		 *	I then need to convert to wide characters and then to UTF-8
		 */
		pszUTF8 = l_convertStringMBToUTF8(job, szHostname, &len);
		if(pszUTF8)
		{
			strncpy(szHostname, pszUTF8, len > iSize ? iSize - 1 : len - 1);
		}
		else
		{
			err = SOCKET_ERROR;	/* Return some error or another */
		}
	}

done:

	if(pszUTF8)
	{
		l_free(pszUTF8);
		pszUTF8 = NULL;
	}

	return err;
#else /* !WINNT */
	return gethostname(szHostname, iSize);
#endif
}

/****************************************************************************/
/**	@brief	Utility for getting environment variables, data is returned in
 *			UTF-8 format, read in multi-byte format.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szName		Name of environment variable, assume in UTF-8 format.
 *	@param	szBuffer	Buffer to hold environment data	in UTF-8 format.
 *	@param	iSize		Size, in bytes of szBuffer, if 0, szBuffer is ignored
 *						and the size of buffer required to store the value
 *						is computed.
 *
 *	@return	0 on error, else size of number of bytes copied or required.
 ****************************************************************************/
static
int
sFlexGetEnvVarA(
	LM_HANDLE *		job,
	char *			szName,
	char *			szBuffer,
	int				iSize)
{
	int			len = 0;
	char *		pszData = NULL;
	char *		pszName = NULL;
	char *		pszTemp = NULL;


	/*
	 *	Convert name to multi-byte
	 */
	pszName = l_convertStringUTF8ToMB(job, szName, &len);
	if(pszName)
	{
		/*
		 *	Determine size of buffer needed to hold wide
		 *	character version of data.
		 */
		len = GetEnvironmentVariableA(pszName, NULL, 0);
		/*
		 *	Check to see if only interested if environment
		 *	variable was set or not
		 */
		if(len && iSize)
		{
			/*
			 *	Allocate buffer to hold data
			 */
			pszData = l_malloc(job, sizeof(char) * len);
			if(pszData)
			{
				len = GetEnvironmentVariableA(pszName, pszData, len);
				if(len)
				{
					/*
					 *	Convert to UTF8
					 */
					pszTemp = l_convertStringMBToUTF8(job, pszData, &len);
					if(pszTemp)
					{
						if(len > iSize)
							len = iSize - 1;
						strncpy(szBuffer, pszTemp, len);
						szBuffer[len] = '\0';

					}
					else
					{
						/*
						 *	Error converting from MB to UTF8.
						 */
						len = 0;
						memset(szBuffer, 0, iSize);
					}
				}
			}
		}
	}

	if(pszData)
	{
		l_free(pszData);
		pszData = NULL;
	}
	if(pszName)
	{
		l_free(pszName);
		pszName = NULL;
	}
	if(pszTemp)
	{
		free(pszTemp);
		pszTemp = NULL;
	}

	return len;
}

/****************************************************************************/
/**	@brief	Utility for getting environment variables, data is returned in
 *			UTF-8 format, read in wide character format.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szName		Name of environment variable, assume in UTF-8 format.
 *	@param	szBuffer	Buffer to hold environment data	in UTF-8 format.
 *	@param	iSize		Size, in bytes of szBuffer, if 0, szBuffer is ignored
 *						and the size of buffer required to store the value
 *						is computed.
 *
 *	@return	0 on error, else size of number of bytes copied or required.
 ****************************************************************************/
static
int
sFlexGetEnvVarW(
	LM_HANDLE *		job,
	char *			szName,
	char *			szBuffer,
	int				iSize)
{
	int			len = 0;
	wchar_t *	pwszData = NULL;
	wchar_t *	pwszName = NULL;
	char *		pszTemp = NULL;


	/*
	 *	Convert name to wide character
	 */
	pwszName = l_convertStringUTF8ToWC(job, szName, &len);
	if(pwszName)
	{
		/*
		 *	Determine size of buffer needed to hold wide
		 *	character version of data.
		 */
		len = GetEnvironmentVariableW(pwszName, NULL, 0);
		/*
		 *	Check to see if only interested if environment
		 *	variable was set or not
		 */
		if(len && iSize)
		{
			/*
			 *	Allocate wide character buffer to hold data
			 */
			pwszData = l_malloc(job, sizeof(wchar_t) * len);
			if(pwszData)
			{
				len = GetEnvironmentVariableW(pwszName, pwszData, len);
				if(len)
				{
					/*
					 *	Convert to UTF8
					 */
					pszTemp = l_convertStringWCToUTF8(job, pwszData, &len);
					if(pszTemp)
					{
						if(len > iSize)
							len = iSize - 1;
						strncpy(szBuffer, pszTemp, len);
						szBuffer[len] = '\0';

					}
					else
					{
						/*
						 *	Error converting from WC to UTF8.
						 */
						len = 0;
						memset(szBuffer, 0, iSize);
					}
				}
			}
		}
	}

	if(pwszData)
	{
		l_free(pwszData);
		pwszData = NULL;
	}
	if(pwszName)
	{
		l_free(pwszName);
		pwszName = NULL;
	}
	if(pszTemp)
	{
		free(pszTemp);
		pszTemp = NULL;
	}

	return len;
}

#endif /* WINNT */

/****************************************************************************/
/**	@brief	Utility for getting environment variables, data is returned in
 *			UTF-8 format.
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szName		Name of environment variable, assume in UTF-8 format.
 *	@param	szBuffer	Buffer to hold environment data	in UTF-8 format.
 *	@param	iSize		Size, in bytes of szBuffer, if 0, szBuffer is ignored
 *						and the size of buffer required to store the value
 *						is computed.
 *
 *	@return	0 on error, else size of number of bytes copied or required.
 ****************************************************************************/
int
l_getEnvironmentVariable(
	LM_HANDLE *		job,
	char *			szName,
	char *			szBuffer,
	int				iSize)
{
#ifdef WINNT
	int			len = 0;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		len = sFlexGetEnvVarA(job, szName, szBuffer, iSize);
	}
	else
	{
		len = sFlexGetEnvVarW(job, szName, szBuffer, iSize);
	}
	return len;
#else	/* !WINNT */
	int		len = 0;
	char *	pszData = NULL;

	if( (szName == NULL) || ( (szBuffer == NULL) &&  iSize) )
		goto done;

	pszData = getenv(szName);

	if(pszData)
	{
		len = strlen(pszData);

		/*
		 *	Want actually value, not just checking if set
		 */
		if(iSize)
		{
			if(len > iSize)
			{
				len = iSize - 1;
			}
			strncpy(szBuffer, pszData, len);
			szBuffer[len] = '\0';
		}
	}
done:
	return len;
#endif /* PC */
}



