/******************************************************************************

	    COPYRIGHT (c) 1996 by Globetrotter Software Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Globetrotter Software Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Globetrotter Software Inc.

 *****************************************************************************/
/*	
 *	Module:	l_sgi.s v1.4.0.0
 *
 *	Function:	flexsyssgi()
 *
 *	Description: 	Used by lm_getid_type()
 *
 *	D. Birns
 *	11/7/96
 *
 *	Last changed:  11/12/96
 *
 */
#ifdef SGI
#include <sys/regdef.h>
#include <sys/asm.h>
#include <sys.s>
#include "sys/syscall.h"
 
GSYSCALL(flexsyssgi, syssgi)
	 RET(flexsyssgi)
#endif

