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
 *	
 *	Module: $Id: l_icmp.c,v 1.5 2003/01/13 22:41:52 kmaclean Exp $
 *	Function: l_icmp.c
 *
 *	Description: Provides Ping functions using the microsoft ICMP dll
 *			
 *	Blane Eisenberg
 *	7/9/97
 *
 *	
 *
 */
#include "lmachdep.h"
#include <pcsock.h>	 				// Winsock header file
#include "ipexport.h"
#include "icmpapi.h"
#define WINSOCK_VERSION 0x0101		// Program requires Winsock version 1.1
#define NO_FLAGS 0								// No special flags specified
HANDLE WINAPI l__IcmpCreateFile(     VOID     );
BOOL WINAPI l__IcmpCloseHandle(     HANDLE  IcmpHandle     );
DWORD
WINAPI
l__IcmpSendEcho(
    HANDLE                   IcmpHandle,
    IPAddr                   DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    );


#define IcmpCreateFile  l__IcmpCreateFile 
#define IcmpCloseHandle  l__IcmpCloseHandle
#define IcmpSendEcho   l__IcmpSendEcho

char szPingBuffer[512];						// General purpose buffer for messages
HANDLE myICMPhandle;

BOOL l_do_ping_operation(char * hostname,char ** msgbuf)
{
  LPSTR lpszHostName;	// A pointer to the time server host
  SOCKADDR_IN	sockAddrLocal;		// Socket address structures
  WSADATA wsaData;							// Winsock implementation details
  int status,i,RequestBufferLength; 
  DWORD multistatus,ReplyBufferLength,timeout;
  char RequestBuffer[128],ReplyBuffer[128];
  PIP_OPTION_INFORMATION   RequestOptions;
  LPHOSTENT lpHostEntry;				// Internet host data structure
  IP_OPTION_INFORMATION ip_opt;
	RequestOptions=&ip_opt;	
	*msgbuf=	&(szPingBuffer[0]);
	// Start the program here
	
	
	status=WSAStartup(WINSOCK_VERSION, &wsaData);
	if (status)
	{
		wsprintf(szPingBuffer, "The Networking Software (WINSOCK) did not respond.\nIt failed with a code of %d  .\nConsult your System Administrator for assistance",
			status);
		return(FALSE);

	
	}
	lpszHostName = hostname;

	if ((lpHostEntry = gethostbyname(hostname)) == NULL)
	{
		wsprintf(szPingBuffer, "The Networking Software (WINSOCK) could not obtain the IP address of the computer \"%s\" .\n\nThis could be due to \"%s\" not being a valid computer name, or the DNS Name Resolution Service\n is not working or not configured properly.\n\nYou may need to include \"%s\" in your \"hosts\" file.\nSee your System Administrator for assistance",
					(LPSTR)lpszHostName,(LPSTR)lpszHostName,(LPSTR)lpszHostName);
		return(FALSE);
	}

	sockAddrLocal.sin_family = AF_INET;
	sockAddrLocal.sin_addr = *((LPIN_ADDR) *lpHostEntry->h_addr_list);
 	RequestBufferLength=32;
	ReplyBufferLength=128;
	timeout=5000;
	RequestOptions->Ttl=32;
	RequestOptions->Tos=0;
	RequestOptions->Flags=0x0;
	RequestOptions->OptionsSize=0;
	RequestOptions->OptionsData=NULL;

	for (i=0;i<32;i++)
	{
		RequestBuffer[i]=i+0x41;
	}

	myICMPhandle=IcmpCreateFile();
	if (INVALID_HANDLE_VALUE==myICMPhandle)
	{
		wsprintf(szPingBuffer, "The Networking Software (WINSOCK) and ICMP are not configured correctly.\n Check configuration, or reinstall networking software.\nSee your System Administrator for assistance");		

		return(FALSE);
	}


	multistatus=IcmpSendEcho( myICMPhandle,	
                sockAddrLocal.sin_addr.s_addr,	RequestBuffer,
                RequestBufferLength, RequestOptions, ReplyBuffer,
                ReplyBufferLength, timeout);

   	if (!multistatus)
	{	
		status=GetLastError();
		wsprintf(szPingBuffer,
				"Server (\"%s\") did not respond within 5 seconds,\nthis server may not exist, or communications are not possible at this time.\nSee your System Administrator for assistance",hostname);
		return(FALSE);
	}

	status=0;
	for (i=0;i<32;i++)
	{
		if (ReplyBuffer[28+i]!=(i+0x41)) status=1;
	}
	if (status)
	{
  		wsprintf(szPingBuffer, "The License Server returned a networking error.\nSee your System Administrator for assistance");
		return(FALSE);
	}

	status=IcmpCloseHandle(myICMPhandle);
	if  (!status)
	{
  		wsprintf(szPingBuffer, "The Networking Software (ICMP could not close down the connection to \"%s\".\n\nThis could be due to \"%s\" not being a valid computer name,\nor the DNS Name Resolution Service not working or configured properly.\n\nYou may need to include this name in your \"hosts\" file.\nSee your System Administrator for assistance",
					(LPSTR)lpszHostName,(LPSTR)lpszHostName);
		return(FALSE);


	}
	WSACleanup();
	return TRUE;

}

