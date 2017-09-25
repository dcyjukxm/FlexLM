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
/* $Id: lm_objfilewinpe.h,v 1.1 2003/04/30 23:58:36 kmaclean Exp $
***********************************************************/
/**   @file win_pefile.h
 *    @brief classes for reading a windows PE file
 *    @version $Revision: 1.1 $
 *
  ************************************************************/

#ifndef INCLUDE_WIN_PEFILE_H
#define  INCLUDE_WIN_PEFILE_H 

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <windows.h>
#include <winnt.h>
//#include "lmclient.h"
#include "lm_objfilecommon.h"
#include "lm_objfilecoff.h"


/*************************************************************/
/** Windows PE File MS-DOS header
 **************************************************************/
class PEHeader {
	
public:	
	PEHeader() : m_debug(0){ };

	PEHeader(int debugFlag) : m_debug(debugFlag) {};
	~PEHeader() {};	

	int hRead(const int fd);	/** read the header */
	void print(void) 				const;			/** print the header for debugging     */
	void debugPrint(void) 			const { if (m_debug) print() ; };
	unsigned long getPEHeaderOffset(void) 	const { return m_data.e_lfanew ;} ;

private:
	unsigned long 			m_fileOffset;  /** the file offset we read the data from */
	IMAGE_DOS_HEADER		m_data;    /** the header */
	DWORD					m_signature;    /** pe file signature */
	int						m_debug;	/** debug flag */
} ;	

/*************************************************************/
/**  Main class to read a PEFile 
 **************************************************************/
class PEFile : public CodeFile
{
public:	
	PEFile(int debugFlag, const char * fileName) : 
		CodeFile(debugFlag, fileName),
		m_dosHeader(debugFlag), 
		m_coffFile(debugFlag,fileName)
	{
	};
	
		/** read the important parts of the file */
	int hRead(void);
		/** Get the file offset to the COFF headder in the PE file.
		 * + sizeof(long) steps past the file signature */
	int fileOpen(void);
	unsigned long getCoffHeaderOffset(void) 		const { return m_dosHeader.getPEHeaderOffset() + sizeof(long); };
	void print(void)  					const ;
	void debugPrint(void) 				const { if (m_debug) print() ; };
	SymbolString * getFirstString(void);

private:
	PEFile() : 
		m_dosHeader(0), 
		m_coffFile(0,NULL) ,
		CodeFile(0,NULL)
	{ };
	PEHeader	m_dosHeader;
	CoffFile	m_coffFile;
};	

	 
#endif 
