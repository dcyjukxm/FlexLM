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
 *	Module: $Id: ls_udp_read.c,v 1.10 2003/05/05 16:01:36 sluu Exp $
 *
 *	Function: ls_udp_read(buf, size, ca)
 *
 *	Description: Read udp message
 *
 *	Parameters:
 *			(char *)buf  -- 	return this filled in
 *			(int) size  -- 		size of buf
 *			CLIENT_ADDR *ca -- 	fill in sockaddr part
 *
 *	Return values:	0 -- success
 *			otherwise errno;
 *	Daniel Birns
 *	4/93
 *
 *	Last changed:  12/26/96
 *
 */


#include "lmachdep.h"
#ifdef SUPPORT_UDP
#include "lmclient.h"
#include "l_prot.h"
#ifndef PC
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#ifndef apollo
#include <netinet/in.h>
#endif /* apollo */
#include <netdb.h>
#endif
#include <errno.h>

#include "lsserver.h"
#include "ls_sprot.h"
#ifndef RELEASE_VERSION
static int debug_udp = -1;
#endif
int ls_test_udp_sernum = -1;

int
ls_udp_read(buf, size, ca)
char *buf;
int size; /*size of buf*/
CLIENT_ADDR *ca; /*fill in sockaddr stuff*/
{
    int ret_cnt;
    struct sockaddr_in sin;
    unsigned int len = sizeof(sin);

    LM_SET_NET_ERRNO(0);
#ifndef RELEASE_VERSION
    if (debug_udp == -1)
	debug_udp = (int)l_real_getenv("DEBUG_UDP");
    if (debug_udp) {
	(void) printf("recvfrom:");
	(void) fflush(stdout);
    }

#endif
    if ((ret_cnt= recvfrom(ls_udp_s, buf, size, 0, (struct sockaddr *)&sin,	/* overrun checked */
								   (int *)&len)) < 0)
    {
#ifndef RELEASE_VERSION
	if (debug_udp)
	{
	    struct sockaddr_in udp_sin;
	    len = sizeof(udp_sin);
	    (void) getsockname(ls_udp_s, (struct sockaddr *)&udp_sin, &len);
	    (void) printf("recvfrom failed with errno = %d,", net_errno);
	    (void) printf("ls_udp_s = %d %d@0x%x\n",
		ls_udp_s,
		ntohs(udp_sin.sin_port),
		udp_sin.sin_addr);
	}
#endif
	return net_errno;
    }

    /*
     *  We have to force them to zero here because the whole 16 bytes of
     *  ca->addr.sin gets used as hash key in ls_symtab.c.
     *	Some platforms sometimes put garbage here.
     */
    memset(sin.sin_zero, 0, sizeof(sin.sin_zero));
    memcpy(&ca->addr.sin, &sin, sizeof(sin));
#ifndef RELEASE_VERSION
    if (debug_udp)
	(void) printf("UDP Got %c from %d@0x%x (time %d)\n", *buf,
		ntohs(ca->addr.sin.sin_port),
		ca->addr.sin.sin_addr, time(0));
#endif
    return ret_cnt;
}

int
ls_udp_send(s, sock, buf, size)
LM_SOCKET s;
struct sockaddr_in *sock;
char *buf;
int size; /*size of buf*/
{
  int ret_cnt = 0;
  unsigned int len;
  int write_it = 1;
  int test_this_time = 0;

	LM_SET_NET_ERRNO(0);
	len = sizeof(struct sockaddr_in);
#ifndef RELEASE_VERSION
	if (debug_udp == -1)
		debug_udp = (int)l_real_getenv("DEBUG_UDP");
	if (debug_udp) {
	    (void) printf("sendto: msg = %c %d@%x\n", *buf,
					ntohs(sock->sin_port),
					sock->sin_addr.s_addr);
	}

#endif
	if (ls_test_udp_sernum == -1)
	{
		if (getenv("TEST_UDP_SERNUM"))
		   sscanf(getenv("TEST_UDP_SERNUM"), "%d", &ls_test_udp_sernum);	/* overrun checked */
		else
		   ls_test_udp_sernum = 0;

		if (ls_test_udp_sernum < 0)
		{
			LOG(("TEST_UDP_SERNUM"));
			ls_test_udp_sernum *= -1;
		}
		else if (ls_test_udp_sernum >0)
		{
			LOG(("random TEST_UDP_SERNUM\n"));
			srand(time(0));
		}
	}
	if (ls_test_udp_sernum && ((rand() % ls_test_udp_sernum) == 0))
	{
	  static int tested_last_time = 0;

		if (tested_last_time)
			tested_last_time = 0;
		else
		{
			test_this_time = tested_last_time = 1;
			if ((rand()%2) == 0)
				write_it = 0;
		}
	}
	if (test_this_time && !write_it)
	{
#ifndef RELEASE_VERSION
		if (debug_udp)
			(void) printf("not sending (%c/%d)\n", *buf, *buf);
#endif
		write_it = 1;
	}
	else
	{
		LM_SET_NET_ERRNO(0);
		if ((ret_cnt= sendto(s, buf, size, 0, (struct sockaddr *)sock,
								len)) != size)
		{
#ifndef RELEASE_VERSION
			if (debug_udp)
			{
			  struct sockaddr_in udp_sin;

				len = sizeof(udp_sin);
				(void) getsockname(ls_udp_s,
					    (struct sockaddr *)&udp_sin, &len);
				(void) printf("sendto failed with errno = %d,",
								 net_errno);
				(void) printf(
				"ls_udp_s = %d %d@0x%x bytes sent = %d\n",
					ls_udp_s, ntohs(udp_sin.sin_port),
						udp_sin.sin_addr, ret_cnt);
			}
#endif
			return errno;
		}
		if (test_this_time && write_it)
		{
			sendto(s,buf,size,0,(struct sockaddr *)sock,len);
#ifndef RELEASE_VERSION
			if (debug_udp)
				LOG((lmtext("sending twice (msg=%c/%d)...\n"),
								*buf, *buf));
#endif
		}
	}
	return ret_cnt;
}

void
ls_resend_last_resp(client)
CLIENT_DATA *client;
{
	(void) ls_udp_send(ls_udp_s, &client->addr.addr.sin,
					client->udp_last_resp, LM_MSG_LEN);
}

#endif	/*  SUPPORT_UDP */
