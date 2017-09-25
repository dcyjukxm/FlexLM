/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_mem.c,v 1.2 2003/01/13 22:41:53 kmaclean Exp $
 *
 *	Function: l_mem_malloc, l_mem_free
 *
 *	Description: allocate and free -- avoid calls to malloc and free
 *		     in signal handlers
 *
 *	Parameters: l_mem_malloc:
 *			size 	-- size in bytes
 *			free_list -- pointer to a pointer to the first
 *				     item in a linked list of free space.
 *				     typically *free_list starts out
 *				     with a certain number of items
 *				     malloc'd at init time.
 *				     These items are generically represented
 *				     by the LM_MEM struct below.  The
 *				     crucial thing is that all linked lists
 *				     which use this module must have a 
 *				     "next" pointer as the first item in
 *				     the struct.
 *		     l_mem_free:
 *			free_list -- same as above
 *			ptr	-- ptr to memory to be free'd -- it's
 *				   actually made the first item on the
 *				   free_list
 *
 *	D. Birns
 *	8/15/94
 *
 *	Last changed:  11/13/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"

/* debugging macros */
#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif

/*
 *	LM_MEM -- generic list -- first item must be "next" pointer
 */
typedef struct mem_struct {
	struct mem_struct *next;
	void *v; 		/* place holder -- not used */
} LM_MEM; 
	

/*
 *	l_mem_malloc -- get item off free_list, or use malloc if
 *			free_list is empty.
 */
void *
l_mem_malloc(job, size, free_list)
LM_HANDLE *job;
int size;
void **free_list;
{
  LM_MEM **fl = (LM_MEM **)free_list, *tmp;

	DEBUG_INIT;
	DEBUG (("size %d free_list %x %s %d\n", size, *free_list, __FILE__, 
						__LINE__));
/* 
 *	The mask is to fix a 64-bit alpha bug! 
 */
        if (!((long)*fl & 0xffffffff)) 
        {
                *fl = (LM_MEM *)malloc(size);
		DEBUG (("malloc got %x %s,%d \n", *fl, __FILE__, __LINE__));
		if (!*fl) return (void *) 0;
		(void) memset((char *)*fl, 0, size);
        }
        tmp = *fl;
	*fl = (*fl)->next;
	tmp->next = (LM_MEM *)0;
	DEBUG (("returning %x free_list %x %s %d\n", tmp, *free_list, __FILE__, 
						__LINE__));
        return (void *)tmp;
}

/*
 *	l_mem_free -- puts ptr on free_list arg
 */
void
l_mem_free(ptr, free_list)
void *ptr;
void **free_list;
{
	DEBUG (("freeing %x free_list %x %s %d\n", ptr, *free_list, __FILE__, 
						__LINE__));

        ((LM_MEM *)ptr)->next = (LM_MEM *)*free_list;
        *free_list = ptr;

	DEBUG (("free_list %x %s %d\n", *free_list, __FILE__, __LINE__));
}
