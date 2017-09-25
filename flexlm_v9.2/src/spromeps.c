/****************************************************************************
 *                  SuperPro Multiple Entry Points Module                   *
 ****************************************************************************
 * This module provides a method for performing SuperPro API commands so    *
 * so you do not have to deal with command packets.  It provides a function *
 * for each API command.                                                    *
 ****************************************************************************/

/****************************************************************************
 *   I M P O R T A N T   N O T E  -  WATCOM 32-BIT WINDOWS DEVELOPERS       *
 ****************************************************************************
 * 32 bit Windows interface cannot handle a call interrupt.  For the Watcom *
 * C Windows developer, you must use WAT_SUPERPRO instead of SUPERPRO.  This*
 * will pad the SUPERPRO call with equivalent Watcom functions to do the    *
 * task switcher control.                                                   *
 *   This function is set off by a define "__WIN32__."  If you are doing    *
 * 32-bit DOS-Extender development, do not incorperate this define          *
 ****************************************************************************/

/* (C) Copyright 1986-1993 Rainbow Technologies, Inc. All rights reserved. */


#include "16_superpro.h"           /* include of core driver definitions */
#include "16_spromeps.h"           /* include of prototype file for this module */

#ifdef __WIN32__                /* Needed for Watcom and 32bit Windows */
   #include <dos.h>
#endif

#ifdef __WATCDLL__         /* IF WATCOMC DLL */
#include "WINDOWS.H"       /* FOR SPECIALIZED FUNCTION */
HINDIR SuperproHandle;     /* FUNCTION HANDLE */
#endif

APIPACKET ApiPacket;            /* API Command Packet */
UNITINFO  InitialUI ;

/****************************************************************************
 *
 *  Function: sproInitialize
 *  Purpose : To initialize the SuperPro Driver
 *
 *  Inputs      : NONE
 *  Outputs     : NONE
 *  Return Value: Status
 *
 *  Description : This routine does autotiming and checks for the valid port
 *                addresses based on the values passed in through the API
 *                packet's portAddrs field.
 ****************************************************************************/
int sproInitialize(void)
{
int error ;

  ApiPacket.functionCode = API_INITIALIZE;
  ApiPacket.intMode      = DEFAULT_INT_METHODS;
  ApiPacket.portAddrs[0] = DEFAULT_PORT_ADDRESS1;
  ApiPacket.portAddrs[1] = DEFAULT_PORT_ADDRESS2;
  ApiPacket.portAddrs[2] = DEFAULT_PORT_ADDRESS3;
  ApiPacket.portAddrs[3] = 0x0;
  ApiPacket.InitialPICMask1 = DEFAULT_PIC_MASK1;
  ApiPacket.InitialPICMask2 = DEFAULT_PIC_MASK2;

  #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
     error = WAT_SUPERPRO(&ApiPacket);
  #else
  #ifdef _INTELC32_
     error = cSUPERPRO(&ApiPacket);
  #else
     error = SUPERPRO(&ApiPacket);
  #endif
  #endif

  InitialUI = ApiPacket.ui;
  return(error);
}


/****************************************************************************
 *
 *  Function: sproFindFirstUnit
 *  Purpose : Find a SuperPro with the specified developer ID and return a
 *            a unit handle.
 *
 *  Inputs      : developerID - your assigned 16 bit developer.
 *
 *  Outputs     : *unitInfo - ptr to a unit info handle structure.
 *
 *  Return Value: Status
 *
 **************************************************************************/
int sproFindFirstUnit(unsigned int developerID, UNITINFO *unitinfo)
{
  int error;

  ApiPacket.functionCode    = API_FIND_FIRST_UNIT;
  ApiPacket.memoryContents  = developerID;
  ApiPacket.ui = InitialUI;

  #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
     error = WAT_SUPERPRO(&ApiPacket);
  #else
  #ifdef _INTELC32_
     error = cSUPERPRO(&ApiPacket);
  #else
     error = SUPERPRO(&ApiPacket);
  #endif
  #endif

  if (unitinfo) *unitinfo = ApiPacket.ui;
  return(error);
}


 /****************************************************************************
 *
 *  Function: sproFindNextUnit
 *  Purpose : To find the next SuperPro unit with the same developer ID
 *            as the unit specified by the unitinfo parameter, and return
 *            a new unit handle.
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *  Outputs     : *unitInfo - ptr to a new unit info handle structure.
 *  Return Value: Status
 *
 **************************************************************************/
int sproFindNextUnit(UNITINFO *unitinfo)
{
  int error;

  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode = API_FIND_NEXT_UNIT;

  #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
     error = WAT_SUPERPRO(&ApiPacket);
  #else
  #ifdef _INTELC32_
     error = cSUPERPRO(&ApiPacket);
  #else
     error = SUPERPRO(&ApiPacket);
  #endif
  #endif

  if (unitinfo) *unitinfo = ApiPacket.ui;
  return(error);
}


 /****************************************************************************
 *
 *  Function: sproRead
 *  Purpose : To read the data in a readable SuperPro Cell
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 address - address 0-63 of the cell of interest.
 *  Outputs     : *data - ptr to 16 bit contents of the addressed cell.
 *  Return Value: Status
 *
 **************************************************************************/
int sproRead(UNITINFO *unitinfo, int address, unsigned int *data)
{
  int error;

  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode  = API_READ;
  ApiPacket.memoryAddress = address;

  #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
     error = WAT_SUPERPRO(&ApiPacket);
  #else
  #ifdef _INTELC32_
     error = cSUPERPRO(&ApiPacket);
  #else
     error = SUPERPRO(&ApiPacket);
  #endif
  #endif

  if (!error) *data = ApiPacket.memoryContents;
  return(error);
}


 /****************************************************************************
 *
 *  Function: sproExtendedRead
 *  Purpose : To read the data in a readable SuperPro Cell
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 address - address 0-63 of the cell of interest.
 *  Outputs     : *data - ptr to 16 bit contents of the addressed cell.
 *                *accessCode - ptr to access code value (0-3) of the
 *                              addressed cell
 *  Return Value: Status
 *
 **************************************************************************/
int sproExtendedRead(UNITINFO  *unitinfo,
                 int           address,
                 unsigned int  *data,
                 unsigned char *accessCode)
{
  int error;

  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode  = API_EXTENDED_READ;
  ApiPacket.memoryAddress = address;

  #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
     error = WAT_SUPERPRO(&ApiPacket);
  #else
  #ifdef _INTELC32_
     error = cSUPERPRO(&ApiPacket);
  #else
     error = SUPERPRO(&ApiPacket);
  #endif
  #endif

  if (!error) {
    *data       = ApiPacket.memoryContents;
    *accessCode = ApiPacket.accessCode;
  }
  return(error);
}

 /****************************************************************************
 *
 *  Function: sproWrite
 *  Purpose : To write to a read/write Superpro Cell.
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 writePassword - 16 bit write password
 *                 address    - address 0-63 of the cell to write to.
 *                 data       - the 16 bit value to write to the target cell.
 *                 accessCode - the Access code to write (0-3)
 *                           0 = Read/write
 *                           1 = Read only
 *                           2 = Counter
 *                           3 = Algo/Password
 *  Outputs     : NONE
 *  Return Value: Status
 *
 **************************************************************************/
int sproWrite(UNITINFO  *unitinfo,
          unsigned int  writePassword,
          int           address,
          unsigned int  data,
          unsigned char accessCode)
{
  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode     = API_WRITE;
  ApiPacket.writePassword    = writePassword;  /* automatically pass the WP */
  ApiPacket.memoryAddress    = address;
  ApiPacket.accessCode       = accessCode;
  ApiPacket.memoryContents   = data;
  return(
    #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
       WAT_SUPERPRO(&ApiPacket)
    #else
    #ifdef _INTELC32_
       cSUPERPRO(&ApiPacket)
    #else
       SUPERPRO(&ApiPacket)
    #endif
    #endif
  );
}


 /****************************************************************************
 *
 *  Function: sproOverwrite
 *  Purpose : To Overwrite any Overwriteable Superpro Cell (cells 5-63)
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 writePassword      - 16 bit write password
 *                 OverwritePassword1 - 16 bit Overwrite password 1
 *                 OverwritePassword2 - 16 bit Overwrite password 2
 *                 address    - address 0-63 of the cell to write to.
 *                 data       - the 16 bit value to write to the target cell.
 *                 accessCode - the Access code to write (0-3)
 *                           0 = Read/write
 *                           1 = Read only
 *                           2 = Counter
 *                           3 = Algo/Password
 *  Outputs     : NONE
 *  Return Value: Status
 *
 **************************************************************************/
int sproOverwrite(UNITINFO  *unitinfo,
              unsigned int  writePassword,
              unsigned int  overwritePassword1,
              unsigned int  overwritePassword2,
              int           address,
              unsigned int  data,
              unsigned char accessCode)
{
  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode   = API_OVERWRITE;
  ApiPacket.writePassword  = writePassword;
  ApiPacket.xtraPassword1  = overwritePassword1;
  ApiPacket.xtraPassword2  = overwritePassword2;
  ApiPacket.memoryAddress  = address;
  ApiPacket.accessCode     = accessCode;
  ApiPacket.memoryContents = data;
  return(
    #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
       WAT_SUPERPRO(&ApiPacket)
    #else
    #ifdef _INTELC32_
       cSUPERPRO(&ApiPacket)
    #else
       SUPERPRO(&ApiPacket)
    #endif
    #endif
  );
}


 /****************************************************************************
 *
 *  Function: sproDecrement
 *  Purpose : This function decrements a specified cell.  If the cell
 *            decrements to 0 and there is an associated active algo
 *            descriptor, the algo descriptor is deactivated.
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 writePassword      - 16 bit write password
 *                 address    - address 0-63 of the cell to decrement.
 *  Outputs     : NONE
 *  Return Value: Status
 *
 **************************************************************************/
int sproDecrement(UNITINFO  *unitinfo,
              unsigned int writePassword,
              int          address)
{
  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode  = API_DECREMENT;
  ApiPacket.writePassword = writePassword;
  ApiPacket.memoryAddress = address;
  return(
    #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
       WAT_SUPERPRO(&ApiPacket)
    #else
    #ifdef _INTELC32_
       cSUPERPRO(&ApiPacket)
    #else
       SUPERPRO(&ApiPacket)
    #endif
    #endif
  );
}


 /****************************************************************************
 *
 *  Function: sproActivate
 *  Purpose : To activate an inactive algo descriptor.
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 writePassword      - 16 bit write password
 *                 activatePassword1  - 16 bit activate password 1
 *                 activatePassword2  - 16 bit activate password 2
 *                 address    - address 0-63 of the cell to decrement.
 *  Outputs     : NONE
 *  Return Value: Status
 *
 **************************************************************************/
int sproActivate(UNITINFO   *unitinfo,
             unsigned int  writePassword,
             unsigned int activatePassword1,
             unsigned int activatePassword2,
             int           address)
{
  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode  = API_ACTIVATE_ALGORITHM;
  ApiPacket.writePassword = writePassword;
  ApiPacket.xtraPassword1 = activatePassword1;
  ApiPacket.xtraPassword2 = activatePassword2;
  ApiPacket.memoryAddress = address;
  return(
    #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
       WAT_SUPERPRO(&ApiPacket)
    #else
    #ifdef _INTELC32_
       cSUPERPRO(&ApiPacket)
    #else
       SUPERPRO(&ApiPacket)
    #endif
    #endif
  );
}

 /****************************************************************************
 *
 *  Function: sproQuery
 *  Purpose : to scramble a bit stream through the superpro.
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 address    - address of the algo descriptor to use.
 *                 *queryData - ptr to input string
 *                 *response  - ptr to returned response string
 *                 *response32 - last 32 bits of response data
 *                 length - length of the input (and response) string.
 *  Outputs     : NONE
 *  Return Value: Status
 *
 **************************************************************************/
int sproQuery(UNITINFO  *unitinfo,
          int           address,
          void          *queryData,
          void          *response,
          unsigned long *response32,
          unsigned int  length)
{
  int error;

#ifdef __WATCDLL__    /* IF WATCOMC DLL */
  DWORD queryData16, response16;

  queryData16 = AllocAlias16( queryData);    /* ALLOCATE 16BIT EQUIVALENT */
  response16 = AllocAlias16( response);

  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode  = API_QUERY;
  ApiPacket.memoryAddress = address;
  ApiPacket.QueryIn       = (unsigned long)queryData16;
  ApiPacket.QueryOut      = (unsigned long)response16;
  ApiPacket.dataLength    = length;

#else                /* IF NOT WATCOMC DLL */

  if (unitinfo) ApiPacket.ui = *unitinfo;
  ApiPacket.functionCode  = API_QUERY;
  ApiPacket.memoryAddress = address;
  ApiPacket.QueryIn       = (unsigned long)queryData;
  ApiPacket.QueryOut      = (unsigned long)response;
  ApiPacket.dataLength    = length;

#endif

  #ifdef __WIN32__        /* 32 bit Windows cannot call interrupts */
     error = WAT_SUPERPRO(&ApiPacket);
  #else
  #ifdef _INTELC32_
     error = cSUPERPRO(&ApiPacket);
  #else
     error = SUPERPRO(&ApiPacket);
  #endif
  #endif

#ifdef __WATCDLL__    /* IF WATCOMC DLL */

   FreeAlias16( queryData16);    /* FREE PREVIOUS ALLOCATION */
   FreeAlias16( response16);

#endif


  if (!error && !response)
    *response32 = ApiPacket.longResult;
  return(error);
}

#ifdef __WIN32__
 /****************************************************************************
 *
 *  Function: WAT_SUPERPRO  (only for Watcom 32-bit Windows development)
 *  Purpose : To offset the problem with interrupt calls and 32 bit Windows
 *               interfaces, this program uses the Watcom C library function
 *               to do the interrupts.
 *
 *  Inputs      :  *ApiPacket - ptr to the information needed for the
 *                    function SUPERPRO.
 *  Outputs     :  NONE
 *  Return Value:  result of the SUPERPRO function.
 *
 **************************************************************************/
int WAT_SUPERPRO(APIPACKET  *apipack)

{

   int error;
   union REGS regs;

      /***  Begin the Critical Section handling  ***/
   regs.w.ax = 0x1681;
   int86( 0x2F, &regs, &regs);

      /***  Call the Superpro function ***/
#ifndef __WATCDLL__    /* IF NOT WATCOMC DLL */
   error = SUPERPRO ( apipack);
#else                  /* IF WATCOMC DLL */
   error = InvokeIndirectFunction( SuperproHandle, apipack );
   error = error & 0xFFFF;  /* MAKE SURE THE HIGH WORD IS BLANK */
#endif


      /***  End the Critical Section handling  ***/
   regs.w.ax = 0x1682;
   int86( 0x2F, &regs, &regs);

   return(error);
}

#endif

#ifdef __WATCDLL__    /* IF WATCOMC DLL */
 /****************************************************************************
 *
 *  Function: LoadDLL (only for Watcom 32-bit Windows DLL development)
 *  Purpose : This is the function to load a 16-bit DLL and give a function
 *               handle to the function, "SUPERPRO".
 *
 *  Inputs      :  NONE
 *
 *  Outputs     :  NONE
 *
 *  Return Value:  result of the LoadDLL function.
 *                  false on error.
 *
 **************************************************************************/

int LoadDLL( void)

{

   HANDLE dll;
   FARPROC addr;

   dll = LoadLibrary ( "superpro.dll");
   if ( dll < 32) return (FALSE);
   addr = GetProcAddress( dll, "SUPERPRO");
   SuperproHandle = GetIndirectFunctionHandle( addr, INDIR_PTR, INDIR_ENDLIST);
   return( TRUE);

}

#endif

#ifdef _INTELC32_
 /****************************************************************************
 *
 *  Function: cSUPERPRO  (only for the Intel 386/486 C Code Builder Kit)
 *  Purpose : To avoid the problem raised by the C compiler not handling the
 *               Pascal calling convention, where SUPERPRO is written with
 *               that convention in mind.
 *
 *  Inputs      :  *ApiPacket - ptr to the information needed for the
 *                    function SUPERPRO.
 *  Outputs     :  NONE
 *  Return Value:  result of the SUPERPRO function.
 *
 **************************************************************************/

/* The Intel 386/486 C Code Builder Kit doesn't know about the Pascal calling
   convention, but that's the only way one can call SUPERPRO.  It turns out
   that using the C Code Builder Kit compiler to compile a passthrough
   function around SUPERPRO solves this, as follows.

   There are only two differences between the Pascal calling convention and
   the C calling convention: the order in which parameters are passed, and
   who pops the parameters off the stack.  The order in which parameters are
   passed is not relevant for SUPERPRO, because there is only one parameter.
   This leaves only the question of who pops the parameters off the stack.
   In Pascal, the callee pops them; in C, the caller pops them.  So if we
   were to call SUPERPRO directly, we would pop the parameter after return,
   even though it was popped already by SUPERPRO.  A Bad Thing.

   But if we simply use a passthrough function, then that function uses the
   LEAVE instruction just after calling SUPERPRO and just before returning,
   as its method of popping the parameters off the stack.  It pops off the
   stack exactly what remains in this stack frame.  If we've been tricky and
   called a Pascal-style routine, there will be no parameters left on the
   stack; the LEAVE will do nothing harmful.

   This trick will break if the C Code Builder Kit is later revised so that
   the compiler generates different code.
*/

int cSUPERPRO(APIPACKET *xxx)
{
        return SUPERPRO(xxx);

} /* cSUPERPRO() */
#endif

