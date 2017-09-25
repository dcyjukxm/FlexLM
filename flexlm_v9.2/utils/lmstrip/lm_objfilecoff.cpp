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
/* $Id: lm_objfilecoff.cpp,v 1.4 2003/06/14 00:27:24 kmaclean Exp $
***********************************************************/
/**   @file l_objfilecoff.cpp
 *    @brief Routines to read COFF info from an object file
 *    @version $Revision: 1.4 $
 *
  ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
//#include "lmclient.h"
#include "lM_objfilecoff.h"

static int s_doRead(int fd, void  *dstPtr, const int count);

/*-----------------4/14/2003 2:56PM-----------------
 * Class CoffFile
 *------------------------------------------------*/

/***********************************************************************
 * Read the coff file header
 * 
 * Return:
 * 		1 - success
 * 		0 - error
 ************************************************************************/
int CoffFile::hRead(void)	  
{
	unsigned long	stringTableOffset;
	unsigned long	offset;

	m_fileOffset = lseek(m_fd,0,SEEK_CUR);

	if (! m_coffHeader.hRead(m_fd, m_fileOffset))
		return 0;

	/* optional header goes here. 
	 * We ignore it so set the offset to just past it. */
	/* if ( ! m_optionalHeader.hRead(fd, m_data.getOptHeaderSize())  )
		return 0;
	*/

	if ((offset = lseek(m_fd,0,SEEK_CUR) ) <= 0)
		return 0;
	offset += m_coffHeader.getOptHeaderSize();
	/* read all the section headers */
	if (! m_sectionList.hRead(m_fd, offset,m_coffHeader.getNumSections()))
		return 0;

	if ( m_coffHeader.getSymTableOffset() == 0 || m_coffHeader.getNumSyms() == 0)
	{
		/* symbols have been stripped */
		fprintf(stderr,"No symbols in executable\n");
		return 0;
	}
	/* caculate the file offset to the string table */
	stringTableOffset = getSymTableFileOffset() + (m_coffHeader.getNumSyms() * sizeof(COFF_SYMBOL));
	
	/* read the string table */
	/* first get the offset to the data for the first section. 
	 * The string table should not extend beyond this point */
	if (! m_stringTable.hRead(m_fd,stringTableOffset ))
		return 0;

	return 1;
}
/***********************************************************************/
/** @brief read the entire coff file.
 * 
 * This does not save all the information in the coff file. It will cause the 
 * read routines to be called for every coff entry that we support. 
 * So if debugging is turned on all the structures in the file will be printed
 * 
 * @return
 *
 ************************************************************************/
int CoffFile::readEntireFile(void)
{
	/* read the file headers */
//	if ( hRead() )
//	{
//		CoffSymbol *symbol ;
//		/* read the symbol entries */
//		/* seek to the first symbol in the coff file */
//		if (lseek(m_fd,getSymTableFileOffset(),SEEK_SET) == -1L)
//		{
//			perror("seek to symbol");
//			return 0;
//		}	
//		for ( int currentSymbol = 0 ; currentSymbol < getNumSyms(); currentSymbol++)
//		{
//			symbol.hRead(m_fd);
//		}
		
//		/* read all the relocation entries */
//		/* loop through all the sections to get the relocation info for each */
		
//	}
	return 0;
}	

/***********************************************************************/
/** @brief Print the Coff file info
 ************************************************************************/
void CoffFile::print(void)  const
{
	if ( m_debug & (DebugTerse | DebugVerbose))
		fprintf(stdout,"CoffFile:: Offset = 0x%x\n",m_fileOffset);
}
/***********************************************************************/
/** @brief read the first symbol from teh file and get it's name
 *
 * @param
 *
 * @return
 *
 ************************************************************************/	  
SymbolString * CoffFile::getFirstString(void)
{
	return  new CoffSymbolString(&m_stringTable,m_debug,m_fd,getNumSyms(),getSymTableFileOffset());
}	  

/*-----------------4/14/2003 2:57PM-----------------
 * class CoffHeader
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Read the Coff Header
 *
 * @param fd  the file descriptor
 * @param offset file offset to start at
 *
 * @return
 *
 ************************************************************************/
int CoffHeader::hRead(const int fd, const unsigned long offset)
{
	if ((unsigned long)lseek(fd, offset, SEEK_SET) != offset)
	{
		perror("lseek");
		return 0;
	}	
	m_fileOffset = lseek(fd, 0, SEEK_CUR);	/* save the current offset */
	if ( read(fd,&m_data, sizeof(COFF_FILE_HEADER)) != sizeof(COFF_FILE_HEADER) )
	{
		perror("Coff Header read");
		return 0;
	}
	debugPrint();
	if ( m_data.f_magic != m_expectedMagic )
	{
		fprintf(stderr,"Bad magic number in COFF header at file offset 0x%x. \n", m_fileOffset);
		fprintf(stderr,"Expected 0x%x found 0x%x\n",m_expectedMagic, m_data.f_magic);
		return 0;
	}
	return 1;
}	
/***********************************************************************/
/** @brief Print the Coff file info
 ************************************************************************/
void CoffHeader::print(void) const
{
	if ( m_debug & DebugPrintStructs)
	{
		fprintf(stdout, "CoffHeader::offset",m_fileOffset);
		fprintf(stdout, " f_magic ", (X_UINT32)m_data.f_magic);
		fprintf(stdout, " f_nscns ", m_data.f_nscns);
		fprintf(stdout, " f_timdat", m_data.f_timdat);
		fprintf(stdout, " f_symptr", m_data.f_symptr);
		fprintf(stdout, " f_nsyms ", m_data.f_nsyms);
		fprintf(stdout, " f_opthdr", m_data.f_opthdr);
		fprintf(stdout, " f_flags\n", m_data.f_flags);
		fprintf(stdout, "CoffHeader::%x",m_fileOffset);
		fprintf(stdout, " %0.8x", (X_UINT32)m_data.f_magic);
		fprintf(stdout, " %0.8x", m_data.f_nscns);
		fprintf(stdout, " %0.8x", m_data.f_timdat);
		fprintf(stdout, " %0.8x", m_data.f_symptr);
		fprintf(stdout, " %0.8x", m_data.f_nsyms);
		fprintf(stdout, " %0.8x", m_data.f_opthdr);
		fprintf(stdout, " %0.8x\n", m_data.f_flags);
	}
}	

/*-----------------4/14/2003 3:11PM-----------------
 * class CoffSectionList
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Read the Coff Section header list
 *
 * @param fd  the file descriptor
 * @param offset file offset to start at
 *
 * @return
 *
 ************************************************************************/
int CoffSectionList::hRead(const int fd, const unsigned long offset, const int numSections)
{
	if ((unsigned long)lseek(fd, offset, SEEK_SET) != offset)
	{
		perror("lseek");
		return 0;
	}	
	m_fileOffset = offset;
	m_numSections = numSections;
	m_list = new CoffSection[m_numSections];
	if ( m_list == NULL )
		return 0;	
	for ( int i = 0 ; i < m_numSections; i++ )
	{                                     
		if ( m_debug == DebugVerbose )
			m_list[i].setDebug(DebugTerse);
		else
			m_list[i].setDebug(m_debug);
		if (!m_list[i].hRead(fd))
			return 0;
	}
	debugPrint();
	return 1;
}	

/***********************************************************************/
/** @brief Print the Coff file info
 ************************************************************************/
void CoffSectionList::print(void) const
{
	if ( m_debug & DebugPrintStructs)
	{
		fprintf(stdout,"CoffSectionList:: Offset = 0x%x\n",m_fileOffset);
		fprintf(stdout,"  %8s %-8s   %8s   %8s   %8s   %8s   %8s   %8s   %8s \n","offset","name","size","scnptr","relptr", "lnnlptr","nreloc","nlnno","flags");
		for ( int i = 0 ; i < m_numSections; i++ )
		{                                     
			m_list[i].debugPrint();
		}
	}
}	
/*-----------------4/14/2003 3:11PM-----------------
 * class CoffSection
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Read the Coff Section header. 
 * 
 * Assume the file pointer points at the section
 *
 * @param fd  the file descriptor
 *
 * @return
 *
 ************************************************************************/
int CoffSection::hRead(const int fd)
{
	m_fileOffset = lseek(fd, 0, SEEK_CUR);	/* save the current offset */
	if ( read(fd,&m_data, sizeof(COFF_SECTION_HEADER)) != sizeof(COFF_SECTION_HEADER) )
	{
		perror("Coff section Header read");
		return 0;
	}
	//debugPrint();
	return 1;
}	
/***********************************************************************/
/** @brief Print the Coff file info
 ************************************************************************/
void CoffSection::print(void) const
{
	char name[COFF_SECTION_NAME_LEN + 1];

	if ( m_debug & DebugPrintStructs)
	{
		strncpy(name,m_data.s_name, COFF_SECTION_NAME_LEN);
		name[COFF_SECTION_NAME_LEN] = '\0';
		fprintf(stdout,"0x%.8x ",m_fileOffset);
		fprintf(stdout, "%-8s ",	name);
		fprintf(stdout, "0x%0.8x ",  (X_UINT32)(m_data.s_size   	));
		fprintf(stdout, "0x%0.8x ",  (X_UINT32)(m_data.s_scnptr 	));
		fprintf(stdout, "0x%0.8x ",  (X_UINT32)(m_data.s_relptr 	));
		fprintf(stdout, "0x%0.8x ",  (X_UINT32)(m_data.s_lnnoptr 	));
		fprintf(stdout, "0x%0.8x ",  (X_UINT32)(m_data.s_nreloc 	));
		fprintf(stdout, "0x%0.8x ",  (X_UINT32)(m_data.s_nlnno 	));
		fprintf(stdout, "0x%0.8x\n",  (X_UINT32)(m_data.s_flags  	));
	}
}	

/*-----------------4/14/2003 3:44PM-----------------
 * Class CoffStringTable
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Read the coff string table
 *
 * @param fd  the file descriptor
 * @param offset the file startig offset
 *
 * @return
 *
 ************************************************************************/
int CoffStringTable::hRead(const int fd, const unsigned long offset)
{
	unsigned long endOfFile = lseek(fd,0,SEEK_END);
	if ((unsigned long)lseek(fd, offset, SEEK_SET) != offset)
	{
		perror("lseek");
		return 0;
	}	
	m_fileOffset = offset;
	/* the first four bytes contain the size of the table */
	if ( read(fd,&m_lenBytes, 4) != 4)
	{
		perror("Coff string table len read");
		return 0;
	}
	if ( m_debug)
		fprintf(stdout,"CoffStringTable:: Offset = 0x%x length = %d\n",m_fileOffset,m_lenBytes  );
	if ( (unsigned long)m_lenBytes + (unsigned long)m_fileOffset > endOfFile )
	{
		fprintf(stderr,"invalid COFF string table length\n");
		return 0;
	}	

	m_data = new char[m_lenBytes];
	if ( m_data == NULL )
	{
		return 0;
	}
	*((long *)m_data) = m_lenBytes;  /* put the lenght in the in-memory copy */
	/* read the string table. Offset by four because the length is in the first 
	 * four bytes of the table */
	if ( m_lenBytes > 4 )
	{
		/* subtract 4 from the length because we already read the length field */
		if (!s_doRead(fd,m_data + 4,m_lenBytes - 4))
		{
			perror("String table read");
			return 0;
		}
	}
	debugPrint();
	return 1;
}	
/***********************************************************************/
/** @brief Print the string table
 ************************************************************************/
void CoffStringTable::print(void) const
{
	if ( m_debug & (DebugTerse | DebugVerbose | DebugPrintStructs))
	{
		fprintf(stdout,"CoffStringTable:: Offset = 0x%x length = %d\n",m_fileOffset,m_lenBytes  );
		if ( m_debug & DebugPrintStrings)
		{
			/* print the entire string table */
			unsigned long		offset = 4; /* step over the length */
			char *	p = m_data + offset; 

			for ( ; offset < m_lenBytes;  )
			{
				fprintf(stdout, "0x%.8x: %s\n",offset, p);
				offset += strlen(p) + 1;
				p = m_data + offset;
			}
		}
	}
}	

/***********************************************************************/
/** @brief given an offset lookup the string in the string table and return it
 *
 * @param offset the offset into the string table
 *
 * @return 
 * 		- On success the pointer to the string
 * 		- On error NULL. the offset is invalid
 *
 ************************************************************************/
char * CoffStringTable::lookupString(unsigned long offset)  const
{
	if ( offset > m_lenBytes - 1 || m_data == NULL)
	{
		/* invalid offset */
		return NULL ;
	}
	return m_data + offset;

}	
/***********************************************************************/
/** @brief Replace the string in the strig table with the new string
 * 
 * Change only the in-memory copy 
 *
 * @param	tableOffset The offset in to the string table
 * @param newString The new string to write
 *
 * @return offset to write the new string to
 *
 ************************************************************************/
int CoffStringTable::replaceString(int fd, unsigned long tableOffset,const char * newString)
{
	char *tablePtr = lookupString( tableOffset);
	int len;
	
	if (tablePtr == NULL)
		return 0;
	len = strlen(tablePtr);
	/* first replace it in the in-memory copy of the string table */
	memset(tablePtr, 0,len); /* zero out first incase the new string is shorter */
	strncpy(tablePtr, newString, len);
	*(tablePtr + len) = '\0';
	
	return m_fileOffset + tableOffset;
}

/*-----------------4/14/2003 3:56PM-----------------
 * class CoffSymbol
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Read a single coff symbol entry.
 * 
 * Assume the file pointer points at the symbol entry in the file
 *
 * @param fd the file descriptor
 * 
 * @return
 * 		- 1 success
 * 		- 0 failure
 * 
 ************************************************************************/
int CoffSymbol::hRead(int fd)
{
	/* save the file position */
	m_offsetInFile = lseek( fd,0, SEEK_CUR);

 	if (read(fd, &m_data, sizeof(COFF_SYMBOL)) != sizeof(COFF_SYMBOL))
		return 0;

	debugPrint();
	/* get rid of any extra AUX entries. We don't use them. */
	for (int i = m_data.n_numaux; i; i-- )
	{
		CoffAuxSymbol aux(m_debug);
		aux.hRead(fd);
	}
	return 1;
}
/***********************************************************************/
/** @brief set a new symbol string in the symbol entry
 *
 * @param  string the new string to set
 *
 ************************************************************************/
void CoffSymbol::setString(const char *string)		
{
		/* clear first because the new string may be shorter */
	memset(m_data._name._n_name,0,COFF_SYMBOL_NAME_LEN);
	strncpy(m_data._name._n_name,string ,COFF_SYMBOL_NAME_LEN);
}
/***********************************************************************/
/** @brief increment the symbol counter
 * 
 * @return the new count
 ************************************************************************/
int	 CoffSymbol::incrementSymbolCount(void)
{
	m_currentSymbolIndex++;
	m_currentSymbolIndex += getNumAuxEntries();
	return m_currentSymbolIndex;
}

/***********************************************************************/
/** @brief printout the symbol entry
 *******************************************************************/
void CoffSymbol::print(int debugLevel) const
{
	if (debugLevel &  DebugPrintStructs)
	{
		fprintf(stdout,"%.6x 0x%.8x: val=0x%.8x scnum=%2d class=%2d aux=%d ",
						m_currentSymbolIndex,
						m_offsetInFile,
						(X_UINT32)m_data.n_value,
						(X_INT32) m_data.n_scnum,
						(X_UINT32)m_data.n_sclass,
						(X_UINT32)m_data.n_numaux
						);
		if ( m_data._name._n_n._n_zeroes == 0 )
		{
			fprintf(stdout,"off=%.8x \n",m_data._name._n_n._n_offset);
		}
		else
		{
			char str[9];
			strncpy(str, m_data._name._n_name,8);
			str[8] = '\0';
			fprintf(stdout,"%s\n",str);
		}
	}
}	
/*-----------------4/14/2003 3:56PM-----------------
 * class CoffAuxSymbol
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Read a single coff aux symbol entry.
 * 
 * Assume the file pointer points at the symbol entry in the file
 *
 * @param fd the file descriptor
 * 
 * @return
 * 
 ************************************************************************/
int CoffAuxSymbol::hRead(int fd)
{
 	if (read(fd, &m_data, sizeof(COFF_SYMBOL)) != sizeof(COFF_SYMBOL))
		return 0;
	else
		return 1;
}
/***********************************************************************/
/** @brief printout the symbol entry
 *******************************************************************/
void CoffAuxSymbol::print(int debugLevel) const
{
	if ( debugLevel & DebugPrintStructs )
	{
		fprintf(stdout,"CoffAuxSymbol:: \n");
		fprintf(stdout," name: %s\n", m_data.aux_file_name);
		fprintf(stdout," symbol.tag_index: %d\n",(int) m_data.symbol.tag_index);
		fprintf(stdout," symbol.s.c_line_nbr: %d\n", (int) m_data.symbol.u1.s.c_line_nbr);
		fprintf(stdout," symbol.s.size: %d\n", (int) m_data.symbol.u1.s.size);
		fprintf(stdout," symbol.func_size: %d\n", (int) m_data.symbol.u1.func_size);
		fprintf(stdout," symbol.high_size: %d\n", (int) m_data.symbol.high_size);
		fprintf(stdout," u2.\n");
		fprintf(stdout," sec.len: %d \n",(int)m_data.sec.len );
		fprintf(stdout," sec.nbr_rel_ents %d\n",(int)m_data.sec.nbr_rel_ents );
		fprintf(stdout," sec.nbr_line_nbrs %d\n",(int)m_data.sec.nbr_line_nbrs );
	}
}	
/*-----------------5/9/2003 4:41PM-----------------
 * CoffRelocationEntry
 *------------------------------------------------*/
/***********************************************************************/
/** @brief read one coff relocation entry
 *
 * @param	fd file descriptor to read from
 *
 * @return
 * 		- 1 success
 * 		- 0 failure
 *
 ************************************************************************/
int CoffRelocationEntry::hRead(const int fd)
{
 	if (read(fd, &m_data, sizeof(COFF_RELOCATION)) != sizeof(COFF_RELOCATION))
		return 0;
	else
	{
		debugPrint();
		return 1;
	}
}	
/***********************************************************************/
/** @brief print the Coff relocation entry
 *
 * @param debugLevel	determines what to print
 ************************************************************************/
void CoffRelocationEntry::print(int debugLevel) const
{
	if ( debugLevel & DebugPrintStructs )
	{
		fprintf(stdout,"CoffRelocationEntry:: \n");
		fprintf(stdout,"  0x%x\n", m_data.u.r_virt_address);
		fprintf(stdout,"  0x%x\n", m_data.r_symtbl_index);
		fprintf(stdout,"  %d\n", m_data.r_type);
	}
}	

/*-----------------5/9/2003 4:41PM-----------------
 * CoffSymbolString Class
 *------------------------------------------------*/
/***********************************************************************/
/** @brief Constructor
 ************************************************************************/								  
CoffSymbolString::CoffSymbolString(CoffStringTable * stringTable,int debug, int fd, int numSymbols, int symbolFileOffset)  :
		m_stringTable(stringTable),
		m_fd(fd),
		m_numSymbols(numSymbols),
		m_nextFileOffset (symbolFileOffset),
		SymbolString(debug) ,
		m_coffSymbol(debug),
		m_stringFromTableOffset(4)	/* step over the length field in the string table */
{
				
	char * x ;
	if ( m_stringTable != NULL )
	{
		/* set the string to the first string in  the string table  */
		x = m_stringTable->lookupString(m_stringFromTableOffset);
		if ( x != NULL && strlen(x) > 0)
		{
			if ( m_debug & DebugPrintStrings)
				fprintf(stdout,"Coff T  %.6d: %s\n",m_stringFromTableOffset,x);
			m_setString( x, strlen(x));
			return ;
		}
	}	
	/* there is no string table so get the first one
	 * from the symbol table */
	int noMoreStrings;
	setNextString(&noMoreStrings);
}

/***********************************************************************/
/** @brief Set the object to the next symbol string in the file
 *
 * @return Success or failure
 ************************************************************************/
int CoffSymbolString::setNextString(int * noMoreStrings)
{

	*noMoreStrings = 0;
	if ( m_stringFromTableOffset)
	{
		/* get the string from the string table */
		char * x ;
		x = m_stringTable->lookupString(m_stringFromTableOffset);
		if ( x != NULL && strlen(x) > 0 )
		{
			/* get the offset to the next string */
			m_stringFromTableOffset += strlen(x) + 1;
			/* get the next string */	
			x = m_stringTable->lookupString(m_stringFromTableOffset);
			if ( x != NULL && strlen(x) > 0)
			{
				if ( m_debug & DebugPrintStrings)
					fprintf(stdout,"Coff T  %.6d: %s\n",m_stringFromTableOffset,x);
				m_setString( x, strlen(x));
				return 1;
			}
		}
		/* no more strings in the string table */
		m_stringFromTableOffset = 0;
	}

	do {

		/* get the string from the symbol table.
		 * Skip entries that point to the string table
		 * because they have already been processed.*/

		/* which symbol number is the symbol we are about to read */
		int numSyms = m_coffSymbol.incrementSymbolCount();
		if ( numSyms > m_numSymbols)
		{
			*noMoreStrings = 1;
			return 1; /* no more symbols */
		}
	
		if ( m_nextFileOffset != 0 )
		{
			/* need to seek to the correct file position because we may have 
			 * changed the file offset by writing to the file since the last 
			 * symbol read */
			if ( (unsigned long)lseek(m_fd,m_nextFileOffset, SEEK_SET) != m_nextFileOffset )
			{
				perror("lseek");
				return 0;
			}
		}
		if (! m_coffSymbol.hRead(m_fd))
			return 0;

		/* get the symbol name */
		/* if it's an offset into thye string table ignore it */
		if ( m_coffSymbol.getZeros() != 0 )
		{
			/* the real string is in the symbol struct.  */
			m_setString( m_coffSymbol.getName(), COFF_SYMBOL_NAME_LEN);
			if ( m_debug & DebugPrintStrings)
				fprintf(stdout,"Coff S  %.6d: %s\n",m_coffSymbol.getSymbolNumber(),getStringPointer());
		}	
		else if ( m_debug  & DebugPrintStrings)
		{
			/* debug message indicating we skipped this string */
			fprintf(stdout,"Coff SX %.6d: %s\n",m_coffSymbol.getSymbolNumber(),
				m_stringTable->lookupString(m_coffSymbol.getStringTableOffset()));
		}

		m_nextFileOffset = lseek(m_fd,0, SEEK_CUR);
		/* skip all the symbols that point into the string table */
	} while ( m_coffSymbol.getZeros() == 0 );

	return 1;
}	
/***********************************************************************/
/** @brief Replace the symbol name in the file
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
int CoffSymbolString::replaceString(int fd, const char *newString)
{
	int oldLen = strlen(getStringPointer());
	int len;
	int offset;

	/* reset our symbol name */
	m_setReplacementString(newString);

	if ( m_stringFromTableOffset )
	{
		/* we're using the string table directly so 
		 * set the string in the string table */
		offset = m_stringTable->replaceString(fd,m_stringFromTableOffset ,getStringPointer());
		len = strlen(getStringPointer());
	}
	
	else if ( m_coffSymbol.getZeros() == 0 )
	{
		/* replace it in the string table  */
		/* it's an offset */
		offset = m_stringTable->replaceString(fd, m_coffSymbol.getStringTableOffset() ,getStringPointer());
		len = oldLen;
	}
	else
	{
		/* replace it in the actual symbol */
		offset = m_coffSymbol.getFileOffset();
		len = COFF_SYMBOL_NAME_LEN;
		/* the real string is here. in the symbol entry */
		m_coffSymbol.setString(getStringPointer());
	}	
	return SymbolString::m_writeNewStringToFile(fd, offset, len);
}	

/***********************************************************************/
/** @brief read a bunch of data from the current file position.
 * 
 * When reading a large amount of data a single read call
 * can not always read it al at once so we loop to read it all.
 * 
 * @param dstPtr		- where to put it
 * @param count		- number of bytes to read
 * 
 * @return
 * 	   - 1 - success
 * 	   - 0 - error
 ************************************************************************/
static int s_doRead(int fd, void  *dstPtr, const int count)
{
	int bytesLeft = count;
	int bytesReadThisTime;
	char *p = (char *)dstPtr;
	do
	{
		bytesReadThisTime = read(fd, p, bytesLeft);
		if ( bytesReadThisTime > 0)
		{
			// the read was a success 
			bytesLeft -= bytesReadThisTime;
			p += bytesReadThisTime;
		}
		else
		{
			/* failure. get out */
			return 0;
		}
		/* if we don't have all the data yet then do it again */
	} while (bytesLeft > 0);
	return 1;
}	
