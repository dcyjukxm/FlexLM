/******************************************************************************

            COPYRIGHT (c) 1997, 2003  by Macrovision Corporation.
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
 *      
 *	Module: dir_pc.c v1.1      
 *      Description:  PC version of opendir and readdir
 *
 *      C. Mayman
 *      8-Apr-97
 *
 *      
 *
 */

#include "lmachdep.h"
#include "flex_utils.h"
#include <windows.h>
#include <sys\stat.h>
#include "dir_pc.h"

/*
// Constants
*/

#define FIRST_READ (0xffff)
#define NO_MORE (FIRST_READ - 1)

/*
// Types
*/

typedef void *FileData;

/*
// Forward definitions
*/

static FileData *allocFileData();
static char *getFileName( FileData *fData );
static int doFindFirst( char* expression, FileData *fData );
static int doFindNext( FileData *fData );
static void doFindClose( FileData *fData );
static void freeFindData( FileData **fData );

DIR *
opendir_PC(
	PCDirInfo *	infoPtr,
	char *		dirname )
{
	FileData *	fDataPtr = NULL;
	char		fullPath[MAX_PATH+1] = {'\0'};
	int			intRet = 0;

	/*
	 *	Return on boundary errors.
	 */
	if( (infoPtr == NULL) || (dirname == NULL) || !(*dirname) )
		return NULL;

	/*
	 *	Initialize the input PCDirInfo structure to be invalid.
	 */
	memset( infoPtr, 0, sizeof(PCDirInfo) );
	infoPtr->directoryEntry.d_namlen = FIRST_READ;
	fDataPtr = allocFileData();
	if( !fDataPtr )
		return NULL;
	infoPtr->fDataPtr = fDataPtr;

	/*
	 *	Find the first file, this essentially "opens" the directory
	 *	in the PC style API's.
	 */
	strcpy( fullPath, dirname );
	if('\\' != fullPath[strlen(fullPath)-1])
		strcat( fullPath, "\\" );
#ifdef PC16
	strcat( fullPath, "*.*" );
#else
	strcat( fullPath, "*" );
#endif
	intRet = doFindFirst( fullPath, infoPtr->fDataPtr );
	if( -1 == intRet )
	{
		freeFindData( &(infoPtr->fDataPtr) );
		return NULL;
	}

	/*
	 *	Return the pointer to the callers data because
	 *	the first parameter is hidden from the caller
	 *	in the macro.
	 */
	return infoPtr;
}

struct dirent *
readdir_PC(
	PCDirInfo *	infoPtr,
	DIR *		dirp)
{
	int				intRet = 0;
	struct dirent *	dPtr = NULL;
	struct dirent *	retVal = NULL;

	/*
	 *	Exit on parameter anomoly.
	 */
	if( (infoPtr == NULL) || (infoPtr != dirp) || !(infoPtr->fDataPtr) )
		return NULL;

	/*
	 *	If the encapsulated "dirent" has a length of FIRST_READ,
	 *	then this is the first call to readdir, and we
	 *	should use the value currrently stored.
	 *	If the value is NO_MORE, then there are no more entries, and
	 *	we should return NULL.
	 */
	
	dPtr = &(infoPtr->directoryEntry);

	if( FIRST_READ == dPtr->d_namlen )
	{
		char *filename = getFileName( infoPtr->fDataPtr );
		if( NULL == filename )
		{
			dPtr->d_namlen = NO_MORE;
			return NULL;
		}

		strcpy( dPtr->d_name, filename );
		dPtr->d_namlen = strlen( filename );
		retVal = dPtr;
	}
	else if( NO_MORE == dPtr->d_namlen )
	{
		retVal = NULL;
	}
	else
	{
		intRet = doFindNext( infoPtr->fDataPtr );

		if( -1 == intRet )
		{
			/*
			 *	There are no more files.
			 */
			dPtr->d_namlen = NO_MORE;
			dPtr->d_name[0] = '\0';
			retVal = NULL;
		}
		else
		{
			/*
			 *	We got another one, so copy it into the
			 *	"dirent" structure.
			 */
			char *filename = getFileName( infoPtr->fDataPtr );
			if( NULL == filename )
			{
				dPtr->d_namlen = NO_MORE;
				return NULL;
			}

			strcpy( dPtr->d_name, filename );
			dPtr->d_namlen = strlen( filename );
			retVal = dPtr;
		}
	}

	return retVal;
}

int
closedir_PC(
	PCDirInfo *	infoPtr,
	DIR *		dirp)
{
	/*
	 *	Exit on parameter anomoly.
	 */
	if( (infoPtr == NULL) || !(infoPtr->fDataPtr) || (infoPtr != dirp) )
		return -1;

	doFindClose( infoPtr->fDataPtr );
	freeFindData( &(infoPtr->fDataPtr) );

	return 0;
}


//*************************************************************
//
// directory browsing routines for Windows NT (Win32)
//
//*************************************************************

#ifdef WINNT

typedef struct _Win32FindData
{
	union
	{
		WIN32_FIND_DATAW wide;
		WIN32_FIND_DATAA ansi;
	} fileData;
	/*
	 *	WIN32_FIND_DATAW.cFileName is MAX_PATH in size
	 */
	char szUTF8Filename[MAX_PATH * 3];	/* 3 bytes for each Wide Char */
	HANDLE hDirectory;
} Win32FindData;

BOOL
L_S_ISDIR(int i)
{
	return (S_IFDIR &i);
}

static
FileData *
allocFileData()
{
	Win32FindData *	dPtr = NULL;

	dPtr = (Win32FindData*) malloc( sizeof(Win32FindData) );

	if(dPtr)
		dPtr->hDirectory = INVALID_HANDLE_VALUE;

	return (FileData*) dPtr;
}

static
char *
getFileName(FileData *fData)
{
	Win32FindData *dPtr = (Win32FindData*) fData;

	if( !dPtr || (INVALID_HANDLE_VALUE == dPtr->hDirectory) )
		return NULL;

	return dPtr->szUTF8Filename;
}

/****************************************************************************/
/**	@brief	Utility for converting data from Wide Character to UTF-8
 *
 *	@param	pFileData	Pointer to a Win32FindData struct, which contains data
 *
 *	@return	Number of bytes converted to UTF-8, data is stored in
 *			((Win32FindData(pFileData))->szUTF8Filename.  0 on error.
 ****************************************************************************/
static
int
sConvertToUTF8(
	FileData *		pFileData)
{
	int				size = 0;
	char *			pszUTF8Result = NULL;
	Win32FindData *	pData = (Win32FindData *)pFileData;


	if( (pFileData == NULL) || (INVALID_HANDLE_VALUE == pData->hDirectory) )
		goto done;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		pszUTF8Result = l_convertStringMBToUTF8(NULL,
							pData->fileData.ansi.cFileName, &size);
	}
	else
	{
		pszUTF8Result = l_convertStringWCToUTF8(NULL,
							pData->fileData.wide.cFileName, &size);

	}

	if(pszUTF8Result)
	{
		if(size > sizeof(pData->szUTF8Filename))
		{
			/*
			 *	Truncate
			 */
			size = sizeof(pData->szUTF8Filename) - 1;
			/*
			 *	Save space for NULL
			 */
			strncpy(pData->szUTF8Filename, pszUTF8Result, size);
			/*
			 *	NULL terminate
			 */
			pData->szUTF8Filename[size] = '\0';
		}
		else
		{
			strncpy(pData->szUTF8Filename, pszUTF8Result, size);
		}
	}
	else
	{
		size = 0;
		pData->szUTF8Filename[0] = '\0';
	}


done:

	if(pszUTF8Result)
	{
		l_free(pszUTF8Result);
		pszUTF8Result = NULL;
	}

	return size;
}

static
int
doFindFirst(
	char *		expression,
	FileData *	fData)
{
	Win32FindData *	dPtr = (Win32FindData*) fData;
	int				retVal = -1;
	int				size = 0;
	wchar_t *		pwszExpression = NULL;
	char *			pszExpression = NULL;
	int				iOSFamily = l_getOSFamily();

    if(dPtr == NULL)
		goto done;

	if(iOSFamily == OS_95_98_ME)
	{
		/*
		 *	Convert to multi-byte first.
		 */
		pszExpression = l_convertStringUTF8ToMB(NULL, expression, &size);
		if(pszExpression)
		{
			dPtr->hDirectory = FindFirstFileA(pszExpression, &(dPtr->fileData.ansi));
		}
		else
		{
			dPtr->hDirectory = INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		/*
		 *	Convert to Wide character, then call FindFirstFileW instead
		 *	of FindFirstFile, which because we don't have UNICODE defined
		 *	is mapped to FindFirstFileA.
		 */
		pwszExpression = l_convertStringUTF8ToWC(NULL, expression, &size);
		if(pwszExpression)
		{
			dPtr->hDirectory = FindFirstFileW(pwszExpression, &(dPtr->fileData.wide));
		}
		else
		{
			dPtr->hDirectory = INVALID_HANDLE_VALUE;
		}
	}

	if( INVALID_HANDLE_VALUE != dPtr->hDirectory )
	{
		/*
		 *	Convert to UTF8
		 */
		size = sConvertToUTF8(fData);
		if(size)
			retVal = 0;
	}
done:
	if(pwszExpression)
	{
		l_free(pwszExpression);
		pwszExpression = NULL;
	}
	if(pszExpression)
	{
		l_free(pszExpression);
		pszExpression = NULL;
	}
	return retVal;
}

static
int
doFindNext(FileData *fData)
{
	Win32FindData *	dPtr = (Win32FindData*) fData;
	int				retVal = -1;
	int				size = 0;
	int				status = 0;

	if( (dPtr == NULL) || (INVALID_HANDLE_VALUE == dPtr->hDirectory) )
		goto done;

	if(l_getOSFamily() == OS_95_98_ME)
	{
		status = FindNextFileA(dPtr->hDirectory, &(dPtr->fileData.ansi));
	}
	else
	{
		status = FindNextFileW(dPtr->hDirectory, &(dPtr->fileData.wide));
	}
	if(status)
	{
		/*
		 *	Convert data to UTF-8
		 */
		size = sConvertToUTF8(fData);
		if(size)
			retVal = 0;
	}

done:

	return retVal;
}

static
void
doFindClose(FileData *fData)
{
	Win32FindData *dPtr = (Win32FindData*) fData;

	if( (dPtr == NULL) || (INVALID_HANDLE_VALUE == dPtr->hDirectory) )
		return;

	FindClose(dPtr->hDirectory);

	dPtr->hDirectory = INVALID_HANDLE_VALUE;
}

static
void
freeFindData(FileData **fData)
{
	Win32FindData *	dPtr = NULL;

	if(fData == NULL)
		return;

	dPtr = (Win32FindData*)(*fData);

	if(dPtr == NULL)
		return;

	/*
	 *	Close the directory if it's still open.
	 */

	if( INVALID_HANDLE_VALUE != dPtr->hDirectory )
	{
		FindClose(dPtr->hDirectory);
		dPtr->hDirectory = INVALID_HANDLE_VALUE;
	}

	l_free(dPtr);
	*fData = NULL;
}

#endif /* WINNT */


