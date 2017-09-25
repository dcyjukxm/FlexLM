/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent 
 *
 *		to run on an unlicensed platform is a violation of your 
 *
 *		license agreement with Macrovision Corporation. 
 *
 *
 *****************************************************************************/
/*	
 *	Module: $Id: l_intelid.c,v 1.14 2003/04/18 23:48:04 sluu Exp $
 *	Function: l_intelid(job)
 *
 *	Description: FLEXlm equivalent of Unix gethostid()
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *
 *	Return:		(HOSTID *) 
 *
 *	M. Christiano
 *	9/3/89
 *	Last changed:  4/1/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_intelid.h"
#include "flex_file.h"

#if _x86_
/*
 * This definition is needed because "CPUID" is not yet recognized
 * by the compiler.
 */

#define CPUID __asm _emit 0x0F __asm _emit 0xA2

/************************************************************************
 *
 * get_psn  -  Performs the assembly language that reads to serial
 *             number from the processor.
 *
 ***********************************************************************/

#pragma optimize( "", off )

static int
get_psn( unsigned long *dwHigh, unsigned long *dwMid, unsigned long *dwLow )
{
  unsigned long enabled = 0;
  unsigned long hiPSN = 0;
  unsigned long midPSN = 0;
  unsigned long loPSN = 0;

	//
	// Make sure the processor supports the CPUID
	//

	enabled = 0;
	__asm
	{
		// Get EFLAGS into the EAX and ECX registers
		pushfd
		pop		eax
		mov		ecx, eax
		
		// Set bit 21 and put the value back into EFLAGS
		xor		eax, 200000h
		push	eax
		popfd

		// Verify that the bit got changed
		pushfd
		pop		eax
		xor		eax, ecx
		jz		CPUIDCHECKDONE

		// If changed, then set support flag
		mov		enabled, 1
	CPUIDCHECKDONE:
	}

	if( !enabled )
	{
#ifndef RELEASE_VERSION
                *dwHigh = 0xA2345678;
                *dwMid = 0xB2345678;
                *dwLow = 0xC2345678;
                return 1;
#else
		return 0;
#endif
        }

	//
	// Make sure the processor has Katmai support
	//

	enabled = 0;
	__asm
	{
		// Check for max CPUID param and vendor string
		MOV		EAX, 00H
		CPUID

		// Compare max CPUID param to value needed for feature flags (which is 1)
		CMP		EAX, 1
		JL		KNICHECKDONE

		// Make sure the vendor string matches "GenuineIntel"
		//CMP		EBX, 756E6547H
		//JNE		KNICHECKDONE
		//CMP		EDX, 49656E69H
		//JNE		KNICHECKDONE
		//CMP		ECX, 6C65746EH
		//JNE		KNICHECKDONE

		// Now get the feature flags
		MOV		EAX, 01H
		CPUID

		// Check the KNI flag
		AND		EDX, 02000000H
		CMP		EDX, 0
		JE		KNICHECKDONE

		// If we get here, then KNI are supported
		MOV		enabled, 1

	KNICHECKDONE:
	}

	if( !enabled )
	{
#ifndef RELEASE_VERSION
                *dwHigh = 0xA2345678;
                *dwMid = 0xB2345678;
                *dwLow = 0xC2345678;
                return 1;
#else
		return 0;
#endif
        }

	//
	// Get the state and value of PSN
	//

	enabled = 0;
	__asm
	{
		// Get the feature flags
		MOV		EAX, 01H
		CPUID

		// Check to see if it's disabled
		AND		EDX, 00040000H
		CMP		EDX, 0
		JE		PSNCHECKDONE

		// If we get here then PSN is enabled
		MOV		enabled, 1

		// Get the high 32 bits
		MOV		hiPSN, EAX
		
		// Get the rest of the bits
		MOV		EAX, 03H
		CPUID
		MOV		midPSN, EDX
		MOV		loPSN, ECX

	PSNCHECKDONE:
	}

	if( !enabled )
	{
#ifndef RELEASE_VERSION
                *dwHigh = 0xA2345678;
                *dwMid = 0xB2345678;
                *dwLow = 0xC2345678;
                return 1;
#else
		return 0;
#endif
        }

	//
	// Return the status and value of PSN
	//

	*dwHigh = hiPSN;
	*dwMid = midPSN;
	*dwLow = loPSN;

	return 1;
}

#pragma optimize( "", on )

static void
ENDget_psn()
{
}

/************************************************************************
 *
 * load_id_info  -  Copies the data to the host id structure.
 *
 ***********************************************************************/

static void
load_id_info( HOSTID *idptr, unsigned long high,
				unsigned long mid, unsigned long low )
{
        idptr->id.intel96[0] = low;
        idptr->id.intel96[1] = mid;
        idptr->id.intel96[2] = high;

        idptr->type = HOSTID_INTEL96;
}

/************************************************************************
 *
 * Stub and loads routines to support multiple processors without
 * having different code for NT and Win95 (which doesn't ever have
 * more than one processor).
 *
 ***********************************************************************/

static HANDLE kernel32_handle = NULL;
static BOOL (WINAPI * ptr_GetProcessAffinityMask)(HANDLE, DWORD*, DWORD*) = NULL;
static BOOL (WINAPI * ptr_SetProcessAffinityMask)(HANDLE, DWORD) = NULL;

static BOOL WINAPI
stub_GetProcessAffinityMask( HANDLE h, DWORD *m1, DWORD *m2 )
{
	*m1 = 1;
	*m2 = 1;
	return TRUE;
}

static BOOL WINAPI
stub_SetProcessAffinityMask( HANDLE h, DWORD m )
{
	return TRUE;
}

static void
affinity_mask_api_init()
{
	DWORD version;
	BOOL is_nt;

	/*
	 * See if this is NT or Win95.
	 */

	version = GetVersion();
	if( version < 0x80000000 )
		is_nt = TRUE;
	else
		is_nt = FALSE;

	/*
	 * We can't statically link to the AffinityMask API since it's
	 * not available; so we're going to dynamically bind to that call.
	 */

	if( is_nt )
	{
		kernel32_handle = LoadLibrary( "kernel32.dll" );
		if( kernel32_handle )
		{
			ptr_GetProcessAffinityMask = (BOOL (WINAPI *)(HANDLE,DWORD*,DWORD*))
				GetProcAddress(
					kernel32_handle,
					"GetProcessAffinityMask" );

			ptr_SetProcessAffinityMask = (BOOL (WINAPI *)(HANDLE,DWORD))
				GetProcAddress(
					kernel32_handle,
					"SetProcessAffinityMask" );
		}
	}

	/*
	 * If for any reason we couldn't get the API, (like we're not
	 * an NT box) we'll just assign the API to our stubs.
	 */

	if( NULL == ptr_GetProcessAffinityMask )
	{
		ptr_GetProcessAffinityMask = stub_GetProcessAffinityMask;
		ptr_SetProcessAffinityMask = stub_SetProcessAffinityMask;
	}

}

static void
affinity_mask_api_free()
{
	if( kernel32_handle )
	{
		FreeLibrary( kernel32_handle );
		kernel32_handle = NULL;
	}

	ptr_GetProcessAffinityMask = NULL;
	ptr_SetProcessAffinityMask = NULL;
}

/************************************************************************
 *
 * l_int_all  -  Gets a cpuid for each processor that this
 *                    process is allowed to run on.
 *
 ***********************************************************************/

HOSTID*
l_intel_all( LM_HANDLE_PTR job, LM_GET_PSN_PROC proc )
{
  unsigned long high, mid, low;
  DWORD proc_mask, sys_mask;
  DWORD mask;
  int i;
  int cpu_index;
  HOSTID *id_list = NULL;
  HOSTID **back_link = &id_list;

	/*
	 * Call the function for each processor.  Note the use of
	 * indirection so the code can work on systems that do
	 * not have the AffinityMask API.
	 */

	affinity_mask_api_init();

	(*ptr_GetProcessAffinityMask)(
			GetCurrentProcess(),
			&proc_mask,
			&sys_mask );

	cpu_index = 0;
	for( i = 0, mask = 0x01; i < (8 * sizeof(sys_mask)); i++, mask <<= 1 )
	{
		if( !(sys_mask & mask) )
			continue;

		(*ptr_SetProcessAffinityMask)(
					GetCurrentProcess(),
					mask );

		if ( 0 != (*proc)( &high, &mid, &low ) )
		{
		  HOSTID *this_id;

#ifdef CPUID_GENERATOR
			this_id = (HOSTID *)calloc(1, sizeof(HOSTID ));
#else
			this_id = l_new_hostid();
#endif /* CPUID_GENERATOR */
			load_id_info( this_id, high, mid, low );

			*back_link = this_id;
			back_link = &this_id->next;

			++cpu_index;
		}
	}

	(*ptr_SetProcessAffinityMask)(
			GetCurrentProcess(),
			proc_mask );

	affinity_mask_api_free();

	return id_list;
}

/************************************************************************
 *
 * l_intelid  -  Makes a HOSTID list of cpuid's using the unencoded
 *               version of the 
 *
 ***********************************************************************/

HOSTID*
l_intelid( LM_HANDLE_PTR job)
{
	return l_intel_all ( job, get_psn );
}

/****************************************************************************
 *
 * When enabled, this will generate a main program that can make an
 * encrypted version of the CPU ID function above.  This allows us to
 * use the function both unencrypted and encrypted in the same
 * source file.
 *
 ***************************************************************************/

#ifdef CPUID_GENERATOR

void FatalError( char *format, ... )
{
  va_list vArgs;

	va_start( vArgs, format );
	vfprintf( stderr, format, vArgs );
	exit( -1 );
}

void UsageThenQuit( char *name )
{
	fprintf( stderr, "\n    Intel CPU ID function obfuscator - Nov 1999\n" );
	fprintf( stderr, "    "COPYRIGHT_STRING(1999)"\n\n" );
	fprintf( stderr,
		"Usage: %s [-n base-variable-name] [-o output-file] [-inc include-file] [-nomask] [-h]\n",
		name );
	exit( 0 );
}



#define NEXT_ARG(x) if(++(x)>=argc) UsageThenQuit(argv[0])

void main( int argc, char **argv )
{
  unsigned char xor_mask[] = { 'g'-' ', 's'-' ', 'i'-' ' };
  unsigned long count;
  unsigned char *rdPtr;
  unsigned char *encBuffer;
  unsigned char *wrPtr;
  int maskOutput;
  FILE *fOut;
  FILE *fOutH;
  char *name;
  int parm;
  int i;
  int x;

	/*
	 * Process command line arguments.
	 */

	name = "l_gcspimx";
	fOut = stdout;
	fOutH = NULL;
	maskOutput = 1;

	i = 1;
	parm = 1;
	while( i < argc )
	{
		if( '-' == *argv[i] )
		{
			if( !strcmp( "-h", argv[i] ) )
				UsageThenQuit( argv[0] );
			else if( !strcmp( "-n", argv[i] ) )
			{
				NEXT_ARG( i );
				name = argv[i];
			}
			else if( !strcmp( "-o", argv[i] ) )
			{
				NEXT_ARG( i );
				fOut = fopen( argv[i], "w" );
				if( NULL == fOut )
					FatalError( "Can't open: %s\n", argv[i] );
			}
			else if( !strcmp( "-inc", argv[i] ) )
			{
				NEXT_ARG( i );
				fOutH = fopen( argv[i], "w" );
				if( NULL == fOutH )
					FatalError( "Can't open: %s\n", argv[i] );
			}
			else if( !strcmp( "-nomask", argv[i] ) )
				maskOutput = 0;
		}
		else
		{
			++parm;
		}
		++i;
	}

	/*
	 * First, read the routine into a buffer and perform
	 * a simple XOR obfuscation.  Note that this masking
	 * can be disabled (good for debugging).
	 */

	count = (unsigned long) ((char*) ENDget_psn - (char*) get_psn);

	encBuffer = malloc( count + 10 );
	if( !encBuffer )
		FatalError( "Can't malloc %d bytes\n", count + 10 );

	if( maskOutput )
	{
		rdPtr = (unsigned char*) get_psn;
		wrPtr = encBuffer;
		for( i = 0; i < (int) count; i++ )
		{
			*wrPtr = *rdPtr++;
			*wrPtr++ ^= xor_mask[i % sizeof(xor_mask)];
		}
	}

	/*
	 * Write out this obscured function.  Note that the first
	 * three bytes are the length.
	 */

	fprintf( fOut, "unsigned char %s[] = {\n", name );
	fprintf( fOut, "    0x%02x, 0x%02x, 0x%02x, ",
			(unsigned char) ((count >> 16) & 0xff),
			(unsigned char) ((count >>  8) & 0xff),
			(unsigned char) ((count >>  0) & 0xff) );
	x = 3;
	i = count;
	rdPtr = encBuffer;
	while( 0 < i )
	{
		if( 10 <= x )
		{
			fprintf( fOut, "\n    " );
			x = 0;
		}
		fprintf( fOut, "0x%02x%s", *rdPtr, (1 < i) ? ", " : "" );
		++rdPtr;
		++x;
		--i;
	}
	fprintf( fOut, "\n};\n\n" );
	fclose( fOut );

	/*
	 * Generate an include file if desired.
	 */

	if( NULL != fOutH )
	{
		fprintf( fOutH,
   "/*\n * AUTO GENERATED - Include file for obfuscated Intel CPU ID function.\n */\n\n"
   "typedef int (*GET_PSN_PROC)( unsigned long*, unsigned long*, unsigned long* );\n"
   "extern unsigned char %s[%d];\n",
			name,
			count );
		fclose( fOutH );
	}

	exit( 0 );
}

#endif /* CPUID_GENERATOR */


#else  /* _x86_ */

/*void main()
{
	printf(" WARNING: The cpu id function has not been ported for ia64!" );
}
*/
#endif /* _x86_ */

