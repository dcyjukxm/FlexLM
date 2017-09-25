/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_asc_hostid.c,v 1.15.4.3 2003/06/30 21:43:39 sluu Exp $
 *
 *	Function: l_asc_hostid(job, id)
 *
 *	Description: Returns an ascii string corresponding to the host ID.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(HOSTID *) id - hostid struct ptr
 *
 *	Return:		(char *) host id, suitable for printing.
 *
 *	M. Christiano
 *	9/3/89
 *
 *	Last changed:  10/26/98
 *
 */


#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "flex_utils.h"

#if defined(PC) 
typedef long (LM_CALLBACK_TYPE  * LM_CALLBACK_PRINT_VID) (HOSTID *id);
#define LM_CALLBACK_PRINT_VID_TYPE (LM_CALLBACK_PRINT_VID)
#else
#define LM_CALLBACK_PRINT_VID_TYPE
#endif

char * API_ENTRY
l_asc_hostid(job, id)
LM_HANDLE *job;
HOSTID *id;
{
	return l_asc_hostid_len(job, id, 0);
}

/* 
 *	l_asc_hostid_len -- same as asc_hostid, with arg for shorter
 *	name=value len
 */
LM_CHAR_PTR API_ENTRY
l_asc_hostid_len(job, id, shortstr)
LM_HANDLE *job;
HOSTID *id;
int shortstr; /* 1 if short */
{
  static char buf [MAX_CONFIG_LINE], *cp;
  int cnt;
  HOSTID *hid;
	
	for (cnt = 0, hid = id; hid; hid = hid->next, cnt++)
		;
	cp = buf;

	if (cnt > 1) 	*cp++ = '"';

	for (hid = id; hid; hid = hid->next)
	{
		l_asc_id_one(job, hid, shortstr, cp);
		cp += strlen(cp);
		if (hid->next) 	*cp++ = ' ';
	}
	if (cnt > 1) 	*cp++ = '"';
	*cp = '\0';
	return buf;
}
/*
 *	l_asc_id_one -- gets ascii hostid for one hostid.
 */
LM_CHAR_PTR API_ENTRY
l_asc_id_one(job, id, shortstr, print)
LM_HANDLE *job;
HOSTID *id;
int shortstr; /* 1 if short */
char *print; /* buffer to print to */
{
  int i;
  char *str;

	if (!id) 
	{
		*print = '\0';
	}
	else if (id->type == HOSTID_DEMO)
	{
		(void)strcpy(print, "DEMO");
	}
	else if (id->type == HOSTID_FLEXLOCK)
	{
		(void)strcpy(print, "FLEXLOCK");
	}
	else if (id->type == HOSTID_LONG)
	{
		if (id->override == NO_EXTENDED)
			sprintf(print, "X%lx", id->id.data);
		else
		{
			if (id->representation == HOSTID_REP_DECIMAL)
			{
/* 
 *	"%ul" works on PC, but may not be 100% portable
 */
#ifdef PC
				sprintf(print, "%c%u", '#', 
						id->id.data);
#else
				sprintf(print, "%c%u", '#', 
						id->id.data);
#endif
			}
			else
			{
/* 
 *	The mask is to fix P2576 on Hal, and possibly other
 *	64-bit systems
 */
				sprintf(print, "%lx", 
					(id->id.data & 0xffffffff));
			}
		}
	}
	else if (id->type == HOSTID_ETHER)
	{
	  char *p = print;

		if (id->override == NO_EXTENDED)
		{
			*p = 'X';
			p++;
		}
		for (i = 0; i < 6; i++)
		{
			sprintf(p, "%2x", id->id.e[i] & 0x0ff);
			if (*p == ' ') *p = '0';
			p += 2;
		}
		*p = '\0';
	}
	else if (id->type == HOSTID_INTEL32 ||
		id->type == HOSTID_INTEL64 ||
		id->type == HOSTID_INTEL96)
	{
	   char *p = print;
	   int len = (id->type == HOSTID_INTEL32) ? (32/32) :
		   (id->type == HOSTID_INTEL64) ? (64/32) :
		   (96/32);

		for (i = len - 1; i >= 0 ; i--)
		{
			if (i != (len - 1)) *p++ = '-';
			sprintf(p, "%04X-%04X", id->id.intel96[i] >> 16, 
						id->id.intel96[i] & 0xffff);
			p += 9;
		}
	}
	else if (id->type == HOSTID_INTERNET)
	{
		strcpy(print, HOSTID_INTERNET_STRING);
		l_addr_to_inet(id->id.internet, 4, 
			&print[strlen(HOSTID_INTERNET_STRING)]);;
	}
	else if (id->type == HOSTID_ANY)
	{
		(void)strcpy(print, "ANY");
	}
	else if (id->type == HOSTID_USER)
	{
		sprintf(print, "%s%s", HOSTID_USER_STRING,	/* OVERRUN */
			id->hostid_user);
	}
	else if (id->type == HOSTID_HOSTNAME)
	{
		sprintf(print, "%s%s", HOSTID_HOSTNAME_STRING,	/* OVERRUN */
			id->hostid_hostname);
	}
	else if (id->type == HOSTID_DISPLAY)
	{
		sprintf(print, "%s%s", HOSTID_DISPLAY_STRING,	/* OVERRUN */
			id->hostid_display);
	}
	else if (id->type == HOSTID_STRING)
	{
		if (shortstr) str = SHORTHOSTID_STRING_STRING;
		else str = HOSTID_STRING_STRING;

		sprintf(print, "%s%s", str, id->id.string);	/* OVERRUN */
	}
	else if (id->type == HOSTID_SERNUM_ID)
	{
		sprintf(print, "%s%s", HOSTID_SERNUM_ID_STRING,		/* OVERRUN */
							id->id.string);
	}
	else if (id->type == HOSTID_DOMAIN)
	{
		sprintf(print, "%s%s", HOSTID_DOMAIN_STRING,	/* OVERRUN */
							id->id.string);
	}
	else if (((id->type == HOSTID_FLEXID1_KEY)
		 || (id->type == HOSTID_FLEXID5_KEY)
		 || (id->type == HOSTID_FLEXID6_KEY)) && 
                (id->id.data & 0xffffffff))
	{
                switch (id->type)
                {
                case HOSTID_FLEXID5_KEY:
                        str = "SENTINEL_KEY=";
                        break;
                case HOSTID_FLEXID1_KEY:
                        str = HOSTID_FLEXID1_KEY_STRING;
                        break;
                case HOSTID_FLEXID6_KEY:
                        str = HOSTID_FLEXID6_KEY_STRING;
                        break;
                }

		
		sprintf(print, "%s%lx", str, id->id.data & 0xffffffff);
	}
	else if ((id->type == HOSTID_FLEXID2_KEY) ||
		(id->type == HOSTID_FLEXID3_KEY) ||
		(id->type == HOSTID_FLEXID4_KEY) ||
		(id->type == HOSTID_FLEXID_FILE_KEY))
	{
		switch (id->type)
		{
		case HOSTID_FLEXID2_KEY: str = HOSTID_FLEXID2_KEY_STRING; break;
		case HOSTID_FLEXID3_KEY: str = HOSTID_FLEXID3_KEY_STRING; break;
		case HOSTID_FLEXID4_KEY: str = HOSTID_FLEXID4_KEY_STRING; break;
		case HOSTID_FLEXID_FILE_KEY: 
			str = HOSTID_FLEXID_FILE_KEY_STRING; break;
		}
		
		sprintf(print, "%s%s", str, id->id.string);	/* OVERRUN */
	}
	else if (id->type == HOSTID_DISK_SERIAL_NUM)
	{
		if (shortstr) str = SHORTHOSTID_DISK_SERIAL_NUM_STR;
		else str = HOSTID_DISK_SERIAL_NUM_STRING;
#ifdef PC
		sprintf(print, "%s%lx", str, id->id.data);	/* OVERRUN */
#else
		sprintf(print, "%s%x", str, id->id.data);	/* OVERRUN */
#endif
	}

#ifdef PC
	else if (id->type == HOSTID_CPU)
	{
		str = HOSTID_CPU_STRING;
		sprintf(print, "%s%lx", str, id->id.data);	/* OVERRUN */
	}
	else if (id->type == HOSTID_DISK_GEOMETRY)
	{
		str = HOSTID_DISK_GEOMETRY_STRING;
		sprintf(print, "%s%lx", str, id->id.data);	/* OVERRUN */
	}
	else if (id->type == HOSTID_BIOS)
	{
		str = HOSTID_BIOS_STRING;
		sprintf(print, "%s%lx", str, id->id.data);	/* OVERRUN */
	}
#endif
	else if (id->type == HOSTID_COMPOSITE)
	{
		str = HOSTID_COMPOSITE_STRING;
		sprintf(print, "%s%s", str, id->hostid_composite);	/* OVERRUN */
	}
	
#ifndef PC_UTILS
	else if (id->type >= HOSTID_VENDOR)
	{
	  LM_VENDOR_HOSTID *h = (LM_VENDOR_HOSTID *)0;

		if (job->options->vendor_hostids)
		{
			for (h = job->options->vendor_hostids; h; h = h->next)
			{
				if (id->type == h->hostid_num)
				{
					sprintf(print, "%s=%s", h->label,
								id->id.vendor);	/* OVERRUN */
					break;
				}
					
			}
		}
		if (!h && job->options->print_vendor_id)
		{
			sprintf(print, "%s", 
				(*LM_CALLBACK_PRINT_VID_TYPE
				 job->options->print_vendor_id)(id));	/* OVERRUN */
		} 
		else if (!h)
		{
/*
 *			P3353 -- used to be that id.vendor had the =
 *				in the value -- now I add it...
 */
			sprintf(print, "%s=%s", 
					id->vendor_id_prefix ?
					id->vendor_id_prefix : "VENDORDEF=", 
						id->id.vendor);		/* OVERRUN */
		}
	}
#endif
	else
		print[0] = '\0';

	return(print);
}
/*
 *	id_types_match -- to handle special cases
 */
int API_ENTRY
l_id_types_match(t1, t2)
int t1, t2;
{
	/* sentinel kludge  -- if both are sentinal, return true */
	if ((t1 == HOSTID_FLEXID5_KEY || t1 == HOSTID_FLEXID1_KEY) &&
		(t2 == HOSTID_FLEXID5_KEY || t2 == HOSTID_FLEXID1_KEY) )
			return 1;
	return (t1 == t2); /* otherwise, simple test */
}
