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
 *	Module: $Id: l_sernum.c,v 1.6 2003/04/18 23:48:05 sluu Exp $
 *
 *	Functions: l_read_sernum(msg, sernum)
 *	           l_write_sernum(msg, sernum)
 *
 *	Description: Checks for duplicate serial number, and increments if
 *		not duplicate
 *
 *	Parameters:	(char *) msg - The message
 *			(int *) serial number
 *
 *	Return:		1 if OK, 0 if duplicate 
 *
 *	D. Birns
 *	5/93
 *
 *	Last changed:  11/13/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"

#ifdef getenv
#undef getenv
#endif

API_ENTRY
l_read_sernum(msg, sernum)
char *msg;
int *sernum;
{
	int msernum = ((msg[MSG_CHECKSUM] >>4) & 0x0f);
	switch (msg[MSG_CMD])
	{
		case LM_HEARTBEAT:
		case LM_HEARTBEAT_RESP:
			return 1;
		default:
			break;
	}


	(*sernum)++;
#ifndef RELEASE_VERSION
	if (l_real_getenv("L_MSG_CKSUM"))	/* overrun checked */
	    (void) printf("read_sernum: sernum = %d expected = %d\n", msernum, *sernum);
#endif
	if (msernum == (*sernum & 0xf) )
	{
		return 1;
	} 
#ifndef RELEASE_VERSION
	if (l_real_getenv("TEST_UDP_SERNUM") && l_real_getenv("DEBUG_UDP"))	/* overrun checked */
		(void) printf("Got message (%c/%d)twice -- ignoring\n", *msg, *msg);
#endif
	    
	(*sernum)--;
	return 0;
}
	
API_ENTRY
l_write_sernum(msg, sernum)
char *msg;
int *sernum;
{
	switch (msg[MSG_CMD])
	{
		case LM_HEARTBEAT:
		case LM_HEARTBEAT_RESP:
			return 1;
		case LM_HELLO:
			*sernum = 1;
			break;
		default:
			(*sernum)++;
			break;
	}

	msg[MSG_CHECKSUM] |= ((((*sernum << 4) & 0xf0)) & 0xff);
#ifndef RELEASE_VERSION
	if (l_real_getenv("L_MSG_CKSUM"))	/* overrun checked */
	{
		(void) printf("encoding sernum = %d msgcmd = %c ", *sernum, *msg);
                (void) printf("%s ", l_debug_datastr((unsigned char *)msg, 147));
                (void) puts("");
	}
#endif
	return 1;
}
	

