#ifndef __spromeps_h  // Sentry, use file only if it's not already included.
#define __spromeps_h

/****************************************************************************
 *                  SuperPro Multiple Entry Points Header file                *
 ****************************************************************************
 * This Header defines alternate functions for performing SuperPro API     *
 * commands using C.  It provides a function for each API command, so you  * 
 * do not have to deal with command packets.                                       *
 ****************************************************************************/

/* (C) Copyright 1989, 1995  Rainbow Technologies Inc. All Rights Reserved   */

#define DEFAULT_PORT_ADDRESS1   0x0
#define DEFAULT_PORT_ADDRESS2   0x0
#define DEFAULT_PORT_ADDRESS3   0x0

#if defined(__BORLANDC__) || defined(__TURBOC__)
#define _RB_FAR_ far
#elif defined(_MSC_VER)
#define _RB_FAR_ _far
#else
#define _RB_FAR_
#endif

/*****  Function Prototypes  *****/

/****************************************************************************
 *
 *  Function: sproInitialize
 *  Purpose : To initialize the SuperPro Driver
 *
 *  Inputs      : NONE
 *  Outputs     : NONE
 *  Return Value: Status 
 *
 ****************************************************************************/
int sproInitialize(void);


/****************************************************************************
 *
 *  Function: sproCfgLibParams
 *  Purpose : To configure the SuperPro driver
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *                 libParams - ptr to a RB_SPRO_LIB_PARAMS structure.
 *  Outputs     : Status word in function result
 *  Return Value: Status (Word)
 *
 ****************************************************************************/
int sproCfgLibParams(UNITINFO   *unitinfo,
                     RBP_SPRO_LIB_PARAMS libParams);

 /****************************************************************************
 *
 *  Function: sproGetExtendedStatus
 *  Purpose : Return the Status (Word) from the previous SuperPro API Call.
 *
 *  Inputs      : *unitInfo - ptr to the current unit info handle structure.
 *  Outputs     : Status word in function result
 *  Return Value: Status (Word)
 *
 **************************************************************************/
int sproGetExtendedStatus(UNITINFO *unitinfo);

 /****************************************************************************
 * Function    : sproGetVersion
 *
 * Purpose     : Returns the driver's version number.
 *
 * Inputs      : thePacket  - pointer to a user allocated API packet.
 *               majVer     - is a pointer to where to store the major version.
 *               minVer     - is a pointer to where to store the minor version.
 *               rev        - is a pointer to where to store the revision.
 *               osDrvrType - is a pointer to where to store the os driver type.
 *
 * Outputs     : majVer     - is a pointer to where to store the major version.
 *               minVer     - is a pointer to where to store the minor version.
 *               rev        - is a pointer to where to store the revision.
 *               osDrvrType - is a pointer to where to store the os driver type.
 *
 * Return Value: Status
 *
 **************************************************************************/
int sproGetVersion(UNITINFO *unitinfo,
                   unsigned char *majVer,
                   unsigned char *minVer,
                   unsigned char *rev,
                   unsigned char *osDrvrType );

 /****************************************************************************
 *
 *  Function: sproFindFirstUnit
 *  Purpose : Find a SuperPro with the specified developer ID and return a
 *            a unit handle.
 *
 *  Inputs      : developerID - your assigned 16 bit developer.
 *             
 *  Outputs     : *unitinfo - ptr to a unit info.
 *
 *  Return Value: Status 
 *
 **************************************************************************/
int sproFindFirstUnit(unsigned int developerID, UNITINFO *unitinfo);

 /****************************************************************************
 *
 *  Function: sproFindNextUnit
 *  Purpose : To find the next SuperPro unit with the same developer ID 
 *            as the unit specified by the unitinfo parameter, and return 
 *            a new unit handle.
 *
 *  Inputs      : *unitinfo - ptr to the current 32 bit unit handle.
 *  Outputs     : *unitinfo - ptr to a returned new 32 bit unit handle.
 *  Return Value: Status 
 *
 **************************************************************************/
int sproFindNextUnit(UNITINFO *unitinfo);

 /****************************************************************************
 *
 *  Function: sproRead
 *  Purpose : To read the data in a readable SuperPro Cell
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
 *                 address - address 0-63 of the cell of interest.
 *  Outputs     : *data - ptr to 16 bit contents of the addressed cell.
 *  Return Value: Status 
 *
 **************************************************************************/
int sproRead(UNITINFO *unitinfo, int address, unsigned int *data);

 /****************************************************************************
 *
 *  Function: sproExtendedRead
 *  Purpose : To read the data in a readable SuperPro Cell
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
 *                 address - address 0-63 of the cell of interest.
 *  Outputs     : *data - ptr to 16 bit contents of the addressed cell.
 *                *accessCode - ptr to access code value (0-3) of the  
 *                              addressed cell 
 *  Return Value: Status 
 *
 **************************************************************************/
int sproExtendedRead(UNITINFO      *unitinfo,
                     int           address,
                     unsigned int  *data,
                     unsigned char *accessCode);

 /****************************************************************************
 *
 *  Function: sproWrite
 *  Purpose : To write to a read/write Superpro Cell. 
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
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
int sproWrite(UNITINFO      *unitinfo,
              unsigned int  writePassword,
              int           address,
              unsigned int  data,
              unsigned char accessCode);
          
 /****************************************************************************
 *
 *  Function: sproOverwrite
 *  Purpose : To Overwrite any Overwriteable Superpro Cell (cells 5-63)
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
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
int sproOverwrite(UNITINFO      *unitinfo,
                  unsigned int  writePassword,
                  unsigned int  overwritePassword1,
                  unsigned int  overwritePassword2,
                  int           address,
                  unsigned int  data,
                  unsigned char accessCode);
              
 /****************************************************************************
 *
 *  Function: sproDecrement
 *  Purpose : This function decrements a specified cell.  If the cell
 *            decrements to 0 and there is an associated active algo 
 *            descriptor, the algo descriptor is deactivated.
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
 *                 writePassword      - 16 bit write password
 *                 address    - address 0-63 of the cell to decrement.
 *  Outputs     : NONE
 *  Return Value: Status 
 *
 **************************************************************************/
int sproDecrement(UNITINFO     *unitinfo,
                  unsigned int writePassword,
                  int          address);
          
 /****************************************************************************
 *
 *  Function: sproActivate
 *  Purpose : To activate an inactive algo descriptor.
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
 *                 writePassword      - 16 bit write password
 *                 activatePassword1  - 16 bit activate password 1
 *                 activatePassword2  - 16 bit activate password 2
 *                 address    - address 0-63 of the cell to decrement.
 *  Outputs     : NONE
 *  Return Value: Status 
 *
 **************************************************************************/
int sproActivate(UNITINFO      *unitinfo,
                 unsigned int  writePassword,
                 unsigned int  activatePassword1,
                 unsigned int  activatePassword2,
                 int           address);
             
 /****************************************************************************
 *
 *  Function: sproQuery
 *  Purpose : to scramble a bit stream through the superpro.
 *
 *  Inputs      :  unitinfo - ptr to the current 32 bit unit handle.
 *                 address    - address of the algo descriptor to use.
 *                 *queryData - ptr to input string
 *                 *response  - ptr to returned response string
 *                 *response32 - last 32 bits of response data
 *                 length - length of the input (and response) string.
 *  Outputs     : NONE
 *  Return Value: Status 
 *
 **************************************************************************/
int sproQuery(UNITINFO               *unitinfo,
              int                     address,
              void                   *queryData,
              void          _RB_FAR_ *response,
              unsigned long _RB_FAR_ *response32,
              unsigned int            length);



#ifdef __WATCDLL__     /* IF WATCOMC DLL */
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

int LoadDLL( void);

#endif

#endif   // include file sentinel

