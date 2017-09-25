/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_cksum.c,v 1.7 2003/01/13 22:41:50 kmaclean Exp $
 *
 *	Function:	l_cksum(), l_sum(str)
 *
 *	Description: 	l_cksum(job, conf, bad, code) - checksum feature line
 *			l_sum(str) - sums a string
 *			l_onebyte(x) - Turns an int into a byte (for checksum)
 *
 *	Parameters:	(char *) str - string to be checksummed
 *			(unsigned) x - int to be turned into 1-byte checksum
 *			(LM_HANDLE *) job - current job
 *			(CONFIG *) conf - feature/increment/upgrade line to
 *						checksum
 *			(VENDORCODE *) code - dummy vendorcode for l_crypt.
 *
 *	Return:		l_cksum() - (unsigned) checksum
 *			(int *) bad - set to 1 - if checksum doesn't match
 *					cksum in input CONFIG struct
 *			l_sum() - (unsigned) checksum
 *			l_onebyte() - (unsigned char) checksum byte
 *
 *	M. Christiano
 *	7/30/94
 *
 *	Last changed:  5/13/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

unsigned API_ENTRY
l_sum(str)
char *str;
{
  unsigned res = 0;

	if (str)
	{
		while (*str)
		{
			res += (unsigned) (*str);
			str++;
		}
	}
	return(res);
}

unsigned char API_ENTRY
l_onebyte(x)
unsigned x;
{
  unsigned char r;

	r = (x & 0xff) +
	    ((x >> 8) & 0xff) +
	    ((x >> 16) & 0xff) +
	    ((x >> 24) & 0xff);
	return(r);
}

unsigned API_ENTRY
l_cksum(job, conf, bad, dummycode)
LM_HANDLE *job;
CONFIG *conf;
int *bad;
VENDORCODE *dummycode;
{
  unsigned x = 0;
  char *code;
  VENDORCODE vc;
  int flags = job->flags;
  LM_KEYLIST *k = conf->lc_keylist;

	
	conf->lc_keylist = 0;
	memcpy((char *)&vc, (char *)dummycode, sizeof(vc));
	l_sg(job, job->vendor, &vc);
	conf->server = (LM_SERVER *) NULL;
	job->flags |= LM_FLAG_MAKE_OLD_KEY; 
	code = lc_crypt(job, conf, 
		l_extract_date(job, conf->code), &vc);
	job->flags = flags;
	conf->lc_keylist = k;
	x = l_onebyte(l_sum(code) + l_sum(conf->code));
	return(x);
}
