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
 /* $Id: flex_string_internal.h,v 1.1 2003/04/09 17:35:48 kmaclean Exp $ 
 ************************************************************************/
/** @file flex_string_internal.h
 *  @brief structs and types internal to the FLEX_STRING routines.
 * 	@warning Do not use this outside of the flex_string_xxx.c files.
 * 
 * @version $Revision: 1.1 $
 * 
 ************************************************************************/


#ifndef INCLUDE_FLEXSTRING_INTERNAL_H

#define INCLUDE_FLEXSTRING_INTERNAL_H

#include "flex_string.h"
/***********************************************************************/
/** Our pseudo string struct. 
 * Holds a string in either UTF8 or Wide. Our prefered format is UTF8 
 ************************************************************************/
typedef struct _FlexString
{
		/** handle for this string */
	FLEX_STRING_HANDLE	handle;
		/** what type of string is this.
		 * Don't think we really need this */
	FLEX_BOOL			isReadOnly;
	/** is this string InUse. We may keep a free list for
	 * faster allocation. */
	FLEX_BOOL			isInUse ;
	/** Was this string allocated on the heap or is it on the stack.  */
	FLEX_BOOL		   isAllocated;

	  	/* at any given time one of these two string pointers may be NULL.
		 * We will do lazy caculatino here and only create the other format 
		 * string as needed. Then we'll keep both formats around until
		 * one of them changes. At that time we'll free the other.  */
	/** utf8 format string */
	 struct _stringUtf8
	 {
			/** pointer to the actual bytes for the string. In UTF-8 format */
	 	FLEX_CHAR_UTF8 *	string;
			/** length in bytes of the buffer pointed to by string_ptr.
			 * Note this is not the same as stringByteLen. We may allocate more
			 * memory than necessary to make string operations more efficent */
		int		bufferByteLen;
			/** length of the string in bytes not including the terminating null */
		int		stringByteLen;
	 	
	 } utf8;	   
	 /** wide char format string */
	 struct _stringWide
	 {
	 		/** pointer to the actual bytes for the string. In UNICODE format */
	 	FLEX_CHAR_WIDE *	string;
			/** length in bytes of the buffer pointed to by string_ptr.
			 * Note this is not the same as stringByteLen. We may allocate more
			 * memory than necessary to make string operations more efficent */
		int		bufferByteLen;
			/** length of the string in bytes not including the terminating null */
		int		stringByteLen;
	 	
	 } wide;	   
		/** length of the string in chars including the terminating null */
	int		stringCharLen;
		/** the maximum size allowed for this string in chars */
	int		maxCharLen;
		/** The job this string is associated with */
	LM_HANDLE * 	job;
} FLEX_STRING;

	
/** gets the string pointer from a handle.
 * Returns on an error */
#define ERROR_RETURN_GET_HANDLE(job,handle, dstPtr)	{\
	if (i_getStringFromHandle(job, handle, dstPtr) != LM_NOERROR)  \
		return job->lm_errno;\
}


FLEX_ERROR i_getStringFromHandle(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_STRING **returnString);
#endif
