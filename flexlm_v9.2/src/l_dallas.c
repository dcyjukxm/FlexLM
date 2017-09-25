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
 *	Module: $Id: l_dallas.c,v 1.12 2003/01/13 22:41:50 kmaclean Exp $
 *      Description:    Controls the dallas dongles
 *
 *
 *      
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
//#if defined(PC) /*&& !defined(WINNT)*/ && !defined(OS2) && !defined(NLM)
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;
//#endif
/*    Access system function prototypes
*/
#ifdef SUPPORT_BORROW_METER
static void add_next_1991_dongle(LM_HANDLE *, LM_METERS **, int *, 
                                        unsigned char *);
#endif SUPPORT_BORROW_METER

#if defined(NLM)
#include <errno.h>
#endif


void strip_leading(char * rom );

#if defined( WINNT) || defined(NLM) || defined(OS2)
uchar    l_ds_setup(uchar);
uchar    l_ds_first(void);
uchar    l_ds_next(void);
uchar    l_ds_access(void);
uchar    l_ds_databyte(uchar);
uchar    l_ds_gndtest(void);
uchar *  l_ds_romdata(void);
uchar    l_ds_keyopen(void);
uchar    l_ds_keyclose(void);
uchar    l_ds_dowcheck(void);


#endif

#ifdef PC16

uchar far pascal dowcheck(void);
uchar far pascal keyopen(void);
uchar far pascal keyclose(void);
uchar far pascal first(void);
uchar far pascal next(void);
uchar far pascal access(void);
uchar far pascal gndtest(void);
uchar far * far pascal romdata(void);
uchar far pascal databyte(uchar);
uchar far pascal setup(uchar);
#endif 

#define ReadMemory 0xF0
check_globes(addr1)
unsigned char * addr1;
{
int cntr; 
uchar mem[5];
if ((((*addr1 & 0x7f) ==0x09) ||  ((*addr1 & 0x7f) ==0x11))
        && l_ds_access())
        {
        l_ds_databyte(ReadMemory);
        l_ds_databyte(0x21);
        l_ds_databyte(0x00);
        l_ds_databyte(0xFF);
        for (cntr=0;cntr<4;cntr++)
                {
                mem[cntr]= l_ds_databyte(0xFF);
                }
        if (mem[0]==0x1c && mem[1]==0x11 &&!mem[2] && !mem[3]) //make sure that this is our dongle
                return 1;
        else
                return 0;       
        }
        else
                return 0;
}

HOSTID * l_check_dallas(job)
LM_HANDLE *job;
{
  unsigned char buff[18], * rom;
  unsigned char  PortNum=job->dongle_ports[1],PortStop;
  HOSTID *last = 0, *ret = 0, *newid;

        memset( buff,'\0',13);
        rom=buff;
        if (PortNum)
        {
                PortStop=PortNum+1;
                PortNum--;      
        }
        else 
        {
                PortStop=4;
                PortNum=0;
        }
        
#ifdef NLM
        ThreadSwitch();
#endif     
        
        if (!l_ds_dowcheck())
        {
#ifndef NLM
                LM_SET_ERROR(job, LM_NODONGLEDRIVER, 502, GetLastError(), 
                                                "Dallas", LM_ERRMASK_ALL);
#else
                LM_SET_ERROR(job, LM_NODONGLEDRIVER, 502, -9, 
                                                "Dallas", LM_ERRMASK_ALL);
#endif
                return 0;
        }
        while ((++PortNum < PortStop) && l_ds_setup((unsigned char)PortNum) )
        {
                
                l_ds_keyopen()  ;
                
                
                if (l_ds_first()) do
                {
#ifdef NLM
                        ThreadSwitch();
#endif     
                        
                        rom = l_ds_romdata();
#ifdef NLM
                        ThreadSwitch();
#endif     
                        
                        sprintf((char *)buff, 
                        "%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X",
                                rom[6], rom[5], rom[4], rom[3],
                                rom[2], rom[1]);
                        
                        
/*
*                              add test for Globetrotter ID as 
*                              well as 1410 ID
*/

                        if (check_globes(rom))   
                        
                        {
                                newid = l_new_hostid();
                                newid->type =  HOSTID_FLEXID2_KEY ;
                                memcpy(newid->id.string, buff, 13); 
                                if (PortNum < 4 ) 
                                {
                                        job->dongle_ports[1]= PortNum; 
                                }
                                PortNum=5;
                                if (!ret) ret = last = newid;
                                else
                                {
                                        last->next = newid;
                                        last = newid;
                                }
                        }
                        
                } while (l_ds_next());
                
                
                l_ds_keyclose(); 
        } //while 
        if (!ret)
        {
                LM_SET_ERRNO(job, LM_NODONGLE, 503, errno);
        }
        return ret;
}

#if !defined(PC16) && !defined(NLM) && defined (SUPPORT_METER_BORROW)

void 
l_enumerate_dallas(job, m, dongle_num )
LM_HANDLE *job;
LM_METERS ** m;
int * dongle_num ;

{
  int OneDongle=1;
  unsigned char  PortNum=job->dongle_ports[1],PortStop;
  HOSTID_PTR newidptr,current=job->idptr;
     		
        if (PortNum)
        {
                PortStop=PortNum+1;
                PortNum--;      
        }
        else 
        {
                PortStop=4;
                PortNum=0;
        }
        
        
        if (l_ds_dowcheck())
        {
                while ((++PortNum < PortStop) && l_ds_setup((unsigned char)PortNum) )
                {
                        
                        l_ds_keyopen()  ;
                        
                        if (1)//gndtest())
                        {
                                if (l_ds_first())
                                { 
                                        do
                                        {
                                                add_next_1991_dongle(job, m, 
                                                        dongle_num, &PortNum);
                                                
                                        } while (l_ds_next());
                                } //l_ds_first
                        
                        } // if gndtest
                        
                        l_ds_keyclose(); 
                } //while 
        }     //l_ds_dowcheck
}
  
  

   
        
#ifdef SUPPORT_BORROW_METER
static
void
add_next_1991_dongle(LM_HANDLE *job, LM_METERS **m, int *dongle_num, 
                unsigned char *PortNum)
{

  unsigned char buff[18], * rom, rom1[10];
  int i;

        rom = l_ds_romdata();
          // is this a 1991 dongle
        if ( ( rom[0] & 0x7f ) == 02)   
        {
                memset(rom1,0,10);
                memcpy(rom1,rom,8);
                strip_leading(rom1);
                for ( i=0; i< 3; i++ )
                {
                        sprintf((char *)buff, "dv:%s-%1d",rom1,i);
                        l_borrow_add_meter_list( job, m, buff);
                        (*dongle_num)++;
                }
                if (*PortNum < 4) job->dongle_ports[1] = *PortNum; 
                *PortNum=5;
        
        }
}
#endif /* SUPPORT_BORROW_METER */

void
strip_leading(char * rom )
{
  DWORDLONG DallasIdNumber;
	DallasIdNumber= ((rom[6]&0xff)<<40) + 
			((rom[5]&0xff)<<32 )+ 
			((rom[4]&0xff)<<24) + 
			((rom[3]&0xff)<<16 )+ 
			((rom[2]&0xff)<<8 )+ 
			(rom[1]&0xff) ; 

        sprintf((char *)rom, "%I64X", DallasIdNumber);


}
#endif /* only winnt */

//#endif /* pc*/ 

