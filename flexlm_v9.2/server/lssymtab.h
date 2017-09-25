#ifndef SYMTAB_H
#define SYMTAB_H
/****************************************************************************
 *									    *
 *	COPYRIGHT (c) 1990, 2003  by Macrovision Corporation.		    *
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
 *	Module: $Id: lssymtab.h,v 1.4 2003/01/13 22:31:36 kmaclean Exp $
 *
 * Jim McBeath, May 1990
 *
 *	Last changed:  10/23/96
 * 11.May.90  jimmc  Initial definition
 */

typedef struct _SpSymHeader {
	char *name;		/* name of this symbol, or immediate key */
	struct _SpSymHeader *next;	/* linked list within bins */
} SpSymHeader;

typedef struct _SpSymTab {
	SpSymHeader h;		/* we maintain a symbol table of tables */
	int size;	/* number of bins (always a power of 2) */
	int logsize;	/* log(2) of size */
	int mask;	/* 1s in lower <logsize> bits */
	int immediate;	/* set means name is immediate instead of pointer */
	SpSymHeader **bins;	/* pointer to array of pointers */
	/* stats */
	int numsyms;		/* total number of symbols in table */
	int numbins;		/* number of bins which are in use */
	int numsearches;	/* incremented on each Find, Add, Remove */
	int numcmps;		/* when numsearches is incremented, this
				 * field is incremented by the number of
				 * compares needed to scan the chain */
	int maxlen;		/* max len of chain - updated after Add, but
				 * not touched on Remove. */
} SpSymTab;

#ifndef lm_args
#if defined(c_plusplus) || defined(__cplusplus)
#define  lm_ext  extern "C"
#define  lm_noargs  void
#define  lm_args(args) args
#else   /* ! c++ */
#define  lm_extern  extern
#if defined(__STDC__) || defined (__ANSI_C__) || defined(ANSI)
#define  lm_noargs  void
#define  lm_args(args) args
#else   /* ! stdc || ansi */
#define  lm_noargs  /* void */
#define  lm_args(args) ()
#endif  /* stdc || ansi */
#endif  /* c++ */
#endif  /* lm_args */

lm_extern SpSymTab 	*SpSymInit 	lm_args((char *,int , int ));
lm_extern SpSymHeader 	*SpSymFind 	lm_args((SpSymTab *, char *));
lm_extern SpSymHeader 	*SpSymAdd 	lm_args((SpSymTab *, SpSymHeader *));
lm_extern SpSymHeader 	*SpSymRemove 	lm_args((SpSymTab *, SpSymHeader *));
lm_extern SpSymHeader 	*SpSymNext 	lm_args((SpSymTab *, SpSymHeader *));
lm_extern char 		*SpSymNextName 	lm_args((SpSymTab *, char *));
lm_extern SpSymTab 	*SpSymTabFind 	lm_args((char *));
lm_extern void		SpSymMemInit 	lm_args((lm_noargs));
#endif /* SYMTAB_H */

/* end */
