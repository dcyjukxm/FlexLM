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
 *	Module: $Id: lm_free_job.c,v 1.33 2003/03/11 00:07:40 jwong Exp $
 *
 *	Description: 	Free a job created by lm_init, along with all
 *			associated malloc'd memory
 *
 *	D. Birns
 *	2/15/94
 *
 *	Last changed:  12/23/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_timers.h"

#include "composite.h"

#ifdef USE_WINSOCK  
#include "pcsock.h"
#endif

#if defined(PC) && !defined(WINNT) && !defined(NLM) && !defined(OS2)
extern pc_timer_delete( LM_HANDLE *job );
#endif


/*
 *	static func definitions here
 */
static void free_job_daemon 	lm_args((LM_DAEMON_INFO *));
static void free_job_options  	lm_args((LM_OPTIONS *));
static void free_job_packages  	lm_args((LM_HANDLE *, CONFIG *));
static void free_job_redirect 	lm_args((LM_HOSTID_REDIRECT *));
static void free_job_lic_files	lm_args((char **));
static void free_job_msgq	lm_args((MSGQUEUE *));
static void free_job_vendorids 	lm_args((LM_HANDLE *, LM_VENDOR_HOSTID *));
static void free_job_conf_servers lm_args((LM_HANDLE *));
static void free_job_key_filters lm_args((LM_HANDLE *));
#ifdef SUPPORT_FIFO 
static void free_job_local	lm_args((LM_LOCAL_DATA *));
#endif
/*
 *	The servers are separately allocated for each license file
 *	The "line" field in the job points to all the FEATURE lines
 *	Each one has a pointer to a server, along with an index
 *	number.  We want to make sure we free each one, but only 
 *	once.  last_server_freed keeps track of this.  It serves
 *	as a bitmask, one bit for each license in LM_LICENSE_FILE.
 */


/*
 *	lc_free_job 
 */

typedef struct _l_list_type {
	struct _l_list_type *next;
	int junk;
} L_LIST_TYPE;

void
l_free_list(void *v)
{
  L_LIST_TYPE *next, *l;

	l = (L_LIST_TYPE *)v;

	while (l)
	{
		next = l->next;
		free(l);
		l = next;
	}
}	


void API_ENTRY
lc_free_job(job)
LM_HANDLE *job;
{
	l_mt_lock(job, __FILE__, __LINE__);
	/* sanity check */
	if ((!job) || (job->type != LM_JOB_HANDLE_TYPE))
	{
		if (!job->lm_errno)  
		{
			LM_SET_ERRNO(job, LM_BADPARAM, 166, 0);
		}
		return;
	}
	lc_disconn(job, 1);

/*
 *	At this point we take it on faith that this is a legitimate
 *	job, created by lm_init().  In the future, we may want to check it
 *	definitively by calling lm_first_job()/lm_next_job(), available
 *	from lm_init.c.
 */

/*
 *	First, unlink the job
 */
	if (job->first_job == job)
	{
/*
 *		First job.  Just make the "next" job the first job
 *		and update the "first job" pointer in each job in the list.
 */
		if (job->next)
		{
		  LM_HANDLE *j = job->next, *f = j;

			while (j)
			{
				j->first_job = f;
				j = j->next;
			}
		}
#ifdef USE_WINSOCK
		else
		{
/*
 *			At this point, we know that 'job' is the only job
 *			in this job list.  It is time to call WSACleanup()
 *			to match the WSAStartup() called in lm_init().
 */
#if !defined(NLM)			
extern FILE * our_std_error;
/* P2909		WSACleanup();
*/		
		if (our_std_error) 
		{
			fflush(our_std_error);
			fclose(our_std_error);
		}
		our_std_error=0;

#endif /* NLM */
		}
#endif		
	}
	else
	{
	  LM_HANDLE *j = job->first_job, *l;

/*
 *		Job in the middle of the list.  Walk the list and fix
 *		up the links
 */
		while (j && j != job)
		{
			l = j;		/* can't be the first so l always set */
			j = j->next;
		}
		if (j && l) l->next = j->next;	/* Unlink job */
	}


/*	
 *	stop and free all timers for this job
 *	do this early on, since timers may refer to the job
 */
#if defined(PC) && !defined(WINNT) && !defined(NLM) && !defined(OS2)
	pc_timer_delete(job); 
#else
	l_timer_job_done(job);
#endif
#ifdef PC
	l_conn_harvest(job);
#endif

	l_free_job_featdata(job, job->featdata);
	job->featdata = NULL;
	l_free_job_license(job);

		
	if (job->last_udp_msg) free(job->last_udp_msg);
	if (job->savemsg) 	free(job->savemsg);
	if (job->recent_reconnects) free(job->recent_reconnects);
	l_free_err_info(&job->err_info);
	if (job->mt_info) 
		l_mt_free(job);
	free_job_msgq(job->msgq_free);
	free_job_msgq(job->msgqueue);
	free_job_vendorids(job, job->options->vendor_hostids);
	free_job_daemon(job->daemon);
#ifdef UNIX
	if (job->rcfile) free(job->rcfile);
	if (job->borrfile) free(job->borrfile);
#endif /* UNIX */
	free_job_options(job->options);
	l_free_list (job->borrow_stat); job->borrow_stat = 0;
	lc_free_hostid(job, job->idptr);
	if (job->mem_ptr1) free(job->mem_ptr1);
	if (job->path_env) free(job->path_env);
	if (job->vd_path_env) free(job->vd_path_env);
	if (job->curr_registry) free(job->curr_registry);

	/* clean up pointers allocated for composite stuff */
	if (job->composite_init) 
	{
		l_free(job->composite_init->info_list);
		l_free(job->composite_init);
	}

	if (job->keymem) free(job->keymem);
	if (job->keymem3) 
	{
	  typedef struct _keymem {
		void (*cleanup)(char *);
		char *context;
		char *mem;
	  } KEYMEM;
	  KEYMEM *keymem;

		keymem = (KEYMEM *)job->keymem3;
		(*keymem->cleanup)(keymem->context);
		free(keymem->context);
		free(keymem->mem);
		free(keymem);
	}
	free_job_key_filters(job);

/* 
 *	a precaution against calling this routine twice:
 */
	job->type = 0;
	(void) free(job);
	job = 0;

}
static
void
free_job_key_filters(job)
LM_HANDLE *job;
{
  L_KEY_FILTER *f = (L_KEY_FILTER *)job->key_filters, *next;
	for (; f; f = next)
	{
		next = f->next;
		free(f);
	}
}

void
l_free_job_license(job)
LM_HANDLE *job;
{
	if (job->lic_file_strings) 
	{
		free(job->lic_file_strings);
		job->lic_file_strings = 0;
	}
	free_job_lic_files(job->lic_files);
	job->lic_files = 0;
	l_free_job_lf_pointers(job);
	job->license_file_pointers = 0;
	if (job->featureset)
	{
		free(job->featureset);
		job->featureset = 0;
	}
	if (job->features) 	
	{
		free(job->features);
		job->features = 0;
	}
	if (job->feat_ptrs) 	
	{
		free(job->feat_ptrs);
		job->feat_ptrs = 0;
	}
	l_free_job_servers(job, job->servers);
	job->servers = 0;
	l_free_job_conf(job, job->line);
	free_job_conf_servers(job);
	job->line = 0;
	free_job_redirect(job->redirect);
	job->redirect = 0;
	free_job_packages(job, job->packages);
	job->packages = 0;
}

/*
 *	free_job_daemon 
 *			Note:  we don't free the daemon->server here,
 *				because this is the same as the job's
 *				server pointer, and gets free'd earlier.
 */
static 
void 
free_job_daemon(daemon)
LM_DAEMON_INFO *daemon;
{
  LM_DAEMON_INFO *d = daemon, *next;

	if (!daemon) return;
	
	do {
		next = d->next;
		free(d);
		d = next;
	} while (d);
}

/*
 *	free_server
 */

void API_ENTRY
l_free_server(job, server)
LM_HANDLE *job;
LM_SERVER *server;
{
  LM_SERVER *s;
        for (s = server; s; s = s->next)
        {
		if (s->idptr)
			lc_free_hostid(job, s->idptr);
        }
	if (server) 
	{
                if (server->filename) free(server->filename);
                free(server);
	}
}
/*
 *	free_job_msgq
 */
static
void
free_job_msgq(msgq)
MSGQUEUE *msgq;
{
  MSGQUEUE *mp, *next;

	if (mp = msgq)
	{
		do {
			next = mp->next;
			free(mp);
			mp = next;
		} while (mp);
	}
}
		

/*
 *	free_job_options
 */

static
void 
free_job_options(options)
LM_OPTIONS *options;
{
	if (!options) return;

	if (options->config_file)	
	{
		free(options->config_file);
	}
#ifdef WINNT 
	if (options->lcm_url) l_free(options->lcm_url);
#endif /* WINNT */

	free(options);
}

/*
 *	free_job_redirect
 */

static
void
free_job_redirect(redirect)
LM_HOSTID_REDIRECT *redirect;
{
  LM_HOSTID_REDIRECT *r = redirect, *next;
	
	if (!redirect) return;

	do {
		next = r->next;
		free(r);
		r = next;
	} while (r);
}


/*	
 *	free_job_lic_files
 */

static 
void 
free_job_lic_files(lic_files)
char **lic_files;
{

	if (lic_files) 
	{
		free(lic_files);
	}
}

#ifdef SUPPORT_FIFO 
/*
 *	free_job_local
 */

static
void
free_job_local(local)
LM_LOCAL_DATA *local;
{
	if (!local) return;

/*	
 *	We ignore errors on close or unlink -- they don't really matter
 *	if they're not valid
 */

	if (local->rfd >=0) (void) close(local->rfd);
	if (local->wfd >=0) (void) close(local->wfd);

	if (local->writename)
	{
		(void) unlink(local->writename);
		free(local->writename);
	}

	if (local->readname)
	{
		(void) unlink(local->readname);
		free(local->readname);
	}
	free(local);
}
#endif /* SUPPORT_FIFO */

/*
 *	free_job_lf_pointers
 */
void
l_free_job_lf_pointers(LM_HANDLE *job)
{
  LICENSE_FILE *tmp = job->license_file_pointers;

	if (!tmp) return;
	while (tmp)
	{
		if (tmp->type == LF_STRING_PTR)
			(void) free(tmp->ptr.str.s);
		else if (tmp->type == LF_FILE_PTR)
			(void) l_lfclose(tmp);
		if (tmp->bufsize && tmp->buffer)
			free(tmp->buffer);
		tmp = tmp->next;
	}
	free(job->license_file_pointers);
	job->license_file_pointers = 0;
}

void API_ENTRY
l_free_job_servers(job, servers)
LM_HANDLE *job;
LM_SERVER *servers;
{
  LM_SERVER *s, *n;
	for(s = servers; s; s = n)
	{
		n = s->next;
/*
 *		Can't use l_free_server here, because
 *		there's 2 different places where servers are
 *		stored -- job->servers and conf->servers, and
 *		they're malloc'd differently
 */
		lc_free_hostid(job, s->idptr);
                if (s->filename) (void) free(s->filename);
	}
	if (servers) free(servers);
	job->servers = 0;
}
/*
 *	l_free_job_conf
 */

void
l_free_job_conf(job, conf)
LM_HANDLE *job;
CONFIG *conf;
{

  CONFIG *c  = conf, *next;
	
		
	if (!conf)  /* still may have to free job->servers */
	{
		return;
	}

	do {
		next = c->next;
		l_free_conf(job, c);
		c = next;
	} while (c);
}
void API_ENTRY 
lc_free_hostid(job, id)
LM_HANDLE *job;
HOSTID *id;
{
  HOSTID *next, *idp;
  extern void l_free_one_hostid lm_args((HOSTID_PTR));

	for (idp = id; idp; )
	{
		next = idp->next;
		l_free_one_hostid(idp);
		idp = next;
	}
}

/*
 *	free_job_packages
 */
static
void
free_job_packages(job, pkg)
LM_HANDLE *job;
CONFIG *pkg;
{
  CONFIG *next;
   
	while (pkg)
	{
		next = pkg->next;
		l_free_conf(job, pkg);
		pkg = next;
	}
}
static
void
free_job_vendorids(job, h)
LM_HANDLE *job;
LM_VENDOR_HOSTID *h;
{
  LM_VENDOR_HOSTID *next;

	while (h)
	{
		next = h->next;
		free(h);
		h = next;
	}
}	

static
void
free_job_conf_servers(LM_HANDLE *job)
{
  LM_SERVER_LIST *next, *p;
	for (p = job->conf_servers; p; p = next)
	{
		next = p->next;
		l_free_server(job, p->s);
		free(p);
	}
	job->conf_servers = 0;
}
void
l_free_keylist(job, keylist)
LM_HANDLE *job;
LM_KEYLIST *keylist;
{
  LM_KEYLIST *next, *k;
	for (k = keylist; k; k = next)
	{
		next = k->next;
		if (k->key) free(k->key);
		free(k);
	}
}
#ifdef SUPPORT_METER_BORROW
void
l_free_job_meters(job)
LM_HANDLE *job;
{
  LM_METERS *d, *next;
	l_borrow_close(job);
	for (d = job->meters; d; d = next)
	{
		next = d->next;
		lc_borrow_free_meter(job, d);
	}
	job->meters = 0;
}

#endif /* SUPPORT_METER_BORROW */
