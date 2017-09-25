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
 *      Module: $Id: winflex.c,v 1.11.4.1 2003/06/30 23:55:20 sluu Exp $
 *      
 *
 *      Function: misc.
 *
 *      Description: temporily resolve unresolved externals for building DLL.
 *
 *      Chia-Chee Kuan 
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lm_lsapi.h"
#include <stdio.h>
#include <stdarg.h> 
#include <string.h>
#include <winsock.h>
#include <windows.h> 
#include <process.h>
extern HINSTANCE hInst  ;
int FLEXLM_type;                         
HANDLE hInstance;
FILE * our_std_error=0;

typedef struct _ThreadData {
        HANDLE hTimerEvent;
} ThreadData;

HANDLE hTimerThread = NULL;
DWORD TimerInterval;
TIMERPROC TimerProc;
HANDLE hTimerEvent;
long * GlobalStrkey=0;
extern void * free_list;
extern HINSTANCE winsock_hInst;
DWORD MainThreadId;
int lm_timers_in_use=0;




#if !defined (FLEX_STATIC)
BOOL WINAPI _CRT_INIT( HANDLE hModule, DWORD dwReason, LPVOID lpReserved );
void KillThread_NT( HWND unused1, UINT TimerThread );

BOOL _stdcall LibMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved )
{   
        extern int _putenv( const char *env );
        /* _CRT_INIT( hModule, dwReason, lpReserved );  */
        switch ( dwReason )
        {
                case DLL_THREAD_ATTACH:
                        break;
                case DLL_PROCESS_ATTACH:
                        _CRT_INIT( hModule, dwReason, lpReserved );     
                        break;
                case DLL_THREAD_DETACH:
                        break;

                case DLL_PROCESS_DETACH:

                        _CRT_INIT( hModule, dwReason, lpReserved );
                        break;
        }
        
        return 1;
}
#endif /* !FLEX_STATIC */
      


 
lm_debug_printf(const char *fmt,... )
{
        int rc;
        char buff_in_dll_data_seg[512];

        va_list marker;
        
        va_start(marker, fmt);
        rc=vsprintf(buff_in_dll_data_seg,fmt,marker);
        va_end( marker );

        if (our_std_error) fprintf(our_std_error,"%s",buff_in_dll_data_seg);
        return strlen( buff_in_dll_data_seg );
}


int API_ENTRY 
lc_sleep(job, seconds)
LM_HANDLE *job;         /* Current license job */
int seconds;
{
        Sleep(seconds*1000);
        return 1;
}



lm_debug_puts( const char *s )
{
        if (our_std_error) 
        {
                fprintf(our_std_error,"%s",s);
                fflush(our_std_error);
        }
        return strlen( s );
}


// This is needed for testsuit.c.  On NT, application's _putenv() has
// no effect on a DLL.  Therefore, application need to call lc_putenv()
// if it expects lmgr32.dll to use that environment variable.

API_ENTRY l_putenv(char far *str)
{
        return _putenv(str);
}


/*
 * Note about this timer stuff:
 *
 *  It's about half way between being a single instance timer
 *  and a multiple instance timer.  So, eventhough you can only
 *  have one timer going, there are things like, "ThreadData"
 *  that are the start of being able to have multiple timers.
 */
void lc_cleanup_internal(void)
{
        if ( winsock_hInst )
        {
              FreeLibrary( winsock_hInst );
              winsock_hInst = 0;
        }
        
        if (hTimerThread)
        {
                SetTimer_NT( NULL, 0, ( DWORD )  1 , TimerProc );
                Sleep( 0 );
		/* P6423 */
		WaitForSingleObject(hTimerThread, INFINITE);
        }

        CloseHandle(hTimerEvent);
}

                
typedef unsigned long (LM_THREAD_FUNCPTR) (LPVOID x1);
unsigned long  TimerThreadProc(LPVOID inData)
{
        ThreadData tData = *((ThreadData*) inData);
        DWORD waitResult;
        BOOL opOK;
	puts("in TimerThreadProc");
        free(inData);
        while (1)
        {
/*
 *              Wait on the abort semaphore with a timeout
 *              of "TimerInterval."  If we time out, we
 *              execute the timer process.  If we do abort,
 *              just reset and wait again.  (The abort
 *              allows us to change the timeout interval.)
 */
                
                waitResult = WaitForSingleObject(
                        tData.hTimerEvent,
                        TimerInterval );
                if( WAIT_TIMEOUT == waitResult )
                {
/*
 *                      Only execute the process if we have one.
 */
                        if( TimerProc )
                                (*TimerProc)(0, 0, 0, 0);
                }
                else
                {
                        if (TimerInterval == 1)
                                break;

                        opOK = ResetEvent(
                                tData.hTimerEvent );

                        if( !opOK )
                                break;
                }
        }
 
        _endthread();
        return 0;
}



UINT
SetTimer_NT( HWND unused1, UINT unused2, UINT msec, TIMERPROC tproc)
{
        ThreadData *tData;
        DWORD timerThreadId;



/*
 *      Save the input values and throw away the previous ones.
 *      (A list of "procs" will be needed if the DLL ever adds
 *      more than one timer process.)
 */
        if (msec)
                TimerInterval = msec;
        else
                TimerInterval = INFINITE ;

        TimerProc = tproc;

        if ( NULL != hTimerThread )
        {
/*
 *              The thread has already been created, so just abort it now
 *              to cause it to restart it's timer.
 */
                SetEvent( hTimerEvent );
        }
        else
        {
/*
 *              Create the semaphore we will use to abort
 *              the timer threads wait.
 */
                hTimerEvent = CreateEvent(
                                NULL,    // Use our Security
                                TRUE,    // Manual reset
                                FALSE,   // Not signaled initially
                                NULL );



                if( NULL == hTimerEvent )
                        return 0;
/*
 *              Initialize the data passed to the thread.  (Note
 *              that we allocate some memory to be used by the
 *              thread rather than reference anything locally.  This
 *              prevents data-context problems.)
 */
                tData = (ThreadData*) calloc( 1, sizeof(ThreadData) );
                if( !tData ) return 0;

                tData->hTimerEvent = hTimerEvent;
/*
 *              Create the thread that will call the timer
 *              process periodically.
 */

		// dynamic-link library for which calls are to be disabled
		hTimerThread = (HANDLE)_beginthread(
			TimerThreadProc,  	// lpStartAddress
			0,
			tData);


                Sleep(0);
                if(!hTimerThread) return 0;
        }
        return (UINT) hTimerThread;

}

void KillTimer_NT( HWND unused1, UINT TimerThread )
{
        SetTimer_NT( NULL, 0, INFINITE, TimerProc );
}

void KillThread_NT( HWND unused1, UINT TimerThread )
{ 
  DWORD result;
        return;
}


char * getenv1(name)
char * name;
{
static char * buf_ptr;
static char buf[200];
long errs; HKEY hkey;
DWORD val_type,buf_len;
#undef getenv
#define getenv getenv

 // first see if there is a corresponding environment variable set

        if (buf_ptr=getenv(name)) return buf_ptr;	/* overrun checked */

// if not see if it is defined in registry             
        errs=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                "SOFTWARE\\FLEXlm License Manager",
                                (DWORD)0,
                                KEY_QUERY_VALUE,
                                &hkey );

	if (errs == ERROR_SUCCESS)
        {
               	buf_len=199;
               	errs=RegQueryValueEx( hkey,name,(DWORD) 0,
				&val_type,buf,&buf_len);
                 
              
        	if (errs == ERROR_SUCCESS)
        	{
			buf_ptr=buf;
			RegCloseKey(hkey);
	       		return buf_ptr;
		}
		else 
		{
			RegCloseKey(hkey);
			return (char *) NULL;
		}
	}

	return (char *) NULL;
}

