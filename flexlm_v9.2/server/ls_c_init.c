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
 *	Module: $Id: ls_c_init.c,v 1.6 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function:	ls_c_init(ca, comm_revision)
 *
 *	Description: 	Initializes a client structure.
 *
 *	Parameters:	(CLIENT_ADDR *) ca - Struct to be initialized
 *			(int) comm_revision - COMM_REVISION of this client.
 *
 *	Return:		None
 *
 *	M. Christiano
 *	7/23/90
 *
 *	Last changed:  11/13/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif

CLIENT_DATA *
ls_c_init(ca, comm_revision)
CLIENT_ADDR *ca;
int comm_revision;
{
  CLIENT_DATA *c;
      
	c = ls_add_client(ca);
	ls_ca_copy(&c->addr, ca);
	c->name[0] = '\0';		/* Username */
	c->node[0] = '\0';		/* Node */
	c->vendor_def[0] = '\0';	/* Vendor-defined data */
	c->use_vendor_def = 1;		/* Use Vendor-defined data */
	c->inet_addr[0] =		/* Internet address */
	c->inet_addr[1] =
	c->inet_addr[2] =
	c->inet_addr[3] = 0;
	c->display[0] = '\0';		/* Display */
	c->time = 0L;			/* Time started */
	c->encryption = 0; 		/* Comm encryption code */
	c->comm_version = 1; 		/* Communication version */
	c->comm_revision = comm_revision; /*  Communication revision */
	c->udp_sernum = 0;		/*incremented by l_msg_cksum()*/
	if (ca->transport == LM_UDP)
	    c->udp_last_resp = (char *)LS_MALLOC(LM_MSG_LEN+1);
	c->capacity = 1;
	/* set to latest time possible to avoid timing out */
	c->lastcomm = time(0);
	c->tcp_timeout = c->udp_timeout = 10000;
	if (lm_job->lm_errno == LM_SERVER_MAXED_OUT)
	{
		DLOG(("Setting NOMORE\n"));
		c->flags |= CLIENT_FLAG_NOMORE;
	}
	return c;
}

void
ls_client_add_hostid(client, hostid)
CLIENT_DATA *client;
HOSTID *hostid;
{
  HOSTID *h, *sav = (HOSTID *)0;

	DEBUG_INIT
	for (h = client->hostids; h; h = h->next) /* find next null pointer */
	{
		if (l_hostid_cmp_one(lm_job, h, hostid))
		{
			/* we already have it, free it then return */
			lc_free_hostid(lm_job, hostid);
			return;
		}
		sav = h;
	}
	if (sav)
		sav->next = hostid;
	else
		client->hostids = hostid;
}
