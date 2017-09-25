#ifndef __superpro_h              // Sentry, use file only if it's not already included.
#define __superpro_h

/* superpro.h
   (C) Copyright 1986-1995  Rainbow Technologies Inc. All Rights Reserved  */

/**  SuperPro API functions.  **/

#define API_INITIALIZE          0
#define API_FIND_FIRST_UNIT     1
#define API_FIND_NEXT_UNIT      2
#define API_READ                3
#define API_EXTENDED_READ       4
#define API_WRITE               5
#define API_OVERWRITE           6
#define API_DECREMENT           7
#define API_ACTIVATE_ALGORITHM  8
#define API_QUERY               9
#define API_GET_VERSION         11
#define API_GET_EXTENDED_STATUS 12
#define API_CONFIG              13

/*
 SuperPro configuration's cmd values to SET or GET library parameters
*/
#define SPD_GET_LIB_PARAMS_CMD 0
#define SPD_SET_LIB_PARAMS_CMD 1

/*
 SuperPro configuration's func values.
*/
#define SPD_DRVR_FLAGS_FUNC    0
#define SPD_HW_TYPE_FUNC       1
#define SPD_PORT_FUNC          2
#define SPD_PIC_FUNC           3

/*
 SuperPro configuration's func-machine type.
*/
#define SPD_HW_AUTO_DETECT     0
#define SPD_HW_TYPE_IBM        1
#define SPD_HW_TYPE_NEC        2
#define SPD_HW_TYPE_FMR        4

/*
 SuperPro configuration's func-router type:
*/
#define SPD_USE_LINKED_DRVR 0x20
#define SPD_USE_SYSTEM_DRVR 0x40

/* Used with SPD_PORT_FUNC
*/
#define SPD_PORT_1             0
#define SPD_PORT_2             1
#define SPD_PORT_3             2
#define SPD_PORT_4             3

/* Used with SPD_PIC_FUNC
*/
#define SPD_PIC_1              0
#define SPD_PIC_2              1



/**  SuperPro API error codes.  **/

#define SP_SUCCESS                      0
#define SP_INVALID_FUNCTION_CODE        1
#define SP_INVALID_PACKET               2
#define SP_UNIT_NOT_FOUND               3
#define SP_ACCESS_DENIED                4
#define SP_INVALID_MEMORY_ADDRESS       5
#define SP_INVALID_ACCESS_CODE          6
#define SP_PORT_IS_BUSY                 7
#define SP_WRITE_NOT_READY              8
#define SP_NO_PORT_FOUND                9
#define SP_ALREADY_ZERO                 10

#define SP_MAX_STATUS                   10

#define USE_STI_CLI                     0X01
#define USE_PIC_REGS                    0X02
#define USE_WIN_CRITICAL                0X04

#define DEFAULT_INT_METHODS             (USE_PIC_REGS | USE_WIN_CRITICAL | SPD_USE_LINKED_DRVR | SPD_USE_SYSTEM_DRVR)

/*
There are 3 methods of disabling interrupts, each method is represented by a
bit in DEFAULT_INT_METHODS. Any combination of methods may be used, but it
does not make much sense to use the PIC if CLI/STI is being used since
CLI/STI affects all the interrupts in the PIC register. BITS NOT DEFINED
IN DEFAULT_INT_METHODS ARE RESERVED AND SHOULD ALWAYS BE 0.


USE_STI_CLI      => Tells the SuperPro driver to use STI/CLI to disable ALL
                    interrupts when using the parallel port.

USE_PIC_REGS     => Tells the SuperPro driver to use the PIC register to
                    Disable interrupts. This is done by using DEFAULT_PIC_MASK1
                    and DEFAULT_PIC_MASK2, which are defined below).

USE_WIN_CRITICAL => Tells the SuperPro driver to issue a DOS system call
                    to start and end critical sections. This is only useful
                    under Windows or a Window's DOS box. This has nothing to
                    do with interrupts, instead it prevents the code inside
                    the critical section from being executed if another
                    process is already executing it.
*/

#define DEFAULT_PIC_MASK1               0X81    /* Mask Timer Tick & Printer */
#define DEFAULT_PIC_MASK2               0X00    /* Don't mask any            */

/*
Here are the definitions of the bits in the 2 PIC registers. Setting a bit
to 1 disables the corresponding interrupt and setting a bit to 0 enables the
corresponding interrupt.

      PIC MASK 1                    PIC MASK 2
  HARDWARE INTERRUPTS           HARDWARE INTERRUPTS            BIT IN PIC
  -------------------           -------------------            ----------
  IRQ0    TIMER TICK            IRQ8    REAL TIME CLOCK           0X01
  IRQ1    KEYBOARD              IRQ9    IRQ2 REDIRECT             0X02
  IRQ2    SLAVE CONTROLLER      IRQ10                             0X04
  IRQ3    COM2                  IRQ11                             0X08
  IRQ4    COM1                  IRQ12   IBM MOUSE EVENT           0X10
  IRQ5    FIXED DISK            IRQ13   MATH COPROCESSOR ERROR    0X20
  IRQ6    FLOPPY DISK           IRQ14   HARD DISK                 0X40
  IRQ7    PRINTER               IRQ15                             0X80

*/

/**  Structure of a unit's handle.  **/

typedef struct {
  unsigned short developerID;     /* unit's developer ID                  */
  unsigned short serialNumber;    /* serial number                        */
  unsigned short port;            /* port address used by the key         */
  unsigned char  reserved[18];    /* reserved area (for internal use)     */
} UNITINFO;

#define SP_STDCALL
#define SP_FASTCALL
#define SP_PASCAL
#define SP_CDECL
#define SP_FAR

#define SP_PTR SP_FAR *

#define SP_IN
#define SP_OUT
#define SP_IO

#if !defined(_RBTYPES_INC)

typedef                void RB_VOID;
typedef unsigned       char RB_BOOLEAN;
typedef unsigned       char RB_BYTE;
typedef unsigned short int  RB_WORD;
typedef unsigned long  int  RB_DWORD;
typedef RB_VOID    SP_PTR   RBP_VOID;
typedef RB_BYTE    SP_PTR   RBP_BYTE;
typedef RB_BOOLEAN SP_PTR   RBP_BOOLEAN;
typedef RB_WORD    SP_PTR   RBP_WORD;
typedef RB_DWORD   SP_PTR   RBP_DWORD;

#endif

typedef struct
{
SP_IO RB_WORD portNum;
SP_IO RB_WORD portAddr;
}SP_CFG_PORT, *SP_PTR_CFG_PORT;

typedef struct
{
SP_IO RB_WORD picNum;
SP_IO RB_WORD picMask;
}SP_CFG_PIC, *SP_PTR_CFG_PIC;


/* This union maps to APIpacket.params, therefore it must be the same size! */
typedef union  _RB_SP_CFG_PARAMS {
SP_IO RB_WORD        hwType;           /* machine type: IBM, NEC, or FMR*/
SP_IO RB_WORD        drvrFlags;        /* request routing flags         */
SP_CFG_PORT          portData;         /* port config info              */
SP_CFG_PIC           picData;          /* pick mask config info         */
SP_IO RB_WORD        data;             /* general WORD ref. label       */
SP_IO RB_DWORD       DWdata;           /* general D-WORD ref. label.    */
} RB_SP_CFG_PARAMS, *SP_PTR RBP_SP_CFG_PARAMS;

typedef struct _RB_SPRO_LIB_PARAMS {                                  /*bytes*/
  SP_IN RB_WORD   cmd;                 /* command - set/get parameters  0-1 */
  SP_IN RB_WORD   func;                /* function to set/get           2-3 */
  SP_IO RB_SP_CFG_PARAMS params;       /* parameters to set/get         4-7 */
} RB_SPRO_LIB_PARAMS, *RBP_SPRO_LIB_PARAMS;

typedef struct {
  unsigned char  functionCode;    /* Function code                        */
  unsigned char  intMode;         /* Method for disabling interrupts      */
  unsigned short portAddrs[4];    /* addresses of port LPT1 - LPT4        */
  unsigned char  reserved1[10];   /* reserved for internal use            */
  unsigned short delay5usec;      /* loops to make at least 5 microsecond */
  unsigned short delay100usec;    /* loops to make at least 100 microsecon*/
  UNITINFO       ui;              /* current unit information             */
  /* . . . . . . . . . . command specific variables . . . . . . . . . . . */
  unsigned short writePassword;   /*                                      */
  unsigned short xtraPassword1;   /*                                      */
  unsigned short xtraPassword2;   /*                                      */
  unsigned short memoryAddress;   /*                                      */
  unsigned short memoryContents;  /*                                      */
  unsigned char  status;          /* Status value returned from driver    */
  unsigned char  accessCode;      /*                                      */
  unsigned long  QueryIn;         /*                                      */
  unsigned long  QueryOut;        /*                                      */
  unsigned long  longResult;      /*                                      */
  unsigned short dataLength;      /*                                      */
  unsigned char  InitialPICMask1; /* PIC mask 1 for API_INITIALIZE only   */
  unsigned char  InitialPICMask2; /* PIC mask 2 for API_INITIALIZE only   */
  unsigned short cmd;             /* Set/Get config information           */
  unsigned short func;            /* opcode indicating the parameter      */
  unsigned long  params;          /* parameter data                       */
  unsigned char  reserved2[20];   /*                                      */
  unsigned char  reserved3[516];  /*                                      */
} APIPACKET;



/**  SuperPro API function prototype.  **/

#ifdef __ZORTC__
#define RNBO_API _pascal
#define APIPACKETPTR APIPACKET *
#define RNBO_FPTR far
#else
#ifdef  __WATC__
#define RNBO_API _pascal
#define APIPACKETPTR APIPACKET *
#define RNBO_FPTR far
#else
#ifdef  __HIGHC__
#define RNBO_API _CC(_DEFAULT_CALLING_CONVENTION | _CALLEE_POPS_STACK)
#define APIPACKETPTR APIPACKET *
#define RNBO_FPTR far
#else
#ifdef __32BIT__
#define RNBO_API _Far16 _Pascal
#define APIPACKETPTR  _Seg16 APIPACKET far *
#define RNBO_FPTR far
#else
#ifdef _INTELC32_
#define RNBO_API
#define APIPACKETPTR APIPACKET *
#define RNBO_FPTR far
#else
#ifdef  __MSVC32__
#define RNBO_API  __cdecl
#define APIPACKETPTR APIPACKET *
#define RNBO_FPTR
#else
#define RNBO_API far pascal
#define APIPACKETPTR APIPACKET far *
#define RNBO_FPTR far
#endif
#endif
#endif
#endif
#endif
#endif

#ifdef CPP
extern "C" { int RNBO_API SUPERPRO( APIPACKETPTR ); };
#else
extern int RNBO_API SUPERPRO( APIPACKETPTR );
#endif

#endif // sentry



