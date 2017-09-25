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
 *	Module: $Id: dll_ver.h,v 1.2 2003/01/13 22:13:11 kmaclean Exp $
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

void DumpExeFile( PIMAGE_DOS_HEADER dosHeader );



#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (DWORD)(addValue))

char* get_version(char* str);

PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva,
                                                PIMAGE_NT_HEADERS pNTHeader);
