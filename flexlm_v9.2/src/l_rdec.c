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
 *	Module: $Id: l_rdec.c,v 1.24 2003/04/18 23:48:04 sluu Exp $
 *
 *	Function:	l_parse_dec
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
 *	Last changed:  11/20/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_pack.h"
#include "string.h"

static int leapyear		lm_args((int));
static char *months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul",
			"aug", "sep", "oct", "nov", "dec" , ""};
static void l_days_ascdate 	lm_args(( LM_HANDLE *, long, char *));
static char * unpack_string 	lm_args(( LM_HANDLE *, unsigned char *, int *));
static unsigned long unpack_num lm_args(( unsigned char *, int *));
static int parse_dec_hostids	lm_args(( LM_HANDLE *, unsigned char *, int *,
						HOSTID **));
static void get_options		lm_args(( unsigned char *, int *, CONFIG *));
static void get_version		lm_args((unsigned char *, int *, char *));
static char * unpack_key	(LM_HANDLE *, unsigned char *, int *);

#ifdef PC16
unsigned long l_unpack(unsigned char * buf,int *ppos, int len);
#endif


static
int
leapyear(year)
int year;
{
	return ((!(year % 4)) && ((year % 100) || (!(year % 400))));
}

/*
 *	l_parse_decimal
 *	return: 1=ok, 0 error
 */

int
API_ENTRY
l_parse_decimal(job, line, conf, err)
LM_HANDLE *job;		/* Current license job */
unsigned char *line;
CONFIG *conf;
char **err;		/* error messages -- return value -- 0 if unneeded */
{
  int pos = 0;
  long l;
  int key_len, expflag, counted_flag, hostid_flag;
  char keytmp[MAX_CRYPT_LEN + 1];
  unsigned char buf[(MAX_CONFIG_LINE * 3)+ 1];
  int id;
  char *cp;

	if (!l_decimal_format(line)) return 0;

	memset(conf, 0, sizeof(CONFIG));
/*
 *	first do vendor name and feature name
 */
	for (cp = conf->daemon; *line && *line != L_DECIMAL_DELIM; cp++)
	{
		*cp = *line++;
	}
	*cp = 0; /* null terminate */
	line++; /* skip delim */

	for (cp = conf->feature; *line && *line != L_DECIMAL_DELIM; cp++)
	{
		*cp = *line++;
	}
	*cp = 0; /* null terminate */
	line++; /* skip delim */

/*
 *	Defaults
 */
	strcpy(conf->version, "1.0");
	strcpy(conf->date, "1-JAN-0");

	if (l_pack_unprint(job, line, buf) < 0)
		return 0;
/*
 *	Section 1: fixed values
 */

/*
 *	Format Version
 */
	(void) l_unpack(buf, &pos, LM_CDLEN_FMTVER); /* ignored for now */

/*
 *	Config type
 */
	conf->type = (short) l_unpack(buf, &pos, LM_CDLEN_CONFTYPE);

	if (conf->type == CONFIG_UPGRADE) strcpy(conf->fromversion, "1.0");
/*
 *	Length of license-key (32,40,48,64-bit)
 */
	l = l_unpack(buf, &pos, LM_CDLEN_KEYLEN);
	switch((int)l)
	{
/*
 *		set key_len to bytes
 */
		case LM_CDKEYLEN_64:	key_len = 8; break;
		case LM_CDKEYLEN_48:	key_len = 6; break;
		case LM_CDKEYLEN_40:	key_len = 5; break;
		case LM_CDKEYLEN_0:	key_len = 0; break;
	}

/*
 *	Expiration date flag (t/f)
 */

	expflag = (short) l_unpack(buf, &pos, LM_CDLEN_EXPFLAG);

/*
 *	Counted flag
 */
	counted_flag = (short) l_unpack(buf, &pos, LM_CDLEN_CNTDFLAG);

/*
 *	Hostid flag
 */
	hostid_flag = (short) l_unpack(buf, &pos, LM_CDLEN_HOSTIDFLAGS);
	switch (hostid_flag)
	{
	case LM_CD_DEMOHOSTID:
		l_get_id(job, &conf->idptr, "DEMO");
		break;
	}
/*
 *	Section 2: License key
 */
	if (key_len)
	{
		conf->L_CONF_FLAGS |= L_CONF_FL_OLD_KEY;
		l = l_unpack(buf, &pos, 32);
		sprintf(conf->code, "%08lX", l);
		key_len -= 4;
		if (key_len)
		{
		  char fmt[10];

			sprintf(fmt, "%%0%dX", key_len * 2);
			l = l_unpack(buf, &pos, key_len * 8);
			sprintf(keytmp, fmt, l);
			strcat(conf->code, keytmp);
		}
	}
/*
 *	Section 3: Variable.  Appearance is set by flags in Section 1
 */

/*
 *	expiration date
 */
	if (expflag)
	{
	  	l = unpack_num(buf, &pos);
		l_days_ascdate(job, l, conf->date);
	}
/*
 *	Count info
 */
	if (counted_flag)
	{
		conf->users = (int) unpack_num(buf, &pos);
	}
/*
 *	Section 4: Variable with type indicator
 */
	while (id = (int) l_unpack(buf, &pos, LM_CDLEN_ID))
	{
		switch (id)
		{
		case LM_CDID_GOT_OPTIONS:
			get_options(buf, &pos, conf);
			break;

		case LM_CDID_START_DATE:
	  	{
		  int xdate;
		  char bindate[DATE_LEN + 1];
		  char cbuf[MAX_CRYPT_LEN  + 1];

			xdate = (int) unpack_num(buf, &pos);
			sprintf(bindate, "%04X", xdate);
			sprintf(cbuf, "%c%c%c%c%c%c%c%c%s",
				conf->code[0], bindate[0],
				conf->code[1], bindate[1],
				conf->code[2], bindate[2],
				conf->code[3], bindate[3],
				&conf->code[4]);
			strcpy(conf->code, cbuf);
		}
			break;
		case LM_CDID_VERSION:
			get_version(buf, &pos, conf->version);
			break;

		case LM_CDID_FROMVERSION:
			get_version(buf, &pos, conf->fromversion);
			break;

		case LM_CDID_VENDOR_STRING:
			conf->lc_vendor_def = unpack_string(job, buf, &pos);
			break;
		case LM_CDID_HOSTID:
			parse_dec_hostids(job, buf, &pos, &conf->idptr);
			break;

		case LM_CDID_VENDOR_INFO:
			conf->lc_vendor_info = unpack_string(job, buf, &pos);
			break;

		case LM_CDID_DIST_INFO:
			conf->lc_dist_info = unpack_string(job, buf, &pos);
			break;

		case LM_CDID_KEY2:
		{
			conf->lc_keylist = (LM_KEYLIST *)
					l_malloc(job, sizeof(LM_KEYLIST));
			conf->lc_keylist->sign_level = 1;
			conf->lc_sign = conf->lc_keylist->key =
					(char *)unpack_key(job, buf, &pos);
			/* kmaclean 3/5/03
			 * P6140
			 * Fixed a bug where SIGN= did not work in decimal format
			 * Just needed to set conf->code
			 *  */
			if (!*conf->code && !(job->flags & LM_FLAG_LIC_GEN))
				l_zcp(conf->code, conf->lc_sign, MAX_CRYPT_LEN);
		}
			break;

		case LM_CDID_USER_INFO:
			conf->lc_user_info = unpack_string(job, buf, &pos);
			break;

		case LM_CDID_ASSET_INFO:
			conf->lc_asset_info = unpack_string(job, buf, &pos);
			break;

#ifdef SUPPORT_METER_BORROW
		case LM_CDID_METER_BORROW_INFO:
			conf->lc_meter_borrow_info =
						unpack_string(job, buf, &pos);
			break;
#endif /* SUPPORT_METER_BORROW */

		case LM_CDID_ISSUER:
			conf->lc_issuer = unpack_string(job, buf, &pos);
			break;

		case  LM_CDID_NOTICE:
			conf->lc_notice = unpack_string(job, buf, &pos);
			break;

		case LM_CDID_PLATFORMS:
			{
			  int num, i;

				num = (int) l_unpack(buf, &pos,
						LM_CDLEN_NUM_PLATFORMS);
				conf->lc_platforms =
					(char **)l_malloc(job,
						(num + 1)* (sizeof (char **)));
				for (i = 0; i < num; i++)
				{
					conf->lc_platforms[i] =
						unpack_string(job, buf, &pos);
				}
			}
			break;

		case LM_CDID_SERIAL:
			conf->lc_serial = unpack_string(job, buf, &pos); break;

		case LM_CDID_ISSUED:
			conf->lc_issued = unpack_string(job, buf, &pos); break;

		case LM_CDID_USER_BASED:
			conf->lc_user_based=
					 (short) unpack_num(buf, &pos);
			 break;

		case LM_CDID_HOST_BASED:
			conf->lc_host_based = (short) unpack_num(buf, &pos);
			 break;

		case LM_CDID_MINIMUM:
			conf->lc_minimum = (short) unpack_num(buf, &pos); break;

		case LM_CDID_BORROW:
			conf->lc_max_borrow_hours = (short) unpack_num(buf, &pos);
			conf->lc_got_options |= LM_LICENSE_TYPE_PRESENT;
			conf->lc_type_mask |= LM_TYPE_BORROW;
			break;

		case LM_CDID_SUPERSEDE_LIST:
			{
			  int num, i, len = 0;
			  char *mem;

				num = (int) l_unpack(buf, &pos,
							LM_CDLEN_NUM_SUPERSEDE);
				conf->lc_supersede_list =
					(char **)l_malloc(job,
						(num + 1)* (sizeof (char **)));
/*
 *				NOTE:  Supersede is elsewhere
 *					malloc'd once for all strings.
 *					We have to go to some trouble
 *					to duplicate this here, else
 *					we get a memleak.
 */
				for (i = 0; i < num; i++)
				{
					conf->lc_supersede_list[i] =
						unpack_string(job, buf, &pos);
					len += strlen(
						conf->lc_supersede_list[i]) + 1;
				}
				mem = (char *)l_malloc(job, len);
				for (i = 0; i < num; i++)
				{
					strcpy(mem, conf->lc_supersede_list[i]);
					free(conf->lc_supersede_list[i]);
					conf->lc_supersede_list[i] = mem;
					mem += strlen(mem) + 1;
				}
			}
			break;
		case LM_CDID_SERVER_HOSTID:
			{
			  LM_SERVER *s, *sp, *sp2;
			  int siz;
			  int cnt;

				for (cnt = 1, sp = conf->server; sp;
								sp = sp->next)
					cnt++;

				siz = cnt * sizeof (LM_SERVER);

				s = (LM_SERVER *) l_malloc(job, siz);
				s->sflags |= L_SFLAG_PARSED_DEC;
				s->sflags |= L_SFLAG_THIS_HOST; /* default */
				s->port = -1; /* default ports */
				parse_dec_hostids(job, buf, &pos, &s->idptr);
				sp2 = s;
				for (sp = conf->server; sp; sp = sp->next)
				{
					sp2->next = sp2 + 1;
					*sp2->next = *sp;
					sp2++;
				}
				if (conf->server)free(conf->server);
				conf->server = s;
			}
			break;

		case LM_CDID_SERVER_PORT:
			if (!conf->server)
			{
				LM_SET_ERROR(job, LM_BADFILE, 390, 0, line, LM_ERRMASK_ALL);
				return LM_BADFILE;
			}
			conf->server->port = (int)
				l_unpack(buf, &pos, LM_CDLEN_SERVER_PORT);
			break;

		case LM_CDID_STARTDATE_ATTR:
			l = unpack_num(buf, &pos);
			l_days_ascdate(job, l, conf->startdate);
			break;

		default:
			LM_SET_ERROR(job, LM_BADFILE, 317, 0, line, LM_ERRMASK_ALL);
			return LM_BADFILE;
		}
	}
	if (conf->server)
	{
	  LM_SERVER_LIST_PTR slp;
		slp = (LM_SERVER_LIST_PTR)l_malloc(job, sizeof(LM_SERVER_LIST));
		slp->s = conf->server;
		slp->next = job->conf_servers;
		job->conf_servers = slp;
	}
	return 1;

}
/*
 *	Returns days since 12/31/96 -- 16-bit -- good through
 *	1/1/97 + 65535 days =~ 2175, which is way beyond valid unix
 *	date...
 */

static int month_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static int month_days_leap[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static
void l_days_ascdate(job, days, date)
LM_HANDLE *job; /* unused for now */
long days;
char *date;
{
	int imonth;
	int year;

	year = 1997;

	for(;;)
	{
		int days_this_year;
		int *p_month_days;
/*
 *		Determine which values to use for this year.
 */
		if( leapyear( year ) )
		{
			days_this_year = 366;
			p_month_days = month_days_leap;
		}
		else
		{
			days_this_year = 365;
			p_month_days = month_days;
		}
/*
 *		See if it's more than a year's worth of days.
 */
		if( days > days_this_year )
		{
			days -= days_this_year;
			++year;
			continue;
		}
/*
 *		Walk the months taking each size away.
 */
		for( imonth = 0; imonth < 12; imonth++ )
		{
			if( days > p_month_days[imonth] )
				days -= p_month_days[imonth];
			else
				break;
		}

		break;
	}

	sprintf(date, "%d-%s-%d",(int) days, months[imonth], year);
}



int API_ENTRY
l_decimal_format(line)
unsigned char *line;
{
  int i;

/*
 *	skip vendor and feature
 */
	while (*line && *line != L_DECIMAL_DELIM &&
			!isspace(*line) && *line != '#')
		line++;
	if (!*line || *line != L_DECIMAL_DELIM) return 0;
	line++; /* skip delim */
	while (*line && *line != L_DECIMAL_DELIM)
		line++;
	if (!*line) return 0;
	line++; /* skip delim */

/*
 *	decimal portion
 */
	for (i = 0; line[i] && line[i] != '\n'; i++)
	{
/*
 *		nnnnn-nnnnn-...
 *		the dashes happen when i%6 == 5
 */
		if ((i % 6) == 5)
		{
			if (line[i] != L_DECIMAL_DELIM) return 0;
		}
		else if (!isdigit(line[i])) return 0;
	}
	if (i<10) return 0;
	return 1;
}


static
char *
unpack_string(job, buf, ppos)
LM_HANDLE *job;
unsigned char *buf;
int *ppos;
{
  int uppercase;
  long l;
  char *ret;
  int store, bits;
  int ishex;
  char tmp[MAX_CONFIG_LINE + 1];
  char *cp = tmp;

	store = (int) l_unpack(buf, ppos, LM_CDLEN_STRING_STORE);
	if (store == LM_STRING_STORE_STRING)
	{
		for (cp = tmp;	*cp = (char) l_unpack(buf, ppos, 8); cp++)
			;
		ret = (char *)l_malloc(job, (cp - tmp) + 1);
		strcpy(ret, tmp);
	}
	else
	{
		ret = (char *)l_malloc(job, MAX_LONG_LEN + 1);
		if (ishex = (int) l_unpack(buf, ppos, LM_CDLEN_STORE_HEX))
			uppercase = (int)
				 l_unpack(buf, ppos, LM_CDLEN_STORE_UPPER);
		switch(store)
		{
		case LM_STRING_STORE_BYTE: bits = 8; break;
		case LM_STRING_STORE_16BIT: bits = 16; break;
		case LM_STRING_STORE_32BIT: bits = 32; break;
		}
		l = l_unpack(buf, ppos, bits);
		if (ishex)
		{
			if (uppercase) sprintf(ret, "%lX", l);
			else sprintf(ret, "%lx", l);
		}
		else sprintf(ret, "%lu", l);
	}
	return ret;
}



static
unsigned long
unpack_num(buf, ppos)
unsigned char *buf;
int *ppos;
{
  int bits;

	bits = (int) l_unpack(buf, ppos, LM_CDLEN_NUM_BITS);
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
	return l_unpack(buf, ppos, bits);
}

static
int
parse_dec_hostids(job, buf, ppos, p_idptr)
LM_HANDLE *job;
unsigned char *buf;
int *ppos;
HOSTID **p_idptr;
{
  int i;
  HOSTID *idptr;

	idptr = (HOSTID *)l_malloc(job, sizeof(HOSTID));
	idptr->type = (short) l_unpack(buf, ppos, LM_CDLEN_HOSTID_TYPE);

	/* see l_pack.h for note about HOSTID_VENDOR */
	if (idptr->type >= LM_CD_HOSTID_VENDOR)
	{
		idptr->type +=
			(HOSTID_VENDOR - LM_CD_HOSTID_VENDOR);
	}
	switch (idptr->type)
	{
	case HOSTID_ANY:	break; /* nothing */
	case HOSTID_FLEXLOCK:	break; /* nothing */
	case HOSTID_ID_MODULE:
	case HOSTID_FLEXID1_KEY:
	case HOSTID_FLEXID5_KEY:
	case HOSTID_FLEXID6_KEY:
	case HOSTID_DISK_SERIAL_NUM:
	case HOSTID_LONG:
	case HOSTID_CPU:
	case HOSTID_DISK_GEOMETRY:
	case HOSTID_BIOS:
		idptr->id.data = unpack_num(buf, ppos);
		break;
	case HOSTID_ETHER:
		for (i = 0 ;i < ETHER_LEN; i++)
		{
			idptr->id.e[i] = (char) l_unpack(buf, ppos, 8);
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
				idptr->id.intel96[i] =
					unpack_num(buf, ppos);
			}
		}
		break;
	case HOSTID_INTERNET:
		for (i = 0 ;i < 4; i++)
		{
		  int wildcard;

			wildcard = (int) l_unpack(buf, ppos, 1);
			if (!wildcard)
				idptr->id.internet[i] =(short)
					l_unpack(buf, ppos, 8);
			else
				idptr->id.internet[i] = (short)-1;
		}
		break;
	default:
		{
		  char *cp;
			if (idptr->type >= HOSTID_VENDOR)
			{
				idptr->vendor_id_prefix =
				unpack_string(job, buf, ppos);
			}
			cp = unpack_string(job, buf, ppos);
			l_zcp(idptr->id.string, cp, MAX_HOSTID_LEN);	/* LONGNAMES */
			free(cp);
		}
		break;
	}
/*
*			Note: we can't put the next one first in the
*			list, since that would reverse the order.
*			We *must* append to the end of the list
*/
	{
	  HOSTID *p, *last = 0, *id;

		id = *p_idptr;

		for (p = id; p; p = p->next)
		{
			last = p;
		}
		if (last) last->next = idptr;
		else id = idptr;
		*p_idptr = id;
	}
	return 0;
}
static
void
get_options(buf, ppos, conf)
unsigned char *buf;
int *ppos;
CONFIG *conf;
{
	conf->lc_got_options = (unsigned short) unpack_num(buf, ppos);
	if (conf->lc_got_options & LM_LICENSE_LINGER_PRESENT)
	{
		conf->lc_linger = (int) unpack_num(buf, ppos);
	}
	if (conf->lc_got_options & LM_LICENSE_DUP_PRESENT)
	{
		conf->lc_dup_group = (int) unpack_num(buf, ppos);
	}
	if (conf->lc_got_options & LM_LICENSE_OVERDRAFT_PRESENT)
	{
		conf->lc_overdraft = (int) unpack_num(buf, ppos);
	}
	if (conf->lc_got_options & LM_LICENSE_CKSUM_PRESENT)
	{
		conf->lc_cksum = (int) l_unpack(buf, ppos, LM_CDLEN_CKSUM);
	}
	if (conf->lc_got_options & LM_LICENSE_OPTIONS_PRESENT)
	{
		conf->lc_options_mask =(unsigned char)
			l_unpack(buf, ppos, LM_CDLEN_OPTIONS);
	}
	if (conf->lc_got_options & LM_LICENSE_TYPE_PRESENT)
	{
		conf->lc_type_mask = (unsigned char)
				l_unpack(buf, ppos, LM_CDLEN_TYPE);
#ifdef SUPPORT_METER_BORROW
		if (conf->lc_type_mask & LM_TYPE_METER_BORROW)
		{
			conf->meter_borrow_counter =
					(int)unpack_num(buf, ppos);
		}
#endif /* SUPPORT_METER_BORROW */
	}
	if (conf->lc_got_options & LM_LICENSE_SUITE_DUP_PRESENT)
	{
		conf->lc_suite_dup = (int) unpack_num(buf, ppos);
	}
	if (conf->lc_got_options & LM_LICENSE_SORT_PRESENT)
	{
		conf->lc_sort = (int)unpack_num(buf, ppos);
	}
}
static
void
get_version(buf, ppos, verbuf)
unsigned char *buf;
int *ppos;
char *verbuf;
{
    int offset;
    char integer[MAX_VER_LEN + 1];
    char frac[MAX_VER_LEN + 1];
    long l;

	strcpy(frac, "0"); /* default */
	offset = (int) l_unpack(buf, ppos, LM_CDLEN_VER_DECPT);
	l = unpack_num(buf, ppos);
	sprintf(integer, "%lu", l);
	if (offset)
	{
	   int i = strlen(integer) - offset;
		strcpy(frac, &integer[i]);
		integer[i] = 0;
	}
	sprintf(verbuf, "%s.%s", integer, frac);
}
/*
 *	It's presumed that line is decimal format
 *	fill in dname buffer with name from decimal line.
 */
void
l_get_dec_daemon(line, dname)
char *line;
char *dname;
{
	while (*line && *line != L_DECIMAL_DELIM)
	{
		*dname++ = *line++;
	}
	*dname = 0; /* null terminate */
}
static
char *
unpack_key( 	LM_HANDLE *job,
		unsigned char *buf, /* buffer */
		int *ppos )/* offset */
{
  int len, offset;
  int i;
  char *ret;
	/* get len in bits */
	len = (unsigned int) l_unpack(buf, ppos, LM_CDLEN_KEY2);
/*
 *	convert bits to len in "xxxx xxxx xxxx ..." format
 *	Each char is 4 bits (len/=4) and every 4 chars we need another
 *	space (len/4)
 */
	len /= 4;
	ret = l_malloc(job, len + (len/4) + 2);
	for (offset = 0, i = 0; i < len; i++)
	{
		if ((len > 12) && !(i %4) && i) 	ret[offset++] = ' ';

		ret[offset++] = l_itox(l_unpack(buf, ppos, 4));
	}
	return ret;
}
