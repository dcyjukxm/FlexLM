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
/* $Id: flex_string_wide.c,v 1.1 2003/04/09 17:36:07 kmaclean Exp $
 ************************************************************************/
 /** @file flex_string_wide.c
  * @brief FLEXlm string routines for wide chars
  * @version $Revision: 1.1 $
  * 
  * FLEXlm string to be used in support of internationalization and generally
  * longer strings.
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
#include "flex_string_internal.h"

/***********************************************************************
 * Static function forwards
 ************************************************************************/
static FLEX_ERROR s_makeBufferSizeWide(LM_HANDLE * job, FLEX_STRING *s, int byteLen);
static FLEX_ERROR s_copyWide(LM_HANDLE * job, FLEX_CHAR_WIDE *dst, FLEX_CHAR_WIDE *src, int byteLen);

/***********************************************************************/
/** @brief Allocate a new string object. Initalize with Wide string.
 * 
 * 	@param job - the job associated with this string
 * 	@param string	- a Wide string to be used as the initial value.
 * 					a copy of 'string' will be made.
 * 					if string is NULL then an empty object will be created
 *  @param byteLen the maximum length of the 'string' passed in
 * 					terminating null not included
 * 	@param returnHandle	- on return the new string handle is placed here
 * 
 * 	@return standard error codes
 * 
 ************************************************************************/
FLEX_ERROR l_flexStrNewWide(LM_HANDLE * job ,FLEX_CHAR_WIDE *string, int byteLen, FLEX_STRING_HANDLE *returnHandle)
{
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnHandle)
	*returnHandle = FLEX_STRING_HANDLE_NONE;

	if (l_flexStrNewEmpty(job,returnHandle) == LM_NOERROR)
	{
		if ( string != NULL)
		{
			/* it's ok if the string is NULL) */
			if (l_flexStrcpyWide(job ,*returnHandle, string,byteLen) != LM_NOERROR )
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
 * 	@param job - the job associated with this string
 *  @param maxByteLen the maximum length of the 'string' passed in. 
 * 					terminating null not included
 * 	@param strHandle	-  the dst string handle
 * 	@param srcString	- the src string buffer to be set. If NULL then 
 * 							the string will be set to the empty string
 * 
 * 	@return standard error codes
 ************************************************************************/
FLEX_ERROR l_flexStrcpyWide(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_WIDE *srcString, int maxByteLen )
{
	FLEX_STRING	*	s;
	int				srcByteLen = 0;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	
	if ( srcString != NULL )
	{
		FLEX_CHAR_WIDE *	tmpSrcString = srcString;
		/* how long is the source string */
		for (	srcByteLen = 0 ; 
				*tmpSrcString != '\0' && srcByteLen <= maxByteLen; 
				tmpSrcString++, srcByteLen++ )
		{
			;
		}
	}
		
	/* bytelen may be zero. less than zero means an error occured */
	if (s_makeBufferSizeWide(job, s,srcByteLen) == LM_NOERROR)
	{
		if (s_copyWide(job, s->wide.string, srcString, srcByteLen) == LM_NOERROR)
		{
			s->wide.stringByteLen = srcByteLen;
		}	
	}
	return job->lm_errno;
}	
/***********************************************************************/
/**  @brief Return the length of a string in bytes
 * 
 * 	@param job - the job associated with this string
 * 	@param strHandle	- the string to be freeed
 * 
 * @return 
 * 		- Greater than or equal to zero indicates the length of the string in bytes.
 * 			not including the terminating null.
 * 		- on error less than 0. one of the LM_xxxx error defines
 ************************************************************************/
FLEX_ERROR l_flexStrlenBytesWide(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, size_t *returnLen)
{
	FLEX_STRING *	s;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job, returnLen)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	*returnLen = s->utf8.stringByteLen;
	return LM_NOERROR;
}
/***********************************************************************/
/**  @brief Get a temporary read only copy of the Wide string .
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
 * @param returnPointer The pointer to the new Wide string is returned here.
 * 
 * @return 
 * 		- Greater than or equal tp zero indicating the length of the string
 * 		- on error less than 0. one of the LM_xxxx error defines
 * 
 ************************************************************************/
FLEX_ERROR l_flexStrGetTmpWide(LM_HANDLE * job, FLEX_STRING_HANDLE strHandle, FLEX_CHAR_WIDE **returnPointer )
{
	FLEX_STRING *	s;
	FLEX_ERROR		err = LM_NOERROR;

	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,returnPointer)
	ERROR_RETURN_GET_HANDLE(job,strHandle,&s)
	*returnPointer = NULL;
	if ( s->utf8.string == NULL && s->wide.string == NULL)
	{
		/* don't ever allow an empty string. Set it to the null string */
		err = l_flexStrcpyWide(job, strHandle,L"",0);
	}
	if ( err == LM_NOERROR )
	{
		/** @todo if the utf8 string is null and the wide is not then 
		 * convert and make a utf8 string. for now just return an error */
		if ( s->wide.string == NULL )
		{
			/** @todo convert from utf8 to wide */
			return LM_NOTSUPPORTED;
			/* 
			s->wide.string = l_convertStringMBToWC(s->utf8.string, &(s->wide.byteLen)); 
			s->wide.bufferByteLen = s->wide.byteLen + sizeof(FLEX_CHAR_WIDE);
			*returnPointer = s->wide.string;
			*/ 
		}	
		*returnPointer = s->wide.string;
	}
	return job->lm_errno;	  
}

/***********************************************************************/
/**  @brief Set the raw Wide buffer to the size specified and clear the string.
 * 
 * If the current string is NULL or to small then new memory will be allocated.
 * If the current string is the same or larger then we don't bother changing it.
 * 
 * 	@param job - job associated with this string
 * 	@param s the string struct pointer
 *	@param byteLen the new length in bytes. Not counting the terninating null
 * 
 * 	@return standard error codes
 ************************************************************************/
static FLEX_ERROR s_makeBufferSizeWide(LM_HANDLE * job, FLEX_STRING *s, int byteLen)
{
	ERROR_RETURN_JOB_CHECK(job)
	ERROR_RETURN_NULL_CHECK(job,s)
	
	/* error check params */
	if ( byteLen < 0)
	{
		LM_SET_ERRNO(job,LM_BADPARAM, 1033, 0)
	}
	else 
	{
		s->wide.stringByteLen = 0; /* always clear the string by setting the length to zero */
		if (s->wide.bufferByteLen < byteLen + (int)sizeof(FLEX_CHAR_WIDE) &&  s->wide.string != NULL )
		{
			/* current buffer is to small. We will re-use the memory 
			 * if the current string buffer is the same or larger than 
			 * the new string */
			l_free(s->wide.string);
			s->wide.string = NULL;
			s->wide.bufferByteLen = 0;
			s->wide.stringByteLen = 0;
		}	
		if ( s->wide.string == NULL && byteLen )
		{
			/* allocate memory */
			if ((s->wide.string = (FLEX_CHAR_WIDE *)l_malloc(job,byteLen + sizeof(FLEX_CHAR_WIDE))) != NULL)
			{
				s->wide.bufferByteLen = byteLen + sizeof(FLEX_CHAR_WIDE);
			}
		}	
		if ( s->wide.string != NULL )
			*(s->wide.string) = '\0';  /* make sure we are null terminated */
	}
	return job->lm_errno;
}	
/***********************************************************************/
/**  @brief Copy a raw Wide string.
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
static FLEX_ERROR s_copyWide(LM_HANDLE * job, FLEX_CHAR_WIDE *dst, FLEX_CHAR_WIDE *src, int maxByteLen)
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
		int charLen = maxByteLen / sizeof(FLEX_CHAR_WIDE);
		 /* safe copy */
		for (  ; *src && charLen > 0; charLen-- )
		{
			*dst++ = *src++;
		}
		*dst = L'\0';
	}
	return job->lm_errno;
}

