/******************************************************************************

            COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *       
 *      
 *      Module: $Id: l_rainbow.c,v 1.10 2003/01/13 22:41:54 kmaclean Exp $
 *
 *      Description:    Controls the rainbow dongles
 *
 *
 *      Last changed:  2/6/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"  
#include "l_prot.h"                                                  
#ifdef PC

static unsigned short l_flexid7[] = 
{
	0xB285, 
	0x36C7,
	0x0
};
static unsigned short l_flexid6[] = 
{
 	0x3FA5,		   
	0xA630,
 	0x39cd,	/* new ID 1*/
	0x36a3,	/* new ID 2*/
	0x4a46,	/* new ID 3*/
	0xf0a9,	/* new ID 4*/
	0x0119,	/* new ID 5*/
	0xdc55,	/* new ID 6*/
	0xc591,	/* new ID 7*/
	0xa2d9,	/* new ID 8*/
	0xd086,	/* new ID 9*/
	0x5615,	/* new ID 10*/
	0xDB34, /* rainbow USB 1 */
	0x0
};
//#define L_FLEXID7_CNT 2 /* The first 2 are flexid 7s */

#include <spromeps.h>
#define SPRO_SERIAL_START       0
HOSTID * 
l_check_rainbow(LM_HANDLE *job, int idtype)
{
  unsigned short serial_1, serial_2;
  RB_SPRO_APIPACKET gSproApiPacket;  /* SuperPro packet */
  HOSTID_PTR newid, last = 0, ret = 0, *hp;
  SP_STATUS sproStat;
  unsigned int id; 
  unsigned short *flexids;
  unsigned short t_id = 0;
        
        /* default to NULL id */
        
	if (job->host_comp && (job->host_comp->type == idtype))
		t_id = (short)(job->host_comp->id.data  >> 16);

        RNBOsproFormatPacket(&gSproApiPacket, sizeof(gSproApiPacket));
        
        if ((sproStat = RNBOsproInitialize(&gSproApiPacket)) != SP_SUCCESS)
	{
           char buf[200];
                sprintf(buf, "Rainbow: %d", sproStat);
                LM_SET_ERROR(job, LM_NODONGLEDRIVER, 501, 0, buf, LM_ERRMASK_ALL);
                return 0;
	}
	if (idtype == HOSTID_FLEXID6_KEY) /* FLEXID 7*/
		flexids = l_flexid6;
	else
		flexids = l_flexid7;

        for (id = 0; flexids[id]; id++)
	{
		if (t_id && (t_id != flexids[id]))
			continue;
		
		if ((RNBOsproFindFirstUnit( &gSproApiPacket, flexids[id] )
							== SP_SUCCESS) 
			&& ((RNBOsproRead(&gSproApiPacket, SPRO_SERIAL_START, 
					&serial_1) == SP_SUCCESS) 
		       && (RNBOsproRead(&gSproApiPacket, SPRO_SERIAL_START+1, 
					&serial_2 ) ==SP_SUCCESS)))
		{
			if (!ret) 
				hp = &ret;		/* first */
			else 
				hp = &last->next;
				
			*hp = l_new_hostid();
			(*hp)->type = idtype ; 
			(*hp)->id.data = (((unsigned long)serial_2) << 16) |
					(unsigned long)serial_1;
			
			if (!last)
				last = *hp;
			else last = last->next;		/* last */



			while (RNBOsproFindNextUnit( &gSproApiPacket)== SP_SUCCESS)
			{
				if ((RNBOsproRead( &gSproApiPacket, 
					SPRO_SERIAL_START,&serial_1 ) ==
							SP_SUCCESS) &&
					(RNBOsproRead( &gSproApiPacket, 
					SPRO_SERIAL_START+1, &serial_2 ) ==
						SP_SUCCESS))
				{
					last->next = l_new_hostid();
					last->next->type = idtype ;
					last->next->id.data = 
					  (((unsigned long)serial_2) << 16)|
						(unsigned long)serial_1;
					last = last->next;
				} 
			}
		}
        
	}
        
        if (!ret)
        {
                LM_SET_ERRNO(job, LM_NODONGLE, 498, errno);
        }
        return ret;
}

#endif /* PC */
