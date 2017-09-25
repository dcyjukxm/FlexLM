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
 *  	Module: $Id: l_ntadd.c,v 1.10 2003/02/13 02:07:18 jwong Exp $
 *      Function: getNTAdd.c
 *
 *      Description: This module is used to get ethernet MAC number from NT
 *      
 *      
 *      
 *      
 *      
 *      
 *
 *      Parameters:     Pointer to LM_JOB_HANDLE
 *
 *      Return:         (bool) - true if successful, false otherwise
 *
 *      Blane Eisenberg
 *      7/14/95
 *      Last changed:  9/27/98
 *
 *
 */
#include <lmachdep.h>
#include "lmclient.h"
#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmserver.h>
#include <lmwksta.h>

#include <ipexport.h>
#include "gsi_iptypes.h"	// type defs for IPHELPER
#include "gsi_ipifcons.h"	// defines for IPHELPER
#include <winioctl.h>	// platform SDK header for IPHELPER
#include "ntddndis.h"	// This defines the IOCTL constants.

#include <string.h>
typedef struct foo11 {
        unsigned char addr[6];
                                        }  eAddress_t;

// multiple device struct
//
typedef struct{
	unsigned char szDeviceAddr[6];		// limit to 10 devices of 32 characters in length
}DEVICESPRESENT;
DEVICESPRESENT devicesPresent[10];
int iDeviceCount;


typedef DWORD (WINAPI * NET_WORKSTATION_ENUM) 
                                        (LPSTR , DWORD, LPBYTE *, DWORD, LPDWORD,
                                         LPDWORD, LPDWORD);


#define NET_WORKSTATION_ENUM_TYPE (NET_WORKSTATION_ENUM)


   
typedef    NET_API_STATUS  (WINAPI *NET_API_BUFFER_FREE  )
                                                (LPVOID);
#define NET_API_BUFFER_FREE_TYPE (NET_API_BUFFER_FREE )

extern BOOL check_new_adapter( eAddress_t etherlist[10], int * NumEtherEntries,
                                unsigned char * Adapteraddress);
typedef  unsigned char ETHERARRAY[6];
extern ETHERARRAY EtherAddrs[5];
extern int EtherCount;

/*
	Device Discovery
*/

// IPHLPAPI.DLL prototype
typedef DWORD (WINAPI * GETNETWORKPARAMS)(PFIXED_INFO, PULONG);
typedef DWORD (WINAPI * GETADAPTERINFO)(PIP_ADAPTER_INFO, PULONG);


#define DEVICE_PREFIX   "\\\\.\\"
#define PERFENUMKEY "Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"

int NTDriverDiscover(char *szDeviceName)
{
   char szObject[MAX_PATH];
   LPSTR lpszDescBuff = NULL;
   DWORD rc;
   DWORD dwIndex1;
   DWORD dwType;
   DWORD dwBufSize;
   HKEY hKey;
   HKEY hObject;

/*
	Manual discovery method will read the NT registry 
	for the PERFENUMKEY value located at networkcards 
	location "0". This method is only reliable under 
	NT 4.0 registry structure. There is no guarantee 
	from MS that this method will work under NT 5.0 
	or greater reliabily. MS recommends the use of 
	the PLATFORM SDK IPHELPER.DLL in order to reliablily 
	detect a MAC address under NT 5.0 or above.
*/

   rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PERFENUMKEY, 0, KEY_READ, &hKey);
   if (rc == ERROR_SUCCESS)
   {
     dwBufSize = MAX_PATH;
     dwIndex1 = 0;

     // enumerate objects
     while ( RegEnumKeyEx(hKey, dwIndex1++, szObject, &dwBufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS )
     {
        if ( (rc=RegOpenKeyEx(hKey, szObject, 0, KEY_READ, &hObject)) == ERROR_SUCCESS)
        {
			// process information on the open key using hObject
			dwBufSize = 0;
			rc = RegQueryValueEx(hObject, "ServiceName", NULL, &dwType, NULL, &dwBufSize);
			// allocate memory for data
			lpszDescBuff = (LPSTR)malloc(dwBufSize + 1);
			rc = RegQueryValueEx(hObject, "ServiceName", NULL, &dwType, lpszDescBuff, &dwBufSize);
			// GOT IT! save return string
			if (lpszDescBuff != NULL)
				strcpy(szDeviceName, lpszDescBuff);
			// clean up memory
			free(lpszDescBuff);
        }
        RegCloseKey(hObject);
        dwBufSize = MAX_PATH;
     }
     RegCloseKey(hKey);
   }
   else
   {
     return -255;
   }
  return 0;
}

/*
	NT method for MAC address detection
*/
static GETNETWORKPARAMS GSIGetNetworkParams = NULL;
static GETADAPTERINFO	GSIGetAdaptersInfo = NULL;
static BOOL				hIphlpapiInit = FALSE;
/*
 *	The handle to this library will be leaked because we never get around to calling FreeLibrary on it.
 *	This should not be a problem because this should only happen 1 time as LoadLibrary is called only
 *	once.
 */
static HANDLE			hIphlapi = NULL;
#define IPHELP_MUTEX_NAME	"IPHelperMutex"


/*
 *	This function is never called because we currently do not have a sdk cleanup routine.
 */
static
void
sCleanupIphl(void)
{
	if(hIphlapi)
	{
		FreeLibrary(hIphlapi);
		hIphlapi = NULL;
	}
}

static
int
sInitIphl(void)
{
	char	szMutexName[MAX_PATH] = {'\0'};
	HANDLE	hIphlpapiMutex = NULL;
	int		rv = 0;

	if(!hIphlpapiInit)
	{
		sprintf(szMutexName, "%s_%d", IPHELP_MUTEX_NAME, getpid());
		hIphlpapiMutex = CreateMutex(NULL, 1, szMutexName);
		if(hIphlpapiMutex == NULL)
		{
			return 0;
		}
		else
		{
			if(GetLastError() == ERROR_ALREADY_EXISTS)
			{
				WaitForSingleObject(hIphlpapiMutex, INFINITE);
			}
			if(!hIphlpapiInit)
			{
				/* 
				 *	This chunk of code should only be executed 1 time, by the first
				 *	thread that get's there.
				 */
				hIphlapi = LoadLibrary("iphlpapi.dll");
				if(hIphlapi == NULL)
				{
					rv = -2;
				}
				else
				{
					GSIGetNetworkParams = (GETNETWORKPARAMS)GetProcAddress(hIphlapi,"GetNetworkParams");
					if(GSIGetNetworkParams)
					{
						GSIGetAdaptersInfo = (GETADAPTERINFO)GetProcAddress(hIphlapi,"GetAdaptersInfo");
					}
				}
				hIphlpapiInit = 1;
			}
			ReleaseMutex(hIphlpapiMutex);
			CloseHandle(hIphlpapiMutex);
		}
	}	
	return rv;
}


int
l_getntaddr(int osver, char e_addr[13])
{
	UCHAR				szDriverName[256];
	DWORD				ErrorNumber = 0;
	DWORD				ReturnedCount = 0;
	PIP_ADAPTER_INFO	pAdapterInfo = NULL,
						pAdapt = NULL;
	DWORD				AdapterInfoSize = 0;
	PIP_ADDR_STRING		pAddrStr = NULL;
	DWORD				Err;
	PFIXED_INFO			pFixedInfo = NULL;
	UCHAR				szTemp[32];
	DWORD				FixedInfoSize = 0;
	int					iReturn = 0,
						i = 0,
						RemovedDevices = 0,
						iLoop = 0;


/**********************************************************************************

	This code uses DEVICEIOCTRL calls with a ring 3 application
	context. Since detection of the device name differs between 
	NT 4 and NT5, two methods are being employed. In NT4, a
	manual device name discovery method is used but looking in
	the registry for the device name. In NT5, MS recommends using
	the IPHELPER.DLL library from the Platform SDK. The registry
	method used for NT4 may work under NT5, but it is not recommended
	since MS may change the entry order.
	ADDITIONAL NOTES:

	12/1/2000: 	This code change will require the present of the IPHelper DLL 
				in order to function. The code will also eliminate the LOOPBACK,
				PPP, and SLIP generated MAC addresses from appearing in the valid
				HOSTID list.
	2/9/2001:	NT4 section is not used. The code was reverted to the original 
				because of MS bugs with iphptapi.dll calls under NT4.

***********************************************************************************/

  
	RemovedDevices = 0;
    memset(e_addr, 0, 13);	/*	initialize ethernet address info	*/

    if (osver == 4)
	{
		/*
		 *	use NT 4.0 driver name detection method (NOT USED AT THIS TIME!!)
		 */
		iReturn = NTDriverDiscover(szDriverName);
		if (iReturn != 0)
		{
			return iReturn;
		}
		/*	check for bad strings	*/
		if (szDriverName == NULL)
			return -255;
		iDeviceCount= 1;		// **** fix this for multile NICs for NT4 later
		/*	NOTE: multiple NICs do not work under NT4 yet.	*/
    }
    else
	{
		if(!hIphlpapiInit)
		{
			iReturn = sInitIphl();
		}

		if(GSIGetNetworkParams && GSIGetAdaptersInfo)
		{
			/*
			 *	use IPHELPER.DLL to detect device driver name
			 */
			if((Err = (*GSIGetNetworkParams)(NULL, &FixedInfoSize)) != 0)
			{
				if (Err != ERROR_BUFFER_OVERFLOW)
				{
					iReturn = (int)Err;
					goto done;
				}
			}
			/*
			 *	Allocate memory from sizing information
			 */
			if((pFixedInfo = (PFIXED_INFO)malloc(FixedInfoSize)) == NULL)
			{
				iReturn = -255;
				goto done;
			}
			if((Err = (*GSIGetNetworkParams)(pFixedInfo, &FixedInfoSize)) == 0)
			{
				pAddrStr = pFixedInfo->DnsServerList.Next;
				while(pAddrStr)
				{
					pAddrStr = pAddrStr->Next;
				}
		
			}
		
			/*
			 *	Get adaptor name
			 */
			AdapterInfoSize = 0;
			Err = (*GSIGetAdaptersInfo)(NULL, &AdapterInfoSize);
			if(Err != 0)
			{
				if (Err != ERROR_BUFFER_OVERFLOW)
				{
					iReturn = (int)Err;
					goto done;
				}
			}
			/*
			 *	Allocate memory from sizing information
			 */
			if((pAdapterInfo = (PIP_ADAPTER_INFO) malloc(AdapterInfoSize)) == NULL)
			{
				iReturn = -255;
				goto done;
			}
			/*
			 *	Get actual adapter information
			 */
			if ((Err = (* GSIGetAdaptersInfo )(pAdapterInfo, &AdapterInfoSize)) != 0)
			{
				iReturn = (int)Err;
				goto done;
			}
			pAdapt = pAdapterInfo;
			strcpy(szDriverName, pAdapt->AdapterName);

			/*	determine multiple NICs	*/
			iLoop = 0;
			while (pAdapt)
			{				  
 				if ((pAdapt->Type  == MIB_IF_TYPE_PPP) || 
 					(pAdapt->Type  == MIB_IF_TYPE_SLIP)||	
 					(strcmp(pAdapt->Description, "MS LoopBack Driver") == 0))
				{	/*	don't count this one */
					RemovedDevices++; 		/*	basically do nothing	*/
				}
				else
				{	/*	got it!	*/
					for (i=0; i < (int)pAdapt->AddressLength; i++)
					{
					  devicesPresent[iLoop].szDeviceAddr[i] = (int)pAdapt->Address[i];
 					}
 					iLoop++;
				}
				pAdapt = pAdapt->Next;
			}	/*	end while	*/
			iDeviceCount = iLoop;	/*	save the device count	*/
		}

	}
done:
	if(pFixedInfo)
	{
		free(pFixedInfo);
		pFixedInfo = NULL;
	}
	if(pAdapterInfo)
	{
		free(pAdapterInfo);
		pAdapterInfo = NULL;
	}
    return iReturn;

}


BOOL l_ntadd( LM_HANDLE * lm_job)
{

	int ReturnedByteCount, Status, osver;
	UCHAR Address[6];
	long dwError;

	HANDLE        hNetApi = 0;
	DWORD EntriesRead,ReturnCode;
	DWORD TotalEntries;
	LPBYTE pBuffer;
	char * widebuff;        
	int i,j; 
	char ethercharbuf[13];
	char etherbuf[6];
	eAddress_t etherlist[10];
	int NumEtherEntries=0;
	int first, status;
	HOSTID_PTR newidptr,current=lm_job->idptr;

	NET_WORKSTATION_ENUM	NetWorkstationEnum    ;
	NET_API_BUFFER_FREE     net_api_buffer_free     ;

	PWKSTA_TRANSPORT_INFO_0 pWti0;

	DWORD dwVersion, dwBuild;
	DWORD dwWindowsMajorVersion, dwWindowsMinorVersion;
	BOOL fIsDLLPresent, fisNT4, fisME, fis95, fis98;
	HANDLE hIPHelpApiDLL = 0;


	fIsDLLPresent = FALSE;	// initialize DLL check var
	osver = 0;				// initialize OS version ID

	fisNT4 = FALSE;			// initialize OS Flag
	fisME = FALSE;
	fis95 = FALSE;
	fis98 = FALSE;
	
	//
	// OS DETECT
	//
	dwVersion = GetVersion();
	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 90))
		fisME = TRUE;
	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 90))
		fis98 = TRUE;
	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 0))
		fis95 = TRUE;

	//
	// Get the build number for Windows NT/Windows 2000 or Win32s.
	//
	if (dwVersion < 0x80000000)				// Windows NT/2000
	    dwBuild = (DWORD)(HIWORD(dwVersion));
	else if (dwWindowsMajorVersion < 4)		// Win32s
	    dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
	else									// Windows 95/98 -- No build number
	    dwBuild =  0;

	if (dwBuild <= 1381)
	{
		fisNT4 = TRUE;	// NT 4.0 behavior
	}
	
	// add this to improve checking
	// works with 98, ME, NT5
	//
	if(!hIphlpapiInit)
	{
		sInitIphl();
	}

	if(GSIGetNetworkParams && GSIGetAdaptersInfo)
		fIsDLLPresent = TRUE;

	if ((fIsDLLPresent) && (fis95 == TRUE))
	{
			fisNT4 = TRUE;		// force NT4 bahavior
	}
	if ((fIsDLLPresent) && (fis98 == TRUE))
	{
			fisNT4 = FALSE;		// force W2K bahavior
	}
	if ((fIsDLLPresent) && (fisME == TRUE))
	{
			fisNT4 = FALSE;		// force W2K bahavior
	}

	// for NT machines.	
	if ((fIsDLLPresent) && (fisNT4 == FALSE))
	{
	// use IP helper DLL to detect NIC card host IDs
	//
		osver = 5;													// hardcoded for now
	  	status = l_getntaddr(osver, ethercharbuf);					// use NT method
		memcpy(etherlist, devicesPresent, sizeof(devicesPresent));	// copy NT struct into blanes struct
		NumEtherEntries = iDeviceCount;								// save NT method device count found
	}
	else
	{
		//
		// use NETBIOS method (if iphelperdll not found)
		//
		hNetApi = LoadLibrary("NETAPI32.Dll" );
		if (hNetApi == NULL )
		{
			return -1; // Could not find Netapi32.dll
		} 
	
		NetWorkstationEnum =(NET_WORKSTATION_ENUM )GetProcAddress(hNetApi,"NetWkstaTransportEnum");
		net_api_buffer_free = (NET_API_BUFFER_FREE)GetProcAddress(hNetApi,"NetApiBufferFree");
	    if ( !NetWorkstationEnum || !net_api_buffer_free)
		{
			return -1; // Could not find procedures in  Netapi32.dll
		} 
		//
		// Enumerate the transports managed by the server
		//
		ReturnCode = (* NetWorkstationEnum )(NULL,
                      0,
	                  & pBuffer,
	                  4096,         // MaxPreferredLength
	                  & EntriesRead,
	                  & TotalEntries,
	                  NULL);       // Optional resume handle
	    if (ReturnCode != 0)
	    {
	    //
	    // Couldn't enumerate the nets, return with an error
        //
	    	FreeLibrary( hNetApi );
	        return -1;
	    }

        // Now we've got the network names
       	//
		for (i = 0, pWti0 = (PWKSTA_TRANSPORT_INFO_0) pBuffer;
			i < EntriesRead; i++, pWti0++) 
		{
			// First convert from wide to single byte ascii encoded
			widebuff=pWti0->wkti0_transport_address;        
			for (j=0;j<12;j++)
			{
				ethercharbuf[j]=*(widebuff+ j*2  );
			}                              
			// Next convert to binary Ethernet address
			for (j=0; j<6 ; j++)
			{
				etherbuf[j]=16*(isdigit(ethercharbuf[j*2]) ? ethercharbuf[j*2]-0x30  : ethercharbuf[j*2]- 55)+
				(isdigit(ethercharbuf[j*2+1]) ? ethercharbuf[j*2+1]-0x30  : ethercharbuf[j*2+1]- 55) ;
			}
		check_new_adapter( etherlist,&NumEtherEntries,etherbuf) ;
		}

		// clean up 
	    ( * net_api_buffer_free  )(pBuffer);
	    FreeLibrary( hNetApi );
	} // end else
	
	
	/* got it!! */

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
    // Free up the buffer allocated by NetxTransportEnum
    //
    if (NumEtherEntries)
      return TRUE;
    else    
      return FALSE;
 }


