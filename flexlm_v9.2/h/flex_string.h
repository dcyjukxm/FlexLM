/******************************************************************************
	    COPYRIGHT (c) 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.
 *****************************************************************************/
 /* $Id: flex_string.h,v 1.1 2003/04/09 17:35:55 kmaclean Exp $ 
 ************************************************************************/
/** @file flex_string.h
 *  @brief FLEXlm string to be used in support of internationalization and 
 * 			longer strings.
 * @version $Revision: 1.1 $
 * 
 * Conceptually this is an object but the reality of it is that we
 * will need to implement it in C.
 ************************************************************************/


#ifndef INCLUDE_FLEXSTRING_H
#define INCLUDE_FLEXSTRING_H

#include "flex_types.h"

/** generic handle used by callers to reference the @ref FlexString object*/
typedef unsigned int	FLEX_STRING_HANDLE ;	
/** Indicated the handle is not valid */
#define FLEX_STRING_HANDLE_NONE  ((FLEX_STRING_HANDLE ) 0)

/** What type of string to number or number to string conversion do 
 * we want to do */
typedef enum _FlexConvertType {
	FlexConvertUndefined = 0,	/** Conversion undefined. Ususally an error */
	FlexConvertAuto,			/** Automatic conversio. The routine will try to figure it out */ 
	FlexConvertHex,				/** conversion  to/from hex */
	FlexConvertHexZeroX,		/** conversion  to/from hex include the 0x */
	FlexConvertDecimal			/** conversion to/from decimal */
} FLEX_CONVERT_TYPE;

FLEX_ERROR l_flexStrNewEmpty	 (LM_HANDLE * job, FLEX_STRING_HANDLE *returnHandle);
FLEX_ERROR l_flexStrNewWide		 (LM_HANDLE * job, FLEX_CHAR_WIDE *string, int byteLen, FLEX_STRING_HANDLE *returnHandle);
FLEX_ERROR l_flexStrNewUtf8		 (LM_HANDLE * job, FLEX_CHAR_UTF8 *string, int byteLen, FLEX_STRING_HANDLE *returnHandle);
FLEX_ERROR l_flexStrcpyWide		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_WIDE *srcString, int maxByteLen  );
FLEX_ERROR l_flexStrcpyUtf8		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_UTF8 *srcString, int maxByteLen  );
FLEX_ERROR l_flexStrDelete		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle);
FLEX_ERROR l_flexStrlenBytesUtf8 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, size_t *returnLen);
FLEX_ERROR l_flexStrlenBytesWide (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, size_t *returnLen);
FLEX_ERROR l_flexStrlenChars	 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, size_t *returnLen);
FLEX_ERROR l_flexStrDupString	 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_STRING_HANDLE  *returnHandle);
FLEX_ERROR l_flexStrDupUtf8		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_UTF8 	**returnPointer );
FLEX_ERROR l_flexStrDupWide		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_WIDE 	**returnPointer );
FLEX_ERROR l_flexStrGetTmpUtf8	 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_UTF8	**returnPointer );
FLEX_ERROR l_flexStrGetTmpWide	 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_WIDE **returnPointer );
FLEX_ERROR l_flexStrItoa		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, int num , FLEX_CONVERT_TYPE convertType);
FLEX_ERROR l_flexStrAtoi 		 (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, int maxChars, FLEX_CONVERT_TYPE convertType, int *returnNum);
FLEX_ERROR l_flexStrcat			 (LM_HANDLE * job, FLEX_STRING_HANDLE dstHandle, FLEX_STRING_HANDLE srcHandle);
FLEX_ERROR l_flexStrcpy			 (LM_HANDLE * job, FLEX_STRING_HANDLE dstHandle, FLEX_STRING_HANDLE srcHandle);



#endif 
