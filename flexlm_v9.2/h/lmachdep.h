 /****************************************************************************

	COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Corporationc and is protected by law.  It may
	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Corporation.

 ***************************************************************************/
/****************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent
 *
 *		to run on an unlicensed platform is a violation of your
 *
 *		license agreement with Macrovision Corporation.
 *
 *
 ***************************************************************************/
/*
 *	Module: $Id: lmachdep.h,v 1.85.2.1 2003/06/23 21:19:22 sluu Exp $
 *
 *	Description: Isolates machine dependencies
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  1/9/99
 *
 */

/*
 *	The general strategy is that compilation will include one of
 *	the machine/OS type keywords, eg: SUNOS4, HP, VAX, etc..., and
 *	based on this keyword, we set the individual options in this
 *	module.  This avoids bundling many (logically) different kinds
 *	of changes in the code under one compilation option, and makes
 *	the resulting code much clearer.
 *
 *	The baseline code is for a Sun3 running SunOS3.x (pre v2.71)
 *	The baseline code is for a Dec Alpha running osf1.x (v2.71 and later)
 *
 *	Option meanings:
 *
 *	ACCEPT_BUG	  - SVR4 accept() bug in socket library exists.
 *	CANT_RE_BIND	  - Once bind fails with address in use, it always fails
 *	DEFAULT_LIC_FILE_SPEC - Allow specifying default license file in daemon
 *	EMBEDDED_FLEXLM	- Compiled for use in embedded systems
 *	FIONBIO_IN_FILIO_H - <sys/filio.h> defines FIONBIO
 *	FIONBIO_IN_SOCKET_H - <sys/socket.h> defines FIONBIO
 *	FNDELAY_IN_FILE_H - <sys/file.h> defines FNDELAY
 *	LOCK_SEMCTL	  - Use semctl(2) calls to lock daemon
 *	LOCK_FLOCK	  - Use flock(2) calls to lock daemon
 *	LOCK_LOCKF	  - Use lockf(2) calls to lock daemon
 *	LOCK_VMS	  - Use VMS locking calls to lock daemon
 *	MULITPLE_VD_SPAWN - Vendor daemons spawn additional copies of themselves
 *	NEWLOCKFILE -- put lockfile in /usr/tmp/.flexlm/.lock<daemon> by default
 *	NO_xxxx		  - "xxxx" call does not exist
 *	NO_DIR_DOT_H      - System has no "sys/dir.h" include (use dirent.h)
 *	NO_GETDOMAINNAME_CALL - The getdomainname() call doesn't exist
 *	NO_INIT_SHARE     - Do not init complex sharable data in client lib
 *	NO_NEED_FDSET     - We don't need the definition of FD_SET
 *	NO_PERF		  - Do not perform the benchmark in lm_hosttype
 *	NO_SERVER	  - no server.
 *	NO_SIGCLD 	  - System has no SIGCLD signal
 *	NO_SSCANF_EMPTY_FIELDS_COUNT - sscanf() doesn't count empty
 *							matched fields.
 *	NO_TIMEVAL	  - No definitions of the timeval structs exist
 *	NO_TIMEZONE	  - TZ is a SYS_V'ism -- This will be defined for BSD
 *	NO_ITIMERVAL	  - No definitions of the itimerval structs exist
 *	NO_TIME_T	  - No struct time_t
 *	NO_UIO_H	  - System doesn't need <uio.h> include (doesn't exist)
 *	PATHTERMINATOR	  - The character that separates the directory name
 *			    from the program name in a fully-qualified pathname
 *	PORT_AT_HOST_SUPPORT - Support for reading license file from port@host
 *	RE_START_TIMER	  - Timers need to be re-set after going off.
 *	READ_ERRNO_0_SIZE_0_OK - read() on pipe returns 0/errno:
 *					0 is OK on this machine
 *	RLIMIT_FOR_GETDTABLESIZE
 *	SSCANF_NFLDS_BROKEN - sscanf() returns bogus nflds (see lm_daemon.c)
 *	SUPPORT_UDP	  - Support UDP protocol
 *	TIMEZONE_NOT_IN_TIME_H - have to define it by hand
 *	UNCONDITIONAL_LMADMIN -  Everyone is a license administrator
 *	UNLIMIT_DESCRIPTORS - vendor daemon should use rlimit() to get more
 *				file descriptors
 *	USE_GETTEXT	  - Use gettext() function to get text from message file
 *	USE_HOSTID	  - Processor/OS uses a 32-bit hostid
 *	USE_LONG_HOSTID	  - Processor/OS uses a 64-bit hostid
 *	USE_ETHERNET_ADDR - "	     "  " ethernet address as ID
 *	ULTRIX_ETHERNET_ADDR - Get ethernet address as on ultrix (vax, mips)
 *	VOID_SIGNAL	  - signal() returns pointer to func returning void
 *	NO_SIGACTION	 - sigaction unavailable
 *	USE_SIGBLOCK	 - use sigblock instead of sigprocmask
 *	TRUE_BSD	 - doesn't have sigaction/strstr/cuserid, etc.
 *	VD_MAKES_TCP_SOCKET - Set if the vendor daemon creates the TCP socket
 *			rather than lmgrd creating it.
 *	LM_LONG_64BIT
 *	BCOPY_MACRO_IN_STRING_H
 *	NO_MKTIME	- mktime() not available -- uses internal version
 *	SELECT_FD_SET	- select requires an (fd_set *) cast to args
 *	DEFAULT_HOSTID_ETHER    - set default hostid type for lmhostid and
 *                        lc_gethostid().
 *	LM_SUPPORT_MT
 *
 */

#ifndef _LM_MACHDEP_H_
#define _LM_MACHDEP_H_


/*
 *	Generic UNIX
 */

#define THREAD_SAFE_TIME
#define SYS_ERRLIST(x) sys_errlist[x]
#define SYS_NERR sys_nerr
#define PATHTERMINATOR '/'		/* Path ends in "/mumble" */
#define PATHSEPARATOR ':'
#define PATHSEPARATORSTR ":"
#define SUPPORT_UDP		/* Support UDP protocol */
#ifdef FIFO
#define SUPPORT_FIFO		/* Support FIFO as an transport protocol */
#endif
#define PORT_AT_HOST_SUPPORT 	/* Allow port@host */
#define MULTIPLE_VD_SPAWN	/* Spawn multiple copies of vendor daemons */
#ifndef UNIX
#define UNIX			/* gets undefined for anything else */
#endif
#define SELECT_FD_SET
#define LM_SET_NET_ERRNO(x)	net_errno = (x)
#define HAVE_GETDOMAINNAME	1
#ifndef NO_MT
#define LM_SUPPORT_MT
#endif /* NO_MT */

/**********************************************************************
 * Modes for VxWorks
 * Added 11/6/02 kmaclean
 * To start with this is only set-up for the simulator on the pc
 *********************************************************************/
#ifdef VXWORKS

#undef UNIX                         /* We're not UNIX */
#undef SUPPORT_UDP                  /* Not using UDP */
#undef FNDELAY_IN_FILE_H

#ifndef CPU
/* needs to be defined in the makefile */
#error CPU is not defined
#endif

#if !defined(GPLATFORM)
#if CPU == SIMNT
#define GPLATFORM "vx_simpc"     /* VxWorks for the simulator */
#else
#error GPLATFORM not defined
#endif
#endif

/* extern char *sys_errlist[]; */
#define USE_SYS_TIMES_H				/* there is no sys/time.h use sys/times.h*/
#define NO_UIO_H					/* No uio.h in VxWorks */
#define NO_DIR_DOT_H  				/* No dir.h in VxWorks */
#define NO_ITIMERVAL				/* we need our own struct itimerval */
#define ITIMER_REAL 1
#define ITIMER_VIRTUAL 2
#define NO_USERNAME_IN_OS           /* there is no user name */
#define SIGVTALRM SIGALRM
#define NO_gethostid				/* There is no gethostid() call in VxWorks */
#define NO_getdtablesize			/* there is no getdtablesize() func in VxWorks */
#define network_control ioctl		/* only FIOBNIO, FIONREAD and SIOCATMARK  are supported*/
#define VOID_STAR_MALLOC			/* Malloc returns a void * */
#define VOID_STAR_CALLOC			/* calloc returns a void * */
#define CONST_STRCPY_PARAM
#define CONST_CHAR_STRNCPY_PARAM
#define CONST_VOID_MEMCPY_PARAM
#define HP_MEMSET_PARAMS

/* HACKS here for things we'll probably remove later because they
 * will not be supported in VXWORKS */
#define LM_BORROW_FILE ""
#define LM_REGISTRY_FILE ""
#undef SYS_ERRLIST
#define SYS_ERRLIST(x)	((char *)"sys_errlist[] not available")
#define getpid()		0
#define _access(x,y) 	-1
/*  Need to write our own gethostbyaddr that used hostGetByAddr()
 * For now just return failure */
#define gethostbyaddr(a,b,c)  (-1)
#define getservbyname(a,b)	((struct servent *) NULL)
#define gethostbyname(a)	(NULL)
#define shutdown(socket,how)	close(socket);
#define NOFILE 32	/* this is used in ul_getdtablesize(). really need to get it from VxWorks  */
#define NO_setitimer

#undef SYS_NERR
#define SYS_NERR 10000 /* big number */

/* still need to determine if we should define EMBEDDED_FLEXLM.
 * Don't think we want to. It removes too much stuff */

#include "vxworks.h"

#endif /* vxworks */


#ifdef EMBEDDED_FLEXLM
#define	L_SET	0	/* set pointer to 'offset' bytes in lseek function */
#endif	/* EMBEDDED_FLEXLM */

#ifdef LYNX
#undef         HAVE_GETDOMAINNAME     /* No Getdomainname call */
#endif        /* LYNX */

#ifdef LYNX_PPC
#undef         HAVE_GETDOMAINNAME     /* No Getdomainname call */
#endif        /* LYNX_PPC */

#if defined( _MSC_VER) && !defined(PC)
#define PC
#elif !defined(FLEXLM_ULTRALITE)
#include <signal.h>

#endif

#if defined( _WIN32) && !defined(WINNT)
#define WINNT
#endif
#ifndef FLEXLM_ULTRALITE
#include <sys/types.h>
#endif

#ifdef WINNT
#include "MvsnLicenseServerMsgs.h"
#include "VendorLicenseServerMsgs.h"
#else /* !WINNT */
/*
 *	Just #define everything out
 */
#define l_flexEventLogIsEnabled()	0
#define l_flexEventLogInit(y,z)
#define	l_flexEventLogCleanup()
#define l_flexEventLogAddString(y,z)
#define	l_flexEventLogFlushStrings(z)
#define	l_flexEventLogWrite(s,t,u,v,w,x,y,z)
#endif

/*
 *	DEC alpha mods
 */
#if defined(__alpha) && !defined(WINNT) && !defined(LINUX)
#define ALPHA

#ifndef VMS
#define OSF
#undef LM_SUPPORT_MT

#define DEFAULT_HOSTID_ETHER
#define LM_LONG_64BIT
#define LONG_POINTER
#define USE_ETHERNET_ADDR	/* Alpha processors use ethernet address */
#define ULTRIX_ETHERNET_ADDR	/* Alpha processors use ULTRIX */
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */
#define NO_DIR_DOT_H
#define WAIT_STATUS_INT
#define NEWLOCKFILE

#define VOID_STAR_CALLOC
#define VOID_EXIT
#define VOID_FREE
#define INT_GETHOSTID
#define VOID_STAR_MALLOC
#define VOID_PERROR
#define VOID_SIGNAL
#define UNSIGNED_SLEEP
#define INT_SPRINTF
#define INT_STRLEN
#define ANSI
#define BCOPY_MACRO_IN_STRING_H
#ifndef RELEASE_VERSION
/*- #define SUPPORT_METER_BORROW*/
#endif /* RELEASE_VERSION */

#endif		/* !VMS */
#endif		/* alpha && !WINNT */


/*
 *	Apollo mods
 */

#ifdef apollo

#define USE_HOSTID		/* Apollo IDs are just like hostids */
#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define NO_GETDOMAINNAME_CALL	/* No getdomainname() call */
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */
#define READ_ERRNO_0_SIZE_0_OK	/* read() return of errno 0, size 0 is OK */

#define CANT_RE_BIND		/* bind can't recover from address in use */

#define _CLASSIC_CTYPE_MACROS	/* Used for SR10.1 compatibility */
#define _CLASSIC_STAT		/* Used for SR10.1 compatibility */

#endif /* apollo */

#ifdef OPENBSD
#define FREEBSD
#endif /* OPENBSD */

/*
 *	BSDI
 */
#if defined (BSDI) || defined(FREEBSD)
#ifdef LM_FLEXLM_DIR
#undef LM_FLEXLM_DIR
#endif /* LM_FLEXLM_DIR */
#define LM_FLEXLM_DIR "/tmp/.flexlm"
#define ANSI
#define VOID_SIGNAL             /* signal() returns void (*f)() on sunOS5 */
#define NO_regular_expressions  /* lmstat uses regular expressions */
#define WAIT_STATUS_INT
#define RLIMIT_FOR_GETDTABLESIZE
#define NO_DIR_DOT_H
#define SIGCLD SIGCHLD
#define DEFAULT_HOSTID_ETHER
#define LOCK_FLOCK              /* Use lockf(2) calls to lock daemon */
#define NO_BUFFER_SIZE
#ifndef FREEBSD
#undef LM_SUPPORT_MT
#endif /* !FREEBSD */
#ifndef OPENBSD
#define NO_GETDOMAINNAME_CALL   /* No getdomainname() call */
#undef HAVE_GETDOMAINNAME
#define NO_setlinebuf
#define NO_sigsetmask
#define NO_sigvec
#define NO_getdtablesize
#define NO_wait3
#endif /* ! OPENBSD */

#endif /* BSDI & FREEBSD */

/*
 *	CONVEX mods
 */

#ifdef convex

#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define NO_DIR_DOT_H
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */
#define NO_SIGCLD		/* convex has no SIGCLD signal */

#endif /* convex */



/*
 *	CRAY mods
 */
#ifdef cray

#ifdef CRAY_NV1
#define NO_BUFFER_SIZE
#else /* CRAY_NV1 */
#define NO_wait3
#define SIGVTALRM SIGALRM
#endif /* CRAY_NV1 */
#define WAIT_STATUS_INT
#define ANSI
#define LM_LONG_64BIT
#define NO_PERF			/* No performance function in lm_hosttype() */
#define LOCK_FLOCK
#define NO_setitimer
#define RE_START_TIMER
#define NO_NEED_FDSET
#define VOID_SIGNAL
#define INT_SPRINTF
#define L_SET 0
#if 0
#define NO_ITIMERVAL
#define NO_TIMEVAL
#define signal bsdsignal
#endif
#define NO_UIO_H
#define  FNDELAY_IN_FILE_H
#define NO_DIR_DOT_H

#endif /* cray */

/*
 *	Dec
 */
#ifdef DEC_UNIX_3
#ifdef THREAD_SAFE_TIME
#undef THREAD_SAFE_TIME
#endif
#endif

/*
 *	DG/UX (mc-88k) mods
 */

#ifdef DGUX

#define UNLIMIT_DESCRIPTORS	/* avail on all versions of DGUX */
#define INT_SPRINTF
#define UNSIGNED_STRLEN
#define VOID_EXIT
#define NO_NEED_FDSET
#define NO_PERF			/* No performance function in lm_hosttype() */
#define LOCK_LOCKF		/* Use lockf(2) calls to lock daemon */
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */

#ifdef SVR4
#define ANSI
#define NO_DIR_DOT_H
#define VOID_SIGNAL		/* signal() returns void (*f)() on sunOS5 */
#define NO_gethostid
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define NO_setlinebuf
#define NO_sigsetmask
extern char *sys_errlist[];
#define NO_wait3
#define NO_sigvec
#define WAIT_STATUS_INT
#define NO_getdtablesize
#define RLIMIT_FOR_GETDTABLESIZE
#define FNDELAY_IN_FILE_H
#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define SSCANF_NFLDS_BROKEN

#else	/* SVR4 */

#define _BSD_SIGNAL_FLAVOR

#define USE_HOSTID
#define READ_ERRNO_0_SIZE_0_OK	/* read() return of errno 0, size 0 is OK */

#endif /* SVR4 */

#endif /* DGUX */


/*
 *	HP-9000 (model 300/400/700/800) modifications
 */

#ifdef HP
#include <sys/types.h>
#include <netinet/in.h>
#define NO_BUFFER_SIZE

/*
 *	First, define the type of hp we are on
 */

#if 0 /* not needed with gplatargs */
#ifdef hp9000s800
#define HP700
#endif
#ifdef __hp9000s800
#define HP700
#endif
#ifdef __hp9000s700
#define HP700
#endif

#ifdef __hp9000s300
#define HP300
#endif

#if !defined (HP700) && !defined(HP300)
#define HP300
#endif
#endif /* 0 */

#define VOID_SIGNAL		/* signal() returns void (*f)() on HP v6.5 */
#define NO_PERF			/* No performance function in lm_hosttype() */
#define LOCK_LOCKF		/* Use lockf(2) calls to lock daemon */
#define USE_LONG_HOSTID		/* HP id modules are longer than 32 bits */
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */
#define READ_ERRNO_0_SIZE_0_OK	/* read() return of errno 0, size 0 is OK */
#define UNLIMIT_DESCRIPTORS
#define VOID_STAR_MEMCPY
#define VOID_STAR_MEMSET
#define VOID_STAR_CALLOC
#define CLOCK_T_CLOCK
#define SIZE_T_FREAD
#define VOID_FREE
#define SIZE_T_FWRITE
#define PID_T_GETPID
#define VOID_STAR_MALLOC
#define SIZE_T_STRLEN
#define NO_TIMEZONE
#define SYS_ERRLIST_NOT_IN_ERRNO_H
#define RLIMIT_FOR_GETDTABLESIZE
#undef SELECT_FD_SET
#define NO_DIR_DOT_H

/*
 *	Stuff that isn't defined
 */
#ifdef HP_V6	/* This stuff was undefined on v6, is defined now (v7) */
#ifndef SIGCHLD
#define SIGCHLD SIGCLD		/* HP doesn't have SIGCHLD !!! */
#endif
#ifndef SIGTSTP
#define SIGTSTP SIGKILL		/* ls_m_init() uses SIGTSTP - SIGKILL is OK */
#endif
#endif

#define NO_flock
#define NO_getdtablesize
#define NO_setlinebuf
#define NO_sigvec		/* No struct sigvec */
#define NO_regular_expressions	/* lmstat uses regular expressions */

#define INT_SPRINTF

#ifdef HP64
#define LM_LONG_64BIT
#include <stdlib.h>
#include <string.h>
#ifdef HPINTEL_64 /* doesn't work yet */
#undef LM_SUPPORT_MT
#define DEFAULT_HOSTID_STRING
#define HP_LONG_HOSTID_STRING
#endif /* HPINTEL_64 */
#else
#ifndef HPUX11
#undef LM_SUPPORT_MT
#endif /* HPUX11 */
#endif /* HP64 */

#ifdef HPINTEL_32
#include <stdlib.h>
#include <string.h>
#undef LM_SUPPORT_MT
#define DEFAULT_HOSTID_STRING
#define HP_LONG_HOSTID_STRING
#endif /* HPINTEL_32 */


#endif /* HP */

#ifdef TANDEM
#define NO_PERF
#define VOID_SIGNAL             /* signal() returns void (*f)() */
#define LOCK_LOCKF              /* Use lockf(2) calls to lock daemon */
#undef LOCK_FLOCK
#define NO_flock
#define NO_setlinebuf
#define NO_sigsetmask
#define NO_DIR_DOT_H
#define ANSI
/*#define UNLIMIT_DESCRIPTORS*/ /* Can't set this for TANDEM -- breaks select */
#define FNDELAY_IN_FILE_H
extern char *sys_errlist[];
#define FIONBIO_IN_FILIO_H
#define VOID_SIGNAL
#define WAIT_STATUS_INT
#define NO_getdtablesize
#define NO_wait3
#define NO_regular_expressions  /* lmstat uses regular expressions */
#define SELECT_FD_SET
#define HOSTID_SYSINFO_STRING
#define DEFAULT_HOSTID_STRING
#endif /* TANDEM */




/*
 *	MIPS mods
 */
#ifdef NEC /* NEC EWS */
#define NO_setlinebuf
#define NO_PERF
#define NO_regular_expressions
#define FIONBIO_IN_FILIO_H
#define FNDELAY_IN_FILE_H
#define WAIT_STATUS_INT
#define INT_SPRINTF
#define SVR4
#define UNLIMIT_DESCRIPTORS
#define VOID_SIGNAL
#define NO_wait3
#define NO_sigvec
#define NO_getdtablesize
#define RLIMIT_FOR_GETDTABLESIZE
#undef LOCK_FLOCK
#define LOCK_LOCKF
#define NO_DIR_DOT_H
#define READ_ERRNO_0_SIZE_0_OK	/* read() return of errno 0, size 0 is OK */
#undef SELECT_FD_SET

#ifdef NECSX4
#include <stdlib.h>
#include <string.h>
#undef FIONBIO_IN_FILIO_H
#define SIGVTALRM SIGALRM
#define SYS_ERRLIST_NOT_IN_ERRNO_H
#endif /* NECSX4 */

#endif /* NEC EWS */

#ifdef SINIX_MIPS
#ifdef THREAD_SAFE_TIME
#undef THREAD_SAFE_TIME
#endif
#undef LM_SUPPORT_MT
#define VOID_SIGNAL
#define LOCK_LOCKF
#define NO_flock
#define NO_setlinebuf
#define NO_sigsetmask
#define ANSI
#define VOID_FREE
#endif

#if (defined (mips) || defined (MIPS)) && !defined(VXWORKS) && !defined(PMAX) && !defined(sgi) && !defined(sony_news) && !defined(NEC) && !defined(WINNT) && !defined(SINIX) && !defined(TANDEM)

#ifndef MIPS
#define MIPS	/* Make sure MIPS is defined */
#endif
#if !defined(sinix)
#define GENUINE_MIPS_MACHINE	/* For inside the code */
#define USE_SIGBLOCK
#define NO_SIGACTION
#define NO_TIMEZONE
#define NO_PERF
#define TRUE_BSD
#define FIONBIO_IN_SOCKET_H
#define NO_MKTIME
#define HOSTID_SYSINFO_STRING
#define DEFAULT_HOSTID_STRING
#endif

extern int errno;		/* Not declared in errno.h! */
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */
#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */
#define VOID_EXIT
#define VOID__EXIT
#define VOID_PERROR
#define INT_SPRINTF
#define VOID_FREE

#ifndef sinix
#define NO_GETCWD
#endif

#endif /* MIPS */

/*SIEMENS*/
#ifdef sinix
#undef MIPS
#ifndef SVR4
#define SVR4
#endif
#define HOSTID_SYSINFO_STRING
#define TIMEZONE_NOT_IN_TIME_H
#define READ_ERRNO_0_SIZE_0_OK
#define LOCK_LOCKF              /* Use lockf(2) calls to lock daemon */
#define NO_NEED_FDSET
#define UNLIMIT_DESCRIPTORS     /* vendor daemon: use rlimit() for more desc. */
#define NO_DIR_DOT_H
#define VOID_SIGNAL             /* signal() returns void (*f)() on sunOS5 */
#define NO_regular_expressions  /* lmstat uses regular expressions */
#define NO_setlinebuf
#define NO_sigsetmask
#define NO_PERF                 /* No performance function in lm_hosttype() */
#define FNDELAY_IN_FILE_H       /* file.h defines FNDELAY */
#define FIONBIO_IN_FILIO_H      /* <sys/filio.h> defines FIONBIO */
extern char *sys_errlist[];
#define NO_wait3
#define NO_sigvec
#define WAIT_STATUS_INT
#define NO_getdtablesize
#define RLIMIT_FOR_GETDTABLESIZE
#define NO_SSCANF_EMPTY_FIELDS_COUNT    /* sscanf() doesn't count empty */
                                        /*   matched fields */
#define SSCANF_NFLDS_BROKEN

#define NO_GETDOMAINNAME_CALL   /* No getdomainname() call */
#define USE_HOSTID
#define INT_SPRINTF
#define DEFAULT_HOSTID_STRING

#endif	/* MIPS */




/*
 *	Motorola 88k mods
 */
#ifdef MOTO_88K

#include <sys/types.h>
#define INT_SPRINTF
#define VOID_SIGNAL
#define NO_PERF			/* No performance function in lm_hosttype() */
#define LOCK_LOCKF		/* Use lockf(2) calls to lock daemon */
#define NO_DIR_DOT_H		/* Motorola doesn't have dir.h */
#define	FNDELAY_IN_FILE_H
#define CANT_RE_BIND		/* bind can't recover from address in use */
#define	L_SET	0	/* set pointer to 'offset' bytes in lseek function */

#define NO_getdtablesize
#define SYSCONF_FOR_GETDTABLESIZE
#define NO_setlinebuf
#define NO_sigvec		/* No struct sigvec */
#define NO_wait3

#define NO_sigsetmask
#define NO_sigmask

#define NO_regular_expressions	/* lmstat uses regular expressions */
#define WAIT_STATUS_INT
#define NO_GETDOMAINNAME_CALL	/* No getdomainname() in 88k BCS */

#define NO_TIME_T

#endif /* MOTO_88K */

#ifdef UNIXWARE
#define DEFAULT_HOSTID_ETHER
#define RLIMIT_FOR_GETDTABLESIZE
#define NO_setlinebuf
#define NO_sigvec		/* No struct sigvec */
#define NO_wait3
#define NO_sigsetmask
#define NO_sigmask
#define NO_DIR_DOT_H		/* Motorola doesn't have dir.h */
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define WAIT_STATUS_INT
#define FIONBIO_IN_FILIO_H
#define NO_PERF			/* No performance function in lm_hosttype() */
#define VOID_SIGNAL
extern char *sys_errlist[];
#define SVR4
#define LOCK_LOCKF
#define NO_getdtablesize
#define FNDELAY_IN_FILE_H
#endif /* UNIXWARE */

#if defined (MOTOSVR4) || defined(ENCORE)

#include <sys/types.h>
#define NO_SPRINTF
#define VOID_SIGNAL
#define NO_PERF			/* No performance function in lm_hosttype() */
#define NO_DIR_DOT_H		/* Motorola doesn't have dir.h */
#define	FNDELAY_IN_FILE_H
#define CANT_RE_BIND		/* bind can't recover from address in use */
#define	L_SET	0	/* set pointer to 'offset' bytes in lseek function */
#define LOCK_LOCKF
#define NO_getdtablesize
#ifdef ENCORE
#define RLIMIT_FOR_GETDTABLESIZE
#else
#define SYSCONF_FOR_GETDTABLESIZE
#endif /* ENCORE */
#define NO_setlinebuf
#define NO_sigvec		/* No struct sigvec */
#define NO_wait3
#define NO_sigsetmask
#define NO_sigmask
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define WAIT_STATUS_INT
#define FIONBIO_IN_FILIO_H
extern char *sys_errlist[];
#define SVR4
#endif

/*
 *      NCR (SVR4) modifications
 */
#ifdef NCR

#define SVR4
#define UNLIMIT_DESCRIPTORS	/* vendor daemon: use rlimit() for more desc. */

#include <sys/types.h>

/*#define BSD_INCLUDES*/
#define NO_SSCANF_EMPTY_FIELDS_COUNT  /* sscanf() doesn't count empty */
					/*   matched fields */
#define NO_PERF			/* No performance function in lm_hosttype() */
#define NO_GETDOMAINNAME_CALL	/* No getdomainname() on this system */

#define LOCK_LOCKF
#define NO_NEED_FDSET
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */
#define NO_setitimer
#define RE_START_TIMER
#define NO_DIR_DOT_H
#define NO_UIO_H
#define	L_SET	0	/* set pointer to 'offset' bytes in lseek function */
#define FIONBIO_IN_FILIO_H
#define NO_setlinebuf
#define NO_sigsetmask
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define NO_getdtablesize
#define NO_wait3

#define INT_SPRINTF
#define VOID_SIGNAL
#define WAIT_STATUS_INT

#endif	/* NCR */

#ifdef LYNX
#define DEFAULT_HOSTID_ETHER
#define _KERNEL
#include <errno.h>
#define SVR4

#include <sys/types.h>

#define NO_PERF                       /* No performance function in lm_hosttype() */
#define NO_GETDOMAINNAME_CALL /* No getdomainname() on this system */

#define LOCK_LOCKF
#define NO_DIR_DOT_H
#define NO_UIO_H
#define L_SET   0       /* set pointer to 'offset' bytes in lseek function */
#define NO_regular_expressions        /* lmstat uses regular expressions */
#define NO_getdtablesize

#define VOID_SIGNAL
#define WAIT_STATUS_INT
#endif /* LYNX */


#if defined( LINUX) || defined(MAC10)
#if defined(ALPHA_LINUX) || defined(VMLINUX64) || defined(RHLINUX64)
#define GLIBC
#ifndef FNDELAY
#define FNDELAY O_NDELAY
#undef SYS_ERRLIST
#define SYS_ERRLIST(x) strerror(x)
#endif
#define LM_LONG_64BIT
#endif
#if !defined( RELEASE_VERSION) && !defined(GLIBC)
/*- #define SUPPORT_METER_BORROW*/
#endif /* RELEASE_VERSION */
#define DEFAULT_HOSTID_ETHER
#if !defined(ALPHA_LINUX) || !defined(RHLINUX64)
/*#define SIGSYS SIGSEGV*/
#endif /* SIGSYS */
#ifndef SIGEMT
#define SIGEMT SIGSEGV
#endif /* !ALPHA_LINUX */
#define SVR4
#define UNLIMIT_DESCRIPTORS   /* vendor daemon: use rlimit() for more desc. */

#include <sys/types.h>

#define NO_PERF                       /* No performance function in lm_hosttype() */
#define NO_GETDOMAINNAME_CALL /* No getdomainname() on this system */

#define LOCK_LOCKF
#define NO_DIR_DOT_H
#define NO_UIO_H
#if defined(LINUX) && defined(REDHAT) && defined(GLIBC)
#include <errno.h>
#undef SYS_ERRLIST
#define SYS_ERRLIST(x)	strerror(x)
#undef SYS_NERR
#define	SYS_NERR	125	/* 1 greater that highest value in errno.h */
#endif
#define NO_regular_expressions        /* lmstat uses regular expressions */
#define RLIMIT_FOR_GETDTABLESIZE

#define VOID_SIGNAL
#define WAIT_STATUS_INT
#ifdef ALPHA_LINUX
#undef LM_SUPPORT_MT
#define LM_LONG_64BIT
#define GLIBC
#endif /* ALPHA_LINUX */

#ifdef MONTAVISTA
#define VOID_SIGNAL             /* signal() returns void (*f)() on sunOS5 */
#ifdef LM_FLEXLM_DIR
#undef LM_FLEXLM_DIR
#endif /* LM_FLEXLM_DIR */
#define LM_FLEXLM_DIR "/tmp/.flexlm"
#endif

#if !defined(REDHAT) || defined(REDHAT5)
#undef LM_SUPPORT_MT
#endif /* REDHAT5 */
#define NO_BUFFER_SIZE
#endif /* LINUX */


/*
 *	NeXT mods
 */
#ifdef NeXT

#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define USE_HOSTID
#define LOCK_FLOCK
#define VOID_STAR_MEMCPY
#define NO_GETCWD
#define USE_SIGBLOCK
#define NO_SIGACTION
#define NO_TIMEZONE
#define TRUE_BSD
#define NO_SPRINTF
#undef __STDC__
#define SYS_ERRLIST_NOT_IN_ERRNO_H
#endif /* NeXT */


/*
 *	PC-client mods
 */

#ifdef PC

#if !defined(FLEXLM_ULTRALITE) && (!defined ( OS2) || ( defined (OS2) && defined (_LMGR_WINDLL) ))
#include <windows.h>
#endif

#ifdef THREAD_SAFE_TIME
#undef THREAD_SAFE_TIME
#endif

#define HAS_STRICMP		/* Needed to L_STREQ_I, which does a case in-sensitive comparison */
#undef PATHSEPARATOR
#undef PATHTERMINATOR
#define PATHSEPARATOR ';'
#undef PATHSEPARATORSTR
#define PATHSEPARATORSTR ";"
#define PATHTERMINATOR '\\'		/* Path ends in "/mumble" */
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define NO_getdtablesize
#define NO_NEED_FDSET
#define NO_ITIMERVAL
#define USE_WINSOCK		/* Use winsock.h rather than socket.h */
#undef LM_SET_NET_ERRNO
#define LM_SET_NET_ERRNO(x)	l__WSASetLastError(x)
#ifdef OS2
#define INCL_DOSPROCESS
#include <os2.h>
#define Sleep(x) DosSleep(x)
#define Yield() DosSleep(0)
#define getenv l_getenv_os2
#else
#define SUPPORT_IPX
#endif /* OS2 */
#define WINSOCK_VERSION	0x0101	/* Winsock version that we are compatible with */
#define LOCK_WINDOWS		/* Not used here, but used in NT */
#undef  MULTIPLE_VD_SPAWN	/* Don't spawn multiple copies of v.d. */
#ifdef FIFO
#undef	SUPPORT_FIFO		/* Don't support FIFO as transport protocol */
#endif
#define VD_MAKES_TCP_SOCKET	/* Vendor creates the TCP socket */
#define SIGNAL_NOT_AVAILABLE	/* Unix signal/kill mechanism not available */
#define	LOCK_LOCKING		/* Use NT _locking() call to lock file */
#define WINNT_TODO		/* Marker of tasks to be done on NT */

#define crypt flm_vms_crypt
#define DEFAULT_HOSTID_ETHER

#define NO_TIMEZONE

#if defined(WINNT) || defined(NLM) || defined(OS2)

#define PC32
#define CPU_USAGE
#ifdef WINNT
/*- #define SUPPORT_METER_BORROW */
#define HAVE_CPUID         /* supports Intel CPU id */
#ifdef _PPC_
#define GPLATFORM               "ppc_n3"
#else
#ifdef _MIPS_
#define GPLATFORM               "mips_n3"
#else
#ifdef _ALPHA_
#define GPLATFORM               "alpha_n3"
#undef HAVE_CPUID         /*  Does not support Intel CPU id */
#else
#ifdef _ia64_
#define GPLATFORM               "ia64_n5"
#undef HAVE_CPUID         /*  Does not support Intel CPU id */
#else /* only thing left should be intel-32-bit */
#define GPLATFORM               "i86_n3"
#endif /* _ALPHA_ */
#endif /* _IA64_ */
#endif /* _MIPS_ */
#endif /* _PPC_ */
#endif /* WINNT */

#ifdef NLM
#define GPLATFORM               "i86_z3"
#else
#ifdef OS2
#define GPLATFORM               "i86_o3"
#endif  /* OS2 */
#endif /* NLM */
#define ANSI


#else /* -->now the 16-bit */

#define PC16
#define GPLATFORM               "i86_w3"
#define NO_TIMERS               /* PC has no setitimer/alarm stuff */
#endif /* WINNT */

#define NO_USERNAME_IN_OS	/* OS doesn't support the idea of username */
#define VOID_SIGNAL		/* signal() returns void (*f)() */
#define NO_PERF			/* No performance function in lm_hosttype() */
#undef UNIX
#undef SYS_ERRLIST
#ifdef NLM
#undef SUPPORT_UDP
#define NO_LMGRD
#define NO_REDUNDANT_SERVER
#define NO_FLEXLM_CLIENT_API
#define Yield() ThreadSwitch()
#define getpid() GetNLMID()
#define lc_sleep(job, secs) delay(secs*1000)
#define GetCurrentTime() (clock()*10)
#define SetTimer(a, b, c, d) 0
#define KillTimer(a, b)
#define SYS_ERRLIST(x) ((char *)"sys_errlist[] not available")
#endif /* NLM */
#if defined(OS2)
#define SYS_ERRLIST(x) ((x>= sys_nerr)? "WinSock error code": sys_errlist[x])
#endif /* OS2 */
#if !defined(OS2) && !defined(NLM)
#define SYS_ERRLIST(x) ((x>=_sys_nerr)? "WinSock error code":_sys_errlist[x])
#endif

#ifndef FLEXLM_ULTRALITE
#define L_SET SEEK_SET		/* Turbo-C's way of doing lseek() */
#endif

/*
 *	SIGNAL timer stuff doesn't exist on PC
 */

#define SIGALRM 0
#define SIGVTALRM 0
#define SIGPIPE 0

#define sleep flm_sleep
/* #define NO_sleep    */

#define NOFILE 32

/*
 *	Stuff that we don't need on the PC
 */
#if !defined(WINNT) && !defined(NLM)
#define setitimer(a, b, c) 0
#endif /* WINNT */
#define sigmask(x) 0
#define sigsetmask(x) 0

#define VOID_STAR_CALLOC
#define VOID_STAR_MALLOC
#define VOID_STAR_REALLOC
#define VOID_STAR_MEMCPY
#define VOID_STAR_MEMSET
/* #define INT_SPRINTF      */
#define NO_SPRINTF
#define NO_UIO_H
#define UNCONDITIONAL_LMADMIN	/* Everyone is an administrator */

#ifdef _LMGR_WINDLL
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 *	Windows/NT (Included inside PC definitions)
 */
#if !defined(WINNT) && !defined(OS2)
#define sscanf wsscanf
int wsscanf( char *buf, char *fmt, ... );
#endif /* WINNT */



#if !defined( FLEX_STATIC_MD) && !defined(FLEX_STATIC_MT)
#define FLEX_STATIC_MT
#endif

#undef printf
#define printf lm_debug_printf
lm_debug_printf( const char *fmt, ... );

#undef puts
#define puts lm_debug_puts
lm_debug_puts( const char *s );

#endif	/* _LMGR_WINDLL */

#endif /* PC */


/*
 *	PC/RT mods
 */
#ifdef PCRT

#define BSD_INCLUDES
#define USE_HOSTID		/* PC/RT systems all use gethostid() */
#define NO_SSCANF_EMPTY_FIELDS_COUNT  /* sscanf() doesn't count empty */
					/*   matched fields */

#if 0
#define SIGVTALRM SIGALRM
#define SIGCHLD SIGCLD
#ifndef SIGTSTP
#define SIGTSTP SIGKILL
#endif /* SIGTSTP */
#define L_SET 0			/* Not in file.h ? */
#endif

#define LOCK_LOCKF
#define NO_NEED_FDSET
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */



#define INT_SPRINTF
#define VOID_SIGNAL
#define NO_DIR_DOT_H

#endif /* PCRT */

#ifdef RS6000
#ifndef RS64
#ifdef THREAD_SAFE_TIME
#undef THREAD_SAFE_TIME
#endif
#endif
#include <net/nh.h>
#define USE_HOSTID		/* PC/RT systems all use gethostid() */
#define NO_SSCANF_EMPTY_FIELDS_COUNT  /* sscanf() doesn't count empty */
					/*   matched fields */
#define NO_SPRINTF

#define LOCK_LOCKF
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */
#define VOID_SIGNAL
#define TIMEZONE_NOT_IN_TIME_H
#define SYS_ERRLIST_NOT_IN_ERRNO_H
#define ANSI
#define WAIT_STATUS_INT
#undef NO_NEED_FDSET
#define USE_FD_SET_FOR_SELECT
#ifdef RS64
#define LM_LONG_64BIT
#define NO_BUFFER_SIZE
#endif /* RS64 */

#undef LM_SUPPORT_MT /* turn off MT for AIX because of bug P6236 */
#endif /* RS6000 */



/*
 *	SCO unix mods
 */
#ifdef sco
#include <sys/types.h>
#define NO_BUFFER_SIZE

#undef LM_SUPPORT_MT
#define ANSI
#define BSD_INCLUDES
#define NO_SSCANF_EMPTY_FIELDS_COUNT  /* sscanf() doesn't count empty */
					/*   matched fields */
#define NO_PERF			/* No performance function in lm_hosttype() */
#define NO_GETDOMAINNAME_CALL	/* No getdomainname() on this system */

#define LOCK_LOCKF
#define NO_NEED_FDSET
#if old_sco
#define NO_TIMEZONE
#define NO_setitimer
#define ITIMER_REAL 1
#define ITIMER_VIRTUAL 2
#define SIGVTALRM SIGPWR	/* No virtual timer on sco */
#define NO_ITIMERVAL
#endif

#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */
#define RE_START_TIMER
#define NO_DIR_DOT_H
#define NO_UIO_H
#define	L_SET	0	/* set pointer to 'offset' bytes in lseek function */
#ifdef SCO5
#define FIONBIO_IN_FILIO_H	/* <sys/filio.h> defines FIONBIO */
#else
#define FIONBIO_IN_SOCKET_H
#endif
#define NO_setlinebuf
#define NO_sigsetmask
#define NO_regular_expressions	/* lmstat uses regular expressions */

#define INT_SPRINTF
#define VOID_SIGNAL
#define WAIT_STATUS_INT
#define DEFAULT_HOSTID_STRING
#ifdef SCO5
#undef SYS_ERRLIST
#define SYS_ERRLIST(x) strerror(x)
#undef LM_SUPPORT_MT
#undef SYS_NERR
#define SYS_NERR 10000 /* big number */
#endif

#endif /* sco */

#ifdef MAC10
#define UNLIMIT_DESCRIPTORS
#define VOID_SIGNAL
#undef SYS_ERRLIST
#define SYS_ERRLIST(x) strerror(x)
#define LOCK_FLOCK
#ifdef LOCK_LOCKF
#undef LOCK_LOCKF
#endif
#ifdef THREAD_SAFE_TIME
#undef THREAD_SAFE_TIME
#endif
#endif


/*
 *	SGI (personal IRIS) modifications
 */

#if defined(SGI) || defined(sgi) || defined(__sgi)
#define NO_BUFFER_SIZE
#include <sys/types.h>

#define VOID_SIGNAL		/* signal() returns void (*f)() */
#define LOCK_LOCKF		/* Use lockf(2) calls to lock daemon */
#define NO_flock
#define NO_setlinebuf
#define NO_sigsetmask
#ifndef SGI4
#define ANSI
#define UNLIMIT_DESCRIPTORS
#endif /* !SGI4 */
#ifdef SGI4
#define UNSIGNED_SLEEP

#define SIZE_T_STRLEN
#define PID_T_GETPID
#define VOID_STAR_CALLOC
#define VOID_STAR_MALLOC
#define SYS_ERRLIST_NOT_IN_ERRNO_H
/* #define CLOCK_T_CLOCK */
#define VOID_FREE

/*
 *	Stuff that is missing on SGI
 */
				/* Missing library functions/system calls */
#define VOID_STAR_MEMCPY
#define VOID_EXIT
#define VOID_PERROR
#define INT_SPRINTF
#define VOID__EXIT
#define VOID_FREE
#endif /* SGI4 */
#endif /* SGI */
#if defined(SGI64 ) || defined (NECSX4)
#define LM_LONG_64BIT
#endif



/*
 *	Sony news modifications
 */

#if defined(sony_news)

#define TRUE_BSD
#define NO_SIGACTION
#define USE_SIGBLOCK
#define USE_HOSTID
#define LOCK_FLOCK
#define NO_NEED_FDSET
#define NO_MKTIME
#undef SELECT_FD_SET

#if defined(mips)
#define INT_SPRINTF
#endif

#endif /* sony_news */


/*
 *	Sun modifications
 */

#ifdef sun

#define USE_HOSTID		/* Sun systems all use gethostid() */
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */
#define KERN_LOOKUP
#define SUN_EXTENDED_HOSTID
#define NO_TIMEZONE

#endif /* sun */


/*
 *	Sun Release 4 modifications
 */
#ifdef SUNOS4
#include <sys/types.h>
#include <sys/time.h>
time_t time();

#define VOID_SIGNAL		/* signal() returns void (*f)() on sunOS4 */
#define VOID_EXIT
#define NO_NEED_FDSET
#define WAIT_STATUS_INT
#define UNLIMIT_DESCRIPTORS	/* vendor daemon: use rlimit() for more desc. */

#endif /* SUNOS4 */



/*
 *      Sun Release 5 (SVR4) modifications
 */
#if defined( SUNOS5) || defined(HAL)

#define SVR4
#define ANSI
#if defined(SUNOS5)
#define FD_SET_LIMITED
#endif

#if 0
#define USE_TLI 		/* Define to use TLI in the daemons */
#endif

#define UNLIMIT_DESCRIPTORS	/* vendor daemon: use rlimit() for more desc. */
#undef SUN_EXTENDED_HOSTID	/* Not on SVR4 */
#define NO_DIR_DOT_H
#undef KERN_LOOKUP		/* No l_kern_peek() function support in SVR4 */
#define VOID_SIGNAL		/* signal() returns void (*f)() on sunOS5 */
#define USE_GETTEXT		/* Use gettext() message lookup code */
#define NO_gethostid
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define NO_setlinebuf
#define NO_sigsetmask
#define TIMEZONE_NOT_IN_TIME_H
#undef NO_TIMEZONE
#define NO_PERF
#define FNDELAY_IN_FILE_H	/* file.h defines FNDELAY */
/*#define FNDELAY (O_NDELAY | O_NONBLOCK)*/
#define FIONBIO_IN_FILIO_H	/* <sys/filio.h> defines FIONBIO */
extern char *sys_errlist[];
#undef LOCK_FLOCK
#define LOCK_LOCKF		/* Use lockf(2) calls to lock daemon */
#define NO_wait3
#define NO_sigvec
#define WAIT_STATUS_INT
#define NO_getdtablesize
#define RLIMIT_FOR_GETDTABLESIZE
#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define SSCANF_NFLDS_BROKEN
#define SYS_ERRLIST_NOT_IN_ERRNO_H
#define READ_ERRNO_0_SIZE_0_OK	/* read() return of errno 0, size 0 is OK */

#if defined( HAL) || defined(SUN64)
#define LM_LONG_64BIT
#endif /*HAL || SUN64 */

#ifdef SUN64
#define USE_FD_SET_FOR_SELECT
#endif /* SUN64 */

#if !defined(SUN64) && !defined(RELEASE_VERSION)
/*- #define SUPPORT_METER_BORROW*/
#endif
#undef SYS_ERRLIST
#define SYS_ERRLIST(x) strerror(x)
#undef SYS_NERR
#define SYS_NERR 10000 /* big number */

#ifdef i386
#undef LM_SUPPORT_MT
#endif /* SUNPC */

#endif	/* SUNOS5 */


/*
 *	Vax (ultrix) modifications
 */

#ifdef ULTRIX

#define DEFAULT_HOSTID_ETHER
#define USE_ETHERNET_ADDR	/* Vax processors use ethernet address */
#define ULTRIX_ETHERNET_ADDR	/* Vax processors use ULTRIX */
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */

#define PIPE_EAGAIN_READ_BUG	/* Ultrix (incorrectly) returns EAGAIN on */
				/*		reads from a pipe */
#define VOID_CLEARERR
#define VOID_EXIT
#define VOID_FREE
#define INT_GETHOSTID
#define VOID_PERROR
#define UNSIGNED_SLEEP
#define LONG_TIME

#endif /* ULTRIX */

#if defined (VAX) && defined (ULTRIX_V2)
#define VOID_SLEEP
#endif


#ifdef ULTRIX_V3
#define VOID_SIGNAL		/* signal() returns void (*f)() on Ultrix 3.x */
#endif /* ULTRIX_V3 */


/*
 *	DECstation3100 (ultrix) modifications
 */

#ifdef PMAX
#include <sys/types.h>

#define USE_ETHERNET_ADDR	/* Ultrix machines use ethernet address */
#define ULTRIX_ETHERNET_ADDR	/* some mips processors use ULTRIX */
#define LOCK_FLOCK		/* Use flock(2) calls to lock daemon */
#define VOID_SIGNAL		/* signal() returns void (*f)() on Ultrix 3.x */
#define PIPE_EAGAIN_READ_BUG	/* Ultrix (incorrectly) returns EAGAIN on */
				/*		reads from a pipe */

#undef VOID_EXIT

#define VOID_CLEARERR
#define VOID_PERROR
#define UNSIGNED_SLEEP
#define SIZE_T_STRLEN
#define NO_TIMEZONE

#ifdef ULTRIX_V3
#undef VOID_STAR_MEMSET
#else
#define VOID_STAR_MEMCPY
#define VOID_STAR_MEMSET
#define VOID_STAR_CALLOC
#define VOID_STAR_MALLOC
#endif

#endif /* PMAX */


/*
 *	VMS mods
 */

#ifdef VMS

#ifdef __DECC   /* used to be ALPHA, this covers OPENVMS on VAX, too */
#define OPENVMS
#endif

#undef UNIX
#define DEFAULT_HOSTID_ETHER
#undef MULTIPLE_VD_SPAWN	/* Don't spawn multiple copies of v.d. */
#undef SUPPORT_UDP		/* VMS doesn't support UDP */
#define USE_ETHERNET_ADDR	/* Vax processors use ethernet address */
#define NO_PERF			/* No performance function in lm_hosttype() */
#define RE_START_TIMER  	/* Timers don't go off periodically */
#define USE_HOSTID		/* VMS system IDs are just like hostids */
#define NO_SSCANF_EMPTY_FIELDS_COUNT	/* sscanf() doesn't count empty */
					/*   matched fields */
#define LOCK_VMS		/* Use VMS locking calls to lock daemon */
#define DEFAULT_LIC_FILE_SPEC	/* Allow specifying default license file */
#define NO_INIT_SHARE		/* Don't initialize complex sharable data */


#define NO_TIMEZONE
#define NO_index
#define UNCONDITIONAL_LMADMIN	/* Everyone is an administrator */
#define index lu_index
#define VOID_STAR_MEMCPY
#define VOID_STAR_MEMSET
#define NO_SIGACTION
#define USE_SIGBLOCK
#define NO_LMGRD
#undef PATHSEPARATOR
#define PATHSEPARATOR ' '

#define READ_EF    flex_ef_1  /* EF set when read completes */
#define TIMEOUT_EF flex_ef_2  /* sleep()/select() timers EF */
#define TIMER_EF   flex_ef_3  /* EF set when timer completes */
#define TEMP_EF    flex_ef_4  /* Temporary for netio */
#define L_TIMER_EF flex_ef_5  /* Temporary for l_timers */

#define FLEXLM_VMS_EF1 READ_EF
#define FLEXLM_VMS_EF2 TIMEOUT_EF
#define FLEXLM_VMS_EF3 TIMER_EF
#define FLEXLM_VMS_EF4 TEMP_EF
#define FLEXLM_VMS_EF5 L_TIMER_EF

#define CPU_USAGE	/* Log CPU usage on VMS */
#undef SELECT_FD_SET

extern READ_EF;
extern TIMEOUT_EF;
extern TIMER_EF;
extern TEMP_EF;
extern L_TIMER_EF;

/*
 *	The sys_errlist[] array exists on VMS, but we need to put a
 *	function in to bypass it, since we have extended the error
 *	string array for the simulated TCP errors.
 */
extern char *flm_vms_sys_error();
#undef SYS_ERRLIST
#define SYS_ERRLIST(x) flm_vms_sys_error(x)

/*
 *	Re-define all the unix section 2 and 3 calls with
 *	new names so we don't collide with anything that
 *	the user might want to emulate.
 */
#define network_read flm_vms_net_read
#define network_write flm_vms_net_write
#define network_close flm_vms_net_close
#define fcntl flm_vms_fcntl
#define getdtablesize flm_vms_getdtablesize
#define gethostbyname flm_vms_gethostbyname
#define gethostid flm_vms_gethostid
#define gethostname lu_gethostname
#define getpwuid flm_vms_getpwuid
#define getservbyname flm_vms_getservbyname
#define getsockname flm_vms_getsockname
#define gettimeofday flm_vms_gettimeofday
#define hotnl flm_vms_htonl
#define ioctl flm_vms_ioctl
#define bind flm_vms_bind
#define listen flm_vms_listen
#define shutdown flm_vms_shutdown
#define accept flm_vms_accept
#define select flm_vms_select
#define socket flm_vms_socket
#define connect flm_vms_connect
#define ntohs flm_vms_ntohs
#define perror flm_vms_perror
#define recv flm_vms_recv
#define setitimer flm_vms_setitimer
#define setpgrp flm_vms_setpgrp
#define getpgrp flm_vms_getpgrp
#define setsockopt flm_vms_setsockopt
#define sleep flm_vms_sleep
#ifdef VAXC
#define mktime flm_vms_mktime
#endif

#define unlink remove

#ifdef OPENVMS
#define NO_SPRINTF
#define NO_TIME_T
extern flm_vms_net_read();
extern flm_vms_net_write();
extern flm_vms_net_close();
extern flm_vms_fcntl();
extern flm_vms_getdtablesize();
extern long flm_vms_gethostid();
extern flm_vms_getsockname();
extern flm_vms_gettimeofday();
extern short flm_vms_ntohs();
extern long flm_vms_htonl();
extern flm_vms_ioctl();
extern flm_vms_bind();
extern flm_vms_listen();
extern flm_vms_shutdown();
extern flm_vms_accept();
extern flm_vms_select();
extern flm_vms_socket();
extern flm_vms_connect();
extern lm_vms_ntohs();
extern flm_vms_recv();
extern flm_vms_setitimer();
extern flm_vms_setpgrp();
extern flm_vms_getpgrp();
extern flm_vms_setsockopt();
extern flm_vms_sleep();
extern flm_vms_decode_errno();
extern lu_gethostname();
extern char *lu_index();
extern lib$get_ef();
extern lib$getjpi();
extern sys$assign();
extern sys$cantim();
extern sys$clref();
extern sys$dassgn();
extern sys$qio();
extern sys$qiow();
extern sys$setef();
extern sys$setimr();
extern sys$setprn();
extern sys$waitfr();
#define VOID_SIGNAL
extern access();
extern chmod();
extern close();
extern dup();
extern execv();
extern getpid();
extern getuid();
extern mkdir();
extern pipe();
extern lu_setlinebuf();
extern fork();
extern wait();
extern lu_wait3();
extern int read(), write();
#ifndef NULL
#define NULL	0		/* They define this as (void *) NULL !!! */
#endif	/* NULL */
#endif	/* OPENVMS */

#define htons(x) (x)		/* Host-to-network byte order */
#define htonl(x) (x)		/* Host-to-network byte order */

#define SIGVTALRM SIGHUP	/* Careful with this one - it is REREAD */
/* #define SIGUSR1 31 */
/* #define SIGUSR2 30 */
/* #define SIGTSTP 29 */
/* #define SIGCHLD 28 */
#define NOFILE 256		/* Should be same as MAX_LINKS */
#define L_SET SEEK_SET		/* lseek() offset type */

#define NO_flock
#define NO_setlinebuf
#define NO_sigvec		/* No struct sigvec */
#define NO_regular_expressions	/* lmstat uses regular expressions */
#define NO_wait3

#define MAXINTERVAL 213		/* Max # of seconds to sleep */
#undef PATHTERMINATOR
#define PATHTERMINATOR ']'	/* Path ends in "]mumble.exe"" */

#define INT_SPRINTF

#endif /* VMS */

/*-
 *	Sizes of some statics that are used in the mainline code,
 *	and set to a memory buffer in Ultralite
 */

#define L_CRYPT_RESULT_LEN 100
#define L_KEY_O_LEN 4		/* x sizeof(long) */
#define L_STRKEY_BLOCKSIZE 8
#define L_STRKEY_MEMBUF_PTR_SIZE sizeof(char *)
#define L_STRKEY_RESULT_LEN 17

/*-
 *	ULTRALITE - just a checkout call for use in deeply embedded systems
 *	The ULTRALITE code uses only 8 C-runtime calls (no sprintf, no sscanf,
 *	etc), and is very easy to port to bare-bones hardware
 */

#ifdef FLEXLM_ULTRALITE

/*
 *	Replace system calls in the mainline FLEXlm code
 */
#define malloc(x) l_ul_malloc(x)
#define free(x) l_ul_free(x)
#define calloc(x,y) l_ul_calloc(x,y)
#define strcpy(x,y) l_ul_strcpy(x,y)

#define strlen(x) l_ul_strlen(x)
#undef SIZE_T_STRLEN
#define UNSIGNED_STRLEN

#define VOID_STAR_MEMCPY
#define VOID_STAR_MEMSET
#define memcpy(x,y,z) l_ul_memcpy(x,y,z)
#define memset(x,y,z) l_ul_memset(x,y,z)

#define strcpy(x,y) l_ul_strcpy(x,y)

#define isxdigit(x) l_isxdigit(x)
#define isdigit(x) l_isdigit(x)
#define isspace(x) l_isspace(x)
#define tolower(x) l_tolower(x)
#define toupper(x) l_toupper(x)
#define islower(x) l_islower(x)
#define isupper(x) l_isupper(x)

/*-
 *	There are certain statics that the HP embedded
 *	compiler can't support.  We define these
 *	absolutely here
 */

#define LM_FUL_MALLOC_SIZE 882	/*- This is the buffer in l_strkey() */
				/*-  in v7 for 64-bit systems */

#define LM_ULTRALITE_STATIC_SIZE (L_CRYPT_RESULT_LEN + \
				 (L_KEY_O_LEN * (sizeof(long))) + \
				 L_STRKEY_BLOCKSIZE + \
				 L_STRKEY_MEMBUF_PTR_SIZE + \
				 L_STRKEY_RESULT_LEN + \
				 LM_FUL_MALLOC_SIZE)

#define LM_UL_ALLOC_BASE *(flexlm_data)
extern char *flexlm_data;

#define LM_FUL_RESULT_PTR &(LM_UL_ALLOC_BASE) /*- l_crypt.c: */
					/*-   static char result[100] */
#define LM_FUL_O_PTR  LM_FUL_RESULT_PTR + L_CRYPT_RESULT_LEN
					/*- l_key.c: static long o[4] */
#define LM_FUL_Y_PTR  LM_FUL_O_PTR + (L_KEY_O_LEN * sizeof(long))
					/*- l_strkey.c: */
					/*- static char y[BLOCKSIZE] */
#define LM_FUL_MEMBUF_PTR LM_FUL_Y_PTR + L_STRKEY_BLOCKSIZE /*- l_strkey.c */
#define LM_FUL_RESULT2_PTR LM_FUL_MEMBUF_PTR + L_STRKEY_MEMBUF_PTR_SIZE
					/*- l_strkey.c: char *membuf */
#define LM_FUL_MALLOC_PTR LM_FUL_RESULT2_PTR + L_STRKEY_RESULT_LEN
#endif /* FLEXLM_ULTRALITE */

#ifdef UNIX
#define CPU_USAGE /* we log CPU usage on all Unix systems */
#endif

/*
 *	External definitions, mostly to keep lint happy (Keep in alphabetical
 *		order).
 */

#if (defined(__STDC__) || defined (__ANSI_C__)) && !defined(NeXT) && !defined(apollo)
#define ANSI
#endif
#if 1 /*#ifdef ANSI*/
#ifndef _NO_NO_NOT_STDLIB_
#ifdef gethostname
#undef gethostname
#define GETHN_DEF
#endif
/* this is where the problem is??? */
#ifndef FLEXLM_ULTRALITE
#include <stdlib.h>
#endif
#if defined(UNIX) && !defined(FLEXLM_ULTRALITE)
#include <unistd.h>
#endif /* UNIX  && !FLEXLM_ULTRALITE */
#ifdef GETHN_DEF
#define gethostname l_gethostname
#endif /* GETHN_DEF */

#endif /* ANSI */

#ifndef FLEXLM_ULTRALITE
#include <string.h>
#endif

#else /* __STDC__ || __ANSI_C__ */

#ifndef NeXT
#ifdef VOID_STAR_CALLOC
extern void *calloc();
#else
extern char *calloc();
#endif

#ifdef VOID_CLEARERR
extern void clearerr();
#endif

#ifdef CLOCK_T_CLOCK
extern clock_t clock();
#endif

#ifdef VOID_EXIT
extern void exit();
#endif

#ifdef VOID__EXIT
void _exit();
#endif

#ifdef SIZE_T_FREAD
extern size_t fread();
#endif

#ifdef VOID_FREE
extern void free();
#endif

#ifdef SIZE_T_FWRITE
extern size_t fwrite();
#endif

extern char *getenv();

#ifdef INT_GETHOSTID
int gethostid();
#endif

#ifdef PID_T_GETPID
extern pid_t getpid();
#endif

extern char *strchr();

#ifdef VOID_STAR_MALLOC
extern void *malloc();
#else
extern char *malloc();
#endif

#ifdef VOID_STAR_MEMCPY
extern void *memcpy();
#else
extern char *memcpy();
#endif

#ifdef VOID_STAR_MEMSET
extern void *memset();
#else
extern char *memset();
#endif

#ifdef VOID_PERROR
void perror();
#endif

#ifdef VOID_STAR_REALLOC
extern void *realloc();
#else
extern char *realloc();
#endif

extern char *strrchr();

#ifdef VOID_SLEEP
void sleep();
#endif

#ifdef UNSIGNED_SLEEP
unsigned sleep();
#endif

#ifdef SIZE_T_STRLEN
extern size_t strlen();
#endif

#ifdef UNSIGNED_STRLEN
extern unsigned int strlen();
#endif

#ifdef INT_SPRINTF
extern int sprintf();
#else
#ifdef NO_SPRINTF
#else
extern char *sprintf();
#endif
#endif

#ifdef LONG_TIME
long time();
#endif

#ifdef VMS
#ifndef OPENVMS
extern noshare int sys_nerr;
#endif
#else

#ifndef PC
extern char *sys_errlist[];
#endif

#endif

#ifndef NO_STR
extern char *strcat();
extern char *strncat();
extern char *strcpy();
extern char *strncpy();
#endif

#endif  /* not NeXT */
#endif  /* not ANSI */

/*
 *	accept() bug in USL socket library  (and in our TLI implementation)
 */
#if 0
#ifdef SVR4	/* This bug is fixed in Solaris 2.x */
#if !defined(SUNOS5) || defined(USE_TLI)
#define ACCEPT_BUG
#endif
#endif /* SVR4 */
#endif

/*
 *	End of lint defintions
 */

/*
 *	Derived defines
 */
#ifdef NO_bcopy
#define bcopy lu_bcopy
#endif

#ifdef NO_bzero
#define bzero lu_bzero
#endif

#ifdef NO_getdtablesize
#define getdtablesize lu_getdtablesize
#endif

#ifdef NO_gethostid
#define gethostid lu_gethostid
#endif

#ifdef NO_setitimer
#define setitimer lu_setitimer
#define getitimer lu_getitimer
#define NO_PERF			/* No performance function in lm_hosttype() */
#endif

#ifdef NO_setlinebuf
#define setlinebuf lu_setlinebuf
#endif

#ifdef NO_sigsetmask
#define sigsetmask lu_sigsetmask
#endif

#ifdef NO_sleep
#define sleep lu_sleep
#endif

#ifdef NO_wait3
#define wait3 lu_wait3
#endif

#ifdef NO_TIMEVAL
struct timeval { long tv_sec; long tv_usec; };
#endif

#ifdef NO_TIME_T
/*
 * For DEC C on VMS 6.1, which defines time_t if __TIME_T isn't defined,
 * but has an incomplete time.h
 */
#define __TIME_T 1
#define time_t long
#endif

#ifdef NO_ITIMERVAL
#ifndef timeval
#define FOOBAR
struct foobar { long tv_sec; long tv_usec; };
#define timeval foobar
#endif
struct itimerval { struct timeval it_interval; struct timeval it_value; };
#ifdef FOOBAR
#undef FOOBAR
#undef timeval
#endif	/* def FOOBAR */
#endif	/* def NO_ITIMERVAL */

#ifdef NO_SERVER
#undef MULTIPLE_VD_SPAWN
#undef SUPPORT_UDP
#endif


#ifdef SPARC_COMPLIANT
#define SPARC_COMPLIANT_FUNCS
#define lm_daemon lis_daemon
#define lm_feat_list lis_feat_list
#define lm_flush_config lis_flush_config
#define lm_free_daemon_list lis_free_daemon_list
#define lm_get_config lis_get_config
#define l_master_list lis_master_list
#define lm_next_conf lis_next_conf
#endif	/* SPARC_COMPLIANT */

/*
 *	Make sure there is some file locking defined
 */
#if !defined(LOCK_FLOCK) && !defined(LOCK_LOCKF) && !defined(LOCK_VMS) && !defined(LOCK_WINDOWS)
/*ERROR None_of_LOCK_FLOCK_or_LOCK_LOCKF_or_LOCK_VMS_are_defined */
#endif

/*
 *	Make sure only 1 type of file locking is defined
 */

#if defined(LOCK_FLOCK) && defined (LOCK_LOCKF)
ERROR - LOCK_FLOCK and LOCK_LOCKF both defined
#endif

#if defined(LOCK_FLOCK) && defined (LOCK_VMS)
ERROR - LOCK_FLOCK and LOCK_VMS both defined
#endif

#if defined(LOCK_LOCKF) && defined (LOCK_VMS)
ERROR - LOCK_LOCKF and LOCK_VMS both defined
#endif

/*
 *	We assume that if VD_MAKES_TCP_SOCKET is set, then ACCEPT_BUG is
 *	never set, too.
 *
 */

#if defined(VD_MAKES_TCP_SOCKET) && defined(ACCEPT_BUG)
ERROR - VD_MAKES_TCP_SOCKET and ACCEPT_BUG both set.
#endif

#ifdef USE_GETTEXT
#include <libintl.h>
#define LM_TEXTDOMAIN "FLEXlm"
#define lmtext(x) gettext(x)	/* get text string from message file */
#else
#define lmtext(x) x		/* text string */
#endif	/* USE_GETTEXT */

#ifndef FLEXLM_ULTRALITE
#ifdef USE_WINSOCK
#define	net_errno	WSAGetLastError()
#define NET_EINTR	WSAEINTR
#define NET_EBADF	WSAEBADF
#define NET_EAGAIN	WSATRY_AGAIN
#define NET_EMFILE	WSAEMFILE
#define NET_ECONNRESET	WSAECONNRESET
#define NET_ECONNABORTED	WSAECONNABORTED
#define NET_ETIMEOUT	WSAETIMEOUT
#define NET_EPIPE	WSAEINTR
#define NET_EIO		WSAEINTR
#define NET_ENOTSOCK	WSAENOTSOCK
#define NET_EWOULDBLOCK WSAEWOULDBLOCK
#define NET_EINPROGRESS WSAEINPROGRESS
#define NET_EADDRINUSE  WSAEADDRINUSE
#define NET_EINVAL	WSAEINVAL
#define NET_ECONNREFUSED WSAECONNREFUSED
#define NET_ETIMEDOUT   WSAETIMEDOUT
#define NET_ESHUTDOWN   WSAESHUTDOWN

#ifndef OS2
#define network_read(d, b, s) recv(d, b, s, 0)	  /* read(2) system call */
#define network_write(d, b, s)  send(d, b, s, 0)  /* write(2) system call */
#define network_close closesocket	/* close(2) system call */
#define network_control ioctlsocket	/* ioctl(2)/fcntl(2) system call */

#else


/*
 * All OS/2 socket calls are mapped through the DLL by calling the DLL's
 * exported version of these calls (the "l__" versions).
 */

#define network_read(d, b, s) l__recv(d, b, s, 0)
#define network_write(d, b, s)  l__send(d, b, s, 0)
#define network_close l__closesocket
#define network_control l__ioctlsocket

#endif /* OS2 */

#define LM_SOCKET unsigned int		/* Socket file descriptor (SOCKET) */
#define LM_BAD_SOCKET (unsigned int) (~0) /* Value for a "bad"/unused socket */
					/* (should be INVALID_SOCKET) */
					/* We define SOCKET and INVALID_SOCKET
					   here to avoid having to include
					   <winsock.h> in every file that
					   checks the socket - since this
					   is a "standard" it should be
					   no problem. */

#else		/* Generic Unix */

#define	net_errno	errno
#define NET_EINTR	EINTR
#define NET_EBADF	EBADF
#define NET_EAGAIN	EAGAIN
#define NET_EMFILE	EMFILE
#define NET_ECONNRESET	ECONNRESET
#define NET_ECONNABORT	ECONNABORT
#define NET_ESHUTDOWN	ESHUTDOWN
#define NET_ETIMEOUT	ETIMEOUT
#define NET_EPIPE	EPIPE
#define NET_EIO		EIO
#define NET_ENOTSOCK	EBADF
#define NET_EWOULDBLOCK EWOULDBLOCK
#define NET_EINPROGRESS EINPROGRESS
#define NET_EADDRINUSE  EADDRINUSE
#define NET_EINVAL	EINVAL
#define NET_ECONNREFUSED ECONNREFUSED
#define NET_ETIMEDOUT   ETIMEDOUT

#ifndef network_read
#define network_read(d, b, s) recv(d, b, s, 0 )	  /* read(2) system call */
#if 0
#define network_read read		/* read(2) system call */
#endif
#endif
#ifndef network_write
#define network_write(d, b, s)  send(d, b, s, 0)  /* write(2) system call */
#if 0
#define network_write write		/* write(2) system call */
#endif
#endif
#ifndef network_close
#define network_close close		/* close(2) system call */
#endif
#ifndef network_control
#define network_control fcntl		/* fcntl(2) system call */
#endif
#define LM_SOCKET int			/* Socket file descriptor */
#define LM_BAD_SOCKET -1		/* Value for a "bad" or unused socket */

#endif	/* USE_WINSOCK */
#endif /* FLEXLM_ULTRALITE */
#ifdef i386
#define flexlm_x86
#endif

#ifndef BCOPY_MACRO_IN_STRING_H
#define bcopy(src, dest, num)	memcpy((dest), (src), (num))
#define bzero(dest, num)	memset((dest), '\0', (num))
#endif /* BCOPY_MACRO */


#define getenv(x) l_getenv(job, x)
#if defined( UNIX) && defined (LM_SUPPORT_MT)
#define LM_UNIX_MT
#endif

#ifdef VXWORKS
#ifdef MIPS
#undef MIPS
#endif
#ifdef FNDELAY_IN_FILE_H
#undef FNDELAY_IN_FILE_H
#endif
#endif

#endif	/* _LM_MACHDEP_H  ***** LAST LINE IN FILE ***** */
