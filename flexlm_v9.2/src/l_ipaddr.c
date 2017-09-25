/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_ipaddr.c,v 1.12 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function: 	l_ipaddr()
 *
 *	Return:		ip address in 4-byte int if ip addr, else 0
 *
 *	D. Birns
 *	9/6/95
 *
 *	Last changed:  10/23/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_socket.h"

/*
 * 	l_ipaddr:  if name is an ipaddr string, return converted to
 *	32-bit unsigned int.
 */
long API_ENTRY
l_ipaddr(name)
char *name;
{

  unsigned char a[4];
  int i, j, k, l;
  long ret;

        if (sscanf(name, "%d.%d.%d.%d", &i, &j, &k, &l)  != 4)	/* overrun checked */ /* DoS threat */
                return (long)0;
	a[0] = (unsigned char)i;
	a[1] = (unsigned char)j;
	a[2] = (unsigned char)k;
	a[3] = (unsigned char)l;
	memset(&ret, 0, sizeof(ret));
	memcpy(&ret, a, 4);
	return (ret);
}
/*
 *	arg is node.
 *	returns 32-bit unsigned int.
 *	arg can be hostname or ipaddress in string format.
 *	returns 32-bit ip-address, or 0 if error
 *	sock->h_addr is filled in with raw data in network byte order if
 *	sock is non-zero
 */

unsigned int
l_get_ipaddr(char *node, char *addr, void *sockp, int bFLEXLMANYHOSTNAME)
{
  struct hostent *host;
  unsigned int ipaddr = 0;
  int j;
  struct sockaddr_in *sock = (struct sockaddr_in *)sockp;

	if (ipaddr = l_ipaddr(node))
	{
		memcpy((char *)&(sock->sin_addr), &ipaddr, 4);
		sock->sin_family = AF_INET;
		return ipaddr;
	}
	else
	{
#ifdef SUNOS5 /* unfortunately, gethostbyname_r is not portable at all */
	  struct hostent he;
	  char buf[1000];
	  int err;
		host = gethostbyname_r(node, &he,
					buf, 1000, &err);
#else
		host = gethostbyname(node);
#endif
		if(host == NULL && bFLEXLMANYHOSTNAME)
		{
			host = gethostbyname("localhost");
		}
		if (host)
		{
			if (sock)
			{
				memcpy((char *)&(sock->sin_addr),
				host->h_addr, host->h_length);
				sock->sin_family = AF_INET;
			}
			if (addr) memcpy(addr, host->h_addr, host->h_length);
			for (j = 0;j<host->h_length;j++)
			{
				ipaddr += (unsigned char)
					host->h_addr[j] << ((3 - j) * 8);
			}
		}
	}
	return ipaddr;
}
