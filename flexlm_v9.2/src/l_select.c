/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module:	l_select.c v1.14.0.0
 *
 *	Function: l_select()
 *
 *	Description:  Wraper around select
 *
 *	Parameters: same args as select
 *
 *	Return: 	same as select
 *
 *	D. Birns
 *	8/15/96
 *
 *	Last changed:  5/27/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#if defined ( PC16) | defined (NLM)
struct timeval { long tv_sec; long tv_usec; } ;
typedef int * fd_set;
int API_ENTRY select ( int nfds, fd_set FAR *, fd_set FAR *, fd_set FAR *,
			const struct timeval FAR * );
#endif /* PC16 */

#ifdef SELECT_FD_SET
#define FD_SET_T fd_set
#else
#define FD_SET_T int
#endif /* SELECT_FD_SET */

#ifdef WINNT
int PASCAL FAR  l__select (int nfds, int * readfds, int * writefds,
		int * except_fds, const struct timeval FAR * timeout);
#endif /* WINNT */

#ifdef OS2
/*
 * We need to do this because we don't want to include
 * pcsock.h in this file.
 */

struct timeval { long tv_sec; long tv_usec; } ;
typedef int * fd_set;
int API_ENTRY l__select( int nfds, fd_set*, fd_set*, fd_set*,
			const struct timeval* );
#define select(a,b,c,d,e) l__select(a,b,c,d,e)
#endif /* OS2 */

int
l_select(nfds, readfds, writefds, exceptfds, timeout)
int nfds;
int *readfds;
int *writefds;
int *exceptfds;
LM_TIMEVAL_PTR timeout;
{
#if defined( WINNT) || defined (OS2)
        if (!nfds && !readfds && !writefds && !exceptfds)
        {
                Sleep( (timeout->tv_usec/1000) + (timeout->tv_sec*1000) );
        	return 0;
	}
#endif

#ifdef PC16
        if (!nfds && !readfds && !writefds && !exceptfds)
        {
          DWORD end_time = GetTickCount()+(DWORD)(timeout->tv_sec*1000)+
			(DWORD) timeout->tv_usec;

		while( GetTickCount() < end_time )
			Yield();

		return 0;

        }
#endif

#if 0 /* defined( SUNOS5) && !defined(SUN64) && !defined(DEBUG_TIMERS)*/
	/* 32-bit solaris only to work around select fd limit */
	return select(nfds,
		(FD_SET_T *)readfds,
		(FD_SET_T *)writefds,
		(FD_SET_T *)exceptfds,
		timeout);
#else 
#if defined( NLM) || defined (WINNT) 
	return l__select(nfds,
		(FD_SET_T *)readfds,
		(FD_SET_T *)writefds,
		(FD_SET_T *)exceptfds,
		timeout);
#else /*-  not PC or SUN stuff */

	return select(nfds,
		(FD_SET_T *)readfds,
		(FD_SET_T *)writefds,
		(FD_SET_T *)exceptfds,
		timeout);
#endif /* PC */
#endif /* SUNOS5 */
}


#if defined (SUNOS5) && !defined(SUN64) && !defined(DEBUG_TIMERS)

#define NEW_SIZE 65536

#ifdef SUN64
typedef long FDTYPE;
#else
typedef int FDTYPE;
#endif

#include <values.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/poll.h>
#include "lmselect.h"

/*
 * Emulation of select() system call using poll() system call.
 *
 * Assumptions:
 *      polling for input only is most common.
 *      polling for exceptional conditions is very rare.
 *
 * Note that is it not feasible to emulate all error conditions,
 * in particular conditions that would return EFAULT are far too
 * difficult to check for in a library routine.
 *
 */

int pselect(int nfds, SELECT_MASK in0, SELECT_MASK out0, SELECT_MASK ex0, struct timeval *tv)
{
	/* register declarations ordered by expected frequency of use */
	register FDTYPE *in, *out, *ex;
	register u_long m;	/* bit mask */
	register int j;		/* loop counter */
	register u_long b;	/* bits to test */
	register int nn, rv, ms;
	struct pollfd pfd[NEW_SIZE];
	register struct pollfd *pp = pfd;
	int lastj = -1;
        FDTYPE zero = 0;
        FDTYPE in0null = 0;
        FDTYPE out0null = 0;
        FDTYPE ex0null = 0;

	/*
	* Check for invalid conditions at outset
	* Required for spec1170
	*/
	if (nfds < 0 || nfds > NEW_SIZE) {
		errno = EINVAL;
		return (-1);
	}

	if (tv != NULL) {
		/* tv_usec is invalid */
		if (tv->tv_usec < 0 || tv->tv_usec >= 1000000) {
			errno = EINVAL;
			return (-1);
		}

		/* tv_sec is invalid */
		if ((tv->tv_sec < 0) || (tv->tv_sec > 100000000)) {
			errno = EINVAL;
			return (-1);
		}
	}

	/*
	* If any input args are null, point them at the null array.
	*/
	if (in0 == NULL) {
		in0 = (SELECT_MASK)&zero;
                in0null = 1;
        }
	if (out0 == NULL) {
		out0 = (SELECT_MASK)&zero;
                out0null = 1;
        }
	if (ex0 == NULL) {
		ex0 = (SELECT_MASK)&zero;
                ex0null = 1;
        }

	/*
	* For each fd, if any bits are set convert them into
	* the apropriate pollfd struct.
	*/
	in = (FDTYPE*)in0;
	out = (FDTYPE*)out0;
	ex = (FDTYPE*)ex0;
	for (nn = 0; nn < nfds; nn += NFDBITS) {
		b = (u_long)(*in | *out | *ex);
		for (j = 0, m = 1; b != 0; j++, b >>= 1, m <<= 1) {
			if (b & 1) {
				pp->fd = nn + j;
				if (pp->fd >= nfds)
					goto done;
				pp->events = 0;
				if (*in & m)
					pp->events |= POLLRDNORM;
				if (*out & m)
					pp->events |= POLLWRNORM;
				if (*ex & m)
					pp->events |= POLLRDBAND;
				pp++;
			}
		}
                if (!in0null)
		   in++;
                if (!out0null)
		   out++;
                if (!ex0null)
		   ex++;
	}
done:
	/*
	* Convert timeval to a number of millseconds.
	*/
	if (tv == NULL) {
		ms = -1;
	} else if (tv->tv_sec > (MAXINT) / 1000) {
		ms = MAXINT;
	} else {
		/*
		* spec1170 complains if the value isn't rounded up.
		*/
		ms = (int)((tv->tv_sec * 1000) + (tv->tv_usec / 1000) +
					(tv->tv_usec % 1000 ? 1 : 0));
	}

	/*
	* Now do the poll.
	*/
	nn = pp - pfd;		/* number of pollfd's */
retry:
	rv = poll(pfd, (u_long)nn, ms);
	if (rv < 0) {		/* no need to set bit masks */
		if (errno == EAGAIN)
			goto retry;
		return (rv);
	} else if (rv == 0) {
		/*
		* Clear out bit masks, just in case.
		* On the assumption that usually only
		* one bit mask is set, use three loops.
		*/
		if (in0 != (SELECT_MASK)&zero) {
                        MASK_ZERO(in0);
		}
		if (out0 != (SELECT_MASK)&zero) {
                        MASK_ZERO(out0);
		}
		if (ex0 != (SELECT_MASK)&zero) {
                        MASK_ZERO(ex0);
		}
		return (0);
	}

	/*
	* Check for EINVAL error case first to avoid changing any bits
	* if we're going to return an error.
	*/
	for (pp = pfd, j = nn; j-- > 0; pp++) {
		/*
		* select will return EBADF immediately if any fd's
		* are bad.  poll will complete the poll on the
		* rest of the fd's and include the error indication
		* in the returned bits.  This is a rare case so we
		* accept this difference and return the error after
		* doing more work than select would've done.
		*/
		if (pp->revents & POLLNVAL) {
			errno = EBADF;
			return (-1);
		}
	}

	/*
	* Convert results of poll back into bits
	* in the argument arrays.
	*
	* We assume POLLRDNORM, POLLWRNORM, and POLLRDBAND will only be set
	* on return from poll if they were set on input, thus we don't
	* worry about accidentally setting the corresponding bits in the
	* zero array if the input bit masks were null.
	*
	* Must return number of bits set, not number of ready descriptors
	* (as the man page says, and as poll() does).
	*/
	rv = 0;
	for (pp = pfd; nn-- > 0; pp++) {
		j = pp->fd / NFDBITS;
		/* have we moved into another word of the bit mask yet? */
		if (j != lastj) {
			/* clear all output bits to start with */
			if (in0 != (SELECT_MASK)&zero) {
#ifdef SUN64
				in = (long *)&in0->fds_bits[j];
#else
				in = &in0[pp->fd / lm_bpi];
#endif
                                *in = 0;
                        }
			if (out0 != (SELECT_MASK)&zero) {
#ifdef SUN64
                                out = (long *)&out0->fds_bits[j];
#else
				out = &out0[pp->fd / lm_bpi];
#endif
                                *out = 0;
                        }
			if (ex0 != (SELECT_MASK)&zero) {
#ifdef SUN64
                                ex = (long *)&ex0->fds_bits[j];
#else
				ex = &ex0[pp->fd / lm_bpi];
#endif
                                *ex = 0;
                        }
			lastj = j;
		}
		if (pp->revents) {
			m = 1 << (pp->fd % NFDBITS);
			if (pp->revents & POLLRDNORM) {
				*in |= m;
				rv++;
			}
			if (pp->revents & POLLWRNORM) {
				*out |= m;
				rv++;
			}
			if (pp->revents & POLLRDBAND) {
				*ex |= m;
				rv++;
			}
			/*
			* Only set this bit on return if we asked about
			* input conditions.
			*/
			if ((pp->revents & (POLLHUP|POLLERR)) &&
				(pp->events & POLLRDNORM)) {
				if ((*in & m) == 0)
					rv++;	/* wasn't already set */
				*in |= m;
			}
			/*
			* Only set this bit on return if we asked about
			* output conditions.
			*/
			if ((pp->revents & (POLLHUP|POLLERR)) &&
			    (pp->events & POLLWRNORM)) {
				if ((*out & m) == 0)
					rv++;	/* wasn't already set */
				*out |= m;
			}
		}
	}
	return (rv);
}
#endif /* SUNOS5 */
