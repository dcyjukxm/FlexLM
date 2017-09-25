/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lmselect.h,v 1.2 2003/01/13 22:13:11 kmaclean Exp $
 *
 *	Description: 	Macros/typedefs to make "select" argument handling
 *			transparent across systems.
 *
 *	M. Christiano
 *	7/26/88
 *
 *	Last changed:  3/13/98
 *
 */

#ifndef _LM_SELECT_H_
#define _LM_SELECT_H_

#ifdef USE_WINSOCK 

#include <pcsock.h>
typedef int  *SELECT_MASK;
#define	MASK_INIT(j, fd)	{ FD_ZERO((fd_set *)j); FD_SET(fd,(fd_set *) j); }
#define ANY_SET(j)		((fd_set *)j)->fd_count
#define MASK_CREATE(mask)	mask = (SELECT_MASK) malloc((unsigned) \
						   (sizeof(fd_set)))
#define MASK_DESTROY(mask)	(void) free(mask)
#define MASK_COPY(dest, src)	{ *((fd_set *)dest) = *((fd_set *)src); }
#define IS_SET(set, fd)		FD_ISSET( fd, (fd_set *)set )
#define MASK_CLEAR(set, fd)	FD_CLR( fd, (fd_set *)set )
#define MASK_SET(set, fd)	{ if (!FD_ISSET(fd, (fd_set *)set))  \
					 FD_SET(fd, (fd_set *)set); }
#define MASK_ZERO(set)		FD_ZERO((fd_set *)set)
#else
#ifdef USE_FD_SET_FOR_SELECT
typedef fd_set *SELECT_MASK;
#define       MASK_INIT(j, fd)        \
			{ FD_ZERO((fd_set *)j); FD_SET(fd,(fd_set *) j); }
#define MASK_CREATE(mask)     mask = (SELECT_MASK) malloc((unsigned) \
	(sizeof(fd_set)))
#define MASK_DESTROY(mask)    (void) free(mask)
#define MASK_COPY(dest, src)  { *((fd_set *)dest) = *((fd_set *)src); }
#define IS_SET(set, fd)               FD_ISSET( fd, (fd_set *)set )
#define MASK_CLEAR(set, fd)   FD_CLR( fd, (fd_set *)set )
#define MASK_CLEAR(set, fd)   FD_CLR( fd, (fd_set *)set )
#define MASK_SET(set, fd)     { if (!FD_ISSET(fd, (fd_set *)set))  \
	FD_SET(fd, (fd_set *)set); }
#define MASK_ZERO(set)                FD_ZERO((fd_set *)set)
#define ANY_SET(mask) l_any_set(mask, lm_max_masks)
#else

typedef int *SELECT_MASK;

#define IS_SET(mask, bit)	((mask)[(bit)/lm_bpi] & (1 << ((bit) % lm_bpi)))
#define MASK_SET(mask, bit)	(mask)[(bit)/lm_bpi] |= (1 << ((bit) % lm_bpi))
#define MASK_INIT(mask, bit)	MASK_ZERO(mask) \
				(mask)[(bit)/lm_bpi] = (1 << ((bit) % lm_bpi))
#define MASK_CLEAR(mask, bit)	(mask)[(bit)/lm_bpi] &= ~(1 << ((bit) % lm_bpi))
#define MASK_ZERO(mask)		{ int _i; for (_i = 0; _i < lm_max_masks; _i++)\
					(mask)[_i] = 0; }
#define MASK_COPY(dest, src)	{ int _i; for (_i = 0; _i < lm_max_masks; _i++)\
					(dest)[_i] = (src)[_i]; }
#define MASK_CREATE(mask)	mask = (SELECT_MASK) malloc((unsigned) \
						   (lm_max_masks * sizeof(int)))
#define MASK_DESTROY(mask)	(void) free(mask)

#define ANY_SET(mask)	l_any_set(mask, lm_max_masks)

#endif /* USE_WINSOCK */
#endif /* SUN64 */
  
extern int lm_nofile;		/*- NOFILE substitute (because of APOLLO) */
extern int lm_bpi;		/*- Number of bits per int on this machine */
extern int lm_max_masks;	/*- Number of masks we use */

#endif
