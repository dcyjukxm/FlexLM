/******************************************************************************

            COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of 
        Macrovision Corporation and is protected by law.  It may 
        not be copied or distributed in any form or medium, disclosed 
        to third parties, reverse engineered or used in any manner not 
        provided for in said License Agreement except with the prior 
        written authorization from Macrovision Corporation.

 *****************************************************************************/
/******************************************************************************
 *
 *
 *      NOTE:   The purchase of FLEXlm source does not give the purchaser
 *
 *              the right to run FLEXlm on any platform of his choice.
 *
 *              Modification of this, or any other file with the intent 
 *
 *              to run on an unlicensed platform is a violation of your 
 *
 *              license agreement with Macrovision Corporation. 
 *
 *
 *****************************************************************************/
/*      
 *      
 *	Module: $Id: l_vol_id.c,v 1.11 2003/04/16 18:05:33 brolland Exp $
 *      Function: l_vol_id.c
 *
 *      Description: This module is used to get volume serial number from
 *      C:.  It could have been an easy task with a call to Win32
 *      GetVolumeInformation().  However, this call is broken under Win32s.
 *      (See Microsoft Deveveloper Network CD: Knowledge Base Bug List:PSS
 *      ID Number Q93639.)  The only way to get around it is to create a 16
 *      bit DLL for our 32 bit DLL (lmgr32.dll) to call.  This module uses
 *      Win32 standard Universal Thunk mechanism to call a 16 bit DLL.
 *
 *      Parameters:     None
 *
 *      Return:         (long) - the C: drive volume serial number
 *
 *      Chia-Chee Kuan Blane Eisenberg
 *      8/25/94   7/14/95 9/9/95
 *  Made changes for Windows 95 32 bit mode and Secure NT
 *  Ethernet address acqusition
 *      Last changed:  9/24/98
 *
 *
 */
static char * lm_no_netbios = (char *)-1;

#define W32SUT_32

#include <lmachdep.h>
#include "lmclient.h"
#include <windows.h>
#include <wsipx.h>
#include "pcsock.h"

// for NetBios way of getting Ethernet Address


#include <lmcons.h>
#include <lmapibuf.h>
#include <lmserver.h>
#include <lmwksta.h>
#include <winioctl.h>
//#include <wchar.h>
#include "ntddndis.h"
#include <string.h>
#include <stdio.h>
#include <nb30.h>
#include <time.h>
//
typedef  unsigned char ETHERARRAY[6];
ETHERARRAY EtherAddrs[5];
int EtherCount=0;
time_t EtherTime;
// 
static get_my_ipx_nodenum(LM_HANDLE * job);     
BOOL get_NT_Address( char * buf);

#include <w32sut.h>
typedef BOOL (WINAPI * PUTREGISTER) ( HANDLE     hModule,
                                      LPCSTR     lpsz16BitDLL,
                                      LPCSTR     lpszInitName,
                                      LPCSTR     lpszProcName,
                                      UT32PROC * ppfn32Thunk,
                                      FARPROC    pfnUT32Callback,
                                      LPVOID     lpBuff
                                    );

long patch_volume( void );

typedef VOID (WINAPI * PUTUNREGISTER) (HANDLE hModule);

//extern int l_getntaddr( int osver, char e_addr[13]);

extern HANDLE hInstance;

#define L_GET_VOL_ID            0
#define L_GET_ETHER_ADDR        1
 

long normal_volume( )   // the way things would always work in a 32 bit world

        {
         long vol_serial_number;
         DWORD max_len, sys_flags;
        char  buf1[20];
        int drive;
                strcpy(buf1,"A:\\");
                for (drive=0;drive<3 ; drive++)
                {
                        if (DRIVE_FIXED==GetDriveType(buf1))
                                break;
                        buf1[0]++;
                }

                GetVolumeInformation(buf1,              /* RootPathName */
                                     NULL,              /* Volume name */
                                     0,                 /* len of vol name */
                                     &vol_serial_number,/* serial number */
                                     &max_len,          /* max len filename */
                                     &sys_flags,        /* system flag */
                                     NULL,              /* file system name */
                                     0                  /* len of sys name */
                                     );
                return(vol_serial_number);
        }




long l_vol_id(void)
{

 // this is for either win32s, windows95, or windows NT on Intel paltform
DWORD version_number;

         version_number= GetVersion();

        if (  version_number < 0x80000000 )      //Windows NT for Intel

        {

        return          normal_volume();
        
        }

        else if (  (LOBYTE(LOWORD(version_number))<4) )  //Win32s for Intel
        {
        /*
         *      Since GetVolumeInformation() does not get volume serial
         *      number in Win32s environment, ( See Microsoft Deveveloper
         *      Network CD: Knowledge Base Bug List:PSS ID Number
         *      Q93639), we will now try to call a 16 bit DDL thunk.
         */

        
        return (patch_volume());        

        }
   else // Windows 95 case

        {
                return  normal_volume();
        
        }


}


   long patch_volume()

 {

        UT32PROC      pfnUTProc = NULL;
        PUTREGISTER   pUTRegister = NULL;
        PUTUNREGISTER pUTUnRegister = NULL;
        HANDLE        hKernel32 = 0;
        int ret;
        long vol_serial_number;
        /* Get Handle to Kernel32.Dll */
        hKernel32 = LoadLibrary( "Kernel32.Dll" ); 
        pUTRegister = (PUTREGISTER) GetProcAddress( hKernel32, "UTRegister" );
        if ( !pUTRegister )
        {
                FreeLibrary(hKernel32);
                return(-1);     
        }

        pUTUnRegister = (PUTUNREGISTER)
                        GetProcAddress(hKernel32,"UTUnRegister" );

        if (! pUTUnRegister )
        {
                FreeLibrary( hKernel32 );
                return(-1);     
        }                                

        ret = (*pUTRegister)( hInstance,        // UTSamp32.DLL module handle
                        "VSN16.DLL",  // name of 16bit thunk dll
                        NULL,            // no init routine
                        "UTProc",        // name of 16bit dispatch routine
                                         // exported from UTSamp16.DLL
                        &pfnUTProc,      // Global variable to receive thunk
                                         // address
                        NULL,            // no callback function
                        NULL );          // no shared memroy

        if ( !ret || !pfnUTProc )
                ret = (*pUTRegister)( hInstance, // UTSamp32.DLL module handle
                        "VSN16.DLL",  // name of 16bit thunk dll
                        NULL,            // no init routine
                        "UTProc",        // name of 16bit dispatch routine
                                         // exported from UTSamp16.DLL
                        &pfnUTProc,      // Global variable to receive thunk
                                         // address
                        NULL,            // no callback function
                        NULL );          // no shared memroy

        
        if ( !ret || !pfnUTProc )
        {
                (*pUTUnRegister) ( hInstance );
                FreeLibrary( hKernel32 );
                return -1; 
        }
        
        /*
         *      Now, call to the 16 bit DLL to get volume serial number.
         */
        vol_serial_number = (* pfnUTProc)( NULL, L_GET_VOL_ID, NULL );

        /*
         *      We are done, free the kernel library handle.
         */
                           
        (*pUTUnRegister) ( hInstance );
        FreeLibrary( hKernel32 );

        return vol_serial_number;
         }


BOOL get_netbios_ether_addr1(LM_HANDLE *job);

BOOL get_pc_ether_addr1( LM_HANDLE *job )
{
//#if defined(_ALPHA_) || defined(_MIPS_) || defined (_PPC_)
//      
//      if (get_netbios_ether_addr1(job))
//              return TRUE;
//      else
//              return get_my_ipx_nodenum( job);
//
//
//#else           // Win32s, Windows 95, or Windows NT on Intel

  // this is for either win32s, windows95, or windows NT on Intel paltform
  DWORD version_number;

       
        if (EtherCount && (time(0)-300) < EtherTime )
        {
          HOSTID_PTR newidptr,current=job->idptr;
          int i,first=1;
                
                for (i=0;i<EtherCount;i++)
                {
                        if (first)
                        {
                                first=0;
                                memcpy(job->idptr->id.e ,EtherAddrs[i] ,6);
                                job->idptr->type = HOSTID_ETHER;

                        }
                        else
                        {
                                newidptr=l_new_hostid();
                                newidptr->type =HOSTID_ETHER;
                                current->next=newidptr;
                                current=newidptr;                       
                                memcpy(current->id.e ,EtherAddrs[i],6); 

                        }
                
                }
                return TRUE;
        }
        EtherCount=0;
        version_number= GetVersion();
        EtherTime=time(0);
        if (  version_number < 0x80000000 )      //Windows NT for Intel

        {

//
//               New Function for adding foolproof ethernet address for nt
//          
/*	This code forces all MAC address query code to go to l_ntadd.c module rather than 
	doing this from this module. This was put in to address the problems encountered 
	when running on 95/98/ME.
*/
extern         BOOL l_ntadd( LM_HANDLE_PTR job);                                              
                if ( l_ntadd(job))  

                {
                        return TRUE;

                }

 
                if (lm_no_netbios == (char *)-1)
                        lm_no_netbios = getenv("LM_NO_NETBIOS");	/* overrun checked */

		return  get_my_ipx_nodenum( job);
        }

        else if (  (LOBYTE(LOWORD(version_number))<4) )  //Win32s for Intel
        {
#if !defined(_ALPHA_) && !defined(_MIPS_) && !defined(_PPC_)
           static old_ether_add(  LM_HANDLE * job);

                return (old_ether_add( job ));  

#endif /* risC */
        }
        else // Windows 95 case

        {        // new way of getting address ( no registry problem)
        
                
                if (lm_no_netbios == (char *)-1)
                        lm_no_netbios = getenv("LM_NO_NETBIOS");	/* overrun checked */
 
                if ( !lm_no_netbios && (get_netbios_ether_addr1(job) == TRUE))
                        return TRUE;
                else
						return  get_my_ipx_nodenum( job);
        
        }

 }
#if !defined(_ALPHA_) && !defined(_MIPS_) && !defined(_PPC_)
BOOL
// static
old_ether_add( LM_HANDLE * job )

{
    
//              FILE *fp;
//      char *mode="a";
//      char *filename="thunk.log";
         char * mem_buff;

        UT32PROC      pfnUTProc = NULL;
        PUTREGISTER   pUTRegister = NULL;
        PUTUNREGISTER pUTUnRegister = NULL;
        HANDLE        hKernel32 = 0;
        BOOL          ret;
        DWORD             k;
        /*

         *      Call to a 16 bit DLL thunk to get NDIS or Packet Driver
         *      ethernet address.
         */
        
        
//              fp = fopen(filename,mode);

        
        // will not work on Windows 95


        /* Get Handle to Kernel32.Dll */
        hKernel32 = LoadLibrary( "Kernel32.Dll" ); 


        pUTRegister = (PUTREGISTER) GetProcAddress( hKernel32, "UTRegister" );
        
//      fprintf(fp,"hInstance: %x\n",hInstance);
//      fprintf(fp,"First putregister: %x\n",pUTRegister);

        if ( !pUTRegister )
        {
                FreeLibrary(hKernel32);
//              fprintf(fp,"pUT register failed");
//              fclose(fp);
                return(-1);     
        }



        pUTUnRegister = (PUTUNREGISTER)
                        GetProcAddress(hKernel32,"UTUnRegister" );

        if (! pUTUnRegister )
        {
                FreeLibrary( hKernel32 );
//              fprintf(fp,"pUTUnregister failed");
//              fclose(fp);

                return(-1);     
        }                                



        ret = (*pUTRegister)( hInstance,           // UTSamp32.DLL module handle
                        "VSN16.DLL",  // name of 16bit thunk dll
                        NULL,            // no init routine
                        "UTProc",        // name of 16bit dispatch routine
                                         // exported from UTSamp16.DLL
                        &pfnUTProc,      // Global variable to receive thunk
                                         // address
                        NULL,            // no callback function
                        NULL );          // no shared memroy


//      fprintf(fp,"First call to UTregister: %x %x\n",ret,pfnUTProc);
//      if (!ret)
//      fprintf(fp,"Error ReturnFirst call to UTregister: %i\n",GetLastError());

        if ( !ret || !pfnUTProc )
                ret = (*pUTRegister)( hInstance,           // UTSamp32.DLL module handle
                                "VSN16.DLL",  // name of 16bit thunk dll
                                NULL,            // no init routine
                                "UTProc",        // name of 16bit dispatch routine
                                                 // exported from UTSamp16.DLL
                                &pfnUTProc,      // Global variable to receive thunk
                                                 // address
                                NULL,            // no callback function
                                NULL );          // no shared memroy
//      fprintf(fp,"Second call to UTregister: %x %x\n",ret,pfnUTProc);
//      if (!ret)
//      fprintf(fp,"Error Return Second call to UTregister: %i\n",GetLastError());



        if (!ret || !pfnUTProc)
        {
                (*pUTUnRegister) ( hInstance );
                FreeLibrary( hKernel32 );
//              fprintf(fp,"pfnUTproc was zero the second time\n");
//                 fclose(fp);

                // if all else fails, try the sneaky way

                return get_my_ipx_nodenum( job );
        }

        mem_buff=(char *)malloc(20);
        /*
         *      Now, call to the 16 bit DLL to get volume serial number.
         */
//      fprintf(fp,"About to call l_get ether addr\n");
//      k=L_GET_ETHER_ADDR;
        ret = (* pfnUTProc)( mem_buff, L_GET_ETHER_ADDR , NULL );
//      fprintf(fp,"returned from l_get_ether_addr : %16x\n",j);
        /*
         *      We are done, free the kernel library handle.
         */
        memcpy(job->idptr->id.e, mem_buff,6);                      
        job->idptr->type=HOSTID_ETHER;
        free(mem_buff);
        (*pUTUnRegister) ( hInstance );
        FreeLibrary( hKernel32 );
//      fprintf(fp,"Ethernet Address: %02x-%02x-%02x-%02x-%02x-%02x\n",buff[0],buff[1],buff[2], buff[3],buff[4],buff[5]);
//          fclose(fp);

        if ( !ret )                                     
                return get_my_ipx_nodenum( job );       
        else
                return ret;
}

#endif /* risc platforms */
//static
BOOL
get_my_ipx_nodenum( LM_HANDLE * job )
{
	char e_addr[13];

        struct sockaddr_ipx ipx_addr;
        SOCKET ipx_s;
        int addrlen;
/*	This will force the call for IPX/SPX to go to l_ntadd.c module.
*/
		l_ntadd(job);
		return 1;

        
        if ((ipx_s=socket(AF_NS, SOCK_STREAM, NSPROTO_SPX)) == INVALID_SOCKET )
                return 0;
                

        memset(&ipx_addr, 0, sizeof(ipx_addr));
        ipx_addr.sa_family = AF_NS;

        if (bind(ipx_s, (const struct sockaddr *) &ipx_addr, sizeof(ipx_addr)))
        {
                closesocket(ipx_s);
                return 0;
        }

        addrlen = sizeof(ipx_addr);
        if (getsockname(ipx_s, (struct sockaddr *) &ipx_addr, &addrlen))
        {
                closesocket(ipx_s);
                return 0;
        }

        memcpy( job->idptr->id.e , ipx_addr.sa_nodenum, sizeof(ipx_addr.sa_nodenum) );
        job->idptr->type=HOSTID_ETHER;
        closesocket(ipx_s);
        return 1;
}

/*
 *      Get Ethernet MAC address by using Netbios Specification and DPMI.
 */

typedef struct _ASTAT_
{
        ADAPTER_STATUS Adapt ; 
        NAME_BUFFER NameBuff[30];
} ASTAT, * PASTAT   ;

ASTAT Adapter ; 



typedef struct foo11 {
        unsigned char addr[6];
                                        }  eAddress_t;

BOOL check_new_adapter( eAddress_t etherlist[10], int * NumEtherEntries,
                                unsigned char * Adapteraddress)
{
  int i,valid_hostid=0;

        // first check if DEST

        if (  Adapteraddress[0] && !strncmp("DEST",Adapteraddress,4))
                                                        // it is, ignore it
                return FALSE;

        // next check if all zeroes......
        for (i=0;i<6;i++) if (Adapteraddress[i] != (char) 0 ) valid_hostid=TRUE;
        if (!valid_hostid) return FALSE;
        // next check if the last 6 digits are 0
        // this is to filter out PPP NDIS_WAN false addresses
        valid_hostid=0;
        for (i=3;i<6;i++) if (Adapteraddress[i] != (char) 0 ) valid_hostid=TRUE;
        if (!valid_hostid) return FALSE;


	// Check to see if this is a loopback adapter
	// or " LOOP "

	if ( Adapteraddress[0]==' ' && 
		 Adapteraddress[1]=='L' && 
		 Adapteraddress[2]=='O' && 
		 Adapteraddress[3]=='O' && 
		 Adapteraddress[4]=='P' &&
		 Adapteraddress[5]==' ' )
			return FALSE;

 

        if (!(*NumEtherEntries)) // no entries yet
        {       
         // put first into list
                memcpy(etherlist[0].addr, Adapteraddress,6);
                (*NumEtherEntries)++;
                return TRUE;
         }
        // next check if this is a repeated ethernet address

        for (i=0;i<  (*NumEtherEntries);i++)
        {
                if (!memcmp( etherlist[i].addr,Adapteraddress,6))
                { // they are equal ignore this one
                        return FALSE;
        
                }
        
        }               // this is a new one, insert into list
        memcpy(etherlist[(*NumEtherEntries)].addr, Adapteraddress,6);
        (*NumEtherEntries)++;
        return TRUE;

        

}

// etherlist[i].addr  gives 6 chars
// etherlist[i].addr[j] gives jth byte of addrress

BOOL get_netbios_ether_addr1(LM_HANDLE * lm_job)
{
eAddress_t etherlist[10];
int NumEtherEntries=0;
NCB ncb;
int i,j;
unsigned char LanEnum[30];
int first;
HOSTID_PTR newidptr,current=lm_job->idptr;
       


// now find the first valid adapter, get the 
// adapter status name 
        memset( &ncb, 0, sizeof(NCB) );

        ncb.ncb_command = NCBENUM;
        ncb.ncb_lana_num=0;
        ncb.ncb_buffer= (unsigned char *)&LanEnum     ;
        ncb.ncb_length= sizeof(LanEnum);

        i=Netbios(&ncb);
        
for ( j=0;j<LanEnum[0];j++)
  {

        memset( &ncb, 0, sizeof(NCB) );

        ncb.ncb_command = NCBRESET;
        ncb.ncb_lana_num=LanEnum[j+1];
        i=Netbios(&ncb);

        memset( &Adapter, 0, sizeof(Adapter) );
        memset( &ncb, 0, sizeof(NCB) );

        ncb.ncb_command = NCBASTAT;
        ncb.ncb_buffer= (unsigned char *)&Adapter     ;
        ncb.ncb_length= sizeof(Adapter);
        ncb.ncb_lana_num=LanEnum[j+1];
                        /* Get COmmands OP code */
        strcpy(ncb.ncb_callname,"*               ");

        
        if ( Netbios(&ncb) )
        {
                break;  //return FALSE;
        }

        if (ncb.ncb_retcode != NRC_GOODRET)
                break;   // Error!!

        
        check_new_adapter( etherlist,&NumEtherEntries,
                                Adapter.Adapt.adapter_address) ;

  }

// Now copy address from structure 
  
        first=1;
        for (i=0 ; i< NumEtherEntries;i++)
        {
                if (first)
                {
                        first=0;
                        memcpy(lm_job->idptr->id.e ,etherlist[i].addr,6);
                        lm_job->idptr->type = HOSTID_ETHER;
                        memcpy(EtherAddrs[EtherCount],etherlist[i].addr,6); EtherCount++;


                }
                else
                {
                        newidptr=l_new_hostid();
                        newidptr->type =HOSTID_ETHER;
                        current->next=newidptr;
                        current=newidptr;                       
                        memcpy(current->id.e ,etherlist[i].addr,6); 
                        memcpy(EtherAddrs[EtherCount],etherlist[i].addr,6); EtherCount++;

                }

        }

        if (NumEtherEntries)
                return TRUE;
        else    
                return FALSE;

}
