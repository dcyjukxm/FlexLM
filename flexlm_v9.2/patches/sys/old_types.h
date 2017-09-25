/****************************************************************************
*
* (C) Unpublished Copyright Novell, Inc. All Rights Reserved.
* (c) Copyright.  1988 WATCOM Systems Inc. All rights reserved.
*
* No part of this file may be duplicated, revised, translated,
* localized or modified in any manner or compiled, linked or 
* uploaded or downloaded to or from any computer system without
* the prior written consent of Novell, Inc.
*
* NetWare C NLM Runtime Library source code
*
*****************************************************************************/

/*----------------------------------------------------------------------------*
 *                                                                            *
 *      NetWare 386 Developer's C Runtime library                             *
 *                                                                            *
 *      This include file defines local types.                                *
 *                                                                            *
 *      This include file also has some Berkley socket related definitions.   *
 *                                                                            *
 *      (c) Copyright.  1989-1990 Novell, Inc.   All rights reserved.         *
 *      (c) Copyright.  1988 WATCOM Systems Inc. All rights reserved.         *
 *                                                                            *
 *----------------------------------------------------------------------------*/

#ifndef _TIME_T_DEFINED_

   typedef unsigned long time_t;   /* time value */
   #define _TIME_T_DEFINED_

#endif

#ifndef _TYPES_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

typedef long ino_t;             /* i-node # type */
typedef long dev_t;             /* device code (drive #) */
typedef long off_t;             /* file offset value */


/*
 * Traditional Unix typedefs
 */

#ifndef ULONG_DEFINED // CCK - modified for FLEXlm
#define ULONG_DEFINED
typedef unsigned long  ulong;
#endif

#ifndef UINT_DEFINED // CCK - modified for FLEXlm
#define UINT_DEFINED
typedef unsigned int   uint;
#endif

#ifndef USHORT_DEFINED // CCK - modified for FLEXlm
#define USHORT_DEFINED
typedef unsigned short ushort;
#endif

#ifndef U_LONG_DEFINED // CCK - modified for FLEXlm
#define U_LONG_DEFINED
typedef unsigned long  u_long;
#endif

#ifndef U_INT_DEFINED // CCK - modified for FLEXlm
#define U_INT_DEFINED
typedef unsigned int   u_int;
#endif

#ifndef U_SHORT_DEFINED // CCK - modified for FLEXlm
#define U_SHORT_DEFINED
typedef unsigned short u_short;
#endif

#ifndef ULONG_DEFINED // CCK - modified for FLEXlm
#define ULONG_DEFINED
typedef unsigned char  u_char;
#endif

#ifndef CADDR_T_DEFINED // CCK - modified for FLEXlm
#define CADDR_T_DEFINED
typedef void *         caddr_t;
#endif


#ifndef NLM	// CCK - modified for FLEXlm
/* 
 * BSD socket defines, structures, macros, and prototypes.
 */

/* BSD socket select.  FD_SETSIZE is number of BSD file descriptors.
 */
#define	FD_SETSIZE	16

typedef	long	fd_array[FD_SETSIZE];
typedef	struct fd_set 
   {
	fd_array	fds;
   } fd_set;

#define	FD_SET( n, p )	   bsd_fd_set(n, p)
#define	FD_CLR( n, p )	   bsd_fd_clr(n, p)
#define	FD_ISSET( n, p )	bsd_fd_isset(n, p)
#define	FD_ZERO(p)	memset((void *)(p), 0, sizeof(*(p)))

extern void bsd_fd_set( 
         int     n, 
         fd_set *p );

extern void bsd_fd_clr( 
         int     n, 
         fd_set *p );

extern int  bsd_fd_isset( 
         int     n, 
         fd_set *p );
#endif

#define _TYPES_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
