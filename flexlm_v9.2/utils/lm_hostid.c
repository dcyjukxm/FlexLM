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
 *	Module: $Id: lm_hostid.c,v 1.15.6.2 2003/07/01 18:22:27 sluu Exp $
 *
 *	Function:	lmhostid
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  11/20/97
 *
 */
#include "lmutil.h"
#include "flex_utils.h"

#ifdef DEFAULT_HOSTID_ETHER
#define DEFAULT HOSTID_ETHER
#else
#ifdef DEFAULT_HOSTID_STRING
#define DEFAULT HOSTID_STRING
#else
#define DEFAULT HOSTID_LONG
#endif /*HOSTID_STRING */
#endif /* HOSTID_ETHER */
HOSTTYPE *h;

lmhostid_main(argc, argv)
int argc;
char *argv[];
{
  int verbose = 0;
  short idtype = DEFAULT;
  char *p,ss[200];
  HOSTID *id = 0;
  int list = 0;
  int badargs = 0;
  char *	pszHostid = NULL;
  char *	pszTemp = NULL;
  int		size = 0;
  int		utf8 = 0;

	lm_job->lm_errno = 0;
	while (argc > 1)
	{

		for (p=argv[1]; *p; p++)
			if (isupper(*p)) *p = tolower(*p);

		if (!strcmp(argv[1], "-v"))      /* This is also a global lmutil option and does not need
										    to be added to the command-line */
		{
			(void) fprintf(ofp, "%s v%d.%d%s\n", myname,
					      lm_job->code.flexlm_version,
					      lm_job->code.flexlm_revision,
					      lm_job->code.flexlm_patch);
			return(0);
		}
		else if (!strcmp(argv[1], "-m"))  /* need not be added to the command-line display */
			verbose = 1;
#if 0
		else if (!strcmp(argv[1], "meter"))
			idtype = HOSTID_METER;
		else if (!strcmp(argv[1], "-meter"))
			idtype = HOSTID_METER;
#endif
		else if (!strcmp(argv[1], "idmodule"))   /* 9/25/02:  [-idmodule|idmodule] is an obsolete option */
			idtype = HOSTID_ID_MODULE;
		else if (!strcmp(argv[1], "-idmodule"))
			idtype = HOSTID_ID_MODULE;
		else if (!strcmp(argv[1], "ether"))
			idtype = HOSTID_ETHER;
		else if (!strcmp(argv[1], "-ether"))
			idtype = HOSTID_ETHER;

/*		else if (!strcmp(argv[1], "-domain"))    Not currently used, but reserved for future use
			idtype = HOSTID_DOMAIN;
*/
		else if (!strcmp(argv[1], "long"))    /* 32-bit hostid */
			idtype = HOSTID_LONG;
		else if (!strcmp(argv[1], "-long"))
			idtype = HOSTID_LONG;
		else if (!strcmp(argv[1], "-internet"))
			idtype = HOSTID_INTERNET;
		else if (!strcmp(argv[1], "internet"))
			idtype = HOSTID_INTERNET;

                else if (!strcmp(argv[1], "-flexid"))
                        idtype = HOSTID_FLEXID1_KEY;
		/* 10/16/02 --- For backward compatibility, not included in documentation. */
                else if (!strcmp(argv[1], "-flexid2"))
                        idtype = HOSTID_FLEXID2_KEY;
                else if (!strcmp(argv[1], "-flexid3"))
                        idtype = HOSTID_FLEXID3_KEY;

				else if (!strcmp(argv[1], "-user"))     /* username */
                        idtype = HOSTID_USER;
				else if (!strcmp(argv[1], "-display"))  /* display */
                        idtype = HOSTID_DISPLAY;
                else if (!strcmp(argv[1], "-string"))   /* string ID */
                        idtype = HOSTID_STRING;
                else if (!strcmp(argv[1], "-vsn"))
                        idtype = HOSTID_DISK_SERIAL_NUM;
                else if (!strcmp(argv[1], "vsn"))
                        idtype = HOSTID_DISK_SERIAL_NUM;
                else if (!strcmp(argv[1], "-hostname"))
                        idtype = HOSTID_HOSTNAME;
                else if (!strcmp(argv[1], "hostname"))
                        idtype = HOSTID_HOSTNAME;
#ifdef WINNT
/* 9/25/02:  [cpu96|-cpu96] [cpu64|-cpu64] [cpu32|-cpu32] [cpu|-cpu] are obsolete options. */
                else if (!strcmp(argv[1], "-cpu96"))
                        idtype = HOSTID_INTEL96;
                else if (!strcmp(argv[1], "cpu96"))
                        idtype = HOSTID_INTEL96;
                else if (!strcmp(argv[1], "-cpu64"))
                        idtype = HOSTID_INTEL64;
                else if (!strcmp(argv[1], "cpu64"))
                        idtype = HOSTID_INTEL64;
                else if (!strcmp(argv[1], "-cpu32"))
                        idtype = HOSTID_INTEL32;
                else if (!strcmp(argv[1], "cpu32"))
                        idtype = HOSTID_INTEL32;
                else if (!strcmp(argv[1], "-cpu"))
                        idtype = HOSTID_INTEL32;
                else if (!strcmp(argv[1], "cpu"))
                        idtype = HOSTID_INTEL32;
#if 0
                else if (!strcmp(argv[1], "-processorid"))
                        idtype = HOSTID_CPU;
                else if (!strcmp(argv[1], "processorid"))
                        idtype = HOSTID_CPU;
                else if (!strcmp(argv[1], "-diskid"))
                        idtype = HOSTID_DISK_GEOMETRY;
                else if (!strcmp(argv[1], "diskid"))
                        idtype = HOSTID_DISK_GEOMETRY;
                else if (!strcmp(argv[1], "-biosid"))
                        idtype = HOSTID_BIOS;
                else if (!strcmp(argv[1], "biosid"))
                        idtype = HOSTID_BIOS;
                else if (!strcmp(argv[1], "-compid"))
                        idtype = HOSTID_COMPOSITE;
                else if (!strcmp(argv[1], "compid"))
                        idtype = HOSTID_COMPOSITE;
#endif	/* 0 */
#endif

				else if (!strcmp(argv[1], "-help"))
					    badargs++;
				else if (!strcmp(argv[1], "-utf8"))
						utf8 = 1;
				else
                {
                        badargs++;
                        fprintf(ofp, "Unknown argument: %s\n",  argv[1]);
                }
		argc--;
		argv++;
	}
	if (badargs)
	{
	        usage();
	        exit(1);
	}
	if (printheader)
	{
		fprintf(ofp,
			  lmtext("The FLEXlm host ID of this machine is \""));
	}
	if (idtype == HOSTID_FLEXID1_KEY || idtype == HOSTID_FLEXID2_KEY || idtype == HOSTID_FLEXID3_KEY)
        {
#ifdef PC
/*
 *        We have to concatenate these 2 types to make them appear as one
 */
          HOSTID *h1, *h2, *h3, *h4, *hp;
          LM_ERR_INFO e;

                memset(&e, 0, sizeof(e));
                l_hostid_copy(lm_job, &h1,
                                lc_getid_type(lm_job, HOSTID_FLEXID1_KEY));
                if (lm_job->lm_errno)
                    l_err_info_cp(lm_job, &e, &lm_job->err_info);

#ifndef _ALPHA_
                l_hostid_copy(lm_job, &h2,
                                lc_getid_type(lm_job, HOSTID_FLEXID2_KEY));
                if (e.maj_errno == LM_NODONGLEDRIVER)
                    l_err_info_cp(lm_job, &lm_job->err_info, &e);
#endif /* _ALPHA_ */
                l_hostid_copy(lm_job, &h3,
                                lc_getid_type(lm_job, HOSTID_FLEXID6_KEY));
                if (e.maj_errno == LM_NODONGLEDRIVER)
                    l_err_info_cp(lm_job, &lm_job->err_info, &e);

				/* aladdin dongle */
                l_hostid_copy(lm_job, &h4,
                                lc_getid_type(lm_job, HOSTID_FLEXID3_KEY));
                if (e.maj_errno == LM_NODONGLEDRIVER)
                    l_err_info_cp(lm_job, &lm_job->err_info, &e);

                /* find last in h1 */
                for (hp = h1; hp && hp->next; hp = hp->next) ;
                if (hp) hp->next = h2; /* append h2 to h1 */
                else h1 = h2;
                id = h1;

                /* find last in h1 */
                for (hp = h1; hp && hp->next; hp = hp->next) ;
                if (hp) hp->next = h3; /* append h3 to h1 */
                else h1 = h3;
                id = h1;

                /* find last in h1 */
                for (hp = h1; hp && hp->next; hp = hp->next) ;
                if (hp) hp->next = h4; /* append h4 to h1 */
                else h1 = h4;
                id = h1;


                if (id && lm_job->lm_errno)
                        LM_SET_ERRNO(lm_job, 0, 0, 0);

#endif /* PC */
        }
        else
			id = l_getid_type(lm_job, idtype);
	pszHostid = l_asc_hostid(lm_job, id);

	if(utf8 == 0)
	{
		/*
		 *	Data should already be in UTF-8 format
		 */
		pszTemp = l_convertStringUTF8ToMB(lm_job, pszHostid, &size);
	}
	fprintf(ofp, "%s", pszTemp == NULL ? pszHostid : pszTemp);

	if(pszTemp)
	{
		l_free(pszTemp);
		pszTemp = NULL;
	}
	if (id && id->next) list = 1;
	if (printheader)
	{
		fprintf(ofp, "\"");
		if (list)
			fprintf(ofp, "\nOnly use ONE from the list of hostids.");
	}
	fprintf(ofp, "\n");
	if (verbose)
	{
		h = lc_hosttype(lm_job, 0);
		if (h) (void) fprintf(ofp, lmtext("System type: %s\n"), h->name);
	}
	if (lm_job->lm_errno) fprintf(ofp, "lmhostid: %s\n",
					lmtext(lc_errstring(lm_job)));
	return(lm_job->lm_errno);
}
