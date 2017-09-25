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
 *	Module: $Id: l_get_id.c,v 1.20 2003/04/18 23:48:03 sluu Exp $
 *
 *	Function: l_get_id(job, id, string)
 *
 *	Description:	Turns a HOSTID string (or list) into a
 *			a HOSTID struct.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) string - The host id
 *			(HOSTID *) id - pointer to a HOSTID structure.
 *					in a generic way, so ck=nnn will work.
 *
 *	Return:		(HOSTID *) id - filled in.
 *			0 -- success
 *			<0 -- lm_errno
 *
 *	Last changed:  10/26/98
 *	M. Christiano
 *	7/20/88
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lgetattr.h"
#include "l_prot.h"
#include <string.h>

static int isdecimal lm_args((char *));

#if defined(PC) 
typedef int ( LM_CALLBACK_TYPE * LM_CALLBACK_PARSE_VID)
		 (HOSTID *id, char *hostid);
#define LM_CALLBACK_PARSE_VID_TYPE (LM_CALLBACK_PARSE_VID)
#else
#define LM_CALLBACK_PARSE_VID_TYPE
#endif

static HOSTID * API_ENTRY l_get_one_id lm_args(( LM_HANDLE *, char *));
static int API_ENTRY check_vendor_ids lm_args(( LM_HANDLE_PTR , HOSTID *, 
							char *));
static void append_vd_prefix	lm_args(( LM_HANDLE *, HOSTID *, char *));


int API_ENTRY
l_get_id(job, id, string)
LM_HANDLE *job;		/* Current license job */
HOSTID **id; 		/* pointer to (HOSTID *) */
char *string;		
{
   int len;
   char *nextstr;
   HOSTID *hid, *prev = (HOSTID *)0;
   int sav_errno = job->lm_errno;

/* 
 *	assume failure 
 */
   if(!string)
   {
	   return 0;	/* success -- NOHOSTID */
   }
   *id = NULL;
/*
 *	Strip off delimiting quotes if it's a list
 */
	if (*string ==  '"')
		string++;
	len = strlen(string);
	if (len && string[len-1] == '"')
		string[len-1] = '\0';

	while (string)
	{
	  char *p1, *p2;
		p1 = strchr(string, ' ');
		p2 = strchr(string, '\t');
		if (p1 || p2) 
		{
			nextstr = (!p2) ? p1 : 
					(!p1) ? p2 : 
					(p1 < p2) ? p1 : p2;
		}
		else nextstr = 0;
		if (nextstr)
		{
			*nextstr++ = '\0';
			while (isspace(*nextstr ))
				nextstr++;

		}
		hid = l_get_one_id(job, string);
		if (!hid) break;  
		if (!*id) *id = hid; /* set to the first in the list */
		if (string && prev) prev->next = hid;
		string = nextstr;
		prev = hid;
	}
	return (sav_errno == job->lm_errno) ? 0 : job->lm_errno;
}

/*
 *	l_get_one_id -- (used to be l_get_id)
 */
static
HOSTID * API_ENTRY
l_get_one_id(job, string)
LM_HANDLE *job;		/* Current license job */
char *string;
{
   int len;
   HOSTID tmp;
   HOSTID *id = &tmp;
   int i;
   char *cp;
   int ishex = 1;
   long cksum_only = job->flags & LM_FLAG_CKSUM_ONLY;
   char typebuf[MAX_HOSTID_LEN + 1];
   char flexidbuf[MAX_HOSTID_LEN + 1];


/*
 *	the first character of the hostid is 'x' or 'X', defeat the
 *	extended hostid checking.
 */
	memset(id, 0, sizeof(HOSTID));
	len = strlen(string);

	if (cp = strchr(string, '='))
		l_zcp(typebuf, string, ++cp - string);
	else if (cp = strchr(string, ':'))
		l_zcp(typebuf, string, ++cp - string);
	else *typebuf = 0;
	if (cp = strchr(string, '-'))
		l_zcp(flexidbuf, string, ++cp - string);
	else *flexidbuf = 0;

		
	for (ishex = 1, i = 0; i < len; i++) /* P1487 --check isxdigit */
	{
		if (!isxdigit(string[i]))
		{
			ishex = 0;
			break;
		}
	}
	id->override = 0;
	if (*string == 'X' || *string == 'x')
	{
		id->override = NO_EXTENDED;
		string++;
	}
/*
 *	Demo software specifies the hostid as "DEMO" or "demo"
 */
	else if (l_keyword_eq(job, string, "DEMO") )
	{
		id->type = HOSTID_DEMO;
	}
	else if (l_keyword_eq(job, string, "FLEXLOCK") )
	{
		id->type = HOSTID_FLEXLOCK;
	}
/*
 *	Any hostid.
 */
	else if (l_keyword_eq(job, string, "ANY"))
	{
		id->type = HOSTID_ANY;
	}
	/*
	 *	Added for P6589
	 */
	else if (l_keyword_eq(job, string, "any"))
	{
		id->type = HOSTID_ANY;
	}

#ifndef FLEXLM_ULTRALITE	/* Ultralite doesn't support any of these */

/*
 *	User, display, or hostname
 */
	else if (l_keyword_eq(job, typebuf, HOSTID_USER_STRING))
	{
		id->type = HOSTID_USER;
		strcpy(id->hostid_user, &string[strlen(HOSTID_USER_STRING)]);	/* OVERRUN */
	}
	else if (l_keyword_eq(job, typebuf, HOSTID_DISPLAY_STRING))
	{
		id->type = HOSTID_DISPLAY;
		strcpy(id->hostid_display, 
				&string[strlen(HOSTID_DISPLAY_STRING)]);	/* OVERRUN */
	}
/* 
 *	String type -- currently only for SCO 5/27/94
 */	
	else if (l_keyword_eq(job, typebuf, HOSTID_STRING_STRING))
	{
		id->type = HOSTID_STRING;
		strcpy(id->id.string, (&string[strlen(HOSTID_STRING_STRING)]));	/* OVERRUN */
	}
	else if (l_keyword_eq(job, typebuf, SHORTHOSTID_STRING_STRING))
	{
		id->type = HOSTID_STRING;
		strcpy(id->id.string, 
			(&string[strlen(SHORTHOSTID_STRING_STRING)]));	/* OVERRUN */
	}
	else if (l_keyword_eq(job, typebuf, HOSTID_SERNUM_ID_STRING))
	{
	  int j;
	  int err = 0; /* assume good */
	  char *cp2;
	  int l = strlen(HOSTID_SERNUM_ID_STRING);
/* 
 *		error checking 
 *			1) all decimal
 *			2) < (roughly) MAX Signed int  (2,399,999,999) 
 */
		for (cp2 = &string[l], j = 0; *cp2; cp2++)
		{
			if (isdigit(*cp2)) j++;
			else if (*cp2 != '-')
			{
				err = 1;
				break;
			}
		}
		if (!err && (j > 10)) err = 1;
		if (!err)
		{
			cp2 = &string[l];
			if (j==10 &&
				((*cp2 > '2') ||
				(*cp2 == '2' && cp2[1]  > '3')))
					err = 1;
		}
		if (err)
		{
			LM_SET_ERROR(job, LM_BADPARAM, 297, 0, string, LM_ERRMASK_ALL);
			return (HOSTID *)0; /* NOHOSTID */
		}
		id->type = HOSTID_SERNUM_ID;
		strcpy(id->id.string, (&string[l]));	/* OVERRUN */
	}
	else if (l_keyword_eq(job, typebuf, HOSTID_HOSTNAME_STRING))
	{
		id->type = HOSTID_HOSTNAME;
		strcpy(id->hostid_hostname, 
				&string[strlen(HOSTID_HOSTNAME_STRING)]);	/* OVERRUN */
	}
	else if (l_keyword_eq(job, typebuf, HOSTID_DOMAIN_STRING))
	{
		id->type = HOSTID_DOMAIN;
		strcpy(id->hostid_hostname, 
				&string[strlen(HOSTID_DOMAIN_STRING)]);	/* OVERRUN */
	}
#ifdef SUPPORT_METER_BORROW
	else if (l_keyword_eq(job, typebuf, HOSTID_METER_BORROW_STRING))
	{
	  char *cp = string;
	  char buf1[3];
	  char buf[MAX_CONFIG_LINE];

		id->type = HOSTID_METER_BORROW;
		cp += strlen(HOSTID_METER_BORROW_STRING);
		if (strchr(cp, ':'))
		{
			*buf1 = *buf = 0;
			sscanf(cp, "%[^:]:%[^:]:%ld", buf1, buf, 	/* overrun threat */
			&id->hostid_value);
			id->vendor_id_prefix = l_malloc(job, 
					strlen(buf1) + strlen(buf) + 2);
			sprintf( id->vendor_id_prefix, "%s:%s", buf1, buf );
		}
		else sscanf(cp, "%ld", &id->hostid_value);			/* overrun threat */
	}
#endif /* SUPPORT_METER_BORROW */
	else if (!cksum_only && job->options->parse_vendor_id && 
		 ((*LM_CALLBACK_PARSE_VID_TYPE
		   job->options->parse_vendor_id)(id, string) == 0))
	{
		append_vd_prefix(job, id, string);
	}

	else if (!cksum_only && check_vendor_ids(job, id, string))
	{
		append_vd_prefix(job, id, string);
	}
        else if ((len == (ETHER_LEN * 2)) && ishex)
/*
 *	Ethernet address
 */
	{
	  unsigned val;
  	  int j;

		id->type = HOSTID_ETHER;
		for (j = 0; j < ETHER_LEN; j++)
		{
			 sscanf(&string[j*2], "%2x", &val);	/* overrun checked */
			id->hostid_eth[j] = (unsigned char) (val & 0xff);
		}
	}
        else if ((string[4] == '-') && 
		(len == 9 || len == 19 || len == 29))
	{
  	  int q, j, k = 0;
  	  char *sp = string;
                

		id->type = (len == 9) ? HOSTID_INTEL32 :
			(len == 19) ? HOSTID_INTEL64 :
			HOSTID_INTEL96;
		k = (id->type == HOSTID_INTEL32) ? 32/32 :
			(id->type == HOSTID_INTEL64) ? 64/32 : 96/32 ;
		while (*sp)
		{
                        if (*sp == '-') sp++;
			sscanf(sp, "%4X-%4X", &q, &j);	/* overrun checked */
			id->id.intel96[--k] = (q << 16) + j;
			sp += 9;
			if (*sp == '-') sp++;
			else break;
		}
	}

	/*-
	 *	Macrovision dongles.
	 */
	else if (l_keyword_eq(job, flexidbuf, HOSTID_FLEXID1_KEY_STRING)
		&& l_getattr(job, LM_DONGLE1)==LM_DONGLE1_VAL)
	{
		id->type = HOSTID_FLEXID1_KEY;
		 sscanf(&string[strlen(HOSTID_FLEXID1_KEY_STRING)],	/* overrun checked */
			      "%lx", &id->hostid_value);		
	}
	else if (l_keyword_eq(job, typebuf, SHORTHOSTID_FLEXID1_KEY_STRING)
		&& l_getattr(job, LM_DONGLE1)==LM_DONGLE1_VAL)
	{
		id->type = HOSTID_FLEXID1_KEY;
		 sscanf(&string[strlen(SHORTHOSTID_FLEXID1_KEY_STRING)],	/* overrun checked */
			      "%lx", &id->hostid_value);		
	}
	else if (l_keyword_eq(job, flexidbuf, HOSTID_FLEXID6_KEY_STRING)
		&& l_getattr(job, LM_DONGLE1)==LM_DONGLE1_VAL)
	{
		id->type = HOSTID_FLEXID6_KEY;
		 sscanf(&string[strlen(HOSTID_FLEXID6_KEY_STRING)],	/* overrun checked */
			      "%lx", &id->hostid_value);		
	}
	else if (l_keyword_eq(job, typebuf, SHORTHOSTID_FLEXID6_KEY_STRING)
		&& l_getattr(job, LM_DONGLE1)==LM_DONGLE1_VAL)
	{
		id->type = HOSTID_FLEXID6_KEY;
		 sscanf(&string[strlen(SHORTHOSTID_FLEXID6_KEY_STRING)],	/* overrun checked */
			      "%lx", &id->hostid_value);		
	}
	else if (l_keyword_eq(job, typebuf, "SENTINEL_KEY=")
		&& l_getattr(job, LM_DONGLE1)==LM_DONGLE1_VAL)
	{
		id->type = HOSTID_FLEXID5_KEY;
		 sscanf(&string[strlen("SENTINEL_KEY=")],	/* overrun checked */
			      "%lx", &id->hostid_value);
	}
	else if (l_keyword_eq(job, flexidbuf, HOSTID_FLEXID2_KEY_STRING)
		&& l_getattr(job, LM_DONGLE2)==LM_DONGLE2_VAL)
	{
		id->type = HOSTID_FLEXID2_KEY;
		 sscanf(&string[strlen(HOSTID_FLEXID2_KEY_STRING)],	/* overrun checked */
			      "%s", id->hostid_string);		
	}
	else if (l_keyword_eq(job, flexidbuf, HOSTID_FLEXID3_KEY_STRING)
		&& l_getattr(job, LM_DONGLE3)==LM_DONGLE3_VAL)
	{
		id->type = HOSTID_FLEXID3_KEY;
		 sscanf(&string[strlen(HOSTID_FLEXID3_KEY_STRING)],	/* overrun checked */
			      "%s", id->hostid_string);		
	}
	else if (l_keyword_eq(job, flexidbuf, HOSTID_FLEXID4_KEY_STRING)
		&& l_getattr(job, LM_DONGLE4)==LM_DONGLE4_VAL)
	{
		id->type = HOSTID_FLEXID4_KEY;
		 sscanf(&string[strlen(HOSTID_FLEXID4_KEY_STRING)],	/* overrun checked */
			      "%s", id->hostid_string);		
	}
	else if (l_keyword_eq(job, flexidbuf, HOSTID_FLEXID_FILE_KEY_STRING))
	{
		id->type = HOSTID_FLEXID_FILE_KEY;
		 sscanf(&string[strlen(HOSTID_FLEXID_FILE_KEY_STRING)],	/* overrun checked */
			      "%s", id->hostid_string);		
	}
	else if (l_keyword_eq(job, typebuf, HOSTID_DISK_SERIAL_NUM_STRING))
	{
		id->type = HOSTID_DISK_SERIAL_NUM;
		 sscanf(&string[strlen(HOSTID_DISK_SERIAL_NUM_STRING)],	/* overrun checked */
			      "%lx", &id->hostid_value);		
	}		
	else if (l_keyword_eq(job, typebuf, SHORTHOSTID_DISK_SERIAL_NUM_STR))
	{
		id->type = HOSTID_DISK_SERIAL_NUM;
		sscanf(&string[strlen(SHORTHOSTID_DISK_SERIAL_NUM_STR)],	/* overrun checked  */
			      "%lx", &id->hostid_value);		
	}		
	else if (l_keyword_eq(job, typebuf, HOSTID_INTERNET_STRING))
	{
	  char t; /* ignored */
		id->type = HOSTID_INTERNET;
		l_inet_to_addr(&string[strlen(HOSTID_INTERNET_STRING)], 
			&t, id->hostid_internet);
	}

	else if (l_keyword_eq(job, typebuf, HOSTID_CPU_STRING))
	{
		id->type = HOSTID_CPU;
		 sscanf(&string[strlen(HOSTID_CPU_STRING)], "%lx", &id->hostid_value);		/* overrun checked */	
	}		
	else if (l_keyword_eq(job, typebuf, HOSTID_DISK_GEOMETRY_STRING))
	{
		id->type = HOSTID_DISK_GEOMETRY;
		 sscanf(&string[strlen(HOSTID_DISK_GEOMETRY_STRING)], "%lx", &id->hostid_value);			/* overrun checked */
	}		
	else if (l_keyword_eq(job, typebuf, HOSTID_BIOS_STRING))
	{
		id->type = HOSTID_BIOS;
		 sscanf(&string[strlen(HOSTID_BIOS_STRING)], "%lx", &id->hostid_value);			/* overrun checked */
	}		
	else if (l_keyword_eq(job, typebuf, HOSTID_COMPOSITE_STRING))
	{
		id->type = HOSTID_COMPOSITE;
		strcpy(id->hostid_composite, &string[strlen(HOSTID_COMPOSITE_STRING)]);
	}		

#endif	/* FLEXLM_ULTRALITE */
	
/*
 *      "Regular" 32-bit hostid
 *	We now accept both decimal and hex hostids
 */
	else if (( (len <= 11) && (*string == '#') && isdecimal(&string[1]) ) ||
		(( (len > 8) && (len <= 10) && isdecimal(string) )))
/*
 *	Decimal LONG
 */
	{
	  char *sp = string;
		if (*sp == '#') sp++; /* decimal prefix */
		id->type = HOSTID_LONG;
#ifdef FLEXLM_ULTRALITE
		id->hostid_value = l_atoul(sp);
#else
		 sscanf(sp, "%lu", &id->hostid_value);	/* overrun checked */
#endif
		id->representation = HOSTID_REP_DECIMAL;
	}
	else if (len && (len <= 8) && ishex)
/*
 *	Hex LONG
 */
	{
		id->type = HOSTID_LONG;
#ifdef FLEXLM_ULTRALITE
		id->hostid_value = l_atoulx(string);
#else
		 sscanf(string, "%lx", &id->hostid_value);	/* overrun checked */
#endif
	}
	else if ((cp = strchr(string, '=')) && (cp != string) && 
		!strchr(&cp[1], '='))
/*		
 *	assume it's vendor-defined for the sake of portable cksum
 */
	{
		id->type = HOSTID_VENDOR;
/* 
 *	P3353 -- this used to copy in the '=' sign also.
 *		Fixing this is going to break lmcksum's, unfortunately...
 */
		l_zcp(id->id.vendor, cp + 1, MAX_HOSTID_LEN);	/* LONGNAMES */
		append_vd_prefix(job, id, string);
	}
        else 
	{
		LM_SET_ERROR(job, LM_BADPARAM, 298, 0, string, LM_ERRMASK_ALL);
		return (HOSTID *)0; /* NOHOSTID */
	}
	if (!(id = l_new_hostid())) return 0;

	memcpy(id, &tmp, sizeof(HOSTID));

#ifndef FLEXLM_ULTRALITE	/* Don't need this check for ULTRALITE */
	if (l_getattr(job, ADVANCED_HOSTIDS)!=ADVANCED_HOSTIDS_VAL)
	{
		switch(id->type)
		{
		case HOSTID_ANY:
		case HOSTID_DEMO:
		case HOSTID_FLEXLOCK:
		case HOSTID_FLEXID1_KEY:
		case HOSTID_FLEXID2_KEY:
		case HOSTID_FLEXID3_KEY: 
		case HOSTID_FLEXID4_KEY: 
		case HOSTID_FLEXID6_KEY: break;
		default:	LM_SET_ERROR(job, LM_FUNCNOTAVAIL, 285, 0, string, LM_ERRMASK_ALL);
			return (HOSTID *)0;
		}
	}
#endif	/* !FLEXLM_ULTRALITE */
	return id;
}

static
int
isdecimal(str)
char *str;
{
  char *cp;

	for (cp = str; *cp; cp++)
	{
		if (!isdigit(*cp)) return 0;
	}
	return 1;
	
}
static
int
API_ENTRY
check_vendor_ids(job, id, string)
LM_HANDLE_PTR job;
HOSTID *id;
char *string;
{
  LM_VENDOR_HOSTID *p;
  LM_VENDOR_HOSTID *h = job->options->vendor_hostids;
  char *strp;
  char hostid[MAX_HOSTID_LEN+1];

	if (!strchr(string, '=')) /* invalid vendor-defined hostid */
		return 0;

	l_zcp(hostid, string, MAX_HOSTID_LEN);
	strp = strchr(hostid, '='); 

	*strp = '\0';
	strp++;

	for (p = h; p; p = p->next)
	{
		if (l_keyword_eq(job, hostid, p->label)) /* found it! */
		{
			id->type = p->hostid_num;
			l_zcp(id->id.string, strp, MAX_HOSTID_LEN);	/* LONGNAMES */
			if (!p->case_sensitive) l_uppercase(id->id.string);
			return 1;
		}
	}
	return 0;
}
static
void
append_vd_prefix(job, idptr, str)
LM_HANDLE *job;
HOSTID *idptr;
char *str;
{
  char buf[MAX_HOSTID_LEN + 1];
  char *cp;

	l_zcp(buf, str, MAX_HOSTID_LEN);
	
	if (cp = strchr(buf, '='))  *cp = 0;
	idptr->vendor_id_prefix = (char *)l_malloc(job, strlen(buf) + 1);
	strcpy(idptr->vendor_id_prefix, buf);
}
