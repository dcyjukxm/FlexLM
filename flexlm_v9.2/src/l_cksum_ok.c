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
 *	Module: $Id: l_cksum_ok.c,v 1.6 2003/01/13 22:41:50 kmaclean Exp $
 *
 *	Function: l_cksum_ok(msg, comm_revision)
 *
 *	Description: 	Verifies the checksum in "msg"
 *
 *	Parameters:	(char *) msg - The message
 *			(int) comm_revision - The communications revision
 *
 *	Return:		0 - Checksum BAD
 *			1 - Checksum OK
 *
 *	M. Christiano
 *	10/16/88
 *
 *	Last changed:  11/13/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"


unsigned char API_ENTRY
l_get_cksum(msg, comm_revision, transport)
char *msg;	/* The message */
int comm_revision;
int transport; /*LM_TCP, LM_UDP, or LM_SPX*/
{
  int i;
  unsigned char cksum = (unsigned char) msg[MSG_CMD];
  int size;

	if (comm_revision < 0 || comm_revision > COMM_NUMREV)
		comm_revision = COMM_NUMREV;	/* Force it to avoid problems */
	size = l_msg_size(comm_revision);

        for (i = MSG_DATA; i < size; i++) {
                cksum += (unsigned char) msg[i];
        }
	if (transport == LM_UDP) cksum &= 0x0f; /*Half a byte*/
#ifndef RELEASE_VERSION
  	if (l_real_getenv("L_MSG_CKSUM"))
		fprintf(stdout, "cksum = %d size = %d %s\n", cksum, size, 
			transport == LM_UDP ? "UDP" : "TCP");
	if (l_real_getenv("L_DUMP_MSGS"))
		fprintf(stdout, "in  %s\n", l_debug_datastr((unsigned char *)msg, size));
#endif

	return(cksum);
}

API_ENTRY
l_cksum_ok(msg, comm_revision, transport)
char *msg;	/* The message */
int comm_revision;
int transport; /*LM_TCP, LM_UDP, or LM_SPX */
{
  unsigned char cksum;

	cksum = l_get_cksum(msg, comm_revision, transport);
	switch (transport)
	{
	case LM_TCP:
#ifdef SUPPORT_IPX
	case LM_SPX:
#endif
#ifdef FIFO
	case LM_LOCAL:
#endif
	    return(((unsigned char) msg[MSG_CHECKSUM]) == cksum);
	case LM_UDP:
	    return(((unsigned char) msg[MSG_CHECKSUM] & 0xf) == cksum);
	}
	return (0); /* Shouldn't be able to get here */
}
