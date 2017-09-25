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
/******************************************************************************
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
 *****************************************************************************/
/*-
 *	Module: $Id: l_getattr.c,v 1.36 2003/05/31 03:13:10 jonl Exp $
 *
 *	Function:	l_getattr(key)
 *
 *	Description: 	Gets the value associated with the attribute "key"
 *
 *	Parameters:	(int) key - The key value required
 *
 *	Return:		(long) - the key's value
 *
 *	M. Christiano
 *	8/29/90
 *
 *	Last changed:  8/17/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include <stdio.h>
static char expdate[10] = "a001";	/*- 1-jan-80 */


#ifdef m88k
#define BCS88K_MOTOROLA		0x100
#define BCS88K_DG		0x200
#define BCS88K_ICON		0x500
#define BCS88K_ENCORE		0x600
#define BCS88K_HARRIS		0x1300
#define BCS88K_CETIA		0x1600
#define BCS88K_DOLPHIN		0x1000
#define BCS88K_OPUS		0xf00
#define BCS88K_UNISYS		0x1500
#define BCS88K_AYDIN_CONTROLS	0x1700
#define BCS88K_MODCOMP		0x1800


#ifdef DGUX
#ifdef SVR4
#include <sys/_int_unistd.h>
#else
#include <sys/m88kbcs.h>
#endif
#endif	/* DGUX */
#if defined( MOTO_88K) || defined(MOTOSVR4)
#include <unistd.h>
#endif	/* MOTO_88K */
long sysconf();
#endif	/* defined(MOTO_88K) || defined(DGUX) */

extern int l_c();
static int getattr_init lm_args(( LM_HANDLE *, unsigned long *, char *));

long API_ENTRY
l_getattr(job, key)
LM_HANDLE *job;
int key;
{
  int ret;
	if (! (job->flags & LM_FLAG_CLEAR_VKEYS))
		l_xorname(job->vendor, &job->code);
	ret = ((long)job->attrs[key]);
	if (! (job->flags & LM_FLAG_CLEAR_VKEYS))
		l_xorname(job->vendor, &job->code);
	return ret;
}

int
l_getattr_init(job, key, vendor_id)
LM_HANDLE *job;		/* Current license job */
VENDORCODE *key;
char *vendor_id;
{
  int ret = 0;
  unsigned long crokeys[4];
	memset(crokeys, 0, sizeof(crokeys));
	crokeys[1] = key->crokeys[0]; crokeys[2] = key->crokeys[1];
	job->flags |= LM_FLAG_CLEAR_VKEYS;
	if (!getattr_init(job, key->keys, vendor_id))
		getattr_init(job, crokeys, vendor_id);
	else
	{
		l_xorname(vendor_id, key);
		l_clear_error(job);
		ret = getattr_init(job, key->keys, vendor_id);
		getattr_init(job, crokeys, vendor_id);
		l_xorname(vendor_id, key);
		job->flags &= ~LM_FLAG_CLEAR_VKEYS;
	}
	return ret;
}


static
int
getattr_init(job, inkey, vendor_id)
LM_HANDLE *job;		/* Current license job */
unsigned long *inkey;
char *vendor_id;
{
  unsigned long keys[4];
  int crokeys = inkey[0] ? 0 : 1; /* if inkey[0] is 0, then it's crokeys */
#ifndef PC
  int platform_variation;
#else
DWORD platform_variation;
int win32s;
#endif
  unsigned long x, y;
#ifdef EMBEDDED_FLEXLM
  static unsigned long _tk[4];   /*- In place of the memory in l_key() */
#endif



#ifndef EMBEDDED_FLEXLM		/* Don't do any of this for embedded */
				/* this will skip all the platform-specific
				   checks, right down to the end */

	if (crokeys)
	{
		if (job->attrs[REAL_KEYS] != REAL_KEYS_VAL)
		{
			job->attrs[LM_PUBKEY_ATTR] = LM_PUBKEY_ATTR_VAL;
			job->attrs[LM_PHASE2_ATTR] = LM_PHASE2_ATTR_VAL;
			return 0;
		}
	}
	else memset((char *)job->attrs, 0, sizeof(job->attrs));
/*-
 *	First, verify the key
 */
	l_key(vendor_id, inkey, keys, 4);
/*-					JUNK only in top 16 bits as of v2.2 */
/*-					JUNK XORed with date as of v2.4 */
	if (crokeys)
	{
	  int i;
		if (!inkey[1] ||! inkey[2])
			return 0;
		x = 0;
		for (i = 1; i < 4; i++)
			x ^= keys[1] >> (i * 8);
		for (i = 0; i < 4; i++)
			x ^= keys[2] >> (i * 8);
		if ((x  & 0x7f)!= (keys[1] & 0x7f))
		{
			LM_SET_ERRNO(job, LM_BADKEYDATA, 541, 0);
			return(-1);
		}
	}
	else
	{
		x = keys[3] ^ JUNK;
		x >>= 16;
		x &= 0xffff;
		x -= (keys[3] & 0xffff);
		y = keys[1] & 0x7f; keys[1] &= ~0x7f;
		if (x || y != (unsigned long) l_c(keys))
		{
			LM_SET_ERRNO(job, LM_BADKEYDATA, 49, 0);
			return(-1);
		}
	}
/*-
 *	Make sure we are on a supported platform
 */
#undef GOTONE
#ifdef VXWORKS
#define GOTONE
	if ((keys[2]) & PLAT_vxworks)
#endif  /* VXWORKS */


#if defined(ALPHA) || defined(_ALPHA_)
#define GOTONE
#ifdef WINNT
	if ((keys[2]) & PLAT_winnt_alpha)
#endif
#ifdef OSF
	if ((keys[1]) & PLAT_alpha_osf)
#endif
#ifdef VMS
	if ((keys[1]) & PLAT_alpha_vms)
#endif
#endif /* ALPHA */

#ifdef LYNX
#define GOTONE
#ifdef LYNX_PPC
      if ((keys[1]) & PLAT_ppc_lynx)
#else
      if ((keys[2]) & PLAT_intel_lynx)
#endif /* LYNX_PPC */

#endif /* LYNX */

#ifdef MAC10
#define GOTONE
	if ((keys[2]) & PLAT_ppc_mac)
#endif /* MAC10  */

#ifdef LINUX
#define GOTONE
#ifdef RHLINUX64
	if ((keys[1]) & PLAT_it64_lr)
#else
#ifdef VMLINUX64
	if ((keys[1]) & PLAT_it64_lv)
#else
#ifdef MONTAVISTA
	if ((keys[2]) & PLAT_montavista)
#else
	if ((keys[2]) & PLAT_linux)
#endif
#endif /* RHLINUX64*/
#endif /* VMLINUX64*/
#endif /* LINUX */

#ifdef FREEBSD
#define GOTONE
#ifdef OPENBSD
	if ((keys[2]) & PLAT_openbsd)
#else
	if ((keys[2]) & PLAT_freebsd)
#endif /* OPENBSD */
#endif /* FREEBSD */
#ifdef BSDI
#define GOTONE
	if ((keys[2]) & PLAT_bsdi)
#endif /* BSDI */
#ifdef TANDEM
#define GOTONE
	if ((keys[2]) & PLAT_tandem)
#endif /* TANDEM */

#ifdef ALPHA_LINUX
#define GOTONE
	if ((keys[2]) & PLAT_alpha_linux)
#endif /* ALPHA_LINUX */


#ifdef apollo
#define GOTONE
	if ((keys[1]) & PLAT_apollo)
#endif
#ifdef convex
#define GOTONE
	if ((keys[1]) & PLAT_convex)
#endif
#ifdef UNIXWARE
#define GOTONE
	if ((keys[1]) & PLAT_unixware)
#endif /* UNIXWARE */

#ifdef _CRAY
#define GOTONE
#ifdef CRAY_NV1
	if ((keys[2]) & PLAT_cray_nv1)
#endif /* NV1 */
#ifdef CRAY_J90
	if ((keys[2]) & PLAT_cray_j90)
#endif /* J90 */
#ifdef CRAY_T3E
	if ((keys[2]) & PLAT_cray_t3e)
#endif /* T3E */
#ifdef CRAY_C90
	if ((keys[2]) & PLAT_cray_c90)
#endif /* C90 */
#ifdef CRAY_T90
	if ((keys[2]) & PLAT_cray_t90)
#endif /* T90 */
#endif /* CRAY */

#ifdef ENCORE
#define GOTONE
	if ((keys[2]) & PLAT_encore)
#endif /* ENCORE */

#ifdef DGX86
#define GOTONE
	if ((keys[2]) & PLAT_dg_intel)
#endif /* DGX86 */

#ifdef m88k
#ifndef GOTONE
#define GOTONE
	platform_variation = sysconf(_SC_BCS_VENDOR_STAMP);
	if (((keys[1]) & PLAT_88k_bcs) ||
	    ((platform_variation == BCS88K_DG) && ((keys[1]) & PLAT_dg)) ||
	    ((platform_variation == BCS88K_MOTOROLA) &&
						(keys[1]) & PLAT_motorola))
#endif
#endif /* m88k */

#ifdef HPINTEL_64
#define GOTONE
	if ((keys[1]) & PLAT_it64_hp)
#endif  /* HPINTEL_64  */

#ifdef HPINTEL_32
#define GOTONE
	if ((keys[1]) & PLAT_it32_hp)
#endif  /* HPINTEL_32  */

#ifdef HP300
#define GOTONE
	if ((keys[1]) & PLAT_hp300)
#endif
#ifdef HP700
#define GOTONE
#ifdef HP64
      if ((keys[2]) & PLAT_hp64)
#else
	if ((keys[1]) & PLAT_hp700)
#endif
#endif /* HP700 */
#ifdef ICL
#ifdef flexlm_x86
#define GOTONE
	if ((keys[2]) & PLAT_icl_intel)
#endif
#ifdef sparc
#define GOTONE
	if ((keys[2]) & PLAT_icl_sparc)
#endif
#endif
#ifdef integraph
#define GOTONE
	if ((keys[2]) & PLAT_integraph)
#endif
#ifdef _ABI_SOURCE
#define GOTONE
	if ((keys[2]) & PLAT_mips_abi)
#endif /* ABI_SOURCE */
#ifdef NECSX4
#define GOTONE
	if ((keys[2]) & PLAT_necsx4)
#endif /* NECSX4 */
#if (defined (mips) || defined (MIPS) || defined(_MIPS_)) && !defined(PMAX) && !defined(SGI) && !defined(sony_news) && !defined(WINNT) && !defined(sinix)
#define GOTONE
#ifdef NEC
	if ((keys[2]) & PLAT_nec_ews)
#else
	if ((keys[1]) & PLAT_mips)
#endif /* NEC */
#endif /* mips */

#if defined (_MIPS_) && defined(WINNT)
#define GOTONE
	if ((keys[2]) & PLAT_winnt_mips)
#endif
#ifdef NCR
#define GOTONE
	if ((keys[1]) & PLAT_ncr)
#endif
#ifdef NeXT
#define GOTONE
	if ((keys[1]) & PLAT_next)
#endif
#if defined( PC) && !defined(_ALPHA_)
	#if defined(WINNT) && !defined(_ALPHA_)
		#if defined (_PPC_)
			#define GOTONE
			if ((keys[2]) & PLAT_winnt_ppc)
		#else /* !PPC */
			#define GOTONE
		       {
			       platform_variation  = GetVersion();
			       win32s =(( platform_variation >
						0x80000000 ) &&
					 (LOBYTE(LOWORD(
					 platform_variation))<4)) ;

		       }
		       if ( (((keys[2]) & PLAT_winnt_intel) &&
				!win32s) || (win32s && (keys[2]) &
				PLAT_windows)  )
		#endif /* PPC */
	#else
		#if defined( _WINDOWS ) && !defined (_ALPHA_)
			#define GOTONE
			if ((keys[2]) & PLAT_windows )
		#else
			#if defined(OS2)
				#define GOTONE
				if ((keys[2]) & PLAT_os2 )
			#else
				#define GOTONE
				if ((keys[1]) & PLAT_netware)
			#endif
		#endif
	#endif
#endif

#ifdef RS6000
#define GOTONE
#ifdef IT32_A
	if ((keys[1]) & PLAT_it32_a)
#else

#ifdef AIX64
	if ((keys[1]) & PLAT_it64_a)
#else
#ifdef RS64
	if ((keys[1]) & PLAT_rs64)
#else
#ifdef AIX_PPC
	if ((keys[2]) & PLAT_aix_ppc)
#else /* !PPC */
	if ((keys[1]) & PLAT_rs6000)
#endif /* IT32_A */
#endif /* AIX64 */
#endif /* RS64 */
#endif /* AIX_PPC */
#endif /* RS6000 */

#ifdef sco
#define GOTONE
	if ((keys[1]) & PLAT_sco)
#endif
#ifdef sinix
#define GOTONE
#ifdef flexlm_x86
	if ((keys[2]) & PLAT_siemens_intel)
#else
	if ((keys[2]) & PLAT_siemens_mips)
#endif
#endif
#ifdef SGI
#define GOTONE
#ifdef SGI6
#ifdef SGI32
	if ((keys[1]) & PLAT_sgi)  /*- 32-bit platform, probably n32 */
#else /* must be 64-bit */
	if ((keys[2]) & PLAT_sgi_r8000) /*- 64-bit platform */
#endif /* SGI32 */
#else /* !SGI6 */
	if ((keys[1]) & PLAT_sgi) /* built on a MIPS3 -- 32-bit, probably o32 */
#endif /* SGI6 */
#endif /* SGI */
#ifdef sony_news
#define GOTONE
	if ((keys[1]) & PLAT_sony_news)
#endif

#ifdef HAL
#define GOTONE
	if ((keys[1]) & PLAT_hal)
#endif /* HAL */

#ifdef SUN64
#define GOTONE
	if  ((keys[2]) & PLAT_sun64)
#endif

#if (defined(sun) || defined(sparc)) && !defined(sun386) && !defined(GOTONE)
#define GOTONE
#ifdef sparc
	if  (((keys[1]) & PLAT_sun) || ((keys[2]) & PLAT_sun32))
#else
#ifdef flexlm_x86
	if ((keys[1]) & PLAT_sunx86)
#else
#ifdef PPC
	if ((keys[2]) & PLAT_sun_ppc)
#else
	if (((keys[1]) & PLAT_sun) || ((keys[2]) & PLAT_sun32))
#endif /* PPC */
#endif /* flexlm_x86 */
#endif /* sparc */
#endif
#ifdef sun386
#define GOTONE
	if ((keys[1]) & PLAT_sun386)
#endif
#if defined(VMS) && !defined(ALPHA)     /* Alpha VMS handled above */
#define GOTONE
	if ((keys[1]) & PLAT_vms)
#else	/* VMS */

#if defined(PMAX) || defined(vax)
#define GOTONE
	if ((keys[1]) & PLAT_dec)
#endif	/* PMAX || vax */

#endif	/* VMS */

#else	/*- EMBEDDED_FLEXLM	- enable everything for EMBEDDED_FLEXLM */

#define GOTONE
	keys = _tk;
	keys[0] = 0xffffffff;
	if (1)

#endif	/* EMBEDDED_FLEXLM */

#ifndef GOTONE
	if ((keys[0] & ((long) 1 << FLEXlm_SOURCE))) /*- Allow source licenses to
					build on any unsupported platform */
#endif
	{
		if (crokeys)
		{
			job->attrs[LM_PUBKEY_ATTR] = LM_PUBKEY_ATTR_VAL;
			job->attrs[LM_PHASE2_ATTR] = LM_PHASE2_ATTR_VAL;
			return 0;
		}
/*-
 *		Get and verify the expiration date
 */
		if (keys[0] & ((long) 1 << NO_EXPIRE))
				job->attrs[NO_EXPIRE] = NO_EXPIRE_VAL;
		else
		{
				(void) sprintf(expdate, "%x", ((keys[3]) & 0x0ffff));
				if (l_date(job, l_asc_date(expdate), L_DATE_EXPIRED))
				{

					LM_SET_ERRNO(job, LM_EXPIREDKEYS, 51,
									0);
					return(-1);
				}
		}
		if (keys[0] & ((long) 1 << DUP_SERVER))
				job->attrs[DUP_SERVER] = DUP_SERVER_VAL;
		if (keys[0] & ((long) 1 << EXTENDED_HOSTID))
				job->attrs[EXTENDED_HOSTID] = EXTENDED_HOSTID_VAL;
		if (keys[0] & ((long) 1 << CLOCK_SETTING))
				job->attrs[CLOCK_SETTING] = CLOCK_SETTING_VAL;
		if (keys[0] & ((long) 1 << SWITCH_LOGFILE))
				job->attrs[SWITCH_LOGFILE] = SWITCH_LOGFILE_VAL;
		if (keys[0] & ((long) 1 << LOG_SUPPORT))
				job->attrs[LOG_SUPPORT] = LOG_SUPPORT_VAL;
		if (keys[0] & ((long) 1 << LF_PATH))
				job->attrs[LF_PATH] = LF_PATH_VAL;
		if (keys[0] & ((long) 1 << LM_DONGLE1))
				job->attrs[LM_DONGLE1] = LM_DONGLE1_VAL;
		if (keys[0] & ((long) 1 << LMADMIN_API))
				job->attrs[LMADMIN_API] = LMADMIN_API_VAL;
		if (keys[0] & ((long) 1 << LM_RUN_DAEMON))
				job->attrs[LM_RUN_DAEMON] = LM_RUN_DAEMON_VAL;
		if (keys[0] & ((long) 1 << LM_DONGLE2))
				job->attrs[LM_DONGLE2] = LM_DONGLE2_VAL;
		if (keys[0] & ((long) 1 << LM_DONGLE3))
				job->attrs[LM_DONGLE3] = LM_DONGLE3_VAL;
		if (keys[0] & ((long) 1 << LM_DONGLE4))
				job->attrs[LM_DONGLE4] = LM_DONGLE4_VAL;
		if (keys[0] & ((long) 1 << DAEMON_INIT))
				job->attrs[DAEMON_INIT] = DAEMON_INIT_VAL;
		if (keys[0] & ((long) 1 << END_USER_OPTIONS))
				job->attrs[END_USER_OPTIONS] = END_USER_OPTIONS_VAL;
		if (keys[0] & ((long) 1 << REREAD))
				job->attrs[REREAD] = REREAD_VAL;
		if (keys[0] & ((long) 1 << START_DATE))
				job->attrs[START_DATE] = START_DATE_VAL;
		if (keys[0] & ((long) 1 << GROUP_DUP))
				job->attrs[GROUP_DUP] = GROUP_DUP_VAL;
		if (keys[0] & ((long) 1 << FEATURESET))
				job->attrs[FEATURESET] = FEATURESET_VAL;
		if (keys[0] & ((long) 1 << ULIST))
				job->attrs[ULIST] = ULIST_VAL;
		if (keys[0] & ((long) 1 << FULL_FLEXLM))
				job->attrs[FULL_FLEXLM] = FULL_FLEXLM_VAL;
		if (keys[0] & ((long) 1 << CHECKOUT))
				job->attrs[CHECKOUT] = CHECKOUT_VAL;
		if (keys[0] & ((long) 1 << FILTERS))
				job->attrs[FILTERS] = FILTERS_VAL;
		if (keys[0] & ((long) 1 << LINGERING))
				job->attrs[LINGERING] = LINGERING_VAL;
		if (keys[0] & ((long) 1 << ADDITIVE))
				job->attrs[ADDITIVE] = ADDITIVE_VAL;
		if (keys[0] & ((long) 1 << WRAPPER))
				job->attrs[WRAPPER] = WRAPPER_VAL;
		if (keys[0] & ((long) 1 << LM_SET_ATTR))
				job->attrs[LM_SET_ATTR] = LM_SET_ATTR_VAL;
		if (keys[0] & ((long) 1 << MULTIPLE_JOBS))
				job->attrs[MULTIPLE_JOBS] = MULTIPLE_JOBS_VAL;
	}
	else  if (!crokeys)
	{
		LM_SET_ERRNO(job, LM_BADPLATFORM, 52, 0);
		return(-1);
	}

	return(0);
}

char * API_ENTRY
l_getexp()
{
	return(l_asc_date(expdate));
}

void
l_xorname(name, vc)
char *name;
VENDORCODE *vc;
{
#define VENDORMAGIC_V7 0x08BC0EF8
  int i;
  char buf[MAX_VENDOR_NAME + 1];
	/* make sure vendor name is all zeros after the first zero */

	memset(buf, 0, sizeof(buf));
	strcpy(buf, name);

	for (i = 0;i < 4; i++)
		vc->keys[i] &= 0xffffffff; /* 64-bit fix */
	vc->keys[0] ^= buf[0] ^
		(buf[1] << 8) ^
		(buf[2] << 16) ^
		(buf[3] << 24)  ^ VENDORMAGIC_V7;
	vc->keys[1] ^= buf[2] ^
		(buf[5] << 8) ^
		(buf[7] << 16) ^
		(buf[4] << 24)  ^ VENDORMAGIC_V7;
	vc->keys[2] ^= buf[4] ^
		(buf[6] << 8) ^
		(buf[1] << 16) ^
		(buf[6] << 24)  ^ VENDORMAGIC_V7;
	vc->keys[3] ^= buf[5] ^
		(buf[0] << 8) ^
		(buf[2] << 16) ^
		(buf[3] << 24)  ^ VENDORMAGIC_V7;

}
