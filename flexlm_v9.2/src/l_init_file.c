/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Inc and is protected by law.  It may
	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Inc.

 *****************************************************************************/
/*
 *	Module: $Id: l_init_file.c,v 1.39 2003/05/20 21:05:48 sluu Exp $
 *
 *	Function: l_init_file(job)
 *
 *	Description: 	Initializes in-memory data associated with license files
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *
 *	Return:		None (global data updated)
 *
 *	M. Christiano
 *	6/21/90
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"
#include "lgetattr.h"
#include "flex_file.h"
#include "flex_utils.h"
#include <sys/types.h>
#ifdef NO_DIR_DOT_H
#include <dirent.h>
#else
#include <sys/dir.h>
#define dirent direct
#endif /* NO_DIR_DOT_H */

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef PC
#include<windows.h>
#include "dir_pc.h"
#ifdef PC16
#define L_S_ISDIR(x) (_S_IFDIR & x)
#endif
#endif

/***********************************************************************
 * Local struct definitions
 ************************************************************************/
/* used to keep track of some file status info */
typedef struct
{
	char *	name;			/* name of a file. May also be port at host */
	struct stat fileStat;	/* results of a stat() call */
	int 	didStat;        /* Did we already do the stat */
	int 	badStat;		/* the stat() call failed */
} FileStatInfo;


/***********************************************************************
 * static function decelerations
 ************************************************************************/
static char * getnextfile 		lm_args((char *));
static char * expand_directories 	lm_args((LM_HANDLE *, char *, char *));
static char * expand_default_at_host	lm_args((LM_HANDLE *, char *, char *));
static void   l_sort_configs		lm_args((LM_HANDLE *));
static char * make_sort_str		lm_args(( LM_HANDLE *, CONFIG *, char *));
static void make_ver_str(char *, CONFIG *, int);
static int compareStats(FileStatInfo *fileStat,FileStatInfo *compareStat);
static char *getFileFromPath(char * name);
static int compareFileNamePortion(char * name1, char * name2);
static int delete_dup_files(LM_HANDLE *job, char *originalPathList);

static void delete_dup_feats(LM_HANDLE *job);

void l_set_user_init3 lm_args((void (*)(void)));

#ifdef PC
typedef void (LM_CALLBACK_TYPE * LS_CB_USER_INIT3) (void);
#define LS_CB_USER_INIT3_TYPE (LS_CB_USER_INIT3)
static LS_CB_USER_INIT3 l_user_init3;

#else

#define  LS_CB_USER_INIT3_TYPE
static void (*l_user_init3)();

#endif

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
#endif /* RELEASE_VERSION */


API_ENTRY
l_init_file(job)
LM_HANDLE *job;		/* Current license job */
{
	char *file = (char *) NULL, *p, **ptrs, *strings;
	LICENSE_FILE *lf;
	LM_SERVER *s, *server, tmp_servers[MAX_SERVERS];
	int i;
	int gotone = 0;
	static char *use_finder = (char *)-1;
	int filetype;

#if defined(PC16) || defined(EMBEDDED_FLEXLM)
#define LM_LIC_LIST_MAX 1024 /*- I was thinking of you, Blane! */
			     /*- Consider this: this one constant, at 10k,
				 adds at least 50Kb to the stack.  No embedded
				 system can tolerate that */
#else
#define LM_LIC_LIST_MAX 10000
#endif


    char buf[LM_LIC_LIST_MAX + 1];
    char file_env[LM_LIC_LIST_MAX + 1];

	DEBUG_INIT;

	memset(tmp_servers, 0, sizeof(tmp_servers));
	if (!job->options->disable_env)
	{
        char vd_env_name[100] = {'\0'};
        char vd_env_name2[100] = {'\0'};
		char szLicFileList[LM_LIC_LIST_MAX + 1] = {'\0'};
        char *lm_env = NULL;
        char *vd_env = NULL;
        char *free_this = 0;
		char szVDEnv[MAX_PATH] = {'\0'};
        char ps[2] = {'\0'};

		sprintf(ps, "%c", PATHSEPARATOR);

		sprintf(vd_env_name, "%s_LICENSE_FILE", *job->alt_vendor ?
				job->alt_vendor : job->vendor);
		sprintf(vd_env_name2, "%s_LF", *job->alt_vendor ?
				job->alt_vendor : job->vendor);
		lm_env = l_getEnvUTF8(job, LM_DEFAULT_ENV_SPEC, szLicFileList, sizeof(szLicFileList));
		if ((job->flags & LM_FLAG_LMUTIL) &&
                        !(job->flags & LM_FLAG_LMUTIL_PRIV))
        {
			/* lmutil gets "$VENDOR_LICENSE_FILE", but not lmgrd */
			free_this = vd_env = l_vendor_license_file(job, 0);
        }
        else if (!(vd_env = l_getEnvUTF8(job, vd_env_name, szVDEnv,	/* overrun checked */
				sizeof(szVDEnv))) && !(vd_env = l_getenv(job, vd_env_name2)))
        {
			/* regular application uses this */
			l_uppercase(vd_env_name);
			vd_env = l_getEnvUTF8(job, vd_env_name, szVDEnv, sizeof(szVDEnv));	/* overrun checked */
        }
		sprintf(file_env, "%s%s%s",
			vd_env ? vd_env : "",
			(vd_env && lm_env) ? ps : "",
			lm_env ? lm_env : "");	/* OVERRUN */
		file = file_env;
		if (file && !*file)
            file = 0;
		if (free_this)
            free(free_this);
	}

#if !defined(NLM) && !defined(VMS)
	/*
	 *	If the user didn't override it, see if we can find the "license
	 *	finder" anywhere.
	 */
	if (use_finder == (char *)-1)
		use_finder = l_getenv(job, "FLEXLM_USE_FINDER");	/* overrun checked */
	if ((file == NULL && job->options->disable_finder != 1) || use_finder)

		file = l_finder(job);
#endif /* NLM */

	/*
	 *	If
	 *	  (1) the developer specified it, and
	 *	  (2) the user didn't override it, and
	 *	  (3) there was no "license finder" available,
	 *	then use the path that was specified
	 */
	if (file == NULL && job->options->got_config_file)
		file = job->options->config_file;
	/*
	 *	No file specified by developer, user, or "license finder"
	 */
	if (!file)
	{

#ifdef WINNT
         /* no license file has been specified, check the registry */
        char buf[200];
        GetWindowsDirectory(buf,199);
        if (buf[0]=='A')
            file="A:\\flexlm\\license.dat";
        else
            file = LM_DEFAULT_LICENSE_FILE;
#else /* WINNT */
            file = LM_DEFAULT_LICENSE_FILE;
#endif
	}
	l_free_job_license(job);

	memset(buf, 0, sizeof(buf));
	file = expand_directories(job, file, buf);

#ifdef PC
#endif

	/*
	 *	Now we have the file, put the data into memory.
	 */
	i = delete_dup_files(job, file);

	errno = 0;
	job->lm_numlf = i;    /* Number of license files in the path */
	job->lic_file_strings = strings =
					(char *) l_malloc(job, strlen(file) + 3);
	job->lic_files = ptrs =
				(char **) l_malloc(job, sizeof(char *) * (i + 1));
	job->license_file_pointers = lf =
				(LF_POINTER) l_malloc(job, sizeof(LICENSE_FILE) * i);
	job->license_file_pointers = lf;
	i = 0;
	(void)strcpy(strings, file);
	while (strings)
	{
		ptrs[i] = strings;
		lf[i].type = LF_NO_PTR;
		lf[i].flags |= LM_LF_FLAG_LINE_START;
		lf[i].next = &lf[i+1];
		strings = getnextfile(strings);
		if (strings != (char *) NULL)
		{
			*strings = '\0';
			strings++;
		}
		i++;
	}
	ptrs[i] = (char *) NULL;	/* Terminate the list */
	lf[i-1].next = (LICENSE_FILE *) NULL;	/* End of list */
	/*
	*		Now walk the list and put all the data in memory (Save
	*		and restore the current file pointer).  First, free any
	*		old license file data.
	*/
	job->lfptr = LFPTR_FILE1;
	while (job->lic_files[job->lfptr])
	{
		if ((lf = l_open_file(job, LFPTR_CURRENT)) != (LF_POINTER) NULL)
		{
			LM_ERR_INFO e;

			/*
			*			P2868 -- the file type can get changed
			*				 from FILE to PORT_AT_HOST when the
			*				 file has USE_SERVER in it.  This change
			*				 happens in lfgets.  Therefore, we save
			*				 the type here (filetype), and test this
			*				 later -- this is used to decide whether to
			*				 fclose the file.
			*/
			filetype = lf->type;
			gotone++;

			server = l_master_list_lfp(job, lf, tmp_servers);
			/*
			*			No servers in the license file can be perfectly valid
			*/
			memset(&e, 0, sizeof(e));
			l_err_info_cp(job, &e, &job->err_info);

			if (job->lm_errno == NO_SERVER_IN_FILE)
				l_err_info_cp(job, &job->err_info, &e);
			l_free_err_info(&e);
			/*
			*			Save the server data away
			*/
			for (i=0, s=server; s; s=s->next, i++)
				;
			if (i > 0)
			{
				int k;
				LM_SERVER_LIST_PTR slp;

				errno = 0;
				s = (LM_SERVER *)
				l_malloc(job, i * sizeof(LM_SERVER));
    			/*
    			*				This needs some explanation:
    			*				Normally, you couldn't just copy this struct,
    			*				because there's malloc'd structs in the
    			*				server.  However, this "server" is a pointer
    			*				to tmp_servers, which are automatics.
    			*				Doing the memcpy, moves the pointers over to
    			*				s, which is the malloc'd servers, and
    			*				is free'd when s is free'd.
    			*/
				memcpy(s, server, i * (sizeof(LM_SERVER)));
    			/*
    			*				Fix up the links
    			*/
				for (k=0; k<i; k++)
            	    s[k].next = &s[k+1];
				s[k-1].next = (LM_SERVER *) NULL;
				slp = (LM_SERVER_LIST_PTR)l_malloc(job, sizeof(LM_SERVER_LIST));
				slp->s = s;
				slp->next = job->conf_servers;
				job->conf_servers = slp;
			}
			else
			{
				s = (LM_SERVER *) NULL;
			}
			/*
			*			Now, read and save the feature lines and point them
			*			all at this server struct.
			*/
			l_allfeat(job, lf, s, job->lfptr);
			/*
			*			Nothing found; free the server structs
			*/
			ptrs++;
			if ((filetype == LF_FILE_PTR)  && (lf->type == LF_PORT_HOST_PLUS))
			{
				int sav = lf->type; /* P2868 */

				lf->type = filetype;
				l_lfclose(lf);
				lf->type = sav;
			}
		}
		job->lfptr++;
	}
	job->lfptr = LFPTR_FILE1;  /* Point to start */
	l_supersede(job, CONFIG_PACKAGE); /* packages first */
	l_post_pkg(job);
	l_supersede(job, 0); /* all configs */
	l_sort_configs(job);
	delete_dup_feats(job);
	for (job->lfptr = LFPTR_FILE1; job->lic_files[job->lfptr]; job->lfptr++)
	{
		l_lfclose(&job->license_file_pointers[job->lfptr]);
	}
	job->lfptr = LFPTR_FILE1;  /* Point to start */
    if (l_user_init3)
        (void) (* LS_CB_USER_INIT3_TYPE l_user_init3)();
	if (gotone)
	{
		if (job->lm_errno == NOCONFFILE ||
					job->lm_errno == SERVNOREADLIC)
			LM_SET_ERRNO(job, 0, 0, 0); /* ok */
		return 0;
	}
	else if (job->lm_errno)
        return job->lm_errno;
	else
	{
		LM_SET_ERRNO(job, LM_NOCONFFILE, 360, 0);
		return LM_NOCONFFILE;
	}
}

/***********************************************************************
 * Given a license file path
 *
 * Return:
 * 		the pointer to the char at the end of the first file in the string
 * 		assuming there is a second file in the string.
 *
 * 		NULL if there is only one file name or none.
 ************************************************************************/
static char *getnextfile(char *str)
{
	int start_len = sizeof(LM_LICENSE_START) - 1;
	int end_len = sizeof(LM_LICENSE_END) - 1;
	char *cp;

	if (L_STREQ_N(str, LM_LICENSE_START, start_len))
	{
		/* some portion of the line has the form:
		 * "SART_LICENSE license line here END_LICENSE" */
		for (cp = str + start_len + 1; *cp; cp++)
			if (L_STREQ_N(cp, LM_LICENSE_END, end_len))
				if (*(cp + end_len) != '\0'  )
					return cp + end_len;
		return NULL;
	}
	/* strchr returns NULL if it can't find the char */
	return strchr(str, PATHSEPARATOR);
}


static
char *
expand_directories(job, list, buf)
LM_HANDLE *job;
char *list;
char *buf;
{
	char file[LM_LIC_LIST_MAX + 1] = {'\0'};
	char *cp = NULL;
	int i = 0;
	struct stat st;
	char *next = buf;
	char tmp[LM_LIC_LIST_MAX + 1 ] = {'\0'};
	char tmp2[LM_LIC_LIST_MAX + 1 ] = {'\0'};

	*file = 0;

	for (cp = file, i = 0; i < LM_LIC_LIST_MAX; i++)
	{
#ifdef NLM
	ThreadSwitch();
#endif /* NLM */

		if (list[i] != PATHSEPARATOR && list[i])
		{
			*cp++ = list[i];
		}
		else
		{

			if (*buf) next[-1] = PATHSEPARATOR;

/*
 *			We have a pathname -- see if it's a dir or file
 */
			memset(&st, 0, sizeof(st));
			*cp = 0;  /* null terminate pathname */
			if (*file == '@' && (!(job->options->flags &
					LM_OPTFLAG_PORT_HOST_PLUS)))
			{
				strcpy(tmp2,
					expand_default_at_host(job, file, tmp));
			}
#ifndef EMBEDDED_FLEXLM
			else if (*file && (*file != '@') && !l_flexStat(job, file, &st) && (L_S_ISDIR(st.st_mode) )
					&& l_files_in_dir(job, file, tmp, 0))

			{
				strcpy(tmp2, tmp);
			}
#endif /* !EMBEDDED_FLEXLM */
			else
			{
/*
 *				Append the filename
 */
				strcpy(tmp2, file);
			}
			if ((strlen(tmp2) + strlen(buf)) < LM_LIC_LIST_MAX)
				strcpy(next, tmp2);
			if (!list[i])
                break;
			if (*next)
                next = &next[strlen(next) + 1];
			cp = file; /* next file */
		}
	}
	return buf;
}

/*
 *	compar -- for the qsort call later
 */
static
int
compar(const void *v1, const void *v2)
{
    char **s1 = (char **)v1, **s2 = (char **)v2;
#ifdef PC
    char buf1[MAX_PATH + 1], buf2[MAX_PATH + 1];

	l_zcp(buf1, *s1, MAX_PATH);
	l_zcp(buf2, *s2, MAX_PATH);
	l_uppercase(buf1);
	l_uppercase(buf2);
	return strcmp(buf1, buf2);
#else
	return strcmp(*s1, *s2);
#endif

}

#ifndef EMBEDDED_FLEXLM

char * API_ENTRY
l_files_in_dir(job, path, buf, filename)
LM_HANDLE *job;
char *path;
char *buf;
char *filename; /* optional -- for options files */
{
	DIR *dirp;
	struct dirent *dp;
	char *cp;
	char *next = buf;
	char fsuffbuf[50];
	char suffbuf[10];
	char suffbuf_txt[15];
	char *ptrs[LM_LIC_LIST_MAX/4];
	char sort_tmp[LM_LIC_LIST_MAX];
	int curr_ptr = 0;
	int i;
	#if defined ( PC ) && !defined ( NLM )
	PCDirInfo directoryInformation;
#endif
#ifdef NLM
	char        expanded_path[MAX_PATH+1];
#endif

	*buf = 0;
	memset(ptrs, 0, sizeof(ptrs));
	if (!filename)
	{
		strcpy(suffbuf, LM_LICENSE_FILE_SUFFIX);
		sprintf(suffbuf_txt, "%s.txt", LM_LICENSE_FILE_SUFFIX);
	}
#ifndef NLM
	dirp = L_OPENDIR(path);
#else
	strcpy(expanded_path,path);
	if ( expanded_path[strlen(expanded_path)-1] != PATHTERMINATOR )
		strcat(expanded_path,"\\*.*");
	else
		strcat(expanded_path,"*.*");
	dirp = L_OPENDIR(expanded_path);
#endif
	if (!dirp)
		return 0;
	while  (dp = L_READDIR(dirp))
	{
		if (filename)
		{
			if (l_keyword_eq(job, filename, dp->d_name))
			{
				sprintf(buf, "%s%c%s", path, PATHTERMINATOR,
					dp->d_name);
				L_CLOSEDIR(dirp);
				return buf;
			}
			continue;
		}
		for (cp = &dp->d_name[strlen(dp->d_name) - 1];
					(cp >= dp->d_name) && (*cp != '.');
								 cp--)
			;
		if (*cp == '.')
		{
			l_zcp(fsuffbuf, cp+1, 9);
			if ( l_keyword_eq(job, fsuffbuf, suffbuf) ||
				l_keyword_eq(job, fsuffbuf, suffbuf_txt))
			{
				sprintf(next, "%s%c%s", path, PATHTERMINATOR, dp->d_name);
				ptrs[curr_ptr++] = next;
				next = &next[strlen(next) + 1];
			}
		}
	}
	L_CLOSEDIR(dirp);
	if (!*buf && !filename)
	{
		if (path[strlen(path) -1] == PATHTERMINATOR) /*P4943*/
			sprintf(buf, "%s*.lic", path);
		else
			sprintf(buf, "%s%c*.lic", path, PATHTERMINATOR);
		return buf;
	}
/*
 *	Now sort it
 */

	qsort(ptrs, curr_ptr, sizeof(char *), compar);
	*sort_tmp = 0;
	cp = sort_tmp;
	for (i=0; i < curr_ptr; i++)
	{
		if (*cp)
		{
			cp += strlen(cp);
			*cp++ = PATHSEPARATOR;
		}
		sprintf(cp, ptrs[i]);
	}
	strcpy(buf, sort_tmp);
	return buf;
}

#endif /* !EMBEDDED_FLEXLM */

/*
 * 	expand_default_at_host
 *	Takes @host and turns it into "23456@host:23457@host:[...]23466@host"
 */
static
char *
expand_default_at_host(job, file, tmp)
LM_HANDLE *job;
char *file;
char *tmp;
{
	char *cp = tmp;
	int i;

#if defined ( PORT_AT_HOST_SUPPORT ) && !defined ( NLM )

	job->flags |=  LM_FLAG_CONNECT_NO_HARVEST;
	for (i = LMGRD_PORT_START; i <= job->port_end; i++)
	{
		if (l_lmgrd_running(job, i, &file[1], LM_TCP))
		{
			/* kmaclean 12/17/2002  Bug fix P6872
			 * Added || job->lm_errno == LM_BADHOST
			 * */
			if (job->lm_errno == LM_HOSTDOWN || job->lm_errno == LM_BADHOST )
				break;
			else
				continue;
		}
		sprintf(cp, "%d%s", i, file);
		cp += strlen(cp);
		if (i < job->port_end)
		{
			*cp++ = PATHSEPARATOR;
		}
	}
	if (cp == tmp)
		strcpy(tmp, file);
	else
	{
		if (cp[-1] == PATHSEPARATOR)
			cp--;
		*cp = 0; /* null terminate */
	}
	job->flags &= ~LM_FLAG_CONNECT_NO_HARVEST ;
#endif /* PORT_AT_HOST_SUPPORT */
	return tmp;
}
void
l_set_user_init3(funcptr)
#ifndef PC16
void (*funcptr)(void);
#else
LS_CB_USER_INIT3 funcptr;
#endif
{
	l_user_init3 = funcptr;
}

static
int
compar_conf(const void *v1, const void *v2)
{
    CONFIG **pc1 = (CONFIG **)v1, **pc2 = (CONFIG **)v2;
    CONFIG *c1 = *pc1, *c2 = *pc2;

	return strcmp(c1->sort_str, c2->sort_str);


}

static
void
l_sort_configs(job)
LM_HANDLE *job;
{
    CONFIG *conf;
    CONFIG **conf_arr;
    int i, cnt;
    char *sort_strings, *curr_sort;
#define SORT_STRING_SIZE (MAX_FEATURE_LEN + 60)

/*
 *	First, we sort by feature name...
 */

	if (!job->line)
        return;
 	/* count */
 	for (cnt = 0, conf = job->line; conf; conf = conf->next, cnt++) ;

	/* malloc temp space for sort */
	conf_arr = (CONFIG **)
			l_malloc(job, (cnt + 1)  * sizeof(CONFIG *));
	curr_sort = sort_strings =
		(char *) l_malloc(job, cnt * SORT_STRING_SIZE);

	/* fill temp array */
	DEBUG(("before sort\n"));
 	for (i = 0, conf = job->line; conf; conf = conf->next, i++)
	{
		conf_arr[i] = conf;
		curr_sort = make_sort_str(job, conf, curr_sort);
		DEBUG(("%10.10s %s %s\n", conf->feature, conf->code,
					conf->sort_str));
	}

	/* sort */
	qsort(conf_arr, cnt, sizeof(CONFIG *), compar_conf);

	/* reset pointers based on sort */
	job->line = conf_arr[0];
	DEBUG(("After sort\n"));
 	for (i = 0; i < cnt; i++)
	{
		DEBUG(("%10.10s %s %s\n", conf_arr[i]->feature,
			conf_arr[i]->code, conf_arr[i]->sort_str));
		conf_arr[i]->next = conf_arr[i + 1];
		conf_arr[i]->last = i ? conf_arr[i - 1] : 0;
		conf_arr[i]->sort_str = 0;
	}
	free(conf_arr); /* free temp space */
	free(sort_strings);
}

static
char *
make_sort_str(job, conf, sort_str)
LM_HANDLE *job;
CONFIG *conf;
char *sort_str;
{
    char pkg_flag;
    char counted_flag;
#define NODELOCKED '0' /* sort first */
#define NOT_NODELOCKED '1' /* sort 2nd */
    char nodelocked_flag = NOT_NODELOCKED;
    char datestr[DATE_LEN+1];
    char version[(MAX_VER_LEN * 2)+ 2];
    unsigned long ldate;
    int lf_num;
    int restricted = 0;
    int borrowable = 0;

#if XX_COMMENT_XX
/***********************************************************************/

	0) In the client, do not break up license -- keep $LM_LICENSE_FILE
		list order
 	   In vendor daemon, break up licenses.
	1) node-locked uncounted before counted for the same feature
	2) borrowable
	3) restricted (PLATFORM, etc.) before unrestricted.
	4) multiple FEATURE lines for the same feature --
				most current first.
	5) SUITE, DUP_NONE and standalone feature:
		 		SUITE .. components="c1 c2"
		 		FEATURE c1
				-- put standalone before component
			   [This solves one of the PACKAGE problems]

	Therefore, sort by
		sort number
		filenumber -- do not break up between files
		feature name
		conf->type (so increment follows feature )
		counted_flag (so uncounted are before counted )
		node-locked (so node-locked are before non-node-locked)
		borrowable
		restricted (platform=, etc.)
		pkg_flag-- indicates dup-none and standalone.
		version (if not feature line, so oldest increment versions
				are used first, if feature, reverse order)
		issued date
		file_order -- otherwise, maintain original order
/***********************************************************************/
#endif
	if (job->flags & LM_FLAG_IS_VD)
        lf_num = 0;
	else
        lf_num = conf->lf;
	if (!conf->package_mask &&
		((conf->type == CONFIG_FEATURE) ||
			(conf->type == CONFIG_INCREMENT)) &&
		(!(conf->lc_got_options & LM_LICENSE_DUP_PRESENT) ||
		conf->lc_dup_group == LM_DUP_NONE))
		pkg_flag = '1'; /* comes first */
	else
		pkg_flag = '2';
	if (!conf->users)
        counted_flag = '1'; /* comes first */
	else
        counted_flag = '2';
	if (conf->lc_issued)
        strcpy(datestr, conf->lc_issued);
	else if (*conf->startdate)
        strcpy(datestr, conf->startdate);
	else
        strcpy(datestr, l_asc_date(l_extract_date(job, conf->code)));
	if (conf->idptr)
        nodelocked_flag = NODELOCKED; /* sort first */
	if (*conf->version)
        make_ver_str(version, conf/*conf->version*/, job->flags);
	else
        *version = 0;
	ldate = ((unsigned long)-1) - (unsigned long)l_date_to_time(job, datestr);
	borrowable = (conf->lc_type_mask & LM_TYPE_BORROW)  ? 1 : 0;
	restricted = conf->lc_platforms ? 0 : 1; /* restricted a lower number */

	if (conf->lc_sort)
		sprintf(sort_str, "%02x%0x2%08x",
			conf->lc_sort, lf_num, conf->file_order);
	else
		sprintf(sort_str, "%02x%02x%-30s%03d%c%c%01d%01d%c%21.21s%08lx%08x",
		100, 			/* 1 -- 0 to ff, sort number*/
		lf_num,			/* 2 don't break up between files */
		conf->feature,  	/* 3 str */
		conf->type,		/* 4 digit */
		counted_flag,		/* 5 char */
		nodelocked_flag, 	/* 6 char  P5881 */
		borrowable,		/* 7 int */
		restricted,		/* 8 int */
		pkg_flag,		/* 9 char */
		version,		/* 10 str */
		ldate,			/* 11 issued date */
		conf->file_order);	/* 12 32-bit int */

	conf->sort_str = sort_str;

	return sort_str + (strlen(sort_str) + 1);
}

static
void
make_ver_str(char *buf, CONFIG *conf, int flags)
{
    char *cp, *bp = buf;
    char num[MAX_VER_LEN + 1], *ip = num;
    char frac[MAX_VER_LEN + 1], *fp = frac;
    int ilen, flen, i;
    char *ver = conf->version;
	/* find decimal point, or eostring */
	for (cp = ver; *cp && *cp != '.'; cp++)
		*ip++ = *cp;
	*ip = 0;
	/* num now has numeric part */
	if (*cp == '.')
        cp++;
	for (; *cp ;cp++)
		*fp++ = *cp;
	*fp = 0;
	/* frac now has fractional part */
	ilen = strlen(num);
	flen = strlen(frac);
	for (i = 0; i < (MAX_VER_LEN - ilen); i++)
		*bp++ = '0';
	strcpy(bp, num);
	bp += ilen;
	*bp++ = '.';
	strcpy(bp, frac);
	bp += flen;
	for (i = 0; i < (MAX_VER_LEN - flen); i++)
		*bp++ = '0';
	*bp = 0;
	if (conf->type == CONFIG_FEATURE && !(flags & LM_OPTFLAG_USE_ALL_FEATURE_LINES))/* reverse order */
	{
		for (bp = buf; *bp; bp++)
		{
			if (*bp == '.')
                continue;
			*bp = '9' - *bp;
		}
	}
}

/***********************************************************************
 *
 *	delete_dup_files:  added in v8
 * kmaclean Jan 27, 2003 - re-wrote the whole thing so :
 * 		1. it actually works
 * 		2. it compares full path named files with relitive path named files
 * 			and matches them if they are the same file.
 *
 *	This makes sure that the license file list has no dups, and
 *	solves many cases where programmers would getting duplicates like
 *	lc_userlist() entries and  so on.
 *	It also makes everything more efficient, so for example, it doesn't
 *	retry a bad server more than once
 *
 * Return
 * 		the number of paths in the path list
 * 		originalPathList will be changed to include no duplicates
 ************************************************************************/
static int delete_dup_files(LM_HANDLE *job, char *originalPathList)
{

	FileStatInfo*	fileInfoList = NULL;
    char *			tmpPathList;
	char *			tmpPtr;
	int				originalPathCount;
	int				newPathCount = 0;
	int				fileIndex;
	int				compareIndex;

	/* copy the string */
    tmpPathList = l_malloc(job, strlen(originalPathList) + 2);
	strcpy(tmpPathList, originalPathList);

	/* count the file names */
	originalPathCount = 1;
	for ( tmpPtr = tmpPathList; tmpPtr && *tmpPtr != '\0';  tmpPtr = getnextfile(tmpPtr) )
	{
		if ( *tmpPtr == PATHSEPARATOR )
		{
			tmpPtr++;
			originalPathCount++;
		}
	}
	newPathCount = originalPathCount;
	if ( originalPathCount <= 1 )
		goto cleanup;	/* there is only one name in the path. get out */

	fileInfoList = l_malloc(job, sizeof(FileStatInfo) * originalPathCount);
	memset(fileInfoList, 0, sizeof(FileStatInfo) * originalPathCount);


	tmpPtr = tmpPathList;
	fileIndex = 0;
	do
	{
		/* build a list of the names in the path. */
		if ( *tmpPtr == PATHSEPARATOR )
		{
			/* found the licese file path separator. step over it */
			*tmpPtr = '\0';
			tmpPtr++;
		}
		if ( *tmpPtr == '.' && *(tmpPtr + 1) == PATHTERMINATOR )
		{
			/* we found './' (UNIX) or '.\' (PC) indicating the current
			 * directory. strip it off first */
			tmpPtr += 2;
		}
		if ( *tmpPtr )
		{
			/* After all that there is still a char here. copy the pointer
			 * into the fileInfoList */
			fileInfoList[fileIndex].name = tmpPtr;
			fileIndex++;
		}
	} while ( (tmpPtr = getnextfile(tmpPtr)) != NULL);

	/* loop through the list looking for duplicate names */
	for ( fileIndex = 0 ; fileIndex < originalPathCount; fileIndex++ )
	{
		int		foundDup ;

		if (fileInfoList[fileIndex].name == NULL)
			continue;  /* this one was deleted earlier */

		/* */
		for (compareIndex = fileIndex + 1; compareIndex < originalPathCount; compareIndex++ )
		{
			/* compare the first name with every following name in the list */
			if (fileInfoList[compareIndex].name == NULL)
				continue;  /* this one was deleted earlier */

			foundDup = 0;
			if ( compareFileNamePortion(fileInfoList[fileIndex].name, fileInfoList[compareIndex].name) == 0 )
			{
				/* the file portion of the path matches */
				/* try comparing the entire string */
				if (strcmp(fileInfoList[fileIndex].name, fileInfoList[compareIndex].name) == 0)
				{
					/* entire string matches. found a duplicate. */
					foundDup = 1;
				}
				else if (compareStats(&fileInfoList[fileIndex], &fileInfoList[compareIndex]))
				{
					/* the stats matched. that's good enough for me */
					foundDup = 1;
				}
			}
			if ( foundDup )
			{
				/* found a duplicate. delete it */
				fileInfoList[compareIndex].name = NULL;
				newPathCount--;
			}
		}
	}
	/* rebuild the original license file path from the string pointer array */
	tmpPtr = originalPathList;
	for ( fileIndex = 0 ; fileIndex < originalPathCount; fileIndex++ )
	{
		if (fileInfoList[fileIndex].name != NULL)
		{
			strcpy (tmpPtr,fileInfoList[fileIndex].name);
			tmpPtr += strlen(fileInfoList[fileIndex].name);
			*tmpPtr++ = PATHSEPARATOR;
		}
	}
	tmpPtr--;
	*tmpPtr = '\0';

	/* clean up */
cleanup:
	if (tmpPathList != NULL)
		free(tmpPathList);
	if ( fileInfoList != NULL )
		free (fileInfoList);

	/* return the number of file names in the new path */
	return newPathCount;

/*
    char *cp, *cp2;
    char *cur, *cur2;

   for (cur = paths; cur ; cur = getnextfile(cur))
	{
		if (*cur == PATHSEPARATOR)
            cur++;
		for (cp = tmp; *cp && (*cp != PATHSEPARATOR); cp++)
			;
		*cp = 0;
		cur2 = cur;
		if ((cur2 = getnextfile(cur2)) != NULL)
		{
			do
			{
				if (*cur2 == PATHSEPARATOR)
        	        cur2++;
				cp2 = cur2;
				if (!strncmp(cp2, tmp, strlen(tmp)))
				{
        	        char *next;

					next = cur2;
					cp = getnextfile(cur2);

					if (cp)
					{
						if (*cp == PATHSEPARATOR)
						{
							cp++;
						}
						while (*cp)
        	                *cur2++ = *cp++;
						*cur2 = 0;
						cur2 = next;
					}
					else
					{
        	            cur2--;
						*cur2 = 0;
						cur2 = NULL;
					}
				}
				else
					cur2 = getnextfile(cur2);
			}   while (cur2);
		}
	}
	free (tmp);
 */
}

/***********************************************************************
 * compare the file name portion of two paths to see if they are equal
 * called by delete_dup_files()
 *
 * Return:
 * 		1 - equal
 * 		0 - no match
 ************************************************************************/
static int compareFileNamePortion(char * name1, char * name2)
{
	char *newName1;
	char *newName2;

	/* need to check for port @ host */

	newName1 = getFileFromPath(name1);
	newName2 = getFileFromPath(name2);
 	return strcmp(newName1, newName2);
}

/***********************************************************************
 * get the file name portion from a path name
 * Return the pointer to it
 ************************************************************************/
static char *getFileFromPath(char * name)
{
	char *prev;
	char *next;

	next = prev = name;

	while ( *next )
	{
		while ( *next && *next != PATHTERMINATOR )
			next++;
		if ( *next ==  PATHTERMINATOR)
		{
			next++;
			prev = next;
		}
		else
		{
			/* hit the end of the string so prev is the file name portion */
			return prev;
		}
	}
	return prev;
}

/***********************************************************************
 * compare stat structures to see if two files are the same
 * called by delete_dup_files()
 *
 * Return:
 * 		1 - the stats match
 * 		0 - no match
 ************************************************************************/
static int compareStats(FileStatInfo *fileStat,FileStatInfo *compareStat)
{
	/* As a last resort do a stat()
	 * and compare the data to see if they are the same file. */
	if ( fileStat->badStat || compareStat->badStat)
	{
		/* we tries earlier to get the stat on one of these.
		 * Don't need to try again. return no match */
		return 0;
	}

	if ( ! fileStat->didStat)
	{
		/* did not do stat already */
		if ( l_flexStat(NULL, fileStat->name, &(fileStat->fileStat)) != 0)
			fileStat->badStat  = 1;
		else
			fileStat->didStat  = 1;
	}
	if ( ! compareStat->didStat)
	{
		/* did not do stat already */
		if (l_flexStat(NULL, compareStat->name, &(compareStat->fileStat)) != 0)
			compareStat->badStat = 1;
		else
			compareStat->didStat = 1;
	}
	if ( fileStat->badStat || compareStat->badStat)
	{
		/* we tried earlier, and failed,  to get the stat on one one of these.
		 * Don't need to try again. return no match */
		return 0;
	}
	if ( fileStat->fileStat.st_ino != 0 &&  (fileStat->fileStat.st_ino == compareStat->fileStat.st_ino  ))
	{
		/* on unix the inodes will match if they are the same */
		return 1;
	}
	if ( fileStat->fileStat.st_mtime  == compareStat->fileStat.st_mtime  &&
		fileStat->fileStat.st_ctime  == compareStat->fileStat.st_ctime  &&
		fileStat->fileStat.st_size  == compareStat->fileStat.st_size)
	{
		/* modification time, creation time, and size all match.
		 * Call it a match */
		return 1;
	}
	/* no match */
	return 0;
}


/*
 *	delete_dup_feats:  see delete_dup_files above.
 */
static
void
delete_dup_feats(LM_HANDLE *job)
{
    CONFIG *c, *c2, *last, *next;

	for (c = job->line; c; c = c->next)
	{
		if (c->type == CONFIG_PORT_HOST_PLUS)
            continue;
		for (last = 0, c2 = c->next; c2; c2 = next)
		{
			if (l_keyword_eq(job, c2->feature, c->feature)
				&& !strcmp(c2->hash, c->hash))
			{
				/* remove the dup */
				next = c2->next;
				if (next)
                    next->last = last;
				if (last)
                    last->next = next;
				l_free_conf(job, c2);
			}
			else
			{
				last = c2;
				next = c2->next;
			}

		}
	}

}
