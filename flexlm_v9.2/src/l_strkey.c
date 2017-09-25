/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Inc.

 *****************************************************************************/
/*	
 *	Module: $Id: l_strkey.c,v 1.53 2003/04/16 17:26:37 brolland Exp $
 *
 *	Function:	l_string_key
 *
 *			This file is always included into other C source
 *			files for security reasons.  It's always a static
 *			function
 *
 *			It must be used with "#include l_strkey" at the
 *	 		top of the file.
 *
 *	M. Christiano
 *	12/21/96
 *
 *	Last changed:  9/9/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lmachdep.h"
#include "lmclient.h"
#include "l_strkey.h"
#include "l_rand.h"
static void our_encrypt2 lm_args(( unsigned char *));
static unsigned int reverse_bits lm_args((int ));

/*-
 *	Turn a string into a L_STRKEY_BLOCKSIZE key
 */
static
unsigned char * API_ENTRY

#ifdef LM_CKOUT

l_string_key(job, input, inputlen, code, len, license_key)
LM_HANDLE *job;
unsigned char *input;
int inputlen;
VENDORCODE *code;	/*- Vendor's "special" code */
unsigned long len;
char *license_key;

#else

l_string_key(job, input, inputlen, code, len)
LM_HANDLE *job;
unsigned char *input;
int inputlen;
VENDORCODE *code;	/*- Vendor's "special" code */
unsigned long len;

#endif /* LM_CKOUT */
{
  int i, j, k;
  unsigned char *p, *q;
  int length;
  unsigned char *newinput;
  int xor_arr[XOR_SEEDS_ARRAY_SIZ][4];
  int idx = (*job->vendor) % XOR_SEEDS_ARRAY_SIZ;
  int demo = 0;

#define MEMBUF (job ? job->mem_ptr1 : membuf)
#ifdef FLEXLM_ULTRALITE
  unsigned char *y = (unsigned char *) LM_FUL_Y_PTR;
  unsigned char *membuf = (unsigned char *) LM_FUL_MEMBUF_PTR;
  
#else
  static unsigned char y[L_STRKEY_BLOCKSIZE];	/*- output ciphertext */
  static unsigned char *membuf = 0;
  LM_KEYLIST *kl = job->lc_this_keylist;
  L_KEY_FILTER *kf = 0;
#endif
  unsigned char * ret = 0;
#ifdef LM_CKOUT
  char lkey[MAX_CRYPT_LEN + 1];
  int keylen;
#endif
  typedef char * (* user_func) lm_args((LM_VOID_PTR /* LM_HANDLE_PTR  */ ,
					    CONFIG_PTR , LM_CHAR_PTR ,
					    VENDORCODE_PTR ));	
  typedef int (*_user_crypt_filter)
			lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int , int));
  typedef void (*_user_crypt_filter_gen)
		lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int ));
  _user_crypt_filter user_crypt_filter = 0;
  _user_crypt_filter_gen user_crypt_filter_gen = 0;

#ifndef FLEXLM_ULTRALITE
	/******************************************************************
	 * kmaclean 11/27/02  ifdef'ed this out for ultra lite.  
	 *****************************************************************/ 
	
	/*
	 *	It's a demo if it's a demo vendor key, OR the vendor name is
	 *	demo and it's the VCG.  It's the VCG if WRAPPER is turned on.
	 *	It's important that it's *not* demo if it's the testsuite,
	 *	and the vendor name is demo in the testsuite, but the
	 *	testsuite doesn't have WRAPPER turned on.
	 */
	if (((l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL) &&
			l_getattr(job, NO_EXPIRE) != NO_EXPIRE_VAL) ||
			((*job->vendor && !strcmp(job->vendor, "demo")) &&
			(l_getattr(job, WRAPPER) == WRAPPER_VAL)))
		demo = 1;

#ifndef LM_CRYPT_HASH
#ifdef LM_CKOUT
	if (!(job->flags & LM_FLAG_MAKE_OLD_KEY) && job->L_SIGN_LEVEL
		&&  (job->L_STRENGTH_OVERRIDE != (char *)LM_STRENGTH_DEFAULT))
	{
		for (kf = (L_KEY_FILTER *)job->key_filters; kf; kf = kf->next)
		{
			if (!kl)
                break; /* use first one */
			if (kf->sign_level == job->lc_this_keylist->sign_level)
				break;
		}
		if (kf)
			user_crypt_filter = (_user_crypt_filter)kf->app_filter;
	}
	else if (job->user_crypt_filter)
		user_crypt_filter = job->user_crypt_filter;
#else
	if (job->lc_this_keylist
		&&  (job->L_STRENGTH_OVERRIDE != (char *)LM_STRENGTH_DEFAULT))
	{
		for (kf = (L_KEY_FILTER *)job->key_filters; kf; kf = kf->next)
		{
			if (!kl)
                break; /* use first one */
			if (kf->sign_level == job->lc_this_keylist->sign_level)
				break;
		}
		if (kf)
			user_crypt_filter_gen = 
				(_user_crypt_filter_gen)kf->gen_filter;
	}
	else if (job->user_crypt_filter_gen)
		user_crypt_filter_gen = job->user_crypt_filter_gen;

#endif
	if (kf && (kf->flags & LM_KF_DIG_SIG) && 
		(kf->app_filter || kf->gen_filter)
		&& (job->L_STRENGTH_OVERRIDE != (char *)LM_STRENGTH_DEFAULT))
	{
#ifdef LM_CKOUT
	  LM_DS_AUTH app_filter = (LM_DS_AUTH)kf->app_filter;
	  unsigned char *sig;
	  int keylen = strlen(license_key);
	  int status;
	  unsigned int ii, b, jj, kk;
	  char s[3];

		sig = (unsigned char *)l_malloc(job, (keylen/2) + 1);
		s[2] = 0;
		for (kk = 0, jj = 0, ii = 0; license_key[ii]; ii++)
		{
			if (license_key[ii] == ' ')
                continue;
			s[jj++] = license_key[ii];
			if (jj == 2)
			{
				jj = 0;
				sscanf(s, "%02X", &b);	/* overrun checked */
				sig[kk++] = b;
			}
		}

		status = (*app_filter)((char *)job, input, inputlen, sig, kk,
			&kf->pubkeysize[0], kf->pubkey, kf->pubkeyinfo1, kf->sign_level);
				
		free(sig);
		if (status)
			ret = 0; /* failed */
			
		else
			ret = (unsigned char *)"ok";
		goto exit_l_string_key;
#else /* else: license generation */
	  static unsigned char *retval = 0;
	  unsigned char *buf;
	  static int prtlen = L_KEY_BYTE_TO_STRLEN(L_MAXPUBKEYSIZ);
	  static int len = L_MAXPUBKEYSIZ;
	  int olen;
	  int i;
	  LM_DS_GEN gen_filter = (LM_DS_GEN)kf->gen_filter;
	  unsigned char *ocp;

		if (!retval)
            retval = (unsigned char *)l_malloc(job, prtlen); /* ascii */
		buf = (unsigned char *)l_malloc(job, len); /* binary -- half len */
		olen = len;
		kf->pubkeyinfo->strength = code->strength;
		if ((*gen_filter)((char *)job, input, inputlen, (char *)kf->pubkeyinfo, 
			buf, &len))
		{
			if (len != olen)
			{
				l_free(retval);
				l_free(buf);
				retval = (unsigned char *)l_malloc(job, prtlen);
				buf = (unsigned char *)l_malloc(job, len);
				if ((*gen_filter)((char *)job, input, inputlen, (char *)kf->pubkeyinfo, 
									buf, &len))
				{
					ret = 0;
					goto exit_l_string_key;
				}
			}
			else
			{
				ret = 0;
				goto exit_l_string_key;
				
			}
		}
		for (ocp = retval, i = 0; i < len; i++)
		{
			if (!(i%2) && i)
                *ocp++ = ' ';
			sprintf((char *)ocp, "%02X", buf[i]);
			ocp+=2;
		}
		*ocp = 0;
		l_free(buf);
		ret = retval;
		goto exit_l_string_key;
#endif
	}
#endif /* LM_CRYPT_HASH */
#else
	/* For ULTRALITE we still need to know if it is a demo */
	if (*job->vendor && strcmp(job->vendor, "demo") == 0)
		demo = 1;

#endif /* FLEXLM_ULTRALITE */
	memset(y, 0, L_STRKEY_BLOCKSIZE);
	length = (inputlen) / L_STRKEY_BLOCKSIZE;
	XOR_SEEDS_INIT_ARRAY(xor_arr)

/*-
 *	V2.38: optimize: malloc once on the first pass, and leave the
 *		malloced data around.  This also avoids a malloc while
 *		we are in code called by lm_timer() (P329).
 *
 *	V2.4a: Fix the optimization by saving the malloc'd ptr in "membuf"
 *		(P359)
 */
	if (job->mem_ptr1_siz == 0)
	{
		job->mem_ptr1_siz = (int)(MAXINPUTLEN + L_STRKEY_BLOCKSIZE);
		newinput = job->mem_ptr1 = (unsigned char *) 
					l_malloc(job, job->mem_ptr1_siz);
	}

/*-
 *	If we don't have a multiple of the blocksize, just
 *	malloc a little more and copy it in.
 */

#ifdef LM_CKOUT /* make sure that license_key is reasonable */
	if (!license_key)
        return 0;
	keylen = strlen(license_key);
	if ((keylen < 12) || (keylen > MAX_CRYPT_LEN))
		return 0;
	if (keylen != 12)
        len = L_SECLEN_LONG;
	for (i = 0; license_key[i] ; i++)
		if (!isxdigit(license_key[i]))
			return 0;
	strcpy(lkey, license_key);
	l_uppercase(lkey);
	if (keylen == MAX_CRYPT_LEN) /* remove start date */
	{
		lkey[1] = lkey[2];
		lkey[2] = lkey[4];
		lkey[3] = lkey[6];
		lkey[4] = lkey[8];
		lkey[5] = lkey[9];
		lkey[6] = lkey[10];
		lkey[7] = lkey[11];
		lkey[8] = lkey[12];
		lkey[9] = lkey[13];
		lkey[10] = lkey[14];
		lkey[11] = lkey[15];
		lkey[12] = lkey[16];
		lkey[13] = lkey[17];
		lkey[14] = lkey[18];
		lkey[15] = lkey[19];
		lkey[16] = 0;
	}
#endif /* LM_CKOUT */
	
	

	if (inputlen % L_STRKEY_BLOCKSIZE)
	{
		length++;
		newinput = MEMBUF;

		if (job->mem_ptr1_siz < (int)(inputlen + L_STRKEY_BLOCKSIZE))
/*-
 *		This shouldn't ever happen, but if it does, just malloc
 *		the new length.
 */
		{
			(void) free((char *) MEMBUF);
			job->mem_ptr1_siz = (int)(inputlen + L_STRKEY_BLOCKSIZE);
			if (job) 
				newinput = job->mem_ptr1 = 
				(unsigned char *) malloc(job->mem_ptr1_siz);
			else
				newinput = membuf =
				(unsigned char *) malloc(job->mem_ptr1_siz);
		}
		if (newinput == (unsigned char *)NULL)
		{
			return(NULL);
		}
		memset(newinput, 0, inputlen + L_STRKEY_BLOCKSIZE);
		memcpy(newinput, input, inputlen);
	}
	else
	{
		newinput = input;
	}
/*-
 *	Ok, "newinput" is the string to be encrypted, "code" is the
 *	initializing vector, and we will take the last block of
 *	cyphertext as the output.
 *	Use block chaining with cyphertext feedback.
 */
	p = newinput;
	for (i = 0; i < length; i++)
	{
		XOR(p, y, y);	/*- encrypt (input XOR last output) */
#ifndef LM_CRYPT_HASH
/*-
 *		Added in v6.1 -- Minimize visibility of seeds.
 *		1) The seeds do appear in raw form as args to l_movelong
 *		   in the this part.  That's the only place they appear.
 *		   Unfortunately, the l_movelong algorithm, aside from
 *		   being not exactly correct, does odd arithmetic which
 *		   *requires* the actual long seed to appear.  There's no
 *		   way around this (short of changing the algorithm).
 *		2) l_movelong now does an XOR.  In all other
 *		   cases l_movelong XOR's over space which is zeroed, therefore
 *		   equivalent to assignment.  In this case, it XORs over the
 *		   first 8 bytes of the input str.  This way, the seeds
 *		   don't appear raw in the y[] buffer.
 */
		if (i == 0)
		{
			if (!user_crypt_filter && 
				!user_crypt_filter_gen &&
				(job->flags & LM_FLAG_MAKE_OLD_KEY))
			{
/*- 
 *				This is compatible with pre-v6.1 
 *				rewritten as a macro to further hide seeds
 */
				q = y; 
				L_MOVELONG(code->data[0] ^ 
				((long)(job->SEEDS_XOR[xor_arr[idx][0]])<<0)
				^((long)(job->SEEDS_XOR[xor_arr[idx][1]])<<8) 
				^((long)(job->SEEDS_XOR[xor_arr[idx][2]])<<16)
				^((long)(job->SEEDS_XOR[xor_arr[idx][3]])<<24),
					q)
				L_MOVELONG(code->data[1] ^
				((long)(job->SEEDS_XOR[xor_arr[idx][0]])<<0)
				^((long)(job->SEEDS_XOR[xor_arr[idx][1]])<<8) 
				^((long)(job->SEEDS_XOR[xor_arr[idx][2]])<<16)
				^((long)(job->SEEDS_XOR[xor_arr[idx][3]])<<24),
					q)
			}
			else
			{
/*-
 *				This version hides the seeds more than
 *				the previous version, *but* it is *subtly* not
 *				compatible with pre-v6.1 license keys.
 *				So we only use it when they use the 
 *				key-filter, since this is not backwards
 *				compatible anyway...
 */
			  	for (k = 0; k < L_STRKEY_BLOCKSIZE; k++)
				{
				  int shift = ((k%4) * 8);
				  unsigned long mask = 0xffL << shift;
/*-
 *					xor in one byte at a time... 
 *					What is xor'd in is one byte
 *					of the 'raw' seeds
 */
					y[k] ^= 
					(((code->data[k/4] & mask) >> shift) 
					^ job->SEEDS_XOR[xor_arr[idx][k%4]]);

					if (!(job->flags&LM_FLAG_MAKE_OLD_KEY))
					{
						y[k] = reverse_bits(y[k]);
					}
				}
			}
		}
		if (!(job->flags &LM_FLAG_MAKE_OLD_KEY) && !demo)
			our_encrypt2(y);
		else
			our_encrypt(y);
#else
		our_encrypt2(y);
#endif /* LM_CRYPT_HASH */
		p += L_STRKEY_BLOCKSIZE;
	}
	if (len == L_SECLEN_SHORT)
	{
		y[7] = (unsigned char) reverse_bits((int)y[7]) & 0xff;
		y[6] = (unsigned char) reverse_bits((int)y[6]) & 0xff;
		y[0] = (unsigned char) ((int) y[7] + (int) y[0]) & 0xff;
		y[1] = (unsigned char) ((int) y[6] + (int) y[1]) & 0xff;
		y[6] = y[7] = 0;

	}
#ifdef LM_CKOUT
	j = L_STRKEY_BLOCKSIZE;
	if (len == L_SECLEN_SHORT)
        j -= 2;
	for (i = 0; i < j; i++)
	{
	  unsigned char x;
	  char c;


		c = lkey[i * 2];
		if (isdigit(c))
            x = (c - '0') << 4;
		else
            x = ((c - 'A') + 10) << 4;

		c = lkey[(i * 2) + 1];
		if (isdigit(c))
            x += (c - '0');
		else
            x += ((c - 'A') + 10);

		if (user_crypt_filter)
			(*user_crypt_filter)(job, & x, i, y[i]);
		if (x != y[i])
            return 0;
	}
#else
#ifndef LM_CRYPT_HASH
	if (user_crypt_filter_gen)
	{
		j = L_STRKEY_BLOCKSIZE;
		if (len == L_SECLEN_SHORT)
            j -= 2;
		for (i = 0; i < j; i++)
			(*user_crypt_filter_gen)(job, &y[i], i);
	}
#endif /* LM_CRYPT_HASH */
#endif /* LM_CKOUT */

	ret = (atox(job, y, len));

exit_l_string_key:

	return ret;
	
}

/*-
 *	Move a longword to the source string (Don't move just a bunch of 0s)
 */

static unsigned char *
l_movelong(ldata, p)
long ldata;
unsigned char *p;
{
	ldata = signed32(ldata); /*- P2249 */
	
	*p++ ^= (unsigned char) (ldata & 0xff);
	if (ldata > 255 || ldata < -256)
		*p++ ^= (unsigned char) ((ldata >> 8) & 0xff);
	if (ldata > 32000 || ldata < -32000)
		*p++ ^= (unsigned char) (((ldata) >> 16) & 0xff);
	if (ldata > 16000000 || ldata < -16000000)
		*p++ ^= (unsigned char) ((ldata >> 24) & 0xff);
	return (p);
}

static
long
signed32(l)
long l;
{
#ifdef LM_LONG_64BIT

        if (sizeof(long) == 8   &&       /*- 64-bit system */
                        (l & 0x80000000)) /*- 32-bit sign bit */
                l |= 0xffffffff00000000;        /*-sign extend it */
#endif /* PC */

        return l;
}


/*-
 *	atox() - Turn an 8-byte binary string into a hex string
 */
static
unsigned char *
atox(job, s, len) 
LM_HANDLE *job;
unsigned char *s;
unsigned long len;
{
  int isshort = (len == L_SECLEN_SHORT);
  int i;
#ifdef FLEXLM_ULTRALITE
  unsigned char *result = (unsigned char *) LM_FUL_RESULT2_PTR;
  unsigned char *hex = (unsigned char *) "0123456789ABCDEF";
  char *r = result;
#else
  static unsigned char hex[] = "0123456789ABCDEF";
  char *r; 
  unsigned char *result;
#endif

#ifndef FLEXLM_ULTRALITE
	if (job->keymem) free(job->keymem);
	r = job->keymem = (char *)l_malloc(job, L_STRKEY_RESULT_LEN);
	result  = (unsigned char *) r;
#endif
	for (i=0; i<8; i++)
	{
		*r++ = hex[(*s >> 4) & 0xf];
		*r++ = hex[*s & 0xf];
		s++;
	}
	if (isshort) result[12] = 0; /* truncate */
	return(result);
}
static
unsigned int
reverse_bits(c)
int c;
{
  unsigned char ret = 0;
  int i, k;

        for (k = 7, i = 0 ; i < 8; i++, k--)
        {
                ret |= ((c & (1<<i)) ? (1 << k) : 0) ;
        }
	return (unsigned int) ret;

}
/*-
 *	our_encrypt() - Encrypt L_STRKEY_BLOCKSIZE bytes, in place
 */

static
void
our_encrypt(s)
unsigned char *s;
{
  unsigned char t0 = s[0];
  unsigned char t1 = s[1];
  unsigned char t2 = s[2];
  unsigned char t3 = s[3];

	s[0] = (unsigned char) ((int) s[7] + (int) s[4]) & 0xff;
	s[1] = (unsigned char) ((int) s[5] + (int) t3) & 0xff;
	s[2] = (unsigned char) (~((int) s[4] + (int) t2)) & 0xff;
	s[3] = (unsigned char) ((int) t0 + (int) s[5]) & 0xff;
	s[4] = (unsigned char) ((int) s[6] + (int) t3) & 0xff;
	s[5] = (unsigned char) (~((int) t1 + (int) s[6])) & 0xff;
	s[6] = (unsigned char) ((int) t0 + (int) s[7]) & 0xff;
	s[7] = (unsigned char) ((int) t1 + (int) t2) & 0xff;
}
/*-
 *	New hash/encrypt algorithm introduced in v7.1 for KEY2
 *	Should give a more unique result
 */


static
void
our_encrypt2(input) 
unsigned char *input;
{
  int i, x1, x2, hir1, r1, r2;
  int seeds[3];

	srand16( input[0] + ((input[1] << 8) & 0xff00), 
		input[2] + ((input[3] << 8) & 0xff00), 
		input[4] + ((input[5] << 8) & 0xff00), 
			seeds);
	x1 = rand16(seeds, 0);
	x2 = rand16(seeds, 0);
	srand16 (x1, x2, input[6] + ((input[7] << 8) & 0xff00), seeds);
	r1 = rand16(seeds, 0);
	x1 = rand16(seeds, 0);
	x2 = rand16(seeds, 0);
	srand16 (r1, x1, x2, seeds);
	hir1 = rand16(seeds, 0);
	r1 += (hir1 << 16);
	x1 = rand16(seeds, 0);
	x2 = rand16(seeds, 0);
	srand16 (hir1, x1, x2, seeds);
	r2 = rand16(seeds, 0);
	r2 += (rand16(seeds, 0) << 16);
	for (i = 0; i < 4; i++)
	{
		input[i] = reverse_bits((r1 >> (i * 8)) & 0xff);
	}
	for (i = 0; i < 4; i++)
	{
		input[i+4] = reverse_bits((r2 >> (i * 8)) & 0xff);
	}
}


