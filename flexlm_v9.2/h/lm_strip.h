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
/* $Id: lm_strip.h,v 1.6.2.1 2003/06/19 23:21:14 kmaclean Exp $
***********************************************************/
/**   @file objfilecommon.h
 *    @brief common classes for reading object files.
 *    @version $Revision: 1.6.2.1 $
 * 
 * These classes are used by lmstrip to read the object file (lib or exe)
 * in it's native format. 
 ************************************************************/

#ifndef INCLUDE_L_LMSTRIP_H
#define  INCLUDE_L_LMSTRIP_H

/** our own boolean type */ 
typedef  unsigned int LMS_BOOL;
#define LMS_TRUE    1
#define LMS_FALSE	0

/** symbol printing options */
typedef int PrintSymbolsType;
const PrintSymbolsType PrintSymbolsTypeNotInList = 1;
const PrintSymbolsType PrintSymbolsTypeOnlyInList = 2;
const PrintSymbolsType PrintSymbolsTypeAll = 3;

/** Options used when stripping a file */
typedef struct _stripOptions
{
	int		zeros;			/** replace the strings with zeros instead of a random name */
	int		random;			/** replace strings with random chars */
	int		listSymbols;	/** Just list the internal or external symbols to be stripped. */
	PrintSymbolsType	printSymbols;	/** Print the symbols from the file that are candidates for stripping */
	
	int		noExternal;		/** don't strip external symbols */
	int		noInternal;		/** don't strip internal symbols */
	int 	noUser;			/** don't strip user symbols */
	
	int		writeMapFile;	/** write the map file */ 
	int		readMapFile;	/** read the map file */
	char *	mapFileName;	/** name of the map file */	
	
	char *	optionsFile;	/** name of the options file to be read */
	
	int		verbose;		/** verbose output */
	int		quiet;			/** quiet output */
	int		debugFlags;		/** debugging flags passed to the CodeFile class constructor */
	int		debugRead;		/** just read the object file. Don't do any processing */

	char *	fileName;		/** name of the object file to be stripped  */
	char *	progName;		/** name of this program.  */
	
} L_STRIP_OPTIONS;

/*************************************************************/
/**  Struct to hold a replacement string 
 **************************************************************/
typedef struct _replacement_string
{
	/** the replacement string */
	char * 	string;
	/** offset into the original string to start the replacement */
	int 	offset;
} REPLACEMENT_STRING;

#define DEFAULT_MAPFILE_NAME	"lmstrip.map"

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif 	

int findString(const L_STRIP_OPTIONS * options, const char *searchForString, REPLACEMENT_STRING * newString);
int findNPBoth(const L_STRIP_OPTIONS * options, const char * searchString);
void randomizeAll(L_STRIP_OPTIONS * options);
int write_mapfile(L_STRIP_OPTIONS*options,char * mapfile);
int read_mapfile(L_STRIP_OPTIONS *options, char * mapfile);
int addSymbol(L_STRIP_OPTIONS *options,char *symbol, char *randName);
int removeSymbol(L_STRIP_OPTIONS *options,char *symbol);
void listNames( L_STRIP_OPTIONS *options);

#if defined(c_plusplus) || defined(__cplusplus)
};
#endif 


#endif 
