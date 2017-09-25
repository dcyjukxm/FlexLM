/************************************************************
 * 
 *	Copyright (c) 2002, 2003 by Macrovision Corporation
 *	This software has been provided pursuant to a License Agreement
 *	containing restrictions on its use.  This software contains
 *	valuable trade secrets and proprietary information of
 *	Macrovision Corporation and is protected by law.  It may
 *	not be copied or distributed in any form or medium, disclosed
 *	to third parties, reverse engineered or used in any manner not
 *	provided for in said License Agreement except with the prior
 *	written authorization from Macrovision Corporation.
 ***********************************************************/
/***********************************************************
 *
 *   Created:    11/12/02 12:36:PM
 *   Author:     Kirk MacLean
 *   Comments:   Abstracted time functions for flex.
 * 				As I need to abstract more I'll put them in here.
 *
  * $Id: l_time.c,v 1.3 2003/01/14 21:47:11 kmaclean Exp $
  *
  ************************************************************/
#include "lmachdep.h"
#include "lmclient.h"
#include "l_time.h"

/**********************************************************************
 * abstracted gettimeofday function
 *********************************************************************/ 
void l_gettimeofday(struct timeval *tp, struct timezone *tzp)
{
#if defined(OS2)
	
	DATETIME datetime;
	DosGetDateTime(&datetime);

	tp->tv_sec = (int) 60*(60*((24*datetime.day)+datetime.hours)+
			      datetime.minutes)+datetime.seconds;
	tp->tv_usec = (int) datetime.hundredths*10;	// OS2 - end

#elif defined(PC) 
	
	DWORD cur_time_in_millisec = GetCurrentTime();
	tp->tv_sec = (int)(cur_time_in_millisec/1000);
	tp->tv_usec = (int)(cur_time_in_millisec%1000)*1000;

#elif defined(VXWORKS)
	
 	struct timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
  
	tp->tv_sec = ts.tv_sec;
	tp->tv_usec = ts.tv_nsec / 1000;
#else

	gettimeofday(tp,tzp);	

#endif
}
