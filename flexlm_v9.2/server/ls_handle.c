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
 *	Module: $Id: ls_handle.c,v 1.10 2003/04/18 23:47:59 sluu Exp $
 *
 *	Daniel Birns
 *	April, 1993
 *	
 *	Last changed:  8/7/97
 *
 */

#include <stdio.h>
#include <sys/types.h>
#ifdef PC
#include <process.h>
#endif /* PC */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"

#ifdef DEBUG_HANDLE
CLIENT_DATA * ls_lookup_client(ca) CLIENT_ADDR *ca; {}
void ls_log_prefix(where,keyword) int where; char *keyword; {}
void ls_log_asc_printf() {}
#undef LS_MALLOC
#undef DLOG
#undef _DLOG
#undef LOG
#undef _LOG
#define LS_MALLOC(x) malloc(x)
#define DLOG(x) printf x 
#define _DLOG(x) printf x
#define LOG(x) printf(x)
#define _LOG(x) printf(x)
#endif

#define CLIENT_DELETE 	0
#define CLIENT_ADD 	1
#define CLIENT_LOOKUP 	2
#ifndef RELEASE_VERSION
static int debug_handle = -1;
#endif

#define H_NUM_BITS (sizeof(int) * 8)
#define H_SETBIT(i, b)  ((i)|=1<<(b))
#define H_UNSETBIT(i, b)  ((i)&=(0xffffffff&~(1<<(b))))
#define H_ISBITSET(i, b)  (((1<<(b))&(i)) >> (b))
#define FREE(x) free(x)

static CLIENT_DATA **handle_to_client;

typedef struct _handle_list {
    int hand_bits;
    int hand_num;
    struct _handle_list *hand_next;
} HANDLE_LIST;

static HANDLE_LIST *first_handle = 0;

/*
 *	Static prototypes here
 */
static HANDLE_LIST * ls_new_handle lm_args((int last_num));
static int num_handles lm_args((lm_noargs));

HANDLE_T
ls_handle_by_client(client)
CLIENT_DATA *client;
{
#ifndef RELEASE_VERSION
    if (debug_handle)
    {
	if (client && (client->handle))
	    (void) printf("ls_handle_by_client: pid=%d &client=%x, handle=%d\n",
			getpid(), client, client->handle);
	else
	    (void) printf("ls_handle_by_client: &client=0\n");
    }
#endif
    if (client && (client->handle))
	return (client->handle);
    else 
	return (HANDLE_T) 0;
}

HANDLE_T
ls_handle_by_addr(addr)
CLIENT_ADDR *addr;
{
    CLIENT_DATA *c;
    c = ls_lookup_client(addr);
#ifndef RELEASE_VERSION
    if (debug_handle)
    {
	(void) printf("ls_handle_by_addr: pid=%d &addr=%x, &client=%x",
			getpid(), addr, c);
	if (c)
	    (void) printf(" handle=%d", c->handle);
	puts("");
    }
#endif
    if (c)
	return c->handle;
    else 
	return (HANDLE_T) 0;
}

CLIENT_DATA *
ls_client_by_handle(handle)
HANDLE_T handle;
{
  int h = handle;
#ifndef RELEASE_VERSION
    if (debug_handle)
    {
	(void) printf("ls_client_by_handle: pid=%d handle=%d &client = %x\n",
			getpid(), handle, handle_to_client[handle]);
    }
#endif
    if (h < 0) h = -h;
    if (h >0 && (h < num_handles()))
    {
	return handle_to_client[h];
    }
    else
    {
/*-
 *	- serverless calls this function
 *		with a NULL handle, so don't log NULL handles
 */
	if (handle > 0)
		LOG(("INTERNAL ERROR: Got invalid handle %d %s %d\n", handle,
						__FILE__, __LINE__));
	return (CLIENT_DATA *)NULL;
    }
}

CLIENT_ADDR *
ls_addr_by_handle(handle)
HANDLE_T handle;
{
#ifndef RELEASE_VERSION
    if (debug_handle)
    {
		(void) printf("ls_addr_by_handle: pid=%d handle=%d",
				getpid(), handle);
		if (handle_to_client[handle])
			(void) printf(" addr=%x", &handle_to_client[handle]->addr);
		puts("");
    }
#endif
	/*
	 *	Make sure we're still in range here
	 */
	if(num_handles() < handle)
	{
		return NULL;
	}
    if (handle > 0 && handle_to_client[handle])
    {
		return &handle_to_client[handle]->addr;
    }
    else
    {
/*-
 *	- serverless calls this function
 *		with a NULL handle, so don't log NULL handles
 */
		if (handle > 0)
		{
#if 0
			/*
			 *	Why do we output this VAGUE quasi error message?
			 */
			DLOG((lmtext("Possible internal error: Got invalid handle %d\n"), handle));
			DLOG((lmtext("	Not error if linger, client has exited (P973 and P5910)\n"), handle));
#endif
		}
		return (CLIENT_ADDR *)NULL;
    }
}

static
int
num_handles()
{
    HANDLE_LIST * hp;
    int num = 0;

    for( hp = first_handle; hp ;hp = hp->hand_next)
    {
	num += H_NUM_BITS;
    }
    return num;
}


HANDLE_T
ls_create_handle(client)
CLIENT_DATA *client;
{

    HANDLE_T i, ret = 0;
    HANDLE_LIST * hp;
    int outahere = 0;
    if (!first_handle) 
    {
	first_handle = ls_new_handle(-1);
/*
 *	Handle 0 is a bad handle -- I set it so it will never be used
 */
	if (!first_handle)
	    return (HANDLE_T) 0;
	H_SETBIT(first_handle->hand_bits,0);
	handle_to_client[0] = (CLIENT_DATA *)NULL;

    }
    hp = first_handle;
    do 
    {
	for (i=0;i<H_NUM_BITS;i++)
	{
	    if (!H_ISBITSET(hp->hand_bits, i))
	    {
		H_SETBIT(hp->hand_bits, i);
		ret = i+(hp->hand_num * H_NUM_BITS);
		outahere = 1;
		break;
	    }
	}
	if (outahere)
	    break;
    } while (hp->hand_next && (hp = hp->hand_next));
/*
 *	We need to malloc a new handle int
 */
    if (!ret)
    {
	hp->hand_next = ls_new_handle(hp->hand_num);
	hp = hp->hand_next;
	H_SETBIT(hp->hand_bits,0);
	ret = 0+(hp->hand_num * H_NUM_BITS);
    }
    handle_to_client[ret] = client;
    client->handle = ret;
#ifndef RELEASE_VERSION
    if (debug_handle == -1)
	debug_handle = (int)l_real_getenv("DEBUG_HANDLE");	
    if (debug_handle)
	(void) printf("ls_create_handle: pid=%d &client=%x handle=%d\n", getpid(), 
		client, ret);
#endif
    return(ret);
}
void
ls_delete_handle(handle)
HANDLE_T handle;
{
    HANDLE_LIST *hp;
    int num = handle/H_NUM_BITS;
    hp = first_handle;
    do 
    {
	;
    } while (hp && (hp->hand_num != num) && (hp = hp->hand_next)) ;
    if (!hp)
    {
	LOG((lmtext("INTERNAL ERROR: deleting non-existent handle: %d\n"), 
	handle));
	return;
    }
    else
    {
	if (H_ISBITSET(hp->hand_bits, handle%H_NUM_BITS))
	{
	    H_UNSETBIT(hp->hand_bits, handle%H_NUM_BITS);
	}
	else
	{
	    LOG((lmtext("INTERNAL ERROR: deleting non-existent handle: %d\n"), 
	    handle));
	    return;
	}
    }
#ifndef RELEASE_VERSION
    if (debug_handle)
    {
	if (!handle_to_client) {
	    DLOG((lmtext("UH OH!! %d@%s"), __LINE__, __FILE__));
	} else {
	    (void) printf("ls_delete_handle: pid=%d handle=%d &client=%d\n",
		    getpid(), handle, 
			handle_to_client[handle]->addr.transport == LM_UDP ? 
			handle_to_client[handle]->addr.addr.sin.sin_port :
			handle_to_client[handle]->addr.addr.fd);
	}
    }
#endif
    handle_to_client[handle] = (CLIENT_DATA *)NULL;
}
static HANDLE_LIST *
ls_new_handle(last_num)
int last_num;
{
    HANDLE_LIST *hp;
    int olen;
    int len_1;

    hp= (HANDLE_LIST *) LS_MALLOC(sizeof(HANDLE_LIST));
    hp->hand_num = last_num + 1;
    hp->hand_bits = 0;
    hp->hand_next = (HANDLE_LIST *)NULL;
    if (!hp)
	return hp;
    len_1 = sizeof (CLIENT_DATA *) * H_NUM_BITS ;
    if (!handle_to_client)
    {
	handle_to_client = (CLIENT_DATA **)LS_MALLOC(len_1);
	(void) memset((char *)handle_to_client, 0, len_1);
    }
    else
    {
	olen = (hp->hand_num) * len_1;
	handle_to_client = (CLIENT_DATA **)realloc(handle_to_client, 
			olen + len_1);
	(void) memset(((char *)handle_to_client) + olen, 0, len_1);
    }

    if (!handle_to_client)
	return (HANDLE_LIST *)0;
    return hp;
}
static void
ls_dump_handles()
{
    HANDLE_LIST *hp;
    HANDLE_T ret;
    int i;
    for (hp=first_handle; hp ;hp = hp->hand_next) 
    {
	for (i=1;i<H_NUM_BITS;i++)
	{
	    if (H_ISBITSET(hp->hand_bits, i))
	    {
		ret = i+(hp->hand_num * H_NUM_BITS);
		(void) printf("bit set %d client = %x\n",  ret, 
					ls_client_by_handle(ret));
	    }
	}
    }
}
#ifdef DEBUG_HANDLE
#if 0
int ls_show_vendor_def,
ls_i_am_lmgrd,
f_nousers,
ls_unlock,
ls_feat_info,
ls_feat_dump;

long
l_getattr(i)
int i;
{}
shutdown() {};
#endif


main()
{
    CLIENT_DATA c[4];
    HANDLE_T h;
    int j,i;
    puts("----sockaddr_in--------------------------------");

	puts("----Add----");
	for (i=1;i<50;i++) {
	    h = ls_create_handle(&c[i%4]);
	    (void) printf("%d:handle = %d, client = %x/%x\n", i, h, 
				ls_client_by_handle(h), &c[i%4]);
	    /*();*/
	}

	puts("----delete----");
	for (i=1;i<30;i++) {
	    ls_delete_handle((HANDLE_T)i);
	    (void) printf("deleting %d\n", i);
	}
	ls_dump_handles();
	puts("----Add----");
	for (i=1;i<50;i++) {
	    h = ls_create_handle(&c[i%4]);
	    (void) printf("%d:handle = %d, client = %x/%x\n", i, h, ls_client_by_handle(h), &c[i%4]);
	}
	for (i=1;i<200;i++)
		printf("ls_client_by_handle(%d) is 0x%x\n", i, 
			ls_client_by_handle(i));
}
#endif
