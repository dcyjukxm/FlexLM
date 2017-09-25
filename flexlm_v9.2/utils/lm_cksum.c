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
 *	Module: $Id: lm_cksum.c,v 1.4 2003/01/13 21:58:48 kmaclean Exp $
 *
 *	Function: 	lmcksum
 *
 *	Description:  	
 *
 *	Parameters:	daemon
 *
 *	Return:		void
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  11/2/98
 *
 */
#include "lmutil.h"

unsigned output_serv(), output_conf(), usum();

lm_cksum(daemon)
char *daemon;
{
  char string[MAX_CONFIG_LINE+1];
  LICENSE_FILE  *lfptr;    
  unsigned cksum = 0;
  int ret = 0;

	lm_job->flags |= LM_FLAG_CKSUM_ONLY;
	fprintf(ofp, lmtext("lmcksum: using license file \"%s\"\n\n"), 									lm_lic_where());
	lfptr = l_open_file(lm_job, LFPTR_FIRST);
	if (!lfptr) 
	{
		fprintf(ofp, lmtext("l_open_file: %s\n"), 
					lmtext(lc_errstring(lm_job)));
		lm_job->flags &= ~LM_FLAG_CKSUM_ONLY;
		return(0);
	}
	while (l_lfgets(lm_job, string, MAX_CONFIG_LINE, lfptr, 0))
	{
	 int nflds;
	 char s1[MAX_CONFIG_LINE], s2[MAX_CONFIG_LINE];

	    s1[0] = s2[0] = '\0';
	    nflds = sscanf(string, "%s %s", s1, s2);
	    if (nflds > 1)
	    {
		if (l_keyword_eq(lm_job, s1, LM_RESERVED_FEATURE) ||
		    l_keyword_eq(lm_job, s1, LM_RESERVED_INCREMENT) ||
		    l_keyword_eq(lm_job, s1, LM_RESERVED_PACKAGE) ||
		    l_keyword_eq(lm_job, s1, LM_RESERVED_UPGRADE))
		{
		  CONFIG *conf = (CONFIG *)l_malloc(lm_job, sizeof (CONFIG));

		  if(!conf)
		  {
			  LM_SET_ERROR(lm_job, LM_CANTMALLOC, 606, 0, 0, LM_ERRMASK_ALL);
			  return 0;
		  }

			memset((char *) conf, '\0', sizeof(CONFIG));
/*
 *			Make sure cksum flag is TRUE!
 */
			if (l_parse_feature_line(lm_job, string, conf, 
								(char **)0))
			{
				if (*daemon == 0 || 
				    !strcmp(daemon, conf->daemon))
				      cksum += output_conf(string, conf, &ret);
			}
			l_free_conf(lm_job, conf);
		}
		else if (l_keyword_eq(lm_job, s1, LM_RESERVED_SERVER))
		{
		  LM_SERVER *serv = (LM_SERVER *)l_malloc(lm_job, sizeof(LM_SERVER));

			memset((char *) serv, '\0', sizeof(LM_SERVER));
			if (l_parse_server_line(lm_job, string, serv))
			{
				cksum += output_serv(string, serv);
			}
			l_free_server(lm_job, serv);
		}
		else if (l_keyword_eq(lm_job, s1, LM_RESERVED_PROG)
		 ||l_keyword_eq(lm_job, s1, LM_RESERVED_PROG_ALIAS))
		{
		  char junk[MAX_CONFIG_LINE]; 
		  char f1[MAX_CONFIG_LINE], f2[MAX_CONFIG_LINE],
			f3[MAX_CONFIG_LINE], program[MAX_CONFIG_LINE];
		  unsigned x = 0;

			f1[0] = f2[0] = f3[0] = program[0] = '\0';
			nflds = sscanf(string, "%s %s %s %s %s", junk, f1,
						program, f2, f3);
			if (nflds >= 2)
			{					/* Found one */
				if (*daemon == 0 || !strcmp(f1, daemon))
				{
					x = l_sum(f1);
					fprintf(ofp, "     %3d: %s", 
							l_onebyte(x), string);
					if (string[strlen(string) - 1] != '\n')
						fprintf(ofp, "\n");
					cksum += x;
				}
			}
		}
		else if (l_keyword_eq(lm_job, s1, LM_RESERVED_FEATURESET))
		{
		  char f1[MAX_CONFIG_LINE], f2[MAX_CONFIG_LINE];
		  unsigned x = 0;

			f1[0] = f2[0] = '\0';
			nflds = sscanf(string, "FEATURESET %s %s", f1, f2);
			if (nflds >= 2)
			{
				if (*daemon == 0 || !strcmp(daemon, f1))
				{
					x = l_sum(f1);
					x += usum(f2);
					fprintf(ofp, "%3d: %s", l_onebyte(x), 
									string);
					cksum += x;
				}
			}
		}
	    }
	}
	fprintf(ofp, lmtext("     %3d: (overall file checksum)\n"), 
							l_onebyte(cksum));
	lm_job->flags &= ~LM_FLAG_CKSUM_ONLY;
	return(ret);
}

unsigned usum(str)
char *str;
{
  unsigned res = 0;

	while (*str)
	{
		res += (unsigned) (isupper(*str) ? tolower(*str) : *str);
		str++;
	}
	return(res);
}

unsigned
output_serv(string, serv)
char *string;
LM_SERVER *serv;
{
  unsigned x = 0;
	
	x = l_sum(l_asc_hostid(lm_job, serv->idptr));
	fprintf(ofp, "     %3d: %s", l_onebyte(x), string);
	if (string[strlen(string) - 1] != '\n')
		fprintf(ofp, "\n");
	return(x);
}

unsigned
output_conf(string, conf, bad)
char *string;
CONFIG *conf;
int *bad;
{
  unsigned x = 0;
  int temp;
  int cs;
	
	conf->server = (LM_SERVER *) NULL;
	temp = conf->lc_got_options;
	cs = conf->lc_cksum;
	conf->lc_cksum = 0;
	conf->lc_got_options &= (~LM_LICENSE_CKSUM_PRESENT);
	x = l_cksum(lm_job, conf, bad, &code);
	if (temp & LM_LICENSE_CKSUM_PRESENT)
	{
		if (x != (unsigned) cs) 
		{
		  long sav = lm_job->options->flags;
			lm_job->options->flags |= 
				LM_OPTFLAG_STRINGS_CASE_SENSITIVE; 
			x = l_cksum(lm_job, conf, bad, &code);
			lm_job->options->flags = sav;
			if (x != (unsigned) cs)  /* set it back */
				x = l_cksum(lm_job, conf, bad, &code);
		}
		if (x == (unsigned) cs) fprintf(ofp, "OK:  ");
		else
		{
			fprintf(ofp, "BAD: ");
			*bad = 1;
		}
	}
	else
		fprintf(ofp, "     ");
	fprintf(ofp, "%3d: %s", l_onebyte(x), string);
	if (string[strlen(string) - 1] != '\n')
		fprintf(ofp, "\n");
	return(x);
}
