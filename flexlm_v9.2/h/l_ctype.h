/******************************************************************************

	    COPYRIGHT (c) 2000, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_ctype.h,v 1.3 2003/05/08 01:32:31 kmaclean Exp $
 *
 *	Description:  Our own ctype
 *
 *	D. Birns
 *	9/2000
 *
 *	Last changed:  12/23/98
 *
 */

#ifndef _L_CTYPE_H
#define L_CTYPE_H

#ifdef isalpha
#undef isalpha
#endif
#ifdef isupper
#undef isupper
#endif
#ifdef islower
#undef islower
#endif
#ifdef isdigit
#undef isdigit
#endif
#ifdef isxdigit
#undef isxdigit
#endif
#ifdef isalnum
#undef isalnum
#endif
#ifdef isspace
#undef isspace
#endif
#ifdef ispunct
#undef ispunct
#endif
#ifdef isprint
#undef isprint
#endif
#ifdef isgraph
#undef isgraph
#endif
#ifdef iscntrl
#undef iscntrl
#endif
#ifdef isascii
#undef isascii
#endif

#define isalpha(x) l_isalpha(x)
#define isupper(x) l_isupper(x)
#define islower(x) l_islower(x)
#define isdigit(x) l_isdigit(x)
#define isxdigit(x) l_isxdigit(x)
#define isalnum(x) l_isalnum(x)
#define isspace(x) l_isspace(x)
#define ispunct(x) l_ispunct(x)
#define isprint(x) l_isprint(x)
#define isgraph(x) l_isgraph(x)
#define iscntrl(x) l_iscntrl(x)
#define isascii(x) l_isascii(x)

int l_isalnum(int c) ;
int l_isalpha(int c) ;
int l_iscntrl(int c) ;
int l_isdigit(int c) ;
int l_isgraph(int c) ;
int l_islower(int c) ;
int l_isprint(int c) ;
int l_ispunct(int c) ;
int l_isupper(int c) ;
int l_isxdigit(int c);

#endif /* L_CTYPE_H */
