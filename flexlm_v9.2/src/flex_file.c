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
/*	$Id: flex_file.c,v 1.12 2003/05/21 18:41:36 sluu Exp $	*/

/**	@file 		flex_file.c
 *	@brief		Wrapper function for file routines.
 *	@version	$Revision: 1.12 $
 *
 *	Wrapper functions for fopen/fclose, open/close.  Support UTF-8 on windows.
*****************************************************************************/

#include "flex_file.h"
#include "flex_utils.h"
#include <fcntl.h>
#include <stdlib.h>
#ifdef WINNT
#include <io.h>
#include <windows.h>
#endif /* WINNT */

#ifdef WINNT
/****************************************************************************/
/**	@brief	ANSI fopen, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	szMode		mode to open file in (UTF-8 format for windows)
 *
 *	@return	A valid FILE pointer, else NULL on error.
 ****************************************************************************/
static
FILE *
sFlexFopenA(
	LM_HANDLE *		job,
	const char *	szFilename,
	const char *	szMode)
{
	FILE *			fp = NULL;
	char *			pszFilename = NULL;
	char *			pszMode = NULL;
	int				size = 0;
	int				status = 0;

	if(szFilename && szMode)
	{
		/*
		 *	Convert filename and mode.
		 */
		pszFilename = l_convertStringUTF8ToMB(job, szFilename, &size);
		if(pszFilename && size)
		{
			pszMode = l_convertStringUTF8ToMB(job, szMode, &size);
			if(pszMode && size)
			{
				/*
				 *	Open file
				 */
				fp = fopen(pszFilename, pszMode);
			}
		}
	}
	/*
	 *	Cleanup
	 */
	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}
	if(pszMode)
	{
		l_free(pszMode);
		pszMode = NULL;
	}

	return fp;

}

/****************************************************************************/
/**	@brief	Wide character fopen, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	szMode		mode to open file in (UTF-8 format for windows)
 *
 *	@return	A valid FILE pointer, else NULL on error.
 ****************************************************************************/
static
FILE *
sFlexFopenW(
	LM_HANDLE *		job,
	const char *	szFilename,
	const char *	szMode)
{
	FILE *			fp = NULL;
	wchar_t *		pwszFilename = NULL;
	wchar_t	*		pwszMode = NULL;
	int				size = 0;
	int				status = 0;

	if(szFilename && szMode)
	{
		/*
		 *	Convert filename and mode.
		 */
		pwszFilename = l_convertStringUTF8ToWC(job, szFilename, &size);
		if(pwszFilename && size)
		{
			pwszMode = l_convertStringUTF8ToWC(job, szMode, &size);
			if(pwszMode && size)
			{
				/*
				 *	Open file
				 */
				fp = _wfopen(pwszFilename, pwszMode);
			}
		}
	}
	/*
	 *	Cleanup
	 */
	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}
	if(pwszMode)
	{
		l_free(pwszMode);
		pwszMode = NULL;
	}

	return fp;

}

/****************************************************************************/
/**	@brief	ANSI freopen(), supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	szMode		mode to open file in (UTF-8 format for windows)
 *	@param	pFile		Currently open file stream that will be closed.
 *
 *	@return	A valid FILE pointer, else NULL on error.
 ****************************************************************************/
static
FILE *
sFlexFreopenA(
	LM_HANDLE *		job,
	const char *	szFilename,
	const char *	szMode,
	FILE *			pFile)
{
	FILE *			fp = NULL;
	char *			pszFilename = NULL;
	char *			pszMode = NULL;
	int				size = 0;
	int				status = 0;

	if(szFilename && szMode)
	{
		/*
		 *	Convert filename and mode.
		 */
		pszFilename = l_convertStringUTF8ToMB(job, szFilename, &size);
		if(pszFilename && size)
		{
			pszMode = l_convertStringUTF8ToMB(job, szMode, &size);
			if(pszMode && size)
			{
				/*
				 *	Open file
				 */
				fp = freopen(pszFilename, pszMode, pFile);
			}
		}
	}
	/*
	 *	Cleanup
	 */
	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}
	if(pszMode)
	{
		l_free(pszMode);
		pszMode = NULL;
	}

	return fp;

}

/****************************************************************************/
/**	@brief	Wide character freopen(), supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	szMode		mode to open file in (UTF-8 format for windows)
 *	@param	pFile		Currently open file stream that will be closed.
 *
 *	@return	A valid FILE pointer, else NULL on error.
 ****************************************************************************/
static
FILE *
sFlexFreopenW(
	LM_HANDLE *		job,
	const char *	szFilename,
	const char *	szMode,
	FILE *			pFile)
{
	FILE *			fp = NULL;
	wchar_t *		pwszFilename = NULL;
	wchar_t	*		pwszMode = NULL;
	int				size = 0;
	int				status = 0;

	if(szFilename && szMode)
	{
		/*
		 *	Convert filename and mode.
		 */
		pwszFilename = l_convertStringUTF8ToWC(job, szFilename, &size);
		if(pwszFilename && size)
		{
			pwszMode = l_convertStringUTF8ToWC(job, szMode, &size);
			if(pwszMode && size)
			{
				/*
				 *	Open file
				 */
				fp = _wfreopen(pwszFilename, pwszMode, pFile);
			}
		}
	}
	/*
	 *	Cleanup
	 */
	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}
	if(pwszMode)
	{
		l_free(pwszMode);
		pwszMode = NULL;
	}

	return fp;

}


/****************************************************************************/
/**	@brief	ANSI open, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	iOpenFlag	open flags, refer to fcntl.h for valid values for open
 *	@param	iPermission	permission flags, refer to sys/stat.h for valid values
 *
 *	@return	A file handle, else -1 on error.
 ****************************************************************************/
static
int
sFlexOpenA(
	LM_HANDLE *		job,
	const char *	szFilename,
	int				iOpenFlag,
	int				iPermission)
{
	char *		pszFilename = NULL;
	int			size = 0;
	int			fd = -1;

	/*
	 *	Convert filename to multi-byte string.
	 */
	pszFilename = l_convertStringUTF8ToMB(job, szFilename, &size);
	if(pszFilename && size)
	{
		fd = open(pszFilename, iOpenFlag, iPermission);
	}

	/*
	 *	Cleanup
	 */
	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}
	return fd;

}

/****************************************************************************/
/**	@brief	UNICODE open, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	iOpenFlag	open flags, refer to fcntl.h for valid values for open
 *	@param	iPermission	permission flags, refer to sys/stat.h for valid values
 *
 *	@return	A file handle, else -1 on error.
 ****************************************************************************/
static
int
sFlexOpenW(
	LM_HANDLE *		job,
	const char *	szFilename,
	int				iOpenFlag,
	int				iPermission)
{
	wchar_t *	pwszFilename = NULL;
	int			size = 0;
	int			fd = -1;

	/*
	 *	Convert filename to Wide Character string.
	 */
	pwszFilename = l_convertStringUTF8ToWC(job, szFilename, &size);
	if(pwszFilename && size)
	{
		fd = _wopen(pwszFilename, iOpenFlag, iPermission);
	}

	/*
	 *	Cleanup
	 */
	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}
	return fd;

}

/****************************************************************************/
/**	@brief	ANSI stat(), accepts data in UTF-8 format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	name of file to check
 *	@param	pStatBuffer	pointer to a struct stat that will receive results
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexStatA(
	LM_HANDLE *		job,
	char *			szFilename,
	struct stat *	pStatBuffer)
{
	char *		pszFilename = NULL;
	int			size = 0;
	int			ret = -1;
	/*
	 *	Convert filename from UTF-8 to MB
	 */
	if( (szFilename == NULL) || (pStatBuffer == NULL) )
		goto done;

	pszFilename = 	l_convertStringUTF8ToMB(job, szFilename, &size);

	if(pszFilename && size)
	{
		ret = stat(pszFilename, (struct stat *)pStatBuffer);
	}

done:

	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}

	return ret;

}

/****************************************************************************/
/**	@brief	UNICODE stat(), accepts data in UTF-8 format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	name of file to check
 *	@param	pStatBuffer	pointer to a struct stat that will receive results
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexStatW(
	LM_HANDLE *		job,
	char *			szFilename,
	struct stat *	pStatBuffer)
{
	wchar_t	*	pwszFilename = NULL;
	int			size = 0;
	int			ret = -1;
	/*
	 *	Convert filename from UTF-8 to WC, then do a wstat()
	 */
	if( (szFilename == NULL) || (pStatBuffer == NULL) )
		goto done;

	pwszFilename = 	l_convertStringUTF8ToWC(job, szFilename, &size);

	if(pwszFilename && size)
	{
		ret = _wstat(pwszFilename, (struct _stat *)pStatBuffer);
	}

done:

	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}

	return ret;

}

/****************************************************************************/
/**	@brief	ANSI rename(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFromFile	Name of file to change
 *	@param	szToFile	New name for file
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexRenameA(
	LM_HANDLE *		job,
	char *			szFromFile,
	char *			szToFile)
{
	char *		pszFromFile = NULL;
	char *		pszToFile = NULL;
	int			size = 0;
	int			ret = -1;

	if( (szFromFile == NULL) || (szToFile == NULL) )
		goto done;

	pszFromFile = l_convertStringUTF8ToMB(job, szFromFile, &size);
	if(pszFromFile && size)
	{
		size = 0;
		pszToFile = l_convertStringUTF8ToMB(job, szToFile, &size);
		if(pszToFile && size)
		{
			ret = rename(pszFromFile, pszToFile);
		}
	}

done:
	if(pszFromFile)
	{
		l_free(pszFromFile);
		pszFromFile = NULL;
	}
	if(pszToFile)
	{
		l_free(pszToFile);
		pszToFile = NULL;
	}
	return ret;

}

/****************************************************************************/
/**	@brief	UNICODE rename(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFromFile	Name of file to change
 *	@param	szToFile	New name for file
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexRenameW(
	LM_HANDLE *		job,
	char *			szFromFile,
	char *			szToFile)
{
	wchar_t	*		pwszFromFile = NULL;
	wchar_t	*		pwszToFile = NULL;
	int				size = 0;
	int				ret = -1;

	if( (szFromFile == NULL) || (szToFile == NULL) )
		goto done;

	pwszFromFile = l_convertStringUTF8ToWC(job, szFromFile, &size);
	if(pwszFromFile && size)
	{
		size = 0;
		pwszToFile = l_convertStringUTF8ToWC(job, szToFile, &size);
		if(pwszToFile && size)
		{
			ret = _wrename(pwszFromFile, pwszToFile);
		}
	}

done:
	if(pwszFromFile)
	{
		l_free(pwszFromFile);
		pwszFromFile = NULL;
	}
	if(pwszToFile)
	{
		l_free(pwszToFile);
		pwszToFile = NULL;
	}
	return ret;

}

/****************************************************************************/
/**	@brief	ANSI access(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to check
 *	@param	iMode		Mode to check
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexAccessA(
	LM_HANDLE *		job,
	char *			szFilename,
	int				iMode)
{
	char *	pszFilename = NULL;
	int		size = 0;
	int		ret = -1;

	if(szFilename == NULL)
		goto done;
	
	/*
	 *	Convert filename to MB
	 */
	pszFilename = l_convertStringUTF8ToMB(job, szFilename, &size);
	if(pszFilename && size)
	{
		ret = access(pszFilename, iMode);
	}
done:
	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}
	return ret;

}

/****************************************************************************/
/**	@brief	UNICODE access(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to check
 *	@param	iMode		Mode to check
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexAccessW(
	LM_HANDLE *		job,
	char *			szFilename,
	int				iMode)
{
	wchar_t *	pwszFilename = NULL;
	int			size = 0;
	int			ret = -1;

	if(szFilename == NULL)
		goto done;
	
	/*
	 *	Convert filename to WC
	 */
	pwszFilename = l_convertStringUTF8ToWC(job, szFilename, &size);
	if(pwszFilename && size)
	{
		ret = _waccess(pwszFilename, iMode);
	}
done:
	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}
	return ret;

}

/****************************************************************************/
/**	@brief	ANSI remove(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to remove
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexRemoveA(
	LM_HANDLE *		job,
	char *			szFilename)
{
	char *	pszFilename = NULL;
	int		size = 0;
	int		ret = -1;
	
	if(szFilename == NULL)
		goto done;

	pszFilename = l_convertStringUTF8ToMB(job, szFilename, &size);
	if(pszFilename && size)
	{
		ret = remove(pszFilename);
	}

done:
	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}
	return ret;

}


/****************************************************************************/
/**	@brief	UNICODE remove(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to remove
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexRemoveW(
	LM_HANDLE *		job,
	char *			szFilename)
{
	wchar_t *	pwszFilename = NULL;
	int			size = 0;
	int			ret = -1;
	
	if(szFilename == NULL)
		goto done;

	pwszFilename = l_convertStringUTF8ToWC(job, szFilename, &size);
	if(pwszFilename && size)
	{
		ret = _wremove(pwszFilename);
	}

done:
	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}
	return ret;

}


/****************************************************************************/
/**	@brief	ANSI unlink(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to unlink
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexUnlinkA(
	LM_HANDLE *		job,
	char *			szFilename)
{
	char *	pszFilename = NULL;
	int		size = 0;
	int		ret = -1;
	
	if(szFilename == NULL)
		goto done;

	pszFilename = l_convertStringUTF8ToMB(job, szFilename, &size);
	if(pszFilename && size)
	{
		ret = unlink(pszFilename);
	}

done:
	if(pszFilename)
	{
		l_free(pszFilename);
		pszFilename = NULL;
	}
	return ret;

}


/****************************************************************************/
/**	@brief	UNICODE unlink(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to unlink
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
static
int
sFlexUnlinkW(
	LM_HANDLE *		job,
	char *			szFilename)
{
	wchar_t *	pwszFilename = NULL;
	int			size = 0;
	int			ret = -1;
	
	if(szFilename == NULL)
		goto done;

	pwszFilename = l_convertStringUTF8ToWC(job, szFilename, &size);
	if(pwszFilename && size)
	{
		ret = _wunlink(pwszFilename);
	}

done:
	if(pwszFilename)
	{
		l_free(pwszFilename);
		pwszFilename = NULL;
	}
	return ret;

}
#endif /* WINNT */




/****************************************************************************/
/**	@brief	Wrapper function for fopen, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	szMode		mode to open file in (UTF-8 format for windows)
 *
 *	@return	A valid FILE pointer, else NULL on error.
 ****************************************************************************/
FILE *
l_flexFopen(
	LM_HANDLE *		job,
	const char *	szFilename,		/* ASSUME UTF-8 for windows */
	const char *	szMode)
{
#ifdef WINNT
	FILE *	fp = NULL;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		fp = sFlexFopenA(job, szFilename, szMode);
	}
	else
	{
		fp = sFlexFopenW(job, szFilename, szMode);
	}
	return fp;
#else /* !WINNT */
	return fopen(szFilename, szMode);
#endif /* WINNT */
}



/****************************************************************************/
/**	@brief	Wrapper function for freopen, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle.
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	szMode		mode to open file in (UTF-8 format for windows)
 *	@param	pFile		Currently open file stream that will be closed.
 *
 *	@return	A valid FILE pointer, else NULL on error.
 ****************************************************************************/
FILE *
l_flexFreopen(
	LM_HANDLE *		job,
	const char *	szFilename,		/* ASSUME UTF-8 for windows */
	const char *	szMode,
	FILE *			pFile)
{
#ifdef WINNT
	FILE *			fp = NULL;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		fp = sFlexFreopenA(job, szFilename, szMode, pFile);
	}
	else
	{
		fp = sFlexFreopenW(job, szFilename, szMode, pFile);
	}
	return fp;
#else /* !WINNT */
	return freopen(szFilename, szMode, pFile);
#endif /* WINNT */
}

/****************************************************************************/
/**	@brief	Wrapper function for fclose, for consistency....
 *
 *	@param	fp		FILE pointer to close
 *
 *	@return	Non zero on error, esle 0.
 ****************************************************************************/
int
l_flexFclose(FILE * fp)
{
	return fclose(fp);
}


/****************************************************************************/
/**	@brief	Wrapper function for open, supports UTF-8 on windows
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	name of file to open (UTF-8 format for windows)
 *	@param	iOpenFlag	open flags, refer to fcntl.h for valid values for open
 *	@param	iPermission	permission flags, refer to sys/stat.h for valid values
 *
 *	@return	A file handle, else -1 on error.
 ****************************************************************************/
int
l_flexOpen(
	LM_HANDLE *		job,
	const char *	szFilename,
	int				iOpenFlag,
	int				iPermission)
{
#ifdef WINNT
	int			fd = -1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		fd = sFlexOpenA(job, szFilename, iOpenFlag, iPermission);
	}
	else
	{
		fd = sFlexOpenW(job, szFilename, iOpenFlag, iPermission);
	}
	return fd;
#else /* !WINNT */
	return open(szFilename, iOpenFlag, iPermission);
#endif /* WINNT */
}

/****************************************************************************/
/**	@brief	Wrapper function for close, for consistency
 *
 *	@param	fd			handle to file to close
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
int
l_flexClose(int fd)
{
#if WINNT
	return _close(fd);
#else /* !WINNT */
	return close(fd);
#endif /* WINNT */
}



/****************************************************************************/
/**	@brief	Wrapper function for stat(), accepts data in UTF-8 format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	name of file to check
 *	@param	pStatBuffer	pointer to a struct stat that will receive results
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
int
l_flexStat(
	LM_HANDLE *		job,
	char *			szFilename,
	struct stat *	pStatBuffer)
{
#ifdef WINNT
	int			ret = -1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		ret = sFlexStatA(job, szFilename, pStatBuffer);
	}
	else
	{
		ret = sFlexStatW(job, szFilename, pStatBuffer);
	}

	return ret;
#else /* !WINNT */
	return stat(szFilename, pStatBuffer);
#endif /* WINNT */
}



/****************************************************************************/
/**	@brief	Wrapper function for rename(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFromFile	Name of file to change
 *	@param	szToFile	New name for file
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
int
l_flexRename(
	LM_HANDLE *		job,
	char *			szFromFile,
	char *			szToFile)
{
#ifdef WINNT
	int				ret = -1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		ret = sFlexRenameA(job, szFromFile, szToFile);
	}
	else
	{
		ret = sFlexRenameW(job, szFromFile, szToFile);
	}

	return ret;
#else /* !WINNT */
	return rename(szFromFile, szToFile);
#endif
}



/****************************************************************************/
/**	@brief	Wrapper function for access(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to check
 *	@param	iMode		Mode to check
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
int
l_flexAccess(
	LM_HANDLE *		job,
	char *			szFilename,
	int				iMode)
{
#ifdef WINNT
	int			ret = -1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		ret = sFlexAccessA(job, szFilename, iMode);
	}
	else
	{
		ret = sFlexAccessW(job, szFilename, iMode);
	}

	return ret;
#else /* !WINNT */
	return access(szFilename, iMode);
#endif
}



/****************************************************************************/
/**	@brief	Wrapper function for remove(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to remove
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
int
l_flexRemove(
	LM_HANDLE *		job,
	char *			szFilename)
{
#ifdef WINNT
	int			ret = -1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		ret = sFlexRemoveA(job, szFilename);
	}
	else
	{
		ret = sFlexRemoveW(job, szFilename);
	}

	return ret;
#else /* !WINNT */
	return remove(szFilename);
#endif
}



/****************************************************************************/
/**	@brief	Wrapper function for unlink(), accepts data in UTF-8
 *			format
 *
 *	@param	job			FLEXlm job handle
 *	@param	szFilename	Name of file to unlink
 *
 *	@return	0 on success, else -1 on error.
 ****************************************************************************/
int
l_flexUnlink(
	LM_HANDLE *		job,
	char *			szFilename)
{
#ifdef WINNT
	int			ret = -1;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		ret = sFlexUnlinkA(job, szFilename);
	}
	else
	{
		ret = sFlexUnlinkW(job, szFilename);
	}

	return ret;
#else /* !WINNT */
	return unlink(szFilename);
#endif
}

