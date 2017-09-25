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
 *	Module: $Id: lm_lic_info.c,v 1.8.4.2 2003/07/01 17:04:21 sluu Exp $
 *
 *	Function:	license_info
 *
 *	Description: 	description of license file line
 *
 *	Parameters:	lmstat flag, CONFIG *
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  12/8/98
 *
 */
#include "lmutil.h"
#include "flex_utils.h"

static void do_types lm_args((CONFIG *));
static void do_options lm_args((CONFIG *));
static void do_dup_group lm_args((CONFIG *));

void
license_info(lmstat, conf)
int lmstat;
CONFIG *conf;
{
  int sd, ed, sm, em, sy, ey;
  char smc[10], emc[10];
  HOSTID *idp, *sav = (HOSTID *)0;
  char *str = "locked to";
  int use_start = (strlen(conf->code) == MAX_CRYPT_LEN);

	if (lmstat) (void) fprintf(ofp, "  ");
	(void) fprintf(ofp, lmtext("\"%s\" v%s, vendor: %s\n"), 
				conf->feature, conf->version, conf->daemon);

	if (conf->users == 0)
	{
		(void) fprintf(ofp, lmtext("  uncounted nodelocked license"));
	}
	else
	{
		if (!lmstat)
		{
		    if (conf->server && conf->server->next)
		    {
			(void) fprintf(ofp, lmtext("  License servers: %s %s %s\n"),
				conf->server->name,
				conf->server->next->name,
				conf->server->next->next ?
					conf->server->next->next->name : "");
		    }
		    else if (conf->server)
		    {
				(void) fprintf(ofp, lmtext("  License server: %s\n"),
					conf->server->name);
		    }
		}
				
		(void) fprintf(ofp, lmtext("  %s license"), !conf->idptr ? 
						"floating" : "nodelocked");
	}

	for (idp = conf->idptr; idp; idp = idp->next) 
	{
	  char *type;
	  int print = 1;

		
	
		switch(idp->type)
		{
		  case HOSTID_LONG: type = "hostid "; break;
		  case HOSTID_ETHER: type = "ethernet address "; break;
		  case HOSTID_ANY:
			(void) fprintf(ofp, lmtext(" %s NOTHING (hostid=ANY)"),
									str);
			print = 0; break;
		  case HOSTID_USER: 
			(void) fprintf(ofp, lmtext("%s username \"%s\""), 
				    		str, idp->hostid_user);
			print = 0; break;
		  case HOSTID_DISPLAY:
			(void) fprintf(ofp, lmtext("%s display \"%s\""), 
				    		str, idp->hostid_display);
			print = 0; break;
		  case HOSTID_HOSTNAME:
			(void) fprintf(ofp, lmtext("%s hostname \"%s\""), 
				    		str, idp->hostid_hostname);
			print = 0; break;
		  case HOSTID_ID_MODULE: type = "HP ID module "; break;
		  case HOSTID_FLEXID1_KEY: 
		  case HOSTID_FLEXID2_KEY: 
		  case HOSTID_FLEXID4_KEY: 
		  case HOSTID_FLEXID5_KEY: type = "FLEXID dongle "; break;
		  case HOSTID_DISK_SERIAL_NUM: type = "Disk serial number "; 
				break;
		  case HOSTID_INTERNET: type = "Internet address "; break;
#if 0
		  case HOSTID_CPU: type = "Microprocessor identifier "; break;
		  case HOSTID_DISK_GEOMETRY: type = "Hard disk geometry identifier"; break;
		  case HOSTID_BIOS: type = "BIOS identifier "; break;
#endif
		  case HOSTID_COMPOSITE: 
				(void) fprintf(ofp, lmtext("%s composite \"%s\""), 
								str, idp->hostid_composite);
				print = 0; 
				break;

#ifdef SUPPORT_METER_BORROW
		  case HOSTID_METER_BORROW: type = "FLEXid Borrow Meter"; break;
#endif /* SUPPORT_METER_BORROW */
		  default: type = ""; break;
		}
		if (idp->type >= HOSTID_VENDOR) 
			type = "Vendor-defined "; 
		if (print)
		{
			char buf[MAX_HOSTID_LEN + 1] = {'\0'};	/* OVERRUN */
			l_asc_id_one(lm_job, idp, 0, buf);
			if (sav)
				fprintf(ofp, "\n\t");
			else
				fprintf(ofp, ", ");
			(void) fprintf(ofp, lmtext("%s %s\"%s\""), 
				    str, type, buf);

			if (idp->next) fprintf(ofp, ",");
		}
		sav = idp;
		str = "or";
	}
	if (!lmstat)
	{
/*
 *		Do some checking on the dates
 */
		(void) sscanf(conf->date, "%d-%[^-]-%d", &ed, emc, &ey);
		if (use_start)
		{
			sscanf(l_asc_date(l_extract_date(lm_job, conf->code)), 
					"%d-%[^-]-%d", &sd, smc, &sy);
			sm = l_int_month(smc);
			em = l_int_month(emc);
			if (ey && ((sy > ey) || 
				((sy == ey) && ((sm > em) || 
					((sm == em) && sd >= ed)))))
			{
				fprintf(ofp, lmtext(
"  ** WARNING: This may be an invalid license: start date >= end_date\n"));
			}
			fprintf(ofp, lmtext("  starts: %s, "), 
				      l_asc_date(l_extract_date(lm_job, 
				      conf->code)));
		}
		if (ey == 0)

			fprintf(ofp, 
			lmtext(" no expiration date\n\n"));
		else
			fprintf(ofp, 
			lmtext("  expires: %s\n"), conf->date);
		if (conf->lc_got_options & LM_LICENSE_TYPE_PRESENT)
			do_types(conf);
		if (conf->lc_got_options & LM_LICENSE_OPTIONS_PRESENT)
			do_options(conf);
		if (conf->lc_got_options & LM_LICENSE_DUP_PRESENT)
			do_dup_group(conf);
		if (conf->lc_got_options & LM_LICENSE_OVERDRAFT_PRESENT)
			fprintf(ofp, 
		"  %d Overdraft available, %d in total (= %d + %d)\n", 
				conf->lc_overdraft, 
					conf->users + conf->lc_overdraft,
					conf->users, conf->lc_overdraft);
			
	}
	else (void) fprintf(ofp, "\n");
	fprintf(ofp, "\n");
}
static
void
do_types(conf)
CONFIG *conf;
{
	if (conf->lc_type_mask & LM_TYPE_CAPACITY) 	
		fprintf(ofp, 
	"  CAPACITY license: checkout count varies based on client platform\n");
	if (conf->lc_type_mask & LM_TYPE_USER_BASED) 	
		fprintf(ofp, 
	"  USER_BASED: Requires all licenses be RESERVEd to USERs\n");
	if (conf->lc_type_mask & LM_TYPE_HOST_BASED) 	
		fprintf(ofp, 
	"  HOST_BASED: Requires all licenses be RESERVEd to HOSTs via options file\n");
	if (conf->lc_type_mask & LM_TYPE_METER) 	
		fprintf(ofp, 
	"  METERED: Checkout via FLEXmeter\n");
	if (conf->lc_type_mask & LM_TYPE_TS_OK) 	
	fprintf(ofp, 
	"  TS_OK: Checkout permitted when client is using terminal client\n");
}
static
void
do_options(conf)
CONFIG *conf;
{
	if (conf->lc_type_mask & LM_OPT_SUITEBUNDLE) 	
	{
		fprintf(ofp, 
	"  SUITE: The name of this package becomes a FEATURE which is\n");
		fprintf(ofp, 
	"         checked out in combination with any COMPONENT FEATURE.\n");
	}
	if (conf->lc_type_mask & LM_OPT_SUPERSEDE) 	
	{
		fprintf(ofp, 
	"  Supersedes FEATURE/INCREMENT lines for \"%s\"\n", 
				conf->feature);
		fprintf(ofp, 
	"             created before %s\n", conf->lc_issued ?
		conf->lc_issued : l_asc_date(l_extract_date(lm_job, conf->code)));
	}
}
static
void
do_dup_group(conf)
CONFIG *conf;
{
  int dup_group = conf->lc_dup_group;
  int got = 0;
	
	if (dup_group & LM_DUP_NONE)
		fprintf(ofp, "\tNo duplicate grouping on this feature\n");
	else if (dup_group == LM_DUP_SITE) /* 0 */
		fprintf(ofp, "\tAll uses by all users are counted a 1 use\n");
	else {
		fprintf(ofp, "  Requests from the same ");
		if (dup_group & LM_DUP_USER) { 
			fprintf(ofp, "USER"); got = 1; 
		}
		if (dup_group & LM_DUP_HOST) 
		{
			if (got) fprintf(ofp, "/"); 
			fprintf(ofp, "HOST"); got = 1;
		}
		if (dup_group & LM_DUP_DISP) 
		{
			if (got) fprintf(ofp, "/"); 
			fprintf(ofp, "DISPLAY"); got = 1;
		}
		if (dup_group & LM_DUP_VENDOR) 
		{
			if (got) fprintf(ofp, "/"); 
			fprintf(ofp, "VENDOR-DEFINED*"); 
		}
		fprintf(ofp, " do not consume a new license\n");
	}
	if (dup_group & LM_DUP_VENDOR)
	{
		fprintf(ofp, 
		"\t*vendor-defined is a customized grouping -- ask your software provider\n");
	}
}
