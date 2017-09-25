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
/* $Id: lm_objfilecoff.h,v 1.3 2003/06/14 00:26:48 kmaclean Exp $
***********************************************************/
/**   @file cofffile.h
 *    @brief Definitions of COFF file structs
 *    @version $Revision: 1.3 $
 *
  ************************************************************/


#ifndef INCLUDE_COFFFILE_H
#define  INCLUDE_COFFFILE_H

#include "lm_objfilecommon.h"

/***********************************************************************
 * Coff file definitions
 ************************************************************************/

#define NUMBER_DIMENSIONS		4
#define COFF_SECTION_NAME_LEN	8	// length of a section name
#define COFF_SYMBOL_NAME_LEN	8   // length of a symbol name that is embedded in the symbol struct
#define	COFF_FILE_NAME_LEN		14  // length of a file name entry

typedef unsigned char	X_UINT8 ;
typedef signed char		X_INT8 ;
typedef unsigned short	X_UINT16 ;
typedef signed short	X_INT16 ;
typedef unsigned long	X_UINT32;
typedef signed long		X_INT32;

#ifdef PC
	/** magic number in the coff header for the intel x86/pentium  processor */
const X_UINT16 CoffMagicNumber = 0x0000014c;
#endif 

/* must not have structure padding */
#pragma pack(push,1)

/* Coff file header */
typedef struct {
	X_UINT16	f_magic;   // Magic number that defines whiuch processor this is for
	X_UINT16	f_nscns;   // Number of sections
	X_UINT32	f_timdat;  // timestamp
	X_UINT32	f_symptr;  // symbol table pointer
	X_UINT32	f_nsyms;   // number of symbols
	X_UINT16	f_opthdr;  // optional header
	X_UINT16	f_flags;
} COFF_FILE_HEADER;

/* generic coff optional header */
typedef struct {
	X_UINT16	magic;
	X_UINT16	version;
	X_UINT32	tsize;
	X_UINT32	dsize;
	X_UINT32	bsize;
	X_UINT32	entry;
	X_UINT32	text_start;
	X_UINT32	data_start;
} COFF_OPT_HDR;

/* coff file section header */
typedef struct {
    char  s_name[COFF_SECTION_NAME_LEN];	// section name
    X_UINT32 s_paddr;
    X_UINT32 s_vaddr;
    X_UINT32 s_size;                      // size of the section data
    X_UINT32 s_scnptr;                    // file offset for section data
    X_UINT32 s_relptr;                    // relocation info file offset
	X_UINT32 s_lnnoptr;                   // line number entrys file offset
    X_UINT16 s_nreloc;                    // number of relocation entries
     X_UINT16 s_nlnno;                     // number of line number entries
    X_UINT32 s_flags;
} COFF_SECTION_HEADER;

/* coff file symbol entry struct */
typedef struct {
    union      // the name of the symbol
	{
		char _n_name[COFF_SYMBOL_NAME_LEN]; // embedded name
		struct
		{
			/* if _n_zeros == 0 then _n_offset is the offset into the 
			 * string table. otherwise the string is in _n_name */
			X_UINT32 _n_zeroes;
			X_UINT32 _n_offset;
		} _n_n;
	} _name;
	X_UINT32 n_value;      // value of this symbol. Usually the offset into the section data for intitalized data
	X_INT16 n_scnum;       // which section is it in
	X_UINT16 n_type;       // the data type
	X_UINT8 n_sclass;      // class of this symbol
	X_UINT8 n_numaux;      // number of auxiliary entries
} COFF_SYMBOL;


/* Coff symbol auxiliary entry.
 * Don't know what all this is. We ignore it. */
typedef union
{
    char aux_file_name[COFF_FILE_NAME_LEN];
    struct
    {
		X_UINT32	tag_index;
        union
        {
            struct
            {
				X_UINT16	c_line_nbr;
				X_UINT16	size;
            } s;
			X_UINT32	func_size;
        } u1;
        union
        {
            struct
            {
				X_UINT32	line_ptr;
				X_UINT32	end_index;
            } s;
            X_UINT16 array_dim[NUMBER_DIMENSIONS];
        } u2;
		X_UINT16	high_size;
    } symbol;
	struct
    {
		X_UINT32	len;
		X_UINT16	nbr_rel_ents;
		X_UINT16	nbr_line_nbrs;
    } sec;
} COFF_AUX_SYMBOL;

typedef struct _coff_relocation {
    union {
        X_UINT32   r_virt_address;
        X_UINT32	r_reloc_count;  
    } u ;
    X_UINT32   r_symtbl_index;
    X_UINT16   r_type;
} COFF_RELOCATION;

#pragma pack(pop)

/*************************************************************/
/**  Coff File Header
 **************************************************************/
class CoffHeader
{
public:
	CoffHeader(int debugFlag) : m_debug(debugFlag), m_expectedMagic(CoffMagicNumber) {} ;
	~CoffHeader() {} ;
	int hRead(const int fd, const unsigned long offset);
	void print(void) 			const;
	void debugPrint(void) 		const { if (m_debug) print() ; };

	int getNumSections(void)	const { return m_data.f_nscns  ; } ;
	int getOptHeaderSize(void)	const { return m_data.f_opthdr ; } ;
	unsigned long getSymTableOffset(void)	const { return m_data.f_symptr ; } ;
	int getNumSyms(void)		const { return m_data.f_nsyms  ; } ;
	int getHeadderSize(void)	const { return sizeof(COFF_FILE_HEADER); } ;

private:
	CoffHeader()  : m_debug(0), m_expectedMagic(CoffMagicNumber) {} ;
	COFF_FILE_HEADER	m_data;			/* coff file header */
	int					m_debug;	   /** do we print debug messages */
	unsigned long 		m_fileOffset;  /** the file offset we read the data from */
	const X_UINT16		m_expectedMagic; /** the magic number we are expecting in the header */
};	

/*************************************************************/
/**  Coff File section Header
 **************************************************************/
class CoffSection
{
public:
	CoffSection(int debugFlag) : m_dataSize(0), m_sectionData(NULL), m_debug(debugFlag)  {} ;
	CoffSection() : m_dataSize(0), m_sectionData(NULL),  m_debug(0)  {} ;
	~CoffSection() {} ;
	int hRead(const int fd);
	void print(void) 				const;
	void debugPrint(void) 			const { if (m_debug) print() ; };
	void setDebug(const int debugFlag)	  { m_debug = debugFlag ; };
	int getDebugFlag(void) 			const { return m_debug; };
	unsigned long getDataOffset(void)		const { return m_data.s_scnptr ;};
	
private:
	COFF_SECTION_HEADER		m_data;
	int						m_dataSize;	// how much raw data is there
	char *					m_sectionData;     // pointer to raw data
	int						m_debug;			/** do we print debug messages */
	unsigned long 			m_fileOffset;  /** the file offset we read the data from */
};	
/*************************************************************/
/**  Coff File section Header List
 **************************************************************/
class CoffSectionList
{
public:
	CoffSectionList(int debugFlag) : m_numSections(0), m_list(NULL), m_debug(debugFlag) {} ;
	~CoffSectionList() {} ;
	int hRead(const int fd, const unsigned long offset, const int numSections);
	void print(void) 			const;
	void debugPrint(void) 		const { if (m_debug) print() ; };
	
private:
	CoffSectionList() : m_numSections(0), m_list(NULL), m_debug(0) {} ;
	int					m_numSections;
	CoffSection	*		m_list;
	int					m_debug;			/** do we print debug messages */
	unsigned long 		m_fileOffset;  /** the file offset we read the data from */
};	
/*************************************************************/
/**  Coff File String table
 **************************************************************/
class CoffStringTable
{
public:
	CoffStringTable(int debugFlag) : 
		m_lenBytes(0), 
		m_data(NULL), 
		m_debug(debugFlag) 
	{} ;
	~CoffStringTable() {} ;
	int hRead(const int fd, const unsigned long offset);
	void print(void) 					const;
	void debugPrint(void) 				const { if (m_debug) print() ; };
	char * lookupString(unsigned long offset) 	const;
	int replaceString( int fd, unsigned long  tableOffset, const char *newString);
	
private:
	CoffStringTable() : 
		m_lenBytes(0), 
		m_data(NULL), 
		m_debug(0) 
	{} ;
	unsigned long		m_lenBytes;		/** length of the string table */
	char *				m_data;			/** the string table */
	int					m_debug;		/** do we print debug messages */
	unsigned long 		m_fileOffset;  /** the file offset we read the data from */
};	
class CoffFile;  /* forward */
/*************************************************************/
/**  Coff symbol entry
 **************************************************************/
class CoffSymbol
{
public:
	CoffSymbol(int debug) : m_debug(debug), m_offsetInFile(0), m_currentSymbolIndex(0)
	{
		memset (&m_data,0,sizeof(COFF_SYMBOL));
	} ;
	~CoffSymbol() {} ;
	int		hRead(const int fd);
	void	print(void) 				const  { print(DebugAll) ; };
	void	print(int debugLevel) 		const;
	void	debugPrint(void) 			const { if (m_debug) print(m_debug) ; };
	unsigned long getFileOffset(void) 			  { return m_offsetInFile;};
	int		getNumAuxEntries(void) 		const { return m_data.n_numaux; 	};
	void	setString(const char *string)	;
	unsigned long	getStringTableOffset(void) 	const { return m_data._name._n_n._n_offset ; };
	int		getZeros(void) 				const { return m_data._name._n_n._n_zeroes ; };
	const char *	getName(void) 		const { return m_data._name._n_name; };
	int		incrementSymbolCount(void) ;
	int		getSymbolNumber(void)		const {return m_currentSymbolIndex;	};

private:
	CoffSymbol()  :  m_debug(0), m_offsetInFile(0), m_currentSymbolIndex(0)
	{
		memset (&m_data,0,sizeof(COFF_SYMBOL));
	} ;
	COFF_SYMBOL		m_data;
	int				m_offsetInFile;
	int 			m_debug;
	int				m_currentSymbolIndex;
			   
};	
/*************************************************************/
/**  Coff symbol aux entry
 **************************************************************/
class CoffAuxSymbol
{
public:
	CoffAuxSymbol(int debug) : 
			m_debug(debug)
	{} ;
	~CoffAuxSymbol() {} ;
	int hRead(const int fd);
	void print(int debugLevel) 		const;
	void debugPrint(void) 			const { if ( m_debug ) print(m_debug) ; };

private:
	CoffAuxSymbol() :
			m_debug (0)
	{};

	COFF_AUX_SYMBOL		m_data;
	const int			m_debug;
			   
};	
/*************************************************************/
/** CoffRelocationEntry
 **************************************************************/
class CoffRelocationEntry
{
public:	
	CoffRelocationEntry(int debug) :
			m_debug(debug)
	{};
	~CoffRelocationEntry() {};
	
	int hRead(const int fd);
	void print(int debugLevel) 		const;
	void debugPrint(void) 			const { if ( m_debug ) print(m_debug) ; };
private:
	CoffRelocationEntry() :
			m_debug (0)
	{};
	COFF_RELOCATION		m_data;
	const int			m_debug;
}	;


/***********************************************************************/
 /** CoffSymbolString class
  * 
  * We first read all the strintg from the string table for processing.
  * Next ad we read symbols we can throw away any symbol that points into the 
  * string table because they have already been processed.
 ************************************************************************/
class CoffSymbolString : public SymbolString 
{
public:
	CoffSymbolString(CoffStringTable * stringTable,int debug, int fd,  int numSymbols, int symbolFileOffset);

	~CoffSymbolString() {};
	int setNextString(int * noMoreStrings) ;
	int replaceString(int fd, const char * newString);
	int getNumAuxEntries(void) 	const { return m_coffSymbol.getNumAuxEntries(); 	};
	int setFirstString(void);

private:
	CoffSymbolString() : m_fd(0), m_numSymbols(0), m_coffSymbol(0), SymbolString(0) {};
	
	CoffSymbol 			m_coffSymbol;
	unsigned long 		m_nextFileOffset;
	const int			m_numSymbols;
	
	int					m_stringFromTableOffset;

	CoffStringTable *	m_stringTable;
	const int			m_fd;
};

/*************************************************************/
/**  Coff File 
 **************************************************************/
class CoffFile : public CodeFile
{
public:
		
	CoffFile() : 
		m_fileOffset(0),
		m_baseOffset(0),
		m_debug(0) ,
		m_coffHeader(0),
		m_sectionList (0),
		m_stringTable(0),
		CodeFile(0,NULL)
	{ };
	
	CoffFile(int debugFlag,const char * fileName) : 
		m_fileOffset(0),
		m_baseOffset(0),
		m_debug(debugFlag),
		m_coffHeader(debugFlag),
		m_sectionList (debugFlag),
		m_stringTable(debugFlag),
		CodeFile(debugFlag, fileName)
	{};
	CoffFile(int debugFlag,int fd, const char * fileName) : 
		m_fileOffset(0),
		m_baseOffset(0),
		m_debug(debugFlag),
		m_coffHeader(debugFlag),
		m_sectionList (debugFlag),
		m_stringTable(debugFlag),
		CodeFile(debugFlag, fd, fileName)
	{};
	
	~CoffFile() { };
	/* set then file descriptor. Used it if the file is already open */ 
	virtual int hRead(void);

	virtual SymbolString * getFirstString(void);
	virtual void print(void) 					const ;
	virtual void debugPrint(void) 				const { if (m_debug) print() ; };
	CoffStringTable * getStringTable()				  { return &m_stringTable ;};
	int getNumSyms(void) 						const { return m_coffHeader.getNumSyms(); };
	unsigned long getSymTableFileOffset(void)	const { return m_coffHeader.getSymTableOffset() + m_baseOffset;};
	unsigned int getFileOffset(void) 			const { return m_fileOffset;};
	int readEntireFile(void);

protected:
	void setBaseOffset(unsigned long baseOffset)	  { m_baseOffset = baseOffset ; };

private:
	CoffHeader			m_coffHeader;                
	unsigned long 		m_fileOffset;
	unsigned long		m_baseOffset;		/** all offsets in the coff headers are relative to this */
	int					m_debug;			/** do we print debug messages */
	/* CoffOptHeader		m_optHeader; */
	CoffSectionList		m_sectionList;
	CoffStringTable		m_stringTable;
} ;


#endif 
