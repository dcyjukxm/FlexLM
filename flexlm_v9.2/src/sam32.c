/*  (CCW)
    SAM32.c
   
    Windows 95/NT/Win32S Access System (06/97 Release V4.3)
   
    Copyright (C) 1996, 1997 Dallas Semiconductor Corporation.
    All rights Reserved. Printed in U.S.A.
    This software is protected by copyright laws of
    the United States and of foreign countries.
    This material may also be protected by patent laws of the United States
    and of foreign countries.
    This software is furnished under a license agreement and/or a
    nondisclosure agreement and may only be used or copied in accordance
    with the terms of those agreements.
    The mere transfer of this software does not imply any licenses
    of trade secrets, proprietary technology, copyrights, patents,
    trademarks, maskwork rights, or any other form of intellectual
    property whatsoever. Dallas Semiconductor retains all ownership rights.
   
    12/10/96 Added new CheckOverdrive function to better test Overdrive
    12/13/96 Fixed Sleep problem.  Replaced Sleep with FastSleep
     6/09/97 Added checkdriver function to dowcheck (NT)
     6/09/97 Must get semaphore before trying to open a handle to the NT driver. Driver is 
             exclusive access.
   
   NOTE: This file will not compile and link as a cpp file.  The file must be linked as a 
         standard c file. (CW)  My dynamic linking functions for Win32s cause errors with
         a cpp compile.
   ALSO: Inorder to have a program run on Win32s, it must be compiled with Microsoft C++ V2.0
*/
#include <dos.h>
#include <signal.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <windef.h>
#include <winbase.h>
#include <winnt.h>

#include "sauth95.h"
#ifdef __BORLANDC__
#define PLRG_INTEGER PMYLARGE_INTEGER
#define LRG_INTEGER MYLARGE_INTEGER
#else
#define PLRG_INTEGER PLARGE_INTEGER
#define LRG_INTEGER LARGE_INTEGER
#endif

#define DO_RESET         0
#define DO_BIT           1 
#define DO_BYTE          2 
#define TOGGLE_OVERDRIVE 3 
#define TOGGLE_PASSTHRU  4 
#define CHECK_BRICK      5 
#define SET_PORT         6


#define SAUTH95_SEM          "SAuth95AccessSem"
#define SA16COMPAT_SEM       "SAuth95Win16BitCompatSem"
#define SAUTH95_PASSTHRU_SEM "SAuth95PassThruSem"
#define SEM_TIMEOUT          10000 //milliseconds
#define PASSTRHU_TIMEOUT     10    //milliseconds
#define ROMSEARCH            0xF0
HINSTANCE hInst;

#define VERSION_MAJOR          04
#define VERSION_MINOR          31

static void   TogglePassthru(void);
static uchar  EnterPassthru (void);
static uchar  ExitPassthru  (void);
static uchar  CheckBusy     (void);
static uchar  DOWBit        (uchar);
static uchar  DOWByte       (uchar);
static uchar  DOWReset      (void);
static uchar  RomSearch     (void);
static uchar  CheckOverdrive(void);
static void   SetPortNumber();
static void   FastSleep(WORD Delay);
static void   MyInt64Add(PLRG_INTEGER pA, PLRG_INTEGER pB);
static BOOL   MyInt64LessThanOrEqualTo(PLRG_INTEGER pA, PLRG_INTEGER pB);
static uchar  CheckDriver(void);
uchar l_ds_next(void);

//95 variables
HANDLE hSemaphore    = NULL,              // Handle of Semaphore
       hSA16CompatSem= NULL;              // Semaphore for Win16 compatibility
HANDLE hDriver       = NULL;              // Handle of Driver
HANDLE hThread       = NULL;              // Handle of Thread
HANDLE hPassThruSem  = NULL;              // Handle of PassThru Semaphore
uchar  RomDta[8]     = {0,0,0,0,0,0,0,0}; // Current ROM Id
uchar  DriverBuff[4];                     // Buffer for exchanging data with driver
void   *lpDriverBuff = DriverBuff;        // Pointer to data buffer
BOOL   AuthFlag      = FALSE;             // Thread has driver control?
BOOL   SemCreated    = FALSE;             // Semaphore created yet?

//nt variables
HANDLE  DOWHandle;          // Handle to file representing our driver 
HANDLE  sHandle = NULL;     // Handle to our semaphore 
DWORD   NumTrans;    
UCHAR   gpn = 1;
UCHAR   Passthru = TRUE;
UCHAR   TimeOut;
UCHAR   mch[3];

//////////////////////////////////////////////////////////
/////////////// software auth vars ///////////////////////

static uchar  SetupOk    = FALSE;
static uchar  FailNext;
static uchar  AccessGood;
static uchar  l0         = 0;
static uchar  Compatl0   = 0;  // for SACWD300.dll compatability
static ushort pa_vals[3] = { 0x378, 0x278, 0x3BC };
static ushort bpa = 0x378;

void atexit_function(void);

typedef unsigned char uchar;
BOOL WinNT  = FALSE;
BOOL Win95  = FALSE;
BOOL Win32s = FALSE;
BOOL loaded = FALSE;

// structure for Win32s.  
typedef struct
{
   uchar   ld;            /* last discrepancy */
   uchar   lastone;       /* true when last part is seen */
   uchar   unit;          /* parallel port unit, 1, 2, or 3 */
   uchar   accflg;        /* true when access has be executed */
   ushort  count;         /* timing for probes of parallel port */
   ushort  port;          /* parallel port base address */
   ushort  setup_ok;      /* has to be equal to MAGIC */
   uchar   romdta[8];
} 
sa_struct;

sa_struct gb;

// Win32s dynamically loaded function prototypes
uchar (pascal *keyopen32)(void);
void  (pascal *keyclose32)(void);
uchar *(pascal *romdata32)(sa_struct *);
uchar (pascal *setup32)(uchar, sa_struct *);
uchar (pascal *first32)(sa_struct *);
uchar (pascal *next32)(sa_struct *);
uchar (pascal *access32)(sa_struct *);
uchar (pascal *databyte32)(uchar, sa_struct *);
uchar (pascal *gndtest32)(sa_struct *);
uchar (pascal *dowcheck32)(void);
uchar (pascal *OverdriveOn32)(void);
void  (pascal *OverdriveOff32)(void);

////////////////////////////////////////////////////////////////////////////////
uchar *romid;

////////////////////////////////////////////////////////////////////////////////
static uchar CallDriver(DWORD FuncNum, uchar Param)
{
  uchar  *pByte = DriverBuff;   // first byte is data
  ushort *pBPA;                 // next two are the port address
  DWORD BuffSize = 0;           // driver returns number of data bytes to us

  (uchar *)pBPA = &DriverBuff[1];           
  memset(DriverBuff,0,sizeof(DriverBuff));  // clear buffer,
  *pByte = Param;                           // set parameter value
  *pBPA  = bpa;                             // and port address

  DeviceIoControl(hDriver, // communicate with driver
                  FuncNum,
                  lpDriverBuff,sizeof(DriverBuff),
                  lpDriverBuff,sizeof(DriverBuff),(LPDWORD)(&BuffSize),
                  NULL);

  return (*pByte);
}
////////////////////////////////////////////////////////////////////////////////
void CleanUp(void)
{
  if(hSemaphore)
    CloseHandle(hSemaphore);// release semaphore handle to system
  return;
}
////////////////////////////////////////////////////////////////////////////////
uchar garbagebag(uchar *ld)// Added for Win16 compatibility 
{                          // returns lastone for SACWD300.dll
  *ld = Compatl0;
  return FailNext;
}
////////////////////////////////////////////////////////////////////////////////
uchar OverdriveOn(void)
{
   if(!Win32s)
   {
      if (!DOWReset())
         return FALSE;

      // Put all overdrive parts into overdrive mode.
      DOWByte(0x3C);

      return ToggleOverdrive() ? TRUE : ToggleOverdrive();
   }
   else if(Win32s)
   {
      if(OverdriveOn32 != NULL)
         return (*OverdriveOn32)();   
   }
   return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
void OverdriveOff(void)
{
   if(!Win32s)
   {
      // Turn overdrive off
      if(ToggleOverdrive())
         ToggleOverdrive();
   }
   else if(Win32s)
   {
      if(OverdriveOff32 != NULL)
         (*OverdriveOff32)();
   }
   return;
}
////////////////////////////////////////////////////////////////////////////////
static uchar EnterPassthru(void)
{
   if(Win95 || WinNT)
   {
      TogglePassthru();
      Passthru = TRUE;
      return TRUE; 
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static uchar ExitPassthru(void)
{
   uchar i = 0;
   
   if(Win95 || WinNT)
   {
      Passthru = TRUE;

      while(Passthru && (i++ < 3))
      {
         TogglePassthru();
         Passthru = !CheckBusy();// if busy lines don't drop we're in passthru mode
      }

      return !Passthru;
   }
   return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
BOOL LoadDriver(void)
{
   SC_HANDLE hSCManager, hDS1410D;
   char Service[] = "DS1410D";
   DWORD LastError;

   /* Get a handle to the Service control manager database */
   hSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);
   if(hSCManager)
   {
      /* get a handle to our driver */
      hDS1410D = OpenService(hSCManager,
                    Service,
                    SERVICE_ALL_ACCESS);
      if(hDS1410D)
      {
         /* attempt to start DS1410D.SYS */
         if(StartService(hDS1410D, 0, NULL))
            return TRUE;
         else
         {
            LastError = GetLastError();
          
            return FALSE;
         }
      }
      else
      {
         hDS1410D= CreateService(
            hSCManager, // handle to service control manager database  
            Service,    // pointer to name of service to start 
            Service,    // pointer to display name 
            SERVICE_ALL_ACCESS, // type of access to service 
            SERVICE_KERNEL_DRIVER,  // type of service 
            SERVICE_AUTO_START, // when to start service 
            SERVICE_ERROR_NORMAL,   // severity if service fails to start 
            "SYSTEM32\\drivers\\DS1410D.SYS",   // pointer to name of binary file 
            "Extended Base",    // pointer to name of load ordering group 
            NULL,   // pointer to variable to get tag identifier 
            NULL,   // pointer to array of dependency names 
            NULL,   // pointer to account name of service 
            NULL    // pointer to password for service account 
         );
         if(hDS1410D)
         {
            /* attempt to start DS1410D.SYS */
            if(StartService(hDS1410D, 0, NULL))
                 return TRUE;
             else
                 return FALSE;
         }
      }
   }
   return FALSE;
}

// This function is called on NT to determine if the driver is loaded on the System
// CheckDriver should only be called by dowcheck()
static uchar CheckDriver(void)
{

   SC_HANDLE hSCManager, hDS1410D;
   char Service[] = "DS1410D";
   /* Get a handle to the Service control manager database */
   hSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_CONNECT);
   if(hSCManager)
   {
      /* get a handle to our driver */
      hDS1410D = OpenService(hSCManager,
                    Service,
                    SERVICE_QUERY_STATUS);
      if(hDS1410D)
      {
         CloseServiceHandle(hDS1410D);
         CloseServiceHandle(hSCManager);
         return TRUE;
      }
      else
      {
         CloseServiceHandle(hSCManager);
         return FALSE;
      }
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar l_ds_dowcheck(void) // Must be used in All Apps!!!!!
{
   // Determine what version of Windows we are running under.
   // Do the approiate thing under each version.
   
   DWORD dwVersion;
   uchar i;
   dwVersion = GetVersion();

   if (dwVersion < 0x80000000)
   { // Windows NT
      Win95  = FALSE;
      Win32s = FALSE;
      WinNT  = TRUE;
   }
   else if (LOBYTE(LOWORD(dwVersion)) < 4)
   {
      // Win32s or Windows 3.1x
      if ((LOBYTE(LOWORD(dwVersion)) == 3) && (HIBYTE(LOWORD(dwVersion)) <= 0x5E))
      {
         Win95  = FALSE;
         WinNT  = FALSE;
         Win32s = TRUE;
      }
      else
      {  // Early Windows 95 Builds
         Win95  = TRUE;
         WinNT  = FALSE;
         Win32s = FALSE;
      }
   }
   else
   {
      Win95  = TRUE;
      WinNT  = FALSE;
      Win32s = FALSE;
   }
   if(Win95)
   {
      if((!hDriver) || (hDriver == INVALID_HANDLE_VALUE))
         hDriver=CreateFile("\\\\.\\VSAUTHD.VXD",0,0,NULL,
                           OPEN_ALWAYS,FILE_FLAG_DELETE_ON_CLOSE,NULL);

      if(hDriver==INVALID_HANDLE_VALUE)
         return FALSE;         // could not get handle to our driver

      hThread=GetCurrentThread();
      if(!hPassThruSem)
      {
         if(!(hPassThruSem = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,
                                    FALSE,
                                    SAUTH95_PASSTHRU_SEM)))
         {
            hPassThruSem = CreateSemaphore(NULL,1,1,SAUTH95_PASSTHRU_SEM);
         }
      }
      return TRUE;
   }
   else if(Win32s)
   {
      if(!loaded)
      {
         hInst = LoadLibrary("swa32ut.dll");
         if(hInst != NULL)
         {
            (FARPROC)setup32 = GetProcAddress(hInst, "setup");   
            (FARPROC)first32 = GetProcAddress(hInst,"first");
            (FARPROC)next32 =  GetProcAddress(hInst,"next");
            (FARPROC)access32 = GetProcAddress(hInst,"access");
            (FARPROC)databyte32 = GetProcAddress(hInst,"databyte");
            (FARPROC)gndtest32 = GetProcAddress(hInst,"gndtest");
            (FARPROC)romdata32 = GetProcAddress(hInst,"romdata");
            (FARPROC)dowcheck32 = GetProcAddress(hInst,"dowcheck");
            (FARPROC)keyopen32 = GetProcAddress(hInst, "keyopen");
            (FARPROC)keyclose32 = GetProcAddress(hInst, "keyclose");
            (FARPROC)OverdriveOn32 = GetProcAddress(hInst, "OverdriveOn");
            (FARPROC)OverdriveOff32 = GetProcAddress(hInst, "OverdriveOff");
            if(setup32 && first32 && next32 && access32 && databyte32 && 
               gndtest32 && romdata32 && dowcheck32 && keyclose32 && keyopen32)
            {
               loaded = TRUE;
               atexit( atexit_function );
            }
            else
            {
               FreeLibrary(hInst);
               loaded = FALSE;
               return FALSE;
            }
            
         }
         else
         {
            return FALSE;
         }
      }
      if(dowcheck32 != NULL)
         i = (*dowcheck32)();
      else
         i = 0;
      return i;

   }
   else if (WinNT)
   {
      if(!CheckDriver()) // driver isn't loaded try and load it
         return ((uchar)LoadDriver());
      else
         return TRUE;
   }
   return FALSE;   
}
////////////////////////////////////////////////////////////////////////////////
// Do not use for win16 only.
uchar nowaitkeyopen(void)// Added for Win16 compatibility.
{                        // Must be used in Win16 Apps!!!!!
   DWORD WaitResult=0;
   if(Win95)
   {   
      if(!AuthFlag)
      {  
         AuthFlag = ((hDriver==INVALID_HANDLE_VALUE)?FALSE:TRUE);

         if(AuthFlag) // have valid driver handle
         {  
            if(!SemCreated)
            {
               hSemaphore = CreateSemaphore(NULL,1,1,SAUTH95_SEM);
               SemCreated = TRUE;
            }

            if(!hSemaphore) // no semaphore handle yet
            {
               hSemaphore = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,
                                       FALSE,
                                       SAUTH95_SEM);
            }

            if(hSemaphore)
            {
               // try to spread access to all applications evenly
               SetThreadPriority(hThread,THREAD_PRIORITY_ABOVE_NORMAL);// 1 point higher

               // Added for Win16 compatibility
               if(OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,FALSE,SA16COMPAT_SEM))
                  return FALSE;

        
               // Added for Win16 compatibility
               hSA16CompatSem=CreateSemaphore(NULL,1,1,SA16COMPAT_SEM);

               WaitResult=WaitForSingleObject(hSemaphore,SEM_TIMEOUT);

               if(WaitResult==WAIT_FAILED)
               {
                  SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
                  ReleaseSemaphore(hSemaphore,1,NULL);

                  // Added for Win16 compatibility
                  if(hSA16CompatSem)
                     CloseHandle(hSA16CompatSem);// release semaphore handle to system

                  AuthFlag = FALSE;
                  Sleep(0);
                  return FALSE;
               }

            }
            else
               return FALSE;
            
         }// if(AuthFlag)
         
      }// if(!AuthFlag)

      if(AuthFlag)
         ExitPassthru();
         
      return AuthFlag;
   }  
   else if(WinNT)
   {
      return FALSE;
   }
   return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
uchar l_ds_keyopen(void) // Must be used in Win32 Apps!!!!!
{

  DWORD WaitResult=0;
  if(Win95)
  {
      if(!AuthFlag)
      {
         AuthFlag = ((hDriver==INVALID_HANDLE_VALUE)?FALSE:TRUE);

         if(AuthFlag) // have valid driver handle
         {
            if(!SemCreated)
            {
               hSemaphore = CreateSemaphore(NULL,1,1,SAUTH95_SEM);
               SemCreated = TRUE;
            }

            if(!hSemaphore) // no semaphore handle yet
            {
               hSemaphore = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,
                                          FALSE,
                                          SAUTH95_SEM);
            }

            if(hSemaphore)
            {
               // try to spread access to all applications evenly
               SetThreadPriority(hThread,THREAD_PRIORITY_ABOVE_NORMAL);// 1 point higher
               WaitResult=WaitForSingleObject(hSemaphore,SEM_TIMEOUT);
               
               if(WaitResult==WAIT_FAILED)
               {
                  SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
                  ReleaseSemaphore(hSemaphore,1,NULL);
                  AuthFlag = FALSE;
                  Sleep(0);
                  return FALSE;
               }
               
               // Added for Win16 compatibility
               hSA16CompatSem=CreateSemaphore(NULL,1,1,SA16COMPAT_SEM);
            }
            else
               return FALSE;
         }// if(AuthFlag)

      }// if(!AuthFlag)

      if(AuthFlag)
      {
         ExitPassthru();
         if(CheckOverdrive())
            OverdriveOff();       // call overdrive off if mixed stacking is not enabled
      }
      return AuthFlag;
   }
   else if(WinNT)
   {
      if (!AuthFlag)                
      {
        // If it existed we still don't have a handle 
        if (sHandle == NULL || !SemCreated) 
        {
           sHandle = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, 
                                FALSE, 
                                "DS1410XAccessSem");
           // Make sure we got a handle.
           if (sHandle != NULL)
              SemCreated = TRUE;
        }
        // Create semaphore if it doesn't exist 
        if (!SemCreated)           
        {
           sHandle = CreateSemaphore(NULL, 1, 1, "DS1410XAccessSem");
           if (sHandle != NULL)
              SemCreated = TRUE;
        }

        if (SemCreated == TRUE)
        {
            // Wait until semaphore value = 0, or 5 second timeout 
            WaitForSingleObject(sHandle, 5000); 
            
            // Once we've got the semaphore we should be able to get the handle
            // (unless semaphore wait timed out, then we fail CreateFile)
            // Try to get handle to our driver
            AuthFlag = !((DOWHandle = CreateFile("LPT9",     
                                           GENERIC_READ | GENERIC_WRITE,
                                           0,       
                                           NULL,   
                                           OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL,
                                          NULL)) == INVALID_HANDLE_VALUE);
            if(AuthFlag)
            {
                // Set port number in the driver
                SetPortNumber();
            
                // Enter EPP passthru mode
                ExitPassthru();
                if (CheckOverdrive())
                    OverdriveOff();
            }
            else
            {
               ReleaseSemaphore(sHandle, 1, NULL);      
               return FALSE;
            }
        }
        else
            return AuthFlag;
      }
      return AuthFlag;
   }
   else if(Win32s)
   {
      return (*keyopen32)();
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar l_ds_keyclose(void) // Must be Used in All Apps!!!!!
{
   if(Win95)
   {
      if(AuthFlag)
      {
         EnterPassthru();
         // Added for Win16 compatibility
         if(hSA16CompatSem)
         {
            CloseHandle(hSA16CompatSem);// release semaphore handle to system
         }

         ReleaseSemaphore(hSemaphore,1,NULL);
         AuthFlag = FALSE;
         // allow fair access to other applications
         SetThreadPriority(hThread,THREAD_PRIORITY_BELOW_NORMAL);// 1 point lower
         Sleep(0); //release time slice
         SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
      }
   }
   else if(WinNT)
   {
      // If we had control, release control 
      if (AuthFlag)                    
      {
         // Enter EPP passthru mode
         EnterPassthru();
         // Close handle to DS1410E.SYS 
         CloseHandle(DOWHandle);               
         // Set semaphore value back to 0 
         ReleaseSemaphore(sHandle, 1, NULL);  
         AuthFlag = FALSE;     
      }
   }
   else if(Win32s)
      (*keyclose32)();
   return TRUE;
}
////////////////////////////////////////////////////////////////////////////////
uchar l_ds_first(void)
{
   if(!Win32s)
   {
      // Don't force a failure here
      FailNext = FALSE;
      // Point Rom Search algorithm to the top
      l0 = 0;

      // Go look for the first DOW part on the bus
      return l_ds_next();
   }
   else if(Win32s)
      return (*first32)(&gb);
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar l_ds_next(void)
{
   uchar tr;
   if(Win95)
   {
      if (SetupOk)
      {
         // See if last search found last button
         if (FailNext)
         {
            FailNext = FALSE;

            // Reset next function
            l0 = 0;
         }
         else
         while ((tr = RomSearch()) != 0) // Do that ROM search thang
         {
            // See if we should force failure
            if (tr == 2)
               FailNext = 1;

            // Detect short circuit

            if (!RomDta[0])
               return FALSE;

            return TRUE;         
         }//else

      }//if (SetupOk)
      return FALSE;
   }
   else if(WinNT)
   {
      if (SetupOk)
      {
         // See if last search found last button
         if (FailNext)                    
         {
            FailNext = FALSE; 
      
            // Reset next function 
            l0 = 0;                                        
         } 
         else while ((tr = RomSearch()) != 0) 
         {
            // See if we should force failure
            if (tr == 2) 
               FailNext = 1;                
           
            // Detect short circuit
            if (!RomDta[0])       
               return FALSE;
      
            AccessGood = TRUE;
      
            return TRUE;
         }
      }
      return FALSE;
   }
   else if(Win32s)
      return (*next32)(&gb);
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar l_ds_access(void)
{
   uchar i, j;

   if(Win95)
   {
      // Assume failure
      AccessGood = FALSE;

      // Send reset pulse
      if (DOWReset())
      {
         // ROM search command byte
         DOWByte(ROMSEARCH);

         // Byte loop
         for (i = 0; i < 8; i++)
         {
            // Bit loop
            for (j = 0; j < 8; j++)
            {
               if (((DOWBit(TRUE) << 1) | DOWBit(TRUE)) == 3)
                  return FALSE;     // Part not there

               // Send write time slot
               DOWBit((uchar) ((RomDta[i] >> j) & 1));
            }
         }

         // Success if we made it through all the bits
         AccessGood = TRUE;
      }

      return AccessGood;
   }
   else if(WinNT)
   {
       // Assume failure 
      AccessGood = FALSE;                                       
      
      // Send reset pulse
      if (DOWReset())                                         
      {
         // ROM search command byte
         DOWByte(0xF0);                                
      
         // Byte loop
         for (i = 0; i < 8; i++)                                     
         {
            // Bit loop
            for (j = 0; j < 8; j++)                                   
            {
               if (((DOWBit(TRUE) << 1) | DOWBit(TRUE)) == 3)                
                  return FALSE;     
      
               // Send write time slot
               DOWBit((UCHAR) ((RomDta[i] >> j) & 1));    
            }      
         }
      
         // Success if we made it through all the bits
         AccessGood = TRUE;         
      }
      
      return AccessGood;
   }
   else if(Win32s)
      return (*access32)(&gb);
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar gndtest(void)
{
   if(Win95)
   {
      if (SetupOk)
      {
         DOWReset();
         return DOWBit(TRUE);
      }

      return FALSE;
   }
   else if(WinNT)
      return SetupOk;
   else if(Win32s)
      return (*gndtest32)(&gb);
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar *l_ds_romdata(void)
{
   ushort i;
   if(!Win32s)
      return (uchar *)RomDta;
   else if(Win32s)
   {
      if(romdata32 != NULL)
      {
         romid = (*romdata32)(&gb);
         for (i = 0; i < 8; i++)
            gb.romdta[i] = romid[i];
         return gb.romdta;
      }
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
uchar l_ds_databyte(uchar data)
{
   if(!Win32s)
      return (SetupOk && AccessGood) ? DOWByte(data) : data;
   else if(Win32s)
   {
      if(databyte32 != NULL)
         return (*databyte32)(data, &gb);
   }
   return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
uchar l_ds_setup(uchar pn)
{
   if(Win95)
   {
      SetupOk = FailNext = AccessGood = FALSE;         // Initialize global flags

      // Reset RomSearch (first, next) algorithm
      FailNext = FALSE;
      l0 = 0;

      if (pn > 0 && pn < 4)                     // Make sure port number is valid
      {
         // This allows all other functions to execute
         SetupOk = TRUE;
         // Set base port address
         bpa = pa_vals[pn - 1];
      }
      else
         bpa = pa_vals[0];  // Set to default in case caller ignores FALSE return

      return SetupOk;                          // Return result of setup function
   }
   else if(WinNT)
   {
      // Initialize global flags
      SetupOk = FailNext = AccessGood = FALSE;         
      
      // Reset RomSearch (first, next) algorithm 
      l0 = 0;                            
      
      // Make sure port number is valid
      if (pn > 0 && pn < 4)           
      {
         // This allows all other functions to execute
         SetupOk = TRUE;            
         // Assign port number
         gpn = pn;
      }
      else
         gpn = 1; 
      
      // Return result of setup function
      return SetupOk;
   }
   else if(Win32s)
   {
      if(setup32 != NULL)
         return (*setup32)(pn, &gb);
      else
         return FALSE;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static 
uchar DOWBit(uchar bit_val)
{
   BOOLEAN BitResult = TRUE;
   if(Win95)
      return CallDriver(DOWBIT,bit_val);
   else if(WinNT)
   {
      if (AuthFlag)
      {
         mch[0] = DO_BIT;  // Tell driver to do a bit 
         mch[1] = bit_val; // Specify bit value 
      
         if (WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL) &&
             ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL))
         {
            BitResult = (BOOLEAN) *mch; 
         }
      } 
      
      return BitResult;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static 
uchar DOWByte(uchar byte_val)
{
   UCHAR ByteResult = byte_val;
   if(Win95)
      return CallDriver(DOWBYTE,byte_val);
   else if(WinNT)
   {
      if (AuthFlag)
      {
         mch[0] = DO_BYTE;  // Tell driver to do a bit 
         mch[1] = byte_val; // Specify bit value 
      
         if (WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL) &&
             ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL))
         {
            ByteResult = *mch;                  
         }
      } 
      
      return ByteResult;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static 
uchar DOWReset()
{
   BOOLEAN ResetResult = FALSE;
   if(Win95)
      return CallDriver(DOWRESET,0);
   else if (WinNT)
   {
      if (AuthFlag)
      {
         mch[0] = DO_RESET; // Tell driver to do a reset 
         mch[1] = gpn;      // Specify a port 
      
         if (WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL) && 
             ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL))
         {
            // Assign result 
            ResetResult = (BOOLEAN) *mch;                     
         }
         Sleep(1);
      } 
      
      return ResetResult;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static 
uchar ToggleOverdrive()
{
   UCHAR ByteResult = 0;
   if(Win95)
      return CallDriver(DOWTOGGLEOD,0);
   else if(WinNT)
   {
      if (AuthFlag)
      {
         mch[0] = TOGGLE_OVERDRIVE; 
      
         if (WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL) &&
             ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL))
         {
            ByteResult = *mch;                  
         }
      } 
      return ByteResult;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static uchar CheckOverdrive()
{
   uchar ByteResult = 0;

   if(Win95)
   {
      return CallDriver(CHECK_OVERDRIVE,0);//Busy Lines Dropped
   }
   else if(WinNT)
   {
      if (AuthFlag)
      {
         mch[0] = CHECK_OVERDRIVE; 
      
         if (WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL) &&
             ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL))
         {
            ByteResult = mch[0];                  
         }
      } 

      return ByteResult;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static 
uchar CheckBusy()
{
   UCHAR ByteResult = 0;
   if(Win95)
      return CallDriver(DOWCHECKBSY,0);//Busy Lines Dropped
   else if(WinNT)
   {
      if (AuthFlag)
      {
         mch[0] = CHECK_BRICK; 
      
         if (WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL) &&
             ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL))
         {
            ByteResult = *mch;                  
         }
      } 
      
      return ByteResult;
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
static
void TogglePassthru(void)
{
   UCHAR i;
   if(Win95)
      CallDriver(DOWTOGGLEPASS,0);
   else if(WinNT)
   {
      for (i = 0; i < 4; i++)
         ToggleOverdrive();
   }
   FastSleep(10);
   return;
}
////////////////////////////////////////////////////////////////////////////////
static 
uchar RomSearch(void)
{
   uchar
         i = 0,
         x = 0,
         ld = l0;
   uchar RomBit,
         Mask;

   if(!Win32s)
   {
      // Reset DOW bus
      if (DOWReset())
         DOWByte(ROMSEARCH);  // Send search command
      else
      {
         return FALSE;   // No DOW parts were found on bus
      }
      // While not done and not bus error
      while ((i++ < 64) && (x < 3))
      {
         Mask = 1 << ((i - 1) % 8);

         // Get last pass bit
         RomBit = RomDta[(i - 1) >> 3] & Mask ? TRUE : FALSE;

         // Send read time slots
         x = DOWBit(TRUE) << 1;
         x |= DOWBit(TRUE);

         // Is there a disagreement in this bit position
         if (!x)
         {
            // Stay on old path or pick a new one ?
            if (i >= ld)
               RomBit = (i == ld);         // Send write 1 if at position of ld 

            // Save this value as temp last disagreement
            if (!RomBit)
               l0 = i;
         }
         else
            RomBit = (x & 1) ^ 1;          // Get lsb of x and flip it

         if (RomBit)
            RomDta[(i - 1) >> 3] |= Mask;          // Set bit in Rom Data byte
         else
            RomDta[(i - 1) >> 3] &= (Mask ^ 0xFF); // Clear bit in Rom Data byte

         // Send write time slot
         DOWBit(RomBit);
      }

      Compatl0=l0;
      return (x == 3) ? 0 : 1 + (ld == l0);
   }
   return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
void atexit_function(void)
{
   if (hInst != NULL)
      FreeLibrary(hInst);
}

////////////////////////////////////////////////////////////////////////////////
static void SetPortNumber()
{
   UCHAR ByteResult = 0;

   if (AuthFlag)
   {
      mch[0] = SET_PORT; 
      mch[1] = gpn; 

      WriteFile(DOWHandle, (LPSTR) mch, 2, &NumTrans, NULL);
      ReadFile(DOWHandle, (LPSTR) mch, 1, &NumTrans, NULL);
   } 
   return;
}

static void MyInt64Add(PLRG_INTEGER pA, PLRG_INTEGER pB)
{
   BYTE c = 0;

   if ((pA->LowPart != 0L) && (pB->LowPart != 0L))
      c = ((pA->LowPart + pB->LowPart) < pA->LowPart) ? 1 : 0;

   pA->LowPart  += pB->LowPart;
   pA->HighPart += pB->HighPart + c;
   return;
}

//////////////////////////////////////////////////////////////////////////////
//
//   Returns TRUE is A <= B.
//
static BOOL MyInt64LessThanOrEqualTo(PLRG_INTEGER pA, PLRG_INTEGER pB)
{
   BOOL CompRes = FALSE;

   if (pA->HighPart < pB->HighPart)
      CompRes = TRUE;
   else if ((pA->HighPart == pB->HighPart) && (pA->LowPart <= pB->LowPart))
      CompRes = TRUE;

   return CompRes;
}
static void FastSleep(WORD Delay)
{
   LRG_INTEGER CountsPerSec, CurrentCount, FinalCount, DelayCounts;
   BOOL          UseOldSleep = TRUE;
    
   if (QueryPerformanceFrequency(&CountsPerSec))
   {
      if (CountsPerSec.HighPart == 0L)
      {
         DelayCounts.HighPart = 0;
         DelayCounts.LowPart  = (CountsPerSec.LowPart / 1000L) * Delay;

         if (DelayCounts.LowPart)
         {
            // Get current count value
            QueryPerformanceCounter(&FinalCount);
            // FinalCount += DelayCounts
            MyInt64Add(&FinalCount, &DelayCounts);

            do
            {
               SleepEx(1, FALSE);
               QueryPerformanceCounter(&CurrentCount);   
            }
            while (MyInt64LessThanOrEqualTo(&CurrentCount, &FinalCount));

            UseOldSleep = FALSE;
         }
      }
   }
 
   if (UseOldSleep)
   {
      // Use regular sleep if hardware doesn't support a performance counter
      SleepEx(Delay, FALSE);
   }
   return;
}
/*
 * Get the version of the Access Layer.
 */
unsigned long GetAccessAPIVersion()
{
  return (((unsigned long)VERSION_MINOR)<<16) + VERSION_MAJOR;
}
////////////////////////////////////////////////////////////////////////////////
// SAM32.c
