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
/* $Id: lm_objfilecommon.h,v 1.3 2003/06/14 00:20:48 kmaclean Exp $
***********************************************************/
/**   @file objfilecommon.h
 *    @brief common classes for reading object files.
 *    @version $Revision: 1.3 $
 * 
 * These classes are used by lmstrip to read the object file (lib or exe)
 * in it's native format. 
 ************************************************************/

#ifndef INCLUDE_OBJFILECOMMON_H
#define  INCLUDE_OBJFILECOMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>
#define read(a,b,c)		_read(a,b,c)
#define open(a,b)		_open(a,b)
#define close(a)		_close(a)
#define lseek(a,b,c)	_lseek(a,b,c)
#define tell(a)			_tell(a)
const int MyOpenFlags = (O_RDWR | O_BINARY );
#else 
#include <unistd.h>
const int MyOpenFlags = (O_RDWR);
#endif 


const int MaxFileNameLen  =	 512;
const int MaxSymbolNameLen = 512;

/** Constaants used by the m_debug member in many structs */
const int DebugNone		= 0;			/** no debugging  */
const int DebugTerse	= 0x01;         /** terse debug output */
const int DebugVerbose	= 0x02;			/** verbose debug putput */
const int DebugReplace	= 0x04;			/** print debug info when a symbol is replaced */
const int DebugPrintStructs	= 0x08;     /** print debug info for all structs read */
const int DebugPrintStrings	= 0x10;     /** print debug info for all strings read */
const int DebugAll		= 0xffffffff;   /** turn on all debugging  */

class CodeFile; /* forward */
/** a string that came from a symbol file or object file */
class SymbolString
{
public:
	SymbolString(int debug) : m_debug(debug)
	{ 
		m_currentSymbolName[0] = '\0' ;
	};
	~SymbolString() {};
	virtual int setNextString(int * noMoreStrings) = 0;
	virtual int replaceString(int fd, const char * newString) = 0;
	const char * getStringPointer(void)  const { return m_currentSymbolName; };
	
	int isValid(void) 	{ return (m_currentSymbolName == '\0') ? 0 : 1  ; };

protected:
	void 		m_setReplacementString(const char *newString) {m_setReplacementString(newString, MaxSymbolNameLen);};
	void 		m_setReplacementString(const char *newString, int maxLen);
	void 		m_setString(const char *newString) { m_setString(newString, MaxSymbolNameLen); };
	void 		m_setString(const char *newString, int maxLen);
	int 		m_writeNewStringToFile(int fd, unsigned long offset, int len) const;
	int			m_debug;

private:
	SymbolString() : m_debug(0) {};
	char 		m_currentSymbolName[MaxSymbolNameLen + 1];

};

/*************************************************************/
/** Virtual base class to enable us to read any type of objeect file
 * with a common interface.
 **************************************************************/
class CodeFile 
{
public:	
	
	static CodeFile * getFileObject(int debugFlag, const char * fileName, const char * progName);
	
	CodeFile(int debugFlag, const char *fileName);
	CodeFile(int debugFlag, int fd, const char *fileName);
	~CodeFile() {};

		/** open the file */ 
	virtual int 	fileOpen(void);		   
		/** close the file */
	virtual void 	fileClose(void);
		/** read the important parts of the file */
	virtual int 	hRead(void) 				  = 0;
	virtual void 	print(void) 			const = 0	;
	virtual void 	debugPrint(void) 		const = 0 ;
	virtual SymbolString * getFirstString(void)   = 0;
	int		getFd(void) 					const {  return m_fd;	}  ;
	void 	setFd(int fd) 						  {	m_fd = fd; };

protected:
	int			m_fd;  /** file descriptor for the open file */
	char		m_fileName[MaxFileNameLen];	/** file name */
	int			m_debug	;

private:
	CodeFile() : m_fd (-1), m_debug(0) 
	{
		m_fileName[0] = '\0';
	};

};


#endif 
