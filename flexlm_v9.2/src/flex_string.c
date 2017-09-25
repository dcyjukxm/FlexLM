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
/* $Id: flex_string.c,v 1.1 2003/04/09 17:36:01 kmaclean Exp $
 ************************************************************************/
 /** @file flexstring.c
  * @brief FLEXlm string struct and routines
  * @version $Revision: 1.1 $
  * 
  * FLEXlm string to be used in support of internationalization and generally
  * longer strings. This file contains the generic and the UTF8 routines.
  * 
  * Conceptually this is an object but the reality of it is that we
  * will need to implement it in C.
  * 
  * @note This is by no means complete. The struct is close to done but 
  * many more functions need to be defined.
  * 
  * */
#include "lmachdep.h"
#include "flex_types.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_privat.h"
#include "flex_string_internal.h"


	/** size of a temporary buffer used for conversions */
#define TMP_CHAR_BUF_SIZE	256

/***********************************************************************
 * Static function forwards
 ************************************************************************/
static FLEX_ERROR s_addStringToList(LM_HANDLE * job, FLEX_STRING *s, FLEX_STRING_HANDLE *returnHandle);
static FLEX_ERROR s_makeBufferSizeWide(LM_HANDLE * job, FLEX_STRING *s, int byteLen);
static FLEX_ERROR s_makeBufferSizeUtf8(LM_HANDLE * job, FLEX_STRING *s, int byteLen);
static FLEX_ERROR s_copyWide(LM_HANDLE * job, FLEX_CHAR_WIDE *dst, FLEX_CHAR_WIDE *src, int byteLen);
static FLEX_ERROR s_flexStrInitNew(LM_HANDLE * job, FLEX_STRING *s, FLEX_STRING_HANDLE *returnHandle);
static FLEX_ERROR s_copyUtf8(LM_HANDLE * job, FLEX_CHAR_UTF8 *dst, FLEX_CHAR_UTF8 *src, int maxByteLen);
static void s_clearWide(FLEX_STRING *s);
static FLEX_CHAR_UTF8 * s_itoxa(FLEX_CHAR_UTF8 *dstBuf, int bufLen, int inputNum, int * newLen);
static FLEX_CHAR_UTF8 * s_itoda(FLEX_CHAR_UTF8 *dstBuf, int bufLen, int inputNum, int * newLen);
static FLEX_CHAR_UTF8 * s_atox(FLEX_CHAR_UTF8 *inputString, int maxLen, int *num);
static FLEX_CHAR_UTF8 * s_atod(FLEX_CHAR_UTF8 *inputString, int maxLen, int *num);
static FLEX_ERROR s_verifyUtf8String(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle);

#include <assert.h>
/***********************************************************************/
/** runtime type checking to make sure all the Flex... types are 
 * the correct size
 *
 * This function will assert on an error. 
 * 
 * @todo Need to call FlexTypeCheck() from some init
 * function Perhaps lc_new_job()?
 ************************************************************************/
void FlexTypeCheck(void)
{
	assert(sizeof(FLEX_CHAR_UTF8) == 1);
	assert(sizeof(FLEX_CHAR_WIDE) == 2);
}	

/***********************************************************************/
/** @brief Allocate a new string object that is empty.
 * 
 * 	@param	job - the job associated with this string
 * 	@param	returnHandle	- on return the new string handle is placed here
 * 
 * 	@return standard error codes
 ************************************************************************/
FLEX_ERROR l_flexStrNewEmpty(LM_HANDLE * job, FLEX_STRING_HANDLE *returnHandle)
{
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnHandle)
	*returnHandle = FLEX_STRING_HANDLE_NONE;

	s = (FLEX_STRING *)l_malloc(job, (size_t)sizeof(FLEX_STRING));
	if ( s != NULL )
	{
		s->isAllocated = FLEX_TRUE;	/* memory was allocated for this string */

		if ((s_flexStrInitNew(job, s, returnHandle)) == LM_NOERROR)
		{   
			FLEX_ERROR ignoredErr;
			s->isInUse = FLEX_TRUE;
			s->job = job;
	
			/* add the string to the list so we can do a lookup on the handle later.
			 * Also sets the returnHandle and the handle member in the string  */
			ignoredErr = s_addStringToList(job, s, returnHandle); /* ignore the return value it's in job->lm_errno */
		}
	}
	
	return job->lm_errno;
}	

/***********************************************************************/
/**  @brief Allocate a new string object. Init with Utf8 string.
 * 
 * 	@param job - the job associated with this string
 * 	@param string	- a UTF-8 string to be used as the initial value.
 * 					a copy of 'string' will be made.
 * 					if string is NULL then an empty object will be created
 *  @param byteLen the maximum length of the 'string' passed in
 * 					terminating null not included
 * 	@param returnHandle	- on return the new string handle is placed here
 * 
 * 	@return standard error codes
 ************************************************************************/
FLEX_ERROR l_flexStrNewUtf8(LM_HANDLE * job, FLEX_CHAR_UTF8 *string, int byteLen, FLEX_STRING_HANDLE *returnHandle)
{
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnHandle)
	ERROR_RETURN_ZERO_CHECK(job,byteLen)
	
	*returnHandle = FLEX_STRING_HANDLE_NONE;
	if (l_flexStrNewEmpty(job,returnHandle) == LM_NOERROR)
	{
		if ( string != NULL)
		{
			if (l_flexStrcpyUtf8(job ,*returnHandle, string, byteLen) != LM_NOERROR )
			{
				/* could not set the string. free the String and return */
				/** @todo  what to do with job->lm_error here? */
				FLEX_ERROR ignoredError = l_flexStrDelete(job, *returnHandle);
			}
		}
	}
	return job->lm_errno;
}


/***********************************************************************/
/**  @brief Set the contents of the string to that passed in.
 * 
 * A copy is made of the srcString. 
 * 
 * @param job - the job associated with this string
 * @param srcString	- the src string buffer to be set. If NULL then 
 * 						the string will be set to the empty string
 * @param strHandle	-  the dst string handle
 * @param maxByteLen the maximum length of the 'string' passed in. 
 * 					terminating null not included
 * 
 * @return standard error codes
 * 
 *********************************************************************/
FLEX_ERROR l_flexStrcpyUtf8(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle,FLEX_CHAR_UTF8 *srcString, int maxByteLen )
{
	FLEX_STRING	*	s;
	int		srcByteLen = 0;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	
	if ( srcString != NULL )
	{
		FLEX_CHAR_UTF8 *	tmpSrcString = srcString;
		/* how long is the source string */
		for (	srcByteLen = 0 ; 
				*tmpSrcString != '\0' && srcByteLen <= maxByteLen; 
				tmpSrcString++, srcByteLen++ )
		{
			;
		}
	}
	/* byteLen may be zero which just sets the string to '\0'. 
	 * Less than zero means an error occured.
	 * 
	 * So if the dst string is not empty and the src string is empty then the 
	 * srcByteLen will be zero. This will cause the dst string to be
	 * truncated to "\0" and it's length set to zero */
	
	/* special case. If the length of hte string is zero but the string 
	 * pointer is not null we still need to allocate memory for the string.
	 * This ++ forces that.  */
	if ( srcString != NULL && srcByteLen == 0 )
		srcByteLen++;

	if (s_makeBufferSizeUtf8(job, s,srcByteLen) == LM_NOERROR)
	{
		if (s_copyUtf8(job, s->utf8.string, srcString, srcByteLen) == LM_NOERROR)
		{
			s->utf8.stringByteLen = srcByteLen;
		}	
	}
	/* lastly, since we changed the utf8 string we need to set the wide string 
	 * to '\0' if it exists. */
	s_clearWide(s);
	
	return job->lm_errno;
}
/***********************************************************************/
/**  @brief Add the contents of the string passed in to the FLEX_STRING
 * 
 * @param job - the job associated with this string
 * @param srcString	- the src string buffer to be concatinated. If NULL then 
 * 						the string will be set to the empty string
 * @param strHandle	-  the dst string handle
 * @param maxByteLen the maximum length of the 'string' passed in. 
 * 					terminating null not included
 * 
 * @return standard error codes
 * 
 *********************************************************************/
FLEX_ERROR l_flexStrcatUtf8(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle,FLEX_CHAR_UTF8 *srcString, int maxByteLen )
{
	FLEX_STRING	*	s;
	int				srcByteLen = 0;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_ZERO_CHECK(job, maxByteLen);
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	
	/* if there is a Wide string ensure there is a Utf8 string */
	s_verifyUtf8String(job, strHandle);

	if (s->utf8.stringByteLen == 0)
	{
		/* dst string is empty so do a copy */
		return l_flexStrcpyUtf8(job, strHandle,srcString, maxByteLen );
	}	

	if ( srcString != NULL )
	{
		FLEX_CHAR_UTF8 *	tmpSrcString = srcString;
		/* how long is the source string */
		for (	srcByteLen = 0 ; 
				*tmpSrcString != '\0' && srcByteLen <= maxByteLen; 
				tmpSrcString++, srcByteLen++ )
		{
			;
		}
	}
	if ( srcByteLen > 0 )
	{
		int newByteLen = s->utf8.stringByteLen +  srcByteLen ;

		if (s->utf8.bufferByteLen >= newByteLen + (int)sizeof(FLEX_CHAR_UTF8) &&  s->utf8.string != NULL )
		{
			/* current buffer is ok, Just append the srcString to the end */
			FLEX_CHAR_UTF8 *dst  = s->utf8.string + (s->utf8.stringByteLen / sizeof(FLEX_CHAR_UTF8));
			s_copyUtf8(job, dst, srcString, srcByteLen);
			return job->lm_errno;
		}	
		else
		{
			FLEX_CHAR_UTF8 *savString  = s->utf8.string;
			int savByteLen = s->utf8.stringByteLen ;
			
			/* current buffer is to small so save the current string, 
			 * allocate a new string */
			if ((s->utf8.string = (FLEX_CHAR_UTF8 *)l_malloc(job,newByteLen + sizeof(FLEX_CHAR_UTF8))) != NULL)
			{
				/* copy old string */
				s->utf8.bufferByteLen = newByteLen  + sizeof(FLEX_CHAR_UTF8);
				s_copyUtf8(job, s->utf8.string, savString, savByteLen);
				/* copy src string */
				s_copyUtf8(job, s->utf8.string + (savByteLen / sizeof(FLEX_CHAR_UTF8)), srcString, srcByteLen);
				s->utf8.stringByteLen = savByteLen + srcByteLen;
			}
			/* free the old string */
			l_free(savString);
		}	
		/* lastly, since we changed the utf8 string we need to set the wide string 
		 * to '\0' if it exists. */
		s_clearWide(s);
	}
	return job->lm_errno;
}	
/***********************************************************************/
/**  @brief Return the length of a string in bytes
 * 
 * 	@param job - the job associated with this string
 * 	@param strHandle	- the string to be freeed
 * 	@param returnLen length of string in bytes returned here
 * 
 * @return 
 * 		- Greater than or equal tp zero indicating the length of the string in bytes
 * 			not including the terminating null.
 * 		- on error less than 0. one of the LM_xxxx error defines
 ************************************************************************/
FLEX_ERROR l_flexStrlenBytesUtf8(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, size_t *returnLen)
{
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job, returnLen)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	
	*returnLen = s->utf8.stringByteLen;
	return LM_NOERROR;
}

/***********************************************************************/
/**  @brief free the entire string
 * 
 * 	@param job		- the job associated with this string
 * 	@param strHandle - the string to be freeed
 * 
 * 	@return standard error codes
 * 		
 ************************************************************************/
FLEX_ERROR l_flexStrDelete(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle)
{
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	/* first free the string data pointers */
	if ( s->wide.string )
	{
		l_free(s->wide.string);
		s->wide.string = NULL;
		s->wide.bufferByteLen = 0;
		s->wide.stringByteLen = 0;
	}
	if ( s->utf8.string )
	{
		l_free(s->utf8.string);
		s->utf8.string = NULL;
		s->utf8.bufferByteLen = 0;
		s->utf8.stringByteLen = 0;
	}
	
	/* set these to zero so that if we try to use this freeed handle
	 * later we'll get an error instead of crashing. */
	s->handle = 0;
	s->isInUse = FLEX_FALSE;
	/* free the actual string if it was allocated. It could have been on the stack */
	if ( s->isAllocated )
	{
		s->isAllocated = FLEX_FALSE;
		l_free(s);
	}
	return job->lm_errno;
}	

/***********************************************************************/
/**  @brief return the length of a string in characters
 * 
 * 	@param job - the job associated with this string
 * 	@param strHandle	- the string 
 * 	@param returnLen	length of the string in chars returned here
 * 
 * @return 
 * 		- One of the LM_xxxx error defines
 ************************************************************************/
FLEX_ERROR l_flexStrlenChars(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, size_t *returnLen)
{
	FLEX_STRING	*	s;
	
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnLen)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)

	RETURN_SET_ERRNO(job,LM_NOTSUPPORTED)

}

/***********************************************************************/
/**  @brief Make a copy of a FLEX_STRING struct
 * 
 * @param job - the job associated with this string
 * @param strHandle	- the string to be coppied
 * @param returnHandle The handle to the new string is returned here.
 * 
 * @return 
 * 		- Greater than or equal tp zero indicating the length of the string
 * 		- on error less than 0. one of the LM_xxxx error defines
 * 
 * @see l_flexStrDupUtf8 l_flexStrDupWide l_flexStrcat l_flexStrcpy 
 ************************************************************************/
FLEX_ERROR l_flexStrDupString(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_STRING_HANDLE  *returnHandle)
{
	FLEX_STRING	*	s;
	
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnHandle)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	/* our prefered format is utf8  */
	if ( s->utf8.stringByteLen )
		l_flexStrNewUtf8(job, s->utf8.string, s->utf8.stringByteLen,returnHandle);
	else
	{
		if (l_flexStrNewEmpty(job, returnHandle) == LM_NOERROR)
		{
			if ( s->wide.stringByteLen )
			{
				/* we do all internal operations on UTF8. so convert to utf8 for 
				 * the copy */
				RETURN_SET_ERRNO(job,LM_NOTSUPPORTED)
				/* 
				(*returnHandle)->utF8.string = l_convertStringWCToMB(s->wide.string, &((*returnHandle)->utf8.byteLen));
				(*returnHandle)->utf8.bufferByteLen = (*returnHandle)->utf8.byteLen + sizeof(FLEX_CHAR_UTF8);
				 */ 
			}
		}
	}
	return job->lm_errno;
}

/***********************************************************************/
/**  @brief Make a copy of a UTF8 string from a FLEX_STRING
 * 
 * This function allocates memory with @ref l_malloc . The caller is responsible
 * for freeing the memory with @ref l_free .
 * 
 * @param job - the job associated with this string
 * @param strHandle	- the string to be coppied
 * @param returnPointer The pointer to the new UTF8 string is returned here.
 * 
 * @return 
 * 		- Greater than or equal tp zero indicating the length of the string
 * 		- on error less than 0. one of the LM_xxxx error defines
 * 
 * @see l_flexStrDupString l_flexStrDupWide l_flexStrcat l_flexStrcpy
 ************************************************************************/
FLEX_ERROR l_flexStrDupUtf8(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_UTF8 	**returnPointer )
{
	FLEX_STRING	*	s;
	FLEX_CHAR_UTF8 * pStr;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnPointer)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	
	/* get the actual string pointer for the utf8 string. 
	 * We call l_flexStrGetTmpUtf8() because it has the code to ensure that 
	 * there is actually a pointer for s->utf8.string. It will also
	 * do any converting necessary if we only have a wide string in the struct
	 *  */
	if (l_flexStrGetTmpUtf8(job, strHandle, &pStr) == LM_NOERROR)
	{
		if ((*returnPointer = l_malloc(job, s->utf8.stringByteLen + sizeof(FLEX_CHAR_UTF8))) != NULL)
		{
			FLEX_CHAR_UTF8 * ptr;
			memcpy(*returnPointer, pStr, s->utf8.stringByteLen);
			ptr = (*returnPointer) + s->utf8.stringByteLen;
			*ptr  = '\0'; /* null terminate */
		}	
	}	
	return job->lm_errno;
}


/***********************************************************************/
/**  @brief Get a temporary read only copy of the UTF8 string .
 * 
 * Gets the actual pointer to the raw string buffer. The raw data pointed to
 * by the returned pointer will exhist until the next flex library call is made.
 * 
 * @note This function is really only usefull for things like getting the string to 
 * pass it to a file open call.
 * 
 * @warning This is a dangerous command. Don't use it unless you know what
 * you are doing. DO NOT modify the string pointed to by this pointer
 * because flex will crash.
 * 
 * @param job - the job associated with this string
 * @param strHandle	- the string to be coppied
 * @param returnPointer The pointer to the new UTF8 string is returned here.
 * 
 * @return 
 * 		- Greater than or equal tp zero indicating the length of the string
 * 		- on error less than 0. one of the LM_xxxx error defines
 * 
 ************************************************************************/
FLEX_ERROR l_flexStrGetTmpUtf8(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle,FLEX_CHAR_UTF8	**returnPointer )
{
	FLEX_STRING *	s;
	FLEX_ERROR	err = LM_NOERROR;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnPointer)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	*returnPointer = NULL;
	
	if ( s->utf8.string == NULL && s->wide.string == NULL)
	{
		/* don't ever allow an empty string. Set it to the null string */
		err = l_flexStrcpyUtf8(job, strHandle,"",0);
		if ( err == LM_NOERROR)
			*returnPointer = s->utf8.string;
	}
	else if ( s->wide.string != NULL )
	{
		err = s_verifyUtf8String(job, strHandle);
		/** @todo if the utf8 string is null and the wide is not then 
		 * convert and make a utf8 string. for now just return an error */
		if ( err == LM_NOERROR && s->utf8.string != NULL )
			*returnPointer = s->utf8.string;
	}                          
	else
		*returnPointer = s->utf8.string;
	return job->lm_errno;	  
}	


/***********************************************************************/
/**  @brief The FLEX_STRING version of int to ascii. 
 * 
 * 	@param job	the job associated with this string
 * 	@param strHandle convert 'num' to ascii and append it to the string
 * 	@param inputNum then number to convert to ascii
 * 	@param convertType Do we convert to decimal or hex
 * 
 * 	@return standard error codes
 ************************************************************************/
FLEX_ERROR l_flexStrItoa(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, int inputNum, FLEX_CONVERT_TYPE convertType )
{                                                        	
	FLEX_CHAR_UTF8	buf[TMP_CHAR_BUF_SIZE + sizeof(FLEX_CHAR_UTF8)];
	FLEX_CHAR_UTF8	*pStr;
	FLEX_STRING *	s;
	int				newLen;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)

	switch ( convertType )
	{
		case FlexConvertHex:		   /* conversion  to hex */
			pStr = s_itoxa(buf,TMP_CHAR_BUF_SIZE ,inputNum,  &newLen);
			break;           
		case FlexConvertHexZeroX:	   /* conversion  to hex include the 0x */
			pStr = s_itoxa(buf,TMP_CHAR_BUF_SIZE , inputNum, &newLen);
			*(--pStr) = 'x';
			*(--pStr) = '0';
			newLen += 2;
			break;           
		case FlexConvertDecimal:	   /* conversion to decimal */
			pStr = s_itoda(buf,TMP_CHAR_BUF_SIZE , inputNum, &newLen);
			break;
		case FlexConvertAuto:		   
		case FlexConvertUndefined:	/* Conversion undefined. Ususally an error */
		default:
			LM_SET_ERRNO(job,LM_BADPARAM, 1043, 0)
	}
	if ( pStr )
	{
		l_flexStrcpyUtf8(job,strHandle,pStr, newLen);
		s_clearWide(s);
	}
	else
		LM_SET_ERRNO(job,LM_BADPARAM, 1044, 0)

	return job->lm_errno;
}

/***********************************************************************/
/**  @brief The FLEX_STRING version of ascii to int. 
 * 
 * Always stops at the first blank. An error will be returned if we try to parse 
 * a char that will not convert to the specified type.
 * 
 * 	param job	the job associated with this string
 * 	@param strHandle The string to convert to ascii
 * 	@param maxChars Maximim number of chars to use to convert to an int. 
 * 					Parsing will stop at the first blank or maxChars number of chars.
 * 					which ever comes first.
 * 					If zero then there is no max.
 * 	@param convertType
 * 	@param returnNum The converted number is returned here
 * 
 * 	@return standard error codes
 ************************************************************************/
FLEX_ERROR l_flexStrAtoi (LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, int maxChars, FLEX_CONVERT_TYPE convertType, int *returnNum)
{
	FLEX_CHAR_UTF8 *pStr;
	FLEX_CHAR_UTF8 *pTmp;
	int				localMax;
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)

	 /* make sure we have a utf8 string if there is a wide string */
	if (s_verifyUtf8String(job, strHandle) != LM_NOERROR)
		return job->lm_errno;

	/* how many chars can we safely process */
	if ( maxChars == 0 )
		localMax = s->utf8.stringByteLen / sizeof(FLEX_CHAR_UTF8);
	else
		localMax = min(maxChars,s->utf8.stringByteLen / (int)sizeof(FLEX_CHAR_UTF8));

	if ( localMax == 0 )
	{
		LM_SET_ERRNO(job,LM_EMPTYSTRING, 1049, 0)
		return job->lm_errno;
	}
	switch ( convertType )
	{
		case FlexConvertHex:		   /* conversion  to hex */
		case FlexConvertHexZeroX:	   /* conversion  to hex include the 0x */
			pStr = s_atox(s->utf8.string,  localMax, returnNum);
			break;           
		case FlexConvertDecimal:	   /* conversion to decimal */
			pStr = s_atod(s->utf8.string, localMax, returnNum);
			break;
		case FlexConvertAuto:		   
			pTmp = s->utf8.string;
			if ( *pTmp == (FLEX_CHAR_UTF8)'0' && ( *(pTmp + 1) == (FLEX_CHAR_UTF8)'x' || *(pTmp + 1) == (FLEX_CHAR_UTF8)'X'))
				pStr = s_atox(s->utf8.string, localMax, returnNum);
			else
				pStr = s_atod(s->utf8.string, localMax, returnNum);
			break;
		case FlexConvertUndefined:	/* Conversion undefined. Ususally an error */
		default:
			LM_SET_ERRNO(job,LM_BADPARAM, 1044, 0)
	}
	if ( pStr == NULL )
		LM_SET_ERRNO(job,LM_BADPARAM, 1045, 0)

	return job->lm_errno;
}



/***********************************************************************/
/**  @brief Concatinate two FLEX_STRINGs 
 * 
 * 	@param job	the job associated with this string
 * 	@param dstHandle the destination string 
 * 	@param srcHandle the src string
 *  
 * 	@return standard error codes
 * 
 *	@see l_flexStrDupString l_flexStrDupUtf8 l_flexStrDupWide l_flexStrcpy
 ************************************************************************/
FLEX_ERROR l_flexStrcat(LM_HANDLE * job, FLEX_STRING_HANDLE dstHandle, FLEX_STRING_HANDLE srcHandle)
{
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,dstHandle,&s)
	ERROR_RETURN_GET_HANDLE(job,srcHandle,&s)
	
	RETURN_SET_ERRNO(job,LM_NOTSUPPORTED)

	return job->lm_errno;
}


/***********************************************************************/
/**  @brief copy one FLEX_STRING to another.
 * 
 * The dst string will be over written.
 * 
 * 	@param job	the job associated with this string
 * 	@param dstHandle the destination string 
 * 	@param srcHandle the src string
 * 
 * 	@return standard error codes
 * 
 * @see l_flexStrDupString l_flexStrDupUtf8 l_flexStrDupWide l_flexStrcat 
 ************************************************************************/
FLEX_ERROR l_flexStrcpy(LM_HANDLE * job, FLEX_STRING_HANDLE dstHandle, FLEX_STRING_HANDLE srcHandle)
{
	FLEX_STRING *	dst;
	FLEX_STRING *	src;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,dstHandle,&dst)
	ERROR_RETURN_GET_HANDLE(job,srcHandle,&src)
	
	if ( src->utf8.stringByteLen == 0)
	{
		/* convert the wide string to utf8 */
		
		/* 
		s->utf8.string = l_convertStringWCToMB(s->wide.string, &(s->utf8.byteLen)); 
		s->utf8.bufferByteLen = s->utf8.byteLen + sizeof(FLEX_CHAR_UTF8);
		*/ 
	}
	l_flexStrcpyUtf8(job, dstHandle,src->utf8.string, src->utf8.stringByteLen);

	return job->lm_errno;
}

/***********************************************************************/
/**  @brief Get the real string pointer from the string handle
 * 
 * 	@warning Only call this routine from within the flexStr_xxx routines. 
 * 			It is not intended for general use in FLEXlm.
 * 
 * 	@param job - job associated with this string
 * 	@param strHandle	- handle tro the string to lookup
 * 	@param returnString - on return this is filled in with the pointer to the string
 * 
 * 	@return standard error codes
 ************************************************************************/
FLEX_ERROR i_getStringFromHandle(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_STRING **returnString)
{
	/* Later we'll use a hash table and handles that are not the same
	 * as the pointer. We do it this way now because we are presses for time */
	
	FLEX_STRING *	s = (FLEX_STRING *)strHandle;
	
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnString)

	*returnString = NULL;
	if ( strHandle == FLEX_STRING_HANDLE_NONE )
		LM_SET_ERRNO(job,LM_BADHANDLE, 1020, 0)
	else if ( s->handle != strHandle )
		LM_SET_ERRNO(job,LM_BADHANDLE, 1021, 0)
	else if ( ! s->isInUse )
		LM_SET_ERRNO(job,LM_BADHANDLE, 1022, 0)
	else
		*returnString = s;
	return job->lm_errno;
}	

/***********************************************************************
 * Static routines
 ************************************************************************/

/***********************************************************************/
/**  @brief Add a new FLEX_STRING to the list so we can look it up later by the handle
 * 
 * 	@param job	- the job this string is for
 * 	@param s	- the new FLEX_STRING to add
 * 	@param returnHandle	- the new handle assigned to this string is returned here
 * 
 * 	@return standard error codes
 ************************************************************************/
static FLEX_ERROR s_addStringToList(LM_HANDLE * job, FLEX_STRING *s, FLEX_STRING_HANDLE *returnHandle)
{
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,s)
	ERROR_RETURN_NULL_CHECK(job,returnHandle)

	/** @todo Later we'll use a hash table and handles that are not the same
	 * as the pointer. We just use the pointer now because we 
	 * are pressed for time */

	s->handle = (FLEX_STRING_HANDLE)s;
	*returnHandle = s->handle;

	return job->lm_errno;
}	

/***********************************************************************/
/**  @brief Set the raw UTF8 buffer to the size specified and clear the string.
 * 
 * If the current string is NULL or to small then new memory will be allocated.
 * If the current string is the same or larger then we don't bother changing it.
 * This func also has the effect of putting '\0' at the beginning of the string.
 * If the current utf8.string pointer is NULL and byteLen is 0 we don't do anything.
 * 
 * 	@param job - job associated with this string
 * 	@param s the string struct pointer
 * 	@param byteLen the new length in bytes Not counting the terninating null.
 * 				if it's zero we do not allocate any memory. 
 * 
 * 	@return standard error codes
 ************************************************************************/
static FLEX_ERROR s_makeBufferSizeUtf8(LM_HANDLE * job, FLEX_STRING *s, int byteLen)
{

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,s)
	/* error check params */
	if ( byteLen < 0 )
	{
		LM_SET_ERRNO(job,LM_BADPARAM, 1013, 0)
	}
	else 
	{
		s->utf8.stringByteLen = 0; /* always clear the string by setting the length to zero */
		if (s->utf8.bufferByteLen < byteLen + (int)sizeof(FLEX_CHAR_UTF8) &&  s->utf8.string != NULL )
		{
			/* current buffer is to small. We will re-use the memory 
			 * if the current string buffer is the same or larger than 
			 * the new string */
			l_free(s->utf8.string);
			s->utf8.string = NULL;
			s->utf8.bufferByteLen = 0;
			s->utf8.stringByteLen = 0;
		}	
		if ( s->utf8.string == NULL && byteLen )
		{
			/* allocate memory + 1 for the '\0' */
			if ((s->utf8.string = (FLEX_CHAR_UTF8 *)l_malloc(job,byteLen + sizeof(FLEX_CHAR_UTF8))) != NULL)
			{
				s->utf8.bufferByteLen = byteLen  + sizeof(FLEX_CHAR_UTF8);
			}
		}	
		if ( s->utf8.string != NULL )
			*(s->utf8.string) = '\0';  /* make sure we are null terminated */
	}
	return job->lm_errno;
}	
/***********************************************************************/
/**  @brief Copy a raw UTF8 string.
 * 
 * The caller must ensure that the dst string is large enough
 * 
 * 	@param job - job associated with this string
 * 	@param dst  the raw dest pointer
 * 	@param src  the raw src pointer
 * 	@param maxByteLen number of bytes to copy. Not including the terminating null
 * 
 * 	@return standard error codes
 ************************************************************************/
static FLEX_ERROR s_copyUtf8(LM_HANDLE * job, FLEX_CHAR_UTF8 *dst, FLEX_CHAR_UTF8 *src, int maxByteLen)
{
	/* don't do anything if the length is zero */
	if ( maxByteLen == 0 )
		return LM_NOERROR;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,src)
	ERROR_RETURN_NULL_CHECK(job,dst)
	/* error check params */
	if ( maxByteLen < 0)
	{
		LM_SET_ERRNO(job,LM_BADPARAM, 1016, 0)
	}
	else 
	{
		int charLen = maxByteLen / sizeof(FLEX_CHAR_UTF8) ;
		 /* safe copy */
		for (  ; *src && charLen > 0; charLen-- )
		{
			*dst++ = *src++;
		}
		*dst = '\0';
	}
	return job->lm_errno;
}
/***********************************************************************/
/**  @brief Initalize a FLEX_STRING struct. 
 * 
 * 	@param job - the job associated with this string
 * 	@param s	- pointer to the string 
 * 	@param returnHandle	- on return the new string handle is placed here
 * 
 *  @return standard error codes
 ************************************************************************/
static FLEX_ERROR s_flexStrInitNew(LM_HANDLE * job, FLEX_STRING *s, FLEX_STRING_HANDLE *returnHandle)
{
	if ( returnHandle != NULL )
		*returnHandle = FLEX_STRING_HANDLE_NONE;
	
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,s)
	ERROR_RETURN_NULL_CHECK(job,returnHandle)
	if ( ! s->isAllocated )
	{
		/* the string is on the stack so zero it out */
		memset((void *)s, 0, sizeof(FLEX_STRING));
	}

	s->isInUse = FLEX_TRUE;
	s->job = job;
	
	/* add the string to the list so we can do a lookup on the handle later.
	 * Also sets the returnHandle and the handle member in the string. */
	return s_addStringToList(job, s, returnHandle);
}	
/***********************************************************************/
/** @brief Clear the wide portion of the string
 *
 * @param s pointer to a FLEX_STRING struct
 *
 ************************************************************************/								  
static void s_clearWide(FLEX_STRING *s)
{
	/* don't need to delete the buffer just set the length of the string 
	 * to zero */
	s->wide.stringByteLen = 0;
}	
/***********************************************************************/
/** @brief Local routine to convert an int to an ascii hex string
 *
 * @param dstBuf  buffer to put the converted string in
 * @param bufLen  size of dstBuf
 * @param inputNum the number to be converted
 * @param newLen  The length of the string built in dstBuf
 *
 * @return pointer to the beginning of the string in dstBuf
 ************************************************************************/
static FLEX_CHAR_UTF8 * s_itoxa(FLEX_CHAR_UTF8 *dstBuf, int bufLen, int inputNum, int * newLen)
{
	static FLEX_CHAR_UTF8 hexBuf[] = "0123456789abcdef";
	
	FLEX_CHAR_UTF8 *s = dstBuf + bufLen - 1;
	int len = 0;

	*s = '\0';
	s--;
	bufLen--;
	if ( bufLen <= 0 )
		return NULL;
	do
	{
		*s = hexBuf[ (int)(inputNum & 0x0f)];
		s--;
		inputNum >>= 4;
		len++;
	} while ( inputNum && --bufLen );
	*newLen = len;
	s++;
	return s;
}	
/***********************************************************************/
/** @brief Local routine to convert an int to an ascii decimal string
 *
 * @param dstBuf  buffer to put the converted string in
 * @param bufLen  size of dstBuf
 * @param inputNum the number to be converted
 * @param newLen  The length of the string built in dstBuf
 *
 * @return pointer to the beginning of the string in dstBuf
 ************************************************************************/
static FLEX_CHAR_UTF8 * s_itoda(FLEX_CHAR_UTF8 *dstBuf, int bufLen, int inputNum, int * newLen)
{
	FLEX_CHAR_UTF8 *s = dstBuf + bufLen - 1;
	int len = 0;
	FLEX_BOOL negative = (inputNum < 0) ? FLEX_TRUE :  FLEX_FALSE;

	*s = '\0';
	s--;
	bufLen--;
	if ( bufLen <= 0 )
		return NULL;
	do
	{
		*s-- = inputNum % 10 + (int)'0';
		inputNum /= 10;
		len++;
	} while ( inputNum && --bufLen );
	
	if ( negative )
	{
		*s-- = '-';
		len++;
	}
	*newLen = len;
	s++;
	return s;
}	
/** max number of hex digits in an int */
#define MAX_HEX_CHARS (sizeof(int) * 2)
/***********************************************************************/
/** @brief Internal routine to convert an ascii string to hex.
 * 
 * Skip leading white space and stop on the first non-hex char.
 *
 * @param inputString the string to convert to hex
 * @param maxChars max number of chars to process
 * @param num  the resulting number
 *
 * @return 
 * 		- the pointer to the char in the inputString past the last char processed.
 * 		- NULL on error
 *
 ************************************************************************/
static FLEX_CHAR_UTF8 * s_atox(FLEX_CHAR_UTF8 *inputString, int maxChars, int *num)
{
	static int hexList[256] = {
		/* 0x00 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	,
		/* 0x10 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	,
		/* 0x20 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	,
		/* 0x30 */   0,  1,  2,  3,  4,  5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1    ,
		/* 0x40 */ 0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1    ,
		/* 0x50 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	,
		/* 0x60 */ -1,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f -1,-1,-1,-1,-1,-1,-1,-1,-1    ,
		/* 0x70 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	,
		/* 0x80 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	,
		/* 0x90 */  -1, -1, -1, -1, -1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	
	};	

	FLEX_CHAR_UTF8 *	localString ;
	int 	index;
	int 	returnNumber = 0;
	int 	maxToProcess ; 

	
	if (inputString == NULL || *inputString == '\0' || num == NULL )
		return NULL;

	localString = inputString;
	/* skip white space */
	while ( *localString == ' ' || *localString == '\t')
	{
		++localString;
		maxChars--;
	}

	/* strip the leading 0x if there is one */
	if ( *localString == (FLEX_CHAR_UTF8)'0' && 
		( *(localString + 1) == (FLEX_CHAR_UTF8)'x' || 
		  *(localString + 1) == (FLEX_CHAR_UTF8)'X'))
	{
		localString += sizeof(FLEX_CHAR_UTF8) * 2;
		maxChars -= 2;
	}
	maxToProcess = min(MAX_HEX_CHARS,maxChars) ; 

	/*  the first word is a hex number. Stop on the first non hex digit */
	for ( index = maxToProcess ; 
			index , hexList[(unsigned int)(*localString)] >= 0;
			index--, localString++)
	{
		returnNumber <<= 4;
		returnNumber += hexList[(unsigned int)(*localString)];
	}
	if (index == maxToProcess)
		return NULL;

	/* return the number */
	*num = returnNumber;
	/* return the new position in the string */
	return localString;
}

/***********************************************************************/
/** @brief Internal routine to convert an ascii string to decimal.
 * 
 * Skip leading white space and stop on the first non-numeric char.
 *
 * @param inputString the string to convert 
 * @param maxChars max number of chars to process
 * @param num  the resulting number
 *
 * @return 
 * 		- the pointer to the char in the inputString past the last char processed.
 * 		- NULL on error
 *
 ************************************************************************/
static FLEX_CHAR_UTF8 * s_atod(FLEX_CHAR_UTF8 *inputString, int maxChars, int *num)
{
	FLEX_CHAR_UTF8 *localString ;
	int index;
	int returnNumber = 0;
	int maxToProcess  ; 

	if (inputString == NULL || *inputString == '\0' || num == NULL )
		return NULL;

	localString = inputString;
	/* skip white space */
	while ( *localString == ' ' || *localString == '\t')
	{
		++localString;
		maxChars--;
	}
	maxToProcess = maxChars;

	/* Stop on the first non-numeric */
	for ( index = maxToProcess ; 
			index, *localString >= '0', *localString <= '9';
			index--, localString++)
	{
		returnNumber *= 10;
		returnNumber += (int)(*localString) + '0';
	}
	if (index == maxToProcess)
		return NULL;

	/* return the number */
	*num = returnNumber;
	/* return the new position in the string */
	return localString;
}
/***********************************************************************/
/** @brief Verify ther is  UTF8 string. If not and there is a Wide string
 * 			then do a conversion. If there is not a wide string the 
 * 			leave it alone
 *
 * @param   job	 job handle
 * @param strHandle the string handle
 *
 * @return stndard error values
 *
 ************************************************************************/
static FLEX_ERROR s_verifyUtf8String(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle)
{
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
		
	/** @todo if the utf8 string is null and the wide is not then 
	 * convert and make a utf8 string. for now just return an error */
	if ( s->utf8.string == NULL && s->wide.string != NULL)
	{

		RETURN_SET_ERRNO(job,LM_NOTSUPPORTED)

		/* 
		s->utf8.string = l_convertStringWCToMB(s->wide.string, &(s->utf8.byteLen)); 
		s->utf8.bufferByteLen = s->utf8.byteLen + sizeof(FLEX_CHAR_UTF8);
		*returnPointer = s->utf8.string;
		*/ 
	}
	return job->lm_errno;
}

