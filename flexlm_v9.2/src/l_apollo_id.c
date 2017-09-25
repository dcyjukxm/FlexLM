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
 *	Module: $Id: l_apollo_id.c,v 1.2 2003/01/13 22:41:49 kmaclean Exp $
 *
 *	Function: l_apollo_id()
 *
 *	Description: Extended hostid checking for Apollo 680x0 systems.
 *
 *	Parameters:	None.
 *
 *	Return:		(int) - Id of the current node.
 *
 *	M. Christiano
 *	1/15/90
 *
 *	Last changed:  10/18/95
 *
 */

/*-
Date: Tue, 12 Dec 89 14:51:46 PST
From: chuck@whitney (Chuck Wilson)
To: matt@whitney
Subject: More secure code for getting Apollo host-id:

Date: Tue, 12 Dec 89 09:16:47 MST
From: dave@hera (Dave Holcomb)
To: chuck@hera
Subject: lmgr.a

Chuck,

I was just talking to Jeff  Jennings about the license  manager.  I have
discovered that Matt  uses  pm_$get_sid_txt to  get  the node  id.  This
method has a serious drawback:  since the code  for this routine resides
in a dynamic library, all "I" have to do to circumvent  it is to bind in
a module with the same name/attributes which  returns the necessary data
to break the license.

Under non-disclosure with Apollo, we (at CAECO)  have used the following
routine  for several years.  Apollo has  said that they  will not change
the OS in this area so  that  the code should continue  to work for some
time.  The code  works under SR9.5+  and under SR10  "as is."  For it to
work  under SR9.2, it must  be compiled with  -DSR9_2.  The routine fits
into a file we call getid.c.

Give me a call for any info.

Dave Holcomb
*/

#ifdef apollo
#include "/sys/ins/base.ins.c"
#include "/sys/ins/proc2.ins.c"
#endif

l_apollo_id()
{
#ifdef apollo

  typedef int (*PFI)();
  PFI idp;
  unsigned int uid[2];
  short code[4];

#ifdef ISP__A88K
#if _ISP__A88K
	code[0] = 0x181f;
	code[1] = 0x0010;
	code[2] = 0x23e0;
	code[3] = 0x205f;
#else
	code[0] = 0x303c;
	code[1] = 0x0010;
	code[2] = 0x4e41;
	code[3] = 0x4e75;
#endif
#else
	code[0] = 0x303c;
	code[1] = 0x0010;
	code[2] = 0x4e41;
	code[3] = 0x4e75;
#endif
	cache_$clear();
	idp = (PFI) code;
	(*idp)(uid);
	return (uid[1] & 0xfffff);
#else
	return(0);
#endif /* apollo */
}
