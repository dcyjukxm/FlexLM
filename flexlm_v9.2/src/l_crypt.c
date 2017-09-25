/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *
 *	Module: $Id: l_crypt.c,v 1.51 2003/06/16 20:13:23 sluu Exp $
 *	Function: lc_crypt(job, conf, sdate, code) 
 *
 *	Description: Generates an encryption value for a feature.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(CONFIG *) conf 	- config struct
 *			(char *) sdate		- Start date
 *			(VENDORCODE *) code 	- Vendor's "special" code
 *
 *	Return:		(char *) - The encrypted code, which should be in
 *				   the license file.
 *
 *	M. Christiano
 *	1/31/90 - Adaped from l_oldcrypt()
 *
 *	Last changed:  12/30/98
 *
 *
 */

 /*-
  *		All global functions, except l_crypt_private, have to
  *		be grouped with #ifdef LM_CRYPT, so it doesn't
  *		get compiled twice
  */

#define L_CRYPT_C
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "l_strkey.h"
#include <stdio.h>
#include <errno.h>

static unsigned char *add lm_args((LM_HANDLE *, char *, char *, 
						unsigned char *));
static unsigned char *addi lm_args((int i, char *d, unsigned char *o));
static unsigned char * add_hostidstr lm_args(( LM_HANDLE *, char *, char *, 
						unsigned char *));
static unsigned char * move_in_one_hostid lm_args(( LM_HANDLE *, HOSTID *, 
						unsigned char *));
static unsigned char * movein_date lm_args(( char *, unsigned char *));
static long l_sernum_to_32 lm_args((char *));
static int valid_code lm_args((char *));
static char * real_crypt lm_args(( LM_HANDLE *, CONFIG *, char *, 
						VENDORCODE *));
static unsigned char * shuffle lm_args((LM_HANDLE *, unsigned char *, 
					unsigned char *));

static void get_ver 	lm_args((char *, long *, long*));
static swap lm_args(( LM_HANDLE *job, LM_SERVER **slist, int pos));
static unsigned char * move_in_hostid lm_args((LM_HANDLE *, HOSTID *id, unsigned char *p));

#if defined(PC) 
typedef char * (LM_CALLBACK_TYPE * LM_CALLBACK_CRYPT)(LM_HANDLE *job,
                                     CONFIG *conf,
                                     char *sdate,
                                     VENDORCODE *code);
#define LM_CALLBACK_CRYPT_TYPE (LM_CALLBACK_CRYPT)
#else
#define LM_CALLBACK_CRYPT_TYPE
#endif


/*
 *	Some constants for flags going into the string
 */

#define ANY_CODE         ((long) 0xab370fd2)
#define USER_CODE        ((long) 0xba1584a2)
#define DISPLAY_CODE     ((long) 0xab8543cc)
#define HOSTNAME_CODE    ((long) 0xbadaef01)
#define HOSTID_DOMAIN_CODE    ((long) 0x49D654F9)
#define HOSTID_COMPOSITE_CODE	((long) 0x2388341c)
#define HOSTID_METER_BORROW_CODE    ((long) 0x4aa87607)
#define HOSTID_VENDOR_CODE ((long) 0x12fe93a)
#define NO_EXTENDED_CODE ((long) 0x74ab99)
#define DEMO_CODE        ((long) 0x122345a)
#define FLEXLOCK_CODE   ((long) 0x1c8c6f21)
#define FLEXID1_CODE	((long) 0x61420b1)
#define FLEXID2_CODE	((long) 0x19481954)
#define FLEXID3_CODE	((long) 0x29ab7264)
#define FLEXID4_CODE	((long) 0x01181954)
#define FLEXID_FILE_CODE	((long) 0x8adf678a)
#define DISK_SERIAL_NUM_CODE	((long) 0x78131a7c)
#define ID_STRING_CODE	((long) 0xaabb007c)
#define SERNUM_ID_STRING_CODE	((long) 0x5c7b549c)
#define INTERNET_CODE	((long) 0x7c7cfea0)
			
#define INCREMENT_FLAG	 ((long) 0xd00128ef)
#define PACKAGE_FLAG	 ((long) 0x038ddeed)
#define UPGRADE_FLAG	 ((long) 0x11052f73)
#define NO_START_DATE_FLAG ((long) 0x73D0c587)


#ifdef LM_CKOUT
static
#endif /* LM_CKOUT */
char * 
#ifdef LM_CRYPT 
API_ENTRY
#endif /* LM_CRYPT */
l_crypt_private(
	LM_HANDLE *	job,		/* Current license job */
	CONFIG *	conf,		/* CONFIG struct for this feature */
	char *		sdate,		/* Start date encoded as 4 chars */
	VENDORCODE *code)	/* Vendor's "special" code */
{
	char *ret = NULL;
	int not_eq = 0;

	
	ret = real_crypt(job, conf, sdate, code);
#ifdef LM_CKOUT

	if (
		!job->user_crypt_filter && 
		!(job->lc_this_keylist) &&
		valid_code( conf->code))
	{
		if (job->flags & LM_FLAG_MAKE_OLD_KEY)
		{
			STRNCMP( conf->code,
			ret, MAX_CRYPT_LEN, not_eq);
		}
		else
		{
			STRNCMP(conf->lc_sign, 
				ret, MAX_CRYPT_LEN, not_eq);
		}
		if (not_eq && !(job->options->flags & 
			LM_OPTFLAG_STRINGS_CASE_SENSITIVE))
		{
			job->options->flags |= 
				LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			ret = real_crypt(job, conf, sdate, code);
			job->options->flags &= 
				~LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
		}
	}
#endif /* LM_CKOUT */
	return ret;
}

/*-
 *	shuffle() - Shuffle 4 bytes of c2 into c1
 */
static
unsigned char *
shuffle(
	LM_HANDLE *		job,
	unsigned char *	c1,
	unsigned char *	c2)
{
  int i = 0;
#ifdef FLEXLM_ULTRALITE
  unsigned char *result= (unsigned char *) LM_FUL_RESULT_PTR;
  unsigned char *r = result;
#else
  unsigned char *result = NULL;
  unsigned char *r = NULL;

	result = (unsigned char *)l_malloc(job, strlen((char *)c1) + 
					strlen((char *)c2) + 1);
	r = result;
#endif /* ULTRALITE */


	if (c1 != (unsigned char *) NULL && c2 != (unsigned char *) NULL)
	{
		for (i = 0; i < 4; i++)
		{
			*r++ = *c1++;
			*r++ = *c2++;
		}
	}
	while (*c1) *r++ = *c1++;
	*r = '\0';
#ifndef FLEXLM_ULTRALITE
	l_free(job->keymem);
	job->keymem = (char *)result;
#endif
	return(result);
}
	

	

static
char *
real_crypt(
	LM_HANDLE *	job,		/* Current license job */
	CONFIG *	conf,		/* CONFIG struct for this feature */
	char *		sdate,		/* Start date encoded as 4 chars */
	VENDORCODE *code)		/* Vendor's "special" code */
{
	unsigned char input_str[MAXINPUTLEN + 1] = {'\0'};
	int i = 0;
	unsigned char *p = input_str;
	long ver = 0;
	long from_ver = 0;
	long fver = 0;	/*- Fractional part of version */
	long from_fver = 0;	/*- Fractional part of from version */
	long users = conf->users;
	char *start_date = sdate;
	char *ret = NULL;
	int use_sdate = 0;
	unsigned long seclen = 0; 
	char *exp_code = 0;
	long sav_strength_override = 0;

  	if ((job->flags & LM_FLAG_MAKE_OLD_KEY) || !conf->lc_keylist)  
		exp_code = conf->code;
	else 
	{
		LM_KEYLIST *kl, *jkl =  job->lc_this_keylist;
		for (kl = conf->lc_keylist; kl ; kl = kl->next)
		{
			if (!jkl)
				break; /* use first one */
			if (kl->sign_level == jkl->sign_level)
				break;
		}
		if (kl)
			exp_code = kl->key;
	}
	memset(input_str, 0, sizeof(input_str));
#ifdef LM_CRYPT
	if (l_check_fmt_ver(job, conf))
	{
		return 0;
	}

#endif
		

	get_ver(conf->version, &ver, &fver);

#ifndef FLEXLM_ULTRALITE
	if (job->options->user_crypt &&
		l_getattr(job, LICENSE_SEEDS)==LICENSE_SEEDS_VAL) 
	{
	    return((*LM_CALLBACK_CRYPT_TYPE
		    job->options->user_crypt)(job, conf, sdate, code));
	}
	if (!L_SECLEN_OK)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 361, 0);
		return 0;
	}
#endif	/* !FLEXLM_ULTRALITE */

/*-
 *	if we're using short, but the key is long, ok
 *	if we're not using start-date, but the key does, ok
 */
	seclen = job->options->sf;
	if ((seclen == L_SECLEN_SHORT) && ((valid_code(exp_code) && 
		((int)strlen(exp_code) > 12) ) ||
		(job->flags & LM_FLAG_CKSUM_ONLY)))
	{
		seclen = L_SECLEN_LONG;
	}
	if ((((job->options->flags & LM_OPTFLAG_LKEY_START_DATE) ||
		(valid_code(exp_code) && (strlen(exp_code) == MAX_CRYPT_LEN)))
		|| (job->flags & LM_FLAG_CKSUM_ONLY)) && 
		(!conf->lc_keylist || job->flags & LM_FLAG_MAKE_OLD_KEY))
	{
		use_sdate = 1;
	}


/*-
 *	Put all the data to be encrypted into one long string
 */

	if (!l_good_bin_date(sdate))
	{
		LM_SET_ERRNO(job, LM_BADDATE, 239, 0);
		return 0;
	}

	p = move_in_hostid(job, conf->idptr, p);
	/*
	 *	Check to make sure this is really a INCREMENT as opposed to a FEATURE line
	 *	with ls_use_all_feature_lines set to 1.
	 */
	if ( (conf->type == CONFIG_INCREMENT) && (!(conf->lc_options_mask & LM_OPT_ISFEAT)) )
		p = l_movelong(INCREMENT_FLAG, p);
	if (conf->type == CONFIG_PACKAGE)
		p = l_movelong(PACKAGE_FLAG, p);
	if (conf->type == CONFIG_UPGRADE)    
	{
		get_ver(conf->fromversion, &from_ver, &from_fver);
		p = l_movelong(UPGRADE_FLAG, p);
		p = l_movelong(from_ver, p);
		p = l_movelong(from_fver, p);
	}
	p = l_movelong(users, p);
#ifndef FLEXLM_ULTRALITE	/* No servers in ULTRALITE */
/*- 
 * 	Multi-user license, get servers' data 
 *	METER_BORROW: server hostid is not authenticated
 */
	if (users > 0 )
	{			
		LM_SERVER *ptr = NULL, *slist[MAX_SERVERS+1]; /*- Sorted List of hosts */
		int count = 0;
		int notsorted = 1;

		for (ptr = conf->server; ptr; ptr = ptr->next) 
		{
			slist[count] = ptr;
			count++;
		}
		for (; count <= MAX_SERVERS; count++)
			slist[count] = (LM_SERVER *) NULL;

		while (notsorted)
		{
			notsorted = 0;
			for (i=0; i<count; i++)
				if (slist[i] && swap(job, slist, i)) 
					notsorted = 1;
		}

#ifdef SUPPORT_METER_BORROW
		if (!(conf->lc_type_mask & LM_TYPE_METER_BORROW))
#endif /* SUPPORT_METER_BORROW */
		{
			for (count = 0; ptr = slist[count]; count++)
			{
				p = move_in_hostid(job, ptr->idptr, p);
			}
		}
	}
#endif	/* !FLEXLM_ULTRALITE */
	
	p = add(job, conf->feature, 0, p);

	p = l_movelong(ver, p);
	p = l_movelong(fver, p);

	if (conf->lc_vendor_def)
	{
		p = add(job, conf->lc_vendor_def, 0, p);
	}
	p = movein_date(conf->date, p);


	if (use_sdate)
	{
		if (!start_date || (int)strlen(start_date) < 4) 
			start_date = sdate = "FFFF";
		while (*start_date)
			*p++ = *start_date++;
	}
	else
		p = l_movelong(NO_START_DATE_FLAG, p);

#ifndef FLEXLM_ULTRALITE	/* Ultralite doesn't use any of this stuff */
/*
 *	Now, get all the optional stuff, from v3.0
 */
	if (conf->lc_got_options & LM_LICENSE_LINGER_PRESENT)
		p = addi(conf->lc_linger, LM_LICENSE_LINGER_STRING, p);
	if (conf->lc_got_options & LM_LICENSE_DUP_PRESENT)
		p = addi(conf->lc_dup_group, LM_LICENSE_DUP_STRING, p);
	if (conf->lc_got_options & LM_LICENSE_SUITE_DUP_PRESENT)
		p = addi(conf->lc_suite_dup, LM_LICENSE_SUITE_DUP_STRING, p);
	if (conf->lc_got_options & LM_LICENSE_WLOSS_PRESENT)
		p = addi(conf->lc_w_loss, LM_LICENSE_WLOSS_STRING, p);
	if (conf->lc_got_options & LM_LICENSE_OVERDRAFT_PRESENT)
		p = addi(conf->lc_overdraft, LM_LICENSE_OVERDRAFT_STRING, p);
	if (conf->lc_got_options & LM_LICENSE_TYPE_PRESENT)
	{
		p = addi(conf->lc_type_mask, TYPESTR, p);
		if (conf->lc_type_mask & LM_TYPE_USER_BASED)
			p = addi(conf->lc_user_based, LM_LICENSE_USER_BASED, p);
		if (conf->lc_type_mask & LM_TYPE_HOST_BASED)
			p = addi(conf->lc_host_based, LM_LICENSE_HOST_BASED, p);
		if (conf->lc_type_mask & LM_TYPE_MINIMUM)
			p = addi(conf->lc_minimum, LM_LICENSE_MINIMUM, p);
		if (conf->lc_type_mask & LM_TYPE_PLATFORMS)
		{
			char **cpp = NULL;

			for (cpp = conf->lc_platforms; *cpp; cpp++)
				p = add(job, *cpp, PLATFORMS, p);
		}
		if ((conf->lc_type_mask & LM_TYPE_BORROW))
			p = addi(conf->lc_max_borrow_hours, LM_LICENSE_BORROW, 
							p);
	}
	if (conf->lc_got_options & LM_LICENSE_OPTIONS_PRESENT)
	{
		p = addi(conf->lc_options_mask, PKGOPT, p);
		if ((conf->lc_options_mask & LM_OPT_SUPERSEDE) && 
						conf->lc_supersede_list)
		{
			char **cpp = NULL;

			for (cpp = conf->lc_supersede_list; *cpp; cpp++)
				p = add(job, *cpp, SUPERSEDE, p);
		}
	}
	p = add(job, conf->lc_issuer, LM_LICENSE_ISSUER, p);
	if (conf->lc_issued)
		p = movein_date(conf->lc_issued, p);
	p = add(job, conf->lc_notice, LM_LICENSE_NOTICE, p);
	p = add(job, conf->lc_prereq, LM_LICENSE_PREREQ, p);
	p = add(job, conf->lc_sublic, LM_LICENSE_SUBLIC, p);
	p = add(job, conf->lc_serial, LM_LICENSE_SERIAL_STRING, p);
	p = add(job, conf->lc_dist_constraint, LM_LICENSE_DIST_CONSTRAINT, p);
	if (*conf->startdate)
	{
		p = add(job, LM_LICENSE_START_DATE, 0, p);
		p = movein_date(conf->startdate, p);
	}
#endif	/* !FLEXLM_ULTRALITE */

	if (conf->strength_override)
	{
		sav_strength_override = (long)job->L_STRENGTH_OVERRIDE;
		job->L_STRENGTH_OVERRIDE = (char *)conf->strength_override;
	}

	ret = (char *)l_string_key(job, input_str, p - input_str, code, seclen
#ifdef LM_CKOUT
		,exp_code
#endif
			);
	if (conf->strength_override)
		job->L_STRENGTH_OVERRIDE = (char *)sav_strength_override;

	if (!ret)
		return 0;


	if (use_sdate && ret && (strlen((char *)ret) == 16) && sdate)
	{
		ret = (char *) shuffle(job, (unsigned char *)ret, 
			(unsigned char *)sdate);
	}
	if (ret == (char *) NULL)
	{
		LM_SET_ERRNO(job, LM_CANTMALLOC, 30, 0);
	}
	return(ret);
}





#ifndef FLEXLM_ULTRALITE	/* No SERVERS in ULTRALITE */

/*-
 *	Swap two LM_SERVER pointers in order to sort the server list
 */
static
int
swap(
	LM_HANDLE *	job,
	LM_SERVER **slist,
	int pos)
{
	LM_SERVER *a = slist[pos], *b = slist[pos+1];
	int swapped = 0;
	char astr[MAX_HOSTID_LEN + 1] = {'\0'};
	char bstr[MAX_HOSTID_LEN + 1] = {'\0'};

/*-
 *	Sort order:	(Same order as type field in id struct)
 *
 *		all HOSTID_LONG, in ascending hostid
 *		all HOSTID_ETHER in ascending ethernet addr
 *		all HOSTID_STRING, in ascending sort order
 *		all HOSTID_ANY, unchanged
 *		all HOSTID_USER, in ascending sort order
 *		all HOSTID_DISPLAY, in ascending sort order
 *		all HOSTID_HOSTNAME, in ascending sort order
 *		all HOSTID_VENDOR, in ascending sort order
 *		all HOSTID_DOMAIN, in ascending sort order
 */

	if (a && b && a->idptr && b->idptr)
	{
		if (b->idptr->type < a->idptr->type)
			swapped = 1;
		else if (b->idptr->type == a->idptr->type)
		{
		    switch(a->idptr->type)
		    {
			case HOSTID_CPU:
			case HOSTID_DISK_GEOMETRY:
			case HOSTID_BIOS:
			case HOSTID_DISK_SERIAL_NUM: 
			case HOSTID_FLEXID1_KEY:		    
			case HOSTID_FLEXID5_KEY:		    
			case HOSTID_FLEXID6_KEY:		    
#ifdef SUPPORT_METER_BORROW
			case HOSTID_METER_BORROW:		    
#endif /* SUPPORT_METER_BORROW */
			case HOSTID_LONG:
				if (signed32(b->idptr->hostid_value) < 
					signed32(a->idptr->hostid_value))
					swapped = 1;
				break;

			case HOSTID_ETHER:
			    {
/*-
 *			    Compare ethernet addresses
 */
					int i;
					for (i=0; i<ETHER_LEN; i++)
					{
						if (a->idptr->id.e[i] < b->idptr->id.e[i])
						{
	/*-
	 *						Done. They are in the right
	 *						order already.
	 */
							break;
						}
						else if (a->idptr->id.e[i] > b->idptr->id.e[i])
						{
							swapped = 1;
							break;
						}
					}
			    }
			    break;
			case HOSTID_INTEL32:
			case HOSTID_INTEL64:
			case HOSTID_INTEL96:
			    {

					int i;
					int len = (a->idptr->type == HOSTID_INTEL32) ? (32/32) :
									(a->idptr->type == HOSTID_INTEL64) ? 
										(64/32) : (96/32);
					for (i=0; i<len; i++)
					{
						if (a->idptr->id.intel96[i] < b->idptr->id.intel96[i])
						{
	/*-
	 *						Done. They are in the right
	 *						order already.
	 */
							break;
						}
						else if (a->idptr->id.intel96[i] > b->idptr->id.intel96[i])
						{
							swapped = 1;
							break;
						}
					}
			    }
			    break;

			case HOSTID_STRING:
			case HOSTID_FLEXID2_KEY:		    
			case HOSTID_FLEXID3_KEY:		    
			case HOSTID_FLEXID4_KEY:		    
			case HOSTID_FLEXID_FILE_KEY:		    
			case HOSTID_USER:
			case HOSTID_DISPLAY:
			case HOSTID_HOSTNAME:
			case HOSTID_VENDOR:
			case HOSTID_DOMAIN:
			case HOSTID_COMPOSITE:
				if (!(job->options->flags & LM_OPTFLAG_P3130))
				{
					strcpy(astr, a->idptr->id.string);	/* OVERRUN */
					strcpy(bstr, b->idptr->id.string);	/* OVERRUN */
					l_lowercase(astr);
					l_lowercase(bstr);
					if (strcmp(astr, bstr) > 0)
						swapped = 1;
				}
				else if (strcmp(a->idptr->hostid_string,
						b->idptr->hostid_string) > 0)
				{
					swapped = 1;
				}
				break;

			case HOSTID_SERNUM_ID:		    
				if (l_sernum_to_32(b->idptr->id.string)
					< l_sernum_to_32( a->idptr->id.string))
				{
					swapped = 1;
				}
				break;

			case HOSTID_INTERNET:
				if (l_inet_cmp(a->idptr->id.internet,
					    b->idptr->id.internet) > 0)
				{
					swapped = 1;
				}
				break;


			default:
/*-
 *		ANY: nothing to do, or 
 *			Don't know what type of hostid this is?  what to do???
 */
			break;
		    }
		}
		if (swapped)
		{
			slist[pos] = b;
			slist[pos+1] = a;
		}
	}
	return(swapped);
}
#endif	/* ! FLEXLM_ULTRALITE */

/*
 *	move_in_hostid() - Move a hostid list into a string, return new 
 *	pointer.
 */
static
unsigned
char *
move_in_hostid(
	LM_HANDLE *		job,
	HOSTID *		id,
	unsigned char *	p)
{
	HOSTID *	hid = NULL;

	for (hid = id; hid; hid = hid->next)
		p = move_in_one_hostid(job, hid, p);
	return p;
}

/*
 *	move_in_one_hostid() - Move a hostid into a string, return new pointer.
 */

static
unsigned
char *
move_in_one_hostid(
	LM_HANDLE *		job,
	HOSTID *		id,
	unsigned char *	p)
{
	int i = 0;

	if (id)
	{
		if (id->type == HOSTID_LONG)  /*- Single-machine license */
		{
			p = l_movelong(id->hostid_value, p);
		}
		else if (id->type == HOSTID_ETHER)
		{
			for (i = 0; i < ETHER_LEN; i++)
				*p++ = id->hostid_eth[i];
		}
		else if (id->type == HOSTID_INTEL32 ||
			id->type == HOSTID_INTEL64 ||
			id->type == HOSTID_INTEL96)
		{
			int len = (id->type == HOSTID_INTEL32) ? (32/32) :
			   (id->type == HOSTID_INTEL64) ? (64/32) :
			   (96/32);
			for (i = 0; i < len; i++)
				p = l_movelong(id->id.intel96[i], p);
		}
		else if (id->type == HOSTID_ANY)
		{
			p = l_movelong(ANY_CODE, p);
		}
#ifndef FLEXLM_ULTRALITE
		else if (id->type == HOSTID_USER)
		{
			p = l_movelong(USER_CODE, p);
			p = add_hostidstr(job, id->hostid_user, 0, p);
		}
		else if (id->type == HOSTID_DISPLAY)
		{
			p = l_movelong(DISPLAY_CODE, p);
			p = add_hostidstr(job, id->hostid_display, 0, p);
		}
		else if (id->type == HOSTID_HOSTNAME)
		{
			p = l_movelong(HOSTNAME_CODE, p);
			p = add_hostidstr(job, id->hostid_hostname, 0, p);
		}
		else if (id->type == HOSTID_DOMAIN)
		{
			p = l_movelong(HOSTID_DOMAIN_CODE, p);
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		else if (id->type == HOSTID_COMPOSITE)
		{
			p = l_movelong(HOSTID_COMPOSITE_CODE, p);
			p = add_hostidstr(job, id->hostid_composite, 0, p);
		}
#ifdef SUPPORT_METER_BORROW
		else if (id->type == HOSTID_METER_BORROW)
		{
			p = l_movelong(HOSTID_METER_BORROW_CODE, p);		
			p = l_movelong(id->hostid_value, p);		
		}
#endif /* SUPPORT_METER_BORROW */
		else if ((id->type == HOSTID_FLEXID1_KEY)
			 || (id->type == HOSTID_FLEXID5_KEY)
			 || (id->type == HOSTID_FLEXID6_KEY))
		{
			p = l_movelong(FLEXID1_CODE, p);		
			p = l_movelong(id->hostid_value, p);		
		}
		else if (id->type == HOSTID_FLEXID2_KEY)
		{
			p = l_movelong(FLEXID2_CODE, p);		
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		else if (id->type == HOSTID_FLEXID3_KEY)
		{
			p = l_movelong(FLEXID3_CODE, p);		
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		else if (id->type == HOSTID_FLEXID4_KEY)
		{
			p = l_movelong(FLEXID4_CODE, p);		
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		else if (id->type == HOSTID_FLEXID_FILE_KEY)
		{
			p = l_movelong(FLEXID_FILE_CODE, p);		
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		else if (id->type == HOSTID_DISK_SERIAL_NUM)
		{
			p = l_movelong(DISK_SERIAL_NUM_CODE, p);		
			p = l_movelong(id->hostid_value, p);		
		}
		else if (id->type == HOSTID_INTERNET)
		{
			int x;
			p = l_movelong(INTERNET_CODE, p);		
			for (x=0;x<4;x++)
				p = l_movelong((long)id->id.internet[x], p);
		}
		else if (id->type == HOSTID_SERNUM_ID)
		{
			p = l_movelong( SERNUM_ID_STRING_CODE, p);		
			p = l_movelong(l_sernum_to_32(id->id.string), p);
		}
		else if (id->type == HOSTID_DEMO)
		{
			p = l_movelong(DEMO_CODE, p);
		}
		else if (id->type == HOSTID_FLEXLOCK)
		{
			p = l_movelong(FLEXLOCK_CODE, p);
		}
#endif	/* !FLEXLM_ULTRALITE */
		else if (id->type == HOSTID_STRING)
		{
			p = l_movelong(ID_STRING_CODE, p);
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		else if (id->type >= HOSTID_VENDOR)
		{
			p = l_movelong(HOSTID_VENDOR_CODE ^ (id->type), p);
			p = add_hostidstr(job, id->hostid_string, 0, p);
		}
		
		if (id->override == NO_EXTENDED)
		{
			p = l_movelong(NO_EXTENDED_CODE, p);
		}
	}
	return(p);
}

static
unsigned char *
add_hostidstr(
	LM_HANDLE *		job,
	char *			str,
	char *			desc,
	unsigned char *	out)
{
	char *	q = NULL;

	if (!(job->options->flags & LM_OPTFLAG_P3130))
		return add(job, str, desc, out);

/*
 *	old, behavior with P3130 bug 
 */
	q = str;
	while (*q)
		*out++ = *q++;
	return out;
}


	
/*
 *	add() adds a string.  If it's case insensitive,
 *	it's normalized to one case.  pre 6.0h, it was always 
 *	uppercase.  Turned out we didn't put some hostid types
 *	in here.  So, to minimize keys being generated differently,
 *	I made the hostid strings lowercase, since that appears
 *	to be more common.
 */
static
unsigned char *
add(
	LM_HANDLE *		job,
	char *			str,
	char *			desc,
	unsigned char *	out)
{
	unsigned char * o1 = NULL, * o2 = NULL;

	if (str && *str)
	{
#ifndef FLEXLM_ULTRALITE	/* This never happens in the ultralite code */
		if (desc) 
			sprintf((char *) out, "%s=%s", desc, str);
		else 
#endif	/* !FLEXLM_ULTRALITE */
			strcpy((char *)out, str);
	}

	o1 = o2 = out;
	while (*o1)
	{
		if (!isspace(*o1))    	/*- Only encrypt non-white space */
			*o2++ = *o1;
		o1++;
	}
	memset(o2, 0, (o1-o2) + 1);
	if (!(job->options->flags & LM_OPTFLAG_STRINGS_CASE_SENSITIVE))
		l_uppercase((char *)out);
	return(out + strlen((char *) out));
}

#ifndef FLEXLM_ULTRALITE

static
unsigned char *
addi(
	int				d,
	char *			desc,
	unsigned char *	out)
{

	(void) sprintf((char *) out, "%s=%d", desc, d);
	return(out + strlen((char *) out));
}
#endif	/* !FLEXLM_ULTRALITE */

/*
 * 	get_ver  -- convert version string to two longs
 *		int part and fraction of 10000
 */

static
void
get_ver(
	char *	str,
	long *	ver,
	long *	frac)
{
#ifdef FLEXLM_ULTRALITE
/*
 *	FLEXLM_ULTRALITE uses no version - it's fixed at 1.0
 */
	if(ver)
		*ver = 1;	
	if(frac)
		*frac = 0;
#else

	char version[MAX_VER_LEN+1] = {'\0'};
	char *_frac = NULL;
	int i = 0, len = 0;
	char *cp = NULL;
	char pzero[2];   /* For Alpha/VMS, where you can't overwrite a constant
						 * like this: p = "0"; *p = 0;
						 */


	strcpy(pzero, "0");
	*ver = *frac = 0;

/*
 *	check if it's a null string --- but really we assume the version
 *	has already been checked with l_valid_version()
 */
	if (!str)
		return;
/*
 *	Good args, proceed
 *	first, setup _frac string
 */
	strncpy(version, str, MAX_VER_LEN);
	version[MAX_VER_LEN] = '\0';
	_frac = pzero;

	i = 0;
	sscanf(version, "%d", &i);	/* overrun threat */
	*ver = i;
	cp = (strchr(version, '.'));
	if (cp && *cp)
	{
		*cp = '\0';
		_frac = cp+1;
	}
/*
 *	Remove trailing zeros from _frac
 */
	for (i=strlen(_frac) - 1; i>=0 ;i--)
	{
		if (_frac[i] == '0')
			/* null terminate */
			_frac[i] = '\0'; 
		else
			break;
	}
	i = 0;
	sscanf(_frac, "%d", &i);	/* overrun threat */
	*frac = i;
	len = strlen(_frac);
/*
 * 	Adjust to 10000ths
 *
 *	.1 = 1000/10000, len = 1, result is 10 * 10^(3-len), or 1000
 *	.01 = 100/10000, len = 2, result = 10 * 10^(3-len), or 100
 *	and .001 = 10/1000, len = 3, result = 10 * 10^(3-len) or 10
 *	.1, 
 */
	*frac *= 10;
	for (i=0;i<(3-len);i++)
		*frac *= 10;
#endif	/* !FLEXLM_ULTRALITE */
}


static
unsigned char *
movein_date(
	char *			date,
	unsigned char *	p)
{
	long day = 0, year = 0;
	char month[10] = {'\0'};

	*month = 0;
#ifdef FLEXLM_ULTRALITE
/*
 *	FLEXLM_ULTRALITE uses a fixed expiration date of 1-jan-0
 */
	day = 1;
	strcpy(month, "jan");
	year = 0;
#else
	memset(month, 0, sizeof(month));
	(void) sscanf(date, "%ld-%[^-]-%ld", &day, month, &year);	/* overrun threat */
	if (year > 1900)
		year -= 1900;		/*- Account for 1991 vs 91 */

	if (isupper(*(month)))
		*(month) = tolower(*(month));
	if (isupper(*(month+1)))
		*(month+1) = tolower(*(month+1));
	if (isupper(*(month+2)))
		*(month+2) = tolower(*(month+2));
#endif

	p = l_movelong(day, p);
	p = l_movelong(year, p);
	*p++ = *month;
	*p++ = *(month+1);
	*p++ = *(month+2);
	return p;
}

#ifndef FLEXLM_ULTRALITE
static
long 
l_sernum_to_32(char * s)
{
	char arr[11] = {'\0'}; /*- maximum decimal hostid */
	char *cps = NULL, *cpa = NULL;
	long l = 0;

	for (cpa = arr, cps = s; *cps; cps++)
		if (*cps != '-') /* remove hyphens */
			*cpa++ = *cps;
	*cpa = 0; /* null terminate */
#ifdef VAX_V5 /* %lu broken on VAX C, and strtoul is broken above 2**30 */
	if (strlen(arr) > 9)
	{
		l = strtoul(&arr[1], (char *)NULL, 10);
		if (arr[0] == '1')
			l += 1000000000;
		else if(arr[0] == '2')
			l += 2000000000;
	}
	else
		l = strtoul(arr, (char *)NULL, 10);
#else
	sscanf(arr, "%lu", &l);	/* overrun threat */
#endif /* VAX_V5 */
	return signed32(l);
}
#endif	/* !FLEXLM_ULTRALITE */


static 
int 
valid_code (char * code)
{
	char *cp = NULL;
	int len = 0;

	if (!code)
		return 0;
	for (cp = code; cp && *cp && ((cp - code) <= MAX_CRYPT_LEN); cp++)
		if (!isxdigit(*cp))
			return 0;
	if (*cp != 0 && cp[1] != 0)
		return 0;
	len = strlen(code);
	if ((len == 12) || (len == 16) || (len == 20))
		return 1;
	return 0;
}
  
#include "l_strkey.c"
