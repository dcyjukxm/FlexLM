/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_dll_ver.c,v 1.11 2003/01/14 21:43:21 kmaclean Exp $
 *
 *	Function: 
 *
 *	Description: 
 *
 *	Parameters:	
 *
 *	J. Friis
 *	9/2/98
 *
 *	Last changed:  9/2/98
 *
 */


#include "lmachdep.h"
#include "lmclient.h"
#include <windows.h>
#include <stdio.h>
//#pragma hdrstop
#include "dll_ver.h"


//
// Dump the imports table (the .idata section) of a PE file
//
void DumpImportsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_IMPORT_DESCRIPTOR importDesc;
    PIMAGE_SECTION_HEADER pSection;
    DWORD importsStartRVA;
    INT delta = -1;
	char* dll_str;
	char* dll;
	char* version;
	char copy[200];

	sprintf(copy, COPYRIGHT_STRING(1988));

    // Look up where the imports section is (normally in the .idata section)
    // but not necessarily so.  Therefore, grab the RVA from the data dir.
    importsStartRVA = pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if ( !importsStartRVA )
        return;

    // Get the IMAGE_SECTION_HEADER that contains the imports.  This is
    // usually the .idata section, but doesn't have to be.
    pSection = GetEnclosingSectionHeader( importsStartRVA, pNTHeader );
    if ( !pSection )
        return;

    delta = (INT)(pSection->VirtualAddress-pSection->PointerToRawData);
    
    importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (importsStartRVA - delta + base);
                
    while ( 1 )
    {
        // See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
        if ( (importDesc->TimeDateStamp==0 ) && (importDesc->Name==0) )
            break;

        dll_str=(PBYTE)(importDesc->Name) - delta + base;
		
		
		if ( !_strnicmp (dll_str, "lmgr", 4) )
		{	
			dll = _strlwr(_strdup(dll_str));
			version = get_version(dll);
			if (version)
				printf("FLEXlm %s %s, %s \n", version, dll, copy);
			else
				printf("Unknown FLEXlm library");
		
		}
	    importDesc++;   // advance to next IMAGE_IMPORT_DESCRIPTOR		
    		
	}
}


char* get_version(char* str)
{
	char* ret = "";
	
	if (!(strcmp(str,"lmgr.dll"))||!(strcmp(str,"lmgr32.dll")))
		ret="4.1";
	else if (!(strcmp(str,"lmgr164a.dll"))||!(strcmp(str,"lmgr324a.dll")))
		ret="4.1a";
	else if (!(strcmp(str,"lmgr165a.dll"))||!(strcmp(str,"lmgr325a.dll")))
		ret="5.0";
	else if (!(strcmp(str,"lmgr165b.dll"))||!(strcmp(str,"lmgr325b.dll")))
		ret="5.11";
	else if (!(strcmp(str,"lmgr165c.dll"))||!(strcmp(str,"lmgr325c.dll")))
		ret="5.12";
	else if (!(strcmp(str,"lmgr166a.dll"))||!(strcmp(str,"lmgr326a.dll")))
		ret="6.0";
	else if (!(strcmp(str,"lmgr166b.dll"))||!(strcmp(str,"lmgr326b.dll")))
		ret="6.1";
	else if (!strcmp(str,"lmgr327a.dll"))
		ret="7.0";
	else if (!strcmp(str,"LMGR327A.DLL"))
		ret="7.0";
	else if (!strcmp(str,"lmgr327b.dll"))
		ret="7.1";
	else if (!strcmp(str,"LMGR327B.DLL"))
		ret="7.1";

	return ret;

}

//
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER
//
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva,
                                                PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned i;
    
    for ( i=0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )
    {
        // Is the RVA within this section?
        if ( (rva >= section->VirtualAddress) && 
             (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }
    
    return 0;
}



//
// top level routine called from PEDUMP.C to dump the components of a PE file
//
void DumpExeFile( PIMAGE_DOS_HEADER dosHeader )
{
    PIMAGE_NT_HEADERS pNTHeader;
    DWORD base = (DWORD)dosHeader;
    
    pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader,
                                dosHeader->e_lfanew );

    // First, verify that the e_lfanew field gave us a reasonable
    // pointer, then verify the PE signature.
    __try
    {
        if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        {
            printf("Not a Portable Executable (PE) EXE\n");
            return;
        }
    }
    __except( TRUE )    // Should only get here if pNTHeader (above) is bogus
    {
        printf( "invalid .EXE\n");
        return;
    }
    

    DumpImportsSection(base, pNTHeader);
    printf("\n");
    

}



//
// Open up a file, memory map it, and call the appropriate dumping routine
//
void lm_dll_ver(LPSTR filename)
{
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase;
    PIMAGE_DOS_HEADER dosHeader;
    
    hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                    
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        printf("Couldn't open file with CreateFile()\n");
        return;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if ( hFileMapping == 0 )
    {
        CloseHandle(hFile);
        printf("Couldn't open file mapping with CreateFileMapping()\n");
        return;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if ( lpFileBase == 0 )
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        printf("Couldn't map view of file with MapViewOfFile()\n");
        return;
    }
    
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if ( dosHeader->e_magic == IMAGE_DOS_SIGNATURE )
    {
        DumpExeFile( dosHeader );
    }

    else
        printf("unrecognized file format\n");
    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
}



