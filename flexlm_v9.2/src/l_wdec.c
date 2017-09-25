/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_wdec.c,v 1.23 2003/04/16 17:26:37 brolland Exp $
 *
 *	Function:	l_print_conf_dec
 *
 *	Description: 	prints a CONFIG * struct into a buffer, in 
 *			a variety of formats.
 *
 *	Parameters:	(LM_HANDLE *)job
 *			(CONFIG *) conf
 *			(char *buf) Must be >= MAX_CONFIG_LINE + 1 in len
 *			(int) format
 *
 *	Return:		NONE (void)
 *
 *	D. Birns
 *	7/30/97
 *
 *	Last changed:  12/8/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_pack.h"
#include "string.h"

static long l_expdate_days	lm_args((LM_HANDLE *, int, int, int));
static int leapyear		lm_args((int));
static void pack_string_id		lm_args((int, unsigned char *, int *, char *));
static void pack_string		lm_args((unsigned char *, int *, char *));
static void pack_num_id		lm_args(( unsigned char *, int *, int , long));
static void pack_num		lm_args(( unsigned char *, int *, long));
static int print_hostids	lm_args(( LM_HANDLE *, HOSTID *, 
					unsigned char *, int *, int));
static int print_version	lm_args(( LM_HANDLE *, unsigned char *, int *, 
						char *, int));
static void pack_key		(int , unsigned char *, int *, char *);

/*
 *	INT32ARG is really only for 16-bit PC 
 */

#define INT32ARG(x)	(x)

int
l_print_conf_dec(job, conf, buf)
LM_HANDLE *job;
CONFIG *conf;
char *buf;
{
  unsigned char pbuf[(MAX_CONFIG_LINE * 3) + 1];
	/* worst case: each character takes 3 decimals */
  int pos = 0;
  unsigned long l;
  char lbuf[9];
  int day = 0;
  char month[10];
  int imonth;
  int year = 0;
  char *startdate;
  char *cp;
  int codelen;
  int keylen;
  int use_start_date = 0;
  char *outcp = buf;
  int i;
  long days;

/*
 *	daemon and feature name
 */
	if (strchr(conf->feature, L_DECIMAL_DELIM))
	{
		LM_SET_ERROR(job, LM_CANT_DECIMAL, 384, 0, conf->feature, 
							LM_ERRMASK_ALL);
		return LM_CANT_DECIMAL;
	}
	if (strchr(conf->daemon, L_DECIMAL_DELIM))
	{
		LM_SET_ERROR(job, LM_CANT_DECIMAL, 385, 0, conf->daemon, 
							LM_ERRMASK_ALL);
		return LM_CANT_DECIMAL;
	}

	sprintf(outcp, "%s%c%s%c", conf->daemon, L_DECIMAL_DELIM, 
					conf->feature, L_DECIMAL_DELIM);
	outcp += strlen(outcp);
	memset(pbuf, 0, sizeof(pbuf));
/* 
 *	Section 1: fixed values
 */
/*
 *	Format Version
 */
	l_pack(pbuf, &pos, LM_CDLEN_FMTVER, conf->decimal_fmt); 

/*
 *	Config type
 */
	l_pack(pbuf, &pos, LM_CDLEN_CONFTYPE, (long)conf->type); 

/*
 *	Length of license-key (32,40,48,64-bit)
 */
	codelen = strlen(conf->code) / 2;
	if (strlen(conf->code) == MAX_CRYPT_LEN)
	{
		use_start_date = 1;
		keylen = (codelen - 2); /* 2 bytes start date */
	}
	else keylen = codelen;
	
	l_pack(pbuf, &pos, LM_CDLEN_KEYLEN, 
					(keylen == 0) ? LM_CDKEYLEN_0 :
					(keylen == 5) ? LM_CDKEYLEN_40 :
					(keylen == 6) ? LM_CDKEYLEN_48 :
							 LM_CDKEYLEN_64); 

/*
 *	Expiration date flag (t/f)
 */

	sscanf(conf->date, "%d-%[^-]-%d", &day, month, &year);	/* overrun threat */
	l_uppercase(month);
	if (year == 0 && !L_STREQ(month, "JAN"))
	{
		LM_SET_ERROR(job, LM_CANT_DECIMAL, 382, 0, conf->date, 
						LM_ERRMASK_ALL);
		return LM_CANT_DECIMAL;
	}
	l_pack(pbuf, &pos, LM_CDLEN_EXPFLAG, (long)(year>0)); 

/*
 *	Counted flag
 */
	l_pack(pbuf, &pos, LM_CDLEN_CNTDFLAG, (long)conf->users > 0); 

/*
 *	Hostid flag
 */
	if (!conf->idptr)
		l_pack(pbuf, &pos, LM_CDLEN_HOSTIDFLAGS,(long)LM_CD_NOHOSTID); 
	else if (conf->idptr->type == HOSTID_DEMO)
		l_pack(pbuf, &pos, LM_CDLEN_HOSTIDFLAGS,(long)LM_CD_DEMOHOSTID);
	else
		l_pack(pbuf, &pos, LM_CDLEN_HOSTIDFLAGS,(long)LM_CD_REALHOSTID); 
/*
 *	Section 2: License key 
 */
/* 
 *	move 1st 8 bytes
 */
	if (keylen)
	{
		if (use_start_date)
		{
	/*
	 *		Ignore start-date
	 */
			lbuf[0] = conf->code[0];
			lbuf[1] = conf->code[2];
			lbuf[2] = conf->code[4];
			lbuf[3] = conf->code[6];
			lbuf[4] = conf->code[8];
			lbuf[5] = conf->code[9];
			lbuf[6] = conf->code[10];
			lbuf[7] = conf->code[11];
			lbuf[8] = 0;
			sscanf(lbuf, "%lx", &l);	/* overrun checked */
			l_pack(pbuf, &pos, 32, l); 
			cp = &conf->code[12];
		}
		else
		{
			l = 0;
			l_zcp(lbuf, conf->code, 8);
			sscanf(lbuf, "%lx", &l);	/* overrun checked */
			l_pack(pbuf, &pos, 32, l); 
			cp = &conf->code[8];
		}
		keylen -= 4;
		if (keylen)
		{
			strcpy(lbuf, cp);
			l = 0;
			sscanf(lbuf, "%lx", &l);	/* overrun checked */
			l_pack(pbuf, &pos, keylen * 8, l); 
		}
	}
/*

 *	Section 3: Variable.  Appearance is set by flags in Section 1
 */

/*
 *	expiration date 
 */
	if (year)
	{

		imonth = l_int_month(month);
		if (!(days = l_expdate_days(job, year, imonth, day)))
		{
			LM_SET_ERROR(job, LM_CANT_DECIMAL, 386, 0, conf->date,
							LM_ERRMASK_ALL);
			return LM_CANT_DECIMAL; /* ERROR */
		}


		pack_num(pbuf, &pos, INT32ARG(days));
	}
/*
 *	Count info
 */	
	if (conf->users)
	{
		pack_num(pbuf, &pos, INT32ARG(conf->users)); 
	}
/*
 *	Section 4: Variable with type indicator
 */

/*
 *	lc_got_options values
 */
	if (conf->lc_got_options)
	{
		pack_num_id(pbuf, &pos, LM_CDID_GOT_OPTIONS,
						INT32ARG(conf->lc_got_options));
		if (conf->lc_got_options & LM_LICENSE_LINGER_PRESENT)
		{
			pack_num(pbuf, &pos, INT32ARG(conf->lc_linger));
		}
		if (conf->lc_got_options & LM_LICENSE_DUP_PRESENT)
		{
			pack_num(pbuf, &pos, INT32ARG(conf->lc_dup_group) );
		}
		if (conf->lc_got_options & LM_LICENSE_OVERDRAFT_PRESENT)
		{
			pack_num(pbuf, &pos, INT32ARG(conf->lc_overdraft));
		}
		if (conf->lc_got_options & LM_LICENSE_CKSUM_PRESENT)
		{
			l_pack(pbuf, &pos, LM_CDLEN_CKSUM , conf->lc_cksum);
		}
		if (conf->lc_got_options & LM_LICENSE_OPTIONS_PRESENT)
		{
			l_pack(pbuf, &pos, LM_CDLEN_OPTIONS, 
						conf->lc_options_mask);
		}
		if (conf->lc_got_options & LM_LICENSE_TYPE_PRESENT)
		{
			l_pack(pbuf, &pos, LM_CDLEN_TYPE , conf->lc_type_mask);
#ifdef SUPPORT_METER_BORROW
			if (conf->lc_type_mask & LM_TYPE_METER_BORROW)
			{
				pack_num(pbuf, &pos, 
					INT32ARG(conf->meter_borrow_counter));
			}
#endif /* SUPPORT_METER_BORROW */
		}
		if (conf->lc_got_options & LM_LICENSE_SUITE_DUP_PRESENT)
		{
			pack_num(pbuf, &pos, INT32ARG(conf->lc_suite_dup));
		}
		if (conf->lc_got_options & LM_LICENSE_SORT_PRESENT)
		{
			pack_num(pbuf, &pos, INT32ARG(conf->lc_sort));
		}
	}

/*
 *	Start date
 */
	if (strlen(conf->code) == MAX_CRYPT_LEN)
	{
		startdate = l_extract_date(job, conf->code);
		if (startdate && !L_STREQ(startdate, LM_DEFAULT_START_DATE))
		{
		  int idate;

			sscanf(startdate, "%x", &idate);	/* overrun threat */
			pack_num_id(pbuf, &pos, LM_CDID_START_DATE, INT32ARG(idate));
		}
	}
/*
 *	Version
 */
	if (i = print_version(job, pbuf, &pos, conf->version, LM_CDID_VERSION))
		return i;

	if (conf->type == CONFIG_UPGRADE)
	{
		if (i = print_version(job, pbuf, &pos, conf->fromversion, 
							LM_CDID_FROMVERSION))
			return i;
	}
/*
 *	lc_vendor_def
 */
	pack_string_id(LM_CDID_VENDOR_STRING, pbuf, &pos, conf->lc_vendor_def);

/*
 *	hostid list
 */
	print_hostids(job, conf->idptr, pbuf, &pos, LM_CDID_HOSTID);
	pack_string_id(LM_CDID_VENDOR_INFO, pbuf, &pos, conf->lc_vendor_info);
	pack_string_id(LM_CDID_DIST_INFO, pbuf, &pos, conf->lc_dist_info);
	pack_string_id(LM_CDID_USER_INFO, pbuf, &pos, conf->lc_user_info);
	pack_key(LM_CDID_KEY2, pbuf, &pos, conf->lc_sign);
	pack_string_id(LM_CDID_ASSET_INFO, pbuf, &pos, conf->lc_asset_info);
#ifdef SUPPORT_METER_BORROW
	pack_string_id(LM_CDID_METER_BORROW_INFO, pbuf, &pos, 
						conf->lc_meter_borrow_info);
#endif /* SUPPORT_METER_BORROW */
	pack_string_id(LM_CDID_ISSUER, pbuf, &pos, conf->lc_issuer);
	pack_string_id(LM_CDID_NOTICE, pbuf, &pos, conf->lc_notice);
	if (conf->lc_type_mask & LM_TYPE_PLATFORMS)
	{
	  char **cpp;
	  int num;

		for (num = 0, cpp = conf->lc_platforms; *cpp; num++, cpp++)
			;
		l_pack(pbuf, &pos, LM_CDLEN_ID, LM_CDID_PLATFORMS);
		l_pack(pbuf, &pos, LM_CDLEN_NUM_PLATFORMS, num);
		for (cpp = conf->lc_platforms; *cpp; cpp++)
		{
			pack_string(pbuf, &pos, *cpp);
		}
	}
	pack_string_id(LM_CDID_SERIAL, pbuf, &pos, conf->lc_serial);
	pack_string_id(LM_CDID_ISSUED, pbuf, &pos, conf->lc_issued);
	pack_num_id(pbuf, &pos, LM_CDID_USER_BASED, INT32ARG(conf->lc_user_based));
	pack_num_id(pbuf, &pos, LM_CDID_HOST_BASED, INT32ARG(conf->lc_host_based));
	if (conf->startdate && *conf->startdate && 
		sscanf(conf->startdate, "%d-%[^-]-%d", &day, month, &year))	/* overrun threat */
	{
		l_uppercase(month);
		imonth = l_int_month(month);
		if (!(days = l_expdate_days(job, year, imonth, day)))
		{
				LM_SET_ERROR(job, LM_CANT_DECIMAL, 522, 0, 
					conf->date, LM_ERRMASK_ALL);
				return LM_CANT_DECIMAL; /* ERROR */
		}
		pack_num_id(pbuf, &pos, LM_CDID_STARTDATE_ATTR, INT32ARG(days)); 
	}
			
	pack_num_id(pbuf, &pos, LM_CDID_MINIMUM, INT32ARG(conf->lc_minimum));
	if (conf->lc_type_mask & LM_TYPE_BORROW) 
	{
		pack_num_id(pbuf, &pos, LM_CDID_BORROW, 
			INT32ARG(conf->lc_max_borrow_hours));
	}
	if ((conf->lc_options_mask & LM_OPT_SUPERSEDE) &&
		conf->lc_supersede_list)
	{
	  char **cpp;
	  int num;

		for (num = 0, cpp = conf->lc_supersede_list; *cpp; num++, cpp++)
			;
		l_pack(pbuf, &pos, LM_CDLEN_ID, LM_CDID_SUPERSEDE_LIST);
		l_pack(pbuf, &pos, LM_CDLEN_NUM_SUPERSEDE, num);
		for (cpp = conf->lc_supersede_list; *cpp; cpp++)
		{
			pack_string( pbuf, &pos, (*cpp));
		}
	}
/*
 *	Server hostid(s)
 */
	if (conf->server && !(conf->server->sflags & L_SFLAG_PRINTED_DEC))
	{
	  LM_SERVER *s;
		for (s = conf->server; s; s = s->next)
		{
			print_hostids(job, s->idptr, pbuf, &pos,
				LM_CDID_SERVER_HOSTID);
			if (s->port > 0)
			{
				l_pack(pbuf, &pos, LM_CDLEN_ID, 
						LM_CDID_SERVER_PORT);
				l_pack(pbuf, &pos, LM_CDLEN_SERVER_PORT, 
						conf->server->port);
			}
		}
		conf->server->sflags |= L_SFLAG_PRINTED_DEC;
	}
		

/*
 *	Print it to buffer
 */
	l_pack_print(pbuf, pos, outcp);
	return 0; /* success */
}
/*
 *	Returns days since 12/31/96 
 */
static
long
l_expdate_days(job, year, imonth, day)
LM_HANDLE *job; /* unused for now */
int year; 
int imonth; 
int day; 
{
  long ret = 0;
  int i;

	if (year < 1900) year += 1900;
	if (year < 1997) return 0;

#ifndef PC16
        ret = (year - 1997) * 365;
#else
        ret = (((long)year) - ((long) 1997)) * 365;
#endif
	for (i = 1997; i < year; i++)
		if (leapyear(i)) ret++;

        if (imonth >= 1) ret += 31;     /* jan */
        if (imonth >= 2)                /* feb */
        {
                ret += 28;
		if (leapyear(year)) ret++;
        }
        if (imonth >= 3) ret += 31;     /* mar */
        if (imonth >= 4) ret += 30;     /* apr */
        if (imonth >= 5) ret += 31;     /* may */
        if (imonth >= 6) ret += 30;     /* jun */
        if (imonth >= 7) ret += 31;     /* jul */
        if (imonth >= 8) ret += 31;     /* aug */
        if (imonth >= 9) ret += 30;     /* sep */
        if (imonth >= 10) ret += 31;    /* oct */
        if (imonth == 11) ret += 30;    /* nov */

        ret += day;
        return ret;
}

static
int
leapyear(year)
int year;
{
	return ((!(year % 4)) && ((year % 100) || (!(year % 400))));
}


static
void
pack_string_id(id, pbuf, ppos, str)
int id;
unsigned char *pbuf;
int *ppos;
char *str;
{
	if (str)
	{
		l_pack(pbuf, ppos, LM_CDLEN_ID, id);
		pack_string(pbuf, ppos, str);
	}
}
static
void
pack_key( 	int id, /* id of key */
		unsigned char *pbuf, /* buffer */
		int *ppos, /* offset */
		char *str) /* actual key */
{
  int len;
  char *cp;
	if (!str) return;
	l_pack(pbuf, ppos, LM_CDLEN_ID, id);
	for (len = 0, cp = str; *cp ; cp++)
	{
		if (isxdigit(*cp)) len++;
	}
	len *= 4; /* 4 bits per hex character */
	l_pack(pbuf, ppos, LM_CDLEN_KEY2, len); /* write num bits in key */
	for (cp = str; *cp ; cp++) /* write 4 bits at a time */
	{
		if (!isxdigit(*cp)) continue;
		l_pack(pbuf, ppos, 4, l_xtoi(*cp)); /* 4 bits */
	}
}
char
l_itox(int i)
{
	if (i < 10) return i + '0';
	else return (i - 10) + 'A';
}
int
l_xtoi(unsigned char c)
{
	if (isdigit(c)) return c - '0';
	else return tolower(c) -'a' + 10;
}


static
void
pack_string(pbuf, ppos, str)
unsigned char *pbuf;
int *ppos;
char *str;
{
  char *cp;
  int ishex = 0;
  int isdec = 0;
  int uppercase;
  int len = strlen(str);
  long val = 0;
  int store = LM_STRING_STORE_STRING;
  int bits;

	if (len <= 10)
	{
		for (isdec = 1, cp = str; *cp; cp++)
		{
			if(!isdigit(*cp))
			{
				isdec = 0;
				break;
			}
		}
	}

	if (!isdec && len <= 8)
	{
		for (ishex = 1, cp = str; *cp; cp++)
		{
			if(!isxdigit(*cp))
			{
				ishex = 0;
				break;
			}
		}
	}
	if (ishex && *str != '0') sscanf(str, "%lx", &val);	/* overrun checked */
	if (isdec && *str != '0') sscanf(str, "%ld", &val);	/* overrun checked */
	if (val)
	{
		if (ishex)
		{
			for (uppercase = 1, cp = str; *cp; cp++)
			{
				if(isalpha(*cp) && !isupper(*cp))
				{
					uppercase = 0;
					break;
				}
			}
		}
/*
 *		mixed case is no good 
 */
		for (cp = str; *cp; cp++)
		{
			if(isalpha(*cp) && 
				((uppercase && !isupper(*cp)) ||
				(!uppercase && isupper(*cp))))
			{
				val = 0;
				break;
			}
		}
	}
	if (val)
	{
		val &= 0xffffffff; /* for 64-bit systems, in case */
#ifndef PC16
		if (val & ~0xffff)
#else
                if (val & ~((long) 0xffff))
#endif
		{
			store = LM_STRING_STORE_32BIT;
			bits = 32;
		}
		else
#ifndef PC16
			if (val & ~0xff)
#else
			if (val & ~((long) 0xff))
#endif
		{
			store = LM_STRING_STORE_16BIT;
			bits = 16;
		}
		else
		{
			store = LM_STRING_STORE_BYTE;
			bits = 8;
		}
		l_pack(pbuf, ppos, LM_CDLEN_STRING_STORE, store);
		l_pack(pbuf, ppos, LM_CDLEN_STORE_HEX, ishex);
		if (ishex) l_pack(pbuf, ppos, LM_CDLEN_STORE_UPPER, uppercase);
		l_pack(pbuf, ppos, bits, val);
	}
	else
	{
		l_pack(pbuf, ppos, LM_CDLEN_STRING_STORE, store);
		for (cp = str; *cp; cp++)
		{
			l_pack(pbuf, ppos, 8, (long) *cp);
		}
		l_pack(pbuf, ppos, 8, (long)0); /* null terminate */
	}
}

static
void
pack_num_id(pbuf, ppos, id, val)
unsigned char *pbuf;
int *ppos;
int id; 
long val;
{
	if (val)
	{
		l_pack(pbuf, ppos, LM_CDLEN_ID, id);
		pack_num(pbuf, ppos, val);
	}
}

static
void
pack_num(pbuf, ppos, val)
unsigned char *pbuf;
int *ppos;
long val;
{
  int bits;

	val = val & 0xffffffff; /* for 64-bit systems, in case */

	if ((val & 0x1) == val) bits = LM_CD_NUM_1BIT;
	else if ((val & 0xf) == val) bits = LM_CD_NUM_4BIT;
	else if ((val & 0xff) == val) bits = LM_CD_NUM_8BIT;
	else if ((val & 0xfff) == val) bits = LM_CD_NUM_12BIT;
	else if ((val & 0xffff) == val) bits = LM_CD_NUM_16BIT;
	else if ((val & 0xffffff) == val) bits = LM_CD_NUM_24BIT;
	else if ((val & 0x7fffffff) == val) bits = LM_CD_NUM_31BIT;
	else bits = LM_CD_NUM_32BIT;
	l_pack(pbuf, ppos, LM_CDLEN_NUM_BITS, bits);
	switch(bits)
	{
	case LM_CD_NUM_1BIT: bits = 1; break;
	case LM_CD_NUM_4BIT: bits = 4; break;
	case LM_CD_NUM_8BIT: bits = 8; break;
	case LM_CD_NUM_12BIT: bits = 12; break;
	case LM_CD_NUM_16BIT: bits = 16; break;
	case LM_CD_NUM_24BIT: bits = 24; break;
	case LM_CD_NUM_31BIT: bits = 31; break;
	case LM_CD_NUM_32BIT: bits = 32; break;
	}
	l_pack(pbuf, ppos, bits, val);
}


static
int
print_hostids(job, id, pbuf, ppos, packid)
LM_HANDLE *job;
HOSTID *id;
unsigned char *pbuf;
int *ppos;
int packid;
{
  HOSTID *idptr;
  int i;

	for (idptr = id; idptr && idptr->type != HOSTID_DEMO; 
			idptr = idptr->next)
	{
	  char *asc;
		if ((idptr->type  - HOSTID_VENDOR) > LM_CD_HOSTID_VENDOR)
		{
			asc = l_asc_hostid(job, idptr);
			LM_SET_ERROR(job, LM_CANT_DECIMAL, 383, 0, asc,
							LM_ERRMASK_ALL);
			return LM_CANT_DECIMAL;
		}
		l_pack(pbuf, ppos, LM_CDLEN_ID, packid);
		/* see l_pack.h for note about HOSTID_VENDOR */
		l_pack(pbuf, ppos, LM_CDLEN_HOSTID_TYPE, 
			idptr->type < HOSTID_VENDOR ? idptr->type : 
			LM_CD_HOSTID_VENDOR + (idptr->type - HOSTID_VENDOR));
		switch (idptr->type)
		{
		case HOSTID_ANY:	
		case HOSTID_FLEXLOCK:	
			break; /* nothing */
		case HOSTID_ID_MODULE:	
		case HOSTID_FLEXID1_KEY:
		case HOSTID_FLEXID5_KEY:
		case HOSTID_FLEXID6_KEY:
		case HOSTID_DISK_SERIAL_NUM:
		case HOSTID_LONG:
		case HOSTID_CPU:
		case HOSTID_DISK_GEOMETRY:
		case HOSTID_BIOS:
			pack_num(pbuf, ppos, INT32ARG(idptr->id.data));
			break;
		case HOSTID_ETHER:	
			for (i = 0 ;i < ETHER_LEN; i++)
			{
				l_pack(pbuf, ppos, 8, (idptr->id.e[i] & 0xff));
			}
			break;
		case HOSTID_INTEL32:	
		case HOSTID_INTEL64:	
		case HOSTID_INTEL96:	
			{
			  int len = (idptr->type == HOSTID_INTEL32) ? 32/32 :
			  (idptr->type == HOSTID_INTEL64) ? 64/32 : 96/32;
				for (i = 0 ;i < len; i++)
				{
					pack_num(pbuf, ppos, 
					INT32ARG(idptr->id.intel96[i]));
				}
			}
			break;
		case HOSTID_INTERNET:	
			for (i = 0 ;i < 4; i++)
			{
			  int wildcard = ((int)idptr->id.internet[i] == -1);

				l_pack(pbuf, ppos, 1, wildcard);
				if (!wildcard)
					l_pack(pbuf, ppos, 8, 
						idptr->id.internet[i]);
			}
			break;
		default:	
			{
			  char *cp = idptr->id.string;
				if (idptr->type >= HOSTID_VENDOR)
				{
					pack_string(pbuf, ppos, 
						idptr->vendor_id_prefix);
					if (*cp == '=') cp++;
				}	
				pack_string(pbuf, ppos, cp);
			}
			break;
		}
	}
	return 0;
}

static 
int
print_version(job, pbuf, ppos,version, id)
LM_HANDLE *job;
unsigned char *pbuf;
int *ppos;
char *version;
int id;
{
  int i;

	if (l_compare_version(job, version, "1.0"))
	{
	  char *v = version, *frac = 0, *cp;
	  char ver[MAX_VER_LEN+1];
	  unsigned long u;
	  
		while (*v == '0') v++; /* skip leading 0's */
		strcpy(ver, v);
		if ((cp = strchr(ver, '.')) && *cp)
		{
			*cp = '\0'; /* split into 2 parts */
			frac = cp+1;
/*		
 *			remove trailing zeros from fractional part
 */
			for (i = (strlen(frac)) - 1; i >= 0 ;i--)
			{
				if (frac[i] == '0')
					/* null terminate */
					frac[i] = '\0'; 
				else break;
			}
		}
		else if ((strlen(ver) == MAX_VER_LEN) && 	
			(strcmp("4294967295", ver) > 0))
		{
			/* number is larger than max int! */
			LM_SET_ERROR(job, LM_CANT_DECIMAL, 316, 0, ver,
							LM_ERRMASK_ALL);
			return LM_CANT_DECIMAL; /* ERROR */
		}

		l_pack(pbuf, ppos, LM_CDLEN_ID, id);
		l_pack(pbuf, ppos, LM_CDLEN_VER_DECPT, frac ? strlen(frac) : 0);
		if (frac) strcat(ver, frac);
		u = 0;
		sscanf(ver, "%lu", &u);	/* overrun checked */
		pack_num(pbuf, ppos, INT32ARG(u));
		
	}
	return 0;
}
