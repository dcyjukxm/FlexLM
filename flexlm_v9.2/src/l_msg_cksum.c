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
 *	Module: $Id: l_msg_cksum.c,v 1.8 2003/04/16 18:25:16 brolland Exp $
 *
 *	Function: l_msg_cksum(msg, comm_revision)
 *
 *	Description: Inserts the checksum into the message.
 *
 *	Parameters:	(char *) msg - The message
 *			(int) comm_revision - Communications revision
 *
 *	Return:		Message checksum filled in.
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
#if defined (UNIX) && defined(ANSI)
#include <unistd.h>
#include <sys/types.h>
#endif /* UNIX & ANSI */

void API_ENTRY
l_msg_cksum(msg, comm_revision, transport)
char *msg;	/* The message */
int comm_revision;	/* Communication revision */
int transport; /*LM_UDP, LM_TCP, or LM_SPX*/
{
  int i;
  unsigned char cksum = (unsigned char) msg[MSG_CMD];
  int size;
  
	if (comm_revision < 0 || comm_revision > COMM_NUMREV)
		comm_revision = COMM_NUMREV;	/* Force it to avoid problem */

	size = l_msg_size(comm_revision);

#ifndef RELEASE_VERSION
#endif
        for (i = MSG_DATA; i < size; i++) 
	{
                cksum += (unsigned char) msg[i];
        }
	if (transport == LM_UDP) cksum &= 0x0f; /*half-a-byte*/
#ifndef RELEASE_VERSION
  	if (l_real_getenv("L_DUMP_MSGS"))
		(void) fprintf(stdout, "out %s\n", l_debug_datastr((unsigned char *)msg, size));
	if (l_real_getenv("L_MSG_CKSUM"))
		(void) fprintf(stdout, "cksum = %d size = %d\n", cksum, size);
#endif



	msg[MSG_CHECKSUM] = (char) cksum;
}
#ifndef RELEASE_VERSION
#include "lcommdbg.h"
char *
l_debug_datastr(data, len)
unsigned char *data;
int len;
{
#ifdef getenv
#undef getenv
#endif
    static char buf[1024]; 
    static char buf2[512]; 
    int i;
    static pid = -1;
    static char type[30];
    (void) sprintf(type, "UNKNOWN(%d)", data[MSG_CMD]);
    for (i=0;i<sizeof(msgs)/sizeof(struct _msgs);i++)
    {
        if (data[MSG_CMD] == msgs[i].code && msgs[i].transl)
        {
            (void)strcpy(type, msgs[i].transl);
            break;
        }
    }
    if (pid == -1)
#ifndef PC
        pid = getpid();
#else
#ifdef PC32
	pid = abs(getpid())%1000;
#else
	;
#endif /* PC 32 */
#endif /* PC */

    (void) sprintf(buf, "pid=%d cmd=%s: ",
            pid,
            type);
    if (getenv("L_DUMP_MSGS"))	/* overrun checked */
    {
      char *cp = buf2;
	for (i=0;i<len;i++)
	{
		if (!data[i])
		{
			sprintf(cp++, ".");
			while (!data[++i] && i < len) ;
			if (i == len) break;
		}
			
		if ((isprint(data[i])) && (data[i] != '\\') && (data[i] < 0x7f))
		{	
			if (data[i] == '\\')
				sprintf(cp++, "%c", '\\');
			sprintf(cp++, "%c", data[i]);
		}
		else
		{
			sprintf(cp, "\\%02x", data[i]);
			cp += 3;
		}
	}
    }
    strcat(buf, buf2);

    return buf;
}
#endif

static int msg_size[COMM_NUMREV+2] = {
			LM_MSG_LEN_v1_0,	/* COMM v1.0 message length */
			LM_MSG_LEN_v1_1,	/* COMM v1.1 message length */
			LM_MSG_LEN_v1_2,	/* COMM v1.2 message length */
			LM_MSG_LEN_v1_3,	/* COMM v1.3 message length */
			LM_MSG_LEN_MIN		/* minimum: MUST BE LAST */
		     };

API_ENTRY
l_msg_size(which)
int which;
{
	if (which < 0 || which > COMM_NUMREV + 1) which = COMM_NUMREV + 1;
	return(msg_size[which]);
}
