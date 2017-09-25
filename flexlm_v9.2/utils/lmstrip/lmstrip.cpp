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
/* $Id: lmstrip.cpp,v 1.8.2.2 2003/06/24 18:15:19 kmaclean Exp $
***********************************************************/
/**   @file lmstrip.cpp
 *    @brief  the new lmstrip for windows written in C++
 *    @version $Revision: 1.8.2.2 $     
 * 
 * This version of lmstrip reads the object file in it's native format 
 * Searches for symbols that match the list of symbols we wish to strip
 * and then replaces the symbol name with a random string of chars.
 * 
 * As of May 1, 2003 the following formats are supported:
 * 		Windows: EXE LIB OBJ
 * 
 * One of the many issues with the old version of lmstrip was that it
 * changed the length of the strings truncating each to not more than 
 * six characters. There are situations where this will not work and 
 * will cause link errors. This version creates random strings that 
 * are the same size as the original.
 *
  ************************************************************/
#include <string.h>
#include "lm_objfilecommon.h"
#include "lm_objfilewinpe.h"
#include "lm_strip.h"
#include "lmclient.h"

static int  replaceStrings(L_STRIP_OPTIONS *options, CodeFile *objFile);
static int  printStrings(L_STRIP_OPTIONS * options, CodeFile *objFile);
static int readObjFile(CodeFile *objFile);
static void doPrintSymbol(const char * str);
static int  readOptionsFile(L_STRIP_OPTIONS * options);
static void Usage(char *progmanName, char *string);
static void checkOptions(L_STRIP_OPTIONS *o);
static void printCopyright(L_STRIP_OPTIONS * options, char * progName);
static void parseCommandLine(L_STRIP_OPTIONS * options, char argc, char **argv);

/***********************************************************************/
/** @brief Main routine for lmstrip
 ************************************************************************/
int main(int argc, char **argv)
{
	CodeFile * 		objFile  = NULL;
	L_STRIP_OPTIONS	options;
	int				ok = 1;

	parseCommandLine(&options, argc, argv);
	printCopyright(&options, options.progName);

	if ( options.listSymbols )
		listNames(&options);   /* list the symbols in the internal tables */

	if ( options.verbose && options.fileName != NULL)
	{
		fprintf(stdout, "Stripping file: %s\n", options.fileName);
		if ( ! options.readMapFile && !options.writeMapFile)
			fprintf(stdout, "No map file used\n");
	}
	
	if ( options.readMapFile )
		ok = read_mapfile(&options, options.mapFileName);  /* read the map file  */
	
	if ( ok && options.optionsFile )
		ok = readOptionsFile(&options); /* read the options file */
	
	/* yes we randomize after reading the map file. this will only
	 * touch symbols that were not read from the map file  */
	if ( ok && ! options.listSymbols)
	{
		randomizeAll(&options);
		if ( options.writeMapFile )
			ok = write_mapfile(&options, options.mapFileName);
		if ( ok  )
		{
			objFile = CodeFile::getFileObject(options.debugFlags, options.fileName, options.progName);
			if ( objFile != NULL )
			{
				if ( (ok = objFile->fileOpen()) != 0 )
				{
					if ( ( ok = objFile->hRead()) != 0)
					{
						if ( options.printSymbols )
							ok = printStrings(&options,objFile);
						else if ( options.debugRead )
							ok = readObjFile(objFile);
						else 
							ok = replaceStrings(&options,objFile);
					}	
					objFile->fileClose();
				}
				if ( objFile != NULL )
					delete objFile;
			}
		}
		if (! ok )
		{
			if ( errno  )
				perror(options.fileName);
			if ( ! options.quiet) 
				fprintf(stderr,"%s: Failed\n",options.progName);		
		}
		else if ( ! options.quiet && (options.random || options.zeros))
			fprintf(stderr,"%s: Done\n",options.progName);		
	}

	/* success or failure */   
	return (ok) ? 0 : 1 ;
}
/***********************************************************************/
/** @brief Parse the command line options
 * 
 * @param options  options struct to fill in from the command line args
 * @param argc		the argc from main
 * @param argv		the argv from main
 * 
 ************************************************************************/
static void parseCommandLine(L_STRIP_OPTIONS * options, char argc, char **argv)
{
	char *pp;

	memset(options, 0, sizeof(L_STRIP_OPTIONS));

	/*  set the program name and strip the leading path */
	pp = *argv;
	pp += strlen(pp) - 1;
	while ( pp > *argv )
	{
		if ( *pp == '\\' ||
			*pp == '/' || 
			*pp == ':' )
		{
			pp++;
			break;
		}
		pp--;
	}
	options->progName = pp;
	argc--;
	argv++;
	if ( !argv || !*argv )
		Usage(options->progName,NULL);
	while ( argc > 0)
	{
		if ( **argv == '-' )
		{
		
			switch ( *((*argv) + 1) )
			{
				case 'p':
					/* print the symbols that are candidates for stripping.
					 * Only prints symbols that are not in the list.
					 * This includes the randomizes symbols read from the 
					 * map file */
					options->printSymbols = PrintSymbolsTypeNotInList;
					switch ( *((*argv) + 2) )
					{
						/* hidden args for debugging */
						case 'a':	/* print all symbols */
							options->printSymbols = PrintSymbolsTypeAll;
							break;
						case 'i' :	/* print only symbols from the internal tables */
							options->printSymbols = PrintSymbolsTypeOnlyInList;
							break;
					}		
					break;
				case 'l' :	
					/* Just list the internal or external symbols to be stripped. */
					options->listSymbols = 1; 
					break;
				case 'e' :	
					/* don't strip external symbols */
					options->noExternal = 1;	
					break;
				case 'i' :	
					/* don't strip internal symbols */
					options->noInternal = 1;	
					break;
				case 'z' :	
					/* replace the strings with zeros instead of a random name */
					/* Zeros can only be used on a fully linked executable
					 * because putting zeros in causes the length of the 
					 * strings to change and this will cause link errors
					 * even if those symbols are not used in the link.
 					 */
					options->zeros = 1;		
					options->random = 0;
					break;
				case 'r' :
					/* strip to a random string */	
					options->random = 1;
					break;
				case 'o' :
					/* next option may be the map file name*/
					if ( argc > 1 && **(argv + 1) != '-')
					{
						argc--;
						argv++;
						options->optionsFile = *argv;
					}
					else
					{
						Usage(options->progName,"Expected file name with -o");
					}	
					break;
				case 'm' :	
					switch ( *((*argv) + 2) )
					{
						case 'r' :		/* read the map file */
							options->readMapFile = 1;
							if ( *((*argv) + 3) == 'w' )
								options->writeMapFile = 1;
							/* next option may be the map file name*/
							if ( argc > 1 && **(argv + 1) != '-')
							{
								argc--;
								argv++;
								options->mapFileName = *argv;
							}
							break;
						case 'w' :		/* write the map file */
							options->writeMapFile = 1;
							if ( *((*argv) + 3) == 'r' )
								options->readMapFile = 1;
							/* next option may be the map file name*/
							if ( argc > 1 && **(argv + 1) != '-')
							{
								argc--;
								argv++;
								options->mapFileName = *argv;
							}
							break;
						default:	
							fprintf(stderr,"Unknown option '%s'\n", *argv);
							Usage(options->progName, NULL);
					}
						  
					/* map file */
					break;
				case 'd' :
					/* debugging flags passed to the CodeFile class constructor */
					if ( *((*argv) + 2) == 'r')
					{
						/* just read the object file don't do any processing */
						options->debugRead = 1;
					}	
							
					argv++;
					argc--;
					if ( argc <= 0 )
					{
						Usage(options->progName,"Expected number following '-d'");
					}
					options->debugFlags = atoi(*argv);	
					break;
				case 'v' :
					/* verbose */	
					options->verbose = 1;
					break;
				case 'q' :
					options->quiet = 1;
					break;	 
				case 'u' :
					options->noUser = 1;		/** don't strip user symbols  */
 					break;

				default:	
					fprintf(stderr,"Unknown option '%s'\n", *argv);
					Usage(options->progName,NULL);
			}
		}
		else
		{
			if ( options->fileName )
			{
				/* already have a file name */
				fprintf(stderr, "Only one file name may be specified: %s\n",
					options->fileName);
				Usage(options->progName, NULL);
			}
			/* assume it's a file name */
			options->fileName = *argv;
		}	
		argv++;
		argc--;
	}
	/* set the default map file name */
	if ( options->mapFileName == NULL )
		options->mapFileName = DEFAULT_MAPFILE_NAME;
	/* validate the command line options */
	checkOptions(options);
}	
/***********************************************************************/
/** @brief Replace strings in the exe or lib file
 *                  
 * @param options  options to use
 * @param objFile  Object representing the object file
 *
 * @return
 * 		- 1 on success
 * 		- 0 on error
 ************************************************************************/
static int replaceStrings(L_STRIP_OPTIONS * options, CodeFile *objFile)
{
	SymbolString *		objString = objFile->getFirstString();
	REPLACEMENT_STRING 	newString;
	int					ok = 1;
	int 				noMoreStrings;
	
	if (objString == NULL ||  ! objString->isValid() )
		return 0;
	do
	{
		if ( findString(options, objString->getStringPointer(), &newString ))
		{
			char bigString[2000 + 1]; /* hack */
			/* The new string may only replace a portion of the old string.
			 * Build the new string to set from the old string.
			 *  */
			strncpy(bigString,objString->getStringPointer(), 2000);
			strncpy(&bigString[newString.offset], newString.string, strlen(newString.string));

			if ( options->verbose )
			{
				fprintf(stdout,"Replacing '%s' with '%s'\n",
					objString->getStringPointer(),bigString);
			}
			if ( ! objString->replaceString(objFile->getFd(), bigString))
			{
				fprintf(stderr,"Failed to replace '%s'\n",objString->getStringPointer());
				ok = 0;
				break;
			}	
		}
		ok = objString->setNextString(&noMoreStrings);
	} while (ok && ! noMoreStrings);
 	delete objString;
	return ok;
}	

/***********************************************************************/
/** @brief print symbols from the file that may need to be stripped
 *
 * @param options  options to use
 * @param objFile  the object file to read
 *
 * @return                                
 * 		- 1 success
 * 		- 0 failure
 ************************************************************************/
static int printStrings(L_STRIP_OPTIONS * options, CodeFile *objFile)
{
	SymbolString *objString = objFile->getFirstString();
	int ok = 0;
	int noMoreStrings;
	
	if (objString == NULL ||  ! objString->isValid() )
		return 0;
	do
	{
		const char * str = objString->getStringPointer();

		if ( options->printSymbols == PrintSymbolsTypeOnlyInList )
		{
			REPLACEMENT_STRING newString;

			if ( findString(options, str, &newString))
			{
				/* found the name in the list so we may want to print it.*/
				doPrintSymbol(str);
			}
		}
		else if ( options->printSymbols == PrintSymbolsTypeNotInList )
		{
			if (! findNPBoth(options, str))
			{
				/* Didn't find the name in the list so we may want to print it.*/
				doPrintSymbol(str);
			}
		}
		else if ( options->printSymbols == PrintSymbolsTypeAll )
		{
			doPrintSymbol(str);
		}
		ok = objString->setNextString(&noMoreStrings);
	} while (ok && ! noMoreStrings);
 	delete objString;
	return ok;
}
/***********************************************************************/
/** @brief A debug routing that just reads all the string objects in the file
 * 
 * This will cause the entire file to be read. This is used with the debug flags
 * to print out the structs from the obj file
 *
 * @param objFile  the object file to read
 *
 * @return                                
 * 		- 1 success
 * 		- 0 failure
 ************************************************************************/
static int readObjFile(CodeFile *objFile)
{
	int ok = 0;
	int noMoreStrings;
	SymbolString *objString = objFile->getFirstString();
	
	if (objString == NULL ||  ! objString->isValid() )
		return 0;
	do
	{
		ok = objString->setNextString(&noMoreStrings);
	} while (ok && ! noMoreStrings);
 	
	delete objString;
	return ok;
}
/***********************************************************************/
/** @brief Do the real rork to print a symbol string
 *
 * @param str  the string to print
 ************************************************************************/
static void doPrintSymbol(const char * str)
{
	if ( *str == '_')
		str++;
	if ( isalpha(*str) && (*str != '$' && *str != '.'))
	{
		if ( strncmp(str,"lc_", 3) == 0 || 
			 strncmp(str,"l_",  2) == 0 ||
			 strncmp(str,"lm_", 3) == 0 ||
			 strncmp(str,"lp_", 3) == 0 ||
			 strncmp(str,"f_",  2) == 0 ||
			 strncmp(str,"fl_", 3) == 0 ||
			 strncmp(str,"sb_", 3) == 0 ||
			 strncmp(str,"ls_", 3) == 0 ||
			 strncmp(str,"flex",4) == 0 ||
			 strncmp(str,"FLEX",4) == 0 ) 
		{
			fprintf(stdout, "*  %s\n",str);
		}
		else
			fprintf(stdout, "   %s\n",str);
	}
}
/***********************************************************************/
/** @brief Read and process the options file
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
static int readOptionsFile(L_STRIP_OPTIONS * options)
{
	FILE *	fp;
	char	buf[257];	
	int		line = 0;	
	int		status = 1; 

	if ( ! options->quiet )
		fprintf(stdout,"Reading options file: %s\n", options->optionsFile);

	if ( (fp = fopen(options->optionsFile, "r")) == NULL )
	{
		perror(options->optionsFile);
		return 0;
	}

	while ( status && fgets(buf,256,fp) )
	{
		char *	p = buf;
		char *	symbol;
		char *	end;
		
		line++;
		/* strip leading white space */
		while ( *p && *p == ' ' || *p == '\t' )
			p++;
		
		if ( *p != '+' && *p != '-')
		{
			fprintf(stderr,
				"Options file Error: %s  line: %d  Expected either '-' or '+'\n", 
				options->optionsFile, line);
			status = 0;
			break;
		}

		/* skip blanks after the + or - */
		symbol = p + 1;
		while ( *symbol && (*symbol == ' ' || *symbol == '\t' ))
			symbol++;

		/* now strip everything after the end of the symbol name*/
		end = symbol;
		while ( *end && *end != ' ' && *end != '\t'  && *end != '\n'  && *end != '\r')
			end++;
		*end = '\0';
		
		if ( ! *symbol )
		{
			fprintf(stderr,
				"Options File Error: %s  line: %d  Missing symbol name line %d\n",
				options->optionsFile,line);
			status = 0;
			break;
		}
		if ( *p == '+' )
			status = addSymbol(options, symbol, NULL);
		else if ( *p == '-' )
			status = removeSymbol(options, symbol);
	}
	fclose(fp);
	return status;
}	
/***********************************************************************/
/** @brief Print a usage message and exit
 ************************************************************************/
static void Usage(char * programName, char *string)
{
	if ( string != NULL )
		fprintf(stderr,"Error: %s\n\n",string);
	printCopyright(NULL, programName);
	fprintf(stderr,"Usage: %s [-e] [-i] [-v] [-q] -[r|p|l]  [-m[rw] [mapfile]] [-o options file]  file\n", programName);
/*	fprintf(stderr,"  -z         Strip the symbols to the empty string instead of a random string\n");
 * We are not documenting the -z option 
 * */
	fprintf(stderr,"  -r         Strip the symbols to a random string (default)\n");
	fprintf(stderr,"  -p         Print symbols from the object file that are not in the list \n");
	fprintf(stderr,"             of symbols to be stripped.\n");
	fprintf(stderr,"  -l         List symbol names from the internal table for symbols that will be stripped\n");

	fprintf(stderr,"  -e         Do not strip external symbols\n");
	fprintf(stderr,"  -i         Do not strip internal symbols\n");
	fprintf(stderr,"  -v         Verbose output\n");
	fprintf(stderr,"  -q         Quiet output\n");
	fprintf(stderr,"  -mr  name  Read the named map file.\n");
	fprintf(stderr,"  -mw  name  Write the named map file.\n");
	fprintf(stderr,"  -mrw name  Read and then update named map file. \n"
				   "             If 'name' is omitted then 'lmstrip.map' is used\n");
	fprintf(stderr,"  -o   name  Read additional symbol names to be stripped or not stripped from 'name'\n"
				   "             Names in the file are of the form:\n"
				   "               +symbol   add the symbol to the list of user symbols to be stripped\n"
				   "               -symbol   remove the symbol from the list of symbols to be stripped\n");
	exit (1);
}	
/***********************************************************************/
/** @brief check the options to ensure nothing conflicts
 ************************************************************************/
static void checkOptions(L_STRIP_OPTIONS *o)
{
	if (! o->listSymbols && ! o->printSymbols  && ! o->zeros)
	{
		/* if the list pr print options are not set then default
		to random*/
		o->random = 1;
	}

	if ( o->random && o->zeros )
		Usage(o->progName, "Error: -r and -z can not be used together");
	if ( o->zeros && (o->readMapFile || o->writeMapFile))
		Usage(o->progName, "Error: -z and -mr or -mw can not be used together");
	if ( o->fileName == NULL && ! o->listSymbols && !o->printSymbols)
		Usage(o->progName, "Error: No file name specified");
	if ( o->optionsFile == NULL)
		o->noUser = 1;         /* no options file implies no user */
}

/***********************************************************************/
/** @brief Print the copyright and version string
 *
 * @param	options  the options to use
 ************************************************************************/
static void printCopyright(L_STRIP_OPTIONS * options, char * progName)
{
	if ( options == NULL || ! options->quiet )
	{
		fprintf(stdout, "%s version %s\n", progName,FLEXLM_VERSION_REVISION_STRING);
		fprintf(stdout, "%s\n", COPYRIGHT_STRING(2002));
	}
}	
