/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_handshake.c,v 1.18 2003/01/13 22:41:52 kmaclean Exp $
 *
 *	Function: l_handshake(job)
 *
 *	Description: Sets up handshaking encryption for communications.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *
 *	Return:		0 - OK
 *			<> 0 - Error
 *
 *	M. Christiano
 *	3/16/90
 *
 *	Last changed:  12/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "l_prot.h"
#ifdef LM_GENERIC_VD
#include "lg_code.h"
#endif /* GENERIC */

#ifdef PC16
#include <time.h>
#endif /* PC16 */

#ifndef RELEASE_VERSION
#define HANDDEBUG(x) if (handdebug) printf x
int which_failed;
#else
#define HANDDEBUG(x) 
#endif

static void encode	lm_args(( long, char *, long, char *));
static int decode	lm_args(( LM_HANDLE *, long, char *, long, char *,int));
static void setup_keys	lm_args(( long *, long *, long *, long *));

l_handshake(job)
LM_HANDLE *job;		/* Current license job */
{
  char msg[LM_MSG_LEN+1];
  char *rmsg;
  char rcvmsg;
  int i;
  int good = 0;
  long d, d2, d3, d4;
  long newkey; 
  char junk[MAX_HAND_DATA_LEN+1], junk2[MAX_HAND_DATA_LEN+1];
  typedef int (*_user_crypt_filter)
			lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int , int));
  typedef int (*_handshake_filter)
			lm_args((LM_VOID_PTR, LM_VOID_PTR, int , 
				unsigned char *, int, int *));
  _user_crypt_filter filter = 0;
  _handshake_filter hfilter = 0;

#ifndef RELEASE_VERSION
  char sent[MAX_HAND_DATA_LEN+1], esent[MAX_HAND_DATA_LEN+1];
  char * handdebug =  l_real_getenv("HANDDEBUG");
#endif

	{
	  VENDORCODE vc;
	  unsigned char uc;
		memcpy(&vc, &job->code, sizeof(vc));
		l_sg(job, job->vendor, &vc);
		newkey = (vc.data[0]) ^ (vc.data[1]);
		memset(&vc, 0, sizeof(vc));
		HANDDEBUG(("\nnewkey  %x, ", newkey));
		if (job->keymem2) /* P5798 */
			filter = (_user_crypt_filter)job->keymem2;
		else if (job->user_crypt_filter)
			filter = (_user_crypt_filter)job->user_crypt_filter;
		else if (job->l_new_job && 
			!(job->options->flags & LM_OPTFLAG_V7_0_HANDSHAKE) && 
			! (!job->L_SIGN_LEVEL &&
				((job->daemon->ver < 7) ||
				((job->daemon->ver == 7) && 
				(job->daemon->rev == 0) ))))
		
			hfilter = (_handshake_filter)job->l_new_job;
		if (filter || hfilter)
		{
			for (i = 0; i < 4 ; i ++)
			{
				uc = (newkey & (0xff << (i *8))) >> (i * 8);
				if (job->user_crypt_filter)
					(*filter)(job, &uc, i, 0);
				else
					(*hfilter)(0, 0, 1, &uc, i, 0);
				newkey ^= (uc << (i * 8));
			}
		}
	}
	HANDDEBUG(("%x\n", newkey));
	job->daemon->encryption = 0;
/*-
 *	Find out if we are encrypting traffic or not
 */
	if (job->options->no_traffic_encrypt) return(0);
/*-
 *	First, build the handshake message
 */
	memset(msg, 0, LM_MSG_LEN+1);

	l_encode_long_hex(&msg[MSG_HAND_TIME], (long) time(0));
	if (job->l_new_job)
		msg[MSG_HAND_CRYPT] = '2';
	else
		msg[MSG_HAND_CRYPT] = '1';

	setup_keys(&d, &d2, &d3, &d4);
	encode(d2, &msg[MSG_HAND_DATA2], newkey, junk);
	encode(d3, &msg[MSG_HAND_DATA3], newkey, junk);
	encode(d4, &msg[MSG_HAND_DATA4], newkey, junk);
	encode(d, &msg[MSG_HAND_DATA], newkey, junk);

#ifndef RELEASE_VERSION
/*-
 *	Save some of the data, in case we have an error.
 */
	memset(sent, 0, MAX_HAND_DATA_LEN+1);
	memset(esent, 0, MAX_HAND_DATA_LEN+1);
	strcpy(sent, junk);
	memcpy(esent, &msg[MSG_HAND_DATA], MAX_HAND_DATA_LEN);
#endif
	l_encode_32bit_packed(&msg[MSG_HAND_GROUP_ID], job->group_id);
		
/*-
 *	Now, send it, and await the reply
 */
	l_sndmsg(job, LM_HANDSHAKE, &msg[MSG_DATA]);
	l_rcvmsg(job, &rcvmsg, &rmsg);

	if (job->lm_errno == NOSERVRESP || !rmsg) goto exit_handshake;
/*-
 *	If the modified time seed matches, we have the new encryption seeds;
 *	otherwise, we have a problem .....
 */
#ifndef RELEASE_VERSION	
	which_failed = 0;
#endif
	if (decode(job,d2,&rmsg[MSG_HAND_DATA2-MSG_DATA],newkey, junk2, -1) 
	 && decode(job, d3, &rmsg[MSG_HAND_DATA3-MSG_DATA],newkey,junk2, -1)
	 && decode(job, d4, &rmsg[MSG_HAND_DATA4-MSG_DATA], newkey,junk2,-1)
	 && decode(job, d, &rmsg[MSG_HAND_DATA-MSG_DATA],  newkey,junk2,rcvmsg))
		good = 1;
	
	memcpy(junk, junk2, MAX_HAND_DATA_LEN);
	if (good)
	{
/*-
 *		Good handshake.  Set up the encryption for the communications.
 */
		job->daemon->encryption = d;
	}
	else
	{
/*-
 *		HANDSHAKE error.  Dump all the handshake data.
 */
		HANDDEBUG(("\nhandshake #%d failed", which_failed));
		HANDDEBUG(("\nBAD HANDSHAKE, %s handshake reply, (reply: %c)\n",
				rcvmsg == LM_HANDSHAKE ? "received" : 
				"did not receive", rcvmsg));
		HANDDEBUG(("Sent: %lx\nsent:      ", d));
		for (i=0; i<MAX_HAND_DATA_LEN; i++) 
		{
			HANDDEBUG(("%x ", 0xff & sent[i]));
		}
		HANDDEBUG(("\nencrypted: "));
		for (i=0; i<MAX_HAND_DATA_LEN; i++) 
		{
			HANDDEBUG(("%x ", 0xff & esent[i]));
		}
		HANDDEBUG(("\nmodified:  "));
		for (i=0; i<MAX_HAND_DATA_LEN; i++) 
		{
			HANDDEBUG(("%x ", 0xff & junk2[i]));
		}
		HANDDEBUG(("\n\nencrypted modified sent: "));
		for (i=0; i<MAX_HAND_DATA_LEN; i++) 
		{
			HANDDEBUG(("%x ", 0xff & junk[i]));
		}
		HANDDEBUG(("\nReceived:                "));
		for (i=0; rmsg && i<MAX_HAND_DATA_LEN; i++) 
		{
			HANDDEBUG(("%x ", 0xff & rmsg[MSG_HAND_DATA-MSG_DATA+i]));
		}
		HANDDEBUG(("\n"));
	}
exit_handshake:
	if (job->daemon->encryption == 0L) /*- P2477 */
		lc_disconn(job, 1);

	return(job->daemon->encryption != 0L);
}
static
void
encode(d, buf, key, junk)
long d;
char *buf;
long key;
char *junk;
{

	memset(junk, 0, MAX_HAND_DATA_LEN+1);
	l_encode_long_hex(junk, (long)d);
	memcpy( buf, 
		l_str_crypt(junk, MAX_HAND_DATA_LEN, key, 1), 
		MAX_HAND_DATA_LEN);
}
static
int
decode(job, d, buf, key, junk, rcvmsg)
LM_HANDLE *job;
long d;
char *buf;
long key;
char *junk;
int rcvmsg;
{
  int verrev;

#ifndef RELEASE_VERSION	
	which_failed++;
#endif
	verrev = (job->daemon->ver * 100) + job->daemon->rev;
	memset(junk, 0, MAX_HAND_DATA_LEN+1);
	l_encode_long_hex(junk, l_modify((long)d));
/*
 *	Handle the case where it's a pre-v6 server, and
 *	it's the extra handshake tests
 */
	if ((rcvmsg == -1 && (verrev < 579) )) return 1;

	if (rcvmsg != LM_HANDSHAKE && rcvmsg != -1) 
	{
		LM_SET_ERRNO(job, LM_BADHANDSHAKE, 375, 0);
		return 0;
	}

	l_str_crypt(junk, MAX_HAND_DATA_LEN, key, 1);
	if (!memcmp( buf, junk, MAX_HAND_DATA_LEN)) return 1;

	if (rcvmsg == -1) 	LM_SET_ERRNO(job, LM_BADHANDSHAKE, 376, 0)
	else 			LM_SET_ERRNO(job, LM_BADHANDSHAKE, 377, 0)

	return 0;
}
static
void
setup_keys(d1, d2, d3, d4)
long *d1;
long *d2;
long *d3;
long *d4;
{
	*d1 = time(0) & 0xffffffff;
	srand((unsigned int) *d1);
	*d2 = (rand() ^ (rand() << 8) ^ (rand() << 16) ^ (rand() << 24)) 
				& 0xffffffff;
	*d3 = (rand() ^ (rand() << 8) ^ (rand() << 16) ^ (rand() << 24) )
				& 0xffffffff;
	*d4 = (rand() ^ (rand() << 8) ^ (rand() << 16) ^ (rand() << 24))
				& 0xffffffff;
	*d1 ^= *d4;
}
