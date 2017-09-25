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

/*****************************************************************************
 *	
 *	Module: $Id: l_comp_id.c,v 1.7 2003/03/10 19:14:58 jwong Exp $
 *
 *	Composite Hostid Module
 *
 *	J. Wong
 *	2/28/03
 *
 *	Last changed:  3/4/03
 *
 *
 *****************************************************************************/

#include "l_sha.h"

#include "lmclient.h"
#include "lm_attr.h"
#include "l_prot.h"

#include <stdlib.h>
#include <errno.h>
#include "string.h"

#include "composite.h"

/************************************************************************************
 *  Function:    StripString()
 *
 *  Description: removes prefix for a flex hostid, if it exists.
 *
 *  Parameters:  (char *) SearchString - substring to strip.
 *               (char *) buffer - data buffer.
 *
 *  Return:      (void)
 *
 *
 ************************************************************************************/
void
StripString( char * SearchString, char * buffer )
{
char Temp[2049];
char * p, *q;
int BeginningLength;
int  SearchStringLen= strlen( SearchString );

	while (  ( p=strstr( buffer , SearchString ) ))
	{
		/* Copy the beginning of the buffer to the temp string */
		q=&(buffer[0]);
		BeginningLength = p - q ;
		Temp[0]=0;
		if ( BeginningLength )
		{
			strncpy(Temp,buffer,BeginningLength);
			Temp[BeginningLength]= 0;
		}
 		
		/* Copy the buffer after the searchstring to the end of the buffer */
		p+=SearchStringLen;
		strcat(Temp,p);

		/* Move the string back into the original buffer */
		strcpy(buffer,Temp );
	}
}


/*******************************************************************************
 *  Function:    char * l_composite_id(LM_HANDLE*, int, int)
 *
 *  Description: create a composite hostid based on established parameters
 *               provided by calling either lc_init_simple_composite() or
 *               lc_init_complex_composite().
 *
 *  Parameters:	 (LM_HANDLE *) job - current job handle.
 *               (int) iType - use simple or complex construction of ID.
 *               (int) iLogic - use logical AND or OR to construct ID.
 *
 *  Return:      (char *) - composite hostid string
 *
 *  NOTES: current default values for iType and iLogic are 1. This will
 *         will change when Phase 2 is initiated and additional functionality
 *         is added.
 *
 *******************************************************************************/

char * 
l_composite_id(LM_HANDLE *job)
{
SHA_CTX sha;
unsigned char digest[20] = {0};
unsigned char hostid_str[4096] = {0};
unsigned char buf[MAX_HOSTID_LEN] = {0};
static char szBuf[MAX_HOSTID_LEN] = {'\0'};
int ret, i;
HOSTID *h, hostid;


	if ((job->composite_init == NULL) || (job->composite_init->id_count <= 0))
	{
		LM_SET_ERRNO(job, LM_COMPOSITEID_INIT_ERR, 618, errno);
		return (NULL);
	}

	/* load hostid_str with simple composite */
	for (i = 0; i < job->composite_init->id_count; i++)
	{
		h = l_getid_type(job, job->composite_init->info_list[i].id_type);
		if (h == NULL)
		{
			LM_SET_ERRNO(job, LM_COMPOSITEID_ITEM_ERR, 619, errno);
			return (NULL);
		}
		memcpy(&hostid, h, sizeof(hostid));
		hostid.next = 0;
		l_zcp((char *)buf, l_asc_hostid(job, &hostid), MAX_HOSTID_LEN);

		strcat((char *)hostid_str, (char *)buf);
		strcat((char *)hostid_str, " "); 
	}

	if (h)
		lc_free_hostid(job, h);


	/* SHA hashing */
	SHAInit(&sha);
	SHAUpdate(&sha, hostid_str, strlen((char *)hostid_str));
	SHAFinal(digest, &sha);

	/* got it! */
	sprintf(szBuf, "%02X%02X%02X%02X%02X%02X%", digest[0], digest[1], digest[2], 
												digest[3], digest[4], digest[5]);
	return (szBuf); 
}
