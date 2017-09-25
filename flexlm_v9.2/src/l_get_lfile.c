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
 *	Module: $Id: l_get_lfile.c,v 1.9 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	l_get_lfile(job, file, type)
 *
 *	Description: 	Reads the license file data from lmgrd/license finder
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) file - "file" specification (port@host or
 *					port@host,port2@host2,port3@host3)
 *			(char *) type - Type of data (from license finder)
 *
 *	Return:		(char *) license file data (in malloced memory)
 *
 *	M. Christiano
 *	6/15/92
 *
 *	Last changed:  7/16/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "lm_attr.h"
#include <sys/types.h>
#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#include <netinet/in.h>
#endif /* USE_WINSOCK */
#include <string.h>

#if !defined(htons) && !defined(PC) && !defined(OSF) && !defined(ANSI)
extern u_short htons();
#endif

char *
l_get_lfile(job, file, type, endpoint)
LM_HANDLE *job;		/* Current license job */
char *file;
char *type;
COMM_ENDPOINT_PTR endpoint;
{
  char *ret = (char *) NULL;
  char ports[MAX_CONFIG_LINE], host[MAX_CONFIG_LINE];
  int nflds;
#ifdef SUPPORT_IPX
  char * netnum ;
#endif /* SUPPORT_IPX */

	while (1)
	{
		ports[0] = host[0] = '\0';
		if (*file == '@')
		{
			strcpy(ports, "-1");
			strcpy(host, &file[1]);
			nflds = 2;
		}
		else
			nflds = sscanf(file, "%[^@]@%s", ports, host);	/* overrun threat */

		if (nflds >= 2)
		{
		  int port = atoi(ports);
		  char *remainder;
		  int s;

		    if (port)
		    {
			if ((remainder = strchr(host, ',')) != (char *) NULL)
			{
				*remainder = '\0';
				remainder++;
				file = strchr(file, ',');
				if (file) file++;
			}
			else
				file = "";

#ifdef SUPPORT_IPX
			if ( netnum=strchr(host, '#') )
			{
				/*
				 *	This host specifies an IPX address
				 */
				*netnum = 0;
				endpoint->transport = LM_SPX;
				endpoint->transport_addr.spx_addr.sa_family = AF_NS;
				l_ether_str_to_num(host, 
				  endpoint->transport_addr.spx_addr.sa_nodenum);
				sscanf(netnum+1, "%lx", 	/* overrun don't care */
				   endpoint->transport_addr.spx_addr.sa_netnum);
				*(long *)&endpoint->transport_addr.spx_addr.
								   sa_netnum =
				  ntohl( *(long *)
				  &endpoint->transport_addr.spx_addr.sa_netnum);
				endpoint->transport_addr.spx_addr.sa_socket =
						htons((unsigned short) port);
				s = l_connect_endpoint(job, endpoint, host);
			}
			else
#endif /* SUPPORT_IPX */
			{
				endpoint->transport = LM_TCP;			
				endpoint->transport_addr.port =
						htons((unsigned short) port);
				s = l_connect_endpoint(job, endpoint, host);
			}
			if (s >= 0)
			{
/* 		
 *			Now we're connected 
 */
			  char msg[LM_MSG_LEN+1];

/*
 *				Queue the request 
 */
				(void) memset(msg, '\0', LM_MSG_LEN);
#ifdef SUPPORT_IPX
				if ( endpoint->transport == LM_SPX )
					l_conn_msg(job, job->vendor, msg, endpoint->transport, 1);
				else
#endif
					l_conn_msg(job, job->vendor, msg, LM_TCP, 1);


				(void) strncpy(&msg[MSG_LF_FINDER_TYPE], type,
							MAX_FINDER_TYPE);
				l_sndmsg(job, LM_SEND_LF, &msg[MSG_DATA]);
/*
 *				Read the messages
 */
				if (ret = l_rcvmsg_str(job))
					break; /* got one -- all done */
			}
			if (*file == '\0' && s >= 0) 
			{
				LM_SET_ERROR(job, LM_SERVNOREADLIC, 48, -1, host, LM_ERRMASK_ALL);
				break;	/* No more to try */
			}
		    }
		    else
			break; /* no valid port */
		}
		else
			break;	/* No valid port@node */
	}
	{
	  int flag;
		flag = (job->flags |= LM_FLAG_CONNECT_NO_HARVEST);
		job->flags &= ~flag; 
		lc_disconn(job, 1); /* force a close */
		job->flags |= flag;
	}

	return(ret);
}
