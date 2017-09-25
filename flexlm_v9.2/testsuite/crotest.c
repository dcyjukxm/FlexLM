/******************************************************************************

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
 *	Module: $Id: crotest.c,v 1.29 2003/01/13 22:55:14 kmaclean Exp $
 */
#include "lmprikey.h"
#include "lmpubkey.h"
#include "lmclient.h"
#include "l_privat.h"
#include "l_prot.h"
#include "code.h"
#include "lm_attr.h"
#include "stdio.h"
#include <string.h>
void gen_init(LM_HANDLE **lm_job, VENDORCODE *site_code, int strength);
void client_init(LM_HANDLE **lm_job, VENDORCODE *site_code, int strength);
void strength_test(int strength);
void time_test(int strength);
char * testit(int, int);
void badkey(int strength);
void keygen_test(void);
void gen(char *feature, LM_HANDLE *gen_job, VENDORCODE *code, FILE *fp);
LM_HANDLE *gen_job;
LM_HANDLE *client_job;
VENDORCODE client_code;
FILE *ofp = 0;

main(int argc, char *argv[])
{
  int t;
  int i;

	ofp = stdout;
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i] , "-o"))
		{
			i++;
			ofp = fopen(argv[i], "w");
		}
		else if (!strcmp(argv[i], "gen"))
		{
			portability_gen(LM_STRENGTH_DEFAULT);
			portability_gen(LM_STRENGTH_113BIT);
			portability_gen(LM_STRENGTH_163BIT);
			portability_gen(LM_STRENGTH_239BIT);
			exit(0);
		}
	}
	keygen_test();
		
	portability();
	badkey(LM_STRENGTH_DEFAULT); 
	badkey(LM_STRENGTH_113BIT); 
	badkey(LM_STRENGTH_163BIT); 
	badkey(LM_STRENGTH_239BIT); 
	strength_test(LM_STRENGTH_DEFAULT);
	strength_test(LM_STRENGTH_113BIT);
	strength_test(LM_STRENGTH_163BIT);
	strength_test(LM_STRENGTH_239BIT);
	f1_100test();
	t = time(0);
	time_test(LM_STRENGTH_DEFAULT);
	time_test(LM_STRENGTH_113BIT);
	time_test(LM_STRENGTH_163BIT);
	time_test(LM_STRENGTH_239BIT);
	fprintf(ofp, "\n");
}
void
keygen_test(void) /* actually, P5308 test */
{
  int prisize;
  int pubsize;
  char prikey[500];
  char pubkey[500];
  char savprikey[500];
  char savpubkey[500];
  int seed3 = 0x11223344;
  int seed4 = 0x55667788;
  int eq, i, j;
  extern int l_genkeys( unsigned long , 
			unsigned long , 
			unsigned long,
			int pubkey_strength,
			int *prikey_size,
			char *prikey,
			int *pubkey_size,
			char *pubkey);
	memset(savprikey, 0, sizeof(savprikey));
	memset(savpubkey, 0, sizeof(savpubkey));
	for (i = 0; i < 8; i++)
	{

		l_genkeys(seed3 + (i << 24), seed4, 0,
			LM_STRENGTH_113BIT,
			&prisize, prikey,
			&pubsize, pubkey);
		eq = 1;
		for (j = 0; j < prisize; j++)
			if (prikey[j] != savprikey[j])
			{
				eq = 0;
				break;
			}
		if (eq)
			printf("bug P5308 line %d\n", __LINE__);
		memcpy(savprikey, prikey, prisize);
		eq = 1;
		for (j = 0; j < pubsize; j++)
			if (pubkey[j] != savpubkey[j])
			{
				eq = 0;
				break;
			}
		if (eq)
			printf("bug P5308 line %d\n", __LINE__);
		memcpy(savpubkey, pubkey, pubsize);
		
	}
	for (i = 0; i < 8; i++)
	{

		l_genkeys(seed3, seed4 + (i << 24),  0x12341234,
			LM_STRENGTH_113BIT,
			&prisize, prikey,
			&pubsize, pubkey);
		eq = 1;
		for (j = 0; j < prisize; j++)
			if (prikey[j] != savprikey[j])
			{
				eq = 0;
				break;
			}
		if (eq)
			printf("bug P5308 line %d\n", __LINE__);
		memcpy(savprikey, prikey, prisize);
		eq = 1;
		for (j = 0; j < pubsize; j++)
			if (pubkey[j] != savpubkey[j])
			{
				eq = 0;
				break;
			}
		if (eq)
			printf("bug P5308 line %d\n", __LINE__);
		memcpy(savpubkey, pubkey, pubsize);
		
	}
}
void
time_test(int strength)
{
 char *keystr;
 int i;
 int t;
 char lic[500];
 int cnt;
	gen_init(&gen_job, &code, strength);
	client_init(&client_job, &client_code, strength);
	keystr = testit(1, strength);
	sprintf(lic, "START_LICENSE\n%s\nEND_LICENSE", keystr);
	t = time(0);
	lc_free_job(client_job);
	client_init(&client_job, &client_code, strength);
	lc_set_attr(client_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(client_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)lic);
	for (cnt = 1, i = 0; time(0) - t < 10; i++, cnt++)
	{
		if (lc_checkout(client_job, "f1", "1.0", 1, LM_CO_NOWAIT, &client_code,
			LM_DUP_NONE))
		{
			fprintf(ofp, "fail line %d\n%s\n", __LINE__, 
			lc_errstring(client_job));
			exit(1);
		}
	}
	lc_free_job(client_job);
	fprintf(ofp, "%s-bit: %d ms.  ", 
		(strength == LM_STRENGTH_DEFAULT) ? "default" : 
		(strength == LM_STRENGTH_113BIT) ? "113" : 
		(strength == LM_STRENGTH_163BIT) ? "163" :
		"239", ((time(0) - t) * 1000 )/cnt);
	free(keystr);
	lc_free_job(gen_job);
}
void
strength_test(int strength)
{
  int i;
  char *keystr, *cp;
  CONFIG *conf;
  char savkey[50];
  char savsign[500];
  char savkeystr[500];

	gen_init(&gen_job, &code, strength);
	memset(savkey, 0, sizeof(savkey)) ;
	memset(savsign, 0, sizeof(savsign)) ;
	memset(savkeystr, 0, sizeof(savkeystr)) ;
	/* 
	 *	Regenerate the same license 3 times, and make sure the 
	 *	results match
	 */
	for (i = 0; i < 5; i++) 
	{
		client_init(&client_job, &client_code, strength);
		if (!(keystr = testit(0, strength)))
			continue;
		for (cp = keystr; *cp; cp++)
			if (*cp == '\n' || *cp == '\r')
				*cp = ' ';
		conf = (CONFIG *)l_malloc(client_job, sizeof(CONFIG));
		if(!conf)
		{
			LM_SET_ERROR(client_job, LM_CANTMALLOC, 604, 0, 0, LM_ERRMASK_ALL);
			return;
		}
		l_parse_feature_line(client_job, keystr, conf, 0);
		if (*savkey && strcmp(conf->code, savkey))
			fprintf(ofp, "consecutive licensekeys don't matchline %d", __LINE__);
		if (*savsign && strcmp(conf->lc_sign, savsign))
			fprintf(ofp, "consecutive signatures don't match line %d\n%s\n%s\n", __LINE__, keystr, savkeystr);
		strcpy(savkey, conf->code);
		strcpy(savsign, conf->lc_sign);
		strcpy(savkeystr, keystr);
		l_free_conf(client_job, conf);
		conf = NULL;
		free(keystr);
		lc_free_job(client_job);
	}
	lc_free_job(gen_job);
}

/*
 *	f1-100 test.  The idea is to make sure that 
 *	when we generate licenses on a fast machine, that
 *	the first few characters of the CRO signature are not the
 *	same.  This happens when the seed doesn't change.
 *	This was a bug in v7.2, where the seed would be the same
 *	when the time didn't change
 */


f1_100test()
{
  char *keystr, *err;
  char feat[500];
  char savkeystr[500];
  char savsign[500];
  char lic[500];
  CONFIG *conf;
  int f;
  char *cp;
	client_init(&client_job, &client_code, LM_STRENGTH_113BIT);
	gen_init(&gen_job, &code, LM_STRENGTH_113BIT);
	for (f = 0; f < 200; f++)
	{
		sprintf(feat, 
		"FEATURE f%d demo 1.0 permanent  uncounted 0 HOSTID=ANY SIGN=0",
			f);
		if (lc_cryptstr(gen_job, feat, &keystr, &code, LM_CRYPT_FORCE, 
					"stdin", &err))
		{
			fprintf(ofp, "cryptstr failed %s %s line %d\n", 
					lc_errstring(gen_job), err, __LINE__);
			free(err);
			if (keystr) free(keystr);
			return 0;
		}
		for (cp = keystr; *cp; cp++)
			if (*cp == '\n' || *cp == '\r')
				*cp = ' ';
		conf = (CONFIG *)l_malloc(client_job, sizeof(CONFIG));
		if(!conf)
		{
			LM_SET_ERROR(client_job, LM_CANTMALLOC, 605, 0, 0, LM_ERRMASK_ALL);
			return 0;
		}
		l_parse_feature_line(client_job, keystr, conf, 0);
		if (*savsign && !strncmp(conf->lc_sign, savsign, 14))
			fprintf(ofp, 
			"Bug:  seeds don't have random value! %s/%sline%d\n",conf->lc_sign, savsign, 
					__LINE__);
		strcpy(savsign, conf->lc_sign);
		l_free_conf(client_job, conf);
		free(keystr);
	}
	lc_free_job(gen_job);
	lc_free_job(client_job);
	return 1;
}
void
badkey(int strength)
{
  char *keystr, *err;
  char feat[500];
  char badkeystr[500];
  char lic[500];
  int rc;
	gen_init(&gen_job, &code, strength);
	strcpy(feat, 
	"FEATURE f1 demo 1.0 permanent  uncounted HOSTID=ANY SIGN=0");
	if (lc_cryptstr(gen_job, feat, &keystr, &code, LM_CRYPT_FORCE, 
		"stdin", &err))
	{
		fprintf(ofp, "cryptstr failed %s %s line %d\n", lc_errstring(gen_job),
			err, __LINE__);
		free(err);
		if (keystr) free(keystr);
		exit(0);
	}
	strcpy(badkeystr, keystr);
	badkeystr[9] = '2';
	sprintf(lic, "START_LICENSE\n%s\n%s\nEND_LICENSE", keystr, badkeystr);

	client_init(&client_job, &client_code, strength);
	if (strength == LM_STRENGTH_DEFAULT)
	{
	  L_KEY_FILTER *f = (L_KEY_FILTER *)client_job->key_filters, *next;
		for (; f; f = next)
		{
			next = f->next;
			free(f);
		}
		client_job->key_filters = 0;
	}

	lc_set_attr(client_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(client_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)lic);
	if (lc_checkout(client_job, "f1", "1.0", 1, LM_CO_NOWAIT, 
		&client_code, LM_DUP_NONE))
		fprintf(ofp, "fail line %d error %d/%d strength %d\n", __LINE__, 
			client_job->lm_errno, 
			client_job->err_info.min_errno, 
			strength );
	if ((rc = lc_checkout(client_job, "f2", "1.0", 1, LM_CO_NOWAIT, 
		&client_code, LM_DUP_NONE)) != LM_BADCODE)
		fprintf(ofp, "fail line %d should have got BADCODE, got %d\n", 
			__LINE__, rc);
	free(keystr);
	lc_free_job(client_job);
	lc_free_job(gen_job);
}
void
gen_init(LM_HANDLE **lm_job, VENDORCODE *site_code, int strength)
{
  static int first = 1;
	if (first)
	{
		LM_CODE_GEN_INIT(site_code);
		first = 0;
	}
	if (lc_init((LM_HANDLE *)0, VENDOR_NAME, site_code, lm_job))
	{
		lc_perror(*lm_job, "lc_init failed");
		exit(-1);
	}
	lc_set_attr(*lm_job, LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)
		strength);
	lc_set_attr(*lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)
		1);
}
void
client_init(LM_HANDLE **lm_job, VENDORCODE *site_code, int strength)
{
	if (lc_new_job((LM_HANDLE *)0, 0, site_code, lm_job))
	{
		lc_perror(*lm_job, "lc_new_job failed");
		exit(-1);
	}
	(*lm_job)->flags |= LM_FLAG_SLOW_VERIFY;
	lc_set_attr(*lm_job, LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)
		strength);
	lc_set_attr(*lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)
		1);
}
portability_gen(strength)
{
  char hostid1[50];
  char hostid2[50];
  char hostid3[50];
  char name[50];
  int version;
  int revision;
  int year = 2020;
  char mon[50];
  char day = 1;
  int count = 1;
  char nodelock[50];
  char feature[500];
  char *keystr, *err;
  static FILE *fp;
  char *cp;

	if (!fp)
		fp = fopen("crotest.exp", "w");
	fprintf(fp, "%s\n", 
		(strength == LM_STRENGTH_DEFAULT) ? "LM_STRENGTH_DEFAULT" :
		(strength == LM_STRENGTH_113BIT) ? "LM_STRENGTH_113BIT" :
		(strength == LM_STRENGTH_163BIT) ? "LM_STRENGTH_163BIT" :
		(strength == LM_STRENGTH_239BIT) ? "LM_STRENGTH_239BIT" :
			"ERROR");
	gen_init(&gen_job, &code, strength);
	strcpy(hostid1, "12345678");
	strcpy(hostid2, "1234567890ab"); /* ether */
	strcpy(hostid3, "FLEXID=7-12345678");
	strcpy(name, "longfeaturenamebutevenlonger");
	version = revision = 1;
	strcpy(mon, "jan");
	sprintf(feature,"\
SERVER host1 %s\n\
FEATURE %s demo %d.%d %d-%s-%d %d SIGN=0",
		hostid1, name, version, revision, day, mon, year, count);
	gen(feature, gen_job, &code, fp);
	sprintf(feature,"\
SERVER host1 %s\n\
SERVER host2 %s\n\
SERVER host3 %s\n\
FEATURE %s demo %d.%d %d-%s-%d %d SIGN=0",
		hostid1, hostid2, hostid3, name, version, revision, day, mon, year, count);
	gen(feature, gen_job, &code, fp);
	sprintf(feature,"\
FEATURE %s demo %d.%d %d-%s-%d uncounted SIGN=0 HOSTID=DEMO",
		name, version, revision, day, mon, year);
	gen(feature, gen_job, &code, fp);
	sprintf(feature,"\
FEATURE %s demo %d.%d permanent uncounted SIGN=0 HOSTID=FLEXID=8-88887777",
		name, version, revision);
	gen(feature, gen_job, &code, fp);
	sprintf(feature,"\
FEATURE %s demo %d.%d permanent uncounted SIGN=0 HOSTID=FLEXID=8-88887777\n\
NOTICE=\"This is a notice field\"",
		name, version, revision);
	gen(feature, gen_job, &code, fp);
	lc_free_job(gen_job);
}
portability()
{
  FILE *fp;
  char line[200];
  int strength;
  char buf[2000];
  int linenum = 0;
  int good = 0;

	fp = fopen("crotest.exp", "r");
	*buf = 0;
	strength = 0;
	while(fgets(line, 200, fp))
	{
		linenum++;
		if (!strncmp(line, "LM_STRENGTH", 11))
		{
			if ( !strcmp(line, "LM_STRENGTH_113BIT\n") )
				strength = LM_STRENGTH_113BIT;
			else if ( !strcmp(line, "LM_STRENGTH_163BIT\n") )
				strength = LM_STRENGTH_163BIT;
			else if ( !strcmp(line, "LM_STRENGTH_239BIT\n") )
				strength = LM_STRENGTH_239BIT;
			else if ( !strcmp(line, "LM_STRENGTH_DEFAULT\n") )
				strength = LM_STRENGTH_DEFAULT;
			else 
			{
				printf("Bad strength %s, existing\n", line);
				exit(1);
			}
			continue;
		}
		else if (!strcmp(line, ".\n"))
		{
		  CONFIG *conf;
		  char licstr[2000];
		  char **feats;

			client_init(&client_job, &client_code, strength);
			sprintf(licstr, "START_LICENSE\n%s\nEND_LICENSE", buf);
			lc_set_attr(client_job, LM_A_DISABLE_ENV, 
				(LM_A_VAL_TYPE)1);
			lc_set_attr(client_job, LM_A_LICENSE_FILE, 
				(LM_A_VAL_TYPE)licstr);
			feats = lc_feat_list(client_job, 0, 0);
			conf = lc_get_config(client_job, *feats);
			if (lc_check_key(client_job, conf, &client_code) != good)
			{
				printf("Error line %d\n\"%s\"\n\"%s\"\n",
					linenum, lc_errstring(client_job),
					buf);
			}
			if (good) good = 0;
			else good = LM_BADCODE;
			lc_free_job(client_job);
			*buf = 0;
			continue;
		}
		strcat(buf, line);
	}
	
}
void 
gen(char *feature, LM_HANDLE *gen_job, VENDORCODE *code, FILE *fp)
{
char *keystr, *err, *cp;

	if (lc_cryptstr(gen_job, feature, &keystr, code, LM_CRYPT_FORCE, 
		"stdin", &err))
	{
		fprintf(ofp, "cryptstr failed %s %s line %d\n", lc_errstring(gen_job),
			err, __LINE__);
		free(err);
		if (keystr) free(keystr);
		exit(0);
	}
	fprintf(fp, "%s.\n", keystr);
	cp = strchr(keystr, 'l'); *cp = 'm'; /* replace 'l' with 'm' */
	fprintf(fp, "%s.\n", keystr);
	free(keystr);
}
char *
testit(int genonly, int strength)
{
  char *keystr, *err;
  char feat[500];
  char savkeystr[500];
  char lic[500];
#if 0
	if (strength == LM_STRENGTH_DEFAULT)
	strcpy(feat, 
	"FEATURE f1 demo 1.0 permanent  uncounted HOSTID=ANY SIGN=0");
	else
#endif
	strcpy(feat, 
	"FEATURE f1 demo 1.0 permanent  uncounted 0 HOSTID=ANY SIGN=0");
	if (lc_cryptstr(gen_job, feat, &keystr, &code, LM_CRYPT_FORCE, 
		"stdin", &err))
	{
		fprintf(ofp, "cryptstr failed %s %s\n", lc_errstring(gen_job),
			err);
		free(err);
		if (keystr) free(keystr);
	}
	else if (!genonly)
	{
		sprintf(lic, "START_LICENSE\n%s\nEND_LICENSE", keystr);
		lc_set_attr(client_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
		lc_set_attr(client_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)lic);
		if (lc_checkout(client_job, "f1", "1.0", 1, LM_CO_NOWAIT, 
			&client_code, LM_DUP_NONE))
			fprintf(ofp, "fail line %d\n%s\n", __LINE__, lc_errstring(client_job));
	}
	return(keystr);
}
