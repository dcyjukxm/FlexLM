/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_dict.c,v 1.4 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function: 	ls_dict
 *
 *	Description: 	dictionary for report log
 *
 *	Parameters:	(char *) str
 *
 *	Return:
 *
 *	D. Birns 
 *	11/6/95
 *
 *	Last changed:  5/31/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifndef DEBUG 
#include "lsserver.h"
#else
#define DLOG
#define LS_MALLOC ls_malloc


void *
ls_malloc(x)
{
	return malloc(x);
}
#endif /* DEBUG */

#include "lssymtab.h"
#include "lsreplog.h"

#define MALLOC_SIZE 0x3000

typedef struct _dict {
	SpSymHeader h;
	char *str;
	DICTKEY id;
} DICT;

typedef struct _userdict {
	SpSymHeader h;
	DICTKEY ipaddr;
	DICTKEY hostname;
	DICTKEY platform;
	DICTKEY user;
	DICTKEY id;
} USERDICT;

static long entries[2], hits[2], mem[2];
static DICTKEY lastdid;
static DICTKEY uid;

static char *curr;
typedef struct _mem {
	struct _mem *next;
	char *mem;
} MEM;
static MEM *currmem;
static MEM *startmem;
static int  currmemsiz;
static int  totmemsiz;

static SpSymTab *dictsyms;
static SpSymTab *usersyms;
/*
 *	prototypes here 
 */
static void mem_flush lm_args((lm_noargs));
static MEM *  more_mem lm_args((lm_noargs));
static DICTKEY dict_add lm_args((char *));
static DICTKEY user_add lm_args((DICTKEY, DICTKEY, DICTKEY, DICTKEY));
static char * dict_malloc lm_args((int));
static void free_shp lm_args(( SpSymTab *, SpSymHeader *));

/* 
 *	ls_dict_lookup_str -- lookup by string
 *	returns id
 */
DICTKEY
ls_dict_lookup_str(str)
char *str;
{
  DICT *d;

	if (d = (DICT *)SpSymFind(dictsyms, str))
	{
		hits[0]++;
		return d->id;
	}
	return (0);
}

void
ls_dict_flush()
{
  DICT *dict = (DICT *)0;
  USERDICT *user = (USERDICT *)0;
  SpSymHeader *shp = (SpSymHeader *)0;

	if (dictsyms)
	{
		dict = 0;
		while (dict = (DICT *)SpSymNext(dictsyms, (SpSymHeader *)dict))
		{
			free_shp(dictsyms, shp);
			shp = (SpSymHeader *)dict;
		}
		free_shp(dictsyms, shp);
	}
	shp = (SpSymHeader *)0;
	if (usersyms)
	{
		while (user = (USERDICT *)SpSymNext(usersyms, 
							(SpSymHeader *)user))
		{
			free_shp(usersyms, shp);
			shp = (SpSymHeader *)user;
		}
		free_shp(usersyms, shp);
	}
	mem_flush();
}
static
void
free_shp(st, shp)
SpSymTab *st;
SpSymHeader *shp;
{

	if (shp)
	{
		if (SpSymRemove(st, (SpSymHeader *)shp))
			free(shp);
		else
		{
			DLOG(("INTERNAL ERROR %s %d\n", __FILE__, __LINE__));
		}
	}
}

		
static
DICTKEY
dict_add(str)
char *str;
{
  DICT *d;
  int len = strlen(str) + 1;

	entries[0]++;
	mem[0] += len;
	d = (DICT *)LS_MALLOC(sizeof (DICT));
	memset (d, 0, sizeof(DICT));
	d->str = (char *)dict_malloc(len);
	strcpy(d->str, str);
	d->id = ++lastdid;
	if (!dictsyms)
	{
/*
 *              Note:  The size, 2nd arg, is 2^n size hash.  So, 10 is
 *                      2^10 (=1024) which is a pretty big hash number
 *
 */
		dictsyms = SpSymInit("dict", 10, 0);
	}
	d->h.name = (char *)d->str;
	if (!SpSymAdd(dictsyms, (SpSymHeader *)d))
	{
#ifndef DEBUG
		DLOG(
		("INTERNAL ERROR: SpSymAdd failed"));
		ls_go_down(EXIT_MALLOC);
#endif
	}


	return d->id;
}

/*
 *	ls_dict -- returns id
 */
DICTKEY
ls_dict(str)
char *str;
{
  char buf[MAX_CONFIG_LINE];
  DICTKEY did;

	if (dictsyms && (did = ls_dict_lookup_str(str)))
		;
	else
	{
		did = dict_add(str);
		sprintf(buf, "%x %x %s\n", LL_DICTIONARY, did, str);
		ls_append_repfile(buf);
#if DEBUG
		printf(buf);
#endif
	}
	return did;
}

void
ls_dict_stat(buf)
char *buf;
{
  int i;

	sprintf(buf, "%x %x %x %x %x %x %x %x %x\n", 
		LL_STAT, entries[0], hits[0],
		mem[0], entries[1], hits[1], mem[1], lastdid, uid);
	for(i=0;i<2;i++)
		mem[i] = hits[i] = entries[i] = 0;

	
}

static
DICTKEY
user_add(kipaddr, khostname, kplatform, kuser)
DICTKEY kipaddr, khostname, kplatform, kuser;
{
  USERDICT *u;

	entries[1]++;
	mem[1] += sizeof(USERDICT);
	u = (USERDICT *)LS_MALLOC(sizeof (USERDICT));
	memset (u, 0, sizeof(USERDICT));
	u->ipaddr = kipaddr;
	u->hostname = khostname;
	u->platform = kplatform;
	u->user = kuser;
	u->id = ++uid;
	if (!usersyms)
	{
/*
 *              Note:  The size, 2nd arg, is 2^n size hash.  So, 10 is
 *                      2^10 (=1024) which is a pretty big hash number
 *
 */
		usersyms = SpSymInit("user", 10, sizeof(DICTKEY) * 4);
	}
	u->h.name = (char *)&(u->ipaddr);
	if (!SpSymAdd(usersyms, (SpSymHeader *)u))
	{
#ifndef DEBUG
		DLOG(
		("INTERNAL ERROR: SpSymAdd failed"));
		ls_go_down(EXIT_MALLOC);
#endif
	}


	return u->id;
}
ls_user_lookup(kipaddr, khostname, kplatform, kuser)
DICTKEY kipaddr, khostname, kplatform, kuser;
{
  USERDICT u, *up;

	u.ipaddr = kipaddr;
	u.hostname = khostname;
	u.platform = kplatform;
	u.user = kuser;

	if (up = (USERDICT *)SpSymFind(usersyms, (char *)&(u.ipaddr)))
	{
		hits[1]++;
		return up->id;
	}
	return (0);
}
		

int
ls_user(client)
CLIENT_DATA *client;
{
	
  char buf[MAX_CONFIG_LINE];
  char ipaddr[9];
  DICTKEY cid, kipaddr, khostname, kplatform, kuser;

	
	sprintf(ipaddr, "%02x%02x%02x%02x", 
		client->inet_addr[0],
		client->inet_addr[1],
		client->inet_addr[2],
		client->inet_addr[3]);
	kipaddr = ls_dict(ipaddr);
	khostname = ls_dict(client->node);
	kplatform = ls_dict(client->platform);
	kuser = ls_dict(client->name);
	
	if (usersyms && (cid = 
		ls_user_lookup(kipaddr, khostname, kplatform, kuser)))
		;
	else
	{
		cid = user_add(kipaddr, khostname, kplatform, kuser);
		sprintf(buf, "%x %x %x %x %x %x\n", LL_USER, cid, 
			kipaddr, khostname, kplatform, kuser);
#ifdef DEBUG
		printf(buf);
#else
		ls_append_repfile(buf);
#endif
	}
	return cid;
}


static
char *
dict_malloc(siz)
int siz;
{
  MEM *memtmp;
  char *ret;

	if (!startmem) 
	{
		startmem = currmem = more_mem();
		curr = currmem->mem;
	}
	if ((currmemsiz + siz) > MALLOC_SIZE)
	{
		currmemsiz = 0;
		memtmp = currmem;
		memtmp->next = currmem = more_mem();
		curr = currmem->mem;
        }
	currmemsiz += siz;
	totmemsiz += siz;
	ret = curr;
	curr += siz;
	return ret;
}

static
MEM *
more_mem()
{
  MEM *m;

	m = (MEM *)LS_MALLOC(sizeof(MEM));
	memset(m, 0, sizeof(MEM));
	m->mem = (char *)LS_MALLOC(MALLOC_SIZE);
	return m;
}

static
void
mem_flush()
{
  MEM *m, *next;
	
	for (m = startmem; m; m = next)
	{
		next = m->next;
		free(m->mem);
		free(m);
	}
	startmem = currmem = (MEM *)0;
	currmemsiz = 0;
	totmemsiz  = 0;
}

void
ls_dict_init()
{
	lastdid = uid =  0;
}
