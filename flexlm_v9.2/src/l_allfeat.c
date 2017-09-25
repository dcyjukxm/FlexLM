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
 *	Module: $Id: l_allfeat.c,v 1.35 2003/05/05 16:10:54 sluu Exp $
 *
 *      Function: l_allfeat(job, lf, s, lfindex)
 *
 *      Description: Gets all the feature lines from the license file
 *
 *      Parameters:     (LM_HANDLE *) job - current job
 *                      (LICENSE_FILE *) lf - The license file to read.
 *                      (LM_SERVER *) s - The server to associate.
 *                      (int) lfindex - license file index
 *
 *      Return:         0 - No feature lines added
 *                      <> 0 - Feature lines found and malloc'ed into CONFIG 
 *                              structs and tacked onto the end of 
 *                              job->line.
 *
 *      M. Christiano
 *      6/21/90
 *
 *	Last changed:  10/27/98
 *
 */


#include "lmachdep.h"
#ifdef PC
#include <malloc.h>
#endif
#include "lmclient.h"
#include "lgetattr.h"
#include <stdio.h>
#include "l_prot.h"

static int portathost_plus      lm_args((LM_HANDLE *job, CONFIG *, 
                                        LM_SERVER *, int));
static int is_use_server        lm_args(( LM_HANDLE *, char *));
static int l_upgrade            lm_args((LM_HANDLE *, CONFIG *));

#define D(_x) (data+ (_x*(MAX_CONFIG_LINE+1)))
#define D2(_x, _y) (data+ (_x*(MAX_CONFIG_LINE+1)) + _y)
#define DATA(_x, _y)    *(data+ (_x*(MAX_CONFIG_LINE+1)) + _y)
static char allfeat_daemon[MAX_DAEMON_NAME + 1];
static void oldkey lm_args(( LM_HANDLE *, CONFIG *));

l_allfeat(job, lf, s, lfindex)
LM_HANDLE *job;
LICENSE_FILE *lf;
LM_SERVER *s;
int lfindex;
{

/*      
 *      Feature lines:
 * FEATURE name DAEMON-name version exp-date num-users code "user-def" hostid 
 */
#define FNAME 0
#define DAEMON 1
#define VERSION 2
#define DATE 3
#define NUMUSERS 4
#define CODE 5
#define USERDEF 6



  char line[MAX_CONFIG_LINE+1];
  CONFIG tempfeat;
  CONFIG *last, *temp;
  int foundsome = 0;

        if (job->line)
        {
                for (last = job->line; last->next; last = last->next) ;
        }
        else
                last = (CONFIG *) NULL;

        if (lf->type == LF_PORT_HOST_PLUS)
        {
                return portathost_plus(job, last, s, lfindex);
        }

/*
 *      Make sure we are at the start of the file
 */
        l_lfseek(lf, 0L, 0);
        while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
        {
#ifdef NLM
        ThreadSwitch();
#endif            
                if (!(job->flags & LM_FLAG_IS_VD) && 
			(!job->flags & LM_FLAG_LMGRD) && 
                        (job->options->flags & LM_OPTFLAG_PORT_HOST_PLUS) && 
                        ((*line == '+') ||
                        is_use_server(job, line)))
                                
                {
                        return portathost_plus(job, last, s, lfindex);
                }
                if (l_parse_feature_line(job, line, &tempfeat, (char **)0)) 
                {
                                
                        temp = (CONFIG *) l_malloc(job, sizeof(CONFIG));
						if(!temp)
						{
							LM_SET_ERROR(job, LM_CANTMALLOC, 596, 0, 0, LM_ERRMASK_ALL);
							return 0;
						}
                        *temp = tempfeat;       /* structure copy */
                        temp->file_order = ++job->config_number;
                        if (temp->server && !s)
                                s = temp->server; /*- due to decimal format */
                        temp->server = s;       /* Back pointer to LM_SERVER */
                        temp->lf = lfindex;     /* Index of license file */
                        foundsome++;
/*
 *                      Link it in to the list
 */
                        temp->last = last;
                        if (last)
                                last->next = temp;
                        else
                                job->line = temp;
                        last = temp;
                }
                else
                {
                        if (job->lm_errno == LM_CANTMALLOC) return (0);
                }
        }
        return(foundsome);
}

/*
 *      l_parse_feature_line - Parses a feature line in memory
 *              returns - 1 if successfully parsed a feature line, 0 if not
 *              does not fill in next and server fields 
 *              NOTE: this doesn't just parse FEATURE lines -- it
 *                      does INCREMENT, UPGRADE and PACKAGE also
 *              NOTE: exit through the goto, so you be sure to
 *                      free the malloc'd data;
 */

int API_ENTRY
l_parse_feature_line(job, line, temp, err)
LM_HANDLE *job;         /* Current license job */
char *line;
CONFIG *temp;
char **err;             /* error messages -- return value -- 0 if unneeded */
{
  char *data = (char *)0;
  char type[MAX_CONFIG_LINE+1];
  int nflds;
  int i, ret = 0;
  int _ov = VERSION, _v = VERSION, _d = DATE, _n = NUMUSERS, 
  _c = CODE, _u = USERDEF;
  int v71plus = 0;

	memset((char *) temp, '\0', sizeof(CONFIG));
	if (l_parse_decimal(job, (unsigned char *)line, temp, err))
	{
		if (!*temp->daemon && allfeat_daemon && *allfeat_daemon) 
			strcpy(temp->daemon, allfeat_daemon);

		ret = 1;
		goto exit_l_parse_feature_line;
	}

	if (err) *err = (char *)0;
	data = (char *)l_malloc(job,  (USERDEF+2)*MAX_CONFIG_LINE+1 );


/*
 *      This small sscanf saves us much time over the much 
 *      longer one done later.  It is especially noticable
 *      in large license files.
 */
	{
	  char *p1, *p2;
		p1 = D(FNAME); p2 = D(DAEMON);
		nflds = sscanf(line, "%s %s %s", type, p1, p2);	/* overrun checked */
	}
/*
 *      Only do the large sscanf if we are sure
 */
	if (nflds == 3)
	{
	
		if (l_keyword_eq(job, type, LM_RESERVED_FEATURE)) 
			temp->type = CONFIG_FEATURE;
		else if (l_keyword_eq(job, type, LM_RESERVED_INCREMENT) &&
				(l_getattr(job, UPGRADE_INCREMENT)==
						UPGRADE_INCREMENT_VAL))
			temp->type = CONFIG_INCREMENT;
		else if (l_keyword_eq(job, type, LM_RESERVED_UPGRADE) &&
				(l_getattr(job, UPGRADE_INCREMENT)==
						UPGRADE_INCREMENT_VAL))
			temp->type = CONFIG_UPGRADE;
		else if (l_keyword_eq(job, type, LM_RESERVED_PACKAGE) &&
				l_getattr(job, FULL_FLEXLM)==FULL_FLEXLM_VAL)
			temp->type = CONFIG_PACKAGE;
		else    
		{
			if (!l_parse_daemon(job, line, type, data))
			{
				temp->type = CONFIG_UNKNOWN;
				ret = 0; /* FAILURE */
			}
		}
	}
	if (nflds == 3 && ((temp->type == CONFIG_FEATURE) ||
					(temp->type ==  CONFIG_INCREMENT) ||
					(temp->type ==  CONFIG_PACKAGE) ||
					(temp->type ==  CONFIG_UPGRADE)))
	{
		if (temp->type == CONFIG_UPGRADE)
		{
			_v++; _d++; _n++; _c++; _u++;
		}
		strcpy(D(_u), "");          
/*- 		Some platforms (e.g. NeXT) don't initialize this 
 *		if the corresponding field is NULL
 */
		if (temp->type == CONFIG_UPGRADE)
		{
			nflds = sscanf(line, 	/* overrun checked */
			"%s %s %s %s %s %s %s %s %[^\n]\n", type,
			D(FNAME), D(DAEMON), D(_ov), D(_v), 
			D(_d), D(_n), D(_c), D(_u));
			if (nflds > _ov)
			{
				if (l_valid_version(D(_ov)))
				{
					strncpy(temp->fromversion, D(_ov), 
					MAX_VER_LEN);
					temp->fromversion[MAX_VER_LEN] = 0;
				}
				else
				{
					LM_SET_ERROR(job, LM_BAD_VERSION, 3, 0, 
						D(_ov), LM_ERRMASK_ALL);
					goto exit_l_parse_feature_line;
				}
			}
			{ /* P5771 */
			  char *cp = D(_c);
				if (!l_keyword_eq_n(job, "start:", cp, 6))
				{
					for (i = 0;  (i < MAX_CRYPT_LEN) && 
						cp[i]; i++)
					{
						if (!isxdigit(cp[i]))
						{
							v71plus++;
							break;
						}
					}
				}
			}
			if (v71plus)
				sscanf(line, 	/* overrun checked */
				"%s %s %s %s %s %s %s %[^\n]\n", type,
				D(FNAME), D(DAEMON), D(_ov), D(_v), 
				D(_d), D(_n), D(_u));
		}
		else if (temp->type == CONFIG_PACKAGE)
		{
			if (l_parse_package(job, line, temp, data, err)) 
			{
				ret =  0;
				goto exit_l_parse_feature_line;
			}
			else ret = 1;
		}
		else
		{
			nflds = sscanf(line, "%s %s %s %s %s %s %s %[^\n]\n", 	/* overrun checked */
					type, D(FNAME), D(DAEMON), D(_v), 
						D(_d), D(_n), D(_c), D(_u));
			{
			  char *cp = D(_c);
				if (!l_keyword_eq_n(job, "start:", cp, 6))
				{
					for (i = 0;  (i < MAX_CRYPT_LEN) && 
						cp[i]; i++)
					{
						if (!isxdigit(cp[i]))
						{
							v71plus++;
							break;
						}
					}
				}
			}
			if (v71plus)
				sscanf(line, "%s %s %s %s %s %s %[^\n]\n", 	/* overrun checked */
					type, D(FNAME), D(DAEMON), D(_v), 
						D(_d), D(_n), D(_u));
		}
		nflds--;                    /* Account for type */
		if (nflds >= _u)
		{                           /* Found one */
		  char *count_str = D(_n);

			temp->next = (CONFIG *) NULL;

			if (l_valid_version((D(_v))))
			{
				l_zcp(temp->version, D(_v), MAX_VER_LEN);
			}
			else
			{
				LM_SET_ERROR(job, LM_BAD_VERSION, 4, 0, D(_v), 
							LM_ERRMASK_ALL);
				ret =  0;
				goto exit_l_parse_feature_line;
			}

			if ((int)strlen(D(FNAME)) <= MAX_FEATURE_LEN)
				strcpy(temp->feature, D(FNAME));
			else
			{
				LM_SET_ERROR(job, LM_BADFILE, 5, 0, D(FNAME),
				LM_ERRMASK_ALL);
				ret =  0;
				goto exit_l_parse_feature_line;
			}
			/*if (!strcmp(temp->feature, "f7")) puts("HERE is f7");*/
			strncpy(temp->daemon, D(DAEMON), MAX_DAEMON_NAME);
			temp->daemon[MAX_DAEMON_NAME] = '\0';
			if (l_keyword_eq(job, D(_d), LM_RESERVED_UNEXPIRING))
				strcpy(temp->date, "1-jan-0");
			else if ((int)strlen(D(_d)) <= DATE_LEN)
				strcpy(temp->date, D(_d));
			else
			{
				LM_SET_ERROR(job, LM_BADFILE, 6, 0, D(_d), 
				LM_ERRMASK_ALL);
				ret =  0;
				goto exit_l_parse_feature_line;
			}
			if (l_keyword_eq_n(job, count_str, LM_LICENSE_COUNT, 
						strlen(LM_LICENSE_COUNT)))
			{
				count_str += strlen(LM_LICENSE_COUNT);
				/* skip to '=' */
				while (*count_str && isspace(*count_str)) 
					count_str++; 
				count_str++; /* skip '=' */
				/* skip spaces after '=' */
				while (*count_str && isspace(*count_str)) 
					count_str++; 
			}
			if (l_keyword_eq(job, count_str, LM_RESERVED_UNCOUNTED))
				temp->users = 0;
			else if (isdigit(count_str[0]))
			{
				sscanf(count_str, "%d", &temp->users);	/* overrun checked */
				temp->upgrade_count = temp->upgrade_remaining = temp->users;
			}
#ifdef SUPPORT_METER_BORROW
			else if (l_keyword_eq_n(job, count_str, 
					LM_LICENSE_METER_BORROW,
			strlen(LM_LICENSE_METER_BORROW)))
			{
			  char junk[10];
			  char arg1[MAX_CONFIG_LINE + 1];
			  char arg2[MAX_CONFIG_LINE + 1];
			  char arg3[MAX_CONFIG_LINE + 1];
			  int _nflds;

				temp->users = LM_COUNT_FROM_METER;
				temp->lc_got_options |=  LM_LICENSE_TYPE_PRESENT;
				temp->lc_type_mask |=  LM_TYPE_METER_BORROW;
				_nflds = sscanf(count_str, "%[^:]:%[^:]:%[^:]:%s", 	/* overrun checked */
				junk, arg1, arg2, arg3);
				if (_nflds == 4)
				{
					temp->meter_borrow_device = 
					l_malloc(job, 
					strlen(arg1) + strlen(arg2)+2);
					sprintf(temp->meter_borrow_device, 
							"%s:%s", arg1, arg2);
					sscanf(arg3, "%d", &temp->meter_borrow_counter);	/* overrun checked */
				}
				else
					sscanf(arg1, "%d", 	/* overrun checked */
						&temp->meter_borrow_counter);
				}
#endif /* SUPPORT_METER_BORROW */
			else
			{
				LM_SET_ERROR(job, LM_BADFILE, 420, 0, count_str,
								LM_ERRMASK_ALL);
				ret =  0;
				goto exit_l_parse_feature_line;
			}
			if (!v71plus)
			{
				
				temp->L_CONF_FLAGS |= L_CONF_FL_OLD_KEY;
				l_zcp(temp->code, D(_c), MAX_CRYPT_LEN);
			}
/*
 *              	Convert license key to uppercase, as l_crypt_private
 *              	provides it.
 */
			if (!job->options->crypt_case_sensitive)
			{
				l_uppercase(temp->code);
			}
			i = strlen(D(_u));
			if (DATA(_u,i) == '\n') DATA(_u,i) = '\0';
			if (DATA(_u,0) == '"')
			{
/*
 *                      Old-style (pre-v3.0) FEATURE/UPGRADE/INCREMENT line
 */
			   char *p;

				p = strchr(D2(_u,1), '"');
				if (p)
				{
					i = p - (D2(_u,1));
					if (i > 0)
					{
						temp->lc_vendor_def = 
							(char *)l_malloc(job, 
								(i + 1));
						strncpy(temp->lc_vendor_def, 
						D2(_u,1), i);
						temp->lc_vendor_def[i] = '\0';
					}
					while (*p && !isspace(*p)) p++;
					while (*p && isspace(*p)) p++;
				}

				if (p && *p)
				{
					if (l_get_id(job, &temp->idptr, p))
					{
						ret =  0;
						goto exit_l_parse_feature_line;
					}
				}
				else
				{
					temp->idptr = (HOSTID *)0; /* NOHOSTID */
				}
			}
			else
			{
			   char *cp;
/*
 *                      	New (v3.0+) FEATURE/UPGRADE/INCREMENT line
 */
				cp = l_parse_attr(job, D(_u), temp);
				if (!err && cp) free(cp);
				else if (err && !*err) *err = cp;

			}
			ret =  1;
			goto exit_l_parse_feature_line;
		}
	}
/* 
 *	Currently, FLOAT_OK only supports exactly one hostid
 */
	if ((temp->lc_type_mask & LM_TYPE_FLOAT_OK) 
		&& (!temp->idptr || temp->idptr->next ||
			!LM_IS_FLEXID(temp->idptr->type)))
	{
		LM_SET_ERROR(job, LM_FLOATOK_ONEHOSTID, 573, 0, temp->idptr ? 
				l_asc_hostid(job, temp->idptr) : "???",
							LM_ERRMASK_ALL);
	}

exit_l_parse_feature_line:
	if (ret && temp->type == CONFIG_UPGRADE && !l_upgrade(job, temp) )
	{
		temp->conf_state = LM_CONF_REMOVED;
	}

	if (ret) oldkey(job, temp);

	if (data)       free(data);
	return(ret);
}


#define strsave(x, where) \
                { char *_c = (char *) l_malloc(job, (strlen(x)+1)); \
                  strcpy(_c, x);  where = _c; }


/*
 *      l_good_version -- return 1 if char * version is valid
 *                      version must be a valid floating point
 *                      number -- numbers optionally followed by . 
 *                      optionally followed by numbers, and
 *                      must be <= MAX_VER_LEN
 */
int API_ENTRY
l_valid_version(ver)
char *ver;
{
  char *cp = ver;
  int got_decimal = 0;

        if (!ver) return 0;

        if ((int)strlen(ver) > MAX_VER_LEN) return 0;

        for (cp = ver; *cp; cp++)
        {
                if (isdigit(*cp))
                        continue;
                if (*cp == '.' && !got_decimal) /* only allow 1 decimal */
                {
                        got_decimal = 1;
                        continue;
                }
/* 
 *      if we got here, there's an invalid character
 */
                return 0;
        }
        return 1;
}

static
int
portathost_plus(job, last, s, lfindex)
LM_HANDLE *job;
CONFIG *last;
LM_SERVER *s;
int lfindex;
{
  CONFIG *temp;

        temp = (CONFIG *) l_malloc(job, sizeof(CONFIG));
		if(!temp)
		{
			LM_SET_ERROR(job, LM_CANTMALLOC, 597, 0, 0, LM_ERRMASK_ALL);
			return 0;
		}
        temp->type = CONFIG_PORT_HOST_PLUS; /* marker record */
        temp->conf_state = LM_CONF_MARKER; /* marker record */
        temp->server = s;       /* Back pointer to LM_SERVER */
        temp->lf = lfindex;     /* Index of license file */
        temp->lc_vendor_def = (char *)l_malloc(job, 1);
        strcpy(temp->code, CONFIG_PORT_HOST_PLUS_CODE);
        strcpy(temp->daemon, job->vendor); /* assume it's the same
                                              as the job */
/*
 *      Link it in to the list
 */
        temp->last = last;
        if (last)
                last->next = temp;
        else
                job->line = temp;
        last = temp;
        return 1;   /* all done -- this is just a dummy record */
}

int API_ENTRY
l_parse_daemon(job, line, type, buf)
LM_HANDLE *job;
char *line;
char *type;
char *buf;
{
        *type = 0;
        *buf = 0;
        sscanf(line, "%s %s", type, buf);	/* overrun threat */
        if (l_keyword_eq(job, type, LM_RESERVED_PROG) ||
                        l_keyword_eq(job, type, LM_RESERVED_PROG_ALIAS))
                l_zcp(allfeat_daemon, buf, MAX_DAEMON_NAME);
        else return 0;
        return 1;
}
static
int
is_use_server(job, line)
LM_HANDLE *job;
char *line;
{
  char *cp;

        if (!*line) return 0;
        cp = &line[strlen(line) - 1];
        if (*cp == '\n') *cp = 0;
        return l_keyword_eq(job, line, LM_RESERVED_USE_SERVER);
}
/*
 *      Make sure uncounted UPGRADE lines have a prior F/I line 
 *      Currently we never compare lc_vendor_def...
 */
static
l_upgrade(job, upgrade)
LM_HANDLE *job;
CONFIG *upgrade;
{
  CONFIG *c;

        if (upgrade->users > 0) return 1;
        for (c = job->line; c; c = c->next)
        {
                if ((c->type == CONFIG_FEATURE || 
                        c->type == CONFIG_INCREMENT) &&
                        l_keyword_eq(job, upgrade->feature, c->feature) &&
                  (l_compare_version(job, upgrade->fromversion, c->version)
                                                                <= 0) &&
                  (l_compare_version(job, upgrade->version, c->version) 
                        > 0) &&
                        l_hostid_cmp(job, upgrade->idptr, c->idptr))
                        return 1;
        }
        return 0;
}
static
void
oldkey(job, conf)
LM_HANDLE *job;
CONFIG *conf;
{
  char *code;
  char *date = l_extract_date(job, conf->code);
  LM_ERR_INFO e;

	memset(&e, 0, sizeof(e));
	l_err_info_cp(job, &e, &job->err_info);

	code = l_crypt_private(job, conf, date, 0);
	l_zcp(conf->hash, code, MAX_CRYPT_LEN);
	l_err_info_cp(job, &job->err_info, &e);
	l_free_err_info(&e);

}
#define LM_CRYPT_HASH
#include "l_crypt.c"
