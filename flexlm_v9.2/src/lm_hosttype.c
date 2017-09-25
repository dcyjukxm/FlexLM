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

 *****************************************************************************/
/*
 *	Module: $Id: lm_hosttype.c,v 1.10 2003/05/31 03:13:10 jonl Exp $
 *
 *	Function: lc_hosttype(job, run_benchmark)
 *
 *	Description: 	Returns the current host type
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) run_benchmark - "Run benchmark" flag
 *				0 -> No benchmark, != 0 -> benchmark
 *
 *	Return:		(int) host type
 *
 *	M. Christiano
 *	3/16/90
 *
 *	Last changed:  5/31/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmhtype.h"
#include "lgetattr.h"

#define NO_PERF /* v7 -- disable this */

#undef USE_UNAME

/*
 *	Machine types
 */
#ifdef gotone
#undef gotone
#endif

typedef struct mtypetab { int mtype; HOSTTYPE data; } MTYPETAB;
#ifdef sun
#define gotone
static MTYPETAB mtypes[] = {
	{ 400, { LM_SOLBOURNE, "solbourne series 500",	0, 0}},
	{0x11, { LM_SUN3_75,	"Sun-3/75",		0, 0}}, /*+ 3/140, 3/150 */
	{0x12, { LM_SUN3_50,	"Sun-3/50",		0, 0}},
	{0x13, { LM_SUN3_260,	"Sun-3/260",		0, 0}}, /* + 3/280 */
	{0x14, { LM_SUN3_110,	"Sun-3/110",		0, 0}},
	{0x17, { LM_SUN3_60,	"Sun-3/60",		0, 0}},
	{0x18, { LM_SUN3_E,	"Sun-3e",		0, 0}},
	{0x21, { LM_SUN4_260,	"Sun-4/260",		0, 0}}, /* + 4/280 */
	{0x22, { LM_SUN4_110,	"Sun-4/110",		0, 0}}, /* + 4/150 */
	{0x23, { LM_SUN4_3xx,	"Sun-4/3xx",		0, 0}},
	{0x24, { LM_SUN4_470,	"Sun-4/470",		0, 0}}, /* + 4/490 */
	{0x31, { LM_SUN386I,	"Sun-386i",		0, 0}},
	{0x41, { LM_SUN3_460,	"Sun-3/460",		0, 0}}, /* +3/470, 3/480 */
	{0x42, { LM_SUN3_80,	"Sun-3/80",		0, 0}},
	{0x51, { LM_SUN4_60,	"Sun Sparcstation 1",	0, 0}},
	{0x52, { LM_SUN4_40,	"Sun-4/40",		0, 0}}, 
	{0x53, { LM_SUN4_65,	"Sun Sparcstation 1+",	0, 0}},
	{0x54, { LM_SUN4_20,	"Sun-4/20",		0, 0}}, /* SLC */
	{0x55, { LM_SUN4C,	"Sun Sparcstation 2",	0, 0}}, /*- sun4c */
	{0x61, { LM_SUN4_E,	"Sun-4e",		0, 0}}, 
	{11, { LM_SUNX86,	"Intel i86 PC",		0, 0}}, 
	{ 0, { LM_SUN,		"Sun",			0, 0}}
	};
#endif
#ifdef MAC10
static MTYPETAB mtypes[] = {
	{0, { LM_MAC,		"Mac",			0, 0 }}
	};
#define gotone
#endif
#ifdef NCR
static MTYPETAB mtypes[] = {
	{0, { LM_NCR,           "NCR",                  0, 0 }}
	};
#define gotone
#endif

#ifdef vax
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_VAX,		"vax",			0, 0 }}
	};
#endif

#ifdef HP
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_HP, 		"hp",			0, 0 }}
	};
#endif

#ifdef apollo
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_APOLLO, 	"apollo",		0, 0 }}
	};
#endif

#ifdef NeXT
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_NEXT, 		"NeXT",			0, 0 }}
	};
#endif

#ifdef SGI
#ifndef SYS_NMLN
#define SYS_NMLN 9
#endif
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_SGI, 		"sgi",			0, 0 }}
	};
#endif
#ifdef HAL
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
        { 0, { LM_HAL,          "hal",                  0, 0 }},
        };
#endif
#ifdef ALPHA_LINUX
#define gotone
static MTYPETAB mtypes[] = {
        { 0, { LM_LINUX,          "alpha_linux",                  0, 0 }},
        };

#else
#ifdef LINUX
#define gotone
static MTYPETAB mtypes[] = {
        { 0, { LM_LINUX,          "linux",                  0, 0 }},
        };
#endif /* LINUX */
#endif /* ALPHA_LINUX */
#ifdef sinix
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
        { 1, { LM_SNI,          "SNI",                  0, 0 }},
        { 2, { LM_RM400,        "SNI RM400",            0, 17 }},
        { 3, { LM_MX300I,       "SNI MX300I",           0, 25 }},
        { 0, { LM_SNI,          "SNI",                  0, 0 }},
        };
#endif

#ifdef NEC
static MTYPETAB mtypes[] = {
	{0, { LM_NEC, 		"NEC",			0, 0 }}
	};
#define gotone
#endif /* NEC */

#ifdef GENUINE_MIPS_MACHINE
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_MIPS, 		"MIPS",			0, 0 }}
	};
#endif

#ifdef DGUX
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{ 1, { LM_DG, 		"DG aviion",		0, 0  }},
	{ 2, { LM_AV3200,	"DG AV3200",		0, 17 }},
	{ 3, { LM_AV4000,	"DG AV4000",		0, 17 }},
	{ 4, { LM_AV4100,	"DG AV4100",		0, 20 }},
	{ 5, { LM_AV4020,	"DG AV4020",		0, 34 }},
	{ 6, { LM_AV4120,	"DG AV4120",		0, 40 }},
	{ 7, { LM_AV5010,	"DG AV5010",		0, 20 }},
	{ 8, { LM_AV5200,	"DG AV5200",		0, 25 }},
	{ 9, { LM_AV5220,	"DG AV5220",		0, 50 }},
	{10, { LM_AV6200,	"DG AV6200",		0, 25 }},
	{11, { LM_AV6220,	"DG AV6220",		0, 50 }},
	{12, { LM_AV6200_20,	"DG AV6200-20",		0, 25 }},
	{13, { LM_AV6220_20,	"DG AV6220-20",		0, 50 }},
	{14, { LM_AV200, 	"DG AV200",		0, 17 }},
	{15, { LM_AV300,	"DG AV300",		0, 17 }},
	{16, { LM_AV310,	"DG AV310",		0, 20 }},
	{17, { LM_AV400,	"DG AV400",		0, 17 }},
	{18, { LM_AV402,	"DG AV402",		0, 34 }},
	{19, { LM_AV410,	"DG AV410",		0, 20 }},
	{20, { LM_AV412,	"DG AV412",		0, 40 }},
	{ 0, { LM_DG,		"DG aviion",		0, 0}}
	};
#endif

#ifdef RS6000
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_RS6000, 	"IBM RS/6000",		0, 0 }}
	};
#endif

#ifdef PMAX
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_DEC, 	"DEC MIPS Ultrix",		0, 0 }}
	};
#endif

#if defined(WINNT) && defined(_MIPS_)
#define gotone
static MTYPETAB mtypes[] = {
        {0, { LM_MIPS,  "MIPS Windows/NT",              0, 0 }}
        };
#endif

#if (defined(ALPHA)||defined(_ALPHA_)) && !defined(VMS)
#define gotone
#ifdef WINNT
static MTYPETAB mtypes[] = {
	{0, { LM_ALPHA_WINNT, 	"DEC Alpha Windows/NT",		0, 0 }}
	};
#else	/* !WINNT */
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_ALPHA, 	"DEC OSF",		0, 0 }}
	};
#endif	/* WINNT */
#endif	/* ALPHA */

#ifdef MOTO_88K
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_MOTO88K, 	"Motorola 88k",		0, 0 }}
	};
#endif
#ifdef MOTOSVR4
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_MOTOSVR4, 	"Motorola SVR4",	0, 0 }}
	};
#endif


#ifdef convex
#define gotone
#define USE_UNAME
static MTYPETAB mtypes[] = {
	{0, { LM_CONVEX, 	"convex",		0, 0 }}
	};
#endif

#if defined(PC) && !defined(_ALPHA_) && !defined(_MIPS_)
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_IBMPC, 	"IBM PC",		0, 0 }}
	};
#endif

#ifdef sco
#define gotone
#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
static MTYPETAB mtypes[] = {
	{0, { LM_SCO_UNIX, 	"sco 80x86",		0, 0 }}
	};
#endif

#ifdef cray
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_CRAY, 		"CRAY",			0, 0 }}
	};
#endif

#ifdef OBSD86
#define gotone
static MTYPETAB mtypes[] = {
	{0, { LM_I86_OBSD, 	"OpenBSD x86",		0, 0 }}
	};
#endif

#ifndef gotone
static MTYPETAB mtypes[] = {
	{0, { LM_UNKNOWN, "" }}
	};
#else
#undef gotone
#endif

#ifdef USE_UNAME
#include <limits.h>
#include <sys/utsname.h>
#endif


static HOSTTYPE unknown = { LM_UNKNOWN, "" };
static int l_perf();

HOSTTYPE * API_ENTRY
lc_hosttype(job, run_benchmark)
LM_HANDLE *job;		/* Current license job */
int run_benchmark;
{
/*
 *	Sun - 1st byte of hostid is machine type
 */
  MTYPETAB *m = mtypes;
#ifdef sun
  HOSTID *hid = lc_gethostid(job);
  long id;
#endif
#ifdef USE_UNAME
  struct utsname name;
#endif
  int p = 0;
  HOSTTYPE *t = (HOSTTYPE *) NULL;
  int avail = 0;

	if (run_benchmark) p = l_perf();
#ifdef sinix
#define gotone

	(void) bzero((char *) &name, sizeof(struct utsname));
	uname(&name);
	if (name.machine)
	{
	    while (m->mtype)
	    {
		if (!strncmp(&(name.machine[0]), &(m->data.name[4]), 6))
			break;
		m++;
	    }
	    if (m->mtype == 0) m = mtypes;
	    if (m == mtypes)
	    {
/*
*                      Try again, with a 5-char match
*/
		while (m->mtype)

		{
			if (!strncmp(&(name.machine[0]),
					&(m->data.name[4]), 5))
							break;
			m++;
		}
		if (m->mtype == 0) m = mtypes;
	    }
	}
	t = &m->data;
#endif  /* sinix */


#ifdef DGUX
#define gotone

	(void) bzero((char *) &name, sizeof(struct utsname));
	uname(&name);
	if (name.machine)
	{
	    while (m->mtype) 
	    {
		if (!strncmp(&(name.machine[3]), &(m->data.name[5]), 4))
			break;
		m++;
	    }
	    if (m->mtype == 0) m = mtypes;
	    if (m == mtypes)
	    {
/*
*			Try again, with a 3-char match
*/
		while (m->mtype) 
		{
			if (!strncmp(&(name.machine[3]), 
					&(m->data.name[5]), 3))
							break;
			m++;
		}
		if (m->mtype == 0) m = mtypes;
	    }
	}
	t = &m->data;
#endif	/* DGUX */

#ifdef sco
#define gotone
	{
	  static struct scoutsname scostr;

		t = &m->data;
	#if OLDSCO
		if (__scoinfo(&scostr, sizeof(struct scoutsname)) != -1)
	#else
		if (syscall(12840, &scostr, sizeof(struct scoutsname)) != -1)
	#endif
		{
		  static char newname[30]; 
				/* (machine: 9 char, bustype: 9 chars) */

			(void)strcpy(newname, "sco ");
			strncat(newname, &(scostr.machine[0]), 9);
			strcat(newname, "/");
			strncat(newname, &(scostr.bustype[0]), 9);
			/*(void) printf(stdout,"NumCPU = %d\n",sco->numcpu); */
			t->name = newname;
		}
	}
#endif	/* sco */

#ifdef sun
#define gotone
	id = hid->id.data;
	id &= 0xff000000;
	id >>= 24;
	while (m->mtype) 
	{
		if ((int) id == m->mtype) break;
		m++;
	}
	t = &m->data;
#endif	/* sun */

/*
 *	Catch any platform here, and return the "generic" value.
 */
#ifndef gotone
	t = &m->data;
#ifdef USE_UNAME
	(void) bzero((char *) &name, sizeof(struct utsname));
	uname(&name);

	if (name.machine)
	{
	  static char longname[SYS_NMLN+50];

		(void) strcpy(longname, t->name);
		(void) strcat(longname, " ");
		(void) strcat(longname, name.machine);
		t->name = longname;
	}
#endif	/* USE_UNAME */
#endif	/* gotone */

	t ->flexlm_speed = p;
	return(t);
}

#ifdef NO_PERF
static
l_perf()
{
	return(0);
}
#else

#include <sys/time.h>
#include <sys/resource.h>

static int go;
#define SCALE 20	/* Scaling factor for output of l_perf() */

static long elapsedtime;
static long cputime();

static 
#ifdef VOID_SIGNAL
void
#endif
_timer(sig)
int sig;
{
	elapsedtime = cputime() - elapsedtime;
	go = 0;
#ifndef VOID_SIGNAL
	return(0);
#endif
}


static
l_perf()
{
  register int i = 0;
#ifdef VOID_SIGNAL
  void (*_utimer)() = 0;	/* User's timer handler */
#else
  int (*_utimer)() = 0;		/* User's timer handler */
#endif
  struct itimerval timeout, _utime;

	go = 1;
/*
 *	First, set up the timers
 */
	timeout.it_interval.tv_sec = 0;
	timeout.it_interval.tv_usec = 0;
	timeout.it_value.tv_sec = 0;
	timeout.it_value.tv_usec = 500000;
	(void) setitimer(ITIMER_VIRTUAL, &timeout, &_utime);
	_utimer = signal(SIGVTALRM, _timer);

	elapsedtime = cputime();
	while (go)
		i = i + 1;

	(void) setitimer(ITIMER_VIRTUAL, &_utime, 0);
	(void) signal(SIGVTALRM, _utimer);
	
/*
 *	Normalize the result
 */
	i /= elapsedtime;
	i /= SCALE;

	return(i);
}

/* #define USE_SYSTEM_TIME	*/
static
long            /* milliseconds */
cputime()
{
  struct rusage ru;

	memset(&ru, 0, sizeof(ru));
#ifndef TANDEM
	getrusage(RUSAGE_SELF,&ru);
#endif /* TANDEM */

	return (ru.ru_utime.tv_usec

#ifdef USE_SYSTEM_TIME
				+ru.ru_stime.tv_usec
#endif
							)/1000 +
		(ru.ru_utime.tv_sec
#ifdef USE_SYSTEM_TIME
				+ru.ru_stime.tv_sec
#endif
							)*1000;
}
#endif /* VMS */
