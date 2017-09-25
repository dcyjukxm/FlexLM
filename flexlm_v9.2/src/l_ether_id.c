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
 *	Module: $Id: l_ether_id.c,v 1.4 2003/01/13 22:41:51 kmaclean Exp $
 *
 *	Function: l_ether_id(job)
 *
 *	Description:	Returns the ethernet ID for this host (VAX).
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *
 *	Return:		(unsigned char *) - the 6 bytes of ethernet ID.
 *
 *	M. Christiano
 *	7/20/88
 *
 *	Last changed:  07/29/97
 *
 */

#include "lmachdep.h"
#include <stdio.h>
#include "lmclient.h"
#ifdef ULTRIX_ETHERNET_ADDR
#include "l_prot.h"
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#ifdef DEC_UNIX_3
static int l_getadr(unsigned char *);
#else
#include <sys/uio.h>
#include <sys/mbuf.h>
#include <sys/map.h>
#include <netinet/in.h>
static int l_getadr(struct ifdevea *);
#endif /* DEC_UNIX_3 */


/*
 *	VAX ethernet devices
 */
  static char *eth_boards[] = { "qe0", /* DEQNA */
				"de0", /* DEUNA */
				"se0", /* VAXstation 3100 */
				"ni0",
				"ln0",
				"xna0",	/* ??? on 5800 */
				"ne0",
				0 };
#endif


unsigned char *
l_ether_id(job)
LM_HANDLE *job;		/* Current license job */
{
#ifdef ULTRIX_ETHERNET_ADDR

  int s, rc = -1;
  int i;	
#ifdef DEC_UNIX_3
  unsigned char arr[6];
#endif /* DEC_UNIX_3 */
#ifdef USE_IFREQ
  struct ifreq ifreq;	/*- This is a hack.  Data doesn't line up right */
#else
  struct ifdevea ifreq;	
#endif
  static unsigned char ethid[6];

/* 
 *	Open a socket for ioctl
 */
	s = socket(AF_INET, SOCK_DGRAM, 0);

#ifdef USE_IFREQ
 	for (i=0; i<6; i++) ifreq.ifr_addr.sa_data[i] = 0;
#else
	for (i=0; i<6; i++) ifreq.default_pa[i] = 0;
#endif

/*
 *	If the vendor supplied a list of ethernet boards, try it first.
 */

	if (job->options->ethernet_boards)
	{
		for (i=0; job->options->ethernet_boards[i]; i++)
		{
			(void) strncpy(ifreq.ifr_name, 
					job->options->ethernet_boards[i],
							IFNAMSIZ);
			rc = ioctl(s, SIOCRPHYSADDR, (caddr_t) &ifreq);
			if (rc >= 0) break;
		}
	}

#ifndef DEC_UNIX_3
/*
 *	If we don't have one already, keep trying to read an address 
 *	until we get it - if none, we will return 0.
 */
	if (rc < 0)
	{
		for (i=0; eth_boards[i]; i++)
		{
			(void) strncpy(ifreq.ifr_name, eth_boards[i], IFNAMSIZ);
			rc = ioctl(s, SIOCRPHYSADDR, (caddr_t) &ifreq);
			if (rc >= 0) break;
		}
	}
#endif /* DEC_UNIX_3 */

/*
 *	If we STILL didn't get one, try the generic code
 */
	if (rc < 0)
	{
#ifdef DEC_UNIX_3
		l_getadr(arr);
#else
		l_getadr(&ifreq);
#endif /* DEC_UNIX_3 */
	}

#ifdef USE_IFREQ	/*- See what I mean ? */
	ethid[0] = (unsigned char) (ifreq.ifr_addr.sa_family & 0xff);
	ethid[1] = (unsigned char) ((ifreq.ifr_addr.sa_family >> 8) & 0xff);
	for (i=0; i<4; i++) 
		ethid[i+2] = (unsigned char) ifreq.ifr_addr.sa_data[i];
#else
	for (i=0; i<6; i++) 
	{
#ifdef DEC_UNIX_3
		ethid[i] = arr[i];
#else
		ethid[i] = (unsigned char) ifreq.default_pa[i];
#endif /* DEC_UNIX_3 */
	}
#endif /* USE_IFREQ */
	(void) close(s);

	return(ethid);

#else	/* ULTRIX_ETHERNET_ADDR */

	return((unsigned char *)NULL);

#endif	/* ULTRIX_ETHERNET_ADDR */
}

#ifdef ULTRIX_ETHERNET_ADDR
#define IFREQCNT 64	/* need enough room for toal number of possible
* interfaces, including aliases for LAT and DECnet
*/

#ifdef DEC_UNIX_3

static
int 
l_getadr(unsigned char hardware[])

    {
    int count = 1;
    int             s;
    struct ifreq    ifreqs[IFREQCNT];
    struct ifconf   ifc;
    struct ifdevea  ifrp;
    int    index    = 0;
    int    ndx, ifr;

    memset(&ifrp, 0, sizeof(struct ifdevea));
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
        perror("Error with socket for ethernet hostid");
        return(0);
        }

        ifc.ifc_req = ifreqs;
        ifc.ifc_len = sizeof(ifreqs);
        if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
            {
            perror("ioctl: SIOCGIFCONF");
            return(0);
            }

        for (ifr = 0; ((ifr < IFREQCNT)&&(strlen(ifreqs[ifr].ifr_name) != 0)); 
ifr++)
            {
                if ((ioctl(s, SIOCGIFFLAGS, &ifreqs[ifr]) < 0)&&(index == 0))
                    {
                    perror("ioctl: SIOCGIFFLAGS");
                    return(0);
                    }

                if ((ifreqs[ifr].ifr_flags & (IFF_POINTOPOINT))
                        == (IFF_POINTOPOINT))
                    {
                    continue;   /* skip point to point */
                    }

                if ((ifreqs[ifr].ifr_flags & (IFF_MULTICAST|IFF_BROADCAST))
                        != (IFF_MULTICAST|IFF_BROADCAST))
                    {
                    continue;   /* skip CI, etc. */
                    }

                strcpy(ifrp.ifr_name, ifreqs[ifr].ifr_name);
                if (ioctl(s, SIOCRPHYSADDR, &ifrp) >= 0)
                 {
                  if (index==0)
                    {
                      if (index == count) return -index;
                      hardware[0+index*6] = ifrp.default_pa[0];
                      hardware[1+index*6] = ifrp.default_pa[1];
                      hardware[2+index*6] = ifrp.default_pa[2];
                      hardware[3+index*6] = ifrp.default_pa[3];
                      hardware[4+index*6] = ifrp.default_pa[4];
                      hardware[5+index*6] = ifrp.default_pa[5];
                      index+=1;
                    }
                  else
                    {
                      for (ndx=0; ; ndx++)
                        {
                          if (ndx == count) return -index;
                          if (ndx == index)
                            {
                              hardware[0+index*6] = ifrp.default_pa[0];
                              hardware[1+index*6] = ifrp.default_pa[1];
                              hardware[2+index*6] = ifrp.default_pa[2];
                              hardware[3+index*6] = ifrp.default_pa[3];
                              hardware[4+index*6] = ifrp.default_pa[4];
                              hardware[5+index*6] = ifrp.default_pa[5];
                              index+=1;
                              break;
                            }
                          if ((ifrp.default_pa[5]==hardware[ndx*6+5])&&
                              (ifrp.default_pa[4]==hardware[ndx*6+4])&&
                              (ifrp.default_pa[3]==hardware[ndx*6+3])&&
                              (ifrp.default_pa[2]==hardware[ndx*6+2])&&
                              (ifrp.default_pa[1]==hardware[ndx*6+1])&&
                              (ifrp.default_pa[0]==hardware[ndx*6+0]))
                            {
                              break;
                            }
                        }
                    }
                 }

            }

    close(s);
    return index;
    }
#else /* older, pre-unix3 code */

/*	l_getadr - this module from DEC.
 *		-- get primary ethernet interface address
 *		read list of net devices via ioctl to ifreqs.  Pass over
 *		loopback device and any serial line interfaces.  Use first
 *		ether device found.  This could cause a problem if there are
 *		multiple ether controllers on the system as it will find the
 *		first device configured.  No panic since most have only one
 *		device.  This function does not require setuid(2) privileges.
 */


static int
l_getadr(ifrp)
struct ifdevea *ifrp;
{
  int s;
  struct ifreq *ifr;
  struct ifreq  ifreqs[IFREQCNT];
  struct ifreq  tmp_ifr;
  struct ifconf ifc;
  char *maybe = NULL;	/* a device that might be it */

	memset (ifrp, 0, sizeof(struct ifdevea));
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return (-1);
	ifc.ifc_req = ifreqs;
	ifc.ifc_len = sizeof(ifreqs);
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) goto error_ret;
	for (ifr = ifreqs; ifr < &ifreqs[IFREQCNT]; ifr++) 
	{
		if (strlen(ifr->ifr_name) == 0) 
		{
			if (maybe == NULL) goto error_ret;
			else		   break;  /* check out maybe */
		} 
		else 
		{
			(void) strcpy (tmp_ifr.ifr_name, ifr->ifr_name);
			if (ioctl(s, SIOCGIFFLAGS, &tmp_ifr) < 0) 
				goto error_ret;
			if ((tmp_ifr.ifr_flags & (IFF_POINTOPOINT))
				  == (IFF_POINTOPOINT)) 
			{
				continue;	/* skip point to point */
			}
			if ((tmp_ifr.ifr_flags & (IFF_DYNPROTO|IFF_BROADCAST))
				    != (IFF_DYNPROTO|IFF_BROADCAST)) 
				continue;	/* skipp CI, etc. */
			if ((tmp_ifr.ifr_flags & (IFF_UP|IFF_RUNNING))
				    != (IFF_UP|IFF_RUNNING)) 
			{
/* 
 *				save this name, it might be the
 *				right one but just isn't up now
 */
				if (maybe == NULL) maybe = ifr->ifr_name;
				continue;	/* not up and running */
			}
			(void) strcpy(ifrp->ifr_name, tmp_ifr.ifr_name);
			maybe = NULL;  /* found real one, clear maybe */
			break;
		}
	}
	if (maybe != NULL) (void) strcpy (ifrp->ifr_name, maybe);
	if (ioctl(s, SIOCRPHYSADDR, ifrp) < 0) goto error_ret;

	close (s);
	return (0);

error_ret:
	close (s);
	return (-1);
}
#endif /* DEC_UNIX_3 */
#endif	/* ULTRIX_ETHERNET_ADDR */
