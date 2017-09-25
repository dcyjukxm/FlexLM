/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *		the right to run FLEXlm on any platform of his choice.
 *		Modification of this, or any other file with the intent
 *		to run on an unlicensed platform is a violation of your
 *		license agreement with Macrovision Corporation.
 *
 *
 *****************************************************************************/
/*
 *	Module: $Id: lm_getid_typ.c,v 1.77 2003/05/31 03:13:10 jonl Exp $
 *	Function: lc_getid_type(job, type)
 *
 *	Description: FLEXlm equivalent of Unix gethostid()
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) type - Type of ID requested
 *
 *	Return:		(HOSTID *) - the FLEXlm host ID.
 *
 *	M. Christiano
 *	9/3/89
 *	Last changed:  12/29/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "lgetattr.h"
#include "flex_file.h"
#include "flex_utils.h"
#include <errno.h>
#ifndef FLEXLM_ULTRALITE
#include <time.h>
#endif /* !ULTRALITE */
#ifdef sco
#include <sys/types.h>
#include <sys/utsname.h>
#endif /* sco */

#ifdef EMBEDDED_FLEXLM
#undef HOSTID_DOMAIN
#endif /* EMBEDDED_FLEXLM */

#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#include <netdb.h>
#endif /* USE_WINSOCK */
#ifdef GOTONE
#undef GOTONE
#endif
#ifdef apollo
#include <apollo/base.h>
#include <apollo/error.h>
#include <apollo/pm.h>
/* std_$call void pm_$get_sid_txt();  Returns string: name.prj.org.node_id */
static int first = 1;
static int real_id;
#endif
#ifdef NECSX4
#include <sys/types.h>
#include <sys/syssx.h>
#else
#ifdef NEC
#include <sys/utsname.h>
#include <sys/systeminfo.h>
#endif /* NEC */
#endif /* NECSX4 */
#ifdef SUNOS5
#include <sys/systeminfo.h>
#endif


#ifdef cray
#include <unistd.h>
#endif /* cray */

#ifdef FREEBSD
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>

#endif /* FREEBSD */
#ifdef LYNX
#include <sys/types.h>
#include <stdio.h>
#include <uio.h>
#include <socket.h>
#include <info.h>
#include <sys/file.h>
#include <netinet/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <bsd/if_var.h>
#include <bsd/if_arp.h>

static
read_from_kernel(fd, off, bufptr, size)
int fd, off, size;
char *bufptr;
{
	if (lseek(fd, off, 0) == -1 ) return 0;
	if (read(fd, bufptr, size) != size) return 0;
	return(size);
}

#endif /* LYNX */


#ifdef LINUX
#define GOTONE
#if !defined( GLIBC) && !defined(ALPHA_LINUX) && !defined(VMLINUX64)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/ipx.h>
#else
#include <linux/sockios.h>
#include <linux/if.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#ifdef DGX86
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/dg_inet_info.h>
#endif /*DGX86*/


#ifdef convex
#include <sys/sysinfo.h>
static int first = 1;
static int real_id;
#endif

#ifdef UNIXWARE
#include <fcntl.h>
#include <sys/types.h>
#include <sys/dlpi_ether.h>
#include <stropts.h>
#endif /* UNIXWARE */
#if defined(SCO5)
#include <stropts.h>
#include <fcntl.h>
#include <sys/dlpi.h>
#include <sys/mdi.h>
#include <sys/dlpi_ether.h>
#endif /* SCO5 */
#ifdef SCO
#include <sys/types.h>
#include <sys/utsname.h>
#define SCOINFO 12840
#endif /* SCO */


#if defined (BSDI)
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
static void bsdi_gethostid(LM_HANDLE *job);
#endif /* BSDI */


#ifdef PC
#include <windows.h>
#endif

#ifdef MAC10

static void mac_ether(LM_HANDLE *job);
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <paths.h>
#include <sysexits.h>
#include <sys/param.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOEthernetController.h>
typedef struct {
	UInt8 MACAddress[kIOEthernetAddressSize ];
	UInt8 activeMedium[32];
	long long linkSpeed;
	long linkStatus;
	Boolean isBuiltIn;
} EnetData;
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices);
static kern_return_t GetEthernetData(io_iterator_t intfIterator, EnetData *edata, UInt32 *gNumEntries);
#endif /* MAC 10 */

#if defined(PC) && !defined(WINNT)
/*
 *	Standard MS-DOS disk information structure.
 */
typedef struct tagMEDIAID
{
    WORD  wInfoLevel;
    DWORD dwSerialNum;     /* Serial number		*/
    char  VolLabel[11];    /* ASCII volume label	*/
    char  FileSysType[8];  /* File system type		*/
} MEDIAID, far *LPMEDIAID;

/*
 *	Standard DPMI real mode register structure.
 */
typedef struct tagREALMODEREG {
    DWORD rmEDI, rmESI, rmEBP, Reserved, rmEBX, rmEDX, rmECX, rmEAX;
    WORD  rmCPUFlags, rmES, rmDS, rmFS, rmGS, rmIP, rmCS, rmSP, rmSS;
} REALMODEREG, FAR *LPREALMODEREG;

#endif /* PC */

#if defined(PC)
typedef HOSTID * (LM_CALLBACK_TYPE * LM_CALLBACK_GET_VID) (short idtype);
#define LM_CALLBACK_GET_VID_TYPE (LM_CALLBACK_GET_VID)
#else
#define LM_CALLBACK_GET_VID_TYPE
#endif

static HOSTID *l_getid_hw lm_args((LM_HANDLE *, int));
static HOSTID *l_getdomain lm_args(( LM_HANDLE *));

#ifdef SUPPORT_METER_BORROW
static HOSTID *l_borrow_hostid(LM_HANDLE *job);
#endif /* SUPPORT_METER_BORROW */

#if 1 /*def PC*/
static void move_to_end_of_idptr_list lm_args(( LM_HANDLE_PTR, HOSTID_PTR ,
							HOSTID_PTR));
#endif
void l_free_one_hostid lm_args((HOSTID_PTR));

#ifdef PC
HANDLE sHandle2, sHandle3, sHandle4, sHandle5;
DWORD dwWaitResult;
#endif

HOSTID * API_ENTRY
lc_getid_type(job, idtype)
LM_HANDLE *job;		/* Current license job */
int idtype;		/* Type of hostid desired -- short promoted to int */
{
	if (LM_API_ERR_CATCH) return 0;
	LM_API_RETURN(HOSTID *,l_getid_type(job, idtype))
}

HOSTID * API_ENTRY
l_getid_type(job, idtype)
LM_HANDLE *job;		/* Current license job */
int idtype;		/* Type of hostid desired -- short promoted to int */
{
  HOSTID *newidptr, *h, *last;
  HOSTID *x;
  HOSTID * in_list;
  int dynamic_hostid;

#ifndef FLEXLM_ULTRALITE
#include <sys/types.h>
#include <time.h>
  long currtime = (long)time(0);
  static char * lm_debug_hostid= (char*) -1;
  static int hostid_check_interval;

	if ( lm_debug_hostid == (char *) -1)
        {
                lm_debug_hostid=getenv("LM_DEBUG_HOSTID");	/* overrun checked */
                if (lm_debug_hostid)
                {
                        hostid_check_interval=(int) strtol(
                                                lm_debug_hostid,
                                                (char ** ) NULL,
                                                10);
                        if ((hostid_check_interval < 1) ||
					(hostid_check_interval > 30))
                                hostid_check_interval = CHECK_HOSTID_INTERVAL;
                }
                else
                        hostid_check_interval = CHECK_HOSTID_INTERVAL;
        }

	if (l_getattr(job, ADVANCED_HOSTIDS)!=ADVANCED_HOSTIDS_VAL)
	{
		switch(idtype)
		{
		case HOSTID_ANY:
		case HOSTID_FLEXLOCK:
		case HOSTID_DEMO:
		case HOSTID_FLEXID1_KEY:
		case HOSTID_FLEXID2_KEY:
		case HOSTID_FLEXID3_KEY:
		case HOSTID_FLEXID4_KEY:
		case HOSTID_FLEXID6_KEY: break;
		default:	LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 285, 0);
			return 0;
		}
	}
#endif	/* !FLEXLM_ULTRALITE */

/*
 *	Bug P1124 fix 4/2/96 -- PC only for now, should be for all later.
 */
/*
 *	Have to get the last idptr in the job
 */
	last=job->idptr;
#ifndef FLEXLM_ULTRALITE
#ifdef SUPPORT_METER_BORROW
 	if (idtype != HOSTID_METER_BORROW)
#endif
	{
		in_list=0;
		while (last)
		{
			if ( l_id_types_match(last->type, idtype) && !in_list )
				in_list=last;
			if (!last->next) break;
			last=last->next;
		}

		dynamic_hostid= LM_IS_FLEXID(idtype) ||
				(idtype > HOSTID_VENDOR);



	/*
	 *      If idtype is in list and if it is not volatile(dynamic), then
	 *      move it to the end of list and return it
	 */

		if (in_list && ! dynamic_hostid)
		{
			move_to_end_of_idptr_list(job, in_list, last);
			return in_list ; /* use what's already in the list */

		}
		if (!in_list || ((currtime - job->last_idptr_time) >
							hostid_check_interval))
		{
			lc_free_hostid(job, job->idptr);
			job->idptr = 0;
			job->last_idptr_time = currtime;
			last=0;
			 /* reset pointer to null, this fixes probelm on novell with
				two dongles  */
		}
		else
		{
			for (h = job->idptr; h; h = h->next)
				if (l_id_types_match(h->type, idtype))
				{
					move_to_end_of_idptr_list(job, h, last);
					return h ; /* use what's already in the list */
				}
		}
	}
#endif /* ULTRALITE */

	if (!(newidptr = (HOSTID *)l_new_hostid()))
	{
		return 0;
	}

	newidptr->type = idtype;
	if (idtype == HOSTID_USER)
	{
		l_zcp(newidptr->hostid_user, lc_username(job, 1), MAX_USER_NAME);/* LONGNAMES */
	}
	else if (idtype == HOSTID_DISPLAY)
	{
		l_zcp(newidptr->hostid_display, lc_display(job, 1),
							MAX_DISPLAY_NAME);/* LONGNAMES */
	}
	else if (idtype == HOSTID_HOSTNAME)
	{
		l_zcp(newidptr->hostid_hostname, lc_hostname(job, 1),
							MAX_SERVER_NAME);/* LONGNAMES */
	}
	else if (idtype == HOSTID_COMPOSITE)
	{
		l_zcp(newidptr->hostid_composite, l_composite_id(job),
							MAX_HOSTID_LEN);
	}

#ifdef SUPPORT_METER_BORROW
	else if (idtype == HOSTID_METER_BORROW)
	{
	  HOSTID *x;
		l_free_one_hostid(newidptr);
		newidptr = 0;
		if (x = l_borrow_hostid(job)) newidptr = x;
	}
#endif /* SUPPORT_METER_BORROW */
#ifdef HOSTID_DOMAIN
	else if (idtype == HOSTID_DOMAIN)
	{
	  HOSTID *d;
		if (d = l_getdomain(job))
		{
			l_free_one_hostid(newidptr);
			newidptr = d;
		}
	}
#endif
#ifndef EMBEDDED_FLEXLM
	else if (idtype == HOSTID_INTERNET)
	{
	  char host[128];
	  unsigned char ipaddr[5];
	  int i;
		if (gethostname(host, 127) ||
			!l_get_ipaddr(host, (char *)ipaddr, 0, 0))
		{
			/* failed */
			job->u_errno = errno;
			l_free_one_hostid(newidptr);
			newidptr = 0;
		}
		for (i = 0; i < 4; i++)
			newidptr->hostid_internet[i] = ipaddr[i];
	}
#endif /* !EMBEDDED FLEXLM */
	else if (idtype >= HOSTID_VENDOR && job->options->get_vendor_id)
	{
/*
 *		For vendor-defined -- the vendor has to malloc this,
 *		partly because it may be a list, and partly because
 *		I have no mechanism of giving him my pointer.
 */
		l_free_one_hostid(newidptr);
		if (x = (*LM_CALLBACK_GET_VID_TYPE
		     job->options->get_vendor_id)((short)idtype))
		{
		  LM_VENDOR_HOSTID *vp;
		  int case_sensitive = 0;
			/* P3373 */
			for (vp = job->options->vendor_hostids;
				vp; vp = vp->next)
			{
				if (vp->hostid_num == x->type)
				{
					case_sensitive =
						vp->case_sensitive;
					break;
				}
			}
			/* end P3373 */
			if (!case_sensitive) l_uppercase(x->id.string);
		}
		newidptr = x;
	}
        else if ( idtype == HOSTID_FLEXID_FILE_KEY )
	{
	  char *str = 0;
	  char *cp = 0, *last = 0;
	  HOSTID *h = newidptr;
	  char buf[500];

                if (!l_get_registry(job, "HOSTID", &str, 0, 0))
		{
			strcpy(buf, str);
			h->type = HOSTID_FLEXID_FILE_KEY;
			for (last = buf; last && *last; last = cp)
			{
				if (last != buf)
				{
					h->next = l_new_hostid();
					h = h->next;
					h->type = HOSTID_FLEXID_FILE_KEY;
				}
				if ((cp = strchr(last, ' ')) && (*cp == ' '))
					*cp++ = 0;
				l_zcp(h->id.string, last, 12);
			}
		}
		else
		{
			l_free_one_hostid(newidptr);
			newidptr = 0;
		}
	}
	else
	{
/*
 *	NOTE: l_getid_hw is set up to expect to put the hostid into
 *	job->idptr -- we really want it to go into newidptr, so
 *	we temporarily shuffle these 2
 */

	  HOSTID *sav = job->idptr;


		job->idptr = newidptr;
		x = l_getid_hw(job, idtype);
		job->idptr = sav; /* put it back */
		if (!x)
		{
			l_free_one_hostid(newidptr);
			newidptr = 0;
		}

	}
/*
 *	Add  to end of list
 */
	if (last) last->next = newidptr;
	else job->idptr = newidptr;
	return(newidptr);
}

/*
 * move_to_end_of_idptr_list(job, h, last);
 *
 *	The job list looks like:
 *				job->idptr->next->next->...
 *	type may be			1    1     2    ...
 *	If h is, say, the first one, then I have to move the two type 1's
 *	to the end of the list, so it will look like
 *
 *				2->1->1
 *				   ^
 *				   |- will return this one, so the list is
 *				      null terminated with type 1.
 *
 */
static
void
move_to_end_of_idptr_list(job, idptr, last)
  LM_HANDLE_PTR job;
  HOSTID_PTR idptr;
  HOSTID_PTR last;
{
#if 0
  HOSTID_PTR before_this_type, after_this_type, last_this_type;

	if (l_id_types_match(last->type,idptr->type) return; /* nothing to do */
        last->next = idptr;
        for (before_this_type = job->idptr; before_this_type ; )
        {
                if (l_id_types_match(before_this_type->next->type, idptr->type))
                        break;
                before_this_type = before_this_type->next;
        }
        for (after_this_type = idptr;
                after_this_type &&
		l_id_types_match(after_this_type->type, idptr->type);
                                after_this_type = after_this_type->next)
        {
                last_this_type = after_this_type;
        }
        last_this_type->next = 0;
        if (before_this_type) before_this_type->next = after_this_type;
        else job->idptr = after_this_type;
#endif /* 0 */
  HOSTID_PTR *previousLinkRef;
  HOSTID_PTR *lastLinkRef;
  HOSTID_PTR subList;

/*
 *      Just return if the end is already this type.
 */

        if (l_id_types_match(last->type, idptr->type))
                return;

/*
 *      Starting from the head of the list, walk until we find
 *      the type we need to move.
 */

        previousLinkRef = &(job->idptr);
        while( *previousLinkRef &&
                        !l_id_types_match((*previousLinkRef)->type,idptr->type))
                previousLinkRef = &((*previousLinkRef)->next);

/*
 *      Starting from the node we just found, look for the last node
 *      of the type we need to move (there might be more than
 *      one node of this type).  We can't use "idptr" because
 *      we only know it has the same type as what we found.  We
 *      can't be sure it's the node linked to what we found.
 */

        lastLinkRef = previousLinkRef;
        while( *lastLinkRef &&
                        l_id_types_match((*lastLinkRef)->type, idptr->type ))
                lastLinkRef = &((*lastLinkRef)->next);

/*
 *      Save the start of the portion we are to move, then remove
 *      it from the list.  (Note we terminate the subList as
 *      part of removing it from the main list.)
 */

        subList = *previousLinkRef;
        *previousLinkRef = *lastLinkRef;
        *lastLinkRef = NULL;

/*
 *      Now put our subList at the end of the list.
 */

        last->next = subList;


}

#if defined( SGI) && !defined(SGI4)
/*
** Including calls which normally would be in libc, but need to be
** in this file to avoid being spoofed.
*/


/* Need to define constants if not in the header file because
   API does not actually support this value (pre 6.3) */

#ifndef SGI_NUM_MODULES

#define SGI_NUM_MODULES         191
#define SGI_MODULE_INFO         192

typedef struct module_info_s {
    unsigned long long serial_num;
    int mod_num;
} module_info_t;

#endif


#if 0 /* fails on irix 5.2 cc... */
extern ptrdiff_t flexsyssgi(int request, ...);
#else
extern flexsyssgi();
#endif

int
flex_get_num_modules(void) {
        int rc;

        rc = flexsyssgi(SGI_NUM_MODULES);

	if (rc == -1) {
		/* API does not actually support getting number of modules */
		return 1;
        } else {
		return rc;
	}

}

int
flex_get_module_info(int mod_index, module_info_t *mod_info,size_t info_size) {
        int rc, module_number;

        if ((rc = flexsyssgi(SGI_MODULE_INFO,mod_index,
			     mod_info,info_size)) != -1)
	{
                return rc;
        } else {
/*
 *	API does not support getting module information, use old API
 */
		mod_info->serial_num = sysid(NULL);
		return 0;
	}

}
#endif	/* SGI */

static
HOSTID *
l_getid_hw(job, idtype)
LM_HANDLE *job;		/* Current license job */
int idtype;		/* Type of hostid desired */
{


#ifdef ENCORE
#define GOTONE
	l_zcp(job->idptr->hostid_hostname, lc_hostname(job, 1),
							MAX_SERVER_NAME);/* LONGNAMES */
	job->idptr->type = HOSTID_HOSTNAME;
	return job->idptr;
#endif /* ENCORE */
/*
 *	Apollo - Uses Node ID
 */

#ifdef apollo
#define GOTONE		/* We have it */
  char sid[150];    /* pm_$get_sid_text() returns string of 140 chars max */
  char *hid;	    /* host ID part of return */
  short len, MAXLEN = 140;	/* We would make this #define except for ... */

	if (first)
	{
		first = 0;
		pm_$get_sid_txt(MAXLEN, sid, &len);
		sid[len] = '\0';
		hid = strrchr(sid, '.');
		if (hid == (char *) NULL)
		{
			real_id = 0;
		}
		else
		{
			hid--;
			*hid = '0'; *(hid+1) = 'x';
			real_id = (int) strtol(hid, (char **) NULL, 16);
		}
	}
	job->idptr->type = HOSTID_LONG;
	job->idptr->hostid_value = real_id;
#endif	/* apollo */

/*
 *	CONVEX - use node serial number
 */

#ifdef convex
#define GOTONE	/* We have it */

  struct system_information sysinfo;

	if (first)
	{
		first = 0;
		getsysinfo(SYSINFO_SIZE, &sysinfo);
		real_id = sysinfo.system_sn;
#if 0
/*
 *	Don't want to break people's hostid if the # of CPUs changes ---
 *		or do we ???
 */
		real_id |= (sysinfo.cpu_count << 16);
#endif
		real_id |= (sysinfo.cpu_type << 24);
	}
	job->idptr->type = HOSTID_LONG;
	job->idptr->hostid_value = real_id;
#endif	/* CONVEX */

#ifdef DGX86
/*
 *	DG-Intel boxes
 */

#define GOTONE
#if 0 /* old --- required root perms */
  char 	      dir[MAX_DEVICES][MAXNAMLEN];
  int 	      num_dirs = 0;
  int	      i;


	/* find and open network directory */
	if ((num_dirs = getdir("/dev/net", dir)))
	{
		/* look for network entries */
		for (i = 0; i < num_dirs;i++)
		{
			if (!GetFactoryEthernet(dir[i], job->idptr))
			{
				job->idptr->type = HOSTID_ETHER;
				return job->idptr;
			}
		}
	}
	return 0;
#else
  int filedescr;
  int retcode;
  int i;
  unsigned long key = 0;
  unsigned char * cp;
  struct  dg_inet_info    ioctl_req;
  struct  dg_inet_if_entry stats,  *statp = &stats;

	filedescr = socket( AF_INET, SOCK_DGRAM, 0);
	job->idptr->type = HOSTID_ETHER;
	if (filedescr < 0)
	{
		LM_SET_ERRNO(job, LM_CANTFINDETHER, 508, errno);
		return (job->idptr = 0);
	}


	while (1)
	{
		ioctl_req.version = DG_INET_INFO_VERSION;
		ioctl_req.selector =  DG_INET_IF_ENTRY;
		ioctl_req.key =  key;
		ioctl_req.buff_len =  sizeof(stats);
		ioctl_req.buff_ptr =  (caddr_t) statp;


		retcode = ioctl(filedescr, SIOCGINFO, &ioctl_req);
		if (retcode < 0)
		{
			LM_SET_ERRNO(job, LM_CANTFINDETHER, 509, errno);
			return (job->idptr = 0);
		}

		if ( ioctl_req.buff_len == 0 ) /* end of list */
		{
			LM_SET_ERRNO(job, LM_CANTFINDETHER, 510, errno);
			return (job->idptr = 0);
		}

		key = ioctl_req.key;

		cp = &statp->ifHwAddr[0];
		if (!statp->ifHwAddrLen) continue;
		for(i=0; i<statp->ifHwAddrLen; i++)
		{
			job->idptr->id.e[i] = *cp++;
		}
		return job->idptr;
	}

#endif /* 0 */
#endif /* DGX86 */
/*
 *	88k machines - use sysconf() call
 */


#if (defined(DGUX) || defined(MOTO_88K) || defined(MOTOSVR4)) && !defined(GOTONE)

#define GOTONE

#ifdef DGUX
#ifdef SVR4
#include <sys/_int_unistd.h>
#else
#include <sys/m88kbcs.h>
#endif
#endif	/* DGUX */
#if defined(MOTO_88K) || defined(MOTOSVR4)
#include <unistd.h>
static moto_gethostid();
#endif	/* MOTO_88K */
long sysconf();

	job->idptr->type = HOSTID_LONG;
/*
 *	sysconf call returns unique machine id if available, -1 otherwise
 */
	if ((job->idptr->id.data = sysconf(_SC_BCS_SYS_ID)) == -1)
	{
#ifdef MOTO_88K
/*
 *		some motorola releases (pre-SVR4) don't support unique
 *		system id's -- include special case code to get the
 *		system id on these machines
 *			(motorola's vendor stamp is 0x100)
 */
		if (sysconf(_SC_BCS_VENDOR_STAMP) == (long)0x100)
		{
			moto_gethostid(job);
		}
#endif
	}
	if (job->idptr->id.data == 0 || job->idptr->id.data == -1)
	{
		job->idptr = (HOSTID *)0; /* NOHOSTID */
	}

#endif	/* DGUX || MOTO_88K */

/*
 *	HP - Uses uname or ethernet or ID Module
 */
#ifdef HP
#define GOTONE
	{
	  static hp_gethostid();

		hp_gethostid(job, idtype);
	}

#endif

#ifdef NCR
#define GOTONE
	ncr_gethostid(job);
#endif /* NCR */

#ifdef BSDI
#define GOTONE
	bsdi_gethostid(job);
#endif /* BSDI */



#ifdef GENUINE_MIPS_MACHINE
#define GOTONE
/*
 *	use hwconf(2) on mips to get the host id
 */
#include <machine/hwconf.h>

   struct hw_config conf;
   int i;

	job->idptr->type = HOSTID_LONG;
	i = hwconf(HWCONF_GET, &conf);
	if (i != -1)
	{
		(void) sscanf(conf.cpubd_snum, "%x", &(job->idptr->id.data));	/* overrun checked */
	}
	else
	{
		job->idptr->id.data = 0;
	}
#endif

/*
 *	PC - punt!
 */
#if defined(PC)
#define GOTONE

  HOSTID *l_check_rainbow(LM_HANDLE *, int );
  HOSTID *l_check_dallas(LM_HANDLE *);
  HOSTID *l_check_aladdin(LM_HANDLE*);
  HOSTID *h;
  DWORD dwError;
        if ( (idtype == HOSTID_FLEXID1_KEY) || (idtype == HOSTID_FLEXID5_KEY )
	 	||( idtype == HOSTID_FLEXID6_KEY ))
        {
		if (l_getattr(job, LM_DONGLE1)!=LM_DONGLE1_VAL)
                {
                        LM_SET_ERRNO(job,LM_FUNCNOTAVAIL,301,errno);
                        job->idptr = 0;
                        return job->idptr;
                }
		if (h = l_check_rainbow(job, idtype))
		{
			memcpy(job->idptr, h, sizeof(HOSTID));
			free(h);
		}
		else
		{
                        return (job->idptr = 0);
		}
        }
        else if ( idtype == HOSTID_FLEXID2_KEY )
        {
		 /* Dallas dongle support (FLEXID8) */

		sHandle2 = CreateSemaphore(NULL, 1, 1, "FLEXDS1410XSema");
		dwError = GetLastError();

		if ((sHandle2 == NULL) && (dwError == ERROR_ACCESS_DENIED))
		{	// service is locking the semaphore condition
			// P6204
			sHandle4 = CreateSemaphore(NULL, 1, 1, "FLEXDS1410XSemaServiceCondition");
			dwError = GetLastError();
			if (sHandle4 == NULL)
			{	// really bad error!
				LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 521, errno);
				job->idptr = 0;
				return job->idptr;
			}

			sHandle5 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FLEXDS1410XSemaServiceCondition");
			dwWaitResult = WaitForSingleObject( sHandle5, 6000L);    /* time-out interval */

			switch (dwWaitResult)
			{
			case WAIT_OBJECT_0:
				if (h = l_check_dallas(job))
				{
					memcpy(job->idptr, h, sizeof(HOSTID));
					free(h);
					ReleaseSemaphore( sHandle5, 1, NULL);	/* clean up semaphore */
				}
				else
				{
					ReleaseSemaphore( sHandle5, 1, NULL);	/* clean up semaphore */
					return (job->idptr = 0);
				}
				break;
			/* Semaphore was nonsignaled, so a time-out occurred. */
			case WAIT_TIMEOUT:
				/* clean up semaphores,
				This will deal with applications
				that crash while holding a semaphore token. */
				ReleaseSemaphore( sHandle5, 1, NULL);
				LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 522, errno);
				job->idptr = 0;
				return job->idptr;
				break;
			} // end switch
		}	// end service sema lock situation

		// normal behavior
		sHandle3 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FLEXDS1410XSema");
		dwWaitResult = WaitForSingleObject( sHandle3, 6000L);    /* time-out interval */

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			if (h = l_check_dallas(job))
			{
				memcpy(job->idptr, h, sizeof(HOSTID));
				free(h);
				ReleaseSemaphore( sHandle3, 1, NULL);	/* clean up semaphore */
			}
			else
			{
				ReleaseSemaphore( sHandle3, 1, NULL);	/* clean up semaphore */
				return (job->idptr = 0);
			}
			break;
		/* Semaphore was nonsignaled, so a time-out occurred. */
		case WAIT_TIMEOUT:
			/* clean up semaphores,
			   This will deal with applications
			   that crash while holding a semaphore token. */
			ReleaseSemaphore( sHandle3, 1, NULL);
			LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 522, errno);
			job->idptr = 0;
			return job->idptr;
			break;
		} // end switch
	}
	/* Aladdin dongle support */
        else if ( idtype == HOSTID_FLEXID3_KEY )
        {

                if (l_getattr(job, LM_DONGLE3)!=LM_DONGLE3_VAL)
                {
                        LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 517, errno);
                        job->idptr = 0;
                        return job->idptr;
                }
		if (h = l_check_aladdin(job))
		{
			memcpy(job->idptr, h, sizeof(HOSTID));
			free(h);
		}
		else
		{
                        return (job->idptr = 0);
		}
        }
		/* not used */
        else if ( idtype == HOSTID_FLEXID4_KEY )
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 519, errno);
                job->idptr = (HOSTID *)0;
                return job->idptr;

        }
#endif



#ifdef PC
#if defined (HAVE_CPUID)
#define GOTONE

        else if ( idtype == HOSTID_INTEL96 ||
        idtype == HOSTID_INTEL64 ||
        idtype == HOSTID_INTEL32 )
        {
          HOSTID * (*func) (LM_HANDLE *)
#ifdef FLEX_STATIC
                = l_eintelid;
                if (job->flags & LM_FLAG_LMUTIL)
                        func = l_intelid;

#else
                = l_intelid;
#endif /* FLEX_STATIC */


		if (h = (*func)(job)) /* we have to append to job->idptr */
		{
		  HOSTID *t;
			h->type = idtype;
		        for (t = h; t && t->next; t = t->next)
		                t->type = idtype;
		        for (t = job->idptr; t && t->next; t = t->next)
		                ;
                        t->type = h->type;
                        memcpy(t->id.intel96, h->id.intel96, sizeof(h->id.intel96));
                        t->next = h->next;
                        l_free_one_hostid(h);
                        return job->idptr;
		}
		else
		{
			LM_SET_ERRNO(job,LM_FUNCNOTAVAIL,520,errno);
                	return 0;
		}
	}
#else /* ifdef HAVE_CPUID */

        else if ( idtype == HOSTID_INTEL96 ||
        idtype == HOSTID_INTEL64 ||
        idtype == HOSTID_INTEL32 )
        {
			LM_SET_ERRNO(job,LM_FUNCNOTAVAIL,520,errno);
                	return 0;
        }
#endif /* not defined CPUID */

/* SafeCast HOSTIDs */
#ifdef WINNT
	else if ( idtype == HOSTID_CPU )
	{
                job->idptr->type = HOSTID_CPU;
                job->idptr->id.data = l_cpu_id();
	}
	else if ( idtype == HOSTID_DISK_GEOMETRY )
	{
                job->idptr->type = HOSTID_DISK_GEOMETRY;
                job->idptr->id.data = l_disk_geometry_id();
	}
	else if ( idtype == HOSTID_BIOS )
	{
                job->idptr->type = HOSTID_BIOS;
                job->idptr->id.data = l_bios_id();
	}
#endif
        else if (idtype== HOSTID_DISK_SERIAL_NUM)
        {
                job->idptr->type = HOSTID_DISK_SERIAL_NUM;
                job->idptr->id.data = l_vol_id();
        }
        else /* if (idtype== HOSTID_DISK_SERIAL_NUM) */
	{
		extern BOOL get_pc_ether_addr1( LM_HANDLE_PTR job);

		/*
		 * host idtype is neither disk serial number nor sentinel key.
		 * We should now use Ethernet address via Packet driver or
		 * NDIS if we are operating in Win32s environment.
		 */
		if (!get_pc_ether_addr1(job))
		{
			job->idptr->type = HOSTID_LONG;
			job->idptr->id.data = -1;
		}
	}
#endif /* WINNT */

#if defined (OS2)
#define GOTONE
#define __WATC__
	else if (idtype== HOSTID_DISK_SERIAL_NUM)
	{
		job->idptr->type = HOSTID_DISK_SERIAL_NUM;
		DosQueryFSInfo(
			3    /* Drive C */,
			FSIL_VOLSER,
			&job->idptr->id.data,
			sizeof(job->idptr->id) );
	}
	else
	{
		extern unsigned long os2_gethostid();

	    	job->idptr->type = HOSTID_ETHER;
		job->idptr->id.data = (long) os2_gethostid();
	}

#endif /* OS2 */

#ifdef NLM
#define GOTONE
	else   {
		#include <nwconn.h>
		GetLANAddress( 1, job->idptr->id.e );
                job->idptr->type = HOSTID_ETHER;
	        }

#endif /* NLM */

#if defined (PCRT) | defined(RS6000)
#define GOTONE
	{
	  static  pcrt_gethostid();

		pcrt_gethostid(job);
	}

#endif /* PCRT */

#ifdef UNIXWARE /* ethernet */

/*
 *	This code came from Tim Idey with Novell Unixware hotline
 *	908-522-5033
 */
#define GOTONE
	char devbuf[MAX_LONGNAME_SIZE] = {'\0'};
	char device[MAX_LONGNAME_SIZE] = {'\0'};
	u_char 	etheraddr[256];
	int fd, ret;
	struct strioctl strioc;
	FILE *fp;
	int i;
	char *cp, *devptr;

	job->idptr->type = HOSTID_ETHER;

	for (i=0;i<ETHER_LEN;i++)
		job->idptr->id.e[i] = 0; /* in case of failure */
	if (!(fp  = l_flexFopen(job, "/etc/confnet.d/netdrivers", "r")))
		goto unixware_fail;

	if (!fgets(devbuf, 256, fp)) goto unixware_fail;	/* overrun checked */

/*
 *	get first word
 */
	for (cp = devbuf; *cp && isspace(*cp); cp++) /* skip leading blanks */
		;
	devptr = cp;
	for (;*cp && !isspace(*cp); cp++)
		;
	*cp = '\0';

	if (!*devptr) goto unixware_fail;

	sprintf(device, "/dev/%s", devptr);	/* OVERRUN */

	if ((fd = l_flexOpen(job, device, O_RDWR, 0)) < 0)
		goto unixware_fail;

	strioc.ic_len = ETHER_LEN;
	strioc.ic_timout = 0;
	strioc.ic_dp = (char *)etheraddr;
	strioc.ic_cmd = DLIOCGENADDR;
	if (ioctl(fd, I_STR, &strioc) < 0)
		goto unixware_fail;

	memcpy(job->idptr->id.e, etheraddr, ETHER_LEN);

unixware_fail:
	if (fp) fclose(fp);
	if (fd) close(fd);
#endif /* UNIXWARE */

#ifdef sco
#define GOTONE


/*
 *	This code came from Tim Idey with Novell Unixware hotline
 *	908-522-5033
 */
  struct scoutsname scouts;
  char devbuf[MAX_LONGNAME_SIZE] = {'\0'};
  char device[MAX_LONGNAME_SIZE] = {'\0'};
  u_char 	etheraddr[256];
  int fd, ret;
#ifdef SCO5
  struct strioctl strioc;
  FILE *fp;
  int i;
  char *cp, *devptr;

	if (idtype == HOSTID_ETHER)
	{
		memset(job->idptr->id.e, 0, sizeof(job->idptr->id.e));
#define LM_DEVFILES "/etc/confnet.d/inet/interface"

		if (!(fp  = l_flexFopen(job, LM_DEVFILES, "r")))
			return 0;

		while (fgets(devbuf, 256, fp))	/* overrun checked */
		{
			devptr = devbuf;
			if (!strncmp(devbuf, "net:", 4))
			{
				/* get 4th field, separated by ':' */
				for (i = 0; devptr && (i < 3); i++)
				{
					if (devptr = strchr( devptr, ':'))
						devptr++;
				}
				if (!devptr) continue;

				if (cp = strchr( devptr, ':'))
				{
					*cp = 0; /* null terminate device name */
				}
				else continue;
				if ((fd = l_flexOpen(job, devptr, O_RDWR, 0)) < 0)
					continue;
				strioc.ic_cmd = MACIOC_GETADDR;
				strioc.ic_timout = 15;
				strioc.ic_len = LLC_ADDR_LEN;
				strioc.ic_dp = (char *)job->idptr->id.e;
				if (ioctl(fd, I_STR, &strioc) < 0)
					continue;
				for (i = 0; i < ETHER_LEN; i++)
					if (job->idptr->id.e[i]) break;
				if (i == ETHER_LEN) continue;

				job->idptr->type = HOSTID_ETHER;
				/*memcpy(job->idptr->id.e, etheraddr, ETHER_LEN);*/
				return job->idptr;
			}
		}
		if (fp) fclose(fp);
		if (fd) close(fd);
		return 0;
	} else
#endif /* SCO 5 */
	if (syscall(SCOINFO, &scouts, sizeof(scouts)) == -1)
	{
		job->idptr->type = HOSTID_LONG;
		job->idptr->id.data = 0;
	}
	else if (idtype == HOSTID_LONG) /* old, buggy style */
	{

	  char *s = &scouts.sysserial[0];

		while (!isdigit(*s) && *s != '\0') s++;
		job->idptr->type = HOSTID_LONG;
		(void) sscanf(s, "%x", &(job->idptr->id.data));	/* overrun checked */
	}
	else
	{
	  char *s = &scouts.sysserial[0];
		job->idptr->type = HOSTID_STRING;
		strcpy(job->idptr->id.string, s);	/* OVERRUN */
	}


#endif	/* sco */

/*
 *	SUN ...
 */
#ifdef sun

	  long lu_gethostid();

#define GOTONE
	{

		if (idtype != HOSTID_LONG) return 0;

		job->idptr->id.data = gethostid();
		job->idptr->type = HOSTID_LONG;
	}
#endif

#ifdef FREEBSD
#define GOTONE
   int sock = -1;
   char inbuf[8192];
   const struct sockaddr_dl *sdl = NULL;
   struct ifconf ifc;
   struct ifreq *ifr;
   int i, n, j;

   if (idtype != HOSTID_ETHER) return 0;
   ifc.ifc_len = sizeof(inbuf);
   ifc.ifc_buf = inbuf;

   sock = socket (AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) return 0;
   if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) return 0;
   ifr = ifc.ifc_req;
   for (i = 0; i < ifc.ifc_len; ) {
      ifr = (struct ifreq *)((caddr_t)ifc.ifc_req + i);
      i += sizeof(ifr->ifr_name) +
           (ifr->ifr_addr.sa_len > sizeof(struct sockaddr)
            ? ifr->ifr_addr.sa_len
            : sizeof(struct sockaddr));
      if (ifr->ifr_addr.sa_family == AF_LINK) {
         sdl = (const struct sockaddr_dl *) &ifr->ifr_addr;
      }

      if (sdl->sdl_type == IFT_ETHER) {
         char *mac_addr;
	 job->idptr->type = HOSTID_ETHER;
         mac_addr = (char *)LLADDR(sdl);
         n = sdl->sdl_alen;
	 j = 0;
         while (--n >= 0) {
		job->idptr->id.e[j++] = *mac_addr++;
         }
         /*
           This is the end !
         */
#ifdef OPENBSD
	/* Keep the first instead of the last one!
	 * so this really is the end! JONL
	 */
	i = ifc.ifc_len;
#endif
      }
   }

   if (sock != -1) close (sock);
#endif /* FREEBSD */

#ifdef cray
#define GOTONE
#ifdef CRAY_NV1
	job->idptr->id.data = gethostid();
#else  /* CRAY_NV1 */
	job->idptr->id.data = sysconf(_SC_CRAY_SERIAL);
#endif /* CRAY_NV1 */
	job->idptr->type = HOSTID_LONG;
#endif /* cray */

#ifdef HOSTID_SYSINFO_STRING
#define GOTONE
#include <limits.h>
#include <sys/systeminfo.h>
  char buf[300];

	if (idtype != HOSTID_STRING) return 0;
	if (sysinfo(SI_HW_SERIAL, buf,300) > 0)
	{
		job->idptr->type = HOSTID_STRING;
		strcpy(job->idptr->id.string, buf);	/* OVERRUN */
	}
#endif /* sinix */


/*
 *	NEC EWS
 */
#ifdef NEC
#define GOTONE

#ifdef NECSX4
  unsigned long long l;

        if (syssx(RDSER, &l))
        {
                job->idptr = (HOSTID *)0;
                job->u_errno = errno;
        }
        else
        {
#if 0
                if (l < (unsigned long long) 0xffffffff)
                {
                        job->idptr->type = HOSTID_LONG;
                        job->idptr->id.data = l;
                }
                else
#endif
                {
                        job->idptr->type = HOSTID_STRING;
                        sprintf(job->idptr->id.string, "%lx", l);
                }
        }
#else /* NECSX4 */

  char buf[300];

	job->idptr->type = HOSTID_LONG;
	if (sysinfo(SI_HW_SERIAL, buf, 300) > 0)
	{
		sscanf(buf, "%x", &job->idptr->id.data);	/* overrun checked */
	}
	else
	{
		job->idptr = (HOSTID *)0;
		job->u_errno = errno;
	}
#endif /* NEC */
#endif /* NECSX4 */

#ifdef LYNX
#define GOTONE
	{
	  int tp_off, i;
	  struct tcpip_info tp;
	  struct ifnet ifn, *ifnet;
	  struct arpcom ac;
	  char ifname[IFNAMSIZ] ;
	  int fd;
	  int gotone = 0;


		if ((tp_off = info(_I_TCPIP_INFO))  &&
			((fd = l_flexOpen(job, "/dev/mem", O_RDONLY, 0)) >= 0)  &&
			read_from_kernel(fd, tp_off, &tp, sizeof(tp)) &&
			read_from_kernel(fd, tp.ifnet, &ifnet, sizeof(ifnet)))
		{
			while (ifnet)
			{

				if (!read_from_kernel(fd, ifnet, &ifn,
								sizeof(ifn)))
					continue;
				if (ifn.if_mtu == ETHERMTU)
				{ /* Ethernet Interface */

					if (!read_from_kernel(fd, ifn.if_name,
						ifname, IFNAMSIZ) ||
					 !read_from_kernel(fd, ifn.p, &ac,
								sizeof(ac)))
						continue;
					for (i=0; i < 6; i++)
					{
						job->idptr->id.e[i] =
							ac.ac_enaddr[i];
						if (job->idptr->id.e[i])
							gotone = 1;
					}
					break;
				}
				ifnet = ifn.if_link.tqe_next;
			}
		}
		if (!gotone)
		{
			job->u_errno = errno;
			job->idptr = (HOSTID *)0;
		}
		close(fd);
	}
#endif /* LYNX */



#ifdef LINUX
{

struct interface {
  char                  name[16];         /* interface name        */
  short                 type;                   /* if type               */
  char                  hwaddr[32];             /* HW address            */
};

	int					fd = -1;
	int					i = 0;
	int					done = 0;
	struct ifreq		ifr;
	struct interface	ife;
	FILE *				fp = NULL;

#define LINUX_DEV0				"xp0"
#define LINUX_DEV1				"eth0"
#define LINUX_LICENSE_ID_FILE	"/proc/sgi_sn/licenseID"

#ifdef MONTAVISTA
	if (idtype == HOSTID_LONG)
	{
		job->idptr->id.data = gethostid();
		job->idptr->type = HOSTID_LONG;
	}
	else
#endif
	{
		/*
		 *	Try license ID file, really only applies to
		 *	Linux Partition server
		 */
		fp = l_flexFopen(job, LINUX_LICENSE_ID_FILE, "r");
		if(fp)
		{
			i = fscanf(fp, "%x", &job->idptr->id.data);	/* overrun checked */
			(void)fclose(fp);
			fp = NULL;
			if(i == 1 && job->idptr->id.data)
			{
				done = 1;
				job->idptr->type = HOSTID_LONG;
			}
		}
		/*
		 *	Next try getting xp0, if not there, then try ethernet address
		 */
		if(!done)
		{
			job->idptr->type = HOSTID_ETHER;
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			if(fd != -1)
			{
				/*
				 *	Now try reading the HW MAC address for partition
				 */
				memset((void *)&ife, 0, sizeof(struct interface));
				memset((void *)&ifr, 0, sizeof(struct ifreq));
				strcpy(ife.name, LINUX_DEV0);
				strcpy(ifr.ifr_name, LINUX_DEV0);

				if(ioctl(fd, SIOCGIFHWADDR, &ifr) >= 0)
				{
					memcpy(ife.hwaddr,ifr.ifr_hwaddr.sa_data,8);
					ife.type = ifr.ifr_hwaddr.sa_family;
					for (i = 0; i < 6; i++)
						job->idptr->id.e[i] = ife.hwaddr[i];
					done = 1;
				}
				close(fd);
				fd = -1;
			}
		}

		if(!done)
		{
			/*
			 *	Get ethernet mac address
			 */
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			memset((char *) &ife, 0, sizeof(struct interface));
			memset((void *)&ifr, 0, sizeof(struct ifreq));
			strcpy(ife.name, LINUX_DEV1);
			strcpy(ifr.ifr_name, LINUX_DEV1);

			if(fd < 0 || ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
			{
				job->idptr = (HOSTID *)0;
				job->u_errno = errno;
			}
			else
			{
				memcpy(ife.hwaddr,ifr.ifr_hwaddr.sa_data,8);
				ife.type = ifr.ifr_hwaddr.sa_family;
				for (i=0; i < 6; i++)
					job->idptr->id.e[i] = ife.hwaddr[i];
			}
			if(fd > 0)
				close(fd);
		}
	}
}
#endif /* LINUX */

#ifdef MAC10
	if (idtype == HOSTID_ETHER)
	{
		mac_ether(job);
	}
#define GOTONE

#endif /* MAC10 */
/*
 *	sgi - has settable hostid - use sysid() call.
 */

#ifdef SGI
#define GOTONE
#ifdef SGI4
	job->idptr->id.data = sysid(0);
	job->idptr->type = HOSTID_LONG;
#else

    /* new way of getting sysinfo number - support multiple modules */

    int i, num_mods;
    module_info_t mod_info;
    HOSTID *t;

    /* new way of getting sysinfo number - support multiple modules */


    num_mods = flex_get_num_modules();

    for (i = 0; i < num_mods; i++) {
	if (flex_get_module_info(i, &mod_info, sizeof(module_info_t)) != -1) {

	    if (i == 0) {
		job->idptr->type = HOSTID_LONG;
		job->idptr->id.data = (unsigned long) mod_info.serial_num;
	    } else {
		HOSTID *newID;
		if ((newID = l_new_hostid()) != 0) {

		    newID->type = HOSTID_LONG;
		    newID->id.data = (unsigned long) mod_info.serial_num;

		    /* need to always append a new id */

		    t = job->idptr;
		    while (t->next != NULL) {
			t = t->next;
		    }

		    t->next = newID;

		}
	    }
	}
    }

#endif	/* SGI4 */
#endif	/* SGI */

/*
 *	Vax/Ultrix, MIPS/Ultrix - Uses ethernet address
 */
#ifdef USE_ETHERNET_ADDR
#define GOTONE

  unsigned char *eid, *l_ether_id();
  int i;
  static unsigned char def[6] = { 0, 0, 0, 0, 0, 0 };

#ifndef VMS
	if (idtype != HOSTID_ETHER) return 0;
#endif /* VMS */

	eid = l_ether_id(job);
	if (eid == 0) eid = def;
	for (i = 0; i < 6; i++)
		job->idptr->id.e[i] = eid[i];
	job->idptr->type = HOSTID_ETHER;
#ifdef VMS
	if (idtype == HOSTID_LONG ||
	    (eid[0] == 0 &&
	     eid[1] == 0 &&
	     eid[2] == 0 &&
	     eid[3] == 0 &&
	     eid[4] == 0 &&
	     eid[5] == 0))
	{
	  long flm_vms_gethostid();

		job->idptr->id.data = flm_vms_gethostid();
		job->idptr->type = HOSTID_LONG;
		LM_SET_ERRNO(job, 0, 174, 0);
	}
#endif /* VMS */
#endif /* USE_ETHERNET_ADDR */

#ifndef GOTONE
  /*static*/ long gethostid();

	job->idptr->type = HOSTID_LONG;
	job->idptr->id.data = gethostid();

#endif
/*
 *	P2784 -- strip spaces  -- because of a ridiculous Siemens bug
 */
	if (job->idptr && (job->idptr->type == HOSTID_STRING))
	{
	  char *cp, *cp2;
		for (cp = job->idptr->id.string; *cp; )
		{
			if (*cp == ' ')
			{
				for (cp2 = cp; *cp2; cp2++)
					cp2[0] = cp2[1];
			}
			else cp++;
		}
	}
	return(job->idptr);
}


HOSTID_PTR
API_ENTRY
l_new_hostid()
{
  HOSTID_PTR p;

	p = (HOSTID_PTR)calloc(sizeof(HOSTID), 1);

	return p;
}

void
l_free_one_hostid(h)
HOSTID_PTR h;
{
	if (h->vendor_id_prefix) free (h->vendor_id_prefix);
	h->vendor_id_prefix = 0;
	free(h);
}

#ifndef FLEXLM_ULTRALITE	/* ULTRALITE is compiled on HP */
#ifdef HP
#include <stdio.h>
#include <sys/hilioctl.h>
#include <sys/fcntl.h>
#include <netio.h>
#include <sys/utsname.h>
static hp_etherid lm_args((LM_HANDLE *));

#define LAN_DEV "/dev/lan"

#define ID_INDEX_MAX	16
/*
 *	this gethostid() function returns the unique
 *	id number of the machine in use. In the HP's case, this number
 *	is the serial number on the HP HIL ID Module.
 *
 */

static
hp_gethostid(job, idtype)
LM_HANDLE *job;		/* Current license job */
int idtype;
{
#ifdef HP_LONG_HOSTID_STRING
	if (idtype == HOSTID_STRING)
	{
		int	len = 0;

		memset(job->idptr->id.string, 0,
			sizeof(job->idptr->id.string));
		job->idptr->type = HOSTID_STRING;
		len = confstr(_CS_PARTITION_IDENT,
				job->idptr->id.string,
				sizeof(job->idptr->id.string));
		if(len > sizeof(job->idptr->id.string))
		{
			/*	set error code here	*/
		}
	}
	else if (idtype == HOSTID_LONG)
#else /* !HP_LONG_HOSTID_STRING */
	if (idtype == HOSTID_LONG)
#endif
	{
	  struct utsname x;

/*
 *		Assume we won't get it
 */
		job->idptr->id.data = 0;
		job->idptr->type = HOSTID_LONG;
		uname(&x);
		if (strlen(x.idnumber) == (ETHER_LEN * 2))
		{
			job->idptr->type = HOSTID_ETHER;
			l_ether_str_to_num(x.idnumber, job->idptr->id.e);
		}
		else
		{
			sscanf(x.idnumber, "%ld", &(job->idptr->id.data));	/* overrun checked */
		}
	}
	else if (idtype == HOSTID_ETHER)
	{
		hp_etherid(job);
	}
	else /* HOSTID_ID_MODULE */
	{
/*
 *	Note that HOSTID_ID_MODULE is only requested by lmhostid or
 *	l_host().  HOSTID_LONG is returned.
 */
	  int digit[ID_INDEX_MAX];
	  static read_id_module();

		job->idptr->type = HOSTID_ID_MODULE;
/*
 *		Assume we won't get it
 */
		job->idptr->id.data = 0;
		if (read_id_module(job, digit) == 0)
		{
		  union { long l; char c[4]; } id_switch;
/*
 *			host id is stored in bytes 4-7 of the returned string.
 */
			id_switch.c[0] = digit[7];
			id_switch.c[1] = digit[6];
			id_switch.c[2] = digit[5];
			id_switch.c[3] = digit[4];
			job->idptr->id.data =  id_switch.l;
		}
	}
}
static
hp_etherid(job)
LM_HANDLE *job;		/* Current license job */
{
  int i;
  char device_name[15];
  static is_hp_etherid();
	if (is_hp_etherid(job, LAN_DEV, PERMANENT_ADDRESS))
		return;
	if (is_hp_etherid(job, LAN_DEV, LOCAL_ADDRESS))
		return;
	for (i = 0; i < 15; i++)
	{
		(void) sprintf(device_name, "%s%d", LAN_DEV, i);
		if (is_hp_etherid(job, device_name, PERMANENT_ADDRESS))
			return;
		if (is_hp_etherid(job, device_name, LOCAL_ADDRESS))
			return;
	}
	job->idptr = (HOSTID *)0;
	LM_SET_ERRNO(job, LM_CANTFINDETHER, 175, errno);
}

static
is_hp_etherid(
	LM_HANDLE *	job,			/* current license job */
	char *		device_name,
	int			reqtype)
{
  struct fis arg;
  int i, fd, gotaddr = 0;

	if ((fd = l_flexOpen(job, device_name, O_RDONLY, 0)) != -1)
	{
		memset(&arg, 0, sizeof(arg));
		arg.reqtype = reqtype;
		arg.vtype = INTEGERTYPE;
		if (ioctl(fd, NETSTAT, &arg) != -1)
		{
			job->idptr->type = HOSTID_ETHER;
			for (i=0; i<6; i++)
			{
				if (job->idptr->id.e[i] = arg.value.s[i])
					gotaddr = 1; /* something is nonzero */
			}
		}
		close(fd);
	}
	return gotaddr;
}

/*
 * read_id_module:
 *
 *	Finds an HP HIL ID module on the machine's HIL, then
 *	reads the magic id number from the module. Finally,
 *	fills in the array pointed at by intbuf_p with the
 *	id number.
 */

static
read_id_module(job, intbuf_p)
LM_HANDLE *job;		/* Current license job */
int *intbuf_p;	/* pointer to array of ID_INDEX_MAX integers */
{
#define DEV_MAX 10
#define HIL_MAX 10
#define MAXSLEEP 2	/* Maximum number of times we will sleep 1 sec. */

 int i;
 unsigned char *buf_p;
 unsigned char buf[ID_INDEX_MAX];
 int maxsleep = MAXSLEEP;
 int done = 0, tryagain;

  static char *dev_path_s = "/dev/hil";

/*	The ID Module may appear at any position in the HIL.
 *	Each position in the HIL has a device node of the
 *	form "/dev/hil[0-9]". So, we check each such node,
 *	starting with /dev/hil0, until we either find the
 *	ID module or run through all the nodes.
 */
	while (maxsleep-- > 0)
	{
	    tryagain = 0;
	    for (i = 0 ; i < HIL_MAX ; i++)
	    {
/*
 *		find the id module and get the id
 */
	      int dev_fd;
	      int j;
	      char dev_s[DEV_MAX];

		(void) sprintf(dev_s, "%s%d", dev_path_s, i);	/* OVERRUN */
		dev_fd = l_flexOpen(job, dev_s, 0, 0) ;
		if (dev_fd < 0)
		{
			if (errno == ENXIO || errno == EBUSY) tryagain = 1;
			continue;	/* no HIL for this position */
		}

		for (j = 0, buf_p = buf; j < ID_INDEX_MAX; j++, buf_p++)
	    		*buf_p = (unsigned char) 0;

		(void) ioctl(dev_fd, HILSC, buf);

		(void) close(dev_fd);

/*
 *		If all bytes beyond the first are 0, this isn't an ID module!
 */
		for (j = 1, buf_p = &buf[1]; j < ID_INDEX_MAX; j++, buf_p++)
		{
	    		if ( *buf_p != (unsigned char) 0 )
			{
				done = 1;
				break;
			}
		}

		if (j < ID_INDEX_MAX)		/* found the id module */
		{
			done = 1;
			break;
		}
	    }
	    if (done || !tryagain || maxsleep <= 0) break;
	    (void) lc_sleep(job, 1);
    	}

	if (i == HIL_MAX) return(1);	/* didn't find the id module */

/*
 *	copy from our local array into the target array
 */
	for (i = 0, buf_p = buf; i < ID_INDEX_MAX; i++, buf_p++, intbuf_p++)
		*intbuf_p = (int) *buf_p;

	return(0);
}
#endif	/* HP */
#endif	/* !FLEXLM_ULTRALITE */

/*
 *	PC/RT - Uses uname system call
 */

#if defined( PCRT) | defined(RS6000)
#include <sys/utsname.h>

static
pcrt_gethostid(job)
LM_HANDLE *job;		/* Current license job */
{
  extern int unamex();
  struct xutsname xutsname;
  int temp;

	job->idptr->type = HOSTID_LONG;
	temp = unamex(&xutsname);
	if (temp < 0)
	{
		job->idptr->id.data = 0L;
	}
	else
	{
		job->idptr->id.data = (long) xutsname.nid;
	}
}

#endif /* PCRT */

#ifdef MOTO_88K

/*
 *	Motorola gethostid() - Get the ethernet address for the machine
 *
 *		Get the address from the ARP tables.
 *		Note: most of this was lifted from arp.c.
 *
 */
#define IFNAMSIZ	16

#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/socket.h>
#include <netinet/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/arp.h>
#include <netdb.h>

#define MAX_IFACES      32              /* max number network interfaces */
#define IP_CTRL         "/dev/inet/ip/control"

static
unsigned long motorola_hostid = 0;

static
moto_gethostid(job)
LM_HANDLE *job;		/* Current license job */
{
  int ip_ctrl;	/* IP control file descriptor	*/
  int ret;
  static struct ifioctl ifaces[MAX_IFACES];
  register struct ifioctl *ifi;
  struct strioctl c;

  union hue {
                struct mungus {
                        struct ifioctl ifi;
                        struct ifindir indir;
                } mungus;
  	        struct arptab arptab;
            } hue;
  struct hostent *hp, *gethostbyaddr();
  struct utsname uts;

	job->idptr->type = HOSTID_LONG;
	job->idptr->id.data = (long) 0;

	if (motorola_hostid == 0)	/* Don't have it yet */
	{
		ret = uname(&uts);	/* Get our system name for use later */
		if (ret < 0) return(0);

			ip_ctrl = l_flexOpen(job, IP_CTRL, 0, 0);
        	if (ip_ctrl < 0) return(0);

        	c.ic_cmd    = IIOCIFLIST;
        	c.ic_timout = 0;        /* default timeout: 15 sec      */
        	c.ic_dp     = (char *) ifaces;
        	c.ic_len    = sizeof (ifaces);

        	ret = ioctl(ip_ctrl, I_STR, &c);
        	if (ret < 0)
		{
			close(ip_ctrl);
			return(0);
		}

        	bzero(c.ic_len + (char *) ifaces, sizeof (ifaces) - c.ic_len);

		ifi = ifaces;

        	do {
                	if ((ifi->flags & IF_ARP) == 0)
                        	continue;

        		c.ic_cmd    = IIOCIOCTL;   /* indirect ioctl       */
        		c.ic_timout = 0;           /* default timeout: 15 sec */
        		c.ic_dp     = (char *) &hue;
        		c.ic_len    = sizeof (hue);

        		hue.mungus.ifi         = *ifi;
        		hue.mungus.indir.cmd   = IFI_ARP_DUMP;
        		hue.mungus.indir.bytes = 0;

        		ret = ioctl(ip_ctrl, I_STR, &c);
        		if (ret < 0)  {
				close(ip_ctrl);
                		return(0);
			}

        		hp = gethostbyaddr(&hue.arptab.in_addr
                           		, sizeof (hue.arptab.in_addr)
                           		, AF_INET);

			if (strcmp(uts.sysname, hp->h_name) != 0)
				continue;

			close(ip_ctrl);
			motorola_hostid = ((hue.arptab.et_addr.octet[2] << 24) |
					(hue.arptab.et_addr.octet[3] << 16) |
					(hue.arptab.et_addr.octet[4] <<  8) |
					(hue.arptab.et_addr.octet[5]));
			break;			/* Got it */

        	} while ((++ifi)->name[0]);

		close(ip_ctrl);
	}
	job->idptr->id.data = (long) motorola_hostid;
	return(0);
}
#endif /* MOTO_88K */



#ifdef NCR

/*	etheradd.c - jye
*	08/23/93
*	Copyright (c) 1993 NCR Corporation
*
*	check all lan cards and print out the bia.
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/inline.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>

#include <sys/xdlpi.h>
#include <sys/ild.h>

boarddata_t brd[20];

int fd;
char *dev = "/dev/lan";

ncr_gethostid(job)
LM_HANDLE *job;		/* Current license job */
{
	struct strioctl ic;
	int slot;
 	int ppa = 0;
	int sav_errno = job->lm_errno;
	int sav_minor = job->errno_minor;
	int sav_u_errno = job->u_errno;

 	memset(brd,0,sizeof(brd));

	if ((fd = l_flexOpen(job, dev, O_RDONLY, 0)) < 0) 		/* changed from RDWR  */
	{
		job->idptr = (HOSTID *)0;
		LM_SET_ERRNO(job, LM_CANTFINDETHER, 256, 0);
		return;
 	}

 	ic.ic_cmd = ILD_RCONFIG;
 	ic.ic_timout = 10;
 	ic.ic_len = sizeof(brd);
 	ic.ic_dp = (char *)brd;


 	if (ioctl(fd,I_STR,&ic) < 0)
	{
		job->idptr = (HOSTID *)0;
		LM_SET_ERRNO(job, LM_CANTFINDETHER, 257, 0);
		return;
 	}

 	close(fd);

 	for (slot = 0; slot < 20;slot++)
	{
 		int indx;

 		ushort brdid=brd[slot].pos[1]<<8|brd[slot].pos[0];
		if (brdid != 0)
		{
			if (sav_errno)
			{
				LM_SET_ERRNO(job, sav_errno, sav_minor, sav_u_errno);
			}
			else
			{
				LM_SET_ERRNO(job, 0, 258, 0);
			}
			job->idptr->type = HOSTID_ETHER;
			for(indx=0;indx<ETHER_LEN;indx++)
				job->idptr->id.e[indx] = brd[slot].bia[indx];
		}

 	}
}

#endif /* NCR */

#if defined (BSDI)
static
void
bsdi_gethostid(job)
LM_HANDLE *job;
{
  struct ifaddrs *ifaddrs;
  struct ifaddrs *ifap;
  struct ether_multi *enm;
  unsigned char *s;
  int nip;
  int i;

      getifaddrs(&ifaddrs,0);
      ifap = ifaddrs;
      i = 0;
      while (ifaddrs->ifa_name)
      {
              ifap = ifaddrs++;
              if (ifap->ifa_addr->sa_family == AF_LINK)
              {
                      switch ((*(struct sockaddr_dl *)
                              ifap->ifa_addr).sdl_type)
                      {
                      case IFT_ETHER:
                      case IFT_FDDI:
                      case IFT_ISO88025:
                      {
                        struct sockaddr_dl *sdl =
                              (struct sockaddr_dl *) ifap->ifa_addr;

                              memcpy(&job->idptr->id.e, LLADDR(sdl),
                                      sizeof( job->idptr->id.e));
                              job->idptr->type = HOSTID_ETHER;
                      }
                              return;

                      }
              }
      }
      job->idptr = (HOSTID *)0;
      LM_SET_ERRNO(job, LM_CANTFINDETHER, 257, 0);
}

#endif /* BSDI */


#ifndef FLEXLM_ULTRALITE

void
l_ether_str_to_num(str, e)
char *str;
unsigned char *e;
{
   int indx, cnt = 0;
   unsigned int i;
   char buf[3];
	for(indx=0;indx<6;indx++)
	{
		buf[0] = str[cnt++];
		buf[1] = str[cnt++];
		buf[2] = '\0';
		sscanf(buf, "%x", &i);	/* overrun checked */
		e[indx] = i;
	}
}

#endif	/* !FLEXLM_ULTRALITE */

#if 0 /*old DGX86*/
static
int
getdir(net_dir, arr)
char *net_dir;
char arr[][MAXNAMLEN];
{

  struct dirent   *dirent = NULL;
  char **cpp;
  DIR         *dir = NULL ;
  int i;

	dir = opendir(net_dir) ;
	if (dir == NULL)
	{
		net_dir = "/dev" ;  /* pre R4.11 */
		dir = opendir(net_dir) ;
	}

	if (dir == NULL) return 0  ;
	for (i = 0; i < MAX_DEVICES  && (dirent = readdir(dir)); )	/* overrun checked */
	{

		if (dirent->d_name[2] == 'e' && dirent->d_name[3] == 'n' &&
						isdigit( dirent->d_name[4] ))
		{
			/* get the factory ethernet address */
			sprintf(arr[i++], "%s/%s",net_dir, dirent->d_name);
		}
	}
	qsort((char *)arr, i, sizeof(*arr), compar);
	closedir(dir);
	return i;
}
/**** GetFactoryEthernet ******************************************************

.PUBLIC:       GetFactoryEthernet

.SYNOPSIS:     Returns the factory ethernet address for the requested ethernet
               device.

.PROTO:        int GetFactoryEthernet(char *device, char **board_addrres) ;

.PARAMS:       device:         device name of board to get information from
                               example: "/dev/net/enet0"

               board_address:  pointer that will be set to malloc'd area
                               containing the factory address for the
                               requested ethernet board. This should be
                               free'd by the calling routine.

.RETURNS:      -1 error
                0 no error
.END

******************************************************************************/

static
int
GetFactoryEthernet(
	char *		device,
	HOSTID *	hostid)
{
  char            *board_address ;
  int             ctr1 ;
  char            buff[BUFF_SIZE] ;
  int             flag;
  int             dev;
  struct strbuf   buff_ptr;
  dl_phys_addr_req_t  *req ;
  dl_phys_addr_ack_t  *rsp ;

	/*   Open the device  */
	dev = l_flexOpen(NULL, device, O_RDWR, 0) ;

	if (dev < 0)
		return(-1) ; /* may just be no more devices */

	/* setup buff and ptrs */
	buff_ptr.len = sizeof (dl_phys_addr_req_t);
	buff_ptr.buf = buff;
	buff_ptr.maxlen = BUFF_SIZE ;

	/*  Request factory hardware ethernet address  */
	req  = (dl_phys_addr_req_t *)buff;
	req->dl_primitive = DL_PHYS_ADDR_REQ ;
	req->dl_addr_type = DL_FACT_PHYS_ADDR;
	if (putmsg(dev, &buff_ptr, NULL, 0) == -1)
	{
		close(dev) ;
		return(-1);
	}

	/* get the factory hardware ethernet address */
	flag = 0 ;
	getmsg (dev, &buff_ptr, 0, &flag);
	close(dev) ;
	rsp = (dl_phys_addr_ack_t *)buff;
	if (rsp->dl_primitive == DL_ERROR_ACK)
	{
		return(-1) ;
	}
	for ( ctr1 = 0; ctr1 < rsp->dl_addr_length; ctr1++ )
		hostid->id.e[ctr1] = 0xFF & buff[ctr1+rsp->dl_addr_offset] ;
	return 0;
}
#endif /* DGX86 */

#ifdef HOSTID_DOMAIN
static
HOSTID *
l_getdomain(job)
LM_HANDLE *job;
{
#ifdef UNIX

  char buf[MAX_DOMAIN_NAME + 1];
  HOSTID *ret = 0, *next, *newid, *sav = 0, *tmp;
  FILE *fp;
  char s[200];
  char *cp;
  int i;

	if (!(ret = next = l_new_hostid())) return 0;
/*
 *	1) call getdomainname()
 */
#ifdef HAVE_GETDOMAINNAME
#ifdef SUNOS5
	if (sysinfo(SI_SRPC_DOMAIN, buf, MAX_DOMAIN_NAME) >0)
#else
	if (!getdomainname(buf, MAX_DOMAIN_NAME) )
#endif
	{
		next->type = HOSTID_DOMAIN;
		l_zcp(next->hostid_domain, buf, MAX_DOMAIN_NAME);
		if (!(newid = l_new_hostid())) return ret;
		next->next = newid;
		next = newid;
	}
#endif /* HAVE_GETDOMAINNAME */
/*
 *	2) read the /etc/resolv.conf file and use the domain there.
 */
	if (fp = l_flexFopen(job, "/etc/resolv.conf", "r"))
	{
		while (fgets(s, 199, fp))	/* overrun checked */
		{
		  char *dp = next->hostid_domain;
			if (L_STREQ_N(s, "domain", 6))
			{
				next->type = HOSTID_DOMAIN;
				/* skip "domain" */
				for (cp = s; *cp && !isspace(*cp); cp++)  ;

				/* skip spaces */
				for (; *cp && isspace(*cp); cp++)  ;

				for (i = 0;
				  *cp && !isspace(*cp) && (i < MAX_DOMAIN_NAME);
								cp++, dp++, i++)
				{
					*dp = *cp;
				}
/*
 *				make sure we don't have it already
 */
				for (tmp = ret; tmp; tmp = tmp->next)
				{
					if ((tmp != next) &&
						L_STREQ(tmp->hostid_domain,
						next->hostid_domain))
					{
						next->type = 0;
						break;
					}
				}
				break;
			}
		}
		if (next->type) /* got one */
		{
			if (!(newid = l_new_hostid())) return ret;
			next->next = newid;
			next = newid;
		}
	}
/*
 *	3) call gethostname(). Use everything after the first dot (.)
 */
	if (!gethostname(s, 199))
	{
		for (cp = s; *cp && *cp != '.'; cp++) ;
		cp++;
		if (*cp)
		{
/*
 *			make sure we don't have it already
 */
			next->type = HOSTID_DOMAIN;
			for (tmp = ret; tmp; tmp = tmp->next)
			{
				if ((tmp != next) &&
					L_STREQ(tmp->hostid_domain,
					next->hostid_domain))
				{
					next->type = 0;
					break;
				}
			}
			if (next->type)
			{
				l_zcp(next->hostid_domain, cp, MAX_DOMAIN_NAME);
				if (!(newid = l_new_hostid())) return ret;
				next->next = newid;
				next = newid;
			}
		}
	}
	/* free the last in the list */
	for (next = ret; next && next->next; sav = next, next = next->next)
		;
	lc_free_hostid(job, next);
	if (sav) sav->next = 0;
	return ret;
#else
	return 0;
#endif /* Unix */

}
#endif /* HOSTID_DOMAIN */



#if defined(PC) && !defined(WINNT) && !defined(NLM) && !defined(OS2)
#pragma optimize ( "", off )

/*
 *	RealInt
 *
 *	Simulate an interrupt in real mode using DPMI function 0300h
 *	When the interrupt is simulated in real mode, the registers will
 *	contain the values in lpRealModeReg. When the interrupt returns,
 *	lpRealModeReg will contain the values from real mode.
 */
BOOL RealInt (BYTE intnum, LPREALMODEREG lpRealModeReg)
{
   BOOL bRetVal = TRUE;
   _asm
   {
       mov  ax, 0300h	/* Simulate real mode interrupt		*/
       mov  bl, intnum	/* Interrupt number to simulate		*/
       mov  bh, 0	/* Flags				*/
       mov  cx, 0	/* Number of words to copy on stack	*/
       les  di, lpRealModeReg
       int  31h
       jnc  Done
       mov  bRetVal, FALSE
   Done:
   }
   return bRetVal;
}

#pragma optimize ( "", on )

#endif

#ifdef MAC10
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices)
{
  kern_return_t kernResult;
  mach_port_t masterPort;
  CFMutableDictionaryRef classesToMatch;

	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
	classesToMatch = IOServiceMatching(kIOEthernetInterfaceClass);
	kernResult = IOServiceGetMatchingServices(masterPort, classesToMatch,
							matchingServices);
	return kernResult;
}
static
kern_return_t
GetEthernetData(io_iterator_t intfIterator, EnetData *edata,
			UInt32 *gNumEntries)
{
  io_object_t intfService;
  io_object_t controllerService;
  io_object_t parentService;
  kern_return_t kernResult = KERN_FAILURE;
  CFMutableDictionaryRef dict;
  CFNumberRef numRef;
  CFStringRef stringRef;
  UInt32 NumEntries;
  CFTypeRef builtinAsCFData;
	for (NumEntries = *gNumEntries;
		intfService = IOIteratorNext(intfIterator);
			NumEntries++)
	{
	  CFTypeRef MACAddressAsCFData;
		bzero(edata[NumEntries].MACAddress, kIOEthernetAddressSize);
		if (IORegistryEntryGetParentEntry( intfService,
			kIOServicePlane, &controllerService ) != KERN_SUCCESS)
			continue;
		MACAddressAsCFData = IORegistryEntryCreateCFProperty(
			controllerService, CFSTR(kIOMACAddress),
					kCFAllocatorDefault, 0);
		if (MACAddressAsCFData)
		{
			/*CFShow(MACAddressAsCFData);*/
			CFDataGetBytes(MACAddressAsCFData,
				CFRangeMake(0,
				kIOEthernetAddressSize),
			edata[NumEntries].MACAddress);
			CFRelease(MACAddressAsCFData);
		}
		kernResult = IORegistryEntryCreateCFProperties(
				controllerService, &dict,
				kCFAllocatorDefault, 0);
		if (kernResult == KERN_SUCCESS)
		{
			edata[NumEntries].linkSpeed = 0;
			numRef = (CFNumberRef)
				CFDictionaryGetValue(dict,
					CFSTR(kIOLinkSpeed));
			if (numRef)
				CFNumberGetValue(numRef,
				kCFNumberLongLongType,
				&edata[NumEntries].linkSpeed);
			edata[NumEntries].linkStatus = 0;
			numRef = (CFNumberRef)
				CFDictionaryGetValue(dict,
					CFSTR(kIOLinkStatus));
			if (numRef)
				CFNumberGetValue(numRef,
					kCFNumberLongType,
					&edata[NumEntries].linkStatus);
			edata[NumEntries].activeMedium[0] = 0;
			stringRef = (CFStringRef)
				CFDictionaryGetValue(dict,
					CFSTR(kIOActiveMedium));
			if (stringRef)
				CFStringGetCString(stringRef,
					edata[NumEntries].activeMedium, 						32, kCFStringEncodingMacRoman);
		}
		kernResult = IORegistryEntryGetParentEntry(
			controllerService, kIOServicePlane,
			&parentService );
		if (KERN_SUCCESS != kernResult)
		{
			IOObjectRelease(controllerService);
			continue;
		}
		builtinAsCFData =
			IORegistryEntryCreateCFProperty(
			parentService, CFSTR("built-in"),
			kCFAllocatorDefault, 0);
		if (builtinAsCFData)
		{
			edata[NumEntries].isBuiltIn = true;
			CFRelease(builtinAsCFData);
		}
		else
		{
			edata[NumEntries].isBuiltIn = false;
		}
		IOObjectRelease(parentService);
		IOObjectRelease(controllerService);
	}
	*gNumEntries = NumEntries;
	IOObjectRelease(intfService);
	return kernResult;
}
static
void
mac_ether(LM_HANDLE *job)
{
  kern_return_t kernResult = KERN_SUCCESS;
  io_iterator_t intfIterator;
  EnetData edata[10];
  UInt16 i, index;
  UInt32 gNumEntries = 0;

	if (FindEthernetInterfaces(&intfIterator) != KERN_SUCCESS)
	{
		LM_SET_ERRNO(job, LM_CANTFINDETHER, 579, kernResult);
		job->idptr = (HOSTID *)0;
		goto exit_mac_ether;

	}

	if (GetEthernetData(intfIterator, edata, &gNumEntries) !=
							KERN_SUCCESS)
	{
		LM_SET_ERRNO(job, LM_CANTFINDETHER, 580, kernResult);
		job->idptr = (HOSTID *)0;
		goto exit_mac_ether;
	}
	for (index = 0; index < gNumEntries; index++)
	{
		if (edata[index].isBuiltIn != true)
			continue;
		for (i = 0; i < kIOEthernetAddressSize; i++)
			job->idptr->id.e[i] =
				edata[index].MACAddress[i];
		break;
#if 0
		printf ("GetLinkSpeed is : %lld\n",
				edata[index].linkSpeed);
		printf ("Ethernet active medium is : %s\n",
				edata[index].activeMedium);
		printf ("GetLinkStatus is : %lx\n",
				edata[index].linkStatus);
#endif

	}
	if (index == gNumEntries) /* didn't find one */
	{
		LM_SET_ERRNO(job, LM_CANTFINDETHER, 581, kernResult);
		job->idptr = (HOSTID *)0;
	}
exit_mac_ether:
	IOObjectRelease(intfIterator);
}

#endif /* MAC10 */
