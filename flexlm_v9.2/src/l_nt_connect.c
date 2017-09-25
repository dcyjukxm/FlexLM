/******************************************************************************

            COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_nt_connect.c,v 1.2 2003/01/13 22:41:53 kmaclean Exp $
 *      Function:       nt2_connect(job,s,transport,node)
 *
 *      Description:    Connects to server at 'endpoint' on host 'node'
 *                      transport is LM_TCP or LM_UDP or LM_SPX 
 *
 *      M. Christiano
 *      4/19/90
 *
 *	Last changed:  12/21/98
 *
 */
#include "winsock2.h" 
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

#define  WSACreateEvent l__WSACreateEvent
#define  WSAEventSelect l__WSAEventSelect
#define  WSAWaitForMultipleEvents l__WSAWaitForMultipleEvents
#define  WSAEnumNetworkEvents  l__WSAEnumNetworkEvents

WSAEVENT WINAPI l__WSACreateEvent(void);
int WINAPI l__WSAEventSelect (SOCKET s, WSAEVENT e, long x);
DWORD WINAPI l__WSAWaitForMultipleEvents ( DWORD cEvents, const WSAEVENT FAR * lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
int WINAPI  l__WSAEnumNetworkEvents ( SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
int PASCAL FAR l__connect (SOCKET s, const struct sockaddr FAR *name, int namelen);
int PASCAL FAR l__closesocket (SOCKET s);

void return_connect_error(s,job,node)
LM_SOCKET s;
LM_HANDLE * job;
char * node;

{
        LM_SET_ERROR(job, 
                LM_CANTCONNECT,
                10, NET_ECONNREFUSED,
                node, LM_ERRMASK_ALL);

        (void) l__closesocket(s);
        s = LM_BAD_SOCKET;
}                

#define RETURN_CONNECT_ERROR {return_connect_error(s,job,node); return FALSE;}

static int periods[] = { 20, 50, 100, 500, 9500 };
static int NumPeriods = 5;
int
nt2_connect(job,s,transport,node,sock,sock_ipx)
LM_HANDLE *job;
LM_SOCKET s ;
int transport;
char * node;
struct sockaddr_in sock;
struct sockaddr_ipx sock_ipx;    
{

int trycount;
WSANETWORKEVENTS networkEvents;
WSAEVENT event;
DWORD rval;

        if (WSA_INVALID_EVENT  == ( event = WSACreateEvent() )  )
                RETURN_CONNECT_ERROR
                
        if ( SOCKET_ERROR == WSAEventSelect( s, event, FD_CONNECT ))
                RETURN_CONNECT_ERROR
                
        for ( trycount = 0 ; trycount < NumPeriods ; trycount++ )
        {

                if ( l__connect(s,
                       (transport==LM_SPX)?(struct sockaddr *)&sock_ipx:
                                           (struct sockaddr *)&sock,
                       (transport==LM_SPX)?sizeof(sock_ipx):sizeof(sock) ) )
                {
                        switch (rval = WSAWaitForMultipleEvents( 1, &event, TRUE, periods[trycount], FALSE))
                        {
                                case WSA_WAIT_TIMEOUT:
                                case WSA_WAIT_EVENT_0:
                                        if (SOCKET_ERROR != WSAEnumNetworkEvents(s,event,
                                                (LPWSANETWORKEVENTS) & networkEvents))
                                        {

                                                if (( networkEvents.lNetworkEvents & FD_CONNECT) &&
                                                        (networkEvents.iErrorCode[FD_CONNECT_BIT] == 0 ) )
                                                        return TRUE;
                                                else
                                                if (networkEvents.iErrorCode[FD_CONNECT_BIT] == WSAECONNREFUSED )
                                                {
                                                        RETURN_CONNECT_ERROR      
                                                }
                                                break;
                                        }
                                        else
                                                RETURN_CONNECT_ERROR
                                default:
                                        RETURN_CONNECT_ERROR
                        }
                }
                else
                {        /*
                         *      Connect() major error.
                         */
                        RETURN_CONNECT_ERROR
                }

        }
    
        /*
         *      Connect() timeout!
         */
        (void) l__closesocket(s);
        s = LM_BAD_SOCKET;
        LM_SET_ERROR(job, LM_HOSTDOWN, 483,
                errno, node,
                LM_ERRMASK_ALL);
        return FALSE;               
}

