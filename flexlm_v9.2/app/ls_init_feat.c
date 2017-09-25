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
 *  Module: $Id: ls_init_feat.c,v 1.61.2.3 2003/06/25 20:53:36 sluu Exp $
 *
 *      Function: ls_init_feat(master_list)
 *
 *      Description: Initialize the feature table for the server
 *
 *      Parameters:     ls_flist - The feature table.
 *                      (LM_SERVER *) master_list - The servers
 *
 *      Return:         ls_flist filled in with the number of licenses available
 *
 *
 *      M. Christiano
 *      2/15/88
 *
 *  Last changed:  1/6/99
 *
 */
/*-
 *      The order of ls_flist is important, and will be more in the
 *      future.  Currently, the order is:
 *              1) SUITE parents 2) all features of a given name in
 *              the order they appear in the license file
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "l_openf.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "flexevent.h"

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


FEATURE_LIST fnull = {  (FEATURE_LIST *) NULL, (FEATURE_LIST *) NULL,
                        (FEATURE_LIST *) NULL,
                        (USERLIST *) NULL, (USERLIST *) NULL,
                        (OPTIONS *) NULL, (OPTIONS *) NULL, (OPTIONS *) NULL,
                        (OPTIONS *) NULL, (OPTIONS *) NULL, (OPTIONS *)0, (OPTIONS *)0, 0,
                        (char *) NULL, "0.0", "0", (char *) NULL, "0",
                        (OLD_CODE_LIST *)NULL, 0, 0, 0, (HOSTID *) NULL,
                        0, 0, 0, 0, 0, 0
                     };
FEATURE_LIST *ls_flist = &fnull;
VENDORCODE *ls_keys;
static FEATURE_LIST *ls_increment();
static FEATURE_LIST *ls_upgrade();
static int same_platforms lm_args(( char **, char **));
int ls_init_log_wrap = 1;
int ls_init_log_unsupp = 1;

#define strsave(x)   (void) strcpy(strptr = LS_MALLOC(strlen(x)+1), x)
static char *strptr;

static ls_duplicate_feature();
extern int ls_use_all_feature_lines;
extern int ls_compare_vendor_on_increment;
extern int ls_compare_vendor_on_upgrade;
extern int ls_enforce_startdate;
extern int ls_tell_startdate;
static CONFIG *ls_feat_validate lm_args((char *, LM_SERVER *, int)) ;

static int ls_feat_init lm_args(( FEATURE_LIST *, char *, int, char *, char *,
                                char *, char *, HOSTID *, int, int,
                                FEATURE_LIST *, CONFIG *));
extern int keysize;
void ls_print_feats();
static int ls_add_support lm_args((char **, CONFIG *, FEATURE_LIST *));
static int ls_enable_pkg lm_args((CONFIG *, FEATURE_LIST *, LM_SERVER *,
                                                FEATURE_LIST **));
static void ls_create_feats lm_args(( char **, LM_SERVER *, FEATURE_LIST **,
                                                        int));
static int good_config lm_args((LM_HANDLE *, CONFIG *, LM_SERVER *));
static void ls_pooled lm_args((FEATURE_LIST *, CONFIG_PTR));
static void ls_pooled_subtract lm_args((FEATURE_LIST *, int));
extern int ls_flexlmd;
#define CREATE_SUITES 1
#define CREATE_FEATURES 2

/*-
 *      We have our own private copies of many functions, as a way
 *      to thwart theft
 */
#define l_crypt_private l_ckout_crypt
#define l_string_key    l_ckout_string_key
static LM_CHAR_PTR     l_ckout_crypt lm_args((LM_HANDLE_PTR job,
                                      CONFIG_PTR conf, LM_CHAR_PTR sdate,
                                              VENDORCODE_PTR code));
static LM_U_CHAR_PTR API_ENTRY
                l_ckout_string_key      lm_args((LM_HANDLE_PTR,
                                        LM_U_CHAR_PTR, int,
                                 VENDORCODE_PTR, unsigned long, char *));

static
void
sSaveSuiteDupGroupSettings()
{
  FEATURE_LIST * pFeature = ls_flist;

  while(pFeature)
  {
    if(pFeature->conf && pFeature->conf->lc_got_options & LM_LICENSE_SUITE_DUP_PRESENT)
    {
      pFeature->suite_dup_group = pFeature->conf->lc_suite_dup;
    }
    else
    {
      pFeature->suite_dup_group = 0;
    }
    pFeature = pFeature->next;
  }
}

void
ls_init_feat(LM_SERVER * master_list)
{
  char **features;
  FEATURE_LIST  *fsave = 0, *f;

  DEBUG_INIT
/*
 *      Get all features from file
 */

  features = lc_feat_list(lm_job, LM_FLIST_ALL_FILES, 0);
  f = ls_flist = (FEATURE_LIST *) LS_MALLOC(sizeof(FEATURE_LIST));
  memset(ls_flist, 0, sizeof(FEATURE_LIST));
  ls_feat_prune();
  l_clear_error(lm_job); /* P4683 */
  ls_create_feats(features, master_list, &f, CREATE_SUITES);
  ls_create_feats(features, master_list, &f, CREATE_FEATURES);
  sSaveSuiteDupGroupSettings();
  if (f)
  {
    fsave = f->last;
    memcpy(f, &fnull, sizeof(FEATURE_LIST));
    f->last = fsave;
  }
  ls_print_feats();
}

static
void
ls_create_feats(
  char **     features,
  LM_SERVER *   master_list,
  FEATURE_LIST ** flist,
  int       mode)
{
  char **	feature = NULL;
  char		szBuffer[MAX_PATH] = {'\0'};
  char *	ppszInStr[20] = {NULL};
  struct config *conf;

    for (feature = features; feature && *feature; feature++)
    {
    ls_feat_start_validate();
    while (conf = ls_feat_validate(*feature, master_list, mode))
    {
      if ((mode == CREATE_SUITES && !(conf->package_mask & LM_LICENSE_PKG_ENABLE)) ||
        (mode == CREATE_FEATURES && (conf->package_mask & LM_LICENSE_PKG_ENABLE)))
      {
        continue;
      }
/*
 *                      Warn if it will expire within 2 weeks
 */
            if (!l_date(lm_job, conf->date, L_DATE_EXPIRED) &&
        (lc_expire_days(lm_job, conf) < 15) )
      {
        LOG((lmtext("Warning: %s expires %s\n"), conf->feature, conf->date));

		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = conf->feature;
			ppszInStr[1] = conf->date;

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_WARN,
								CAT_FLEXLM_LICENSE_FILE,
								MSG_FLEXLM_NEAR_EXPIRED,
								2,
								ppszInStr,
								0,
								NULL);
		}
      }

/*
 *                      If we are using all feature lines and this is a FEATURE
 *                      line, simply force this to be an INCREMENT line!
 */
      if (ls_use_all_feature_lines &&
            (conf->type == CONFIG_FEATURE))
      {
        conf->type = CONFIG_INCREMENT;
        conf->lc_options_mask |= LM_OPT_ISFEAT;
      }
            if (conf->type == CONFIG_FEATURE)
            {
/*
 *                              First, check to see if this feature is already
 *                              here ... walk the preceeding feature list
 *                              looking for a match.
 */
        if (!ls_duplicate_feature(feature, *flist, conf, 1))
        {
/*
 *                                      This is a new feature, put it in the
 *                                      table
 */
          if (!ls_add_support(feature, conf, *flist))
            *flist = (*flist)->next;
        }
      }
      else if (conf->type == CONFIG_INCREMENT)
/*
 *                      INCREMENT line.  Find the last feature that matches,
 *                      if it exists, then increment it.
 */
            {
        *flist = ls_increment(feature, conf, *flist);
            }
      else if (conf->type == CONFIG_UPGRADE)
      {
/*
 *                      UPGRADE line.  Find the last feature that matches,
 *                      if it exists, then upgrade it and remove the upgraded
 *                      licenses.
 */
        *flist = ls_upgrade(feature, conf, *flist);
      }
      else
      {
       LOG((lmtext("Feature \"%s\": unsupported license file type %d\n")
          , conf->feature, conf->type));
      }
    }
  }
}

static int foundone;
static CONFIG * filepos;

/*
 *      Start ls_feat_validate at the beginning of the license file
 *      Also, throw out any feature/increment lines that are duplicates.
 */

void
ls_feat_start_validate()
{
    foundone = 0;
    filepos = 0L;
}

void
ls_feat_prune()
{
  CONFIG *line, *comp, *delete, *keep, *prevline = 0, *prevcomp = 0, *prev;

  line = lm_job->line;
  while (line)
  {
    for (comp = lm_job->line; comp && comp != line; comp = comp->next)
    {
/*
 *                      1) If they're both components, check their enabling
 *                         feat's license-keys
 *                      2) If neither is a component, compare their license
 *                         keys.
 *                      3) if one is a component and the other isn't, skip it
 */
      if (((comp->package_mask & LM_LICENSE_PKG_COMPONENT) &&
        (line->package_mask & LM_LICENSE_PKG_COMPONENT) &&
        l_keyword_eq(lm_job, comp->feature, line->feature) &&
        (comp->parent_feat && line->parent_feat &&
        l_keyword_eq(lm_job, comp->parent_feat->code, line->parent_feat->code))) ||
        (!(comp->package_mask & LM_LICENSE_PKG_COMPONENT) &&
        !(line->package_mask & LM_LICENSE_PKG_COMPONENT) &&
        l_keyword_eq_n(lm_job, comp->hash, line->hash, MAX_CRYPT_LEN)))
      {
        char a[MAX_CONFIG_LINE+1];
/*
 *                              Decide which one to delete
 *                              save earliest in file
 */
        if (comp->file_order < line->file_order)
        {
          delete = line;
          prev = prevline;
          keep = comp;
          line = prev;
        }
        else
        {
          delete = comp;
          prev = prevcomp;
          keep = line;
          comp = prev;
        }


        if (l_keyword_eq(lm_job, delete->daemon, lm_job->vendor))
        {
          l_print_config(lm_job, delete, a);
          strcpy(&a[60], " [...]");
          LOG((lmtext("WARNING: duplicate license file lines:\n   %s\n"), a));
        }
        if (prev)
        {
          if (delete->next)
            delete->next->last = prev;
        }
        else
          line = lm_job->line = delete->next;

        if (line->package_mask & LM_LICENSE_PKG_ENABLE)
        {
          CONFIG *comp1;
/*                                      It's  an enabling feature line,
 *                                      a component feature would have a pointer
 *                                      to it as "parent_feat" -- we have
 *                                      to change any that do from  line to comp
 */
          for (comp1 = lm_job->line; comp1; comp1 = comp1->next)
          {
            if (comp1->parent_feat == delete)
              comp1->parent_feat = keep;
          }
        }
                l_free_conf(lm_job, delete);  /* free it */
                break;

      }
      prevcomp = comp;
    }
    prevline = line;
    line = line->next;
  }
}

/*
 *      Validate the requested feature.  If it is OK, return the license
 *      file line for it.
 */

static
CONFIG *
ls_feat_validate(
  char *    feature,
  LM_SERVER * master_list,
  int     mode)
{
  struct config *conf = (struct config *) NULL;
  long  codedate = 0, curdate = 0;
  char *	bindate;
  char *	ppszInStr[20] = {NULL};


    while (1)
    {
    while (conf = l_next_conf_or_marker(lm_job, feature, &filepos, 0, 0))
    {
/*
 *      This if test was added in v8, and cuts
 *      reading the license time in half
 */
            if ( (  mode == CREATE_SUITES && !(conf->package_mask & LM_LICENSE_PKG_ENABLE)) ||
                    (mode == CREATE_FEATURES && (conf->package_mask & LM_LICENSE_PKG_ENABLE)))
            {
        continue;
            }

            if (foundone && !ls_use_all_feature_lines
                 && (conf->type == CONFIG_FEATURE))
            {
        if ( mode == CREATE_SUITES )
        {
		  ppszInStr[0] = conf->feature;
		  ppszInStr[1] = conf->feature;
		  ppszInStr[2] = conf->code;

          LOG((lmtext("Error: Duplicate \"%s\" feature line found\n"),
              conf->feature));
          LOG((lmtext("\t->\"%s\", key=%s skipped.\n"),
              conf->feature, conf->code));

        }

		}
	else if (!foundone || (conf->type == CONFIG_INCREMENT) ||
        (conf->type == CONFIG_UPGRADE) ||
        ls_use_all_feature_lines)
      {
        break;
      }
    }
    if (!conf)
      break;                       /* @ end of list */

    if (strcmp(conf->daemon, lm_job->vendor))
      continue;
                                                        /* Not for us */
        if (((conf->package_mask == LM_LICENSE_PKG_COMPONENT) &&
      (!good_config(lm_job, conf->parent_pkg, master_list) ||
      !good_config(lm_job, conf->parent_feat, master_list))) ||
            ((conf->package_mask != LM_LICENSE_PKG_COMPONENT) &&
      !good_config(lm_job, conf, master_list)))
    {
/*
 *                              The following test for CREATE_SUITES is
 *                              only so we don't print this message on
 *                              every pass
 */

                        /*if (mode == CREATE_SUITES) P6177*/
      {
        int notwrap;

        if (notwrap = strcmp(feature, "FLEXwrap"))
          notwrap = strcmp(feature, "SAMwrap");

        if ((ls_init_log_wrap && ls_init_log_unsupp) ||
          (notwrap && ls_init_log_unsupp) ||
          (!notwrap && ls_init_log_wrap))
        {

          if (conf->lc_future_minor)
          {
            LOG((lmtext("Future file format/Invalid license key for \"%s\", perhaps typed in wrong)\n"),
              feature));

          }
          else if (conf->users != -1)
          {
            char a[MAX_CONFIG_LINE+1];
            l_print_config(lm_job, conf, a);
            /* kmaclean 2/10/03
             * don't cut the line off at 60 chars */
            /* strcpy(&a[60], " [...]"); */
            LOG((lmtext("Invalid license key (inconsistent authentication code)\n")));
            LOG(("\t==>%s\n", a));

			if(l_flexEventLogIsEnabled())
			{
				ppszInStr[0] = a;

				l_flexEventLogWrite(lm_job,
									FLEXEVENT_WARN,
									CAT_FLEXLM_LICENSE_FILE,
									MSG_FLEXLM_BAD_AUTHCODE,
									1,
									ppszInStr,
									0,
									NULL);
			}
          }
        }
      }
            continue;
    }
/*
 *              Make sure we haven't expired
 */
        if (l_date(lm_job, conf->date, L_DATE_EXPIRED)) /* EXPIRED */
        {
			ppszInStr[0] = feature;

			if(l_flexEventLogIsEnabled())
			{
				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LICENSE_FILE,
									MSG_FLEXLM_LICENSE_EXPIRED,
									1,
									ppszInStr,
									0,
									NULL);
			}

          LOG((lmtext("EXPIRED: %s\n"), feature));
          continue;
        }
/*
 *              Now make sure our current date is later than the date this
 *              feature starts (with 1 day slop).
 */
    bindate = l_extract_date(lm_job, conf->code);

    if (bindate && (ls_user_crypt == NULL) &&
      ls_enforce_startdate &&
      (l_getattr(lm_job, START_DATE) == START_DATE_VAL))
    {

      sscanf(l_extract_date(lm_job, &(conf->code[0])), "%x", &codedate);  /* overrun checked */
      sscanf(l_bin_date(0), "%x", &curdate);  /* overrun checked */
      if (curdate < codedate - 1)
      {
        if (ls_tell_startdate)
        {
			ppszInStr[0] = feature;

			if(l_flexEventLogIsEnabled())
			{
				l_flexEventLogWrite(lm_job,
									FLEXEVENT_INFO,
									CAT_FLEXLM_LICENSE_FILE,
									MSG_FLEXLM_NOT_READY,
									1,
									ppszInStr,
									0,
									NULL);
			}

          LOG((lmtext("Feature %s is not enabled yet\n"), feature));
          LOG_INFO((INFORM, "The current date is \
                earlier than the start date \
                for this feature."));
        }
        continue;
      }
    }
    /* P3949 */
    if (*conf->startdate && l_date(lm_job, conf->startdate, L_DATE_START_OK))
    {
		LOG((lmtext("Feature %s is not enabled yet, starts on %s\n"),
		  feature, conf->startdate));

		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = feature;
			ppszInStr[1] = conf->startdate;

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LICENSE_FILE,
								MSG_FLEXLM_NOT_READY_YET,
								2,
								ppszInStr,
								0,
								NULL);
		}
		continue; /* Not started yet */
    }
    if (conf->users /*|| (conf->lc_type_mask & LM_TYPE_FLOAT_OK)*/)
      foundone = 1; /* don't consider uncounted! */
    break;
  }
    return(conf);
}


/*
 *      Initialize a feature
 */
static
int
ls_feat_init(
  FEATURE_LIST *  f,
  char *      name,
  int       nlic,
  char *      version,
  char *      code,
  char *      expdate,
  char *      vendor_def,
  HOSTID *    id,
  int       overdraft,
  int       dup,
  FEATURE_LIST *  package,
  CONFIG *    conf)
{
  char *vdef, *hid;

/*
 *      First, the "static" stuff from the license file
 */

  if (!f)
  {
    if (!lm_job->lm_errno)
    {
      LM_SET_ERRNO(lm_job, LM_BADPARAM, 511, 0)
    }
    return lm_job->lm_errno;
  }

  f->feature = name;
  f->nlic = nlic + overdraft;

  if(f->nlic == 0)
    f->flags |= LM_FL_FLAG_UNCOUNTED;
  else
    f->flags &= ~LM_FL_FLAG_UNCOUNTED;

  f->lowwater = 0;
  f->res = 0;
  f->overdraft = overdraft;
  strncpy(f->version, version, MAX_VER_LEN);
  f->version[MAX_VER_LEN] = '\0';
  f->maxborrow = 0;
  (void) l_zcp(&(f->expdate[0]), expdate, DATE_LEN);
    if (vendor_def)
    {
    strsave(vendor_def);
    vdef = strptr;
    }
    else
  vdef = (char *) NULL;
    f->vendor_def = vdef;
    (void) strncpy(&(f->code[0]), code, MAX_CRYPT_LEN);
    f->code[MAX_CRYPT_LEN] = '\0';
    hid = l_asc_hostid(lm_job, id);
  if (hid)
  {
    strsave(hid);
    hid = strptr;
  }
  f->id = hid;
  if (dup >= 0)
  {
    f->dup_select = dup;
    f->sticky_dup = 1;
  }
  else
  {
    f->dup_select = 0;
    f->sticky_dup = 0;
  }
/*
 *      Next, the variable stuff
 */
  f->timeout = 0;                 /* No timeout */
  f->linger = 0;                  /* No linger */
  f->u = (USERLIST *) NULL;
  f->queue = (USERLIST *) NULL;
  f->opt = (OPTIONS *) NULL;
  f->include = (OPTIONS *) NULL;
  f->exclude = (OPTIONS *) NULL;
  f->b_include = (OPTIONS *) NULL;
  f->b_exclude = (OPTIONS *) NULL;
  f->package = package;
  f->type_mask = conf->lc_type_mask;
  f->user_based = conf->lc_user_based;
  f->minimum = conf->lc_minimum;
  f->host_based = conf->lc_host_based;
  f->conf_dup_group = conf->lc_dup_group;
  if (conf->lc_platforms) /* carefully copy the memory */
  {
    char **cp1, **cp2;
    char *bufp;
    int cnt, len;

    for (cnt=0, len=0, cp1 = conf->lc_platforms; *cp1; cp1++, cnt++)
    {
      len += strlen(*cp1) + 1;
    }
    f->platforms = (char **)LS_MALLOC((cnt + 1) * sizeof(char *));
    bufp = *f->platforms = (char *)LS_MALLOC(len + 1);
    for (cnt = 0, cp1 = conf->lc_platforms; *cp1; cp1++, bufp += strlen(bufp) + 1, cnt++)
    {
      strcpy(f->platforms[cnt] = bufp, *cp1);
    }
  }

  f->conf = conf;

/*
 *  Set package flags -- 4 cases, or 0
 */
  if (conf->package_mask & LM_LICENSE_PKG_SUITE)
    f->flags |= LM_FL_FLAG_SUITE_PARENT;
  else if (conf->package_mask & LM_LICENSE_PKG_BUNDLE)
    f->flags |= LM_FL_FLAG_BUNDLE_PARENT;
  else if (conf->package_mask & LM_LICENSE_PKG_ENABLE)
    f->flags |= LM_FL_FLAG_NONSUITE_PARENT;
  else if (conf->package_mask & LM_LICENSE_PKG_COMPONENT)
  {
    if (package && package->conf && (package->conf->package_mask & LM_LICENSE_PKG_SUITE))
      f->flags |= LM_FL_FLAG_SUITE_COMPONENT;
    else if (package && package->conf && (package->conf->package_mask & LM_LICENSE_PKG_BUNDLE))
      f->flags |= LM_FL_FLAG_BUNDLE_COMPONENT;
    else
      f->flags |= LM_FL_FLAG_NONSUITE_COMPONENT;
  }
  ls_pooled(f, conf);
  return 0; /* success */
}

/*
 *      ls_pooled -- add this date to the list for this feature list
 */
static
void
ls_pooled(
  FEATURE_LIST *  fl,
  CONFIG *    conf)
{
  LS_POOLED *pooled;

    pooled = (LS_POOLED *)LS_MALLOC(sizeof (LS_POOLED));
    memset(pooled, 0, sizeof (LS_POOLED));
    pooled->next = fl->pooled;
    fl->pooled =  pooled;
/*
 *      set the values
 */
    pooled->users = conf->users ? conf->users : 1;
    strcpy(pooled->date, conf->date);
    strcpy(pooled->code, conf->code);
    if (conf->lc_sign)
  {
    strsave(conf->lc_sign);
    pooled->lc_sign = strptr;
  }
    if (conf->lc_vendor_def)
    {
        strsave(conf->lc_vendor_def);
        pooled->lc_vendor_def = strptr;
    }
    if (conf->lc_dist_info)
    {
        strsave(conf->lc_dist_info);
        pooled->lc_dist_info = strptr;
    }
  if (conf->lc_user_info)
  {
    strsave(conf->lc_user_info);
    pooled->lc_user_info = strptr;
  }
  if (conf->lc_asset_info)
  {
    strsave(conf->lc_asset_info);
    pooled->lc_asset_info = strptr;
  }
  if (conf->lc_issuer)
  {
    strsave(conf->lc_issuer);
    pooled->lc_issuer = strptr;
  }
  if (conf->lc_notice)
  {
    strsave(conf->lc_notice);
    pooled->lc_notice = strptr;
  }
  if (conf->parent_feat)
  {
    strsave(conf->parent_feat->feature);
    pooled->parent_featname = strptr;
    strsave(conf->parent_feat->code);
    pooled->parent_featkey = strptr;
  }
}

/*
 *      ls_pooled_subtract -- used by ls_upgrade to decrement
 */
static
void
ls_pooled_subtract(
  FEATURE_LIST *  fl,
  int       cnt)
{
  LS_POOLED *el;

  for (el = fl->pooled; el && cnt; el = el->next)
  {
    if (el->users > cnt)
    {
      el->users -= cnt;
      cnt = 0;
    }
    else
    {
      cnt -= el->users;
      el->users = 0;
    }
  }
}

void
ls_pooled_check(FEATURE_LIST * fl)
{
  FEATURE_LIST *flp;
  LS_POOLED *el;
  char *	ppszInStr[20] = {NULL};

    for (flp = fl; flp; flp = flp->next)
    {
        if (!flp->feature)
      continue;
        if (l_date(lm_job, flp->expdate, L_DATE_EXPIRED))
        {
			ppszInStr[0] = flp->feature;
			ppszInStr[1] = flp->expdate;

			if(l_flexEventLogIsEnabled())
			{
				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LICENSE_FILE,
									MSG_FLEXLM_LICENSE_EXPIRED_ONREREAD,
									2,
									ppszInStr,
									0,
									NULL);
			}
       /* Turn Event logging on   */

            LOG(("%s expired -- rereading to update\n",
                            flp->feature));
            ls_reread();
            return;
        }
        for (el = flp->pooled; el; el = el->next)
        {
      if (l_date(lm_job, el->date, L_DATE_EXPIRED))
      {
		  if(l_flexEventLogIsEnabled())
		  {
			  ppszInStr[0] = flp->feature;
			  ppszInStr[1] = el->date;
			  l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LICENSE_FILE,
								MSG_FLEXLM_LICENSE_EXPIRED_ONREREAD,
								2,
								ppszInStr,
								0,
								NULL);
		  }

        LOG(("%s expired -- rereading to update\n",
                flp->feature));
        ls_reread();
        return;
      }
        }
    }
}

/*
 *      ls_add_support() - add support for this feature
 */

static
int
ls_add_support(
  char **     feature,
  CONFIG *    conf,
  FEATURE_LIST *  f)
{
  int flsize = sizeof(FEATURE_LIST);
  int dup = -1;
  FEATURE_LIST *pkg = (FEATURE_LIST *)0; /* defaults to not parent nor
                                            component */


  strsave(*feature);
  if (conf->parent_feat )
  {
    pkg = f_get_feat(conf->parent_feat->feature, conf->parent_feat->code, 0);
  }
  else if (conf->package_mask & LM_LICENSE_PKG_ENABLE)
  {
    pkg = LM_PKG_PARENT;
    if (conf->lc_got_options & LM_LICENSE_SUITE_DUP_PRESENT)
      dup = conf->lc_suite_dup;
  }

  if (dup == -1 && conf->lc_got_options & LM_LICENSE_DUP_PRESENT)
    dup = conf->lc_dup_group;

  if (!ls_feat_init(f, strptr, (conf->lc_type_mask & LM_TYPE_FLOAT_OK) ? 1 : conf->users,
    conf->version, &(conf->code[0]), conf->date, conf->lc_vendor_def,
    conf->idptr, conf->lc_overdraft, dup, pkg, conf))
  {
    f->next = (FEATURE_LIST *) LS_MALLOC(flsize);
    (void)memset((char *)f->next, 0, flsize);
    f->next->feature = (char *) NULL;
    f->next->last = f;      /* Back link */
    return lm_job->lm_errno;
  }
  else
    free(strptr);
  return lm_job->lm_errno;
}

/*
 *
 */
static
ls_duplicate_feature(
  char **     feature,
  FEATURE_LIST *  f,
  CONFIG *    conf,
  int       print)  /* "print duplicate feature" flag */
{
  FEATURE_LIST *x;
  int dup = 0;
  char okey[MAX_CRYPT_LEN + 1];

  if (!f)
    return 0;
    for (x = f->last; x && x->feature; x = x->last)
    {
    if (!strcmp(x->feature, *feature))
    {
      if ((!ls_use_all_feature_lines ||
        !strcmp(x->code, conf->code)) &&
        !(x->flags & LM_FL_FLAG_UNCOUNTED) && conf->users )
      {
        if (print)
        {
          LOG((lmtext(
      "WARNING: Duplicate \"%s\" (%s) feature line found, not using this line.\n"),
                    *feature, x->code));
        }
        dup = 1;
        break;
      }
    }
    }
    return(dup);
}

static
int
same_platforms(
  char ** list1,
  char ** list2)
{
  char **cp1, **cp2;

    if (!list1 && !list2)
    return 1;
    if (!list1 || !list2)
    return 0;
    for (cp1 = list1, cp2 = list2; *cp1 && *cp2; cp1++, cp2++)
  {
    if (strcmp(*cp1, *cp2))
      return 0;
  }
    if (*cp1 || *cp2)
    return 0;
    return 1;
}

/*
 *      ls_increment() - Process the INCREMENT line
 */
static
FEATURE_LIST *
ls_increment(
  char **     feature,
  CONFIG *    conf,
  FEATURE_LIST *  f)
{
  int haveone = 0;
  FEATURE_LIST *l;

  if (l_date(lm_job, conf->date, L_DATE_EXPIRED))
    return f;

  if (conf->lc_type_mask & LM_TYPE_METER)
    return 0;
  for (l = f; l; l = l->last)
  {
    if  (ls_pool(l, conf))
    {
/*
 *                THEY match.  INCREMENT the feature.
 */
      l->nlic += conf->users + conf->lc_overdraft;
      l->overdraft += conf->lc_overdraft;
      l->type_mask |= conf->lc_type_mask;
      l->user_based += conf->lc_user_based;
      l->host_based += conf->lc_host_based;
      l->minimum += conf->lc_minimum;
      ls_pooled(l, conf);
      haveone = 1;
      break;
    }
  }
  if (haveone == 0)
  {
    ls_add_support(feature, conf, f);
    f = f->next;
  }
  return(f);
}

static
int
sUpgradeThisOne(
  CONFIG *    conf,
  FEATURE_LIST *  pFeat)
{
  int   iRet = 0;
  if(pFeat)
  {
    if (((pFeat->feature && l_keyword_eq(lm_job, conf->feature, pFeat->feature)) &&
      (l_compare_version(lm_job, conf->fromversion, pFeat->version) <= 0) &&
      (l_compare_version(lm_job, conf->version, pFeat->version) > 0) &&
      (!ls_compare_vendor_on_upgrade ||
      (l_keyword_eq(lm_job, conf->lc_vendor_def, pFeat->vendor_def)))))
    {
      iRet = 1;
    }
  }
  return iRet;
}

static
FEATURE_LIST *
sProcessUpgrade(
  char **     feature,
  CONFIG *    conf,
  FEATURE_LIST *  pFeat,
  FEATURE_LIST *  f,
  int *     pFoundOne)
{
  FEATURE_LIST *  pEntry = NULL;
  int       bDone = 0;

  conf->upgrade_count = conf->upgrade_remaining = conf->users;

  for(pEntry = pFeat; !bDone && pEntry; pEntry = pEntry->next)
  {
    if(sUpgradeThisOne(conf, pEntry))
    {
      ls_i_fhostid(pEntry); /* only do this if necessary to save time */

      /*
       *  Process the upgrade for this entry.
       */
      if (!l_hostid_cmp(lm_job, conf->idptr, pEntry->hostid))
        continue;
      /*
       *  THEY match.  UPGRADE the feature.
       */
      if (pEntry->nlic <= 0)
      {
        LOG((lmtext("Prior INCREMENT line for UPGRADE (%s:%s->%s) has no licenses\n"),
          conf->feature, conf->fromversion, conf->version));
        continue;
      }

      *pFoundOne = 1;

      if(conf->upgrade_remaining > pEntry->nlic)
      {
        /*
         *  Have more upgrades than licenses for this user.
         */
        ls_pooled_subtract(pEntry, pEntry->nlic);
        conf->upgrade_remaining = conf->upgrade_remaining - pEntry->nlic;
        LOG((lmtext("UPGRADING %d \"%s\" from version %s to version %s\n")
              , pEntry->nlic, pEntry->feature, pEntry->version, conf->version));
        pEntry->nlic = 0;
      }
      else
      {
        ls_pooled_subtract(pEntry, conf->upgrade_remaining);
        pEntry->nlic -= conf->upgrade_remaining;
        LOG((lmtext("UPGRADING %d \"%s\" from version %s to version %s\n")
              , conf->upgrade_remaining, pEntry->feature, pEntry->version, conf->version));
        conf->upgrade_remaining = 0;
        break;
      }
    }
    else
      break;

  }
  /*
   *  Add support for it.
   */
  conf->users = conf->upgrade_count - conf->upgrade_remaining;
  ls_add_support(feature, conf, f);
  return f->next;
}


/*
 *      ls_upgrade() - Process the UPGRADE line
 */
static
FEATURE_LIST *
ls_upgrade(
  char **     feature,
  CONFIG *    conf,
  FEATURE_LIST *  f)
{
  int haveone = 0;
  FEATURE_LIST *l;
  char *	ppszInStr[20] = {NULL};

  if (l_date(lm_job, conf->date, L_DATE_EXPIRED))
    return f;

    for (l = f; l; l = l->last)
    {
    int doit = 0;

    /*
     *  Determine the lowest version that we can upgrade.
     */
    if(!sUpgradeThisOne(conf, l))
      continue;
    if(sUpgradeThisOne(conf, l->last))
      continue;

    ls_i_fhostid(l); /* only do this if necessary to save time */

      if (!l_hostid_cmp(lm_job, conf->idptr, l->hostid))
      continue;
    /*
     *  THEY match.  UPGRADE the feature.
     */

    f = sProcessUpgrade(feature, conf, l, f, &haveone);
    break;
    }
  if (haveone == 0)
  {
    LOG((lmtext("Warning: no prior INCREMENT line for UPGRADE (%s:%s->%s)\n"),
      conf->feature, conf->fromversion, conf->version));

	if(l_flexEventLogIsEnabled())
	{
		ppszInStr[0] = conf->feature;
		ppszInStr[1] = conf->fromversion;
		ppszInStr[2] = conf->version;
		l_flexEventLogWrite(lm_job,
							FLEXEVENT_WARN,
							CAT_FLEXLM_LICENSE_FILE,
							MSG_FLEXLM_LICENSE_NOINCREMENTLINE_ONUPGRADE,
							3,
							ppszInStr,
							0,
							NULL);
	}
  }
  return(f);
}

/*
 *      good_config-- re-encrypt the data, compare to the code in the file.
 *  Return:  1 == success, 0 == failure
 */
static
int
good_config(
  LM_HANDLE *   job,
  CONFIG *    conf,
  LM_SERVER *   master_list)
{
  int i = 0;
  VENDORCODE vc, *key;
  unsigned long signature;
  char *code = "";
  char *name = ls_flexlmd ? "flexlmd" : lm_job->vendor;
  int result = 0;
  extern void (*L_UNIQ_KEY5_FUNC)();
  LM_KEYLIST *kl;

  if (!conf)
    return 0; /* P3948 */
    for (key = &vendorkeys[0]; !result && (i < keysize); i++, key++)
    {
/*
 *  We have to verify each license-key/signature
 */
    memcpy((char *)&vc, (char *)key, sizeof(vc));
    if (conf->L_CONF_FLAGS & L_CONF_FL_OLD_KEY)
    {
      job->L_SIGN_LEVEL = 0;
      if (!(result = l_good_lic_key(job, conf, key)))
        continue;
    }
    for(kl = conf->lc_keylist; kl; kl = kl->next)
    {
      job->L_SIGN_LEVEL = (char *)kl->sign_level;
      if (!(result = l_good_lic_key(job, conf, key)))
        break;
    }
  }
  return result;
}

/*
 *  ls_pool -- centralized function to determine if a CONFIG
 *      should be pooled with an existing FEATURELIST
 */
int
ls_pool(
  FEATURE_LIST *  l,
  CONFIG *    conf)
{
  char *l_id = (char *)0;
  char *conf_id = NULL;
  int both_not_suite_components = 0;
  int both_suite_parents = 0;
  int both_suite_components = 0;

  if(!conf)
    return 0; /* different pool */
  conf_id = l_asc_hostid(lm_job, conf->idptr);

  if (!conf_id)
    conf_id = "";
  l_id = l->id ? l->id : "";
/*-
 *              This is where duplicate grouping is decided.
 *              This if test is getting out of hand!
 */
  if (!(l->flags & LM_FL_FLAG_SUITE_COMPONENT || l->flags & LM_FL_FLAG_BUNDLE_COMPONENT)
      && !(conf->parent_feat &&
    (conf->parent_feat->package_mask &
      LM_LICENSE_PKG_SUITEBUNDLE)))
  {
    both_not_suite_components = 1;
  }
  else if ((l->flags & LM_FL_FLAG_SUITE_PARENT || l->flags & LM_FL_FLAG_BUNDLE_PARENT)
    && (conf->package_mask & LM_LICENSE_PKG_SUITEBUNDLE))
  {
    both_suite_parents = 1;
  }

  else if ((conf->parent_feat && (conf->parent_feat->package_mask &
    LM_LICENSE_PKG_SUITEBUNDLE)) &&
    (l->flags & LM_FL_FLAG_SUITE_COMPONENT || l->flags & LM_FL_FLAG_BUNDLE_COMPONENT) &&
    l->package && l->package->feature &&
    !strcmp(conf->parent_feat->feature, l->package->feature) &&
      !l_compare_version(lm_job, l->package->version, conf->parent_feat->version) )
  {
    both_suite_components = 1;
  }

  if (l->feature &&
    /* same names ... */
       !strcmp(conf->feature, l->feature) &&
    /* same versions ... */
      !l_compare_version(lm_job, conf->version, l->version) &&
    /* same server hostid -- P5505 */
      ((!l->conf && !conf->server) || (l->conf && ((l->conf->server == conf->server) ||
    (l->conf->server && conf->server &&
    l_hostid_cmp_exact(lm_job, l->conf->server->idptr,
            conf->server->idptr))))) &&
    /* same hostid ... */
      !strcmp(conf_id, l_id) &&
    /* same type_mask ... */
      (l->type_mask == conf->lc_type_mask) &&
    /* same dup_group ... */
      (l->conf_dup_group == conf->lc_dup_group) &&
    /* don't pool meter licenses ... */
      !(l->type_mask & LM_TYPE_METER) &&
    /* if compare_vendor, same vendor_def ... */
      (!ls_compare_vendor_on_increment ||
      !lc_xstrcmp(lm_job, conf->lc_vendor_def, l->vendor_def)) &&
    /* same platforms ... */
      same_platforms(conf->lc_platforms, l->platforms) &&
    /* same platforms ... */
      ( both_not_suite_components ||
        both_suite_parents ||
        both_suite_components ))
  {
    return 1; /* they should pool */
  }
  else
    return 0; /* They're different pools */
}

#define LM_CKOUT
#include "l_crypt.c"
#ifdef WINNT
#include "ls_svk.c"
#endif

