/*
 **********************************************************
 * OLDDEFS.H - Old service macros supplied to provide
 * compatibility with older releases of the HASP API.
 *
 **********************************************************
 */
#ifndef OLDDEFS_H_
#define OLDDEFS_H_

#if !defined(HASPAPI_H_)
#error Do not include OLDDEFS.H directly. Include HASP.H instead
#endif

enum
{
   /*
    *  Services common to all local HASPs.
    */
   IS_HASP          = LOCALHASP_ISHASP,
   GET_HASP_CODE    = LOCALHASP_HASPCODE, 
   GET_HASP_STATUS  = LOCALHASP_HASPSTATUS,

   /*
    *  HASP-3, MemoHASP, HASP36, and MemoHASP36 services.
    */
   READ_HASP_MEMO        =  MEMOHASP_READMEMO,
   WRITE_HASP_MEMO       =  MEMOHASP_WRITEMEMO,
   GET_HASP_IDNUM        =  MEMOHASP_HASPID,
   READ_HASP_MEMO_BLOCK  =  MEMOHASP_READBLOCK,
   WRITE_HASP_MEMO_BLOCK =  MEMOHASP_WRITEBLOCK,

   /*
    *  TimeHASP memory services.
    */
   READ_TIME_MEMO          =  TIMEHASP_READMEMO,
   WRITE_TIME_MEMO         =  TIMEHASP_WRITEMEMO,
   GET_TIME_IDNUM          =  TIMEHASP_HASPID,
   READ_TIME_MEMO_BLOCK    =  TIMEHASP_READBLOCK,
   WRITE_TIME_MEMO_BLOCK   =  TIMEHASP_WRITEBLOCK,

   /*
    *  TimeHASP clock services.
    */
   SET_TIME  =  TIMEHASP_SETTIME,
   GET_TIME  =  TIMEHASP_GETTIME,
   SET_DATE  =  TIMEHASP_SETDATE,
   GET_DATE  =  TIMEHASP_GETDATE,

   /*
    *  NetHASP network services.
    */
   CHECK_LAST_STATUS     =  NETHASP_LASTSTATUS,
   GET_NETHASP_CODE      =  NETHASP_HASPCODE,
   NETHASP_SET_IDLETIME  =  NETHASP_SETIDLETIME,
   SET_IDLE_TIME         =  NETHASP_SETIDLETIME,

   /*
    *  NetHASP memory services.
    */
   READ_NETHASP_MEMO        =  NETHASP_READMEMO,
   WRITE_NETHASP_MEMO       =  NETHASP_WRITEMEMO,
   GET_NETHASP_IDNUM        =  NETHASP_HASPID,
   READ_NETHASP_MEMO_BLOCK  =  NETHASP_READBLOCK,
   WRITE_NETHASP_MEMO_BLOCK =  NETHASP_WRITEBLOCK
};


#endif /* OLDDEFS_H_ */

