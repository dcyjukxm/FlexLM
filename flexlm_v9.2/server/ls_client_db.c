/******************************************************************************

		COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_client_db.c,v 1.7 2003/04/18 23:47:59 sluu Exp $
 *	Function:	ls_add_client(), ls_lookup_client(), 
 *			ls_delete_client(), ls_all_clients()
 *
 *	Description: 	These uses CLIENT_ADDR as KEY to return 
 *			pointer to appropriate CLIENT_DATA struct
 *			If KEY is new, the struct is created (malloc'd).
 *			
 *			ls_delete_client() frees the space associated
 *			with the KEY argument.
 *		
 *			ls_all_clients() takes a callback argument,
 *			which is the function that gets the clients.
 *
 *
 *	D. Birns
 *	4/22/93
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <sys/types.h>
#include "lsserver.h"
#include "ls_sprot.h"
#include "lssymtab.h"
#include <errno.h>
#define CLIENT_DELETE   0
#define CLIENT_ADD      1
#define CLIENT_LOOKUP   2
long ls_connections;


static SpSymTab *ls_client_tab = (SpSymTab *)0;

/* local prototypes here */
static int ls_client_compare lm_args((void *client1, void *client2));
static CLIENT_DATA * ls_new_client lm_args((lm_noargs));
static void ls_process_client lm_args((CLIENT_ADDR *c,
			            int mode, CLIENT_DATA **ret )) ;
static void free_client lm_args((CLIENT_DATA *));
static print_clients lm_args(( CLIENT_DATA *,void *));
/* End prototypes */

#ifndef RELEASE_VERSION
static char * debug_handle = (char *)-99;
static print_clients();
#endif





/*
 *ls_delete_client -- delete a client from the CLIENT_DATA database
 */

void
ls_delete_client(ca)
CLIENT_ADDR *ca;
{
	ls_process_client(ca, CLIENT_DELETE, NULL);
	return;
}


/*
 * ls_add_client -- return a pointer to CLIENT_DATA struct using
 * the fd or sockaddr_in as a KEY.  If this is a new KEY, it's
 * inserted in the CLIENT_DATA database
 */
CLIENT_DATA *
ls_add_client(ca)
CLIENT_ADDR *ca;
{
	CLIENT_DATA *cd;

	ls_process_client(ca, CLIENT_ADD, &cd);
	cd->handle = ls_create_handle(cd);
	return cd;
}

/*
 *	ls_lookup_client(): given an ADDR, return the DATA
 */
CLIENT_DATA *
ls_lookup_client(ca)
CLIENT_ADDR *ca;
{
	CLIENT_DATA *cd = NULL;
	ls_process_client(ca, CLIENT_LOOKUP, &cd);
	return cd;
}

/*
 *      ls_ca_copy()
 *                      ca1 = ca2
 *                      like strcpy()
 */
void
ls_ca_copy(ca1, ca2)
CLIENT_ADDR *ca1, *ca2;
{
	memcpy((char *)ca1, (char *)ca2, sizeof(CLIENT_ADDR));
}




/*
 *	ls_process_client() - add/delete/lookup based on a CLIENT_ADDR
 *				if it's an add, the third arg is the 
 *				returned (CLIENT_DATA **)
 */
static 
void
ls_process_client(ca, mode, ret ) /*return a handle for ADD or lookup*/
CLIENT_ADDR *ca;
int mode; 		/* CLIENT_DELETE, CLIENT_ADD, CLIENT_LOOKUP*/
CLIENT_DATA ** ret; 	/* used when adding */
{

  CLIENT_DATA *client_p;
  SpSymHeader *shp;

	
	if (!ls_client_tab) /* first time -- initialize */
	{
/*
 *		Note:  The size, 2nd arg, is 2^n size hash.  So, 10 is
 *			2^10 (=1024) which is a pretty big hash number
 *
 */
		ls_client_tab = SpSymInit("clients", 10 , sizeof(ADDR_ID));
	}

#ifndef RELEASE_VERSION
	if (debug_handle == (char *)-99)
		debug_handle = l_real_getenv("DEBUG_CLIENT_DB");
	if (debug_handle && (mode != CLIENT_LOOKUP))
		DLOG(("ls_process_client: %s %d, mode %s\n",
		    ca->transport == LM_TCP ? "fd" : 
			    ca->transport == LM_UDP ? "port" : "local", 
		    ca->transport == LM_TCP ? ca->addr.fd: 
			    ca->transport == LM_UDP ? 
				ntohs(ca->addr.sin.sin_port) : 
				ca->addr.fd, 
			mode == CLIENT_ADD ? "CLIENT_ADD" :
		        mode == CLIENT_DELETE ? "CLIENT_DELETE" :
		        mode == CLIENT_LOOKUP ? "CLIENT_LOOKUP" :
		                            "ERROR: UNKNOWN MODE"));
	if (debug_handle && (!strcmp(debug_handle, "hash")))
		printf(
		"hash stats:  syms %d bins %d searches %d cmps %d maxlen %d\n", 
			ls_client_tab->numsyms,
			ls_client_tab->numbins,
			ls_client_tab->numsearches,
			ls_client_tab->numcmps,
			ls_client_tab->maxlen);
	if (debug_handle && (!strcmp(debug_handle, "dump")))
		ls_print_clients();
#endif
	client_p = ls_new_client();

	/*Initialize client struct for the lookup*/
	memcpy((char *)&client_p->addr, (char *)ca, sizeof(CLIENT_ADDR));

		/*perform the search...*/
	if (shp = SpSymFind(ls_client_tab, (char *)&client_p->addr.addr))
	{
		/* We found it */

		free((char *)client_p);
		client_p = (CLIENT_DATA *)shp->name;

		switch (mode)
		{
		case CLIENT_ADD: /*fall through*/
			DLOG(("OOPS: CLIENT_ADD found client (%d)\n", 
			                                       ca->addr.fd));
		case CLIENT_LOOKUP:
			*ret = client_p;
			return;
		case CLIENT_DELETE:
			ls_replog_delete_client(client_p);
			if (ca->is_fd) ls_connections--;
			free_client(client_p);
			
			ls_delete_handle(client_p->handle);
			LM_SET_NET_ERRNO(0);

			if (!SpSymRemove(ls_client_tab, shp))
			{
			        DLOG(("SpSymRemove failed to delete a client record "));
			        _DLOG((lmtext("%s:%d\n"), __FILE__, __LINE__));
			        return;
			}
			/*free these up -- not needed anymore*/
			free((char *)shp);
			free((char *)client_p);
			return;
		default:
			LOG( ("INTERNAL ERROR: bad process_client case\n"));

			break;
		}
	}
	else  /* Lookup failed */
	{
		switch(mode)
		{
		case CLIENT_LOOKUP: /* failed */
#ifndef RELEASE_VERSION
			if (debug_handle) DLOG(("lookup failed\n"));
#endif
			free((char *)client_p);
			*ret = NULL; 
			return;

		case CLIENT_ADD: /*Install this record in database */
			if (ca->is_fd) ls_connections++;
			shp = (SpSymHeader *)LS_MALLOC(sizeof (SpSymHeader));
			shp->name = (char *)&client_p->addr;
			if (!(shp = SpSymAdd(ls_client_tab, shp)))
			{
			    DLOG(
			      ("INTERNAL ERROR: SpSymAdd failed to install "));
			    _DLOG(("a client record, "));
			    _DLOG(("fd = %d, exiting\n", ca->is_fd ? 
							ca->addr.fd : -1));
			    ls_go_down(EXIT_MALLOC);
			}
			*ret = client_p;
			break;

		case CLIENT_DELETE:
			DLOG(("INTERNAL ERROR: ls_delete_client couldn't find client\n"));
#ifndef RELEASE_VERSION
			{
			  int i; char *cp = (char*)ca;
				DLOG(("ca:"));
				for (i = 0; i < sizeof(CLIENT_ADDR); i++)
					_DLOG(("%02x", cp[i]));
				_DLOG(("\n"));
			}
			DLOG(("Can't find first in this list:\n"));
			print_clients(client_p, 0);
			ls_print_clients();
#endif
			free((char *)client_p);
			break;
		default:
			free((char *)client_p);
			DLOG( ("INTERNAL ERROR: bad process_client case\n"));
			break;
		}
	}
}


/*
 *	ls_all_clients -- call callback function for each client in
 *			  the database, and call this function with
 *			  data as an arg
 */

void
#ifdef ANSI
ls_all_clients(int (*callback) (CLIENT_DATA *cd, void *data), void *data)
#else
ls_all_clients(callback, data)
int (*callback) lm_args((CLIENT_DATA *cd, void *data));
void *data;
#endif
{
  SpSymHeader *shp = (SpSymHeader *)0;
	if (!ls_client_tab)
		return;
	while (shp = SpSymNext(ls_client_tab, shp))
	{
		if (!(*callback)((CLIENT_DATA *)shp->name, data))
			return;
	}
}


/*
 * routines to malloc CLIENT_DATA or CLIENT_DATA structs
 */
static 
CLIENT_DATA *
ls_new_client()
{
	CLIENT_DATA *p = (CLIENT_DATA *)LS_MALLOC(sizeof(CLIENT_DATA));
	memset((char *)p, 0, sizeof(CLIENT_DATA));
	return p;
}

/*
 * Routines used for communications
 */
void
ls_encode_ca(where, ca)
char *where;
CLIENT_ADDR *ca;
{
	if (ca) l_encode_int(where, ls_handle_by_addr(ca));
	else l_encode_int(where, 0);
}
void
ls_decode_ca(from, ca)
char *from;
CLIENT_ADDR *ca;
{
	CLIENT_ADDR *ca2;
	HANDLE_T i;
	l_decode_int(from, &i);
	if (ca2 = ls_addr_by_handle(i))
		ls_ca_copy(ca, ca2);
	else
		memset((char *)ca, 0, sizeof(CLIENT_ADDR));
}

static 
print_clients(cd, ptr)
CLIENT_DATA *cd;
void *ptr; /*ignored*/
{
 int i;
 char *cp = (char *)cd;
	DLOG(("ptr %x, is_fd %d port %d, fd %d handle %d raw:",
		cd,
		cd->addr.is_fd,
		ntohs(cd->addr.addr.sin.sin_port),
		cd->addr.addr.fd,
		cd->handle
		));
	for (i = 0; i < sizeof(CLIENT_ADDR); i++)
		_DLOG(("%02x", cp[i]));
	_DLOG(("\n"));
	return 1;
}

#ifndef RELEASE_VERSION 
void
ls_print_clients()
{
	ls_all_clients(print_clients, (void *)NULL);
}
#endif /* RELEASE_VERSION */

static
void
free_client(client)
CLIENT_DATA *client;
{
  GROUP *g, *nextg;

	if (client->udp_last_resp) free((char *)client->udp_last_resp);

#ifdef SUPPORT_FIFO
	if (client->addr.local.name) free((char *)client->addr.local.name);
#endif
	if (client->project) free(client->project);
	lc_free_hostid(lm_job, client->hostids);
	if (client->sn) free(client->sn);
	for (g = client->groups; g; g = nextg)
	{
		nextg = g->next;
		free(g->name);
		free(g);
	}
	for (g = client->hostgroups; g; g = nextg)
	{
		nextg = g->next;
		free(g->name);
		free(g);
	}
}
