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
 *      Module: $Id: l_sap.c,v 1.3 2003/01/13 22:41:55 kmaclean Exp $
 *      Function:       l_sap(job), 
 *                      
 *
 *      Description:    Performs the equivalent of gethost by name for 
 *                              SPX
 *
 *      Parameters:     (char *) Host Name - Name of computer to find info
 *                      (char *)SpxAddress -- char[10] buffer to put SPX address
 *                                              into
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
#include "lmachdep.h"
#ifdef WINNT
#include "pcsock.h"
#include <svcguid.h>
#endif

#ifdef NLM
#include <nwipxspx.h>
#endif

unsigned char Spx[10];

 int l_sap(char * HostName, char * SpxAddress)
 {
#ifdef NLM
        int ret;
        ret = IpxGetInternetworkAddress(SpxAddress);
        return !ret;


#endif /*NLM */

#ifdef WINNT
 CSADDR_INFO  csa[13];
 DWORD csalength=13*sizeof(CSADDR_INFO);
 DWORD mylength=256;
 DWORD bytesRequired;
 

 PPROTOCOL_INFO protocolInfo;

 int protocolCount;
 
 int i,nRet;
 DWORD protocolIndex;
 int protocols[256];
 unsigned char buffer[2048];
 GUID guid = SVCID_FILE_SERVER     ; //(NW) SVCID_FILE_SERVER",
        bytesRequired = sizeof(buffer);

    nRet = EnumProtocols( NULL, buffer, &bytesRequired );
    if ( nRet <= 0 ) {
        return 0;
    }

       //
    // Walk through the available protocols and pick out the ones which
    // support the desired characteristics.
    //

    protocolCount = nRet;
    protocolInfo = (PPROTOCOL_INFO)buffer;

    for ( i = 0, protocolIndex = 0;
          i < protocolCount && protocolIndex < 255;
          i++, protocolInfo++ ) {

        //
        // If "reliable" support is requested, then check if supported
        // by this protocol.  Reliable support means that the protocol
        // guarantees delivery of data in the order in which it is sent.
        // Note that we assume here that if the caller requested reliable
        // service then they do not want a connectionless protocol.
        //

           if ( protocolInfo->iAddressFamily  != 6 ) 
                continue;
          

        //
        // This protocol fits all the criteria.  Add it to the list of
        // protocols in which we're interested.
        //

        protocols[protocolIndex++] = protocolInfo->iProtocol;
    }

    //
    // Make sure that we found at least one acceptable protocol.  If
    // there no protocols on this machine which meet the caller's
    // requirements then fail here.
    //

    if ( protocolIndex == 0 ) {
        return 0;
    }

    protocols[protocolIndex] = 0;

    nRet = GetAddressByName(
          NS_DEFAULT,     // name space to query for service address 
                         // information
        &guid,  // the type of the service
        "gsi",  // the name of the service
        protocols,    // points to array of protocol identifiers
        RES_FIND_MULTIPLE,    // set of bit flags that specify aspects of 
                         // name resolution
          NULL,
                         // reserved for future use, must be NULL
        &csa, // points to buffer to receive address 
                         // information
        &csalength,  // points to variable with address 
                             // buffer size information
        NULL,  // points to buffer to receive alias 
                         // information
        NULL 
                         // points to variable with alias buffer 
                         // size information
        );

        if (nRet > 0 )
        {
                memcpy(SpxAddress,csa[0].RemoteAddr.lpSockaddr->sa_data,10);    
                 return 1;
        }
        else
        {
                   return 0;
        }       

#endif

 }
