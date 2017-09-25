/****************************************************************************
 *									    *
 *	COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.		    *
 *	This software has been provided pursuant to a License Agreement	    *
 *	containing restrictions on its use.  This software contains	    *
 *	valuable trade secrets and proprietary information of	 	    *
 *	Macrovision Corporation and is protected by law.  It may 	    *
 *	not be copied or distributed in any form or medium, disclosed 	    *
 *	to third parties, reverse engineered or used in any manner not 	    *
 *	provided for in said License Agreement except with the prior 	    *
 *	written authorization from Macrovision Corporation.		    *
 *									    *
 ****************************************************************************/
/*
 *	Module: $Id: ls_symtab.c,v 1.5 2003/03/12 21:29:26 sluu Exp $
 *
 * Jim McBeath, May 1990
 *
 * 11.May.90  jimmc  Initial definition
 * 14.Aug.90  jimmc  Move match and list functions out to separate files
 *	Last changed:  08/07/97
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "ls_sprot.h"
#include "lssymtab.h"
#ifdef PC
#include <string.h>
#include <stdlib.h>
#endif /* PC */

SpSymTab *SPsymtabsymtab;
#define SpMemBucket void
static SpMemBucket *SPbktSymtab;
#define IMMEDIATE 1
#define NULL_TERM_PTR 0
#define MIN_BINARY_SIZ 5

#define NAMEEQ(st,name1,name2) \
     (((name1)==(name2))||\
     (((st)->immediate == NULL_TERM_PTR)&&(strcmp((name1),(name2))==0)) ||\
     (((st)->immediate >= MIN_BINARY_SIZ) && \
	memcmp((name1), (name2), ((st)->immediate))==0))


#define SpFree free

#define LS_MALLOC(x)    ls_malloc((x), __LINE__, __FILE__)


char *
SpBktCalloc(ignore, siz)
SpMemBucket *ignore;
int siz;
{
  char *ret;
	ret = LS_MALLOC(siz);
	(void) memset(ret, 0, siz);
	return ret;
}

char *
SpBktStrSave(ignore, ptr)
SpMemBucket *ignore;
char *ptr;
{
  int len;
  char *ret;
	len = strlen(ptr);
	ret = LS_MALLOC(len + 1);
	(void)strcpy(ret, ptr);
	return ret;
}


SpSymTab *
SpSymInit(name,logsize,imm)
char *name;		/* name of symbol table */
int logsize;		/* log(2) of size, e.g. logsize=5 means size is 32 */
int imm;		/* set for immediate mode
			 *  0 if strings
			 *  1 if immediate -- pointers are the key
			 *  2-4 reserved
			 * >4 -- size to memcmp
			 */
{
	SpSymTab *st;
	int size,mask;
	int i,n;

	st = SpSymTabFind(name);
	st = (SpSymTab *)SpBktCalloc(SPbktSymtab,sizeof(SpSymTab));
	st->h.name = SpBktStrSave(SPbktSymtab,name);
	if (SPsymtabsymtab)
		SpSymAdd(SPsymtabsymtab,(SpSymHeader *)st);
	st->immediate = imm;
	if (logsize>16)
		logsize=16;
	size = 1;
	mask = -1;
	for (i=0;i<logsize;i++) {
		size = size<<1;
		mask = mask<<1;
	}
	st->logsize = logsize;
	st->size = size;
	st->mask = ~mask;
	n = sizeof(SpSymHeader *)*size;
	st->bins = (SpSymHeader **)SpBktCalloc(SPbktSymtab,n);
	return st;
}

/* Frees up the memory used by a symbol table.
 * Note that this does NOT free up the memory used by the entries in the
 * symbol table - before calling this function, you probably will want to
 * empty out the symbol table and free the storage you allocated for
 * each item. */
void
SpSymTabFree(st)
SpSymTab *st;
{
	if (SPsymtabsymtab)
		(void)SpSymRemove(SPsymtabsymtab,(SpSymHeader *)st);
	if (st->h.name)
		SpFree(st->h.name);
	if (st->bins)
		SpFree((char *)(st->bins));
	SpFree((char *)st);
}

SpSymTab *
SpSymTabFind(name)
char *name;
{
	SpSymTab *st;

	if (!SPsymtabsymtab)
		return (SpSymTab *)0;
	st = (SpSymTab *)SpSymFind(SPsymtabsymtab,name);
	return st;
}

#ifdef NOT_USED_BY_FLEXLM
void
SpSymResetStats(st)
SpSymTab *st;
{
	int h;
	SpSymHeader *sh;
	int maxlen;

	st->numsearches = 0;
	st->numcmps = 0;
	st->maxlen = 0;
	st->numsyms = 0;
	st->numbins = 0;
	for (h=0; h<st->size; h++) {
		maxlen = 0;
		if (st->bins[h])
			st->numbins++;
		for (sh=st->bins[h]; sh; sh=sh->next) {
			st->numsyms++;
			maxlen++;
		}
		if (maxlen>st->maxlen)
			st->maxlen=maxlen;
	}
}
#endif /* NOT_USED_BY_FLEXLM */

/* This hash function hashes the last st->logsize characters of the name
 * string.  This will cause symbols with the same last characters to hash
 * to the same index, which is OK because we use strcmp when scanning the
 * buckets, and strcmp starts comparing at the beginning, so will quickly
 * skip over the other symbols.  Thus the only potentially troublesome
 * symbols (in terms of creating undesirably long search times) would be
 * those which differ from each other only in the middle characters.
 */
int
SpSymHash(st,name)
SpSymTab *st;
char *name;
{
	int h;
	int i;
	union {
		char *p;
		char c[sizeof(char *)];
	} u;

	h = 0;
	switch (st->immediate) {
	case IMMEDIATE:
		u.p = name;
		for (i=0; i<sizeof(char *); i++) {
			h = (h<<1) + u.c[i];
		}
		break;
	case NULL_TERM_PTR:
		while (*name) {
			h = (h<<1) + *name++;
		}
		break;
	default:  /*compare immediate bytes*/
		for(i=0;i<st->immediate;i++) {
			if (*name)
			{
				h = (h<<1) + *name;
			}
			name++;
		}
		break;
	}

	h = h & st->mask;
	return h;
}

SpSymHeader *
SpSymFind(st,name)
SpSymTab *st;
char *name;
{
	int h;
	SpSymHeader *p;

	h = SpSymHash(st,name);
	st->numsearches++;
	for (p=st->bins[h];p;p=p->next) {
		st->numcmps++;
		if (NAMEEQ(st,p->name,name))
			return p;
	}
	return (SpSymHeader *)0;
}

/* adds a symbol to the table.  user must allocate it and pass to us */
SpSymHeader *
SpSymAdd(st,sym)
SpSymTab *st;
SpSymHeader *sym;
{
	int h;
	SpSymHeader *p;
	int maxlen;
	char buf[30];
	char *name;

	h = SpSymHash(st,sym->name);
	st->numsearches++;
	maxlen = 0;
	for (p=st->bins[h];p;p=p->next) {
		st->numcmps++;
		maxlen++;	/* count items in this chain */
		if (NAMEEQ(st,p->name,sym->name)) {
			switch (st->immediate) {
			case NULL_TERM_PTR:
				name = sym->name;
			default:
				(void)sprintf(buf,"0X%lX",sym->name);
				name = buf;
				break;
			}
			return (SpSymHeader *)0;
		}
	}
	maxlen++;	/* we're adding one */
	if (maxlen>st->maxlen)
		st->maxlen = maxlen;
	sym->next = st->bins[h];
	st->bins[h] = sym;
	st->numsyms++;
	if (!sym->next)
		st->numbins++;
	return sym;
}

/* removes a symbol from the table.  does not deallocate it */
/* if not in table, quietly ignores it */
SpSymHeader *
SpSymRemove(st,sym)
SpSymTab *st;
SpSymHeader *sym;
{
	int h;
	SpSymHeader **pp;

	h = SpSymHash(st,sym->name);
	st->numsearches++;
	for (pp= &(st->bins[h]); *pp; pp= &((*pp)->next)) {
		st->numcmps++;
		if (*pp == sym) {
			*pp = sym->next;
			sym->next = (SpSymHeader *)0;
			st->numsyms--;
			return sym;
		}
	}
	return (SpSymHeader *)0;	/* could not find it... */
}

SpSymHeader *
SpSymNext(st,sym)
SpSymTab *st;
SpSymHeader *sym;
{
	int h;
	SpSymHeader *p;

	if (!sym) {
		/* no previous symbol, return first symbol in table */
		for (h=0;h<st->size;h++) {
			if (st->bins[h])
				return st->bins[h];
		}
		return (SpSymHeader *)0;	/* empty table */
	}
	h = SpSymHash(st,sym->name);	/* get bin number */
	for (p=st->bins[h];p;p=p->next) {	/* scan to find symbol */
		if (p==sym) {
			if (p->next)	/* return next on chain if there */
				return p->next;
			/* if no next, we will fall out of loop */
		}
	}
	while (++h<st->size) {
		if (st->bins[h])
			return st->bins[h];
	}
	return (SpSymHeader *)0;	/* end of the table */
}

#ifdef NOT_USED_BY_FLEXLM
char *
SpSymNextName(st,name)
SpSymTab *st;
char *name;
{
	int h;
	SpSymHeader *p;

	if (!name) {
		/* no previous symbol, return first symbol in table */
		for (h=0;h<st->size;h++) {
			if (st->bins[h])
				return st->bins[h]->name;
		}
		return (char *)0;	/* empty table */
	}
	h = SpSymHash(st,name);	/* get bin number */
	for (p=st->bins[h];p;p=p->next) {	/* scan to find symbol */
		if (NAMEEQ(st,p->name,name)) {
			if (p->next)	/* return next on chain if there */
				return p->next->name;
			/* if no next, we will fall out of loop */
		}
	}
	while (++h<st->size) {
		if (st->bins[h])
			return st->bins[h]->name;
	}
	return (char *)0;	/* end of the table */
}
#endif /* NOT_USED_BY_FLEXLM */

/* end */
