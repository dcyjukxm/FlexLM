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
 *	Module: $Id: l_master_lis.c,v 1.24 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	lc_master_list(job), 
 *			l_master_list_lfp(job, lf, servers)
 *
 *	Description:	Gets the list of master server nodes.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(LICENSE_FILE *)lf -- license file pointer
 *			(LM_SERVERS *)servers -- pointer to array of
 *						[MAX_SERVERS] servers.
 *
 *	Return:		(struct master_list *) - The master list structures or
 *						NULL, if no masters found.
 *						lc_master_list does NOT
 *						read the license file --
 *						it presumes the file was
 *						already read, and returns
 *						master list from the 
 *						line (CONFIG *)element in 
 *						job handle
 *			_lfp(LM_SERVER *) - points to filled in servers
 *					    array, actually read from file.
 *
 *	NOTE:
 *
 *	M. Christiano
 *	2/13/88
 *
 *	Last changed:  10/13/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"
#include "l_socket.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>

#ifndef PC
#include <netinet/in.h>
#endif

#ifdef USE_WINSOCK        
#include <pcsock.h>      
#endif                    


/*
 *	lc_master_list -- read the license file, and return
 *			a pointer to the servers.  This is
 *			actually hung off the job handle, and
 *			free'd and malloc'd each time.
 */
LM_SERVER * API_ENTRY
lc_master_list(job)
LM_HANDLE *job;		/* Current license job */
{
  LICENSE_FILE *lf;
  LM_SERVER *x;

	if (LM_API_ERR_CATCH) return 0;

	lf = l_open_file(job, LFPTR_CURRENT);
	if (!lf) 
	{
		LM_API_RETURN(LM_SERVER *, NULL)
	}

/*
 *	First time, malloc max space needed
 */
	if (!l_reset_job_servers(job)) LM_API_RETURN(LM_SERVER *,0)

	x = l_master_list_lfp(job, lf, job->servers);
	if (job->line && job->line->server  && 
		(job->line->server->sflags & L_SFLAG_THIS_HOST)
			&& job->servers && job->servers->idptr)
	{
	  char hostname[MAX_HOSTNAME + 1];
		/*job->line->server->port = -1;*/ /* P4778 */
	  if(job->line->server->idptr)
	  {
		  lc_free_hostid(job, job->line->server->idptr);
		  job->line->server->idptr = NULL;
	  }
	  
		job->line->server->idptr = 
			(HOSTID *)l_malloc(job, sizeof (HOSTID));
		memcpy(job->line->server->idptr, job->servers->idptr, 
			sizeof(HOSTID));
		/*job->servers->port = -1;*/
		gethostname(hostname, MAX_HOSTNAME);
		strcpy(job->line->server->name, hostname);
		strcpy(job->servers->name, hostname);
		x = job->servers;
	}
	if ((!x) && (!job->lm_errno))
		LM_SET_ERRNO(job, LM_NO_SERVER_IN_FILE, 66, 0);
	l_lfclose(lf);
	LM_API_RETURN(LM_SERVER *,(x))
}


LM_SERVER *API_ENTRY
l_master_list_lfp(job, lf, servers)
LM_HANDLE *job;		/* Current license job */
LICENSE_FILE *lf;
LM_SERVER *servers;
{
  char line[MAX_CONFIG_LINE+1];
  int cur = -1;
  int done = 0;
  LM_SERVER *s;
  int i;

	if (lf->type == LF_PORT_HOST_PLUS)
	{
	  char *cp = lf->ptr.commspec;

		cur++;
		memset(servers, 0, sizeof (LM_SERVER));
		while (!done)
		{
			if (*cp == LM_PORT_HOST_PLUS_SIGN)
			{
				cp++;
			}
	/* 
	 *		TODO redundant
	 */
			l_parse_commtype(job, cp, &servers[cur]);
			if (cp = strchr(cp, ','))
			{
				cp++;
				cur++;
				servers->next = &servers[cur];
			} 
			else
			{
				servers[cur].next = (LM_SERVER *)0;
				done = 1;
			}
			if (cur >= MAX_SERVERS) done = 1;
		} 
	} 
	else 
	{
		while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
		{
		    if (l_parse_server_line(job, line, &servers[cur+1])) 
		    {
			cur++;
			if (lf->endpoint.transport_addr.port 
						&& servers[cur].port == -1)
				servers[cur].port = 
					ntohs(lf->endpoint.transport_addr.port);
/*
 *			make sure it's not a dup
 */
			for (i = 0, s = servers; i < MAX_SERVERS && 
				cur > 0 && s != &servers[cur]; s++, i++)
			{
				if (s->port == servers[cur].port &&
					L_STREQ(s->name, servers[cur].name))
				{
					lc_free_hostid(job, servers[cur].idptr);
					if (servers[cur].filename) 
						free(servers[cur].filename);
					memset(&servers[cur], 0, sizeof(LM_SERVER));
					cur--;
					continue;
				}
			}

			if (cur > 0)
				servers[cur-1].next = &servers[cur];

			if (cur == MAX_SERVERS-1)
				break;			/* Found all we need */
		    }
		}
	}
	if (cur >= 0 && job->lm_errno != BADFILE)
		return(servers);
	else
	{
		return((LM_SERVER *)NULL);
	}
}

/*
 *	l_parse_server_line() - Parses a server line, in memory
 *
 * 	returns 1 if parsed the line as a server line, 0 if not
 */

int API_ENTRY	
l_parse_server_line(job, line, sb)
LM_HANDLE *job;		/* Current license job */
char *line;
LM_SERVER *sb;
{
  char *f1, *f2, *f3, *f4, *f5;
  int nflds;

	if (!line ) return 0;
	if ( !(f1 = (char *)l_malloc(job, (MAX_CONFIG_LINE+1)*5)) )
	{
		return 0;
	}
	else
	{
		memset(f1, 0, ((MAX_CONFIG_LINE+1) * 5));
		f2 = f1+MAX_CONFIG_LINE+1;
		f3 = f2+MAX_CONFIG_LINE+1;
		f4 = f3+MAX_CONFIG_LINE+1;
		f5 = f4+MAX_CONFIG_LINE+1;		
	}

	nflds = sscanf(line, "%s %s %s %s %s", f1, f2, f3, f4, f5);	/* overrun checked */
	if (nflds >= 3 && l_keyword_eq(job, f1, LM_RESERVED_SERVER))
	{				/* Found one */
		memset(sb, 0, sizeof (LM_SERVER));
		if (strlen(f2) <= MAX_HOSTNAME)
		{
#ifdef VMS
		    char *p;
/*
 *			Uppercase the server node name for VMS
 */
		 	l_uppercase(f2);
/*
 *			Remove any trailing ':'s from the server
 *			node name - VMS people like to specify
 *			node names as "name::"
 */
			p = f2 + strlen(f2) - 1;
			while (*p == ':') p--;
			*(p+1) = '\0';
#endif
			if(l_keyword_eq(job, f2, LM_RESERVED_THIS_HOST) 
				|| getenv("FLEXLM_ANYHOSTNAME"))	/* P6396 */	/* overrun checked */
			{
				strcpy(sb->name, lc_hostname(job, 0));
				sb->sflags |= L_SFLAG_THIS_HOST;
			}
			else	/*	Added for P6589	*/
			{
				char	buffer[MAX_CONFIG_LINE] = {'\0'};

				strcpy(buffer, LM_RESERVED_THIS_HOST);
				l_uppercase(buffer);

				if(l_keyword_eq(job, f2, buffer))
				{
					strcpy(sb->name, lc_hostname(job, 0));
					sb->sflags |= L_SFLAG_THIS_HOST;
					
				}
				else
				{
					strcpy(sb->name, f2);
				}
			}
		}
		else
		{
			LM_SET_ERROR(job, LM_BADFILE, 68, 0, f2, LM_ERRMASK_ALL);
			free( f1 );
			return 0;
		}
		if (*f3 == '"') /* host lists not supported here */
		{
			LM_SET_ERROR(job, LM_BADFILE, 68, 0, f3, LM_ERRMASK_ALL);
			free( f1 );
			return 0;
		}

/*
 *		P1628 -- don't return an error if l_get_id fails:
 *			 it could be vendor-defined...
 */

		if(sb->idptr)
		{
			lc_free_hostid(job, sb->idptr);
			sb->idptr = NULL;
		}
	
		l_get_id(job, &sb->idptr, f3);	 
		if (!sb->idptr)  /* P2778 */
		{
			LM_SET_ERROR(job, LM_BADFILE, 402, 0, f3, LM_ERRMASK_ALL);
			free( f1 );
			return 0;
		}
/* 
 *		port init must be init to -1, since 0 is valid on VMS
 */
		sb->port = -1;		/* Assume no port */
		sb->commtype = LM_NO_TRANSPORT_SPECIFIED;
		sb->filename = (char *) NULL;
		if (nflds > 3)
		{
			l_parse_commtype(job, f4, sb);
		}
		if (nflds > 4)
		{
			l_parse_commtype(job, f5, sb);
		}		
		sb->fd1 = (LM_SOCKET) -1;	/* Not connected yet */
		sb->fd2 = (LM_SOCKET) -1;	/* Not connected yet */
		sb->state = 0;	/* Not connected yet */
		sb->next = (LM_SERVER *)NULL;
		free(f1);
		return(1);
	} 
	else 
	{
		free(f1); 
		return(0);
	}
	    
	/* NOTREACHED*/
	free(f1); 
	return(0);
}

void
l_parse_commtype(job, spec, sb)
LM_HANDLE *job;		/* Current license job */
char *spec;
LM_SERVER *sb;
{
  int socketnum = -1;	 /* was UMR fixed 7.1e */
  char host[100];
  char buf[40];
  long netnum;
  char *cp;
#ifdef SUPPORT_IPX
  int got;
  char * nodenum ;
#endif /* SUPPORT_IPX */

	l_zcp(buf, spec, 39);
	l_uppercase(buf);
	*host = 0;

	if (!strncmp(buf, LM_FILE_PREFIX, LM_FILE_PREFIX_LEN)) 
/*
 *		Shared files
 */
	{
		spec += LM_FILE_PREFIX_LEN;
		sb->commtype = LM_FILE_COMM;
		sb->filename = (char *)l_malloc(job, strlen(spec) + 1);
		if (sb->filename) 	strcpy(sb->filename, spec);
		else 			sb->commtype =LM_NO_TRANSPORT_SPECIFIED;
		return; /* all done */
	}

	if (!strncmp(buf, LM_TCP_PREFIX, LM_TCP_PREFIX_LEN))
	{
		spec += LM_TCP_PREFIX_LEN;
		sb->commtype = LM_TCP;
	}
	if  (!strncmp(buf, LM_UDP_PREFIX, LM_UDP_PREFIX_LEN))
	{
		spec += LM_UDP_PREFIX_LEN;
		sb->commtype = LM_UDP;
	}
#ifdef SUPPORT_IPX
	else if (!strncmp(buf, LM_SPX_PREFIX, LM_SPX_PREFIX_LEN)) 
	{
		spec += LM_SPX_PREFIX_LEN;
		sb->commtype = LM_SPX;
	}
	else /* default is TCP */
	{
		sb->commtype = LM_TCP; 
	}
/*
 *	Generic scanf good for all commtypes (so far)
 */
#endif /* SUPPORT_IPX */
	if (*spec == '@')
	{
		if (!(cp = (char *)l_malloc(job, strlen(spec) + 3)))
			return;
		sprintf(cp, "-1%s", spec);
	}
	else cp = spec;
#ifdef SUPPORT_IPX
	got = 
#endif
	sscanf(cp, "%d@%[^#,]#%lx", &socketnum, host, &netnum);	/* overrun threat */
	if (*spec == '@') free(cp);

#ifdef SUPPORT_IPX
	if (sb->commtype == LM_SPX)
	{
		if ( got == 3)
		{
			*(long *)&(sb->spx_addr.sa_netnum) = netnum;
			nodenum = host;
			l_ether_str_to_num(nodenum, sb->spx_addr.sa_nodenum);
			*(long *)&sb->spx_addr.sa_netnum =
				    ntohl( *(long *) &sb->spx_addr.sa_netnum);
		}
		else
		{
/*
 *			Use the specified server name to get
 *			IPX address(node and network number)
 */


                  unsigned char SpxAddress[10];
                  int iRet;

#if !defined( NLM ) && defined( WINNT )
                        if (WinsockVersion()==2)
                                iRet=   l_sap2(sb->name, SpxAddress ) ;
                        else
#endif
#if defined( PC16 ) || defined ( OS2 )
				iRet=0;
#else
                                iRet=   l_sap(sb->name, SpxAddress ) ;
#endif

                        if (iRet)
                        {

                                memcpy(sb->spx_addr.sa_nodenum,
                                        &(SpxAddress[4]) , 6);
                                memcpy(sb->spx_addr.sa_netnum,
                                SpxAddress , 4);
                        }
			else
			{

                                memset(sb->spx_addr.sa_nodenum,
                                        0 , 6);
                                memset(sb->spx_addr.sa_netnum,
                        	        0 , 4);				

			}
#if 0
/*		  struct hostent *nw_svr;
		  SOCKADDR_IPX *addr_ipx;
				
			nw_svr = gethostbyname(sb->name);
			if (nw_svr)
			{
				addr_ipx= (SOCKADDR_IPX *)(nw_svr->h_addr_list);
				memcpy(sb->spx_addr.sa_nodenum,
						       addr_ipx->sa_nodenum, 6);
				memcpy(sb->spx_addr.sa_netnum,
						       addr_ipx->sa_netnum, 4);
			}
*/
#endif /*0 */
		}
		sb->spx_addr.sa_socket=ntohs((short)socketnum);
		sb->spx_addr.sa_family = AF_NS;
		return; /* all done */
	}
#endif /* SUPPORT_IPX */
/*
 *	Default - a TCP/IP port number for lmgrd
 */
	sb->port = socketnum;
	if (*host)
	{
		strncpy(sb->name, host, MAX_HOSTNAME);
		sb->name[MAX_HOSTNAME] = '\0';
	}
}

LM_SERVER *
l_reset_job_servers(job)
LM_HANDLE *job;
{
  int siz = sizeof(LM_SERVER ) * MAX_SERVERS;
  LM_SERVER *x;

	if (!job->servers) 
	{
		job->servers = (LM_SERVER *)l_malloc(job, siz);
		if (!job->servers) return job->servers;
	}
	else
	{
		for (x = job->servers; x; x = x->next)
		{
			lc_free_hostid(job, x->idptr);
		}
	}
	memset(job->servers, 0, siz);
	return job->servers;
}
