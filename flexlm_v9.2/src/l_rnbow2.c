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
 *      Module: $Id: l_rnbow2.c,v 1.7 2003/01/13 22:41:55 kmaclean Exp $
 *
 *      Description:    Controls the rainbow dongles
 *			For the Second set of two dongles
 *
 *      Last changed:  2/6/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"  
#include "l_prot.h"                                                  

#if defined(PC) && !defined(WINNT) && !defined(NLM) && !defined(OS2)
#include "16_superpro.h"
#include "16_spromeps.h"
#define SPRO_SERIAL_START       0
#define SPRO_GSI_DEV_ID         0x3FA5
#define SPRO_GSI_DEV_ID2        0xA630
                /*
                 *      ID_STRING on PC indocates a hardware key desired.
                 *      Read in the hardware key serial number.
                 */
//                extern int sproInitialize(void);
//                extern int sproFindFirstUnit(unsigned int, UNITINFO *);
//                extern int sproRead(UNITINFO *, int, unsigned int *);
//                extern int sproFindNextUnit(unsigned int, UNITINFO *);
HOSTID * l_check_rainbow2(LM_HANDLE *job)
{               
  unsigned short serial_1, serial_2;
  UNITINFO u;
  HOSTID_PTR *newid, *ret = 0, *last = 0;

        /* default to NULL id */
        //sproStat = RNBOsproFormatPacket( &gSproApiPacket, sizeof(gSproApiPacket) );
        if ( sproInitialize() == SP_SUCCESS )
        {
                if ((sproFindFirstUnit( SPRO_GSI_DEV_ID, &u )== SP_SUCCESS)
                        && ((sproRead( &u, SPRO_SERIAL_START, 
                                &serial_1 ) == SP_SUCCESS) &&
                        (sproRead(&u, SPRO_SERIAL_START+1, 
                                &serial_2) == SP_SUCCESS)))
                {
                        last = ret = l_new_hostid(); /* must first one */
                        ret->type = HOSTID_FLEXID6_KEY;
                        ret->id.data = (((unsigned long)serial_2) << 16) |
                                                (unsigned long)serial_1;
                }                       
                
                while (sproFindNextUnit( &u )== SP_SUCCESS)
                {
                        if ((sproRead( &u, SPRO_SERIAL_START, &serial_1) == 
                                SP_SUCCESS) &&
                                (sproRead(&u, SPRO_SERIAL_START+1, &serial_2)
                                == SP_SUCCESS))
                        {
                                last->next = l_new_hostid();
                                last->next->type = HOSTID_FLEXID6_KEY ;
                                last->next->id.data =
                                        (((unsigned long)serial_2) << 16) |
                                                (unsigned long)serial_1;
                                last = last->next
                        } 
                }                       
                
                
                
                if ((sproFindFirstUnit( SPRO_GSI_DEV_ID2, &u )== SP_SUCCESS)
                        && ((sproRead(&u, SPRO_SERIAL_START, &serial_1)
                                == SP_SUCCESS) &&
                                (sproRead( &u, SPRO_SERIAL_START+1, 
                                        &serial_2 ) == SP_SUCCESS)))
                {
                        newid = l_new_hostid();
                        newid->type = HOSTID_FLEXID6_KEY;
                        newid->id.data =
                        (((unsigned long)serial_2) << 16) |
                                (unsigned long)serial_1;
                        if (!ret)  last = ret = newid;
                        else 
                        {
                                last->next = newid;
                                last = newid;
                        }

                } 
                        
                
                while (sproFindNextUnit( &u )== SP_SUCCESS)
                {
                        if ( (sproRead( &u, SPRO_SERIAL_START, &serial_1 )
                                == SP_SUCCESS) &&
                                (sproRead(&u, SPRO_SERIAL_START+1, &serial_2 )==
                                SP_SUCCESS))
                        {
                                last->next = l_new_hostid();
                                last->next->type = HOSTID_FLEXID6_KEY;
                                last->next->id.data = 
                                        (((unsigned long)serial_2) << 16) |
                                                (unsigned long)serial_1;
                                last = last->next;
                        } 
                        
                        
                }                       
        }
        else

        return ret;
}
#endif
#if defined (WINNT) || defined (OS2) 
#define GOTONE

#ifdef OS2
#undef _WIN32_  /* Causes problems with OS/2 spromeps */
#undef WIN32
#undef _WIN32
#undef __WIN32__
#undef __NT__
#endif

#include <spromeps.h>
#define SPRO_SERIAL_START       0
#define SPRO_GSI_DEV_ID         0x3FA5
#define SPRO_GSI_DEV_ID2        0xA630
HOSTID * l_check_rainbow2(LM_HANDLE *job)
{
  unsigned short serial_1, serial_2;
  RB_SPRO_APIPACKET gSproApiPacket;  /* SuperPro packet */
  HOSTID_PTR newid, last = 0, ret = 0;
  SP_STATUS sproStat;
        
        /* default to NULL id */
        
        RNBOsproFormatPacket(&gSproApiPacket, sizeof(gSproApiPacket));
        
        if ((sproStat = RNBOsproInitialize(&gSproApiPacket)) != SP_SUCCESS)
	{
           char buf[200];
                sprintf(buf, "Rainbow: %d", sproStat);
                LM_SET_ERROR(job, LM_NODONGLEDRIVER, 501, 0, buf, LM_ERRMASK_ALL);
                return 0;
	}

        if ((RNBOsproFindFirstUnit( &gSproApiPacket, SPRO_GSI_DEV_ID )
                                                == SP_SUCCESS) 
                && ((RNBOsproRead(&gSproApiPacket, SPRO_SERIAL_START, 
                                &serial_1) == SP_SUCCESS) 
               && (RNBOsproRead(&gSproApiPacket, SPRO_SERIAL_START+1, 
                                &serial_2 ) ==SP_SUCCESS)))
        {
                ret = last = l_new_hostid();
                ret->type = HOSTID_FLEXID6_KEY ;
                ret->id.data = (((unsigned long)serial_2) << 16) |
                                (unsigned long)serial_1;
        }                       
        
        while (RNBOsproFindNextUnit( &gSproApiPacket)== SP_SUCCESS)
        {
                if ((RNBOsproRead( &gSproApiPacket, 
                        SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                        (RNBOsproRead( &gSproApiPacket, SPRO_SERIAL_START+1, 
                                &serial_2 ) ==SP_SUCCESS) )
                {
                        last->next = l_new_hostid();
                        last->next->type = HOSTID_FLEXID6_KEY ;
                        last->next->id.data = 
                                (((unsigned long)serial_2) << 16) |
                                                (unsigned long)serial_1;
                        last = last->next;
                } 
        }                       
        
        if ((RNBOsproFindFirstUnit( &gSproApiPacket, SPRO_GSI_DEV_ID2)
                                                        == SP_SUCCESS) 
                && ((RNBOsproRead( &gSproApiPacket, SPRO_SERIAL_START, 
                                        &serial_1) == SP_SUCCESS) 
                && (RNBOsproRead( &gSproApiPacket, SPRO_SERIAL_START+1,
                                        &serial_2) == SP_SUCCESS)))
        {
                newid = l_new_hostid();
                newid->type = HOSTID_FLEXID6_KEY;
                newid->id.data = (((unsigned long)serial_2) << 16) |
                                                (unsigned long)serial_1;
                if (!ret) ret = newid;
                else last->next = newid;

                last = newid;
        }                       
        
        while (RNBOsproFindNextUnit( &gSproApiPacket)== SP_SUCCESS)
        {
                if ((RNBOsproRead( &gSproApiPacket, SPRO_SERIAL_START,
                                &serial_1 ) ==SP_SUCCESS) &&
                (RNBOsproRead( &gSproApiPacket, SPRO_SERIAL_START+1,
                        &serial_2 ) ==SP_SUCCESS) )
                {
                        last->next = l_new_hostid();
                        last->next->type = HOSTID_FLEXID6_KEY;
                        last->next->id.data =
                                        (((unsigned long)serial_2) << 16) |
                                                (unsigned long)serial_1;
                        last = last->next;
                } 
        }                       
        if (!ret)
        {
                LM_SET_ERRNO(job, LM_NODONGLE, 498, errno);
        }
        return ret;
}

#endif

#ifdef NLM
#include "superpro.h"
#include "spromeps.h"

#define SPRO_SERIAL_START       0
#define SPRO_GSI_DEV_ID         0x3FA5
#define SPRO_GSI_DEV_ID2        0xA630
void l_check_rainbow2(job)
LM_HANDLE *job;
{
unsigned int serial_1, serial_2;

UNITINFO  info;
 /* SuperPro packet */
int OneDongle=1;
HOSTID_PTR newidptr,current=job->idptr;
// extern void INT3(void);
//#pragma aux INT3="int 3h"      
//  INT3();               
/* default to NULL id */
job->idptr->id.data = 0;
                ThreadSwitch();
                if ( sproInitialize(  ) == SP_SUCCESS)
                {
                        if (sproFindFirstUnit( SPRO_GSI_DEV_ID,&info )== SP_SUCCESS)
                                { ThreadSwitch();
                                if (OneDongle)
                                        {
                                        if ( (sproRead( &info, SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                                                (sproRead(&info, SPRO_SERIAL_START+1,&serial_2 ) ==SP_SUCCESS) )
                                                        {
                                                        current->id.data =
                                                        (((unsigned long)serial_2) << 16) |
                                                        (unsigned long)serial_1;
                                                        OneDongle=0;
                                                        } 
                                        }
                                        else
                                        {
                                        if (!(newidptr = (HOSTID *)l_malloc(job,sizeof(HOSTID))))
                                                {
                                                LM_SET_ERRNO(job, LM_CANTMALLOC, 259, 0);
                                                return ;
                                                }
        
                                        memset(newidptr, 0, sizeof(HOSTID));
                                        newidptr->type = HOSTID_FLEXID6_KEY ;
                                        current->next=newidptr;
                                        current=newidptr;                       
                                        if ( (sproRead( &info, SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                                                (sproRead( &info, SPRO_SERIAL_START+1,&serial_2 ) ==SP_SUCCESS) )
                                                        {
                                                        current->id.data =
                                                        (((unsigned long)serial_2) << 16) |
                                                        (unsigned long)serial_1;
                                                        } 
                                        
                                        }               
                                }                       
                                ThreadSwitch();
                                while (sproFindNextUnit( &info)== SP_SUCCESS)
                                { ThreadSwitch();
                                         if (!(newidptr = (HOSTID *)l_malloc(job,sizeof(HOSTID))))
                                                {
                                                LM_SET_ERRNO(job, LM_CANTMALLOC, 259, 0);
                                                return ;
                                                }
        
                                        memset(newidptr, 0, sizeof(HOSTID));
                                        newidptr->type = HOSTID_FLEXID6_KEY ;
                                        current->next=newidptr;
                                        current=newidptr;                       
                                        if ( (sproRead( &info, SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                                                (sproRead( &info, SPRO_SERIAL_START+1,&serial_2 ) ==SP_SUCCESS) )
                                                        {
                                                        current->id.data =
                                                        (((unsigned long)serial_2) << 16) |
                                                        (unsigned long)serial_1;
                                                        } 
                                        
                                                        
                                }                       
                
        
                
                        if (sproFindFirstUnit( SPRO_GSI_DEV_ID2,&info )== SP_SUCCESS)
                                { ThreadSwitch();
                                if (OneDongle)
                                        {
                                        if ( (sproRead( &info, SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                                                (sproRead(&info, SPRO_SERIAL_START+1,&serial_2 ) ==SP_SUCCESS) )
                                                        {
                                                        current->id.data =
                                                        (((unsigned long)serial_2) << 16) |
                                                        (unsigned long)serial_1;
                                                        OneDongle=0;
                                                        } 
                                        }
                                        else
                                        {
                                        if (!(newidptr = (HOSTID *)l_malloc(job,sizeof(HOSTID))))
                                                {
                                                LM_SET_ERRNO(job, LM_CANTMALLOC, 259, 0);
                                                return ;
                                                }
        
                                        memset(newidptr, 0, sizeof(HOSTID));
                                        newidptr->type = HOSTID_FLEXID6_KEY ;
                                        current->next=newidptr;
                                        current=newidptr;                       
                                        if ( (sproRead( &info, SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                                                (sproRead( &info, SPRO_SERIAL_START+1,&serial_2 ) ==SP_SUCCESS) )
                                                        {
                                                        current->id.data =
                                                        (((unsigned long)serial_2) << 16) |
                                                        (unsigned long)serial_1;
                                                        } 
                                        
                                        }               
                                }                       
                
                                while (sproFindNextUnit( &info)== SP_SUCCESS)
                                { ThreadSwitch();
                                        if (!(newidptr = (HOSTID *)l_malloc(job,sizeof(HOSTID))))
                                                {
                                                LM_SET_ERRNO(job, LM_CANTMALLOC, 259, 0);
                                                return ;
                                                }
        
                                        memset(newidptr, 0, sizeof(HOSTID));
                                        newidptr->type = HOSTID_FLEXID6_KEY ;
                                        current->next=newidptr;
                                        current=newidptr;                       
                                        if ( (sproRead( &info, SPRO_SERIAL_START,&serial_1 ) ==SP_SUCCESS) &&
                                                (sproRead( &info, SPRO_SERIAL_START+1,&serial_2 ) ==SP_SUCCESS) )
                                                        {
                                                        current->id.data =
                                                        (((unsigned long)serial_2) << 16) |
                                                        (unsigned long)serial_1;
                                                        } 
                                        
                                                        
                                }                       
                
        
                
                                
                }
}




#endif
