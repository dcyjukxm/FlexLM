/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_inst.c,v 1.14 2003/05/05 16:12:02 sluu Exp $
 *	Function:	 lminstall
 *
 *	D. Birns
 *	7/17/97
 *
 *	Last changed:  12/8/98
 *
 */
#include "lmutil.h"
#include "flex_file.h"
#include <sys/stat.h>

extern FILE *ofp;
static void strip_trailing_blanks lm_args((char *));
static void check_syntax lm_args((char *));
extern char *copyright;  
static int max_lines = 400;

lmutil_install(argc, argv)
char **argv;
{
  FILE *ifp = 0;
  int i;
  char  *buf;

  char *cp;
  char *ifname = 0, *ofile_str, *err;
  int cryptstr_flags = LM_CRYPT_IGNORE_FEATNAME_ERRS;
  int badargs = 0;

  	buf = (char *) l_malloc(lm_job, MAX_CONFIG_LINE * max_lines);

	for (i = 1; i < argc; i++)
	{
		if (l_keyword_eq(lm_job, argv[i], "-i"))
		{
			i++;
			if (i >= argc) break;
			if (L_STREQ(argv[i], "-"))
			{
				ifp = stdin;
				ifname = "stdin";
			}
			else if (!(ifp = l_flexFopen(lm_job, argv[i], "r")))
			{
				fprintf(stderr, "Can't open %s: ", argv[i]);
				perror("");
				usage();
				exit(1);
			}
			else ifname = argv[i];
				
		}
		else if (l_keyword_eq(lm_job, argv[i],"-odecimal"))
			cryptstr_flags |= LM_CRYPT_DECIMAL_FMT;
		else if (l_keyword_eq(lm_job, argv[i],"-help"))
		{
			usage();
			exit(0);
		}
		else if (l_keyword_eq(lm_job, argv[i],"-maxlen"))
		{
		  int max = 0;
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &max);
			if (!max) fprintf(stderr, 
				"Error: -maxlen %s Invalid line length\n", argv[i]);
			else
				lc_set_attr(lm_job, LM_A_MAX_LICENSE_LEN, 
					(LM_A_VAL_TYPE)max);
		}
		else if (l_keyword_eq(lm_job, argv[i],"-overfmt"))
		{
			i++;
			if (i >= argc) break;
			switch(*argv[i])
			{
			case '2':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V2); break;
			case '3':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V3); break;
			case '4':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V4); break;
			case '5':
				if (!strncmp(argv[i],"5.1", 3))
					lc_set_attr(lm_job, 
						LM_A_LICENSE_FMT_VER, 
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V5_1);
				else
					lc_set_attr(lm_job, 
						LM_A_LICENSE_FMT_VER, 
					(LM_A_VAL_TYPE)LM_BEHAVIOR_V5); 
				break;
			case '6':
				lc_set_attr(lm_job, LM_A_LICENSE_FMT_VER, 
				(LM_A_VAL_TYPE)LM_BEHAVIOR_V6); break;
			}
		}
		else 
		{
                        fprintf(ofp, "Unknown argument %s\n", argv[i]);
                        badargs++;
                
                }
	}
	if (badargs)
	{
	        usage();
	        exit(1);
	}
	if (!ifp)
	{
		get_lic_from_user(buf);
	}
	else
	{
	  int linenum;

		cp = buf;
		linenum = 0;
		while (fgets(cp, MAX_CONFIG_LINE, ifp))
		{
		  
			cp += strlen(cp);
			linenum++;
			if (linenum >= max_lines)
			{
			  char *tmp;
			  int nmax_lines = max_lines *2;

				tmp = (char *)l_malloc(lm_job, nmax_lines * 
							MAX_CONFIG_LINE);
				memcpy(tmp, buf, max_lines * MAX_CONFIG_LINE);
				max_lines = nmax_lines;
				cp = tmp + (cp - buf);
				free(buf);
				buf = tmp;
			}

		}
	}

	if (!*buf)
	{
		fprintf(stderr, "No input, exiting\n");
		exit(1);
	}
	if (lc_cryptstr(lm_job, buf, &ofile_str, &code, 
			cryptstr_flags, ifname, &err))
	{
		fprintf(stderr, "Errors found: %s\n", err);
		exit (lm_job->lm_errno);
	}
	else if (err)
	{
		fputs(err, stderr);
		free(err);
	}
	check_syntax(ofile_str);
	fputs(ofile_str, ofp);
	exit(0);


}
static
void
check_syntax(file_str)
char *file_str;
{
  int len;
  char *buf;
  char **featlist;
  CONFIG *conf;
  char *err;

	len = strlen(file_str);
	if (!(buf = (char *)l_malloc(lm_job, len + 80)))
		return;
	sprintf(buf, "START_LICENSE\n%s\nEND_LICENSE", file_str);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	for (conf = lm_job->line; conf; conf = conf->next)
	{
		if (err = lc_chk_conf(lm_job, conf, 0))
		{
			fprintf(stdout, "Error: %s/%s: \"%s\"\n", conf->feature,
							conf->code, err);
			free(err);
		}
	}
	free(buf);

}

int
get_lic_from_user(buf)
char *buf;
{
  char inbuf[MAX_CONFIG_LINE];
  char storebuf[MAX_CONFIG_LINE];
  CONFIG *conf;
  LM_SERVER s;
  char dbuf1[100], dbuf2[100];
  struct stat st;
  int rc;
  char *err;
  char *cp = buf;
  char *cps = storebuf;
  int blanks = 0;
  char default_file[MAX_LONGNAME_SIZE + 1] = {'\0'};
  int d, m, y;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  long l;
  int i;
  int good;


	fprintf(ofp, "%s - %s", "lminstall", copyright);

	while (!ofp || ofp == stdout)
	{
#ifdef THREAD_SAFE_TIME
		l_get_date(&d, &m, &y, &l, &tst);
#else /* !THREAD_SAFE_TIME */
		l_get_date(&d, &m, &y, &l);
#endif
		y += 1900;
		m += 1;
		sprintf(default_file, "%d%02d%02d.lic", 
			y, m, d);	/* OVERRUN */
		for (i = 1; !l_flexStat(lm_job, default_file, &st); i++)
		{
			sprintf(default_file, "%d%02d%02d_%d.lic", 
				y,m,d,i);	/* OVERRUN */
		}
		fprintf (stdout, "Enter path to output license file\n");
		fprintf(stdout, "[default %s] q=quit:  ", default_file);
		fgets(inbuf, MAX_CONFIG_LINE, stdin);
		inbuf[strlen(inbuf) - 1] = 0; /* strip newline */
		if (*inbuf)
			strcpy(default_file, inbuf);	/* OVERRUN */
		if (l_keyword_eq(lm_job, inbuf, "q"))
		{
			fprintf(stdout, "Exiting\n");
			exit(0);
		}
		if (!l_flexStat(lm_job, default_file, &st))
		{
			fprintf(stdout,
			"File exists, do you want to append (y/n)? ");
			fgets(inbuf, MAX_CONFIG_LINE, stdin);
			inbuf[strlen(inbuf) - 1] = 0; /* strip newline */
			if (!l_keyword_eq(lm_job, inbuf, "y"))
				continue;
		}

		if (!(ofp = l_flexFopen(lm_job, default_file, "a")))		/* LONGNAMES */
		{
			fprintf(stdout, "Can't open output file %s\n", 
					default_file);
		}
	}
	fprintf(stdout, 
	"Please enter the license file below.  As you enter license lines,\n");
	fprintf(stdout,
	"they will be checked for errors.\n\n");
	fprintf(stdout, "[Enter Q to quit, or 2 Returns will also exit.]\n\n\n");
	while (fprintf(stdout, ": "), fgets(inbuf, MAX_CONFIG_LINE, stdin))
	{
		good = 0;
		strip_trailing_blanks(inbuf);
		if (inbuf[strlen(inbuf) - 1] == '\\')
		{
			strcpy(cps, inbuf);
			cps += strlen(cps);
			continue;
		} 
		else
		{
			strcpy(cps, inbuf);
			cps = storebuf;
		}
		if (!*inbuf) blanks++;
		if (blanks == 2 || l_keyword_eq(lm_job, storebuf, "q"))
			return 0;
		else if (blanks == 1) fprintf(stdout, "(One more RETURN to finish)\n");
			
		if (!*storebuf) continue;
		conf = l_malloc(lm_job, sizeof(CONFIG));
		if(!conf)
		{
			LM_SET_ERROR(lm_job, LM_CANTMALLOC, 607, 0, 0, LM_ERRMASK_ALL);
			return 0;
		}
		if (l_decimal_format(storebuf))
		{
			rc = l_parse_decimal(lm_job, (unsigned char *)storebuf, 
				conf, 0);
			if (!rc) 
			{
				fprintf(stdout, 
				"Invalid decimal line, please re-enter: %s\n", 
						lc_errstring(lm_job));
				
			}
			else
			{
				fprintf(stdout, "Good\n");
				good = 1;
			}
			
		}
		else if (l_parse_server_line(lm_job, storebuf, &s))
		{
			fprintf(stdout, "Good\n");
			good = 1;
		}
		else if (l_parse_daemon(lm_job, storebuf, dbuf1, dbuf2))
		{
			fprintf(stdout, "Good\n");
			good = 1;
		}
		else 
		{
			if (l_parse_feature_line(lm_job, storebuf, conf, &err) 
				&& !err)
			{
				if (err = lc_chk_conf(lm_job, conf, 0))
				{
					fprintf(stdout, "Error: \"%s\"\n", err);
					free(err);
				}
				else
				{
					fprintf(stdout, "Good\n");
					good = 1;
				}
			}
			else 
			{
				if (err)
				{
					fprintf(stdout, "Syntax error: \"%s\"\n", err);
					free(err);
				}
				fprintf(stdout, "Invalid line, please re-enter\n");
			}
		}
		if (good)
		{
			strcpy(cp, storebuf);
			cp += strlen(cp);
			*cp++ = '\n';
		}
		l_free_conf(lm_job, conf);
	}
	return 0;

}
static
void
strip_trailing_blanks(inbuf)
char *inbuf;
{
  int len = strlen(inbuf);

	for (len--; len >=0 && isspace(inbuf[len]); len--)
		inbuf[len] = 0;
}
