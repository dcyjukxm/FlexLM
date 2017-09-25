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
 *	Module: $Id: l_lmgrdr.c,v 1.15 2003/03/13 23:07:51 kmaclean Exp $
 *
 *	Function:	l_lmgrd_running
 *
 *	Description: 	return 0 if can connect to port@host.
 *
 *	Parameters:	port, hostname
 *
 *	Return:		FLEXlm errno if error, else 0 if success
 *
 *	D. Birns
 *	7/7/97
 *
 *	Last changed:  11/26/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "l_prot.h"
#include "lgetattr.h"

#ifndef PC
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#else
#include "pcsock.h"
#endif /* !PC */

static int supported lm_args(( char *, char *vendor));

int
la_init(LM_HANDLE *job)
{
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 112, 0);
                return(0);
        }
	return l_la_init(job);
}

int
l_la_init(LM_HANDLE *job)
{
	job->flags |= LM_FLAG_LMUTIL;
	job->options->flags |= LM_OPTFLAG_NO_HEARTBEAT;
	if ( job->options->commtype == LM_UDP )
			job->options->commtype = LM_TCP;

	if (lc_set_attr(job, LM_A_TRY_COMM, (LM_A_VAL_TYPE)1)) 
		return job->lm_errno;
	if (lc_set_attr(job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)0)) 
			return job->lm_errno;
	if (lc_set_attr(job, LM_A_ALLOW_SET_TRANSPORT, (LM_A_VAL_TYPE)0)) 
		return job->lm_errno;
	if (lc_set_attr(job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE) 0)) 
		return job->lm_errno;
	if (lc_set_attr(job, LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)4)) 
		return job->lm_errno;
	return 0;
	
}

int
API_ENTRY
l_lmgrd_running(job, port, hostname, transport)
LM_HANDLE_PTR job;
int port;	/* in natural, host order */
LM_CHAR_PTR hostname;
int transport;
{
  COMM_ENDPOINT endpoint;
#ifdef SUPPORT_IPX
  LM_SERVER * s;
#endif


	memset(&endpoint, 0, sizeof(endpoint));
	if (!hostname || !*hostname)
		return LM_BADPARAM;

#ifdef SUPPORT_IPX

	if ( LM_SPX == transport )
	{
		s= (LM_SERVER *) port;	
		memcpy(&endpoint.transport_addr.spx_addr, &s->spx_addr, 
						sizeof(SOCKADDR_IPX) );
		port = endpoint.transport_addr.spx_addr.sa_socket ;
	}
#endif

	endpoint.transport = transport;

#ifdef SUPPORT_IPX
	if (LM_SPX != transport)
#endif
		endpoint.transport_addr.port = 
					htons((unsigned short)(port & 0xffff));


	if (l_connect_endpoint(job, &endpoint, hostname) >=0 )
	{
		lc_disconn(job, 1);
		return 0;
	}
	else
		return job->lm_errno;
}

LMGRD_STAT *
la_lmgrds(job, vendor)
LM_HANDLE *job;
char *vendor;
{
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 112, 0);
                return(0);
        }
	return l_lmgrds(job, vendor);
}

LMGRD_STAT_PTR API_ENTRY 
l_lmgrds(job, vendor)
LM_HANDLE_PTR job;
LM_CHAR_PTR vendor; /* optional */
{
  LMGRD_STAT *lmgrds = 0;
  LMGRD_STAT *lp, *last;
  LMGRD_STAT *lmgrd;
  LMGRD_STAT l, *ltmp;
  int first_unused_default_port;
  char *hostname;
  int verrev = 0;
  LM_SERVER *sav_servers = job->servers;
  int more = 0;
  char sav_server_name[MAX_HOSTNAME + 1]; /* to avoid checking default
					   *	range more than once 
					   */

	job->servers = 0;
	*sav_server_name = 0;
	job->flags |= LM_FLAG_CONNECT_NO_HARVEST;

	if (!job->lic_files) l_flush_config(job);

	for (job->lfptr = 0; 
			(job->lfptr < job->lm_numlf) || more; 
			more ? 0 : job->lfptr++)
	{
	  COMM_ENDPOINT e;
	  unsigned short port;
	  LM_SERVER *server;

		memset(&l, 0, sizeof(l));
		if (server = lc_master_list(job))
		{
		  LM_SERVER *s, *ls;

			l.server = (LM_SERVER *) l_malloc(job, 
					MAX_SERVERS * sizeof (LM_SERVER));
			for (s = server, ls = l.server ; s; s = s->next, ls++)
			{
/*
 *				Copy only the things that we need,
 *				and avoid things like hostids...
 */
				memcpy(ls, s, sizeof(LM_SERVER));
				ls->idptr = 0; /* we don't need it, and
						  causes malloc/free confusion
						  */
				if (s->next) ls->next = ls + 1;
			}
/*
 *			This host -- assume it's not redundant 
 */
			if ((l.server->sflags & L_SFLAG_THIS_HOST) &&
					(hostname = lc_lic_where(job)) &&
					(hostname = strchr(hostname, '@')))
			{
				strcpy(l.server->name, hostname + 1);
			}
		}
#ifdef SUPPORT_IPX		
		
		if ( l.server && l.server->commtype == LM_SPX  )
			l.server->port=l.server->spx_addr.sa_socket ;	
#endif
		
		if (l.server && l.server->port == -1 &&
			L_STREQ(l.server->name, sav_server_name))
			goto next_file; /* nothing to do */
		else if (l.server && l.server->port == -1 &&
			!L_STREQ(l.server->name, sav_server_name ) 
#ifdef SUPPORT_IPX
			&& l.server->commtype != LM_SPX
#endif
			)
		{
/*
 *			Find first unused port for this server
 */
	
			first_unused_default_port = LMGRD_PORT_START;
			for (lp = lmgrds; lp; lp = lp->next)
			{
				if (lp->server && 
				  L_STREQ(lp->server->name, l.server->name)
				    && (lp->server->port >= LMGRD_PORT_START)
				    && (lp->server->port <= job->port_end))
				{
					first_unused_default_port = 
						lp->server->port + 1;
					break;
				}
						
			}
			for (port = more ? more : first_unused_default_port; 
				port <= job->port_end; port++)
			{
				if (port < job->port_end) more = port + 1;
				else 
				{
					more = 0;
					strcpy(sav_server_name, l.server->name);
				}
				if (!l_lmgrd_running(job, port, 
					l.server->name, LM_TCP))
				{
					l.up = 1;
					l.server->port = port;
					l.flexlm_ver = job->daemon->ver;
					l.flexlm_rev = job->daemon->rev;
					first_unused_default_port = port + 1;
					break;
				}
				else
				{
					if (first_unused_default_port 
						!= LMGRD_PORT_START)
						l.up = -1;
					else
						l_err_info_cp(job, &l.e, 
								&job->err_info);
					if (job->lm_errno == LM_HOSTDOWN)
						break;
				}
					
			}
		}
		else if (server)
		{
		  LM_SERVER *s;
		  int transport;
		  int port_or_serverstruct;
			for (s = l.server; s; s = s->next)
			{
#ifdef SUPPORT_IPX
				if (s->commtype == LM_SPX )
				{
					transport = LM_SPX;
					port_or_serverstruct = (int)s;
				}
				else
#endif
				{
					transport = LM_TCP;
					port_or_serverstruct = s->port;
				}

				if (!l_lmgrd_running(job, port_or_serverstruct, 
						s->name, transport ))

				{
					l.up = 1;
					l.flexlm_ver = job->daemon->ver;
					l.flexlm_rev = job->daemon->rev;
					break;
				}
			}
			if (!s) l_err_info_cp(job, &l.e, &job->err_info);
		}
		else goto next_file; /* It's probaly an "uncounted" license */
		if (server)
		{
			if (l.server->port == -1)
				sprintf(l.port_at_host, "@%s", l.server->name);
			else
			{
				if (l.server->next && l.server->next->next)
					sprintf(l.port_at_host, "%d@%s,%d@%s,%d@%s", 
						l.server->port, l.server->name,
						l.server->next->port, 
						l.server->next->name,
						l.server->next->next->port, 
						l.server->next->next->name);
#ifdef SUPPORT_IPX
				else if ( l.server->commtype == LM_SPX )
				{
#define NODENUM l.server->spx_addr.sa_nodenum
#define NETNUM l.server->spx_addr.sa_netnum
					sprintf(l.port_at_host, 
						"%d@%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X#%2.2X%2.2X%2.2X%2.2X", 
						htons(l.server->port),
						(unsigned char ) NODENUM[0] & 0xff,
						(unsigned char ) NODENUM[1] & 0xff,
						(unsigned char ) NODENUM[2] & 0xff,
						(unsigned char ) NODENUM[3] & 0xff,
						(unsigned char ) NODENUM[4] & 0xff,
						(unsigned char ) NODENUM[5] & 0xff,
						(unsigned char ) NETNUM[0] & 0xff,
						(unsigned char ) NETNUM[1] & 0xff,
						(unsigned char ) NETNUM[2] & 0xff,
						(unsigned char ) NETNUM[3] & 0xff );
					

				}

#endif
				else
					sprintf(l.port_at_host, "%d@%s", 
						l.server->port, l.server->name);
			}
		}

		if (l.up == -1) goto next_file;

		verrev = (l.flexlm_ver * 100) + (l.flexlm_rev);

		if (l.up && verrev >= 575)
		{
			l.license_paths = 
				l_get_lfile(job, l.port_at_host,
						LM_FINDER_GET_PATHS, &e);
			l.vendor_daemons = 
				l_get_lfile(job, l.port_at_host,
						LM_FINDER_GET_DLIST, &e);
		}
		else if (job->lic_files)
		{
		  int len = 0;
		  int cnt;
		  char *cp;
			for (cnt = 0; cnt < job->lm_numlf && job->lic_files[cnt]; cnt++)
				len += strlen(job->lic_files[cnt]);
			
			cp = l.license_paths = (char *)l_malloc(job, len + cnt + 1);
			for (cnt = 0; cnt < job->lm_numlf && job->lic_files[cnt]; cnt++)
			{
				strcpy(cp, job->lic_files[cnt]);
				cp += strlen(cp);
				if (cnt < (job->lm_numlf -1))
					*cp++ = PATHSEPARATOR;
			}
		}
		l_free_job_servers(job, job->servers);
/*
 *		Make sure it's not a dup
 */
		for (lp = lmgrds; lp ; lp = lp->next)
		{
			if (lp->up && 
				L_STREQ(lp->port_at_host, l.port_at_host))
			{
				break;
			}
		}
		if (lp) 
		{
			goto next_file;
		}
/*
 *		If vendor specified, check it
 */
		if ((vendor && *vendor && !l.vendor_daemons) || 
			!supported(l.vendor_daemons, vendor)) goto next_file;

		lmgrd = (LMGRD_STAT *)l_malloc(job, sizeof (LMGRD_STAT));
		memcpy(lmgrd, &l, sizeof(l));
/*
 *		Make sure the list is in numeric port number order
 */
		for (last = 0, lp = lmgrds; lp; lp = lp->next)
		{
			if (lp->server->port > lmgrd->server->port)
			{
				if (last) 
				{
					lmgrd->next = lp;
					last->next = lmgrd;
				}
				else
				{
					lmgrd->next = lp;
					lmgrds = lmgrd;
				}
				break;
			}
			last = lp;
		}
		if (!last) lmgrds = lmgrd;
		else last->next = lmgrd;
		continue;
next_file:
		ltmp = (LMGRD_STAT *)l_malloc(job, sizeof(LMGRD_STAT));
		memcpy(ltmp, &l, sizeof(l));
		lc_free_lmgrd_stat(job, ltmp);
	}
	if (job->servers) l_free_job_servers(job, job->servers);
	job->servers = sav_servers; 
#ifdef PC
	job->flags &= ~LM_FLAG_CONNECT_NO_HARVEST;
	l_conn_harvest(job);
#endif
	return lmgrds;
}
/*
 *	lc_free_lmgrd_stat
 *	NOTE: The server part is tricky (for historic reasons)
 *	make sure that the hostids aren't allocated just for this
 *	server.
 */
void
API_ENTRY
lc_free_lmgrd_stat(job, lmgrds)
LM_HANDLE_PTR job; /* unused */
LMGRD_STAT_PTR lmgrds;
{
  LMGRD_STAT *next, *lmgrd;

	for (lmgrd = lmgrds; lmgrd; lmgrd = next)
	{
		next = lmgrd->next;
		if (lmgrd->license_paths)
			free(lmgrd->license_paths);
		if (lmgrd->license_file)
			free(lmgrd->license_file);
		if ( lmgrd->vendor_daemons)
			free(lmgrd->vendor_daemons);
		if (lmgrd->server) free(lmgrd->server);
		if (lmgrd->e.context) free(lmgrd->e.context);
		if (lmgrd->e.errstring) free(lmgrd->e.errstring);
		free(lmgrd);
	}
}

static
int 
supported(list, vendor)
char *list;
char *vendor;
{
  char *cp = list, *vp;
  char v[100];

	if (!vendor || !*vendor) return 1; /* ok */
	while (*cp)
	{
		vp = v;
		while(*cp && *cp != ' ')
			*vp++ = *cp++;
		*vp = 0;
		if (L_STREQ(v, vendor))
			return 1; /* found it */
		cp++;
	}
	return 0; /* specified, and not there */
}

