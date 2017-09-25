/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lg_vkeys.c,v 1.3 2003/04/18 23:47:50 sluu Exp $
 *
 *	Function:	ls_upgrade_vkeys
 *
 *	Description: 	This file must never be shipped to source customers.
 *
 *	D. Birns
 *	12/6/95
 *
 *	Last changed:  10/23/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#ifdef LM_GENERIC_VD
#include "lg_code.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "lgetattr.h"
#include "ls_sprot.h"
#include "ls_aprot.h"
#include "ls_glob.h"
#include "lm_attr.h"
#include "l_prot.h"

static LM_HANDLE *upgrade_job;

#ifndef PC
static
int
app_filter(conf)
CONFIG *conf;
{
  int ret = -1; /* assume failure */
  char feature[20], *cp = feature;
  
	if (!conf->lc_vendor_def || !*conf->lc_vendor_def)
		return 0;

	strcpy(feature, l_platform_name());	/* OVERRUN */
	cp = strchr(feature, '_');
	cp += 2;
	*cp = '\0';
	if (!strcmp(feature, "sun4_u"))
		strcpy(feature, "sun");
	else if (!strcmp(feature, "alpha_u"))
		strcpy(feature, "alpha_osf");
	else if (!strncmp(feature, "cray", 4))
		strcpy(feature, "cray");
	else if (!strcmp(feature, "decs_u"))
		strcpy(feature, "dec");
	else if (!strcmp(feature, "moto_u"))
		strcpy(feature, "motorola");
	else if (!strcmp(feature, "ppc_x"))
		strcpy(feature, "sun_ppc");
	else if (!strcmp(feature, "i86_x"))
		strcpy(feature, "sunx86");
	else if (!strcmp(feature, "i86_u"))
		strcpy(feature, "unixware");
	else if (!strcmp(feature, "i86_l"))
		strcpy(feature, "linux");
	else if (!strcmp(feature, "i86_n"))
		strcpy(feature, "winnt_intel");
	else if (!strcmp(feature, "i86_z"))
		strcpy(feature, "netware");
	else if (!strcmp(feature, "i86_w"))
		strcpy(feature, "windows");

	if (!strcmp(feature, conf->lc_vendor_def)) 	ret = 0;
	else 	lc_set_errno(upgrade_job, LM_BADPLATFORM);
	return ret;
}
#endif /* PC */

/*-
 *	Upgrade vendorkeys
 */


void
ls_upgrade_vkeys(argc, argv, vendor_name)
int argc;
char *argv[];
char *vendor_name;
{
  int i;
  extern LM_HANDLE *lm_job;

LM_CODE(upgradecode, UPGRADE_SEED_1, UPGRADE_SEED_2, UPGRADE_KEY1,
	UPGRADE_KEY2, UPGRADE_KEY3, UPGRADE_KEY4, UPGRADE_KEY5);

	if (lc_init(lm_job, vendor_name, &upgradecode, &upgrade_job))
		return; /* no can do! */
	lc_set_attr(upgrade_job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE)0);

	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-c") && (++i < argc))
		{
			lc_set_attr(upgrade_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
			lc_set_attr(upgrade_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
								argv[i]);
		}
#ifndef PC
	lc_set_attr(upgrade_job, LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)app_filter);
#endif /* PC */
	if (!lc_checkout(upgrade_job, UPGRADE_SERVER_FEATURE, "1.0", 1, LM_CO_LOCALTEST, 
		&upgradecode, LM_DUP_NONE))
	{
		lm_job->attrs[ADVANCED_HOSTIDS] = ADVANCED_HOSTIDS_VAL;
		lm_job->attrs[END_USER_OPTIONS] = END_USER_OPTIONS_VAL;
		lm_job->attrs[UPGRADE_INCREMENT] = UPGRADE_INCREMENT_VAL;
		lm_job->attrs[LM_RUN_DAEMON] = LM_RUN_DAEMON_VAL;
		lm_job->attrs[DUP_SERVER] = DUP_SERVER_VAL;
		lm_job->attrs[LICENSE_SEEDS] = LICENSE_SEEDS_VAL;
	} 
#ifndef PC
	else if (l_getattr(lm_job, LM_RUN_DAEMON) != LM_RUN_DAEMON_VAL) 
	{
		if (lc_checkout(upgrade_job, GENERIC_SERVER_FEATURE, "1.0", 1, 
				LM_CO_LOCALTEST, &upgradecode, LM_DUP_NONE))
		{
		
			LOG((lmtext(
			"flexlmd not licensed for this host, exiting\n")));
						
			ls_go_down(EXIT_WRONGHOST);
		}
	}
		
#endif /* PC */
	lc_free_job(upgrade_job);
	return;
}
#endif /* LM_GENERIC_VD */
