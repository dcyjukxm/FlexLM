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
 *      Module: $Id: l_sap2.c,v 1.3 2003/01/13 22:41:55 kmaclean Exp $
 *	
 *      Function:       l_sap2(job), 
 *                      
 *
 *      Description:    Performs the equivalent of gethost by name for 
 *				SPX  Uses winsock 2.0 functions
 *
 *      Parameters:     (char *) Host Name - Name of computer to find info
 *                      (char *)SpxAddress -- char[10] buffer to put SPX address
 *						into
 *                      
 *                          
 *
 *      Return:          True if success, False if not
 *
 *      NOTE:
 *
 *      B. Eisenberg
 *      2/13/98
 *
 *      Last changed:  2/13/98
 *
 */
/* #include "lmachdep.h" */
#if defined( _MSC_VER) && !defined(PC)
#define PC
#endif

#if defined( _WIN32) && !defined(WINNT)
#define WINNT
#endif

#ifdef WINNT
#include <winsock2.h>
#include "pcsock.h"
#include <svcguid.h>
#include <nspapi.h>
#endif

#ifdef NLM
#include <nwipxspx.h>
#endif

unsigned char Spx[10];

 int l_sap2(char * HostName, char * SpxAddress)
 {
    
 WSAQUERYSET qs;
 LPWSAQUERYSET pqs;
 LPCSADDR_INFO pcsa;
 DWORD dwFlags;
 int nRet;
 HANDLE hLookup;		     
 DWORD dwResultLen;
 LPWSAQUERYSETA pResultBuf ;
 GUID guid = SVCID_FILE_SERVER     ; //(NW) SVCID_FILE_SERVER"
 int iStop=0,dwNdx ;
   
	 memset(&qs, 0, sizeof(WSAQUERYSET));
	 qs.dwSize = sizeof(WSAQUERYSET);
	 qs.dwNameSpace = NS_ALL ;  //_SAP;



	 qs.lpServiceClassId = &guid;


	 //
	// Assign Service Name
	//
	qs.lpszServiceInstanceName = HostName; 
		
	
	//
	dwFlags = LUP_RETURN_NAME|LUP_RETURN_ADDR; 
	nRet = WSALookupServiceBegin(&qs,
					 dwFlags, 
					 &hLookup);
	if (nRet == SOCKET_ERROR)
	{

		return 0;
	}


	 dwResultLen = 1024;
	 pResultBuf = (LPWSAQUERYSET)malloc(dwResultLen);
 	
  	while(iStop < 1)
	{
		dwFlags = LUP_RETURN_NAME|LUP_RETURN_ADDR;
		nRet = WSALookupServiceNext(hLookup,
				dwFlags, 
				&dwResultLen,
				(LPWSAQUERYSET)pResultBuf);
		if (nRet == SOCKET_ERROR)
		{
			// Buffer too small?
			if (WSAGetLastError() == WSAEFAULT)
			{
				free(pResultBuf);
				pResultBuf = malloc (dwResultLen);
				continue;
			}
			if ((WSAGetLastError() != WSA_E_NO_MORE) || !iStop)
			{
				free(pResultBuf);
				return 0;
			}
		
		} 
		else
			iStop++;

	}
	//
	// Cast a pointer to the resulting WSAQUERYSET
	//

	pqs = (LPWSAQUERYSET)pResultBuf;
	
	//
	// and to the first CSADDR_INFO
	//

	pcsa = pqs->lpcsaBuffer;

	nRet = WSALookupServiceEnd(hLookup);
	memcpy(SpxAddress,pcsa->RemoteAddr.lpSockaddr->sa_data,10);
	free(pResultBuf);

	return TRUE;


 }
