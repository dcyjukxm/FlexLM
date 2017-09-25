/************************************************************
 	Copyright (c) 2003 by Macrovision, Corporation.
 	This software has been provided pursuant to a License Agreement
 	containing restrictions on its use.  This software contains
 	valuable trade secrets and proprietary information of
 	Macrovision Corporation and is protected by law.  It may
 	not be copied or distributed in any form or medium, disclosed
 	to third parties, reverse engineered or used in any manner not
 	provided for in said License Agreement except with the prior
 	written authorization from Macrovision Corporation.
 ***********************************************************/
/* $Id: lm_objfilecommon.cpp,v 1.2 2003/05/14 17:32:04 kmaclean Exp $
***********************************************************/
/**   @file lm_objfilecommon.cpp
 *    @brief  common routines for reading object files
 *    @version $Revision: 1.2 $     
 *
  ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lm_objfilecoff.h"
#include "lm_objfilewinpe.h"
#include "lm_objfilelib.h"

/***********************************************************************/
/** @brief Open a file and read the header.
 *
 * @param name  name of file
 *
 * @return the fd returned by open()
 ************************************************************************/
int CodeFile::fileOpen(void)		  
{
	
	if ( ( m_fd = open(m_fileName, MyOpenFlags)) < 0)
	{
		perror(m_fileName);
		return 0;
	}

	return m_fd;
}	
/***********************************************************************/
/** @brief close the file
 *
 ************************************************************************/
void CodeFile::fileClose(void) 
{
	if ( m_fd >= 0 )
	{
		close(m_fd);
		m_fd = -1;
	}
}	
/***********************************************************************/
/** @brief Get a CodeFile object of the correct type.
 *
 * @param fileName  name of file to open
 * @param progName name of program for usage messages.
 *
 * @return on sucecss a pointer to a new CodeFile object
 *
 ************************************************************************/
CodeFile * CodeFile::getFileObject(int debugFlag, const char * fileName, const char * progName)	  
{
	CodeFile  * fileThing = NULL;
	if ( fileName == NULL || strlen(fileName) == 0 )
	{
		fprintf(stderr,"No file name specified\n");
		return 0;
	}
	/* what type of file is it. Based on the extension */
	if ( strstr(fileName, ".exe" ) != NULL)
	{
		fileThing = new PEFile(debugFlag, fileName);
	}
	else if ( strstr(fileName, ".obj" ) != NULL)
	{
		fileThing = new CoffFile(debugFlag, fileName);
	}	
	else if ( strstr(fileName, ".lib" ) != NULL)
	{
		fileThing = new LibFile(debugFlag, fileName);
	}	
	else
	{
		fprintf(stderr,"%s: Unknown file type\n",fileName);
	}	
	return fileThing;
}
/***********************************************************************/
/** @brief Constructor for CodeFile::
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
CodeFile::CodeFile(int debugFlag, const char *fileName) : 
	m_fd (-1), 
	m_debug(debugFlag) 
{
	if ( fileName )
		strncpy(m_fileName, fileName, MaxFileNameLen);
	else
		m_fileName[0] = '\0';
};

/***********************************************************************/
/** @brief Constructo to call if the file is already open
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
CodeFile::CodeFile(int debugFlag, int fd, const char *fileName) : 
	m_fd (fd), 
	m_debug(debugFlag) 
{
	if ( fileName )
		strncpy(m_fileName, fileName, MaxFileNameLen);
	else
		m_fileName[0] = '\0';
};

/***********************************************************************/
/** @brief Change the saved symbol string
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
void SymbolString::m_setReplacementString(const char * newString, int maxLen)
{
	int newLen;
	int oldLen;
	
	if ( m_debug & DebugReplace)
		fprintf(stdout,"CoffSymbolString::replaceString() %-20s  with %s\n",m_currentSymbolName, newString);

	if ( newString == NULL)
		newLen = 0;
	else
		newLen = strlen(newString);
	oldLen = strlen(m_currentSymbolName);
	if ( newLen == 0 )
	{
		/* empty string.
		 * This means that we are settig the old string to zeros */
		memset (m_currentSymbolName, 0, oldLen + 1);
	}
	else if ( oldLen <= newLen )
	{
		/* the new string is the same or longer than the old string. */
		strncpy(m_currentSymbolName, newString,min(oldLen,maxLen));
		/* don't null terminate */
	}
	else
	{
		/* usually here because the symbol name has a leading underscore and the
		 * new string does not. */
		if ( newLen == 0 )
		{
			/* we are clearing the entry */
			memset(m_currentSymbolName, 0, min(oldLen,maxLen));
		}
		else
		{
			/* only change the last chars in the string */
			int diff = oldLen - newLen;
			strncpy(&(m_currentSymbolName[min(diff,maxLen-1)]), newString,newLen);
			/* don't null terminate */
		}
	}	
}	
/***********************************************************************/
/** @brief Change the saved symbol string
 *
 * @param
 ************************************************************************/
void SymbolString::m_setString(const char * newString, int maxLen)
{
	int oldLen = strlen(m_currentSymbolName);
	if ( newString == NULL || *newString == '\0' )
	{
		/* empty string.
		 * This means that we are settig the old string to zeros */
		memset (m_currentSymbolName, 0, oldLen + 1);
	}
	else
	{
		/* the new string is the same or longer than the old string. */
		strncpy(m_currentSymbolName, newString,maxLen);
		m_currentSymbolName[maxLen] = '\0';
	}
}	
/***********************************************************************/
/** @brief write the new symbol string to the file
 *
 * @param
 *
 * @return
 *
 ************************************************************************/																   
int SymbolString::m_writeNewStringToFile(int fd, unsigned long offset, int len) const 
{
	/* seek to the proper place in the file and write the new string */
	if ((unsigned long)lseek(fd, offset, SEEK_SET) != offset)
		return 0;
	if (write(fd, m_currentSymbolName, len ) != len)
		return 0;
	return 1;
}
