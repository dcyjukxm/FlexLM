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
  * $id: $
  *
  ************************************************************/
#ifndef _L_TIME_H
#define  _L_TIME_H

#ifdef USE_SYS_TIMES_H

#include <sys/times.h>

#elif !defined(PC)

#include <sys/time.h>

#else

#include <time.h>
struct timezone {
        int     tz_minuteswest;
        int     tz_dsttime;
};

#endif 

lm_extern void l_gettimeofday lm_args((struct timeval *tp,struct timezone *tzp));

#endif 
