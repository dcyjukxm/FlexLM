/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

/*****************************************************************************/
/*
 *	 Module: $Id: stderr.h,v 1.2 2003/01/13 22:13:13 kmaclean Exp $
 *
 *	Description: cover fprintf( stderr, ... ) to PC debug output device.
 *
 *	Chia-Chee Kuan
 *	8/4/94
 *
 *	Last changed:  4/11/95
 *
 */

#include <stdarg.h>
#define fprintf  fprintf_stderr

#undef stderr		
#define stderr 0

static
void
fprintf_stderr(dev, fmt, va_alist)	/* same args as printf */
int dev;
char *fmt;
va_list va_alist;
{
#ifndef OS2
	va_list pvar;

	va_start(pvar, fmt);

	(void) lm_debug_printf(fmt,va_alist);
	va_end(pvar);
#endif
}
