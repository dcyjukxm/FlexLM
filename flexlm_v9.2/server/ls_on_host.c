/******************************************************************************

	    COPYRIGHT (c) 1992, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_on_host.c,v 1.10.4.2 2003/06/26 21:58:29 sluu Exp $
 *
 *	Function:	ls_on_host(which)
 *
 *	Description: 	Returns whether we are running on the specified host
 *
 *	Parameters:	(char *) which - Host name to check
 *
 *	Return:		(int) 0 => not on this host <>0 => on this host
 *
 *	M. Christiano
 *	8/15/92
 *
 *	Last changed:  11/13/98
 *
 */
#ifndef lint
static char *sccsid = "ls_on_host.c:1.11";
#endif

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "flex_utils.h"
#include <sys/types.h>
#ifdef USE_WINSOCK	
#include <pcsock.h>
#else
#include <netdb.h>
#endif

#ifdef SUPPORT_IPX
static get_my_ipx_nodenum( char *);
#endif /* Support IPX */


int
ls_on_host(char * which)
{
	char **			hosts = NULL;
	char **			tmpAliases = NULL;
	int				iNumAliases = 0;
	int				i = 0;
	long			ipaddr = 0;
	struct hostent *me, *alias, *server_line = 0;
	char			hostname[MAX_PATH+1] = {'\0'};
	char			uhostname[MAX_PATH+1] = {'\0'};
	char			uwhich[MAX_PATH+1] = {'\0'};
	char			thishost[MAX_PATH+1] = {'\0'};
	int				size = 0;
	int				noalias = (int)getenv("NO_FLEXLM_HOSTNAME_ALIAS");
	char			server_h_addr[4];


	l_zcp(thishost, which, MAX_PATH);
	l_zcp(uwhich, thishost, MAX_PATH);
	l_uppercase(uwhich);

#if FIFO
	if (lm_job->options->commtype == LM_LOCAL)
		return 1;
#endif
	hostname[0] = '\0';
	(void) gethostname(hostname, MAX_PATH);
	l_zcp(uhostname, hostname, MAX_PATH);
	l_uppercase(uhostname);
#ifndef RELEASE_VERSION
	if(l_real_getenv("OUTPUT_HOSTNAMES"))
	{
		char *	pHost = uhostname;
		int		temp = 0;
		
		fprintf(stderr, "gethostname: ");
		while(*pHost != '\0')
		{
			temp = (int)*pHost;
			temp &= 0xff;
			fprintf(stderr, "%#2x ", temp);
			pHost++;
		}
		pHost = uwhich;
		fprintf(stderr, "\nlicense file: ");
		while(*pHost != '\0')
		{
			temp = (int)*pHost;
			temp &= 0xff;
			fprintf(stderr, "%#2x ", temp);
			pHost++;
		}
		fprintf(stderr, "\n");
		fflush(stderr);
	}
#endif
	if (strcmp(uhostname, uwhich) == 0)
		return(1);
	
/*
 *	If this isn't the "offical" name of the host, try the aliases
 *	as well as gethostbyname's h_name, which is sometimes not the same
 *	as what gethostname returns
 */
#ifndef RELEASE_VERSION
	if (l_real_getenv("DEBUG_HOSTNAME"))
		return 0;
#endif
	if (getenv("FLEXLM_HOSTNAME_OK")) /*P6526*/
	{
		LOG(("FLEXLM_HOSTNAME_OK set.  This will only work with single servers, not 3-server license files.\n"));
		return 1;
	}
	if (!(ipaddr = l_ipaddr(thishost)))
		ipaddr = l_ipaddr(uwhich);

	if (!ipaddr && !(server_line = gethostbyname(thishost)))
		server_line = gethostbyname(uwhich);
	if (server_line)
		memcpy(server_h_addr, server_line->h_addr, 4) ;
	else
		memset(server_h_addr, 0, 4);
	if (!noalias)
	{
		if (!(me = gethostbyname(hostname)))
			me = gethostbyname(uhostname);
		if (me) 
		{
			if (ipaddr && !memcmp(me->h_addr, &ipaddr, 4)) 
				return(1);
			if (*server_h_addr && 
				!memcmp(me->h_addr, server_h_addr, 4)) 
				return(1);
			if (strcmp(me->h_name, thishost) == 0)
				 return(1);
			hosts = me->h_aliases;
			/*
			 *	Count number of aliases
			 */
			while(hosts && *hosts)
			{
				iNumAliases++;
				hosts++;
			}
			/*
			 *	Now allocate buffers for entries
			 */
			hosts = malloc(sizeof(char *) * iNumAliases);
			if(hosts)
			{
				tmpAliases = me->h_aliases;
				for(i = 0; i < iNumAliases; i++)
				{
					hosts[i] = malloc(strlen(*tmpAliases) + 1);
					if(hosts[i])
					{
						strcpy(hosts[i], *tmpAliases);
						tmpAliases++;
					}
					else
					{
						/*
						 *	Free what's been allocated and bail.
						 */
						iNumAliases = i;
						for(i = 0; i < iNumAliases; i++)
						{
							if(hosts[i])
							{
								free(hosts[i]);
								hosts[i] = NULL;
							}
							free(hosts);
							hosts = NULL;
							return 0;
						}
						break;
					}
				}
			
			}
			else
				return 0;
			i = 0;
			while (i < iNumAliases)
			{
				if (strcmp(hosts[i], thishost) == 0)
				{
					
					for(i = 0; i < iNumAliases; i++)
					{
						if(hosts[i])
						{
							free(hosts[i]);
							hosts[i] = NULL;
						}
					}
					free(hosts);
					hosts = NULL;
					return(1);
				}
				alias = gethostbyname(hosts[i]);
				if (alias && *server_h_addr && 
					!memcmp(server_h_addr, alias->h_addr, 4))
				{

					for(i = 0; i < iNumAliases; i++)
					{
						if(hosts[i])
						{
							free(hosts[i]);
							hosts[i] = NULL;
						}
					}
					free(hosts);
					hosts = NULL;
					return(1);
				}
				i++;
			}
			if(iNumAliases)
			{
				for(i = 0; i < iNumAliases; i++)
				{
					if(hosts[i])
					{
						free(hosts[i]);
						hosts[i] = NULL;
					}
				}
				free(hosts);
				hosts = NULL;
			}
		}
	}
#ifdef SUPPORT_IPX
    /*
     *      Check the name with our IPX node name.
     */
    if ( get_my_ipx_nodenum(hostname) )
		return !stricmp(hostname,thishost);
#endif /* SUPPORT_IPX */
	
	return(0);
}


#ifdef SUPPORT_IPX
static
int
get_my_ipx_nodenum(char * buff)
{
	struct sockaddr_ipx ipx_addr;
	LM_SOCKET ipx_s;
	int addrlen;

	*buff = 0;

	if ((ipx_s=socket(AF_NS, SOCK_STREAM, NSPROTO_SPX)) == LM_BAD_SOCKET )
		return 0;
        

	memset(&ipx_addr, 0, sizeof(ipx_addr));
	ipx_addr.sa_family = AF_NS;

	if (bind(ipx_s, (const struct sockaddr *) &ipx_addr, sizeof(ipx_addr)))
	{
		network_close(ipx_s);
		return 0;
	}

	addrlen = sizeof(ipx_addr);
	if (getsockname(ipx_s, (struct sockaddr *) &ipx_addr, &addrlen))
	{
		network_close(ipx_s);
		return 0;
	}

	sprintf( buff, "%02x%02x%02x%02x%02x%02x", 
			 (unsigned char)ipx_addr.sa_nodenum[0], (unsigned char)ipx_addr.sa_nodenum[1],
			 (unsigned char)ipx_addr.sa_nodenum[2], (unsigned char)ipx_addr.sa_nodenum[3],
			 (unsigned char)ipx_addr.sa_nodenum[4], (unsigned char)ipx_addr.sa_nodenum[5] );

	network_close(ipx_s);
	return 1;
}
#endif /* SUPPORT_IPX */
