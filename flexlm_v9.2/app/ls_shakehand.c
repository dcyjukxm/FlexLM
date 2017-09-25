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
 *	Module: $Id: ls_shakehand.c,v 1.15 2003/01/13 22:22:37 kmaclean Exp $
 *
 *	Function: ls_shakehand(msg, key, user)
 *
 *	Description: Replies to client handshake.
 *
 *	Parameters:	(char *) msg - The handshake message
 *			(VENDORCODE *) key - Vendor's encryption key.
 *			(CLIENT_DATA *) user - Our user's data.
 *
 *	Return:		0 - OK
 *			<> 0 - Error
 *
 *	M. Christiano
 *	3/16/90
 *
 *	Last changed:  11/13/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "ls_sprot.h"
#include "ls_aprot.h"

#ifndef RELEASE_VERSION
#define HANDDEBUG(x) if (handdebug) printf x
#else
#define HANDDEBUG(x) 
#endif

static void decode	lm_args((long *, char *, long, CLIENT_DATA *, char *));
static void encode	lm_args(( char *, long, int, char *));

ls_shakehand(msg, key, user)
char *msg;		/* Message we got */
VENDORCODE *key;	/* Our encryption seed */
CLIENT_DATA *user;	/* Our user's data */
{
  char resp[LM_MSG_LEN+1];
  char junk[MAX_HAND_DATA_LEN+1];
  char junk2[MAX_HAND_DATA_LEN+1];
  char junk3[MAX_HAND_DATA_LEN+1];
  char junk4[MAX_HAND_DATA_LEN+1];
  unsigned char uc;
  long d, d2, d3, d4;
  long newkey = key->data[0] ^ key->data[1];
  int i;
  int verrev;
#ifndef RELEASE_VERSION
  int handdebug = (int) l_real_getenv("HANDDEBUG");
#endif
  typedef int (*_user_crypt_filter)
			lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int , int));
  _user_crypt_filter filter = 0;
  extern int (*L_NEW_JOB)();
  typedef int (*_handshake_filter)
			lm_args((LM_VOID_PTR, LM_VOID_PTR, int , 
				unsigned char *, int, int *));
  _handshake_filter hfilter = 0;

	d = d2 = d3 = d4 = 0;
	memset(resp, 0, LM_MSG_LEN+1);
	if (lm_job->keymem2 /* P5798 */
	 && (user->flexlm_ver < 7))
		filter = (_user_crypt_filter)lm_job->keymem2;
	else if (lm_job->user_crypt_filter)
		filter = (_user_crypt_filter)lm_job->user_crypt_filter;
	else if (msg[MSG_HAND_CRYPT] == '2')
		hfilter = (_handshake_filter)L_NEW_JOB;
				
	HANDDEBUG(("\nnewkey  %x, ", newkey));

	if (filter || hfilter)
	{
		for (i = 0; i < 4 ; i ++)
		{
			uc = (newkey >> (i * 8)) & 0xff;
			if (lm_job->user_crypt_filter)
				(*filter)(lm_job, &uc, i, 0);
			else
			{
				(*hfilter)(0, 0, 1, &uc, i, 0);
			}
			newkey ^= (uc << (i * 8));
		}
	}
	HANDDEBUG(("%x\n", newkey));

	if (user->flexlm_ver >= 7)
		l_decode_32bit_packed(&msg[MSG_HAND_GROUP_ID], &user->group_id);
	else
		user->group_id = NO_GROUP_ID;

/*
 *	Get the time from the handshake message.  Modify it, and send it
 *	back.  The time is our comm encryption seed.
 */
	decode(&d, &msg[MSG_HAND_DATA], newkey, user, junk);
	verrev = (user->flexlm_ver * 100) + user->flexlm_rev;
	if ((verrev > 575))
	{
		decode(&d2, &msg[MSG_HAND_DATA2], newkey, user, junk2);
		decode(&d3, &msg[MSG_HAND_DATA3], newkey, user, junk3);
		decode(&d4, &msg[MSG_HAND_DATA4], newkey, user, junk4);
	}
/*-	P2477 */
 
#ifndef RELEASE_VERSION
/*
 *	Dump out what we got, if the debug flag is on
 */
	HANDDEBUG(("Got: %lx\ngot:      ", d));
	for (i=0; i<MAX_HAND_DATA_LEN; i++) 
	{
		HANDDEBUG(("%x ", 0xff & msg[MSG_HAND_DATA+i]));
	}
	HANDDEBUG(("\nmodified: "));
	for (i=0; i<MAX_HAND_DATA_LEN; i++) 
	{
		HANDDEBUG(("%x ", 0xff & junk[i]));
	}
#endif
/*
 *	Now, build the handshake message to send back.
 */
	resp[MSG_HAND_CRYPT] = '1';
	if ((verrev > 575))
	{
		encode(&resp[MSG_HAND_DATA2], newkey, user->flexlm_ver, junk2);
		encode(&resp[MSG_HAND_DATA3], newkey, user->flexlm_ver, junk3);
		encode(&resp[MSG_HAND_DATA4], newkey, user->flexlm_ver, junk4);
	}
	encode(&resp[MSG_HAND_DATA], newkey, user->flexlm_ver, junk);
#ifndef RELEASE_VERSION
/*
 *	Tell them what we are sending back.
 */
	HANDDEBUG(("\nsending:  "));
	for (i=0; i<MAX_HAND_DATA_LEN; i++) 
	{
		HANDDEBUG(("%x ", 0xff & junk[i]));
	}
	HANDDEBUG(("\n"));
#endif
	ls_client_send(user, LM_HANDSHAKE, resp);

	user->encryption = d;
	return(user->encryption == 0);
}
static
void
decode(d, buf, key, user, junk)
long *d;
char *buf;
long key;
CLIENT_DATA *user;	
char *junk;
{
  int i;
	l_str_dcrypt(buf, MAX_HAND_DATA_LEN, key, user->flexlm_ver >= 5);
	for (i=0; (i < MAX_HAND_DATA_LEN) && buf[i]; i++)
	{
	  int k = buf[i];
		if (!(isxdigit(buf[i])))
		{
			*d = 0x11111111; /*- guaranteed bad handshake */
			LOG((lmtext("Bad handshake detected with %s %s %s\n"),
				user->name, user->node, user->display));
			break;
		}
	}
	if (!*d) l_decode_long_hex(buf, d);
	if (!*d) *d = 0x11111111; /*- bogus */
	memset(junk, 0, MAX_HAND_DATA_LEN+1);
	l_encode_long_hex(junk, l_modify(*d));
}
static
void
encode(buf, newkey, client_ver, junk)
char *buf;
long newkey;
int client_ver;
char *junk;
{
	memcpy(buf, 
		l_str_crypt(junk, MAX_HAND_DATA_LEN, newkey, client_ver >= 5), 
		MAX_HAND_DATA_LEN);
}
