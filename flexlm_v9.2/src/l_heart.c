/******************************************************************************

	    COPYRIGHT (c) 1996, 2003  by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/*	
 *	Module: $Id: l_heart.c,v 1.3 2003/01/13 22:41:52 kmaclean Exp $
 *
 *	Function:	l_heartbeat 
 *
 *	Description: 	processes heartbeat message
 *
 *	Return:
 *
 *	D. Birns
 *	7/22/96
 *
 *	Last changed:  7/25/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#ifndef NO_FLEXLM_CLIENT_API 
#include "lm_comm.h"
#ifdef CPU_USAGE
#ifdef UNIX
#include <sys/types.h>
#include <sys/times.h>
#ifdef ANSI
#include <unistd.h>
#endif /* ANSI */
#endif /* UNIX */
#ifdef VMS
#include <jpidef.h>
struct ilist {short length; short code; int *retadr; int *retlen; int zero; };
int sys$getjpiw();
#endif /* VMS */
#ifdef OS2
#include "cpu_os2.h"
#endif /* OS2 */
#endif /* CPU_USAGE */
static int get_clock_tick lm_args((lm_noargs));

int
l_heartbeat(job, in, out)
LM_HANDLE *job;
char *in;
char *out;
{
#ifdef CPU_USAGE
  int cpu_usages[4];
#ifdef UNIX
  struct tms t;
  int clk_tck;
#endif /* UNIX */

#ifdef WINNT
FILETIME Creation,Exit,System,User;
HANDLE h;
#endif /* WINNT */

#ifdef VMS
  struct ilist itmlist;
  int ticks, retl;
#endif /* VMS */

#ifdef OS2
  unsigned long usageSys, usageUser;
#endif /* OS2 */
#endif /* CPU_USAGE */

	memset(out, '\0', LM_MSG_LEN+1);

#ifdef CPU_USAGE
	memset(cpu_usages, 0, sizeof(cpu_usages));
	out -= MSG_DATA;

#ifdef WINNT
#define GOTONE
/* Low order 100 ns tics per .1 seconds */
#define LOW_ORDER_PER_TENTH_SEC  1000000  

/* High Order word conversion 2^32/1000000 */
#define HIGH_ORDER_PER_TENTH_SEC 4295    
	
	h=GetCurrentProcess();
	GetProcessTimes(h,&Creation,&Exit,&System,&User); 
	cpu_usages[0]= System.dwHighDateTime*HIGH_ORDER_PER_TENTH_SEC  +
			(System.dwLowDateTime/LOW_ORDER_PER_TENTH_SEC);

	cpu_usages[1]= User.dwHighDateTime*HIGH_ORDER_PER_TENTH_SEC
		+ (User.dwLowDateTime/LOW_ORDER_PER_TENTH_SEC);
	
#endif /* WINNT */

#ifdef VMS
#define GOTONE
	itmlist.length = (short)4;
	itmlist.code = (short)JPI$_CPUTIM;
	itmlist.retadr = &ticks;
	itmlist.retlen = &retl;
	itmlist.zero = 0;
	(void)sys$getjpiw(0, 0, 0, &itmlist, 0, 0, 0);
	cpu_usages[0] = ticks / get_clock_tick();
	cpu_usages[1] = 0;
	cpu_usages[2] = 0;
	cpu_usages[3] = 0;
#endif /* VMS */

#ifdef OS2
#define GOTONE
	if( 0 == OS2_GetCPUTimeUsed( &usageSys, &usageUser ) )
	{
		cpu_usages[0] = (int) usageSys;
		cpu_usages[1] = (int) usageUser;
	}
#endif /* OS2 */

#ifndef GOTONE
	times(&t);

	if ((clk_tck = get_clock_tick()) > 0)
	{
		cpu_usages[0] = t.tms_utime / clk_tck;
		cpu_usages[1] = t.tms_stime / clk_tck;
		cpu_usages[2] = t.tms_cutime / clk_tck;
		cpu_usages[3] = t.tms_cstime / clk_tck;
	} 
#endif /* GOTONE */

	sprintf(&out[MSG_HB_CPU_USAGE_1], "%x", cpu_usages[0]);
	sprintf(&out[MSG_HB_CPU_USAGE_2], "%x", cpu_usages[1]);
	sprintf(&out[MSG_HB_CPU_USAGE_3], "%x", cpu_usages[2]);
	sprintf(&out[MSG_HB_CPU_USAGE_4], "%x", cpu_usages[3]);
#endif /*CPU_USAGE*/
	return 1;
}
#endif /* NO_FLEXLM_CLIENT_API */


/*
 *	get_clock_tick - 
 *	machine dependent code
 *	times() returns values in "ticks" which is, on unix,
 *	set to sysconf(_SC_CLK_TCK).  On most systems this is 60.
 *	this means that times() returns values in 1/60 of a second.
 *	So if times() returns 60, it means 1 second.
 *	This function returns the number of ticks in 1/10th of a second.
 *	On most unix systems, this will be '6'
 */
static 
int
get_clock_tick(lm_noargs)
{
#ifdef SUNOS4
	return 6;
#endif
#if defined(ANSI) && defined(UNIX)
	return (sysconf(_SC_CLK_TCK) / 10);
#endif
#ifdef VMS
	return 10;
#endif /* VMS */
#ifdef WINNT
	return 1000000;
#endif /* WINNT */
#ifdef PC16
	return 10;
#endif
}
