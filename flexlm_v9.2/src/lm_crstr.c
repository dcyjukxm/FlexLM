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
 *	Module: $Id: lm_crstr.c,v 1.49 2003/04/18 23:48:06 sluu Exp $
 *
 *	Function: 	lc_cryptstr
 *
 *	Description: 	Converts a file string to a file string with
 *			20-char codes filled in or returns
 *			returns the crypt code for the first feature.
 *
 *	Parameters:	job
 *			str 	char * of the whole license file
 *			return_str ptr to char *  -- returned string
 *			code	VENDORCODE *.  If code is 0, it will
 *				only format.
 *			flags
 *				LM_CRYPT_ONLY -- return just the
 *				crypt code for only the first FEATURE
 *				LM_CRYPT_FORCE -- crypt all lines, even those
 *				with 20-character codes already set
 *			filename char * for error reporting
 *			errors ptr to char * -- for error reporting
 *	Return:		if flag & LM_CRYPT_ONLY, returns a crypt code, 
 *			otherwise returns the whole file, with license key 
 *			filled in.
 *
 *			The memory for the returned value is
 *			malloc'd by this routine.  It is the
 *			responsibility of the caller to free it, if
 *			needed.
 *			returns 0 if successful, or -1. See errors.
 *
 *	D. Birns
 *	10/11/94
 *
 *	Last changed:  12/11/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "l_prot.h"
#include "l_strkey.h"

#ifdef PC
#include "pcsock.h"
#endif

typedef struct _featset {
	char vendorname[MAX_CONFIG_LINE+1];
	char code[MAX_CONFIG_LINE+1];
} FeatSet;

typedef struct _linelist {
	struct _linelist *next;
	char *text;		/* the text line */
	int linetype;
#define L_CONFIG 1
#define L_SERVER 2
#define L_FEATSET 3
#define L_DAEMON 4
	int flags;
#define LL_KEY_OLD 1
#define LL_KEY_NEW 2

	union {
		CONFIG *feature;
		LM_SERVER *server;
		FeatSet *featset;
	} d;
	int linenum;
} LineList;

static void 	free_ll 	lm_args(( LM_HANDLE *, LineList *));
static LM_SERVER * getserverlist lm_args(( LM_HANDLE *, LineList *));
static void 	parsefeaturelist lm_args(( LM_HANDLE *, LineList *, int, VENDORCODE *));
static char * 	cryptfeaturelist lm_args((LM_HANDLE *, LineList *, LM_SERVER **,
					int, VENDORCODE *));		
static int 	l_parse_featset_line lm_args((LM_HANDLE *, char *, FeatSet *fp));
static void 	parsefeaturesets lm_args(( LM_HANDLE *, LineList *));
static void 	cryptfeaturesets lm_args(( LM_HANDLE *, LineList *, 
						VENDORCODE *, int));
static char * 	getfeatcodes 	lm_args(( LM_HANDLE *, LineList *, char *));
static LineList * readstring 	lm_args(( LM_HANDLE *, char *, char **));
static char * 	writelicfile 	lm_args(( LM_HANDLE *, LineList *, int));
static int  	docryptfeat 	lm_args(( LM_HANDLE *, CONFIG *, LM_SERVER *, 
						VENDORCODE *, int, int));
static void 	docryptfeatset 	lm_args(( LM_HANDLE *, FeatSet *, LineList *, 
							VENDORCODE *, int));
static void 	errstring 	lm_args((LM_HANDLE *, int, char *));

static char *	fname;
static char *	errbuf;
static int 	err; /* return number of errors */
static int      ignore_featname_errs;
static int      print_linenum_in_errstr = 1;
static int      crypt_cksum_only = 0;
static int      decimal_fmt = 0;


int API_ENTRY
lc_cryptstr(job, str, return_str, code, flag, filename, errors)
LM_HANDLE *job;
char *str, **return_str;
VENDORCODE *code;
int flag;
char *filename;
char **errors;
{
  LineList *linelist;
  LM_SERVER *serverlist;
  char * t, *lltext = (char *)0;

	if (LM_API_ERR_CATCH)
		return job->lm_errno;

	job->flags |= LM_FLAG_LIC_GEN;
	if (code)  /* make sure the job is up-to-date */
	{
	  LM_VENDORCODE_PUBKEYINFO *p;
	  int i;
	
		for (i = 0, p = code->pubkeyinfo; i < LM_MAXSIGNS; i++, p++)
		{
			if (!p->pubkey_fptr)
				break;
			l_add_key_filter(job, 0, (char *)p->pubkey_fptr, 
				(int)p->sign_level, LM_KF_DIG_SIG, 
				p->pubkeysize, p->pubkey, p, p->strength);
		}
	}
	if (!(fname = filename))
		fname = "";
	if (errors)
		*errors = errbuf = (char *)0;
	err = 0;

	linelist = readstring(job, str, &lltext);

	if (!(ignore_featname_errs = (flag & LM_CRYPT_IGNORE_FEATNAME_ERRS)))
		job->options->flags |= LM_OPTFLAG_REPORT_FEATNAME_ERRS;

	print_linenum_in_errstr = (!(flag & LM_CRYPT_NO_LINE_NUMBERS));
	crypt_cksum_only = (flag & LM_CRYPT_FOR_CKSUM_ONLY);
	decimal_fmt = (flag & LM_CRYPT_DECIMAL_FMT);
	serverlist = getserverlist(job, linelist);
	parsefeaturelist(job, linelist, flag, code);
	t = cryptfeaturelist(job, linelist,&serverlist, flag , code);
		/* crypt feat lines as needed */
	if (t)
	{
		if (!(flag & (LM_CRYPT_ONLY|LM_CRYPT_KEY2_ONLY)))
		{
			parsefeaturesets(job, linelist);
			cryptfeaturesets(job, linelist, code, 
						flag & LM_CRYPT_FORCE);
		}
		if (flag & (LM_CRYPT_ONLY|LM_CRYPT_KEY2_ONLY))
		{
			*return_str = (char *)l_malloc(job, strlen(t)+1);
			(void)strcpy(*return_str, t);
		}
		else
			*return_str = writelicfile(job, linelist, 
				flag & (LM_CRYPT_ONLY|LM_CRYPT_KEY2_ONLY));
	}
	if (lltext)
		free(lltext);
	free_ll(job, linelist);
	if (errors)
		*errors = errbuf;
	job->flags &= ~LM_FLAG_LIC_GEN;
	LM_API_RETURN(int, (err))
}

static
void
free_ll(job, linelist)
LM_HANDLE *job;
LineList *linelist;
{
  LineList *ll, *next;
	for (ll = linelist; ll; ll = next)
	{
		next = ll->next;
		switch (ll->linetype)
		{
		case L_SERVER: 
/* 
 *			There's a problem in the way these lists are
 *			set up for red-servers that will cause a free 
 *			memory problem if we just free the servers
 *			These servers are set up like
 *				ll->d.server = 1st server
 *				ll->d.server->next = 2nd server
 *				ll->d.server->next->next = 3rd server
 *				ll->next->d.server->next = 2nd server
 *				ll->d.next->server->next->next = 3rd server
 *				ll->d.next->next->server->next->next = 3rd s
 *			So, we solve this by setting ll->d.server->next
 *			to 0.  This way all 3 servers get freed once.
 */
			ll->d.server->next = 0; 
			l_free_server(job, ll->d.server);
			break;
		case L_CONFIG:
			{
				l_free_conf(job, ll->d.feature);
				ll->d.feature = NULL;
			}
			break;
		case L_FEATSET: free(ll->d.featset);
			break;
		}
		free(ll);
	}
}


static
LM_SERVER *
getserverlist(job, linelist)
LM_HANDLE *job;
LineList *linelist;
{
	LM_SERVER *slist = (LM_SERVER *)0, *sl, **slp;
	LineList *ll;
	LM_SERVER server;

	slp = &slist;
	for (ll=linelist; ll; ll=ll->next)
	{
		if (!ll->text)
			continue;
		if (l_parse_server_line(job, ll->text, &server))
		{
			sl = (LM_SERVER *)l_malloc(job, sizeof(LM_SERVER));
			*sl = server;	/* structure copy */
			if (sl->port <= 0 && 
				strcmp( job->options->license_fmt_ver, 
							LM_BEHAVIOR_V6) < 0)
			{
				sl->port = LMGRD_PORT_START;
				sprintf(ll->text, "%s %d", ll->text, 
					LMGRD_PORT_START);
			}
			sl->idptr = (HOSTID *)0;
			sl->idptr = server.idptr;
			*slp = sl;
			slp = &sl->next;
			ll->linetype = L_SERVER;
			ll->d.server = sl;
			if (sl->sflags & L_SFLAG_THIS_HOST &&
					sl->idptr && 
					(sl->idptr->type != HOSTID_DEMO) &&
					(sl->idptr->type != HOSTID_ANY))
			{
			  LM_ERR_INFO e;

				memset(&e, 0, sizeof(e));
				l_err_info_cp(job, &e, &job->err_info);

				if (!l_host(job, sl->idptr))
				{
#ifdef PC
					char  TempBuf[MAX_HOSTNAME];
					if (!gethostname(TempBuf, MAX_HOSTNAME))
							strcpy(sl->name,TempBuf);
#else
					gethostname(sl->name, MAX_HOSTNAME);
#endif
					sprintf(ll->text, "SERVER %s %s",
					sl->name, l_asc_hostid(job, sl->idptr));
					if (sl->port > 0)
						sprintf(ll->text, "%s %d", 
							ll->text, sl->port);
				}
				l_err_info_cp(job, &job->err_info, &e);
				l_free_err_info(&e);
			}
		}
		else 
		{
		  int len1 = strlen(LM_RESERVED_PROG);
		  int len2 = strlen(LM_RESERVED_PROG_ALIAS);
		  char buf1[100];
		  char buf2[100];
		  char buf3[100];
		  char bufa[100];
		  char bufb[100];

			l_zcp(bufa, ll->text, len1);
			l_zcp(bufb, ll->text, len2);
			if ( (l_keyword_eq( job, bufa, LM_RESERVED_PROG))
			 || l_keyword_eq( job, bufb, LM_RESERVED_PROG_ALIAS ))
			{
				ll->linetype = L_DAEMON;
				if (strcmp( job->options->license_fmt_ver, 
							LM_BEHAVIOR_V6) < 0)
				{
					if (sscanf(ll->text, "%s %s %s", 	/* overrun threat */
						buf1, buf2, buf3) == 2) 
						sprintf(ll->text, "%s %s %s", 
							LM_RESERVED_PROG, 
							buf2, buf2);
				}
			}
				
		}
		if (job->lm_errno)
		{
			errstring(job, ll->linenum, 0);
			l_clear_error(job);
		}

	}
	return slist;
}
/* 
 *	errstring -- set value of return error string,
 *			if !str, use lc_errstring on job->lm_errno
 */
static 
void
errstring(job, linenum, str)
LM_HANDLE *job; 
int linenum;
char *str;
{
  char *buf;
  char *oldbuf;
  char *lcerr = "";
  char *tmp;

	if (job->lm_errno == LM_DEMOKIT)
		return;
	err = -1;
	if (job->lm_errno) 
		lcerr = lc_errstring(job);
	if (errbuf)
		oldbuf = errbuf;
	else
		oldbuf = "";

	tmp = str;
	if (!tmp)
		tmp = "";

	buf = (char *)l_malloc(job, strlen(oldbuf) + strlen(tmp) + 
				strlen(fname) + strlen(lcerr) + 30);


	if (errbuf)
		sprintf(buf, "%s\n", errbuf);
	else
		*buf = '\0';


	if (str)
	{
		if (linenum && print_linenum_in_errstr)
			sprintf(buf, "%s%s:line %d: %s\n\t%s", buf, fname, 
				linenum, str, lcerr);
		else
			sprintf(buf, "%s%s: %s\n\t%s", buf, fname, 
				str, lcerr);
	}
	else if (linenum > 0 && print_linenum_in_errstr)
		sprintf(buf, "%s%s:line %d:%s", buf, fname, linenum, 
					lcerr);
	else
		sprintf(buf, "%s%s:%s", buf, fname, lcerr);

	if (errbuf)
		free(errbuf);
	errbuf = buf;
}


static
void
parsefeaturelist(job, linelist, flag, code)
LM_HANDLE *job;
LineList *linelist;
int flag;
VENDORCODE *code;
{
	CONFIG *flist, *fl, **flp;
	LineList *ll;
	CONFIG feature;
	int rc;
	char *flerr;

	flp = &flist;
	for (ll=linelist; ll; ll=ll->next) 
	{
		if (l_parse_feature_line(job, ll->text,&feature, &flerr))
		{
			fl = (CONFIG *)l_malloc(job, sizeof(CONFIG));
			if(!fl)
			{
				LM_SET_ERROR(job, LM_CANTMALLOC, 603, 0, 0, LM_ERRMASK_ALL);
				return;
			}
			*fl = feature;	/* structure copy */
			*flp = fl;
			ll->linetype = L_CONFIG;
			ll->d.feature = fl;
			if(code && l_keyword_eq(job, fl->daemon, job->vendor))
			{
				if ((fl->code[0]) && 
					!(fl->L_CONF_FLAGS & L_CONF_FL_OLD_KEY))
					fl->code[0] = 0;
				if (fl->code[0]) 
					fl->L_CONF_FLAGS |= L_CONF_FL_MAKE_KEY_OLD;

				if (job->lm_errno == LM_FUTURE_FILE)
					job->lm_errno = LM_BADFILE;
				rc = l_validdate(job, fl->date);
				if (rc && (rc != LM_DATE_TOOBIG))
				{
					LM_SET_ERRNO(job, rc, 227, 0);
					errstring(job, ll->linenum, 0);
					l_clear_error(job);

					if (rc != LM_LONGGONE)
						continue;
				}
				if ((flag & (LM_CRYPT_ONLY|LM_CRYPT_KEY2_ONLY)) && 
					l_keyword_eq(job, fl->daemon, job->vendor))
					return;
			}
		}
		if (job->lm_errno)
		{
			errstring(job, ll->linenum, flerr);
			if (flerr)
				free(flerr);
			l_clear_error(job);
		}
	}
}

static
char *	/* 1 or crypt code if OK, 0 if error */
cryptfeaturelist(job, linelist,serverlistp, flag, code)
LM_HANDLE *job;
LineList *linelist;
LM_SERVER **serverlistp;
int flag;
VENDORCODE *code;
{
   	LM_SERVER *serverlist = *serverlistp;
	LineList *ll;
	CONFIG *f;
	int t;
	unsigned z = 0;
	VENDORCODE v;
	char buf[30];

	for (ll=linelist; ll; ll=ll->next)
	{
		if (ll->linetype==L_CONFIG)
		{
			f = ll->d.feature;
			if (!f->server && serverlist)
				f->server = serverlist;
			/* only encrypt our own feature lines */
			if (code && l_keyword_eq(job, f->daemon,job->vendor)) 
			{
				t = docryptfeat(job, f,serverlist, code, 
					ll->linenum, flag);
				if (t) 
				{
					errstring(job, ll->linenum, 
						"crypt features failed");
					return (char *)0; /* failure */
				}
				if (flag & LM_CRYPT_ONLY)
				{
					if (f->lc_sign)
						return f->lc_sign;
					return f->code;
				}
				if (!serverlist && f->server)
					serverlist = f->server;
			} 
			else if (flag & (LM_CRYPT_ONLY|LM_CRYPT_KEY2_ONLY))
			{
				errstring(job, ll->linenum, 
					"Cannot encrypt for wrong daemon name");
				return (char *)0;
			}
			else 
			{
				if (l_check_fmt_ver(job, f))
				{
					errstring(job, ll->linenum, 
						f->feature);
					return 0;
				}
			}

			if (code && 
				(f->lc_got_options & LM_LICENSE_CKSUM_PRESENT))
			{
			  CONFIG *feature = (CONFIG *)
					l_malloc(job, sizeof(CONFIG));
			  long sav = job->flags;

			  if(!feature)
			  {
				  LM_SET_ERROR(job, LM_CANTMALLOC, 604, 0, 0, LM_ERRMASK_ALL);
				  return 0;
			  }
				job->flags |= LM_FLAG_CKSUM_ONLY;
				memcpy(&v, code, sizeof(v));
				v.data[0] = v.data[1] = 0;
				l_sg(job, job->vendor, &v);
/*
*				Have to call parse again, with cksum
*				flag set to true
*/
				l_parse_feature_line(job, 
						ll->text, feature, 0);
				l_zcp(feature->code, f->code, 
						MAX_CRYPT_LEN);
				z = l_cksum(job, feature, (int *)&z,&v);
				f->lc_cksum = (int) z;
				l_free_conf(job, feature);
				job->flags = sav;
			}
			if (decimal_fmt)
				f->decimal_fmt = LM_CONF_DECIMAL_FMT;
			if (l_print_config(job, f, ll->text))
				errstring(job, ll->linenum, 
					"Formatting error");
/* 
 *			If it was decimal, and SERVER name was this_host
 *			attempt to convert to real hostname
 */
			l_zcp(buf, ll->text, strlen(LM_RESERVED_SERVER));
			if (f->server && 
				(f->server->sflags & L_SFLAG_THIS_HOST) &&
				l_keyword_eq(job, buf, LM_RESERVED_SERVER) &&
					f->server->idptr && 
				(f->server->idptr->type != HOSTID_DEMO) &&
				(f->server->idptr->type != HOSTID_ANY)	&&
					!l_host(job, f->server->idptr))
			{
			  char tmp[MAX_CONFIG_LINE];
			  char *cp;
			  char hostname[MAX_HOSTNAME + 1];

				/* skip SERVER */
				cp = ll->text;
				while (*cp && !isspace(*cp))
					cp++;
				while (*cp && isspace(*cp))
					cp++;
				/* skip hostname */
				while (*cp && !isspace(*cp))
					cp++;
				if (cp) /* if not, there's a bug */
				{
					*hostname = 0;
					gethostname(hostname, MAX_HOSTNAME);
					if (*hostname)
					{
						strcpy(tmp, cp);
						sprintf(ll->text, 
							"SERVER %s", hostname);
						strcat (ll->text, tmp);
					}
				}
			}
				
		}
	}
   	*serverlistp = serverlist;
	return (char *)1;
}

static
int	/* 1 if parsed OK as FEATURESET line, 0 if otherwise */
l_parse_featset_line(job, text,fp)
LM_HANDLE *job;
char *text;
FeatSet *fp;
{
	char f1[MAX_CONFIG_LINE+1],f2[MAX_CONFIG_LINE+1],f3[MAX_CONFIG_LINE+1];
	int nflds;

	nflds = sscanf(text,"%s %s %s",f1,f2,f3);	/* overrun threat */
	if (nflds==3 && l_keyword_eq(job, f1,LM_RESERVED_FEATURESET))
	{
		strcpy(fp->vendorname,f2);
		strcpy(fp->code,f3);
		return 1;
	}
	return 0;
}


static
void 
parsefeaturesets(job, linelist)
LM_HANDLE *job;
LineList *linelist;
{
	FeatSet *flist, *fl, **flp;
	LineList *ll;
	FeatSet featset;
	int saverrno = job->lm_errno;

	job->lm_errno = 0;

	flp = &flist;
	for (ll=linelist; ll; ll=ll->next)
	{
		if (l_parse_featset_line(job, ll->text,&featset))
		{
			fl = (FeatSet *)l_malloc(job, sizeof(FeatSet));
			*fl = featset;	/* structure copy */
			*flp = fl;
			ll->linetype = L_FEATSET;
			ll->d.featset = fl;
		}
		if (job->lm_errno)
		{
			errstring(job, ll->linenum, 0);
			l_clear_error(job);
		}
	}
	job->lm_errno = saverrno;
}

static
void
cryptfeaturesets(job, linelist, code, forceit)
LM_HANDLE *job;
LineList *linelist;
VENDORCODE *code;
int forceit;
{
	LineList *ll;
	FeatSet *f;

	for (ll=linelist; ll; ll=ll->next)
	{
		if (ll->linetype==L_FEATSET)
		{
			f = ll->d.featset;
			docryptfeatset(job, f,linelist, code, forceit);
			sprintf(ll->text,"%s %s %s\n",LM_RESERVED_FEATURESET,
				f->vendorname, f->code);
		}
	}
}

/* return one string which is the concatenation of all of the feature codes */
static
char *
getfeatcodes(job, linelist,vendorname)
LM_HANDLE *job;
LineList *linelist;
char *vendorname;
{
	LineList *ll;
	int rawstralloc;
	int rawstrlen;
	char *rawstr;
	char *newrawstr, *oldrawstr;
	char *fcode;
	int len;

	rawstralloc = 100;
	rawstr = (char *)l_malloc(job, rawstralloc);

	rawstrlen = 0;
	rawstr[0] = 0;
	for (ll=linelist; ll; ll=ll->next)
	{
		if (ll->linetype==L_CONFIG &&
		    l_keyword_eq(job, ll->d.feature->daemon,vendorname))
		{
			fcode = ll->d.feature->code;
			len = strlen(fcode);
			if (len+rawstrlen+1>rawstralloc)
			{
				rawstralloc *= 2;
				newrawstr = (char *)l_malloc(job, rawstralloc);
				strcpy(newrawstr,rawstr);
				oldrawstr = rawstr;
				rawstr = newrawstr;
				free(oldrawstr);
			}
			strcpy(rawstr+rawstrlen,fcode);
			rawstrlen += len;
		}
	}
	return rawstr;
}

static
LineList *
readstring(job, str, buf)
LM_HANDLE *job;
char *str, **buf;
{
	LICENSE_FILE lf;
	char line[MAX_CONFIG_LINE+100];
	LineList *linelist, *ll, **llp;
	int linenum = 0;
	int numlines;
	int numnl;
	int i;
	char *cp;

	memset(&lf, 0, sizeof(lf));
	lf.type = LF_STRING_PTR;
	lf.ptr.str.s = lf.ptr.str.cur = str;

	/* read in the whole file */
	llp = &linelist;
/* 
 *	len is not big enough, because there has to be extra room 
 *	for the 20-character codes, in case they're not already in the string.  
 *	we use (numnl * 30) to add to len.
 *		Add 1) enough room for additional 20-char license key
 *		2) enough room for additional newlines [(len/40) * 2]
 *		3) decimal and 3 places accuracy for version (4 bytes)
 *		4) Null end
 *		5) HOSTID= VENDOR_STRING=
 *		*******************************
 *		WARNING -- this bit is prone to error.  There's no sure
 *		way to predict the length of the output string.
 *		We COULD use MAX_CONFIG_LINE, but that's probably going to
 *		break the PC version.
 */

/*
 *	Calculate size of buffer 
 */
	numnl = 0;
	while (l_lfgets(job, line, MAX_CONFIG_LINE, &lf, &i)) 
	{
		numnl++;
	}
	cp = *buf = (char *)l_malloc(job, numnl * MAX_CONFIG_LINE);
/*
 *	reset file to read it into this buffer
 */
	memset(&lf, 0, sizeof(lf));
	lf.type = LF_STRING_PTR;
	lf.ptr.str.s = lf.ptr.str.cur = str;
	while (l_lfgets(job, line, MAX_CONFIG_LINE, &lf, &numlines))
	{
		ll = (LineList *)l_malloc(job, sizeof(LineList));
		(void) memset(ll, 0, sizeof(LineList));
		ll->text = cp; /* 
				* later this is where the FEATURE line
				* gets written, so this space has to be
				* bigger than the input text 
				*/
		cp += MAX_CONFIG_LINE;
		*llp = ll;
		llp = &ll->next;
		strcpy(ll->text, line);
		ll->linenum = linenum + 1;
		linenum += numlines;
	}
	*llp = (LineList *)0;
	return linelist;
}

static 
char *
writelicfile(job, linelist, crypt_only)
LM_HANDLE *job;
LineList *linelist;
int crypt_only;
{
  LineList *ll;
  char *str, *cur;
  int len = 0;
  int ignore_server = 0;


	for (ll=linelist; ll; ll=ll->next) 
	{
		if ((ll->linetype == L_CONFIG) && ll->d.feature->server && 
			(ll->d.feature->server->sflags & L_SFLAG_PRINTED_DEC))
			ignore_server = 1;
	}

	for (ll=linelist; ll; ll=ll->next) 
	{
		if (ignore_server && (ll->linetype == L_SERVER || 
			ll->linetype == L_DAEMON))
			continue;
		len += strlen (ll->text) + 1; /* 1 for newline */
	}

	cur = str = (char *)l_malloc(job, len + 1);
	*str = '\0';
	for (ll=linelist; ll; ll=ll->next) 
	{
		if (ignore_server && (ll->linetype == L_SERVER || 
			ll->linetype == L_DAEMON))
			continue;
		sprintf(cur, "%s\n", ll->text);
		cur += strlen(cur); 
	}
	return str;
}


/* Generate license key as needed.
 * If license key is "0" or starts with '?', generate license key;
 * if code is string starting with "start:", look for date (in standard
 * date format) to use as starting date, and generate license key;
 * if anything else, pass through as is.
 */
static
int	/* 0 if OK, 1 if error */
docryptfeat(job, featp,serverlist, vcode, linenum, flag)
LM_HANDLE *job;
CONFIG *featp;
LM_SERVER *serverlist;
VENDORCODE *vcode;
int linenum;
int flag;
{
	char *code, *newcode;
	char *startdate;
	char *bin_date = (char *)0;
	char *cferr;
	int t;
	int forceit = flag & LM_CRYPT_FORCE;
	int docrypt = forceit ? 1 : 0;
	LM_KEYLIST *kl;

	code = featp->code;
	if ((job->options->flags & LM_OPTFLAG_LKEY_START_DATE) && 
				(code[0]=='?' || !strcmp(code,"0")))
	{
		code="start:today";
	}
	if (L_STREQ_N(code,"start:",6) || L_STREQ_N(code,"START:",6)) 
	{
		docrypt = 1;
		/* start date can be a standard date or "today" */
		startdate = code + 6;	/* skip over "start:" */
		if (strcmp(startdate,"today")==0 ||
		    strcmp(startdate,"TODAY")==0)
		{
			startdate = (char *)0;
				/* let l_bin_date fill it in */
		}
		if (featp->type == CONFIG_PACKAGE)
			startdate = "1-jan-0";
		else if (startdate)
		{
			t = l_validdate(job, startdate);
			if (t && t!=LONGGONE) 
			{
				errstring(job, -1, startdate);
				l_clear_error(job);
				return 1;	/* bad start date */
			}
			if (t == LM_LONGGONE)
			{
				l_clear_error(job);
			}
		}
		bin_date = l_bin_date(startdate);
	}
	else if
		(forceit) bin_date = l_extract_date(job, code);
	else if
		(L_STREQ(code, "0")) docrypt = 1;
	if (docrypt)
	{

		if (!featp->server)
			featp->server = serverlist;
		if (!featp->server && featp->users >0)
		{
			errstring(job, linenum, 
				"Counted FEATURE line missing SERVER line");
			return 0;	 /* pretend it's ok, so processing 
					    won't stop */
		}
		for (kl = featp->lc_keylist; kl; kl = kl->next)
		{
		  char *cp;

			job->lc_this_keylist = kl;

			cp = lc_crypt(job, featp, bin_date, vcode);
			job->lc_this_keylist = 0;
			if (!cp)
			{
				errstring(job, linenum, featp->feature);
				l_clear_error(job);
			}
			else
			{
				l_free(kl->key);
				kl->key = 0;
				kl->key = l_malloc(job, strlen(cp) + 1);
				strcpy(kl->key, cp);
			}

			if (kl->key && kl->sign_level == 1)
				featp->lc_sign = kl->key;
		}
		if (featp->L_CONF_FLAGS & L_CONF_FL_MAKE_KEY_OLD)

		{
		  int sav_flags = job->flags;
			job->flags |= LM_FLAG_MAKE_OLD_KEY;
			newcode = lc_crypt(job, featp, bin_date, vcode);
			job->flags = sav_flags;
			if (!newcode || !*newcode)
			{
				if (!newcode)
					newcode = featp->code;
				if (!newcode)
					newcode = "0";
				errstring(job, linenum, featp->feature);
				l_clear_error(job);
			}
			strcpy(featp->code, newcode);	/* stuff it back in */
		}
		if (cferr = lc_chk_conf(job, featp, 
						ignore_featname_errs ? 0 : 1))
		{
		  char buf[MAX_CONFIG_LINE];
		  char buf2[MAX_CONFIG_LINE];

			l_print_config(job, featp, buf);
			strcpy(&buf[65], " ...");
			sprintf(buf2, "\"%s\"\n%s", buf, cferr);
			errstring(job, linenum, buf2);
			free(cferr);
			l_clear_error(job);
		}
	}
/*
 *	Add the client/borrower-side 
 */
#ifdef SUPPORT_METER_BORROW
	if (featp->lc_type_mask & LM_TYPE_METER_BORROW)
	{
	  CONFIG *c = (CONFIG *)l_malloc(job, sizeof(CONFIG));
	  HOSTID *h = (HOSTID *)l_malloc(job, sizeof(HOSTID));


	  if(!c)
	  {
		  LM_SET_ERROR(job, LM_CANTMALLOC, 604, 0, 0, LM_ERRMASK_ALL);
		  return 1;
	  }
		memset(h, 0, sizeof(h));
	  	l_conf_copy(job, c, featp);
		c->users = 0;
		c->lc_type_mask &= ~LM_TYPE_METER_BORROW;
		c->lc_got_options &= ~LM_LICENSE_TYPE_PRESENT;
		c->idptr = h;
		*c->code = 0;
		h->type = HOSTID_METER_BORROW;
		h->id.data = featp->meter_borrow_counter;
		if (newcode = lc_crypt(job, c, bin_date, vcode))
		{
			featp->lc_meter_borrow_info = (char *)l_malloc(job, 
				strlen(newcode) + 1);
			strcpy(featp->lc_meter_borrow_info, newcode);
		}
		l_free_conf(job, c);
	}
#endif /* SUPPORT_METER_BORROW */
	return 0;	/* OK */
}

static void
docryptfeatset(job, featsetp, linelist, code, forceit)
LM_HANDLE *job;
FeatSet *featsetp;
LineList *linelist;
VENDORCODE *code;
int forceit;
{
	char *rawstr, *newcode;

	if (forceit || L_STREQ(featsetp->code, "0")) 
	{
		rawstr = getfeatcodes(job, linelist,featsetp->vendorname);
		newcode = (char *)l_string_key(job, (unsigned char *)rawstr,
			strlen(rawstr), code, (unsigned long) job->options->sf);
		strcpy(featsetp->code, newcode);
		free(rawstr);
	}
}

int API_ENTRY
lc_convert(job, str, return_str, errors, flag)
LM_HANDLE *job;
char *str; 
char **return_str;
char **errors;
int flag;
{
	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	if (flag == LM_CONVERT_TO_DECIMAL)
		flag = LM_CRYPT_DECIMAL_FMT;
	else if
		(flag == LM_CONVERT_TO_READABLE) flag = 0;
	else 
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 410, 0);
		return LM_BADPARAM;
	}
		
	LM_API_RETURN(int, lc_cryptstr(job, str, return_str, 0, 
		flag | LM_CRYPT_IGNORE_FEATNAME_ERRS, 0, errors))
}


#include "l_strkey.c"
