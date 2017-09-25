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
 *	Module: $Id: l_host.c,v 1.17 2003/04/18 23:48:03 sluu Exp $
 *
 *	Function: l_host(job, id)
 *
 *	Description: Verifies that the program is running on the specified host
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(HOSTID *) id - The host id
 *
 *	Return:		(int) - 0 - OK, ie. we are running on the specified
 *					host.
 *				NOTTHISHOST - We are not running on the
 *						specified host.
 *
 *	M. Christiano
 *	2/13/88
 *	Last changed:  11/3/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "l_socket.h"
#include "flex_utils.h"
#ifndef PC
#include <netdb.h>
#include <sys/socket.h>
#else
#include "pcsock.h"
#endif


#ifdef apollo
static int first = 1;		/* Only do the extended check once
							(if it succeeds) */
#endif

#if defined(PC)
typedef int (LM_CALLBACK_TYPE *LM_CALLBACK_CHECK_VID)(HOSTID *id1, HOSTID *id2);
#define LM_CALLBACK_CHECK_VID_TYPE (LM_CALLBACK_CHECK_VID)
#else
#define LM_CALLBACK_CHECK_VID_TYPE
#endif

static int l_host_one lm_args(( LM_HANDLE *, HOSTID *));
#ifndef EMBEDDED_FLEXLM
static int domain_eq lm_args(( LM_HANDLE *, char *,char *));
#endif

/*
 *	l_host checks the whole list -- it returns success if any match
 */
API_ENTRY
l_host(job, id)
LM_HANDLE *job;		/* Current license job */
HOSTID *id;	/* The host we are checking for */
{
  HOSTID *idp;
  int ret = LM_NOTTHISHOST;

	for (idp = id; idp; idp = idp->next)
	{
		if (!(ret = l_host_one(job, idp))) return 0;
	}
	LM_SET_ERROR(job, ret, 57, 0, l_asc_hostid(job, id), LM_ERRMASK_ALL);
	return ret;
}


/*
 *	l_host_one checks one hostid
 */
static
int
l_host_one(job, id)
LM_HANDLE *job;		/* Current license job */
HOSTID *id;	/* The host we are checking for */
{
  int ret = 0;
  HOSTID *hid;

/*
 *	If we have ANY hostid, pass it here
 */
	if (id->type == HOSTID_ANY) return(ret);
	if (id->type == HOSTID_FLEXLOCK) return(ret);
	if (id->type == HOSTID_SERNUM_ID) return ret;
/*
 *	First, we put the platforms where we do extended hostid
 *	checking.  If the extended check fails, we return bad
 *	status.  If it succeeds, we go on to do the generic
 *	hostid check.
 */

#ifdef apollo
	if (l_getattr(job, EXTENDED_HOSTID) == EXTENDED_HOSTID_VAL)
	{
		if (first && job->options->normal_hostid == 0)
		{
		  long real_id;

			real_id = l_apollo_id();
			hid = l_getid_type(job, (short) HOSTID_LONG);
			if (hid == NULL || id->type != HOSTID_LONG ||
			    real_id != hid->hostid_value)
			{

				return(LM_NOTTHISSHOST);
			}
			first = 0;
		}
	}
#endif

/*
 *	END OF EXTENDED CHECKS.  Normal hostid checking follows
 */

/*
 *	Compare the hostid we are looking for with what is
 *	on this machine.
 */
	job->host_comp  = id;
	hid = l_getid_type(job, id->type);
#ifdef SUPPORT_METER_BORROW
	if ((id->type == HOSTID_METER_BORROW) && !hid)
	{
		return LM_BORROW_ERROR;
	}
#endif /* SUPPORT_METER_BORROW */
	job->host_comp = 0;
	if (hid && l_hostid_cmp(job, hid, id))
	{
		return(0);
	}
	ret = LM_NOTTHISHOST;

#ifdef HP
/*
 *      On HPs we also try the ID Module if the uname fails
 */
	if (job->options->flags & LM_OPTFLAG_SUPPORT_HP_IDMODULE)
	{
		hid = l_getid_type(job, HOSTID_ID_MODULE);
		if (hid && l_hostid_cmp(job, hid, id))
		{
			return(0);
		}
	}
#endif
	return ret;
}
/*
 * 	l_hostid_cmp_exact -- compares 2 hostid lists -- all hostids must be
 *			identical, and in the same order
 *			1 == identical,
 *			0 == not identical
 */
int API_ENTRY
l_hostid_cmp_exact(job, h1, h2)
LM_HANDLE *job;		/* Current license job */
HOSTID *h1;
HOSTID *h2; /* the one in the license file */
{
  HOSTID *hid1, *hid2;

	for (hid1 = h1, hid2 = h2; hid1 && hid2;
				hid1 = hid1->next, hid2 = hid2->next)
	{
		if (!l_id_types_match(hid1->type, hid2->type) ||
			memcmp(&hid1->id, &hid2->id, sizeof(hid1->id)))
			return 0;
	}
	if (hid1 || hid2) return 0;
	return 1;
}

/*
 * 	l_hostid_cmp -- compares 2 hostid lists -- if any in the 2 lists match,
 *			returns 1, else 0
 */
int API_ENTRY
l_hostid_cmp(job, h1, h2)
LM_HANDLE *job;		/* Current license job */
HOSTID *h1;
HOSTID *h2; /* the one in the license file */
{
  HOSTID *hid1, *hid2;

	if (!h1 && !h2) return 1;
	if (!h1 || !h2) return 0; /* NOHOSTID vs. valid hostid */
	for (hid1 = h1; hid1; hid1 = hid1->next)
		for (hid2 = h2; hid2; hid2 = hid2->next)
			if (l_hostid_cmp_one(job, hid1, hid2))
				return 1;
	return 0;
}

/*
 * 	l_hostid_cmp_one -- compares 2 hostid -- if match
 *			returns 1, else 0
 */

int API_ENTRY
l_hostid_cmp_one(job, h1, h2)
LM_HANDLE *job;		/* Current license job */
HOSTID *h1;
HOSTID *h2; /* the one in the license file */
{
  int cmp = 0; /* assume failure */

/*
 *	First make broad comparisons, including if types don't match
 *	or any, etc.
 */
	if (!h1 && !h2)
		cmp = 1; /* success */
	else if (!h1 || !h2)
		; /* failure */
	else if (!l_id_types_match(h1->type, h2->type))
		; /* failure */
	else if ((h1->type == HOSTID_ANY && h2->type == HOSTID_ANY ) ||
                (h1->type == HOSTID_FLEXLOCK && h2->type == HOSTID_FLEXLOCK ))
	{
		cmp = 1; /* success */
	}
/*
 *	Passed initial tests.
 *	Following are the main comparison routines
 */
/*
 *	SERNUM_ID -- we encrypt these, but we should never actually
 *	compare them.
 */
	else if (h1->type == HOSTID_SERNUM_ID && h2->type == HOSTID_SERNUM_ID )
	{
		cmp = 1; /* success */
	}
	else if ( (h1->type == HOSTID_LONG) ||
  		  (h1->type == HOSTID_FLEXID1_KEY) ||
  		  (h1->type == HOSTID_FLEXID5_KEY) ||
  		  (h1->type == HOSTID_FLEXID6_KEY) ||
#ifdef SUPPORT_METER_BORROW
  		  (h1->type == HOSTID_METER_BORROW) ||
#endif /* SUPPORT_METER_BORROW */
		  (h1->type == HOSTID_DISK_SERIAL_NUM) || 
		  (h1->type == HOSTID_CPU) || 
		  (h1->type == HOSTID_DISK_GEOMETRY) || 
		  (h1->type == HOSTID_BIOS) )
	{
		if ((h1->hostid_value & 0xffffffff) ==
					(h2->hostid_value & 0xffffffff))
			cmp = 1; /* success */
	}
	else if (h1->type == HOSTID_ETHER)
	{
	  int i;

		for (i = 0; i < 6; i++)
		{
			if (h1->hostid_eth[i] != h2->hostid_eth[i]) break;
		}
		if (i == 6) cmp = 1;
	}
	else if (h1->type == HOSTID_INTEL32 ||
		h1->type == HOSTID_INTEL64 ||
		h1->type == HOSTID_INTEL96)
	{
	  int i;
	  int len = (h1->type == HOSTID_INTEL32) ? 32/32 :
		  (h1->type == HOSTID_INTEL64) ? 64/32 : 96/32;

		for (i = 0; i < len; i++)
		{
			if (h1->id.intel64[i] != h2->id.intel64[i]) break;
		}
		if (i == len) cmp = 1;
	}
	else if ((h1->type == HOSTID_STRING)
	 	|| (h1->type == HOSTID_FLEXID2_KEY)
	 	|| (h1->type == HOSTID_FLEXID3_KEY)
	 	|| (h1->type == HOSTID_FLEXID4_KEY)
	 	|| (h1->type == HOSTID_FLEXID_FILE_KEY))
	{
		if (l_keyword_eq(job, h1->hostid_string, h2->hostid_string)) cmp = 1;
	}
	else if (h1->type == HOSTID_USER)
	{
		if (l_keyword_eq(job, h1->hostid_user, h2->hostid_user)) cmp = 1;
	}
	else if (h1->type == HOSTID_DISPLAY)
	{
		if (l_keyword_eq(job, h1->hostid_display, h2->hostid_display)) cmp = 1;
	}
	else if (h1->type == HOSTID_HOSTNAME)
	{
		if (l_keyword_eq(job, h1->hostid_hostname, h2->hostid_hostname)) cmp = 1;
	}
	else if (h1->type == HOSTID_COMPOSITE)
	{
		if (l_keyword_eq(job, h1->hostid_composite, h2->hostid_composite)) cmp = 1;
	}
#ifndef EMBEDDED_FLEXLM
	else if (h1->type == HOSTID_DOMAIN)
	{
		if (domain_eq(job, h1->hostid_domain, h2->hostid_domain))
				cmp = 1;
	}
#endif
	else if (h1->type == HOSTID_INTERNET)
	{
		cmp = !l_inet_cmp(h1->hostid_internet, h2->hostid_internet);
	}
	else if (h1->type >= HOSTID_VENDOR)
	{
	  LM_VENDOR_HOSTID *h = 0;

		if (job->options->vendor_hostids)
		{

			for (h = job->options->vendor_hostids; h; h = h->next)
			{
				if (h->hostid_num == h1->type)
				{
					if (l_keyword_eq(job, h1->id.vendor,
							h2->id.vendor))
						cmp = 1;
					break;
				}
			}
		}
		if (!h && job->options->check_vendor_id)
		{
			cmp = (* LM_CALLBACK_CHECK_VID_TYPE
				job->options->check_vendor_id)(h1, h2);
		}
	}
	return(cmp);
}

int
l_hostid_copy(job, to, from)
LM_HANDLE *job;
HOSTID **to;
HOSTID *from;
{
	if (!from) return (int)(*to = 0); /* success */

	return l_get_id(job, to, l_asc_hostid(job, from));
}

#ifndef EMBEDDED_FLEXLM
static
int
domain_eq(job, client_domain, lic_domain)
LM_HANDLE *job;
char *client_domain, *lic_domain;
{
  char *cp = client_domain; /* actual client domain */
  char *hostp;
  char hostname[MAX_HOSTNAME + 1];
  int ret = 0; /* assume they don't match */
  struct hostent *he2;
  char ipbuf[4];

/*
 *		The domain in the license can be a subset of the true domain
 */
#if !defined ( PC16 ) && ! defined ( NLM )
	while (cp)
	{
		if (L_STREQ(cp, lic_domain))
		{
			ret = 1;
			break;
		}
		if (cp = strchr(cp, '.')) cp++;
	}
	if (!ret) return 0;
/*
 *	Now do a reverse lookup on their hostname to verify
 *	If any part of this fails, we return success anyway,
 *	since this is not a foolproof test.
 */
	if (job->flags & LM_FLAG_IS_VD)
	{
		/*if (!(he1 = gethostbyname(lc_hostname(job, 1)))) return 1;*/
		hostp = lc_hostname(job, 1);
	}
	else
	{
		if (gethostname(hostname, MAX_HOSTNAME))
			return 1;
		hostname[MAX_HOSTNAME] = 0;
		hostp = hostname;
		/*if (!(he1 = gethostbyname(hostname))) return 1;*/
	}
	if (!l_get_ipaddr(hostp, ipbuf, 0, 0))
		return 1;
#if 0
	ipbuf[0] = he1->h_addr[0];
	ipbuf[1] = he1->h_addr[1];
	ipbuf[2] = he1->h_addr[2];
	ipbuf[3] = he1->h_addr[3];
#endif /* 0 */
	if (!(he2 = gethostbyaddr(ipbuf, 4, AF_INET)))
		return 1;
	if (!(cp = he2->h_name)) return 1;
	if (!(cp = strchr(cp, '.'))) return 1;  /* no domain in the hostname */
	cp++;
	ret = 0; /* assume false */
	while (cp)
	{
		if (L_STREQ(cp, lic_domain))
		{
			ret = 1;
			break;
		}
		if (cp = strchr(cp, '.')) cp++;
	}
	return ret;
#else
	return FALSE;
#endif /* !PC16 */

}
#endif /* EMBEDDED_FLEXLM */
