/****************************************************************************
*
* (C) Unpublished Copyright Novell, Inc. All Rights Reserved.
* (c) Copyright 1982, 1986 Regents of the University of California.
*     All rights reserved.
*
* No part of this file may be duplicated, revised, translated,
* localized or modified in any manner or compiled, linked or 
* uploaded or downloaded to or from any computer system without
* the prior written consent of Novell, Inc.
*
* NetWare C NLM Runtime Library source code
*
*****************************************************************************/

#ifndef _SYS_TIME_H_INCLUDED

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval
   {
   long  tv_sec;     /* seconds */
   long  tv_usec;    /* and microseconds */
   };

#define _SYS_TIME_H_INCLUDED
#endif  /* _SYS_TIME_H_INCLUDED */
