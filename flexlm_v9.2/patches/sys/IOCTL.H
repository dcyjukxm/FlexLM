/****************************************************************************
*
* (C) Unpublished Copyright Novell, Inc. All Rights Reserved.
*
* No part of this file may be duplicated, revised, translated,
* localized or modified in any manner or compiled, linked or 
* uploaded or downloaded to or from any computer system without
* the prior written consent of Novell, Inc.
*
* NetWare C NLM Runtime Library source code
*
* BSD file ioctl opcodes, vastly simplified for NW/386.
*
*****************************************************************************/

#ifndef _SYS_IOCTL_H_INCLUDED

/*----------------------------------------------------------------------------*
 *                                                                            *
 *      NetWare 386 Developer's C Runtime library                             *
 *                                                                            *
 *      BSD file ioctl opcodes, vastly simplified for NW/386.                 *
 *                                                                            *
 *      (c) Copyright.  1989-1991 Novell, Inc.  All rights reserved.          *
 *                                                                            *
 *----------------------------------------------------------------------------*/

/* @(#)ioctl.h 2.17 88/02/08 SMI; from UCB 7.1 6/4/86 */

/*
** Copyright (c) 1982, 1985, 1986 Regents of the University of California.
** All rights reserved.  The Berkeley software License Agreement
** specifies the terms and conditions for redistribution.
*/

#include <io.h>
#include <sys/filio.h>
#include <sys/sockio.h>

#define _SYS_IOCTL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int ioctl( int fd, int request, ... ); // (only when 'fd' is a socket...)

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_IOCTL_H_INCLUDED */
