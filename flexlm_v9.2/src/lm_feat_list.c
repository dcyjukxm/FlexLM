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
 *	Module: $Id: lm_feat_list.c,v 1.18 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	lc_feat_list(job, flags, dupaction)
 *
 *	Description:	Gets the list of features from the config file.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) flags - LM_A_FLIST_ONLY_FLOATING, etc.
 *			(void) (*dupaction)() - Action routine when
 *						 a duplicate feature is found.
 *
 *	Return:		(char **) - The list of features
 *
 *	Notes:		The strings and pointers to the strings are
 *			malloc'ed.  Each subsequent call will free
 *			the old list and re-malloc.
 *
 *	M. Christiano
 *	9/28/88
 *
 *	Last changed:  12/15/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_openf.h"
#include "lgetattr.h"
#include <stdio.h>
#if !defined(MOTO_88K) && !defined(PC)
#include <sys/types.h>
#ifndef VXWORKS
#include <sys/file.h>
#else 
#include "iolib.h"
#endif
#endif
#include <sys/types.h>
#include <errno.h>
#include "l_prot.h"


static char **	lc_feat_list_lfp	lm_args((LM_HANDLE *, LICENSE_FILE *lf, 
					int flags));
static char ** 	l_feat_list_curr 	lm_args(( LM_HANDLE *, int));
static void 	l_remove_dup 		lm_args((LM_HANDLE *, char **));
static void 	l_remove_dups		lm_args(( LM_HANDLE *, void (*)()));	/* Call when a duplicate is found */


char ** API_ENTRY
lc_feat_list(job, flags, dupaction)
LM_HANDLE *job;		/* Current license job */
int flags;		/* flags */
void (*dupaction)(void);	/* Call when a duplicate is found */
{

  char **ofeat_ptrs = 0;
  char **nfeat_ptrs = 0;
  char *nfeatures = 0;
  char *ofeatures = 0;

	if (LM_API_ERR_CATCH) return 0;

	if (!(flags & LM_FLIST_ALL_FILES))
		LM_API_RETURN(char **, l_feat_list_curr(job, flags))
/*
 *	Do all files 
 */
	if (job->feat_ptrs) /* free and init if necessary */
	{
		free(job->features);
		free(job->feat_ptrs);
		job->feat_ptrs = 0;
		job->features = 0;
	}
	if (!job->lic_files) l_init_file(job);
	for (job->lfptr = LFPTR_FILE1; job->lic_files[job->lfptr]; job->lfptr++)
	{
/* 
 *		Don't read files twice
 */
		if (job->lfptr > LFPTR_FILE1)
		{
		  int i;
			for (i = LFPTR_FILE1; i < job->lfptr; i++)
			{
				if (L_STREQ(job->lic_files[job->lfptr], 
						job->lic_files[i]))
					goto next_one; /* already have */
			}
		}
		if (job->feat_ptrs)
		{
			ofeat_ptrs = job->feat_ptrs; /* save them */
			ofeatures = job->features; /* save them */
			job->feat_ptrs = 0;
			job->features = 0;
		}
		l_feat_list_curr(job, flags);

		if (ofeat_ptrs ) /* merge the 2 lists */
		{
		  int i, cnt=0, ocnt=0, ncnt=0, len = 0, olen = 0, nlen = 0;
		  char *cp;

/*
 *			count number of feature names
 */
			for (i = 0, ocnt = 0; ofeat_ptrs[i]; ocnt++, i++) 
				olen += strlen(ofeat_ptrs[i]) + 1; 
			if (job->feat_ptrs)
			{
				for (i = 0, ncnt = 0; job->feat_ptrs[i]; 
								ncnt++, i++) 
				{
					nlen += strlen(job->feat_ptrs[i]) + 1; 
				}
			}
			cnt = ocnt + ncnt + 1; /* for null terminator */
			len = olen + nlen;
/*
 *			malloc new memory
 */
			nfeatures = (char *)l_malloc(job, len);
			nfeat_ptrs = (char **)l_malloc(job, cnt * sizeof(char *));
/*
 *			Copy from old into new
 */
			cnt = 0;
			for (i = 0, cp = nfeatures; ofeat_ptrs[i]; 
					cnt++, i++, cp += strlen(cp) + 1)
			{
				strcpy(cp, ofeat_ptrs[i]);
				nfeat_ptrs[cnt] = cp;
			}
			if (job->feat_ptrs)
			{
			  char *cp2;
				for (i = 0; job->feat_ptrs[i]; 
					cnt++, i++, cp += strlen(cp) + 1)
				{
					strcpy(cp, job->feat_ptrs[i]);
					if (cp2 = strchr(cp, '\\'))
						*cp2 = 0;
						
					nfeat_ptrs[cnt] = cp;

				}
			}
			free(ofeat_ptrs);
			free(ofeatures);
			free (job->feat_ptrs);
			free (job->features);
			job->feat_ptrs = nfeat_ptrs;
			job->features = nfeatures;
		}
next_one:	;
	}
	l_remove_dups(job, dupaction);
	LM_API_RETURN(char **, job->feat_ptrs)
}

static
char ** 
l_feat_list_curr(job, flags)
LM_HANDLE *job;		/* Current license job */
int flags;		/* flags */
{
  LICENSE_FILE *lf;
  char **p;

	if (job->feat_ptrs)
	{
		free(job->features);
		free((char *) job->feat_ptrs);
		job->feat_ptrs = 0;
		job->features = 0;
	}
	if (!(lf = l_open_file(job, LFPTR_CURRENT)))
	{
		return(job->feat_ptrs);
	}
	else
	{
		p = lc_feat_list_lfp(job, lf, flags);
		l_lfclose(lf);
		return(p);
	}
}

static
char **
lc_feat_list_lfp(job, lf, flags)
LM_HANDLE *job;		/* Current license job */
LICENSE_FILE *lf;
int flags;		/* flags */
{
  char line[MAX_CONFIG_LINE+1];
  char *f1, *f2, *f3, *f4, *f5, *f6;
  int nflds;
  int cur = -1;
  char *curptr;
  int numfeat = 0, howbig = 0;
  CONFIG *pkgs = 0, *temppkg = 0, *conf, *lfconf;
  int isdec = 0;
  int i;

	if (job->lm_errno == LM_HOSTDOWN)
		l_clear_error(job);
	for (i = 0; i < job->lm_numlf; i++)
		if (&job->license_file_pointers[i] == lf )
			break;
	for (lfconf = job->line; lfconf; lfconf = lfconf->next)
		if (lfconf->lf == i)
			break;
#ifndef NO_FLEXLM_CLIENT_API
	if (lf->type == LF_PORT_HOST_PLUS)
	{
		return l_get_featlist_from_server(job, flags, lfconf);
	}
#endif /* NO_FLEXLM_CLIENT_API */

	f1 = (char *)l_malloc(job, (MAX_CONFIG_LINE+1)*7);	/* memory threat */
	f2 = f1+MAX_CONFIG_LINE+1;
	f3 = f2+MAX_CONFIG_LINE+1;
	f4 = f3+MAX_CONFIG_LINE+1;
	f5 = f4+MAX_CONFIG_LINE+1;
	f6 = f5+MAX_CONFIG_LINE+1;

	while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
	{
		
	    nflds = 0;
	    if (!(isdec = l_decimal_format((unsigned char *)line)))
		    nflds = sscanf(line, "%s %s %s %s %s %s", f1, f2, f3,	/* overrun checked */
						f4, f5, f6);
            if (nflds == 1 && l_keyword_eq(job, f1, LM_RESERVED_USE_SERVER) && 
		!(job->flags & LM_FLAG_IS_VD))
	    {
		free(f1);
#ifdef NLM
		return 0;
#else
		return l_get_featlist_from_server(job, flags, lfconf);
#endif /* NLM */
	    }
		

/*
 *  	    This tests if it's INCREMENT/PACKAGE -- we aren't a LITE user 
 */

            if (isdec || 
		(nflds == 6 && (!(flags & LM_FLIST_ONLY_FLOATING) || 
			strcmp(f6, "0")) &&
			(l_keyword_eq(job, f1, LM_RESERVED_FEATURE) ||
			l_keyword_eq(job, f1, LM_RESERVED_UPGRADE) ||
			l_keyword_eq(job, f1, LM_RESERVED_INCREMENT) ||
			l_keyword_eq(job, f1, LM_RESERVED_PACKAGE) )))
	    {
			if (l_keyword_eq(job, f1, LM_RESERVED_PACKAGE))
			{
/*
 *				We create a matrix of CONFIG's:
 *				in one direction is packages, and
 *				in the other direction is components
 */
				temppkg = (CONFIG *)
						l_malloc(job, sizeof (CONFIG));
/*
 *				We use f1 as big safe temp space
 *				it should be unused at this point in
 *				the code
 */
				if (l_parse_package(job, line, temppkg, f1, 
								(char **)0))
				{
					l_free_conf(job, temppkg);
					temppkg = NULL;
					continue; /* bad package */
				}
				if (pkgs) temppkg->next = pkgs;
				pkgs = temppkg;
/*
 *				The next 2 lines were added for v6, but
 *				are they a bug in v5 also???
 */


				numfeat++;
				howbig += strlen(pkgs->feature) + 1;
				for(temppkg = pkgs->components; temppkg; 
							temppkg = temppkg->next)
				{
					numfeat++;
					howbig += strlen(temppkg->feature) + 1;
				}
			}
			else
			{
				numfeat++;
				if (isdec) howbig += strlen(line) + 1;
				else howbig += strlen(f2) + 1;
			}
	    }
	}
	if (numfeat > 0)
	{
	    (void) l_lfseek(lf, (long) 0, L_SET);
	    job->feat_ptrs = (char **) l_malloc(job, (numfeat+1) * sizeof(char *));
	    job->features = (char *) l_malloc(job, (unsigned) howbig);
	    curptr = job->features;
	    while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
	    {
 	      CONFIG *c = (CONFIG *)l_malloc(job, sizeof (CONFIG));

/* 
 *		If it's an INCREMENT line, we can't be LITE 
 *		We don't want PACKAGE lines here -- we've already got them
 *		in pkgs, and they need to be expanded.
 */

		if (l_parse_feature_line(job, line, c, 0) && c->type != CONFIG_PACKAGE)
		{				/* Found one */
			cur++;
			job->feat_ptrs[cur] = curptr;
			strcpy(job->feat_ptrs[cur], c->feature);
			curptr += strlen(c->feature) + 1;
		}
		if (c)
		{
			l_free_conf(job, c);
			c = NULL;
		}
	    }
	    job->feat_ptrs[cur+1] = NULL;
	}
	if (cur == -1)
	{
		LM_SET_ERRNO(job, LM_NOFEATURE, 163, 0);
	}

	free( f1 ); 
	/* look for PACKAGEs for this FEATURE */
	for (temppkg = pkgs; temppkg; 
			temppkg = temppkg->next)
	{
		/*if (l_keyword_eq(job, c->feature, temppkg->feature))*/
		{
			for (conf = temppkg->components;
				conf ; 
				conf = conf->next)
			{
			    cur++;
			    job->feat_ptrs[cur] = 
						curptr;
			    (void) strcpy(
				  job->feat_ptrs[cur],
				  conf->feature);
			    curptr += 
				  strlen(conf->feature) 
						  + 1;
			}
		}
			
	}
/*
 *	Free the packages matrix 
 */
	for (temppkg = pkgs; temppkg; temppkg = pkgs)
	{
		pkgs = temppkg->next;
		l_free_conf(job, temppkg);
		temppkg = NULL;
	}
		
	return(job->feat_ptrs);
}
static
void
l_remove_dups(job, dupaction)
LM_HANDLE *job;
void (*dupaction)();	/* Call when a duplicate is found */
{
  char **p1, **p2;
	for (p1 = job->feat_ptrs; p1 && *p1; p1++)
	{
		for (p2 = p1 + 1; *p2; )
		{
			if (l_keyword_eq(job, *p1, *p2)) /* P5423 */
			{
				l_remove_dup(job, p2);
				if (dupaction && 
					l_getattr(job, FULL_FLEXLM) == 
						FULL_FLEXLM_VAL)
					(*dupaction)(*p2, 0);
			}
			else p2++;
		}
	}
}
/*
 *	l_remove_dup
 *	there's 2 places we have to fixup:  job->feat_ptrs and job->features.
 *	We move the strings past the one to be removed to the left.
 */

static
void 
l_remove_dup(job, ptr)
LM_HANDLE *job;
char **ptr;
{
  char *tmp;
  int cnt, len, wlen = strlen(*ptr) + 1;

	for (len = 0, cnt = 1; ptr[cnt]; cnt++) 
		len += strlen(ptr[cnt]) + 1; 

	if (!len) 
	{
		*ptr = 0;
		return;
	}

	tmp = (char *)l_malloc(job, len);

	/* move the strings to the left 'len' */
	memcpy(tmp, ptr[1], len);
	memcpy(*ptr, tmp, len);
	for (cnt = 1; ptr[cnt]; cnt++)  /* fix up pointers */
		ptr[cnt] -= wlen;
	for (cnt = 1; ptr[cnt]; cnt++)  
		ptr[cnt] = ptr[cnt+1];
	free(tmp);
}
