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
/* $Id: lm_objfilewinpe.cpp,v 1.1 2003/04/30 23:57:11 kmaclean Exp $
***********************************************************/
/**   @file readpe.c
 *    @brief Read a Microsoft Portable Executable (PE) file. A .exe by any other name.
 *    @version $Revision: 1.1 $
 *
  ************************************************************/

#include "lm_objfilewinpe.h"

int PEFile::fileOpen(void)
{
	int val;

	val = CodeFile::fileOpen();
	if ( val  )
	{
		m_coffFile.setFd(CodeFile::getFd());
	}
	return val;
}	

/***********************************************************************/
/** @brief Read the PE file header and the coff header
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int PEFile::hRead(void)
{
	if (! m_dosHeader.hRead(m_fd))
		return 0;

	if ( (unsigned long)lseek(m_fd,  getCoffHeaderOffset(), SEEK_SET) !=  getCoffHeaderOffset() )
	{
		return 0;
	}
	return m_coffFile.hRead();
}	
/***********************************************************************/
/** @brief Print the PE file info
 *
 ************************************************************************/
void PEFile::print(void) const
{
	fprintf(stdout,"PE file\n");
}
/***********************************************************************/
/** @brief  get the next string from the symbol table
 *
 * @param
 *
 * @return
 *
 ************************************************************************/	  
SymbolString * PEFile::getFirstString(void)
{
	return m_coffFile.getFirstString();
}

/***********************************************************************/
/** @brief Read rhe PE file header from the file
 *
 * @param fd the file descriptor
 *
 * @return
 *
 ************************************************************************/
int PEHeader::hRead(const int fd)
{
	/* read the dos header */
	if (read(fd,&m_data, sizeof(IMAGE_DOS_HEADER)) != sizeof(IMAGE_DOS_HEADER)  )
	{
		perror ("Read DOS Header");
		return 0;
	}
	/* step over the MS DOS stub */
	if ( m_data.e_lfanew )
	{
		if (lseek(fd, m_data.e_lfanew, SEEK_SET) != m_data.e_lfanew)
		{
			debugPrint();
			perror ("Lseek e_lfanew");
			return 0;
		}
	}
	/* read the PE file signature */
	if (read(fd,&m_signature, sizeof(DWORD)) != sizeof(DWORD))
	{
		debugPrint();
		perror ("Read PE signature ");
		return 0;
	}
	debugPrint();
	return 1;
}	

/***********************************************************************/
/** @brief Print the PE header for debugging
 *
 ************************************************************************/
void PEHeader::print(void) const
{
	fprintf(stdout, "IMAGE_DOS_HEADER\n");
	fprintf(stdout, "  e_magic       : 0x%x\n",(unsigned int)m_data.e_magic);
	fprintf(stdout, "  e_lfanew      : 0x%x\n",(unsigned int)m_data.e_lfanew);

	fprintf(stdout, "Signature: 0x%x\n", m_signature);
}	
