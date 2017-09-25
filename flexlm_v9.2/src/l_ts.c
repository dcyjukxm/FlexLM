
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "l_Wtsapi32.h"


typedef BOOL (WINAPI * WTSENUMERATESESSIONSTYPE)(HANDLE, 
                                                 DWORD, 
                                                 DWORD, 
                                                 PWTS_SESSION_INFO *, 
                                                 DWORD *);

typedef VOID (WINAPI * WTSFREEMEMORYTYPE)(PVOID);

/* originally defined in winuser.h, in Aug 2001 platform SDK */
#define SM_REMOTESESSION        0x1000

#define L_OS_WINDOWS_95      1
#define L_OS_WINDOWS_98      2
#define L_OS_WINDOWS_ME      3
#define L_OS_WINDOWS_NT      4
#define L_OS_WINDOWS_NT351   5
#define L_OS_WINDOWS_2000    6
#define L_OS_WINDOWS_XP      7
#define L_OS_WINDOWS_DOT_NET 8

static BOOL						bWtsapi32Init = FALSE;
static BOOL						bTSClient = FALSE;
static WTSENUMERATESESSIONSTYPE	GSIWtsEnumerateSessions = NULL;;
static WTSFREEMEMORYTYPE		GSIWtsFreeMemory = NULL;
static HANDLE					hWtsApi = NULL;
#define TS_MUTEX_NAME			"TerminalServerFlexlmMutex"
/*
 *	The above changes result in:
 *	1. Memory associated with freeing hWtsApi being leaked as we don't have
 *		a mechanism for freeing it.  This should be minor though.
 */

/*
 *	This function is never called because we currently do not have a sdk cleanup routine.
 */
static
void
sCleanupIphl(void)
{
	if(hWtsApi)
	{
		FreeLibrary(hWtsApi);
		hWtsApi = NULL;
	}
}


int isTSOK(void);
int GetWindowsVersion();

int GetWindowsVersion()
{
DWORD dwVersion, dwWindowsMajorVersion, dwWindowsMinorVersion;
DWORD dwBuild;
int iVersion;

	dwVersion = GetVersion();
 
	// Get the Windows version.

	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

	// Get the build number.

	if (dwVersion < 0x80000000)              // Windows NT/2000/XP
	    dwBuild = (DWORD)(HIWORD(dwVersion));
	else if (dwWindowsMajorVersion < 4)      // Win32s
	    dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
	else                                     // Windows 95/98/Me
	    dwBuild =  0;

//	printf("MajorVersion: %ld MinorVersion: %ld\n\n", dwWindowsMajorVersion, dwWindowsMinorVersion);

	if ((dwWindowsMajorVersion == 5) && (dwWindowsMinorVersion == 2))
		iVersion = L_OS_WINDOWS_DOT_NET; 	//printf("Windows .NET\n");
	if ((dwWindowsMajorVersion == 5) && (dwWindowsMinorVersion == 1))
		iVersion = L_OS_WINDOWS_XP; 		//printf("Windows XP\n");
	if ((dwWindowsMajorVersion == 5) && (dwWindowsMinorVersion == 0))
		iVersion = L_OS_WINDOWS_2000; 		//printf("Windows 2000\n");
	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 0))
		iVersion = L_OS_WINDOWS_NT; 		//printf("Windows NT 4.0\n");
	if ((dwWindowsMajorVersion == 3) && (dwWindowsMinorVersion == 0))
		iVersion = L_OS_WINDOWS_NT351; 		//printf("Windows NT 3.51\n");
	if ((dwWindowsMajorVersion == 4) && (dwWindowsMinorVersion == 1))
		iVersion = L_OS_WINDOWS_ME; 		//printf("Windows 95,98,Me\n");

	return (iVersion);
}

int
isTSOK(void)
{
	int                 OSVersion;
	PWTS_SESSION_INFO	pSessionInfo = NULL;
	DWORD				dwSessionCount = 0, dwErr = 0;
	char				*szReturn = NULL;
	char				szMutexName[MAX_PATH] = {'\0'};
	HANDLE				hWtsapi32Mutex = NULL;
	BOOL                bRemote = FALSE;


	/* if env is set bypass act as local */
	szReturn = getenv("FLEXLM_TS_GATE");	/* overrun checked */
	if (szReturn != NULL)
	{
		if(strcmp(szReturn, "bypass") == 0)
			return 0;
	}

	if(!bWtsapi32Init)
	{
		sprintf(szMutexName, "%s_%d", TS_MUTEX_NAME, getpid());
		hWtsapi32Mutex = CreateMutex(NULL, 1, szMutexName);
		if(hWtsapi32Mutex == NULL)
		{
			return 0;
		}
		else
		{
			if(GetLastError() == ERROR_ALREADY_EXISTS)
			{
				WaitForSingleObject(hWtsapi32Mutex, INFINITE);
			}
			if(!bWtsapi32Init)
			{
				OSVersion = GetWindowsVersion();
				switch(OSVersion)
				{		
					case L_OS_WINDOWS_95:
					case L_OS_WINDOWS_98:
					case L_OS_WINDOWS_ME:
							// revert to v7.2 behavior
							break;
					case L_OS_WINDOWS_NT351:
					case L_OS_WINDOWS_NT:
							hWtsApi = LoadLibrary("wtsapi32.dll");
							if (hWtsApi)
							{
								GSIWtsEnumerateSessions = (WTSENUMERATESESSIONSTYPE)GetProcAddress(hWtsApi,"WTSEnumerateSessionsA");
								if (GSIWtsEnumerateSessions)
								{
									GSIWtsFreeMemory = (WTSFREEMEMORYTYPE)GetProcAddress(hWtsApi,"WTSFreeMemory");
									if (GSIWtsFreeMemory)
									{
										((* GSIWtsEnumerateSessions )( WTS_CURRENT_SERVER_HANDLE,
																		0,
																		1,
																		&pSessionInfo,
																		&dwSessionCount));
								
										if (!pSessionInfo || !pSessionInfo->State)
										{
											// local client
											((* GSIWtsFreeMemory )( pSessionInfo));
										}
										else
										{
											// remote client
											((* GSIWtsFreeMemory )( pSessionInfo));
											bTSClient = 1;
										}
									}	// end if GSIWtsFreeMemory
								}	// end if GSIWtsEnumerateSessions
							}	// end if hWtsApi
							FreeLibrary(hWtsApi);
							hWtsApi = NULL;
							break;
					case L_OS_WINDOWS_2000:
					case L_OS_WINDOWS_XP:
					case L_OS_WINDOWS_DOT_NET:
							bRemote = GetSystemMetrics(SM_REMOTESESSION);
							if (bRemote)
							{
								// remote client
								bTSClient = 1;
							}
							break;
					default:
							break;
				}	/* end switch*/ 
				bWtsapi32Init = 1;
			}	// end if bWtsapi32Init
			ReleaseMutex(hWtsapi32Mutex);
			CloseHandle(hWtsapi32Mutex);
		}	// end else mutex
	}

	return bTSClient ? 1 : 0;
}
