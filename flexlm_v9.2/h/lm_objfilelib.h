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
/* $Id: lm_objfilelib.h,v 1.3 2003/06/16 21:11:24 kmaclean Exp $
***********************************************************/
/**   @file lm_objfilelib.h
 *    @brief classes for reading a windows .LIB file
 *    @version $Revision: 1.3 $
 *
  ************************************************************/

#ifndef INCLUDE_LM_OBJFILELIB_H
#define  INCLUDE_LM_OBJFILELIB_H 

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <windows.h>
#include <winnt.h>
#include "lm_objfilecommon.h"
#include "lm_objfilecoff.h"

/*************************************************************/
/** the Endian
 **************************************************************/
typedef enum {
	BigEndianType,
	LittleEndianType
} EndianType;

/*************************************************************/
/** LIB member header
 **************************************************************/
class LibMemberHeader
{
public:
	LibMemberHeader(int debugFlag) : m_debug(debugFlag) {} ;
	~LibMemberHeader() {} ;
	int hRead(const int fd, const unsigned long fileOffset);
	void print(void) 				const;
	void debugPrint(void) 			const { if (m_debug) print() ; };
	int getDataSize(void) 			const { return m_size; };
	int getMode(void) 				const { return m_mode ; } ;
	const char *getName(void) 		const { return (const char *)&(m_data.Name[0]); };
	unsigned long getNextMemberFileOffset(void) const ;

protected:	
	int					m_debug;			/** do we print debug messages */

private:
	LibMemberHeader()  : m_debug(0) {} ;
	IMAGE_ARCHIVE_MEMBER_HEADER	m_data;		/** file header */
	unsigned long 		m_fileOffset;  		/** the file offset we read the data from */
	int					m_size; 			/** size extracted from m_data after the read */
	int					m_mode; 			/** the mode extracted from m_data */
};
/*************************************************************/
/** LIB file header
 **************************************************************/							  
class LibHeader 
{
public:
	LibHeader(int debugFlag) : m_debug(debugFlag) {} ;
	~LibHeader() {} ;
	int hRead(const int fd, const unsigned long fileOffset);
	void print(void) 					const;
	void debugPrint(void) 				const { if (m_debug) print() ; };
	unsigned long getNextOffset(void)	const { return m_nextFileOffset; };

private:
	LibHeader()  : m_debug(0) {} ;
	unsigned long 		m_fileOffset;  /** the file offset we read the data from */
	unsigned long 		m_nextFileOffset;  /** the file offset for the next member in the file */
	int 				m_debug;
};	
/*************************************************************/
/**  class to hold the Library symbol offset / name list found in the 
 * first and second archive members . The Table of Contents
 **************************************************************/
class LibSymbolTOCBase
{
public:
	
	LibSymbolTOCBase(int debugFlag) : 
		m_debug(debugFlag) ,
		m_header(debugFlag),
		m_symbolCount(0),
		m_stringListSize(0),
		m_sectionOffsetList(NULL),
		m_symbolStringList(NULL),
		m_symbolStringBuf(NULL),
		m_fileOffset(0)
	{};	  
	~LibSymbolTOCBase();	
	int 	hRead(const int fd, EndianType whichEndian, const unsigned long fileOffset);
	virtual void 	print(void) 					const;
	void 	debugPrint(void) 						const { if (m_debug) print() ; };
	unsigned long getStringFileOffset(int index) 	const;
	char *	getString(int index) 					const ;
	int 	setNewString(int index, const char * newString);
	int 	getNumSymbols(void) 					const { return m_symbolCount;};
	unsigned long getNextOffset(void)				const { return m_header.getNextMemberFileOffset(); };

protected:
	virtual int getStringListSize(int sectionOffsetCount, int symbolCount);
		/** number of symbols in the lib */
	DWORD				m_symbolCount;	
		/** number of offsets in th section offset list in the lib */
	DWORD				m_sectionOffsetCount;	
		/** list of offsets to the archive member for this symbol */
	DWORD *				m_sectionOffsetList;
		/** Size of the string list  */
	int 				m_stringListSize;
		/** list of offsets into the file for where each of the strings
		 * in m_symbolStringBuf can be found. */
	DWORD *				m_symbolStringFileOffsetList;
		/** This array of pointers is built to point to each of the 
		 * null terminated strings in m_symbolStringBuf.  */
	char  **			m_symbolStringList;
		/** A buffer to hold the string list read from the lib finle */
	char  *				m_symbolStringBuf;

private:	
	LibSymbolTOCBase() : m_header(0) {};	
		/**  Read any more data that is beyond the section offset list and 
		 * before the string list */
	virtual int readMoreData(int fd, EndianType whichEndian, DWORD * symbolCount) ;
		/** the lib member header */
	LibMemberHeader		m_header;
		/** the file offset we read the data from */
	unsigned long 		m_fileOffset;  
	int					m_debug;
} ;	
/*************************************************************/
/**  class to hold the Library symbol offset / name list found in the 
 * first and second archive members . The Table of Contents
 **************************************************************/
class LibSymbolTOC1 : public LibSymbolTOCBase
{
public:
	
	LibSymbolTOC1(int debugFlag) : 
		LibSymbolTOCBase(debugFlag),
		m_debug(debugFlag) ,
		m_fileOffset(0)
	{};	  
	~LibSymbolTOC1() {};	
//	unsigned long getNextOffset(void)				const {	return m_nextFileOffset;};

private:	
	LibSymbolTOC1() : LibSymbolTOCBase(0) {};	
		/** the file offset we read the data from */
	unsigned long 		m_fileOffset;  
	int					m_debug;
} ;	
/*************************************************************/
/**  class to hold the Library symbol offset / name list found in the 
 * first and second archive members . The Table of Contents
 **************************************************************/
class LibSymbolTOC2  : public LibSymbolTOCBase
{
public:
	
	LibSymbolTOC2(int debugFlag) : 
		LibSymbolTOCBase(debugFlag),
		m_debug(debugFlag) ,
		m_fileOffset(0),
		m_indexOffsetList(NULL)
	{};	  
	~LibSymbolTOC2();	
	int readMoreData(int fd, EndianType whichEndian, DWORD * symbolCount) ;
	int getStringListSize(int sectionOffsetCount, int symbolCount);
	void 	print(void) 							const;

private:	   
	LibSymbolTOC2() : LibSymbolTOCBase(0) {};
		/** list of indexes to the offsets .
		 * There are m_symbolCount entries in this list */
	WORD *				m_indexOffsetList;
		/** the file offset we read the data from */
	unsigned long 		m_fileOffset;  
	int					m_debug;
} ;	

/*************************************************************/
/** Class file for the file name list in the library
 **************************************************************/
class LibFileNameTOC 
{
public:
	LibFileNameTOC(int debugFlag) : m_header(debugFlag) {};
	~LibFileNameTOC() {};
	int hRead(int fd, const unsigned long fileOffset);
	unsigned long getNextOffset(void)				const { return m_nextFileOffset; };
private:
	LibFileNameTOC() : m_header(0) {};
		/** the lib member header */
	LibMemberHeader		m_header;
		/** the file offset we read the data from */
	unsigned long 		m_fileOffset; 
	unsigned long 		m_nextFileOffset; 
};	
/*************************************************************/
/** class for reading the LIB file
 **************************************************************/
class LibFile : public CodeFile
{
public:	
	LibFile(int debugFlag, const char * fileName) : 
		CodeFile(debugFlag, fileName),
		m_header(debugFlag),
		m_symbolTOC1(debugFlag),
		m_symbolTOC2(debugFlag),
		m_fileNameTOC(debugFlag)
	{
	};
	
		/** read the important parts of the file */
	int hRead(void);
	void print(void)  			const ;
	void debugPrint(void) 		const { if (m_debug) print() ; };
	SymbolString * getFirstString(void);

private:
	LibFile() :
		CodeFile(0, NULL),
		m_header(0),
		m_symbolTOC1(0),
		m_symbolTOC2(0),
		m_fileNameTOC(0)
	{
	};
	LibHeader			m_header;
	LibSymbolTOC1 		m_symbolTOC1;	/** list of symbols in first Table of contents */
	LibSymbolTOC2 		m_symbolTOC2;	/** list of symbols in the second table of contents */
	LibFileNameTOC		m_fileNameTOC;  /** list of file names in the library */
	unsigned long 		m_fileOffset;  /** the file offset we read the data from */
	unsigned long		m_eofOffset;  /** offset of the end of file */
};	
/*************************************************************/
/** Class to hold the actual LIB member info
 **************************************************************/							  
class LibMember : public CoffFile
{
public:
	LibMember(int debugFlag, int fd, const char * fileName, int fileOffset) : 
		CoffFile(debugFlag, fd, fileName),
		m_header(debugFlag),
		m_debug(debugFlag),
		m_fileOffset(fileOffset)
	{
	};
	int hRead(void);
	void print(void)  		const;
	void debugPrint(void) 	const { if (m_debug) print() ; };
	~LibMember() {};
	unsigned long getSymTableFileOffset(void) 	const { return CoffFile::getSymTableFileOffset() + CoffFile::getFileOffset() ;};
	unsigned long getNextMemberFileOffset(void) const {return m_header.getNextMemberFileOffset();};
	
private:
	LibMember() :
		CoffFile(0,0,NULL),
		m_header(0),
		m_debug(0),
		m_fileOffset(0)
	{
	};

	LibMemberHeader		m_header;
	unsigned long 		m_fileOffset;  /** the file offset we read the data from */
	int					m_debug;
};	

/*************************************************************/
/** Get the next symbol string from the library
 **************************************************************/
class LibSymbolString : public SymbolString
{
public:
	LibSymbolString (
			LibSymbolTOC1 * symbolList1, 
			LibSymbolTOC2 * symbolList2, 
			int debugFlag, 
			int fd, 
			unsigned long nextOffset, 
			unsigned long eofOffset, 
			char * fileName) : 
				m_libSymbolTOC1(symbolList1),
				m_libSymbolTOC2(symbolList2),
				m_fd(fd),
				m_nextMemberOffset(nextOffset),
				m_fileName(fileName),
				m_libMember(NULL),
				m_libMemberSymbol(NULL),
				m_libSymbolListNextIndex(0),
				m_eofOffset(eofOffset),
				SymbolString(debugFlag) 
	{};
	~LibSymbolString() {};
	
	int setNextString(int * noMoreStrings);
	int replaceString(int fd,const char * newString);

private:
	LibSymbolString() :
		m_fd(0),
		m_fileName(NULL),
		m_libSymbolListNextIndex(0),
		SymbolString(0)
	{};

	LibMember *		m_libMember;
	SymbolString *	m_libMemberSymbol;
	int				m_libSymbolListNextIndex;
	unsigned long	m_nextMemberOffset;
	
	LibSymbolTOC1 *	m_libSymbolTOC1;
	LibSymbolTOC2 *	m_libSymbolTOC2;
	const int		m_fd;
	const char *	m_fileName;
	unsigned long 	m_eofOffset;
};

#endif

