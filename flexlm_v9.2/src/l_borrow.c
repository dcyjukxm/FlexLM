/*****************************************************************************e

	    COPYRIGHT (c) 2001, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_borrow.c,v 1.96 2003/06/17 20:40:41 sluu Exp $
 *
 *	D. Birns
 *	1/01
 */

#include "lmachdep.h"
/* 
 *	Format of the borrow info is
 *	key is "borrow"<feature> where <feature> is the feature name
 *	value is:
 *		start time unix time format in hex
 *		L_BORROW_SEPARATOR
 *		exp time for the borrow period, in seconds offset
 *			from the expiration time.
 *		L_BORROW_SEPARATOR
 *		hostid -- native for unix, vol-serial for pc
 *		L_BORROW_SEPARATOR
 *		server hostids separated by L_BORROW_ID_SEP when redundant
 *		L_BORROW_SEPARATOR
 *		FEATURE line from license
 */

#include "lmclient.h"
#include "lm_comm.h"
#include "l_prot.h"
#include "l_borrow.h"
#include "errno.h"
#include <time.h>
#include "l_rand.h"
#ifndef RELEASE_VERSION
static long cryptclear = -1L;
#endif
typedef unsigned int (*_borrowid) lm_args((LM_VOID_PTR, LM_VOID_PTR, int , 
				unsigned char *, int, int *));

#define INVALID_BHOSTID "Zz"
static char * bget(LM_HANDLE *job, char **preg);
static int bgetx(LM_HANDLE *job, char **preg, long *l);
static void infoborrow(LM_HANDLE *job, CONFIG *conf, char *);
static unsigned char * server_bcrypt(LM_HANDLE *job, unsigned char *reg, unsigned short salt, int len);
unsigned char *(*l_borrow_dptr)(LM_HANDLE *, unsigned char *, int , int) = NULL;
unsigned char * l_borrow_decrypt(LM_HANDLE *job, unsigned char *str, int len, int);
#define MAX_BCRYPT_LEN 120 /* so it always fits in a msg */

int l_borrow_in_seconds = 0; /* used for testsuite */

/*
 *	l_borrow -- create the local borrow file
 *			This happens after we've successfully checked
 *			out a feature, and indicated we wanted this
 *			checkout to borrow the license.
 */
void
l_borrow(LM_HANDLE *job, char *feature, CONFIG *conf)
{
  char feat[MAX_CONFIG_LINE + 1];
  char parentfeat[MAX_CONFIG_LINE + 1];
  char parentpkg[MAX_CONFIG_LINE + 1];
  char serverids[(MAX_HOSTID_LEN * 3) + 10 ];
/*
 *	borrow_data: 	This variable will hold all the local borrow information
 *		it gets set, encrypted and stored
 */
  unsigned char borrow_data[(MAX_CONFIG_LINE * 3) + 20];
  char key[MAX_FEATURE_LEN+100];
  long t;
  unsigned short salt;
  int len = job->options->max_license_len;
  LM_SERVER *s;
  char *cp;
  char hostid1[MAX_CONFIG_LINE + 1];
  char hostid2[MAX_CONFIG_LINE + 1];
  int reglen;
  long lingert = 0;
  long exp;
  unsigned int b_id;
  char b_id_buf[9];
  _borrowid borrowid = (_borrowid)job->l_new_job;
  int i;
  int featname_4bytes;
  int eval = 0;

	memset(borrow_data, 0, sizeof(borrow_data));
/* 
 *	First we checkin the license -- it's lingering at the server side
 *	NOTE:  If the borrow fails at this point, the user has lost a 
 *		license, because it's still being consumed at the server side.
 *		Therefore, all possible errors should be anticipated elsewhere.
 *		For example, ls_checkout.c should detect errors that occur
 *		because of the license file.  lc_checkout() should detect
 *		any other errors.
 */
	l_checkin(job, feature, 1);

	job->flags |= LM_FLAG_BORROWED_CHECKOUT;

	if (!(job->options->flags & LM_OPTFLAG_BORROW_OK) )
	{
		/* lc_checkout shd have found this */
		LM_SET_ERRNO(job, LM_NOBORROWSUPP, 548, 0);
		return;
	}
	if (!(conf->lc_type_mask & LM_TYPE_BORROW))
	{
		/* ls_checkout shd have found this */
		LM_SET_ERRNO(job, LM_NOBORROWSUPP, 558, 0);
		return;
	}
	if (((int)job->borrow_linger_minutes) * (l_borrow_in_seconds ? 60 : 1)
		> (conf->lc_max_borrow_hours * 60))
	{
	  char buf[100];
		/* ls_checkout shd have found this */
		sprintf(buf, "%d > %d", job->borrow_linger_minutes/60,
			conf->lc_max_borrow_hours);
		LM_SET_ERROR(job, LM_BORROW_TOOLONG, 559, 0, buf, 
					LM_ERRMASK_ALL);
		return;
	}
  
	
	*serverids = 0;
/*
 *	We must be able to re-authenticate the CONFIG struct, so
 *	we need the server hostids to do this.
 */
	for (cp = serverids, s = conf->server; s; s = s->next)
	{
		if (!s->idptr) break;
		if (*serverids) 
		{
			strcat(cp, L_BORROW_ID_SEP);
			cp += strlen(L_BORROW_ID_SEP);
		}
		strcpy(cp, l_asc_hostid(job, s->idptr));
		cp += strlen(cp);
	}
/* 
 *	to force l_print_config to print the whole feature line without
 *	newlines
 */
	job->options->max_license_len = -1; 

	if (l_print_config(job, conf, feat))
	{
		/* shouldn't happen */
		LM_SET_ERRNO(job, LM_INTERNAL_ERROR, 547, 0);
		return;
	}
/* 
 *	We have to be able to borrow packages, and in order to do that,
 *	we need the parent config and pkg info
 */
	if (conf->parent_feat)
	{
		if (l_print_config(job, conf->parent_feat, parentfeat))
		{
			LM_SET_ERRNO(job, LM_INTERNAL_ERROR, 547, 0);
			return;
		}
		if (l_print_config(job, conf->parent_pkg, parentpkg))
		{
			LM_SET_ERRNO(job, LM_INTERNAL_ERROR, 547, 0);
			return;
		}
	}
/*
 *	When we use the borrow file, we must ensure that the client is
 *	on the same hostid it was when it checked out the license.
 *	Otherwise, it would be easy to copy these borrow files around
 *	or put them in nfs locations accessible from many clients
 *	We use the default hostid for this, and disk-serial num on PCs.
 *	The reason for disk-serial num is that it's always there, in the
 * 	case of laptops where ether hostid may go away.
 */
	if (l_hostid(job, HOSTID_DEFAULT, hostid1))
	{
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 557, 0);
		return;
	}
#ifdef PC
	if (l_hostid(job, HOSTID_DISK_SERIAL_NUM, hostid2))
#endif
		strcpy(hostid2, INVALID_BHOSTID);

	job->options->max_license_len = len; /* restore it */


	{
	  static int j = 0;
		srand(l_now() + (j<<20));
		rand(); rand(); rand(); rand(); 
		salt = rand() & 0xffff;
	}
	t = time(0);


	lingert = job->borrow_linger_minutes * (l_borrow_in_seconds ? 1 : 60); 
	exp = lingert + t;

/*
 *	local Borrow data.  Print the string in ascii format
 */
	sprintf((char *)borrow_data, "%lx%s%x%s%s%s%s%s%s%s%s%s%s%s%s\n",  
		t, L_BORROW_SEPARATOR, 		/* current time */
		lingert, L_BORROW_SEPARATOR, 	/* linger seconds */
		hostid1, L_BORROW_SEPARATOR, 	/* my hostid */
		hostid2, L_BORROW_SEPARATOR, 	/* my hostid */
		serverids, L_BORROW_SEPARATOR,  /* CONFIG server id(s) */
		feat,				/* ascii FEAT/INCR line */
		conf->parent_feat ? L_BORROW_SEPARATOR : "",
		conf->parent_feat ? parentfeat : "", /* ascii parent feat */
		conf->parent_feat ? L_BORROW_SEPARATOR : "",
		conf->parent_feat ? parentpkg : ""/* ascii parent pkg */
			);
#ifndef RELEASE_VERSION
	if (cryptclear == -1L)
		cryptclear = ((long)l_real_getenv("FLEXLM_BCLEAR"));	/* overrun checked */
#endif /* RELEASE_VERSION */
	reglen = strlen((char *)borrow_data);
/*
 *	borrowid is a value returned from the lm_new.c file
 *	It's used as a kind of handshake with the server
 */
	b_id = borrowid(0, 0, 4, 0, 0, 0); /* the 4 indicates get borrowid */
	if (!b_id) 
	{
		eval = 1;
		b_id = 1234;
	}
	b_id ^= L_BORROW_MAGIC; /*- obfuscation from l_borrow.h */
	sprintf(b_id_buf, "%x", b_id);

#ifndef RELEASE_VERSION
	if (!cryptclear)
#endif
	if (!job->L_BORROW_STRING) 
	{
/*
 *		L_BORROW_STRING not set, we're doing the encryption
 *		and storage
 */
	  char buf[MAX_FEATURE_LEN + 1];
	  int l  = strlen((char *)borrow_data);
		memset(buf, 0, sizeof(buf));
		strcpy(buf, feature);
		memcpy(&featname_4bytes, buf, sizeof(int));
/* 
 *		SERVER_BCRYPT:
 *			Let the server add a layer of encryption to the
 *			'borrow_data' string.  We do this for 1 to 7 times.  That's
 *			what the next for loop controls.  The 1 to 7 times
 *			will be different for each for vendor, and each
 *			feature.  The featname_4bytes part will be 
 *			different for each feature.
 */
		for (i = 0; i < ((b_id ^ L_BORROW_MAGIC2 ^ featname_4bytes) % 7) + 1; i++)
		{
			if (!server_bcrypt(job, borrow_data, 
				(unsigned short)(salt + i), l))
			{
				LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 565, 0);
				return;
			}
/*
 *			Borrowid called twice:
 *				o first time (arg 5) it encrypts each byte
 *				o second time (arg 7) it reorders the bytes 
 *				  in the string
 */
			if (!eval)
			{
			borrowid(0, 0, 5, borrow_data, reglen, 0); 
			borrowid(0, 0, 7, borrow_data, reglen, 0);
			}
		}
	}


	sprintf(key, "borrow-%s-%s", b_id_buf, feature);
#ifdef PC
	reglen += 1; /* for null terminator */
#endif
	if (job->L_BORROW_STRING)
		sprintf(job->L_BORROW_STRING, "%s=%s", key, borrow_data);
	else
	{
	  char reg2[(MAX_CONFIG_LINE * 3) + 100];
		sprintf(reg2, "%04x%08lx", salt, exp);
		memcpy(&reg2[12], borrow_data, reglen + 1);
		l_set_registry(job, key, reg2, reglen + 12, 1);
	}
	infoborrow(job, conf, b_id_buf);
}
static
unsigned 
char *
server_bcrypt(LM_HANDLE *job, unsigned char *borrow_data, unsigned short salt, int l)
{


  int m;
  int i, k;
  char msg[LM_MSG_LEN + 1];
  char type;
  char *rcv;
  char ret[MAX_CONFIG_LINE + 20];

	memset(ret, 0, sizeof(ret));
	for (k=0, i=l ; i > 0; k += MAX_BCRYPT_LEN, i -= MAX_BCRYPT_LEN)
	{
		memset(msg, 0, sizeof(msg));
		msg[MSG_VD_INFO_PARAM-MSG_DATA] =  LM_VD_BORROW;
		if (i < MAX_BCRYPT_LEN) m = i;
		else 			m = MAX_BCRYPT_LEN;

		l_encode_int(&msg[MSG_VD_INFO_BMSG_LEN-MSG_DATA], m);
		l_encode_16bit_packed(&msg[MSG_VD_INFO_BMSG_SALT-MSG_DATA],  
							salt);
		memcpy(&msg[MSG_VD_INFO_BMSG - MSG_DATA], &borrow_data[k], m);
		
		if (!l_sndmsg(job, LM_VENDOR_DAEMON_INFO, msg))
			return 0;
		if (!l_rcvmsg(job, &type, &rcv) || type != LM_OK)
			return 0;
		memcpy(&ret[k], &rcv[MSG_VD_INFO_BMSG-MSG_DATA], m);
	}
	memcpy(borrow_data, ret, l);
	return borrow_data;
}

/*
 *	l_ckout_borrow:  Attempt checkout from local borrow file
 */

l_ckout_borrow( LM_HANDLE_PTR job,		
	const LM_CHAR_PTR feature,	
	const LM_CHAR_PTR version,	
	int nlic,		/* unused for now */
	int flag,		/* queueing flag -- unused */
	const VENDORCODE_PTR key, /* The vendor's key */
	int dup_group)		/* unused */
{
  int ret = 0;
  int i;
  char regkey[MAX_FEATURE_LEN+20];
  unsigned char *borrow_data = 0;
  long exp = 0;
  long start = 0;
  char *feat;
  char *parent;
  char *pkg;
  CONFIG *conf = 0;
  CONFIG *parentconf = 0;
  CONFIG *pkgconf = 0;
  char *idstr;
  int servercnt = 0;
  char *servers[3];
  LM_SERVER *s = 0, *s_sav = 0;
  char *servermem;
  int package = 0;
  LM_SERVER_LIST_PTR slp;
  char *hostid1 = 0;
  char *hostid2 = 0;
  int reglen = 0;
  unsigned int salt;
  char *regmem = 0;
  unsigned int b_id;
  char b_id_buf[9];
  _borrowid borrowid = (_borrowid)job->l_new_job;
  HOSTID *h1 = 0;
  HOSTID *h2 = 0;
  HOSTID *h = 0;
  int featname_4bytes;
  static char *perr = (char *)-1;
  int eval = 0;

	if (perr == (char *)-1)
		perr = getenv("L_BORROW_ERR");	/* overrun checked */
	if (l_baddate(job)) /* P 6189 */
	{
		ret = job->lm_errno;;
		goto errexit_l_ckout_borrow;
	}


	if (!(job->options->flags & LM_OPTFLAG_BORROW_OK))
	{
		LM_SET_ERRNO(job, LM_NOBORROWSUPP, 551, 0);
		ret = LM_NOBORROWSUPP;
		goto errexit_l_ckout_borrow;
	}

	/*
	 *	Initialize feature db
	 */
	if (job->line == (CONFIG *) NULL)
	{
		l_init_file(job);
		/*
		 *	Read data from registry and put into list.
		 */
		if(job->l_new_job)
			l_read_borrow(job, feature);
	}

	b_id = borrowid(0, 0, 4, 0, 0, 0); /* the 4 indicates get borrowid */
	if (!b_id)
	{
		eval = 1;
		b_id = 1234;
	}
	b_id ^= L_BORROW_MAGIC;
	if (b_id) sprintf(b_id_buf, "%x", b_id);
	else *b_id_buf = 0;

	sprintf(regkey, "borrow-%s-%s", b_id_buf, feature);
	if (job->L_BORROW_STRING && *job->L_BORROW_STRING)
	{
		borrow_data = (unsigned char *)job->L_BORROW_STRING;
		borrow_data = (unsigned char *)strchr((char *)borrow_data, '=');
		if (borrow_data) borrow_data++;
	}
	else
	{
	  char *cpreg = 0;
		l_get_registry(job, regkey, &cpreg, &reglen, 1);
#ifdef PC
		reglen -= 1;
#endif
		if (cpreg)
		{
/*
 *			Note:  The registry contains 2 clear text
 *			fields at the start:  the 'salt' and the
 *			expiration time.  The expiration time is unused.
 *			Total length is exactly 12 characters for these
 *			2 fields.
 */
			regmem = l_malloc(job, reglen + 1);
			memcpy(regmem, cpreg, reglen);	/* memory threat */
			regmem[reglen] = 0;
			sscanf(regmem, "%04x", &salt);	/* overrun threat */
			reglen -= 12;
			borrow_data = (unsigned char *)regmem + 12;
		}
	}
	if (!borrow_data)
	{
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 554, 0);
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (!l_borrow_dptr)
	{
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 566, 0);
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	/* get each field:  exp */

#ifndef RELEASE_VERSION
	if (cryptclear == -1L)
		cryptclear = ((long)l_real_getenv("FLEXLM_BCLEAR"));	/* overrun checked */
	if (!cryptclear)
#endif /* RELEASE_VERSION */
	if (!job->L_BORROW_STRING) 
	{
	  char buf[MAX_FEATURE_LEN + 1];
	  int times;
		memset(buf, 0, sizeof(buf));
		strcpy(buf, feature);
		memcpy(&featname_4bytes, buf, sizeof(int));
		times = ((b_id ^ L_BORROW_MAGIC2 ^ featname_4bytes) % 7) + 1;

		for (i = 0; i < times; i++)
		{
/*
 *			Borrowid called twice:
 *				o first time (arg 8) it reorders the bytes 
 *				  in the string
 *				o second time (arg 6) it encrypts each byte
 */

			if (!eval)
			{
			borrowid(0, 0, 8, borrow_data, reglen, 0); 
			borrowid(0, 0, 6, borrow_data, reglen, 0);
			}
			borrow_data = (*l_borrow_dptr)(job, borrow_data, reglen, 
					salt + (times - (i + 1)));
		}
	}
	if (!borrow_data)
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (bgetx(job, (char **)&borrow_data, &start)) 
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (bgetx(job, (char **)&borrow_data, &exp)) 
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}

	exp += start;


	if (!(hostid1 = bget(job, (char **)&borrow_data))) 
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (!(hostid2 = bget(job, (char **)&borrow_data))) 
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (l_get_id(job, &h1, hostid1))
		h1 = 0;
	else h = h1;
	if (!h && strcmp(hostid2, INVALID_BHOSTID))
	{
		if (l_get_id(job, &h2, hostid1))
			h2 = 0;
		else h = h2;
	}
	
	if (!h || 
		(h1 && (h1->type != L_DEFAULT_HOSTID)) ||
		(!h1 && h2 && (h2->type != HOSTID_DISK_SERIAL_NUM)))
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (h && l_host(job, h))
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (!(idstr = bget(job, (char **)&borrow_data))) 
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (!(feat = bget(job, (char **)&borrow_data))) 
	{
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	if (parent = bget(job, (char **)&borrow_data))
	{
		if (!(pkg = bget(job, (char **)&borrow_data)))
		{
			ret = LM_BORROW_LINGER_ERR;
			goto errexit_l_ckout_borrow;
		}
	}


	memset(servers, 0, sizeof(servers));
	if (*idstr)
	{
		servercnt++;

		servers[0] = idstr;
		if (servers[1] = strstr(servers[0], L_BORROW_ID_SEP))
		{
			servercnt++;
			*servers[1] = 0; /* null terminate servers[0] */
			servers[1] += strlen(L_BORROW_ID_SEP);
			if (servers[2] = strstr(servers[1], L_BORROW_ID_SEP))
			{
				servercnt++;
				/* null terminate servers[1] */
				*servers[2] = 0; 
				servers[2] += strlen(L_BORROW_ID_SEP);
			}
		}
	}


	if ((time(0) > exp) || (time(0) < start))
	{
#ifndef RELEASE_VERSION
		if (perr && (*perr == '0')) fprintf(stderr, "\ntime %x exp %x start %x\n", 
					time(0), exp, start);
#endif /* RELEASE_VERSION */
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 560, 0);
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}


	conf = (CONFIG *)l_malloc(job, sizeof(CONFIG ));
	if (!l_parse_feature_line(job, feat, conf, (char **)0))
	{
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 550, 0);
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
	else
	{
		conf->next = job->line;
		job->line = conf;
	}
	if (parent)
	{
		parentconf = (CONFIG *)l_malloc(job, sizeof(CONFIG ));
		if (!l_parse_feature_line(job, parent, parentconf, (char **)0))
		{
			LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 550, 0);
			ret = LM_BORROW_LINGER_ERR;
			goto errexit_l_ckout_borrow;
		}
		pkgconf = (CONFIG *)l_malloc(job, sizeof(CONFIG ));
		if (!l_parse_feature_line(job, pkg, pkgconf, (char **)0))
		{
			LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 550, 0);
			ret = LM_BORROW_LINGER_ERR;
			goto errexit_l_ckout_borrow;
		}
		conf->parent_feat = parentconf;
		conf->parent_pkg = pkgconf;
		conf->package_mask = LM_LICENSE_PKG_COMPONENT;
		/*
		 *	Copy over any data from enabling feature
		 */
		l_CopyPackageInfoToComponent(job, parentconf, conf);
		package = 1;
	}
	if ((exp - start) > (conf->lc_max_borrow_hours * 
			(l_borrow_in_seconds ? 1 : (60 * 60)))) /* counterfeit? */
	{
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 561, 0);
		ret = LM_BORROW_LINGER_ERR;
		goto errexit_l_ckout_borrow;
	}
/* 
 *	We have to create server(s) for the hostid(s)
 */
	servermem = (char *)l_malloc(job, sizeof(LM_SERVER) * servercnt);
	for (i = 0; i < servercnt; i++)
	{
		s = (LM_SERVER *)&servermem[i * sizeof(LM_SERVER)];
		if (s_sav) s_sav->next = s;
		if (!conf->server) conf->server = s;

		s->idptr = 0;
		if (l_get_id(job, &s->idptr, servers[i]))
		{
			free(s->idptr);
			if (package)
			{
				l_free_conf(job, pkgconf);
				l_free_conf(job, parentconf);
			}
			else
			{
				l_free_conf(job, conf);
				conf = 0;
			}
			LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 556, 0);
			ret = LM_BORROW_LINGER_ERR;
			goto errexit_l_ckout_borrow;
		}
		s_sav = s;
	}
	slp = (LM_SERVER_LIST_PTR)l_malloc(job, sizeof(LM_SERVER_LIST));
	slp->s = conf->server;
	slp->next = job->conf_servers;
	job->conf_servers = slp;

	if (!package) 
	{
		strcpy(conf->feature, feature);
	}
	else
		parentconf->server = conf->server;
	ret = !l_local_verify_conf(job, conf, feature, version, key, 0, 0);


	conf->borrow_flags |= LM_CONF_BORROWED;
	if (!ret)
	{
		l_ckout_ok(job, feature, version, nlic, key, &conf, dup_group, 
							0, flag);
	}
	goto exit_l_ckout_borrow;

errexit_l_ckout_borrow:
	if (ret && conf) 
	{
		if (job->line) job->line = job->line->next;
		l_free_conf(job, conf);
		conf = 0;
	}
	{
#ifndef RELEASE_VERSION
		

		if (perr && *perr == '2')
			fprintf(stderr, "%s ", borrow_data ? (char *)borrow_data : "null");
#endif

		if (perr && *perr > '0') lc_perror(job, "borrow");
	}

exit_l_ckout_borrow:

	if (parentconf)
	{
		l_free_conf(job, parentconf);
		parentconf = NULL;
	}
	if (pkgconf)
	{
		l_free_conf(job, pkgconf);
		pkgconf = NULL;
	}
	if (conf) conf->parent_feat = 0;
	if (conf) conf->parent_pkg = 0;
	if (regmem) free(regmem);
	if (h) lc_free_hostid(job, h);
	h = 0;
	return ret;
}


int
l_read_borrow(LM_HANDLE_PTR	job,
	      const LM_CHAR_PTR	feature)
{
	int		ret = 0;
	int		i;
	char		regkey[MAX_FEATURE_LEN+20];
	unsigned char *	borrow_data = 0;
	long		exp = 0;
	long		start = 0;
	char *		feat = NULL;
	char *		parent = NULL;
	char *		pkg = NULL;
	CONFIG *	conf = NULL;
	char *		idstr = NULL;
	int		servercnt = 0;
	char *		servers[3];
	LM_SERVER *	s = 0,
		  *	s_sav = 0;
	char *		servermem = NULL;
	int		package = 0;
	LM_SERVER_LIST_PTR slp;
	char *		hostid1 = NULL;
	char *		hostid2 = NULL;
	int		reglen = 0;
	unsigned int	salt;
	char *		regmem = NULL;
	unsigned int	b_id;
	char		b_id_buf[9];
	_borrowid borrowid = (_borrowid)job->l_new_job;
	HOSTID *	h1 = NULL;
	HOSTID *	h2 = NULL;
	HOSTID *	h = NULL;
	int		featname_4bytes;
	static char *	perr = (char *)-1;
	int		eval = 0;

	if (perr == (char *)-1)
		perr = getenv("L_BORROW_ERR");	/* overrun checked */

	b_id = borrowid(0, 0, 4, 0, 0, 0); /* the 4 indicates get borrowid */
	if (!b_id)
	{
		eval = 1;
		b_id = 1234;
	}
	b_id ^= L_BORROW_MAGIC;
	if (b_id)
		sprintf(b_id_buf, "%x", b_id);
	else
		*b_id_buf = 0;

	sprintf(regkey, "borrow-%s-%s", b_id_buf, feature);
	if (job->L_BORROW_STRING && *job->L_BORROW_STRING)
	{
		borrow_data = (unsigned char *)job->L_BORROW_STRING;
		borrow_data = (unsigned char *)strchr((char *)borrow_data, '=');
		if (borrow_data)
			borrow_data++;
	}
	else
	{
		char *cpreg = 0;
		
		l_get_registry(job, regkey, &cpreg, &reglen, 1);
#ifdef PC
		reglen -= 1;
#endif
		if (cpreg)
		{
/*
 *			Note:  The registry contains 2 clear text
 *			fields at the start:  the 'salt' and the
 *			expiration time.  The expiration time is unused.
 *			Total length is exactly 12 characters for these
 *			2 fields.
 */
			regmem = l_malloc(job, reglen + 1);
			memcpy(regmem, cpreg, reglen);	/* memory threat */
			regmem[reglen] = 0;
			sscanf(regmem, "%04x", &salt);	/* overrun threat */
			reglen -= 12;
			borrow_data = (unsigned char *)regmem + 12;
		}
	}
	if (!borrow_data)
	{
		goto done;
	}
	if (!l_borrow_dptr)
	{
		goto done;
	}
	/* get each field:  exp */

#ifndef RELEASE_VERSION
	if (cryptclear == -1L)
		cryptclear = ((long)l_real_getenv("FLEXLM_BCLEAR"));	/* overrun checked */
	if (!cryptclear)
#endif /* RELEASE_VERSION */
	if (!job->L_BORROW_STRING) 
	{
		char buf[MAX_FEATURE_LEN + 1];
		int times;

		memset(buf, 0, sizeof(buf));
		strcpy(buf, feature);
		memcpy(&featname_4bytes, buf, sizeof(int));
		times = ((b_id ^ L_BORROW_MAGIC2 ^ featname_4bytes) % 7) + 1;

		for (i = 0; i < times; i++)
		{
/*
 *			Borrowid called twice:
 *				o first time (arg 8) it reorders the bytes 
 *				  in the string
 *				o second time (arg 6) it encrypts each byte
 */

			if (!eval)
			{
				borrowid(0, 0, 8, borrow_data, reglen, 0); 
				borrowid(0, 0, 6, borrow_data, reglen, 0);
			}
			borrow_data = (*l_borrow_dptr)(job, borrow_data, reglen, 
					salt + (times - (i + 1)));
		}
	}
	if (!borrow_data)
	{
		goto done;
	}
	if (bgetx(job, (char **)&borrow_data, &start)) 
	{
		goto done;
	}
	if (bgetx(job, (char **)&borrow_data, &exp)) 
	{
		goto done;
	}

	exp += start;


	if (!(hostid1 = bget(job, (char **)&borrow_data))) 
	{
		goto done;
	}
	if (!(hostid2 = bget(job, (char **)&borrow_data))) 
	{
		goto done;
	}
	if (l_get_id(job, &h1, hostid1))
		h1 = 0;
	else
		h = h1;
	if (!h && strcmp(hostid2, INVALID_BHOSTID))
	{
		if (l_get_id(job, &h2, hostid1))
			h2 = 0;
		else 
			h = h2;
	}
	
	if (!h || 
		(h1 && (h1->type != L_DEFAULT_HOSTID)) ||
		(!h1 && h2 && (h2->type != HOSTID_DISK_SERIAL_NUM)))
	{
		goto done;
	}
	if (h && l_host(job, h))
	{
		goto done;
	}
	if (!(idstr = bget(job, (char **)&borrow_data))) 
	{
		goto done;
	}
	if (!(feat = bget(job, (char **)&borrow_data))) 
	{
		goto done;
	}
	if (parent = bget(job, (char **)&borrow_data))
	{
		if (!(pkg = bget(job, (char **)&borrow_data)))
		{
			goto done;
		}
	}


	memset(servers, 0, sizeof(servers));
	if (*idstr)
	{
		servercnt++;

		servers[0] = idstr;
		if (servers[1] = strstr(servers[0], L_BORROW_ID_SEP))
		{
			servercnt++;
			*servers[1] = 0; /* null terminate servers[0] */
			servers[1] += strlen(L_BORROW_ID_SEP);
			if (servers[2] = strstr(servers[1], L_BORROW_ID_SEP))
			{
				servercnt++;
				/* null terminate servers[1] */
				*servers[2] = 0; 
				servers[2] += strlen(L_BORROW_ID_SEP);
			}
		}
	}


	if ((time(0) > exp) || (time(0) < start))
	{
#ifndef RELEASE_VERSION
		if (perr && (*perr == '0')) fprintf(stderr, "\ntime %x exp %x start %x\n", 
					time(0), exp, start);
#endif /* RELEASE_VERSION */
		goto done;
	}


	conf = (CONFIG *)l_malloc(job, sizeof(CONFIG ));
	if(!conf)
	{
		goto done;
	}
	if (!l_parse_feature_line(job, feat, conf, (char **)0))
	{
		free(conf);
		conf = NULL;
		goto done;
	}
	else
	{
		conf->next = job->line;
		job->line = conf;
	}
	if (parent)
	{
		package = 1;
	}

	if (!package) 
	{
		strcpy(conf->feature, feature);
	}

	{
#ifndef RELEASE_VERSION
		

		if (perr && *perr == '2')
			fprintf(stderr, "%s ", borrow_data ? (char *)borrow_data : "null");
#endif

		if (perr && *perr > '0') lc_perror(job, "borrow");
	}

done:
	if (conf)
	{
		conf->parent_feat = 0;
		conf->parent_pkg = 0;
	}
	if (regmem)
		free(regmem);
	if (h)
		lc_free_hostid(job, h);
	h = 0;
	return ret;
}


static
char *
bget(LM_HANDLE *job, char **preg)
{
 char *borrow_data = *preg;
 char *cp;
	if (!borrow_data || !*borrow_data)
	{
		LM_SET_ERRNO(job, LM_BORROW_LINGER_ERR, 556, 0);
		return 0;
	}
	if (cp = strstr(borrow_data, L_BORROW_SEPARATOR))
	{
		*cp = 0; /* null terminate */
		cp += strlen(L_BORROW_SEPARATOR);
	}
	*preg = cp;
	return borrow_data;
}
static
int
bgetx(LM_HANDLE *job, char **preg, long *l)
{
  char *cp;
  *l = 0;	/* NULL pointer checked */
	if (!(cp = bget(job, preg)))
		return LM_BORROW_LINGER_ERR;
	if (sscanf(cp, "%lx", l) != 1)	/* overrun checked */
		return LM_BORROW_LINGER_ERR;
	return 0;
}

static
void
infoborrow(LM_HANDLE *job, CONFIG *conf, char *b_id)
{
  char *oreg = 0;
  int olen;
  char *borrowinfo, *regcp;
  char buf[MAX_CONFIG_LINE + 1];
  int len;
  char *cp = 0, *ocp;
  char feat[MAX_FEATURE_LEN + 1];
  char vendor[MAX_VENDOR_NAME + 1];
  char code[MAX_CRYPT_LEN + 1];
  long start, exp;
  int bid;
	
/*
 *	Format the borrow information into buf
 */
	sprintf(buf, "%s%s%s%s%lx-%x-%s-%4.4s\n", job->vendor, 
		L_BORROW_SEPARATOR2, conf->feature,L_BORROW_SEPARATOR2,  
				time(0), (job->borrow_linger_minutes * 
				(l_borrow_in_seconds ? 1 : 60)), 
					conf->code, b_id);
	len = strlen(buf);
/*
 *	If it is already in the registry
 *	update it properly
 */
	if (!l_get_registry(job, L_BORROW_REG, &oreg, &olen, 1))
	{
	  int b_idl;
		regcp = borrowinfo = l_malloc(job, len + olen + 1) ;
		sscanf(b_id, "%x", &b_idl);	/* overrun checked */
		b_idl = (b_idl >> 16) & 0xffff; /*only high order bits wanted*/
		l_get_registry(job, L_BORROW_REG, &oreg, &olen, 1);
		for (cp = oreg; cp && *cp; )
		{
			ocp = cp;
			if (!(cp = l_parse_info_borrow(job, cp, feat, vendor, &start, &exp, code, &bid)))
				continue;
			/* don't put info older than a week into list */
			if (((long)(time(0) - exp)) > 
#ifdef RELEASE_VERSION
				7*24*60*60   /* one week */
#else
				60*60 /* one hours */
#endif
				) 
			{
				;
			}
			else if (b_idl == bid && 
				l_keyword_eq(job, feat, conf->feature) &&
				l_keyword_eq(job, vendor, conf->daemon))
				;
			else
			{
				memcpy(regcp, ocp, cp - ocp); 
				regcp += cp - ocp;
			}
		}
	}
	else
		regcp = borrowinfo = l_malloc(job, len + 1) ;
	strcpy(regcp, buf); /* append the current borrow info */

	l_set_registry(job, L_BORROW_REG, borrowinfo, strlen(borrowinfo), 1);
	free(borrowinfo);
}
char *
l_parse_info_borrow(LM_HANDLE *job, char *buf, /* input */
		/* following features are return values */
	char *feature, /* char buffer*/
	char *vendor,
	long *start, 
	long *exp, 
	char *code,
	int *b_id)	/* char buffer */
{
 char *cp, *cp2;
 char *buf2;
 char *ret = 0;

	buf2 = (char *)l_malloc(job, strlen(buf )+ 1);	/* memory threat */
	strcpy(buf2, buf);

	cp = buf2;
	cp2 = strstr(cp, L_BORROW_SEPARATOR2);
	if (!cp2) goto exit_l_parse_info;
	*cp2 = 0;
	l_zcp(vendor, cp, MAX_VENDOR_NAME);
	cp = cp2 + 3;
	cp2 = strstr(cp, L_BORROW_SEPARATOR2);
	if (!cp2) goto exit_l_parse_info;
	*cp2 = 0;
	l_zcp(feature, cp, MAX_FEATURE_LEN);
	cp = cp2 + 3;
	*b_id = 0;
	*start = 0;
	*exp = 0; 
	*code = 0;
	if (sscanf(cp, "%lx-%lx-%30[^-]-%x", start, exp, code, b_id) != 4)	/* overrun threat */
		goto exit_l_parse_info;
	*exp += *start;
	if (cp = strchr(cp, '\n'))
	{
		cp++;
		ret = buf + (cp - buf2);
	}
exit_l_parse_info:
	if (buf2) free(buf2);
	/*if (ret == '\n') ret++;*/
	return ret;
}

unsigned char *
l_borrow_decrypt(LM_HANDLE *job, unsigned char *str, int len, int salt)
{
	
  char buf[MAX_BCRYPT_LEN + 1];
  unsigned int b1, b2;
  typedef unsigned int (*_borrowcb)
			lm_args((LM_VOID_PTR, LM_VOID_PTR, int , 
				unsigned char *, int, int *));
  _borrowcb borrowcb;
  int i, l, m, done, remaining;
  int itmp;
  int order[L_BCRYPT_ROUNDS][MAX_BCRYPT_LEN + 1];
  unsigned char seeds[L_BCRYPT_ROUNDS][MAX_BCRYPT_LEN + 1];
  int shuffle_seeds[3];
  int crypt_seeds[3];
  int round;
  int salt2;
/*
 *	We have to decrypt it in groups of MAX_BCRYPT_LEN bytes, since that's
 *	the way it's encrypted at the server, because of the max 
 *	msg size
 */


	borrowcb = (_borrowcb)job->l_new_job;
	if (!borrowcb) return 0;
	b1 = borrowcb(0, 0, 2, 0, 0, 0);
	b2 = borrowcb(0, 0, 3, 0, 0, 0);
	if (!b1)
	{
		b1 = 1234;
		b2 = 5678;
	}


	if (!len) len = strlen((char *)str); /* null-terminated*/
	l = len;
	for (done=0, remaining=l ; remaining > 0; 
		done += MAX_BCRYPT_LEN, remaining -= MAX_BCRYPT_LEN)
	{

		if (remaining < MAX_BCRYPT_LEN) m = remaining;
		else 			m = MAX_BCRYPT_LEN;

/* 
 *		Initialize the seeds
 */
		/*fprintf(stdout, "b1 %x b2, %x salt = %x len = %d\n", b1, b2, salt, m);*/
		salt2 = m + salt;
		srand16((b1 + b2 + salt2) & 0xffff, (b1 ^ b2 + salt2) & 0xffff,
			((b2 + b2 + salt2) >> 16) & 0xffff, shuffle_seeds);
		srand16((b1 + b2 - salt2) & 0xffff, (b1 ^ b2 - salt2) & 0xffff,
			((b2 + b2 - salt2) >> 16) & 0xffff, crypt_seeds);
/*
 *		We do this for many "rounds" to increase the 
 *		obfuscation a bit
 *		In order to "unwind" the encryption, we have 
 *		recreate the shuffle order in the same way it
 *		was created.  I do this to an array, and then apply it
 *		later.  The same thing for the subtraction.
 */
		for (round = 0; round < L_BCRYPT_ROUNDS; round++)
		{
			rand16_2(shuffle_seeds);
			rand16_2(shuffle_seeds);
			rand16_2(shuffle_seeds);

			rand16_2(crypt_seeds);
			rand16_2(crypt_seeds);
/*-
 *			First shuffle the bytes
 */
			for (i = 0; i < m; i++) order[round][i] = i;
			for (i = 0; i < (m - 1); i++ )
			{

				/* shuffle the array */
				if (rand16_2(shuffle_seeds) % 2)
				{
					itmp = order[round][i]; 
					order[round][i] = order[round][i+1]; 
					order[round][i + 1] =itmp;/*swap bytes*/
				}
			}
			for (i = 0; i < m ; i++ )
			{
				seeds[round][i] = rand16_2(crypt_seeds) & 0xff;
			}

		}


		for (round = L_BCRYPT_ROUNDS - 1; round >= 0; round--)
		{
			memcpy(buf, &str[done], m);
			buf[m] = 0;
/*-
 *			Apply the shuffle based on the "order" array
 *			setup above
 */
			for (i = 0; i < m; i++) 
				str[(order[round][i]) + done] = buf[i];
/*-
 *			Now decrypt them using subtraction.
 */
			for (i = 0;i < m; i++)
			{
				str[done+i] -= seeds[round][i];
			}
		}
	}
	
	/*fprintf(stdout, "returning %s\n", str);*/
	return str;
}
long
l_borrow_string_to_time(LM_HANDLE *job, char *str)
{
  char date[200];
  long t = -1;
  int h = -1, m = -1;

	sscanf(str, "%[^:]:%d:%d", date, &h, &m);	/* overrun checked */
	if ((t = l_date_to_time(job, date)) == -1)
		return 0;
	if ((h != -1) && (m != -1))
	{
		/* set back to beginning of day */
		t -= (60 * 60 * 24); t++;
		t += (h * (60 * 60)) + (m * 60);
	}
	return t;
}

int 
l_borrow_stat(LM_HANDLE *job,  void * vptr, int la_flag)
{
  char *stat;
  char *cp, *cp2;
  LM_BORROW_STAT *s, **sptr = (LM_BORROW_STAT **)vptr;

	s = *sptr = 0;
	l_free_list(job->borrow_stat);
	s = 0;
	if (!l_get_registry(job, "infoborrow", &stat, 0, 1))
	{
	   char vendor[MAX_CONFIG_LINE];
	   char feature[MAX_CONFIG_LINE];
	   char code[MAX_CONFIG_LINE];
	   long start;
	   time_t end;
	   long endl;
	   int b_id;
	   struct tm *tm;
	   int printed = 0;
	   char exp[100];
	   char featid[MAX_FEATURE_LEN + 10];

		cp = stat;
		while (cp && *cp && cp[1])
		{
			cp = l_parse_info_borrow(job, cp, feature, 
					vendor, &start, &endl, code, &b_id);
			if (!la_flag && !l_keyword_eq(job, vendor, job->vendor))
				continue;
			end = (time_t)endl;
			if (!s)
			{
				s = *sptr = job->borrow_stat = (LM_BORROW_STAT *)
					l_malloc(job, sizeof(LM_BORROW_STAT));
			}
			else
			{
				s->next = job->borrow_stat = (LM_BORROW_STAT *)
					l_malloc(job, sizeof(LM_BORROW_STAT));
				s = s->next;
			}
			l_zcp(s->feature, feature, MAX_FEATURE_LEN);
			l_zcp(s->vendor, vendor, MAX_FEATURE_LEN);
			l_zcp(s->code, code, MAX_CRYPT_LEN);
			s->start = start;
			s->end = (time_t)endl;
			s->borrow_binary_id = b_id;
		}
		
	}
	else
	{
		return job->lm_errno;
	}
	s = *sptr;
	return 0;
	
}

/*
 *	Borrow return specific defines.
 */
#define REMOVE_FEATURE	2
#define LM_BORROW_KEY	"LM_BORROW"
#ifdef PC
#define FLEXLM_BORROW_REG_KEY	"Software\\FLEXlm License Manager\\Borrow"
#else /* !PC */
#define SEPARATOR	"="
#endif


/*
 *	Purpose:	Update the "infoborrow" field in the "registry"
 *	Input:		job - FLEXlm job handle
 *				pStat - borrow information
 *	Returns:	None
 */
static
void
sDeleteBorrowInfo(
	LM_HANDLE *			job,
	LM_BORROW_STAT *	pStat)
{
	char *				stat = NULL;
	char *				cp = NULL;
	char *				cp2 = NULL;
	char *				pCurr = NULL;
	char *				buffer = NULL;
	int					iLen = 0;
	char				vendor[MAX_CONFIG_LINE] = {'\0'};
	char				feature[MAX_CONFIG_LINE] = {'\0'};
	char				code[MAX_CONFIG_LINE] = {'\0'};
	long				start;
	long				endl;
	int					b_id;


	if(l_get_registry(job, "infoborrow", &stat, &iLen, 1))
		return;

	buffer = l_malloc(job, iLen  + 1);
	if(buffer == NULL)
		return;
	memset(buffer, 0, iLen + 1);
	/*
	 *	Write contents of data into buffer, removing entry specified by pStat,
	 *	then rewrite it to the registry.
	 */
	cp = stat;

	while (cp && *cp && cp[1])
	{
		pCurr = cp;
		cp = l_parse_info_borrow(job, cp, feature,
				vendor, &start, &endl, code, &b_id);
		if(!(strcmp(pStat->feature, feature) == 0 &&
			strcmp(pStat->vendor, vendor) == 0 &&
			strcmp(pStat->code, code) == 0 &&
			pStat->start == start &&
			pStat->end == (time_t)endl &&
			pStat->borrow_binary_id == b_id))
		{
			memcpy(&buffer[strlen(buffer)], pCurr, cp - pCurr);
		}
	}
	l_set_registry(job, "infoborrow", buffer, strlen(buffer) + 1, 1);
	l_free(buffer);
	return;
}

/*
 *	Purpose:	Update the local borrow info from "registry"
 *	Input:		job - FLEXlm job handle
 *				pStat - borrow information
 *	Returns:	0 on success, else an error occurred.
 */
static
int
sDeleteBorrow(
	LM_HANDLE *			job,
	LM_BORROW_STAT *	pStat)
{
	char		szName[1024] = {'\0'};

#ifdef PC

	HKEY		hKey = NULL;
	int			i = 0;
	char		szBorrowName[1024] = {'\0'};
	long		size = sizeof(szBorrowName);

	sprintf(szName, "borrow-%x", pStat->borrow_binary_id);
	/*
	 *	Now iterate through the values and determine which one matches
	 */


	if(RegOpenKeyEx(HKEY_CURRENT_USER,
		FLEXLM_BORROW_REG_KEY, 0, KEY_WRITE | KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		LM_SET_ERRNO(job, LM_BORROW_DELETE_ERR, 615, 0);
		return job->lm_errno;
	}

	while(RegEnumValue(hKey, i++, szBorrowName, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		/* NULL terminate */
		szBorrowName[size] = '\0';

		/*	Make comparison to "borrow-XXXX????-feature_name" */
		if(strncmp(szBorrowName, szName, strlen(szName)) == 0 &&
			strncmp((char *)(&szBorrowName[strlen(szBorrowName) - strlen(pStat->feature)]),
					pStat->feature, strlen(pStat->feature)) == 0)
		{
			/*
			 *	Delete this entry from registry
			 */
			(void)RegDeleteValue(hKey, szBorrowName);
			(void)RegDeleteValue(hKey, LM_BORROW_KEY);
			sDeleteBorrowInfo(job, pStat);
		}
		size = sizeof(szBorrowName);
	}

	RegCloseKey(hKey);

#else /* !PC */
	char *		pszData = NULL;
	char *		pszBorrowName = NULL;
	char		szKey[1024] = {'\0'};
	char *		pszSep = NULL;
	int			iLen = 0;

	pszBorrowName = l_malloc(job, job->borrfile_s + 1);
	if(!pszBorrowName)
	{
		/*
		 *	error out
		 */
		LM_SET_ERRNO(job, LM_CANTMALLOC, 614, 0);
		return job->lm_errno;
	}


	sprintf(szName, "borrow-%x", pStat->borrow_binary_id);

	for(pszData = l_get_next_registry(job->borrfile, pszBorrowName, &iLen);
				*pszBorrowName; pszData = l_get_next_registry(pszData, pszBorrowName, &iLen))
	{

		/*	Make comparison to "borrow-XXXX????-feature_name" */
		if(strncmp(pszBorrowName, szName, strlen(szName)) == 0)

		{
			pszSep = strstr(pszBorrowName, SEPARATOR);
			if(pszSep)
			{
				strncpy(szKey, pszBorrowName, pszSep - pszBorrowName);
				szKey[pszSep - pszBorrowName] = '\0';
			}
			else
			{
				strncpy(szKey, pszBorrowName, iLen);
			}
			if(strncmp((char *)(&szKey[strlen(szKey) - strlen(pStat->feature)]),
						pStat->feature, strlen(pStat->feature)) == 0)
			{
				/*
				 *	Delete this entry from registry
				 */
				(void)l_delete_registry_entry(job, szKey, 1);
				(void)l_delete_registry_entry(job, LM_BORROW_KEY, 1);
				sDeleteBorrowInfo(job, pStat);
			}
		}
	}
	if(pszBorrowName)
		free(pszBorrowName);
#endif /* PC */
	return 0;
}

/*
 *	Purpose:	Modify borrow info, calling it again will restore original info.
 *	Input:		job - FLEXlm job handle
 *				pStat - borrow information
 *	Returns:	0 on success, else an error occurred
 */
static
int
sModifyBorrow(
	LM_HANDLE *			job,
	LM_BORROW_STAT *	pStat)
{
	char			szName[1024] = {'\0'};

#ifdef PC

	HKEY			hKey = NULL;
	int				i = 0;
	char			szBorrowName[1024] = {'\0'};
	char *			pBuffer = NULL;
	long			size = sizeof(szBorrowName);
	unsigned int	type = 0;
	unsigned int	datasize = 0;


	sprintf(szName, "borrow-%x", pStat->borrow_binary_id);
	/*
	 *	Now iterate through the values and determine which one matches
	 */


	if(RegOpenKeyEx(HKEY_CURRENT_USER,
		FLEXLM_BORROW_REG_KEY, 0, KEY_WRITE | KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		LM_SET_ERRNO(job, LM_BORROW_DELETE_ERR, 613, 0);
		return job->lm_errno;
	}

	while(RegEnumValue(hKey, i++, szBorrowName, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		/* NULL terminate */
		szBorrowName[size] = '\0';

		/*	Make comparison to "borrow-XXXX????-feature_name" */
		if(strncmp(szBorrowName, szName, strlen(szName)) == 0 &&
			strncmp((char *)(&szBorrowName[strlen(szBorrowName) - strlen(pStat->feature)]),
					pStat->feature, strlen(pStat->feature)) == 0)
		{
			/*
			 *	Read actually data
			 */
			datasize = 0;
			l_get_registry(job, szBorrowName, &pBuffer, &datasize, 1);;
			if(datasize)
			{
				int	j = 0;
				/*
				 *	Simple XOR of all the even bytes
				 */
				while(j < datasize)
				{
					pBuffer[j] ^= 0xFF;
					j += 2;
				}
				l_set_registry(job, szBorrowName, pBuffer, datasize, 1);
			}
			break;
		}
		size = sizeof(szBorrowName);
	}

	RegCloseKey(hKey);

#else /* !PC */
	char *		pszData = NULL;
	char *		pszBorrowName = NULL;
	char		szKey[1024] = {'\0'};
	char		buffer[8192] = {'\0'};
	char *		pBuffer = NULL;
	char *		pszSep = NULL;
	int			iLen = 0;
	int			datasize = 0;

	pszBorrowName = l_malloc(job, job->borrfile_s + 1);
	if(!pszBorrowName)
	{
		/*
		 *	error out
		 */
		LM_SET_ERRNO(job, LM_CANTMALLOC, 612, 0);
		return job->lm_errno;
	}


	sprintf(szName, "borrow-%x", pStat->borrow_binary_id);

	for(pszData = l_get_next_registry(job->borrfile, pszBorrowName, &iLen);
				*pszBorrowName; pszData = l_get_next_registry(pszData, pszBorrowName, &iLen))
	{

		/*	Make comparison to "borrow-XXXX????-feature_name" */
		if(strncmp(pszBorrowName, szName, strlen(szName)) == 0)

		{
			pszSep = strstr(pszBorrowName, SEPARATOR);
			if(pszSep)
			{
				strncpy(szKey, pszBorrowName, pszSep - pszBorrowName);
				szKey[pszSep - pszBorrowName] = '\0';
			}
			else
			{
				strncpy(szKey, pszBorrowName, iLen);
			}
			if(strncmp((char *)(&szKey[strlen(szKey) - strlen(pStat->feature)]),
						pStat->feature, strlen(pStat->feature)) == 0)
			{
				datasize = 0;
				l_get_registry(job, szKey, &pBuffer, &datasize, 1);
				if(datasize)
				{
					int	j = 0;

					/*
					 *	Copy over data or l_set_registry won't write it out
					 */
					if(datasize > sizeof(buffer))
						datasize = sizeof(buffer);
					memcpy(buffer, pBuffer, datasize);
					/*
					 *	Simple XOR of all even bytes in data
					 */
					while(j < datasize)
					{
						buffer[j] ^= 0xFF;
						j += 2;
					}
					l_set_registry(job, szKey, buffer, datasize, 1);
				}
				break;
			}
		}
	}
	if(pszBorrowName)
		free(pszBorrowName);
#endif /* PC */
	return 0;
}

/*
 *	Purpose:	Return a borrowed license to the server
 *	Input:		job - FLEXlm job handle
 *				pszFeature - name of feature to return
 *				pszDisplay - display name to use, if NULL, will FLEXlm will calculate
 *	Returns:	0 on success, else an error occurred.
 */
static
int
sBorrowReturn(
	LM_HANDLE *	job,
	char *		pszFeature,
	char *		pszDisplay)
{
	int					i;
	char *				szFeature  = NULL;
	time_t				currTime = 0;
	char *				szVendorNameLic = NULL;
	char				szVendor[1024] = {'\0'};
	char *				pszLicFile = NULL;
	LM_BORROW_STAT *	pBorrow = 0;
	LM_BORROW_STAT *	pTemp = NULL;

	/*
	 *	Make sure input good
	 */
	if(!job || !pszFeature)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 609, 0);
		goto done;
	}
	/*
	 *	Get borrow info (infoborrow)
	 */
	if (l_borrow_stat(job, &pBorrow, 1))
	{
		LM_SET_ERRNO(job, LM_BORROW_ERROR, 610, 0);
		goto done;
	}
	/*
	 *	Determine if we actually have this feature borrowed
	 */
	for (pTemp = pBorrow; pTemp; pTemp = pTemp->next)
	{
		if(strcmp(pszFeature, pTemp->feature) == 0)
		{
			break;
		}
	}

	if(pTemp == NULL)
	{
		/*
		 *	Couldn't find feature in question, error out.
		 */
		LM_SET_ERRNO(job, LM_NOFEATURE, 611, 0);
		goto done;
	}

	/*
	 *	Determine if feature is still valid
	 */
	time(&currTime);
	if(currTime > pTemp->end)
	{
		sDeleteBorrow(job, pTemp);
	}
	else
	{
		/*
		 *	Modify it before we attempt a remove
		 */
		sModifyBorrow(job, pTemp);
		i = l_return_early(job,
							pszFeature,
							pBorrow->vendor,
							lc_username(job, 1),
							lc_hostname(job, 1),
							pszDisplay ? pszDisplay : lc_display(job, 1) );
		
		if(i)
		{
			/*
			 *	Unmodify it since we couldn't return it
			 */
			sModifyBorrow(job, pTemp);
			if(job->lm_errno == BADPARAM)
			{
				LM_SET_ERRNO(job, LM_BORROW_RETURN_SERVER_ERR, 595, 0);
			}
		}
		else
		{
			/*
			 *	If this fails, at least we modified it so user can't use it again.
			 */
			sDeleteBorrow(job, pTemp);
		}
	}

done:
	return job->lm_errno;
}

/*
 *	Purpose:	External interface for returning a borrowed license early
 *	Input:		job - FLEXlm job handle
 *				pszFeature - name of feature to return
 *				pszDisplay - display name to use, if NULL, will FLEXlm will calculate
 *	Returns:	0 on success, else an error occurred.
 */
int
API_ENTRY
lc_borrow_return(
	LM_HANDLE *	job,
	char *		pszFeature,
	char *		pszDisplay)
{
	return sBorrowReturn(job, pszFeature, pszDisplay);
}


#ifdef COMMENT

Author: D. Birns
Date:	July 1, 2001

Linger-based borrowing internals.
_________________________________

Overview.	

	I. 	User indicates that they a checkout should borrow the license.
	II. 	The checkout creates the local borrow-data.
	III. 	Subsequent checkouts use the local borrow-data to do 
		the checkout, avoiding consuming any other licenses.

I.  User indicates that checkout should borrow a license
________________________________________________________

	$LM_BORROW
	lmborrow command
	LM_A_BORROW_EXPIRE

	These methods all set an expiration date/time for borrowing in
		job->borrow_linger_minutes
	

	Check to make sure the app supports borrowing?  LM_OPTFLAG_BORROW_OK
	If not, checkout fails.
	
II.  Checkout creates local borrow-data
_______________________________________

	1. checkout the license from the server.

		The server ensures that the feature supports borrowing, or
		the checkout is denied.

		The server ensures the checkout period does not exceed the
		max specified in the license, or the checkout is denied.

		The server lingers the license, and marks it as borrowing.
		This is done because the checkout call specified a bit
		in the checkout message:
			msgparam[MSG_CO_FLAGS2] |= MSG_CO_FLAGS2_BORROW;

	2.  l_borrow creates the local borrow data

		First the license is checked in.  This begins the linger
		period in the server.

		Ascii, cleartext version of the borrow data is created. 
		This includes:

			o current time 
			o linger seconds
			o client hostid -- 2 for windows, ether and 
				disk-serial number
			o 1-3 server hostids for the license
			o ascii feature line
			o if package, parent feature and feature

		if job->L_BORROW_STRING is set, we are done.  In practice,
		I think this is likely to make things less secure, not more,
		so I doubt this will get used.

	3. Encrypt the local borrow data.

		There are 3 different types of encryption applied, and all
		3 are applied in a loop, so that it is done a certain 
		number of times.

		loop (n times)
		{
			server_bcrypt()
			borrowid(0, 0, 5, borrow_data, reglen); 
			borrowid(0, 0, 7, borrow_data, reglen); 
		}				

		a. loop (n times):
			n is from 1 to 8 times.
			This is a function of
				The first 4 chars of the feature name
				The vendor (via SEED3 and SEED4)

			The SEED3 and SEED4 are used to generate
			lm_new.c, which is called via 
			lm_new.cs l_new_job(0,0,4,0,0);
			In the code this function is renamed borrowid(),
			but it is the same function.

		b. server_bcrypt()

			We send the current state of borrow_data to
			server, and it adds a layer of encryption 
			in ls_borrow_encrypt().

			ls_borrow_encrypt takes the borrow data plus
			a "salt".  This is a number that the client
			makes up using the normal rand()/srand() functions,
			which is seeded with the current time.

			This salt becomes crucial to decrypting it,
			so it is stored in cleartext as with the
			borrow-data

			This uses seeds it gets from 
			l_new_job( 0, 0, 2, 0, 0); 
			l_new_job( 0, 0, 3, 0, 0); 

			Note that this function is renamed borrowcb()
			in ls_borrow_encrypt().

			It uses these seeds as seeds to rand16_2() which
			is a 16-bit random number generator.  

			The encryption is addition, and then order
			shuffling.

		c. borrowid(0, 0, 5, borrow_data, len)

			borrowid() is l_new_job() in lm_new.c

			This function encrypts the data.
			It uses substitution, and arithment operations.

		d. borrowid(0, 0, 7, borrow_data, len)

			This reorders bytes in the string.


	4. Store the local borrow data

		We have store all this data plus everything needed
		to decrypt it.

		a. Key:  We store it by key, which is 

			sprintf(key, "borrow-%s-%s", b_id_buf, feature);

			This is the way it has to be looked up later.
			The b_id_buf comes from borrowid(0, 0, 4, 0, 0), so
			it is available to all apps and vendor daemon, since 
			it is based on SEEDs 3 and 4.

		b. Value:

			Ascii salt
			Ascii expiration time (32 bit unix date)
			Encrypted borrow-data

		On windows this goes in the Registry in a separate
		directory for borrow data.

		On Unix, it goes in $HOME/.flexlmborrow

III. Use borrow-data for checkout
_________________________________

	Checkouts for apps supporting borrowing always look in the 	
	registry first to see if it can use borrow-data before doing
	a regular checkout.  This function is l_ckout_borrow(), which
	takes the same args as lc_checkout().

	1. look up registry entry.

		The key is borrow-borrowid-feature where borrowid
		comes from lm_new.cs l_new_job(), called borrowid(0,0,4,0,0).
		The 4 indicates return the borrowid.  This number is
		unique per vendor, and is based on SEEDs 3 and 4.  feature
		is the feature name.
	
		There is a clear text copy of the salt number, used to 
		encrypted the encrypted borrow-data.  
		
	2. decrypt the borrow data.

		This is done via l_borrow_decrypt().  However, this is
		obfuscated a little, because there is a global 
		function pointer called l_borrow_dptr which is set to 
		point to this function, and the assignment is done in
		lm_new.c.

		l_borrow_decrypt() is the reverse of the encryption
		process.  It is assymetric, so you cannot use this function
		to do the encryption without reverse engineering it.

		The encryption is done partly in the vendor daemon,
		but the decryption is 100% done in the client.

	3.  Checkout against the borrow-data

		This is like a checkout, and a near-normal checkout
		is done.  We have a real feature line, so everything
		there is checked.  The server is not of course, but
		instead we check the clients hostid to make sure 
		it is the client that borrowed the license.
		We also check the current time against the linger time.
		We use l_ckout_ok to authenticate the license.

	If any of this fails, a normal checkout is attempted.
	Borrow errors are not reported, only the subsequent 
	normal checkout errors.  L_BORROW_ERR env variable can be
	used to give the minor number of the borrow error.

#endif  /* COMMENT */
