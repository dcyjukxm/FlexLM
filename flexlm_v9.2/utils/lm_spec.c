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
 *	Module: $Id: lm_spec.c,v 1.4 2003/01/13 21:58:49 kmaclean Exp $
 *
 *	Function:	lc_featspec()
 *			lc_findfeat()
 *
 *	Description: 	routines for specifying a feature 
 *			with name:spec=value
 *
 *	Parameters:	featspec:	job, str, ret_feat, ret_spec
 *			findfeat:	job, feat, spec, last_conf
 *
 *	Return:		featspec:	void
 *			findfeat	CONFIG *
 *
 *	D. Birns
 *	8/24/96
 *
 *	Last changed:  12/8/98
 *
 */
#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif /* LM_INTERNAL */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
void
lc_featspec(job, str, ret_feat, ret_spec)
LM_HANDLE *job;
char *str;
char **ret_feat;
char **ret_spec;
{
        *ret_spec = '\0';
        *ret_feat = str;
        for(;*str;str++)
        {
                if (*str == ':')
                {
                        *str = '\0'; /* null terminate feature */
                        *ret_spec = str+1;
                        return;
                }
        }
}
CONFIG *
lc_findfeat(job, feat, spec, last)
LM_HANDLE *job;
char *feat;
char *spec;
CONFIG *last;
{
  CONFIG *f, *start = (last ? last->next : job->line);
  char buf[MAX_CONFIG_LINE];
  char *field, *value = 0, *cp;
  int quote = 0;
 
        if (spec)
        {
                strncpy(buf, spec, MAX_CONFIG_LINE);
                field = buf;
                for (cp = field; *cp; cp++)
                {
                        if (*cp == '=')
                        {
                                *cp = '\0'; /* null terminate field */
                                value = cp+1;
                                continue;
                        }
                        if (value == cp && *cp == '"') /* skip 1st quote */
                        {
                                quote = 1;
                                value++;
                                continue;
                        }
                        if (quote && *cp == '"')
                        {
                                *cp = '\0'; /* erase quote ending */
                                break;
                        }
                }
        }
        for (f = start; f; f = f->next)
        {
                if (!strcmp(f->feature, feat) &&
                        (!last || strcmp(f->code, last->code)) )
                {
                  HOSTID *hid;
 
                        if (!spec) return(f);
                        if (!strcmp(field, "VERSION") &&
                                !l_compare_version(job, f->version, value))
                                return f;
                        if  (!strcmp(field, "HOSTID"))
                        {
                          int rc;
                                l_get_id(job, &hid, value);
                                rc = l_hostid_cmp(job, f->idptr, hid);
                                free(hid);
                                if (!rc) return f; /* match */
                        }

			if (
				(!strcmp(field, "EXPDATE") &&
				!strcmp(f->date, value)) ||

				(!strcmp(field, LM_LICENSE_KEY) &&
				!strcmp(f->code, value)) ||

				(!strcmp(field, "VENDOR_STRING") &&
				!lc_xstrcmp(job,
					f->lc_vendor_def, value)) ||

				(!strcmp(field, "dist_info") &&
				!lc_xstrcmp(job,
					f->lc_dist_info, value)) ||

				(!strcmp(field, "user_info") &&
				!lc_xstrcmp(job,
					f->lc_user_info, value)) ||

				(!strcmp(field, "asset_info") &&
				!lc_xstrcmp(job,
					f->lc_asset_info, value)) ||

				(!strcmp(field, "ISSUER") &&
				!lc_xstrcmp(job,
					f->lc_issuer, value)) ||

				(!strcmp(field, "NOTICE") &&
				!lc_xstrcmp(job,
					f->lc_notice, value))
				)

			{
				return f;
			}
                }
                else
                {
/*
 *                      See if this is part of the package...
 */
			if (f->parent_feat &&
                                !strcmp(f->parent_feat->feature, feat))
                                return f;
                }
        }
        return 0;
}

