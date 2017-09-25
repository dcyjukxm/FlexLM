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
/* $Id: lm_objfilelib.cpp,v 1.4 2003/06/16 21:10:56 kmaclean Exp $
***********************************************************/
/**   @file lm_objfilelib.cpp
 *    @brief classes for reading a windows .LIB file
 *    @version $Revision: 1.4 $
 *
  ************************************************************/

#include "lm_objfilelib.h"
#include "lm_objfilecoff.h"

static void printNumChars(const char * string, const BYTE *data, int count);
static void swapEndianDWORD(DWORD *buf , int count, EndianType whichEndian);
static void swapEndianWORD(WORD *buf , int count, EndianType whichEndian);


/***********************************************************************/
/** @brief read the file headders
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int LibFile::hRead(void)
{
	/* save current file offset */
	m_fileOffset = lseek(m_fd,0,SEEK_CUR);
	/* read the lib header member */
	if (! m_header.hRead(m_fd,m_fileOffset))
		return 0;

	if ( ! m_symbolTOC1.hRead(m_fd,BigEndianType,m_header.getNextOffset()))
		return 0;
	if ( ! m_symbolTOC2.hRead(m_fd,LittleEndianType,m_symbolTOC1.getNextOffset())  )
		return 0;
	if ( ! m_fileNameTOC.hRead(m_fd,m_symbolTOC2.getNextOffset()) )
		return 0;
	m_eofOffset = lseek(m_fd, 0, SEEK_END);
	debugPrint();

	return 1;
}	
/***********************************************************************/
/** @brief print debugging info
************************************************************************/
void LibFile::print(void)  const
{
	if ( m_debug )
		fprintf (stdout,"LibFile:: Done reading \n");
}	
/***********************************************************************/
/** @brief read the first symbol from the file and get it's name
 *
 * @param
 *
 * @return
 *
 ************************************************************************/	  
SymbolString * LibFile::getFirstString(void)
{
	SymbolString *newStr; 
	unsigned long nextOffset = 0;
	int	noMoreStrings;

	nextOffset = m_fileNameTOC.getNextOffset() ;

	/* read it */
	newStr = new LibSymbolString(&m_symbolTOC1, &m_symbolTOC2, m_debug, m_fd, nextOffset,m_eofOffset, m_fileName);
	if (newStr->setNextString(&noMoreStrings))
		return newStr;
	else
	{
		delete newStr;
		return NULL;
	}
}	
/***********************************************************************/
/** @brief Read the library header
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int LibHeader::hRead(const int fd, const unsigned long fileOffset)
{
	char 	start[IMAGE_ARCHIVE_START_SIZE + 1];

	/* save current file offset */
	m_fileOffset = lseek(fd,fileOffset,SEEK_SET);
	if ( read(fd,start, IMAGE_ARCHIVE_START_SIZE) != IMAGE_ARCHIVE_START_SIZE  )
	{
		perror("read");
		return 0;
	}
	if ( strncmp(start,IMAGE_ARCHIVE_START , IMAGE_ARCHIVE_START_SIZE ) != 0)
	{
		fprintf(stderr,"Not a Windows library file\n");
		return 0;
	}

	m_nextFileOffset = lseek(fd,0,SEEK_CUR);
	debugPrint();
	return 1;
}	
/***********************************************************************/
/** @brief Print the LibHeader 
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
void LibHeader::print(void) const
{
	fprintf(stdout, "LibHeader:: !<arch>\n");
}	

/***********************************************************************/
/** @brief Read a LibMemberHeader
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int LibMemberHeader::hRead(const int fd, const unsigned long fileOffset)
{
	/* save current file offset */
	m_fileOffset = lseek(fd,fileOffset,SEEK_SET);
	if ( read(fd, &m_data, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER)) != sizeof(IMAGE_ARCHIVE_MEMBER_HEADER ))
	{
		perror ("read lib headder");
		return 0;
	}
	debugPrint();
	if ( strncmp(reinterpret_cast<const char *>(&(m_data.EndHeader[0])),IMAGE_ARCHIVE_END,2) != 0)
	{
		/* bad end of header */
		fprintf(stderr,"Bad Library member header at offset 0x%x\n",m_fileOffset);
		return 0;
	}
	/* now get the size field */
	m_size = atoi(reinterpret_cast<const char *>(&m_data.Size[0]));

	/* get the mode field. It's in octal */
	sscanf(reinterpret_cast<const char *>(&(m_data.Mode[0])), "%o", &m_mode);

 	return 1;
}
/***********************************************************************/
/** @brief caculate thefile offset to the next lib member in the file
 *
 * @return the new offset
 *
 ************************************************************************/
unsigned long LibMemberHeader::getNextMemberFileOffset(void) const 
{
	unsigned long tmp = m_fileOffset + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) + m_size;
	if ( tmp & 0x01 )
	{
		/* All lib member headers start on an even offset */
		tmp += 1;
	}
	return tmp;
}

/***********************************************************************/
/** @brief print the lib header
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
void LibMemberHeader::print() const
{
	if ( m_debug & DebugPrintStructs)
	{
		fprintf(stdout,"LibMemberHeader:: 0x%x\n",m_fileOffset);
		printNumChars("  Name      =",&(m_data.Name[0]),16);
		printNumChars("  Date      =",&(m_data.Date[0]),12);
    	printNumChars("  UserID    =",&(m_data.UserID[0]) ,6);    
    	printNumChars("  GroupID   =",&(m_data.GroupID[0]) ,6);   
    	printNumChars("  Mode      =",&(m_data.Mode[0]) ,8);      
    	printNumChars("  Size      =",&(m_data.Size[0]) ,10);     
    	printNumChars("  EndHeader =",&(m_data.EndHeader[0]),2);  
	}
}
/***********************************************************************/
/** @brief LibSymbolTOC destructor
 ************************************************************************/
LibSymbolTOCBase::~LibSymbolTOCBase(void)
{
	if ( m_symbolStringFileOffsetList )
		delete[] m_symbolStringFileOffsetList;
	if (m_symbolStringList)
		delete[] m_symbolStringList;
	if (m_symbolStringBuf)
		delete[] m_symbolStringBuf;
}	
/***********************************************************************/
/** @brief LibSymbolTOC destructor
 ************************************************************************/
LibSymbolTOC2::~LibSymbolTOC2(void)
{
	if (m_indexOffsetList)
		delete[] m_indexOffsetList;
}	  

/***********************************************************************/
/** @brief read the symbon names list
 *
 * @param fd	file to read from
 *
 * @return
 * 	- 0 failure
 * 	- 1 success
 ************************************************************************/
int LibSymbolTOCBase::hRead(const int fd, EndianType whichEndian, const unsigned long fileOffset)
{
	unsigned long stringOffset;
						 
	/* read the lib header member */
	if (! m_header.hRead(fd, fileOffset))
		return 0;

	/* save current file offset */
	m_fileOffset = lseek(fd,0,SEEK_CUR);
	/* the next DWORD is the symbol count */
	if ( read(fd, &m_sectionOffsetCount,4) != 4 )
		return 0;
	swapEndianDWORD(&m_sectionOffsetCount, 1,whichEndian);

	m_sectionOffsetList = new DWORD[m_sectionOffsetCount];

	/* read the list of section offsets */
	if ( read(fd, m_sectionOffsetList, m_sectionOffsetCount * sizeof(DWORD)) != (int)(m_sectionOffsetCount * sizeof(DWORD)) )
		return 0;
	swapEndianDWORD(m_sectionOffsetList, m_sectionOffsetCount, whichEndian);

	readMoreData(fd,whichEndian,&m_symbolCount);
	m_stringListSize = getStringListSize(m_sectionOffsetCount, m_symbolCount);

	/* read and process the strings */
	m_symbolStringFileOffsetList = new DWORD[m_symbolCount];
	m_symbolStringBuf = new char[m_stringListSize];
	m_symbolStringList = new char*[m_stringListSize];

	stringOffset = lseek(fd,0,SEEK_CUR);
	if ( ! read(fd, m_symbolStringBuf, m_stringListSize) )
		return 0;
	/* now build the list of string pointers */
	char *  pString = m_symbolStringBuf;
	int lastLen = 0;
	for ( unsigned int  i = 0 ; i < m_symbolCount; i++ )
	{
		stringOffset += lastLen ;
		m_symbolStringFileOffsetList[i] = stringOffset;
		m_symbolStringList[i] = pString;
		lastLen = strlen(pString) + 1;
		pString += lastLen;
	}
	debugPrint();
	return 1;
}
/***********************************************************************/
/** @brief   Read any more data that is beyond the section offset list and 
 *           before the string list
 *
 * @param  fd	the file to read from
 * @param whichEndian  the endian the data wil be in
 * @param returnSymbolCountHere  FIled in with the symbol count
 *
 * @return
 * 		- 0 Failure
 * 		- 1 Success
 ************************************************************************/	  
int LibSymbolTOCBase::readMoreData(const int fd, EndianType whichEndian, DWORD * returnSymbolCountHere)
{
	*returnSymbolCountHere = m_sectionOffsetCount;
	return 1;
}	

/***********************************************************************/
/** @brief caculate the size of the string list
 *
 * @param  sectionOffsetCount number of section offset entries
 * @param  symbolCount	number of symbols
 *
 * @return size of the string list
 *
 ************************************************************************/
int LibSymbolTOCBase::getStringListSize(int sectionOffsetCount, int symbolCount)
{
	/* string list size is the data size from the member header 
	 * subtract the number of section offset entries times sizeof(DWORD)
	 * subtract one more DWORD for the symbol count field
	 * Plus one byte because the lib member is terminated with '\n' */
	return (((m_header.getDataSize() - 
					(sectionOffsetCount * sizeof(DWORD))) - sizeof(DWORD)) + 1);
}
/***********************************************************************/
/** @brief   Read any more data that is beyond the section offset list and 
 *           before the string list
 *
 * @param  fd	the file to read from
 * @param whichEndian  the endian the data wil be in
 * @param returnSymbolCountHere  FIled in with the symbol count
 *
 * @return
 * 		- 0 Failure
 * 		- 1 Success
 ************************************************************************/	  
int LibSymbolTOC2::readMoreData(const int fd, EndianType whichEndian, DWORD * returnSymbolCountHere)
{
	DWORD symCount;
	/* the next DWORD is the symbol count */
	if ( read(fd, &symCount,4) != 4 )
		return 0;
	swapEndianDWORD(&symCount, 1,whichEndian);
	*returnSymbolCountHere = symCount;

	m_indexOffsetList = new WORD[symCount];
	
	if ( read(fd, m_indexOffsetList, symCount * sizeof(WORD)) != (int)(symCount* sizeof(WORD)) )
		return 0;
	swapEndianWORD(m_indexOffsetList, symCount, whichEndian);
	
	return 1;
}	
/***********************************************************************/
/** @brief caculate the size of the string list
 *
 * @param  sectionOffsetCount number of section offset entries
 * @param  symbolCount	number of symbols
 *
 * @return size of the string list
 *
 ************************************************************************/
int LibSymbolTOC2::getStringListSize(int sectionOffsetCount, int symbolCount)
{
	int tmp = LibSymbolTOCBase::getStringListSize(sectionOffsetCount, symbolCount);
	/* In addition to the base class we also need to 
	 * subtract the number of section index entries times sizeof(WORD)
	 * subtract one more DWORD for the symbol count field
	 * */
	tmp -= (symbolCount * sizeof(WORD));
	tmp -= sizeof(DWORD);
	return tmp;
}
/***********************************************************************/
/** @brief get the offset into the file where the symbol string 
 * is stored for the given index
 *
 * @param  index  The index into the file offset table
 *
 * @return
 * 		- The offset into the file for the string
 * 		- 0 on error
 *
 ************************************************************************/
unsigned long LibSymbolTOCBase::getStringFileOffset(int index) const
{
	if ( index >= 0 && index < (int)m_symbolCount )
		return m_symbolStringFileOffsetList[index];
	else 
		return 0;
}	
/***********************************************************************/
/** @brief Get the string for the index specified
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
char * LibSymbolTOCBase::getString(int index) const 
{
	if ( index >= 0 && index < (int)m_symbolCount )
		return m_symbolStringList[index];
	else 
		return NULL;
}
/***********************************************************************/
/** @brief change the in memory copy of the string
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int LibSymbolTOCBase::setNewString(int index, const char * newString)
{
	int newLen;
	int oldLen;
	char * ptr;
		
	if ( ( ptr = getString(index)) == NULL)
			return 0;
	
	oldLen = strlen(ptr);
	newLen = strlen(newString);

	if ( oldLen <= newLen )
	{
		/* the new string is the same or loonger thatn the old string. */
		strncpy(ptr, newString,oldLen);
		ptr[oldLen] = '\0';
	}
	else
	{
		/* usually here because the symbol name has a leading underscore and the
		 * new string does not. */
		int diff = oldLen - newLen;
		strncpy(&(ptr[diff]), newString,newLen);
	}	
	return 1;	
}	
/***********************************************************************/
/** @brief Print the LibSymbolTOC
 ************************************************************************/
void LibSymbolTOCBase::print(void) const
{
	if ( m_debug & (DebugTerse | DebugVerbose | DebugPrintStructs))
	{
		fprintf (stdout,"LibSymbolTOCBase:: %x\n", m_fileOffset);
		fprintf (stdout,"  m_symbolCount = %x\n", m_sectionOffsetCount);
		if ( m_debug & DebugPrintStrings)
		{
			fprintf (stdout,"  member  Symbol offset string offset  symbol\n");
			for ( int i = 0 ; i < (int)m_sectionOffsetCount ; i++)
			{
				fprintf (stdout,"  %.4d: %.8x  %.8x  %s\n", 
						i,
						m_sectionOffsetList[i],
						m_symbolStringFileOffsetList[i],
						m_symbolStringList[i] == NULL ? "" : m_symbolStringList[i]
						);
			}
		}
	}
}	

/***********************************************************************/
/** @brief Print the LibSymbolTOC2
 ************************************************************************/
void LibSymbolTOC2::print(void) const
{
	if ( m_debug & (DebugTerse | DebugVerbose | DebugPrintStructs))
	{
		fprintf (stdout,"LibSymbolTOC2:: %x\n", m_fileOffset);
		fprintf (stdout,"  m_symbolCount = %x\n", m_symbolCount);
		fprintf (stdout,"  m_sectionOffsetCount = %x\n", m_sectionOffsetCount);
		if ( m_debug & DebugPrintStrings)
		{
			int i;
			/* First print the index to section offset list */
			
			fprintf (stdout,"  number  Section offset \n");
			for (i = 0 ; i < (int)m_sectionOffsetCount ; i++)
			{
				fprintf (stdout,"  %.4d: %.8x \n", 
						i,
						m_sectionOffsetList[i]
						);
			}
			fprintf (stdout,"  member Section File offset  symbol\n");
			for ( i = 0 ; i < (int)m_symbolCount ; i++)
			{
				fprintf (stdout,"  %.4d: %.8x  %.8x  %s\n", 
						i,
						m_indexOffsetList[i],
						m_symbolStringFileOffsetList[i],
						m_symbolStringList[i] == NULL ? "" : m_symbolStringList[i]
						);
			}
		}
	}
}	
/***********************************************************************/
/** @brief Read the lib member for the file name list
 *
 * @param fd  the file to read from
 *
 * @return
 * 		- 0 failure
 * 		- 1 success
 *
 ************************************************************************/
int LibFileNameTOC ::hRead(int fd, const unsigned long fileOffset)
{
	m_fileOffset = lseek(fd,fileOffset,SEEK_SET);
	if ( ! m_header.hRead(fd, fileOffset) )
		return 0;
	/* don't realy care what is in here. just seek over it.
	add one because the member is terminated with a'\n'*/
	m_nextFileOffset= m_header.getNextMemberFileOffset();
	return 1;

}	
/***********************************************************************/
/** @brief Read the LibMember object from the file
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int LibMember::hRead(void)
{
	if (lseek(m_fd, m_fileOffset, SEEK_SET) < 0)
		return 0;
	if ( m_header.hRead(m_fd,m_fileOffset) )
	{
		/* all coff offsets in a LIB file are relative to the coff header.
		 * Unlike a Windows PE file where the coff offsets are relative to 
		 * the beginning of the file */
		setBaseOffset(lseek(m_fd, 0, SEEK_CUR));
		if ( CoffFile::hRead() )
		{
			return 1;
		}
	}
	return 0;
}	  
/***********************************************************************/
/** @brief Print the LibMember class
 ************************************************************************/
void LibMember::print(void)  const 
{
	if ( m_debug & (DebugTerse | DebugVerbose))
		fprintf(stdout,"LibMember:: 0x%x\n", m_fileOffset);
}	

/***********************************************************************/
/** @brief get the next symbol string
 * 
 * 1. Iterate throught the first table of contents
 * 2. Iterate through the second table of contents
 * 3. Read each Coff object in turn from the file
 * 	  get the next string from there.
 *
 * @return
 *
 ************************************************************************/
int LibSymbolString::setNextString(int * noMoreStrings)
{
	int ok = 1;
	*noMoreStrings = 0;
	if ( m_libSymbolTOC1 != NULL )
	{
		const char *ptr;
		/* get the symbol string for this index */
		if ( ( ptr = m_libSymbolTOC1->getString(m_libSymbolListNextIndex)) != NULL)
		{
			m_setString(ptr);
			if ( m_debug & DebugPrintStrings)
				fprintf(stdout,"Lib     %.6d: %s\n",m_libSymbolListNextIndex,getStringPointer());
			m_libSymbolListNextIndex++;
			return 1;
		}	
		else
		{
			/* no more string in this TOC. Go to the next */
			m_libSymbolListNextIndex = 0;
			m_libSymbolTOC1 = NULL;
		}	
	}
	if ( m_libSymbolTOC2 )
	{
		const char *ptr;
		/* get the symbol string for this index */
		if ( ( ptr = m_libSymbolTOC2->getString(m_libSymbolListNextIndex)) != NULL)
		{
			m_setString(ptr);
			if ( m_debug & DebugPrintStrings)
				fprintf(stdout,"Lib     %.6d: %s\n",m_libSymbolListNextIndex,getStringPointer());
			m_libSymbolListNextIndex++;
			return 1;
		}	
		else
		{
			/* no more string in this TOC. Go to the next */
			m_libSymbolListNextIndex = 0;
			m_libSymbolTOC2 = NULL;
		}	
	}

	/* now we iterate through the coff objects */
		/* no current libMember so set the first one */
		
	do
	{
		if ( m_libMember == NULL )
		{
			*noMoreStrings = 0;
			if ( m_nextMemberOffset  <= 0 || m_nextMemberOffset >= m_eofOffset )
			{
				/* no more symbols */
				*noMoreStrings = 1;
				break;
			}
			/* We have a new member to lookup in the file */
			/* get a new LibMember which contains a CoffFile */
			m_libMember = new LibMember(m_debug, m_fd, m_fileName,m_nextMemberOffset);

			if ((ok = m_libMember->hRead()) != 0)
			{
				m_nextMemberOffset = m_libMember->getNextMemberFileOffset() ;
				if ((m_libMemberSymbol = m_libMember->getFirstString()) != NULL)
				{
					/* get the next symbol from this CoffFile object */

					m_setString(m_libMemberSymbol->getStringPointer());
					break;
				}	
			}

			/* no symbols in this member so free it */
			delete m_libMember;
			m_libMember = NULL;
			if ( ! ok )
				break;
		}

		if ( m_libMemberSymbol != NULL )
		{
			*noMoreStrings = 0;
			/* get the next symbol from this CoffFile object */
			ok = m_libMemberSymbol->setNextString(noMoreStrings);
			if ( ok && ! *noMoreStrings )
			{
				m_setString(m_libMemberSymbol->getStringPointer());
				break;
			}

			/* no more symbol strings for this LibMember. free it */
			delete m_libMemberSymbol;
			m_libMemberSymbol = NULL;
			delete m_libMember;
			m_libMember = NULL;
			if ( ! ok )
				break;

		}
	} while (1);
	
	return ok;
}	

/***********************************************************************/
/** @brief replace symbol strings in the lib
 *
 * @param
 *
 * @return
 *
 ************************************************************************/	  
int LibSymbolString::replaceString(int fd, const char * newString)
{
	
	/* always need to replace the number of bytes that were there in the first place */
	int oldLen = strlen(getStringPointer());
	
	if ( strlen(newString) == 0 )
	{
		fprintf(stderr,"Can't set symbols to zeros in a library\n");
		return 0;
	}
	/* reset our symbol name */
	m_setReplacementString(newString);

	if ( m_libMemberSymbol )
	{
		/* the LibMember should replace the string  */
		return m_libMemberSymbol->replaceString(fd, getStringPointer());
	}

	if ( m_libSymbolTOC1 != NULL)
	{
		/* replace the in-memory copy */
		if ( m_libSymbolTOC1->setNewString(m_libSymbolListNextIndex - 1, getStringPointer()))
		{
			/* write to the file */
			return SymbolString::m_writeNewStringToFile(fd, m_libSymbolTOC1->getStringFileOffset(m_libSymbolListNextIndex - 1), oldLen);
		}
	}	
	if ( m_libSymbolTOC2 != NULL)
	{
		/* replace the in-memory copy */
		if ( m_libSymbolTOC2->setNewString(m_libSymbolListNextIndex - 1, getStringPointer()))
		{
			/* write to the file */
			return SymbolString::m_writeNewStringToFile(fd, m_libSymbolTOC2->getStringFileOffset(m_libSymbolListNextIndex - 1), oldLen);
		}
	}	
	return 0;
}	


/***********************************************************************/
/** @brief Local function to print out some number of chars in a string
 *
 * @param
 *
 * @return
 *
 ************************************************************************/	  
static void printNumChars(const char  * string, const BYTE *data, const int count)
{
	fprintf(stdout,string); 
	for ( int i = 0; i < count; i++)
	{
		fprintf(stdout,"%c",data[i]);
	}
	fprintf(stdout,"\n");
	fflush(stdout);
}	
/***********************************************************************/
/** @brief read a number of big endian DWORDs from a file
 *
 * @param buf the array of DWORDs to swap
 * @param count  the number of DWORDs to swap
 * @param whichEndian which endian should the result be in
 ************************************************************************/
static void swapEndianDWORD(DWORD *buf , int count, EndianType whichEndian)
{

	static DWORD t = 0x01020304;
	EndianType myEndian;
	if ( *((unsigned char *)&t) == 4 )
		myEndian = LittleEndianType;
	else
		myEndian = BigEndianType;

	if ( myEndian == whichEndian )
		return ; /* Same endian. don't need to do anything */

	for ( int i = 0; i < count; i++, buf++ )
	{
		DWORD x;
		x = *buf;
		x = (x >> 24) | (x << 24) | (x & 0x0000ff00) << 8 | (x &0x00ff0000) >> 8;
		*buf = x;
	}
	return ;
}	

/***********************************************************************/
/** @brief read a number of WORDs from a file
 *
 * @param buf the array of WORDs to swap
 * @param count  the number of WORDs to swap
 * @param whichEndian which endian should the result be in
 ************************************************************************/
static void swapEndianWORD(WORD *buf , int count, EndianType whichEndian)
{
	static WORD t = 0x0102;
	EndianType myEndian;
	if ( *((unsigned char *)&t) == 2 )
		myEndian = LittleEndianType;
	else
		myEndian = BigEndianType;

	if ( myEndian == whichEndian )
		return ; /* Same endian. don't need to do anything */

	for ( int i = 0; i < count; i++, buf++ )
	{
		WORD x;
		x = *buf;
		x = (x >> 8) | (x << 8) ;
		*buf = x;
	}
	return ;
}	
