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
 *      Module: $Id: l_aladdin.c,v 1.5 2003/04/18 23:48:01 sluu Exp $
 *
 *      Description:    Controls the aladdin dongles
 *
 *
 *      Last changed:  4/10/2001
 *
 */

#include "lmachdep.h"
#include "lmclient.h"  
#include "l_prot.h"
#ifdef PC

#include "hasp.h"

#define GSICode1 21307		/* from aladdin */
#define GSICode2 21513		/* from aladdin */
#define DEMOCode1 15417
#define DEMOCode2 9632
#define MAXDEVICES 55
int iDevicesFound, Service, LptNum, SeedCode, Pass1, Pass2;
int p1, p2, p3, p4;
ULONG IDNum;
int iLoop, iDevCount;

/* dongle structure for aladdin USB dongle */
typedef struct AladdinDongle{
	int	 iPortNum;			/* 201-255 for USB */
	unsigned long lHostID; 			/* hostid number from dongle */
	int  iType;				/* USB or Parallel */
	int  iMem;				/* on board memory on dongle */
	int  iVer;				/* driver version of HASP */
} ALADDINDONGLESTRUCT;

ALADDINDONGLESTRUCT dongleArray[64];
#define LM_DONGLE_TYPE_PARALLEL 	0
#define LM_DONGLE_TYPE_USB 		1

HOSTID * l_check_aladdin(LM_HANDLE *job)
{
int iType;
char buf[16];
HOSTID_PTR newid, ret = 0, last = 0;

	Pass1 = GSICode1;
	Pass2 = GSICode2;
	LptNum = HASP25;		/* "HASP and MemoHASP on IBM" key type      */
	iType = LM_DONGLE_TYPE_USB;	/* in case we ever use the parallel dongles */
					/* for now we force USB dongle mode         */
	switch (iType)
	{
	case LM_DONGLE_TYPE_PARALLEL:
		break;
	case LM_DONGLE_TYPE_USB:
		Service = LOCALHASP_ISHASP;
		hasp (Service, SeedCode, LptNum, Pass1, Pass2, &p1, &p2, &p3, &p4);
		if (p1 != 1)
		{
			LM_SET_ERRNO(job, LM_NODONGLE, 511, errno);	/* no dongle found */
			return ret = 0;
		}

		/* save number of USB devices present*/
		iDevCount = p2;

		/* get status of device(s)
		 is it USB or PARALLEL */
		Service = LOCALHASP_HASPSTATUS;
		if (iDevCount > 1)
			hasp (Service, SeedCode, 201, Pass1, Pass2, &p1, &p2, &p3, &p4);
		else
			hasp (Service, SeedCode, LptNum, Pass1, Pass2, &p1, &p2, &p3, &p4);

		if (p3 > 200)
		{
			LptNum = p3;
		}
		else
		{
			iDevCount = 1;
		}

		/* 	in case there are multiple USB devices
			we must loop throught all 55 possible devices to insure that all devices
			are found. We cannot depend on plug & play system.
		*/
		iDevicesFound = 0;
		for (iLoop = 0; iLoop < MAXDEVICES; iLoop++)
		{
			/* get first set of dongle info
			*/
			Service = LOCALHASP_HASPSTATUS;
			hasp (Service, SeedCode, (201 + iLoop), Pass1, Pass2, &p1, &p2, &p3, &p4);
			if (p3 != (201+iLoop))
				continue;
			dongleArray[iDevicesFound].iPortNum = p3;
			dongleArray[iDevicesFound].iType = p2;
			dongleArray[iDevicesFound].iMem = p1;
			dongleArray[iDevicesFound].iVer = p4;
	
			/* get dongles hostid */
			Service = MEMOHASP_HASPID;
			hasp (Service, SeedCode, p3, Pass1, Pass2, &p1, &p2, &p3, &p4);
			if (p3 == 0)
			{
				/*IDNum = p1 + 65536 * p2; */
				IDNum = (unsigned) p2;
				IDNum <<= 16;
				IDNum |= (unsigned) p1;
				dongleArray[iDevicesFound].lHostID = IDNum;
				iDevicesFound++;
				//if (iDevicesFound == iDevCount)
				//	  break;	/* get out of for loop */
			}
			//LptNum++;	/* goto the next port */
		} 
		
		/* save dongle info to job */
		for (iLoop = 0; iLoop  < iDevicesFound; iLoop++)
		{
			if (iLoop == 0)
			{
				/* first one only */
				last = ret = l_new_hostid(); 				/* must first one */
				ret->type = HOSTID_FLEXID3_KEY;
				ret->id.data = dongleArray[iLoop].lHostID;		/* long int format */
				sprintf(buf, "%.8x", dongleArray[iLoop].lHostID);	/* create hex value string */
				memcpy(ret->id.string, buf, sizeof(buf));	/* OVERRUN */
			}
			else
			{	/* save the rest, if any */
				last->next = l_new_hostid();
				last->next->type = HOSTID_FLEXID3_KEY ;
				last->next->id.data = dongleArray[iLoop].lHostID;	/* long int format */
				sprintf(buf, "%.8x", dongleArray[iLoop].lHostID);	/* create hex value string */
				memcpy(last->next->id.string, buf, sizeof(buf));	/* OVERRUN */
				last = last->next;
			}
		}
	} /* end case */

	/* if no dongles found */
	if (!ret)
	{
		LM_SET_ERRNO(job, LM_NODONGLE, 499, errno);
	}

  return ret;
}

#endif	/* PC define */
