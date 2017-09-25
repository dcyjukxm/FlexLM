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
 *      Module: $Id: service.c,v 1.21.2.3 2003/06/25 21:25:25 sluu Exp $
 *
 *      Function:       main() - main routine for lmgrd on Windows NT
 *
 *      Description:    Main program for the "master" license manager server.
 *
 *      Parameters:     None
 *
 *      Return:         None.
 *
 *      Blane Eisenberg / Chia-Chee Kuan
 *
 *
 *      Last changed:  12/17/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#include "flexevent.h"

#ifdef WINNT
  #include "windows.h"
  #include "l_service.h"
  #include "flex_utils.h"
#endif

#define SIGTERM        15      /* Software termination signal from kill */

/*
  This event is signalled when the
  lmgrd thread ends
*/
HANDLE                  hServDoneEvent = NULL;
SERVICE_STATUS          ssStatus;       /* current status of the service */
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwGlobalErr;
DWORD                   TID = 0;
HANDLE                  threadHandle = NULL;
static WORD             lmgrd_argc;
static char *  *lmgrd_argv;
/* drt: 05/21/03 was word changed to int */
int           run_lmgrd_as_service = TRUE;
int           run_lmgrd_as_applet  = FALSE;

extern char ServiceName[];
extern SERVICE_STATUS          GSIServiceStatus;
extern SERVICE_STATUS_HANDLE   GSIServiceStatusHandle;

FILE * debug_out;
static char  file_name[256];

/*
  declare the service threads:
*/
VOID    service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID    service_ctrl(DWORD dwCtrlCode);
BOOL    ReportStatusToSCMgr(DWORD dwCurrentState,
                            DWORD dwWin32ExitCode,
                            DWORD dwCheckPoint,
                            DWORD dwWaitHint);
VOID    StopSampleService(LPTSTR lpszMsg);
VOID    die(char *reason);
VOID    worker_thread(LPTSTR *lpszArgv);
VOID    StopFLEXlmService(LPTSTR lpszMsg);

/*****************************************
*   Module only partially used     *
******************************************/
/*  main() --
    all main does is call StartServiceCtrlDispatcher
    to register the main service thread.  When the
    API returns, the service has stopped, so exit.
*/
VOID
main(int argc, char *argv[], char *envp[])
{
    int   my_argc = 0;
    char ** my_argv = NULL;
    char  lmgrd_names[128] = {'\0'};
    char *  names_ptr[2] = { NULL, NULL };
    char *  ll = NULL;
    DWORD VersionNumber = 0;
  SERVICE_TABLE_ENTRY dispatchTable[] = {
        {TEXT("FLEXlm License Server"),(LPSERVICE_MAIN_FUNCTION)service_main },
        {NULL, NULL }
    };

  int i = 0;
  int iSize = 0;

  /*
   *	Initialize event log, if error, DON'T die, just keep running
   */
  l_flexEventLogInit(NULL, "lmgrd");

  /*
   *  Convert wide character argv to UTF8
   */
  my_argv = l_getUTF8Cmdline(NULL, &my_argc);
  if(my_argv == NULL)
  {
	fprintf(stderr, "LMGRD - initialization error, exiting.\n");
	fflush(stderr);
	exit(1);
  }

    /*
   *  First check if user wish to start lmgrd as an console app rather than
   *  a Service.  lmgrd with -app parameter will be started as an
   *  application.
   */
  lmgrd_argc = 1;
  strcpy(lmgrd_names, my_argv[0]);
  names_ptr[0] = lmgrd_names;
  lmgrd_argv=names_ptr;
#if 1
  VersionNumber = GetVersion();
  if (( VersionNumber >= 0x80000000 ) || (ll = l_real_getenv("USERNAME")) )
  {
    run_lmgrd_as_service = FALSE;
    main_service_thread(my_argc, my_argv);

  }
  if (!StartServiceCtrlDispatcher(dispatchTable))
  {
        run_lmgrd_as_service = FALSE;
        main_service_thread(my_argc, my_argv);
  }
#endif /* P4917 */
  if(my_argv)
  {
    l_freeUTF8Cmdline(my_argc, my_argv);
    my_argv = NULL;
  }
  l_flexEventLogCleanup();
}


/*****************************
*   Module Not Used    *
******************************/
/*  service_main() --
    this function takes care of actually starting the service,
    informing the service controller at each step along the way.
    After launching the worker thread, it waits on the event
    that the worker thread will signal at its termination.
*/
VOID
service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
    DWORD  dwWait = 0;

  strcpy(ServiceName,lpszArgv[0]);

  /* register our service control handler: */

    sshStatusHandle = RegisterServiceCtrlHandler(
                                    TEXT("FLEXlm License Server"),
                                    /* Wide_Service_Name, */
                                    (LPHANDLER_FUNCTION)service_ctrl);

    if (!sshStatusHandle)
        goto cleanup;

    /* SERVICE_STATUS members that don't change in example */
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;


    /* report the status to Service Control Manager. */
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, /* service state */
        NO_ERROR,              /* exit code */
        1,                     /* checkpoint */
        3000))                 /* wait hint */
  {
        goto cleanup;
  }

    /*
   *  create the event object. The control handler function signals
   *  this event when it receives the "stop" control code.
   */
    hServDoneEvent = CreateEvent(
                NULL,    /* no security attributes */
                TRUE,    /* manual reset event */
                FALSE,   /* not-signalled */
                NULL);   /* no name */

    if (hServDoneEvent == (HANDLE)NULL)
        goto cleanup;

    /* report the status to the service control manager. */
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, /* service state */
        NO_ERROR,              /* exit code */
        2,                     /* checkpoint */
        3000))                 /* wait hint */
  {
        goto cleanup;
  }

  /*
   *  start the thread that performs the work of the service.
   *  lmgrd_argc = (WORD)dwArgc;
   */
    threadHandle = _beginthreadex(
                    NULL,       /* security attributes */
                    0,          /* stack size (0 means inherit parent's stack size) */
                    (LPTHREAD_START_ROUTINE)worker_thread,
                    0,/* lpszArgv, argument to thread */
                    0,          /* thread creation flags */
                    &TID);      /* pointer to thread ID */

    if (!threadHandle)
        goto cleanup;

    /* report the status to the service control manager. */
    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING, /* service state */
        NO_ERROR,        /* exit code */
        0,               /* checkpoint */
        0))              /* wait hint */
  {
        goto cleanup;
  }

    /* wait indefinitely until hServDoneEvent is signaled. */
    while (1)
    {
    dwWait = WaitForSingleObject(hServDoneEvent,  /* event object */
                    INFINITE);    /* wait indefinitely */

    /* Kill FLEXlm lmgrd, which will then terminate all the vendor daemons. */
    if (ls_kill_chld(SIGTERM) != LM_BORROW_DOWN)
      break;
    }

cleanup:

    if (hServDoneEvent != NULL)
        CloseHandle(hServDoneEvent);
    if (threadHandle != NULL)
    {
        CloseHandle(threadHandle);
        threadHandle = NULL;
    }


    /* try to report the stopped status to the service control manager. */
    if (sshStatusHandle != 0)
  {
        (VOID)ReportStatusToSCMgr(
                            SERVICE_STOPPED,
                            dwGlobalErr,
                            0,
                            0);
  }

    /*  When SERVICE MAIN FUNCTION returns in a single service
    process, the StartServiceCtrlDispatcher function in
    the main thread returns, terminating the process.
    */
    return;
}


/*****************************
*   Module Not Used    *
******************************/

/*  service_ctrl() --
  this function is called by the Service Controller whenever
  someone calls ControlService in reference to our service.
*/
VOID
service_ctrl(DWORD dwCtrlCode)
{
    DWORD  dwState = SERVICE_RUNNING;

    /* Handle the requested control code. */
    switch(dwCtrlCode)
  {

        /* Pause the service if it is running. */
        case SERVICE_CONTROL_PAUSE:

            if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
                SuspendThread(threadHandle);
                dwState = SERVICE_PAUSED;
            }
            break;

        /* Resume the paused service. */
        case SERVICE_CONTROL_CONTINUE:

            if (ssStatus.dwCurrentState == SERVICE_PAUSED) {
                ResumeThread(threadHandle);
                dwState = SERVICE_RUNNING;
            }
            break;

        /* Stop the service. */
        case SERVICE_CONTROL_STOP:

            dwState = SERVICE_STOP_PENDING;

            /* Report the status, specifying the checkpoint and waithint, */
            /*  before setting the termination event. */
            ReportStatusToSCMgr(
                    SERVICE_STOP_PENDING, /* current state */
                    NO_ERROR,             /* exit code */
                    1,                    /* checkpoint */
                    3000);                /* waithint */

            SetEvent(hServDoneEvent);
            return;

        /* Update the service status. */
        case SERVICE_CONTROL_INTERROGATE:
            break;

        /* invalid control code */
        default:
            break;

    }

    /* send a status response. */
    ReportStatusToSCMgr(dwState, NO_ERROR, 0, 0);
}


/*****************************
*   Module Not Used    *
******************************/

/*
  worker_thread() --
    this function does the actual nuts and bolts work that
    the service requires.  It will also Pause or Stop when
    asked by the service_ctrl function.
*/
VOID
worker_thread(LPTSTR *lpszArgv)
{
    main_service_thread(lmgrd_argc, lmgrd_argv);
    return;
}



/* utility functions... */


/*****************************
*   Module Not Used    *
******************************/

/*
  ReportStatusToSCMgr() --
    This function is called by the ServMainFunc() and
    ServCtrlHandler() functions to update the service's status
    to the service control manager.
*/
BOOL
ReportStatusToSCMgr(DWORD dwCurrentState,
                    DWORD dwWin32ExitCode,
                    DWORD dwCheckPoint,
                    DWORD dwWaitHint)
{
    BOOL fResult;

  /* Disable control requests until the service is started. */
    if (dwCurrentState == SERVICE_START_PENDING)
        ssStatus.dwControlsAccepted = 0;
    else
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
            SERVICE_ACCEPT_PAUSE_CONTINUE;

    /* These SERVICE_STATUS members are set from parameters. */
    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwCheckPoint = dwCheckPoint;

    ssStatus.dwWaitHint = dwWaitHint;

    /* Report the status of the service to the service control manager. */
    if (!(fResult = SetServiceStatus(
                sshStatusHandle,    /* service reference handle */
                &ssStatus))) {      /* SERVICE_STATUS structure */

        /* If an error occurs, stop the service. */
        StopFLEXlmService("SetServiceStatus");
    }
    return fResult;
}


/*
  The StopFLEXlmService function can be used by any thread to report an
  error, or stop the service.
*/
VOID
StopFLEXlmService(LPTSTR lpszMsg)
{
    CHAR    chMsg[256] = {'\0'};
	CHAR *	ppszInStr[3] = {NULL};
    LPTSTR  lpszStrings[2] = {NULL, NULL};

    dwGlobalErr = GetLastError();

    sprintf(chMsg, "%d", dwGlobalErr);

    lpszStrings[0] = chMsg;
    lpszStrings[1] = lpszMsg;

	if(l_flexEventLogIsEnabled())
	{
		ppszInStr[0] = chMsg;
		ppszInStr[1] = lpszMsg;

		l_flexEventLogWrite(NULL,
							FLEXEVENT_ERROR,
							CAT_FLEXLM_LMGRD,
							MSG_FLEXLM_LMGRD_WIN32_SERVICE,
							2,
							ppszInStr,
							0,
							NULL);
	}

/*
    Set a termination event to stop SERVICE MAIN FUNCTION.

    SetEvent(hServDoneEvent);
*/
}

void getenv_service(char * ServiceName,char * name,char * buf_ptr,int buf_len)
{
	HKEY  hkey = NULL;
	char  registry[MAX_PATH] = {'\0'};
	int   val_type = 0;
	long  errs = 0;
	char *  pszUTF8Value = NULL;
	int   size = 0;

	if( (ServiceName == NULL) || (name == NULL) || (buf_ptr == NULL) || (buf_len == 0) )
		goto done;

	buf_ptr[0] = '\0';

	sprintf(registry,"SOFTWARE\\FLEXlm License Manager\\%s", ServiceName);  /* OVERRUN */
	// if not see if it is defined in registry
	errs = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registry, (DWORD)0,
			  KEY_ALL_ACCESS, &hkey );

	if(errs == ERROR_SUCCESS)
	{
		pszUTF8Value = l_regQueryValue(NULL, hkey, name, &val_type, &size);
		if(pszUTF8Value)
		{
			strncpy(buf_ptr, pszUTF8Value, size > buf_len ? buf_len : size);
			l_free(pszUTF8Value);
			pszUTF8Value = NULL;
		}
	}

done:
	if(hkey)
		RegCloseKey(hkey);
	return ;


}

/*
 *  Callback routine for service control manager startup from LMGRD.
 *  see l_m_init.c for handle initialization.
 *
 *  Input:
 *    dwCtrlCode -  SCM control code.
 *
 *  Return:
 *    no return
 */

void Process_Service_Message(DWORD dwCtrlCode)
{
  BOOL fReturn = FALSE;
  char szErrorStr[MAX_PATH] = {'\0'};

  switch (dwCtrlCode)
  {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
      GSIServiceStatus.dwCurrentState            = SERVICE_STOPPED;
      GSIServiceStatus.dwCheckPoint              = 0;
      GSIServiceStatus.dwWaitHint                = 0;
      GSIServiceStatus.dwWin32ExitCode           = 0;
      GSIServiceStatus.dwServiceSpecificExitCode = 0;

            fReturn = SetServiceStatus (GSIServiceStatusHandle, &GSIServiceStatus);
      if (fReturn == 0)
      {
        wsprintf(szErrorStr, "SetServiceStatus error - (%ld)", GetLastError());
        StopFLEXlmService(szErrorStr);
      }
	  else
	  {
			l_flexEventLogWrite(NULL,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_LMGRD_WIN32_SERVICE_STOPPED,
								0,
								NULL,
								0,
								NULL);
	  }
      /* shutdown vendor daemons */
      iProcess_Service_Message = 1; /* tell Vendor Daemon that message came from here */
      ls_kill_chld(SIGTERM);
      break;
    case SERVICE_CONTROL_PAUSE: /* basically do nothing for this state */
      GSIServiceStatus.dwCurrentState            = SERVICE_PAUSED;
      GSIServiceStatus.dwCheckPoint              = 0;
      GSIServiceStatus.dwWaitHint                = 0;
      GSIServiceStatus.dwWin32ExitCode           = 0;
      GSIServiceStatus.dwServiceSpecificExitCode = 0;
            fReturn = SetServiceStatus (GSIServiceStatusHandle, &GSIServiceStatus);
            break;
    case SERVICE_CONTROL_CONTINUE: /* basically do nothing for this state */
      GSIServiceStatus.dwCurrentState            = SERVICE_RUNNING;
      GSIServiceStatus.dwCheckPoint              = 0;
      GSIServiceStatus.dwWaitHint                = 0;
        GSIServiceStatus.dwWin32ExitCode           = 0;
      GSIServiceStatus.dwServiceSpecificExitCode = 0;
            fReturn = SetServiceStatus (GSIServiceStatusHandle, &GSIServiceStatus);
      break;
  }
}
