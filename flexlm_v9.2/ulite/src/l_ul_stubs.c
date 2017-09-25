/******************************************************************************

	COPYRIGHT (c) 1998, 2003  by Macrovision Corporation.
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
 *	Module:	l_ul_stubs.c v1.6.0.0
 *
 *	Function:	Stubs for "FLEXlm-Ultralite"
 *
 *	Description: 	Stub routines for the smallest version of FLEXlm
 *
 *	Parameters:	see individual routines
 *
 *	Return:
 *
 *	M. Christiano
 *	2/28/98
 *
 *	Last changed:  8/11/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"

#define lm_errno err_info.maj_errno

char *l_ul_strncpy();

l_good_bin_date() { return(1); }
l_clear_error(job) LM_HANDLE *job; { if (job) job->lm_errno = 0; }
l_check_fmt_ver() { return(0); }

void l_set_error(job, err, x1, x2, x3, x4)
LM_HANDLE *job; int err, x1, x2, x4; char *x3;
{ if (job) job->lm_errno = err; }

void API_ENTRY
l_zcp(d, s, l)
char *d, *s;
int l;
{ if (d && s && l) { (void) l_ul_strncpy(d, s, l); *((d)+l) = '\0'; } 
  else if (d) *d = 0; 
}

void API_ENTRY 
l_uppercase(str)
char *str;
{
  char *cp;

	for (cp = str; *cp; cp++)
		if (*cp <= 'z' && *cp >= 'a')
			*cp = *cp + 'A' - 'a';
}
void API_ENTRY 
l_lowercase(str)
char *str;
{
  char *cp;

	for (cp = str; *cp; cp++)
		if (*cp <= 'Z' && *cp >= 'A')
			*cp = *cp + 'a' - 'A';
}

/*-----------------------------------------------------------------------*/
/*
 *	System functions
 */

#ifdef FLEXLM_ULTRALITE_STATIC_DEBUG
char _static_memory[LM_ULTRALITE_STATIC_SIZE];
#endif

#ifdef HP		/* was: __hp9000s800 */
#define VOID_STAR_MALLOC
#define VOID_STAR_CALLOC
#if 1		/* hpux11 */
#define SIZE_T_MALLOC_PARAM
#define CONST_STRCPY_PARAM
#define SIZE_T_STRLEN
#define CONST_CHAR_STRLEN_PARAM
#define CONST_CHAR_STRNCPY_PARAM
#define CONST_VOID_MEMCPY_PARAM
#define HP_MEMSET_PARAMS
#endif	/* 1  - hpux11 */
#endif

#ifdef VOID_STAR_MALLOC
void *
#else
char *
#endif
l_ul_malloc(n)
#ifdef SIZE_T_MALLOC_PARAM
size_t n;
#else
unsigned n;
#endif
{
#ifdef FLEXLM_ULTRALITE_STATIC_DEBUG
  static int first = 1;
#endif
  char *foo = (char *) LM_FUL_MALLOC_PTR;

#ifdef DEBUG
	printf("malloc(%d)\n", n);
#endif
#ifdef FLEXLM_ULTRALITE_STATIC_DEBUG
	if (first)
	{
		first = 0;
		(void) printf("Size of all statics/malloc area is %d\n",
				LM_ULTRALITE_STATIC_SIZE);
	}
#endif
	return(
#ifdef VOID_STAR_MALLOC
		(void *)
#endif
			foo);
}

#ifdef VOID_STAR_CALLOC
void *
#else
char *
#endif
l_ul_calloc(n, m)
#ifdef SIZE_T_MALLOC_PARAM
size_t n, m;
#else
unsigned n, m;
#endif
{
  int i;
  char *p = (char *) LM_FUL_MALLOC_PTR;
  char *foo = (char *) LM_FUL_MALLOC_PTR;

#ifdef DEBUG
	printf("calloc(%d, %d)\n", n, m);
#endif
	for (i = 0; i< n*m; i++) *p++ = '\0';
	return(
#ifdef VOID_STAR_MALLOC
		(void *)
#endif
			foo);
}
	
void l_ul_free(p) 
#ifdef VOID_STAR_MALLOC
	void
#else
	char 
#endif
		*p;	{ }

l_isxdigit(c) char c; { return((c >= '0' && c <= '9') ||
			     (c >= 'a' && c <= 'f') ||
			     (c >= 'A' && c <= 'F')); }
			
l_isdigit(c) char c; { return((c >= '0' && c <= '9')); }
l_isspace(c) char c; { return((c == ' ' || c == '\t')); }

/*
 * Copy string s2 to s1.  s1 must be large enough.
 * return s1
 */

char *
l_ul_strcpy(s1, s2)
char *s1;
#ifdef CONST_STRCPY_PARAM
const 
#endif
	char *s2;
{
  char *os1;

	os1 = s1;
	while (*s1++ = *s2++) ;
	return(os1);
}

#ifdef SIZE_T_STRLEN
size_t
#else
unsigned int
#endif
l_ul_strlen(s) 
#ifdef CONST_CHAR_STRLEN_PARAM
const char *s;
#else
const char *s;
#endif
{
#ifdef SIZE_T_STRLEN
  size_t
#else
  unsigned int
#endif
  		i = 0;

	while (*s++) i++;
	return (i);
}

/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

char *
l_ul_strncpy(s1, s2, n)
char *s1;
#ifdef CONST_CHAR_STRNCPY_PARAM
const
#endif
	char *s2;
int n;
{
  register i;
  register char *os1;

	os1 = s1;
	for (i = 0; i < n; i++)
		if ((*s1++ = *s2++) == '\0') {
			while (++i < n)
				*s1++ = '\0';
			return(os1);
		}
	return(os1);
}

void *
l_ul_memcpy(dest, src, num)
#ifdef CONST_VOID_MEMCPY_PARAM
void *dest;
const void *src;
size_t num;
#else
unsigned char *dest, *src;
int num;
#endif
{
  unsigned char *_dest = (unsigned char *) dest;
  unsigned char *_src = (unsigned char *) src;

	while (num-- > 0) *_dest++ = *_src++;
}

void *
l_ul_memset(dest, c, num)
#ifdef HP_MEMSET_PARAMS
void *dest;
int c;
size_t num;
#else
unsigned char *dest;
unsigned char c;
int num;
#endif
{
  unsigned char *_dest = (unsigned char *) dest;

	while (num-- > 0) *_dest++ = c;
}
