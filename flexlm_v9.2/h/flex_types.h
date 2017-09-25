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
/* $Id: flex_types.h,v 1.1 2003/04/09 17:35:40 kmaclean Exp $
***********************************************************/
/**   @file flex_types.h
 *    @brief FLEXlm specific types
 *    @version $Revision: 1.1 $
 *
  ************************************************************/

#ifndef INCLUDE_FLEX_TYPES_H
#define INCLUDE_FLEX_TYPES_H

	/** The FLEXlm UTF8 char type */
typedef unsigned char		FLEX_CHAR_UTF8	;
	/** The FLEXlm UNICODE (wide) char type */
typedef unsigned short		FLEX_CHAR_WIDE	;

#if 0
/*************************************************************
 *  kmaclean 4/4/2003 */
/** @todo We'll need to add these later when we have time to go through
 * every platform to set the proper sizes
 **************************************************************/
	/** The Flexlm 1 byte signed integer type */
typedef signed char			FLEXINT8; 
	/** The FLEXlm 1 byte unsigned integer type */
typedef unsigned char		FLEXUINT8;
	/** The FLEXlm 2 byte signed integer type */
typedef signed short		FLEXINT16;
	/** The FLEXlm 2 byte unsigned integer type */
typedef unsigned short		FLEXUINT16;
	/** The FLEXlm  4 byte signed integer type */
typedef signed long			FLEXINT32;
	/** The FLEXlm  4 byte unsigned integer type */
typedef unsigned long		FLEXUINT32;
	/** The FLEXlm  8 byte signed integer type */
typedef signed _int64		FLEXINT64;
	/** The FLEXlm  8 byte unsigned integer type */
typedef unsigned _int64		FLEXUINT64;
#endif 


/** FLEXlm Boolean type. */
typedef unsigned int FLEX_BOOL;
/** FLEXlm boolean true @ref FLEX_BOOL */
#define FLEX_TRUE	1
/** FLEXlm boolean FALSE @ref FLEX_BOOL */
#define FLEX_FALSE	0


#endif 
