/****************************************************************************
*
* (C) Unpublished Copyright Novell, Inc. All Rights Reserved.
* (c) Copyright by WATCOM Systems Inc. 1988.  All rights reserved.
*
* No part of this file may be duplicated, revised, translated,
* localized or modified in any manner or compiled, linked or 
* uploaded or downloaded to or from any computer system without
* the prior written consent of Novell, Inc.
*
* NetWare C NLM Runtime Library source code
*
*****************************************************************************/
      
#ifndef _TYPES_H_INCLUDED
 #include <sys\types.h>
#endif
#ifndef _UTIME_H_INCLUDED

struct utimbuf
   {
   time_t      acctime;        /* access time */
   time_t      modtime;        /* modification time */
   };

#ifdef __cplusplus
extern "C" {
#endif

int   utime(
         const char *,
         struct utimbuf * );

#ifdef __cplusplus
}
#endif

#define _UTIME_H_INCLUDED
#endif
