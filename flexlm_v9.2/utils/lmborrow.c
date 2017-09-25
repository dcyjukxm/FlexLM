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
 *	Module: $Id: lmborrow.c,v 1.41 2003/04/18 23:48:11 sluu Exp $
 *
 *	Function:	lmborrow
 *
 *	D. Birns
 *	March, 2001
 *
 */
#include "lmutil.h"
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "l_borrow.h"

static int borrow_stat(int id);
static int borrow_clear(void);
extern LM_HANDLE *lm_job;
static int borrow_usage(void);
char szDisplayName[MAX_DISPLAY_NAME + 1] = {'\0'};

#define REMOVE_FEATURE	2
#define LM_BORROW_KEY	"LM_BORROW"
#ifdef PC
#define FLEXLM_BORROW_REG_KEY	"Software\\FLEXlm License Manager\\Borrow"
#else /* !PC */
#define SEPARATOR	"="
#endif

char *
l_parse_info_borrow(
	LM_HANDLE *		job,
	char *			buf, /* input */
		/* following features are return values */
	char *			feature, /* char buffer*/
	char *			vendor,
	long *			start,
	long *			exp,
	char *			code,
	int *			b_id);	/* char buffer */

static
int
sDeleteBorrowInfo(LM_BORROW_STAT * pStat)
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


	if(l_get_registry(lm_job, "infoborrow", &stat, &iLen, 1))
		return 0;

	buffer = l_malloc(lm_job, iLen  + 1);
	if(buffer == NULL)
		return 0;
	memset(buffer, 0, iLen + 1);
	/*
	 *	Write contents of data into buffer, removing entry specified by pStat,
	 *	then rewrite it to the registry.
	 */
	cp = stat;

	while (cp && *cp && cp[1])
	{
		pCurr = cp;
		cp = l_parse_info_borrow(lm_job, cp, feature,
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
	l_set_registry(lm_job, "infoborrow", buffer, strlen(buffer) + 1, 1);
	l_free(buffer);

	return 0;
}

char *l_strlwr(char *string)
{
      char *s;

      if (string)
      {
            for (s = string; *s; ++s)
                  *s = tolower(*s);
      }
      return string;
}

#ifdef PC
static
void
sPrintError()
{
	void *	pMsgBuf;
	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(char *) &pMsgBuf,
			0,
			NULL);
	fprintf(ofp, "%s", (char *)pMsgBuf);
	LocalFree(pMsgBuf);

}
#endif /* PC */

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
		fprintf(ofp, "Error: unable to delete local borrow info.\n");
		LM_SET_ERRNO(lm_job, LM_BORROW_DELETE_ERR, 591, 0);
		return lm_job->lm_errno;
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
		LM_SET_ERRNO(job, LM_CANTMALLOC, 593, 0);
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
		fprintf(ofp, "Error: unable to delete local borrow info.\n");
		LM_SET_ERRNO(lm_job, LM_BORROW_DELETE_ERR, 591, 0);
		return lm_job->lm_errno;
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
			sDeleteBorrowInfo(pStat);
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
		LM_SET_ERRNO(job, LM_CANTMALLOC, 593, 0);
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
				sDeleteBorrowInfo(pStat);
				break;
			}
		}
	}
	if(pszBorrowName)
		free(pszBorrowName);
#endif /* PC */
	return 0;
}

static
int
sBorrowReturn(
	int		newargc,
	char **	newargv)
{
	int					i;
	char *				szFeature  = NULL;
	time_t				currTime = 0;
	char				szVendorNameLic[MAX_PATH] = {'\0'};
	char				szVendor[MAX_PATH] = {'\0'};
	char *				pszLicFile = NULL;
	LM_BORROW_STAT *	pBorrow = 0;
	LM_BORROW_STAT *	pTemp = NULL;


	if(newargc >= 5 && strcmp(newargv[2], "-d") == 0)
	{
		strcpy(szDisplayName, newargv[3]);
		szFeature = newargv[4];
	}
	else
	{
		if(strcmp(newargv[2], "-d") == 0)
		{
			return borrow_usage();
		}
		else
		{
			szFeature = newargv[2];
		}
	}

	/*
	 *	Get borrow info (infoborrow)
	 */
	if (l_borrow_stat(lm_job, &pBorrow, 1))
	{
		fprintf(ofp, "Error: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	/*
	 *	Determine if we actually have this feature borrowed
	 */
	for (pTemp = pBorrow; pTemp; pTemp = pTemp->next)
	{
		if(strcmp(szFeature, pTemp->feature) == 0)
		{
			break;
		}
	}

	if(pTemp == NULL)
	{
		/*
		 *	Couldn't find feature in question, error out.
		 */
		fprintf(ofp, "Error: %s not currently borrowed.\n", szFeature);
		LM_SET_ERRNO(lm_job, LM_NOFEATURE, 590, 0);
		return lm_job->lm_errno;
	}

	sprintf(szVendor, "%s_LICENSE_FILE", pTemp->vendor);
	l_getEnvUTF8(lm_job, szVendor, szVendorNameLic,
		sizeof(szVendorNameLic));
	lc_get_attr(lm_job, LM_A_LICENSE_FILE_PTR, (short *)&pszLicFile);
	if(pszLicFile == NULL)
	{
		if(szVendorNameLic[0] != '\0')
		{
			lc_set_attr(lm_job, LM_A_LICENSE_FILE_PTR, (LM_A_VAL_TYPE)szVendorNameLic);
		}
		else
		{
			lc_set_attr(lm_job, LM_A_LICENSE_FILE_PTR, (LM_A_VAL_TYPE)".");
		}
	}

	/*
	 *	Determine if feature is still valid
	 */
	time(&currTime);
	if(currTime > pTemp->end)
	{
		fprintf(ofp, "Warning: %s has already expired.\n", szFeature);
		sDeleteBorrow(lm_job, pTemp);
	}
	else
	{
		/*
		 *	Modify it before we attempt a remove
		 */
		sModifyBorrow(lm_job, pTemp);
		i = l_return_early(lm_job,
							szFeature,
							pBorrow->vendor,
							lc_username(lm_job, 1),
							lc_hostname(lm_job, 1),
							szDisplayName[0] == '\0' ? lc_display(lm_job, 1) : szDisplayName);
		
		if(i)
		{
			/*
			 *	Unmodify it since we couldn't return it
			 */
			sModifyBorrow(lm_job, pTemp);
			if(lm_job->lm_errno == BADPARAM)
			{
				LM_SET_ERRNO(lm_job, LM_BORROW_MATCH_ERR, 595, 0);
			}
			fprintf(ofp, "lmborrow: %s\n",
				lmtext(lc_errstring(lm_job)));
			return lm_job->lm_errno;
		}
		else
		{
			/*
			 *	If this fails, at least we modified it so user can't use it again.
			 */
			sDeleteBorrow(lm_job, pTemp);
		}
	}
	return 0;
}



int
lmborrow_main(int argc, char * argv[])
{
	int id = 0;

	if (argc == 1)
	{
		return borrow_usage();
	}
	if (argv[1][0] != '-')
	{
		if (argc  < 3) return borrow_usage();
			return borrow_set(argv[1], argv[2],
				argc == 4 ? argv[3]: 0);
	}
	else if (!strcmp(argv[1], "-status"))
	{
		if (argc == 3)
		{
	/*		if ( !strcmp(argv[2], "-id"))   This is written to the registry under HKEY_CURRENT_USER
                                            For example:  borrow-d98991fa-borrow2 (Registry name)
             id = 1;                        The b_id is 0xd989 = 55689  */

				
			return borrow_usage();
		}
		return (borrow_stat(id));
	}
	else if (!strcmp(argv[1], "-clear"))
	{
		return borrow_clear();
	}
	else if (!strcmp(argv[1], "-return"))
	{
		/*
		 *	Check to make sure enough paramters
		 */
		if(argc < 3)
		{
			return borrow_usage();
		}
		else
		{
			/*
			 *	To get around an issue where for some unknown reason, we process
			 *	port@host as a license file.  Turn this back to the default which
			 *	results in port@host processing as expected, at least from the borrow
			 *	perspective.
			 */
			(void)lc_set_attr(lm_job, LM_A_PORT_HOST_PLUS, (LM_A_VAL_TYPE) 1);
			return sBorrowReturn(argc, argv);
		}
	}
	else
	{
		return borrow_usage();
	}
}

static
int
borrow_stat(int id)
{
  char *stat;
  char *cp, *cp2;
  LM_BORROW_STAT *s = 0;
  int printed = 0;
  struct tm *tm;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  char exp[100];
  char featid[200];
#define FMT "%-10.10s %-35.35s %s\n"

	if (l_borrow_stat(lm_job, &s, 1))
	{
		fprintf(ofp, "Error: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	for (; s; s = s->next)
	{

		if (!printed)
		{
		  char buf[50];
			if (id)
				strcpy(buf, "Feature (FLEXlm Program id)");
			else
				strcpy(buf, "Feature");
			printed = 1;
			fprintf(ofp, FMT, "Vendor", buf, "Expiration");
			fprintf(ofp, FMT, "______", "________", "__________");
			fprintf(ofp, "\n");
		}
#ifdef THREAD_SAFE_TIME
		localtime_r(&s->end, &tst);
		tm = &tst;
#else /* !THREAD_SAFE_TIME */
		tm = localtime(&s->end);
#endif
		if (id)
			sprintf(featid, "%s (%05d)", s->feature, s->borrow_binary_id);
		else
			sprintf(featid, "%s", s->feature);
		sprintf(exp, "%d-%s-%02d %02d:%02d",
			tm->tm_mday,
			tm->tm_mon == 0 ? "Jan" :
			tm->tm_mon == 1 ? "Feb" :
			tm->tm_mon == 2 ? "Mar" :
			tm->tm_mon == 3 ? "Apr" :
			tm->tm_mon == 4 ? "May" :
			tm->tm_mon == 5 ? "Jun" :
			tm->tm_mon == 6 ? "Jul" :
			tm->tm_mon == 7 ? "Aug" :
			tm->tm_mon == 8 ? "Sep" :
			tm->tm_mon == 9 ? "Oct" :
			tm->tm_mon == 10 ? "Nov" : "Dec",
			(tm->tm_year + 1900) % 2000,
			tm->tm_hour, tm->tm_min);
		fprintf(ofp, FMT, s->vendor, featid, exp);
	}
	return 0;

}

static
int
borrow_clear(void)
{
	/* clear from HKCU */
	if (l_set_registry(lm_job, "LM_BORROW", 0, 0, 1))
	{
		fprintf(ofp, "Borrow: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	else
		fprintf(ofp, "Clearing LM_BORROW\n");

	/* clear from HKLM */
	if (l_set_registry(lm_job, "LM_BORROW", 0, 0, 0))
	{
		fprintf(ofp, "Borrow: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}

	return 0;
}

int
borrow_set(char *vendor, char *expire, char *t)
{
	char buf[MAX_CONFIG_LINE + 1];
	char *stopstr;
	char *temp;
	char *token;
	long day;
	int d, m, y;
#ifdef THREAD_SAFE_TIME
	struct tm tst;
#endif
	long x;
	int h = 0;
	char tbuf[50];

	/* check expire input */
	temp = (char*)malloc(128);
	strcpy(temp, expire);
	token = strtok(temp, "-");

	/* validate day range */
	day = strtol(token, &stopstr, 10);
	if ((day < 1) || (day > 31))
	{
		fprintf(ofp,"Error: invalid expire day value '%s'.\n", token);
		free(temp);
		return (-1);
	}
	token = strtok(NULL, "-");
	if (token == NULL)
	{
                fprintf(ofp, "Error: invalid expire month value '%s'.\n", token);
                free(temp);
                return (-1);
	}
	
	/* validate month range */
        if (strncmp(l_strlwr(token), "jan", 3) != 0)
          if (strncmp(l_strlwr(token), "feb", 3) != 0)
            if (strncmp(l_strlwr(token), "mar", 3)!= 0)
              if (strncmp(l_strlwr(token), "apr", 3)!= 0)
                if (strncmp(l_strlwr(token), "may", 3)!= 0)
                  if (strncmp(l_strlwr(token), "jun", 3)!= 0)
                    if (strncmp(l_strlwr(token), "jul", 3)!= 0)
                      if (strncmp(l_strlwr(token), "aug", 3)!= 0)
                        if (strncmp(l_strlwr(token), "sep", 3)!= 0)
                          if (strncmp(l_strlwr(token), "oct", 3)!= 0)
                            if (strncmp(l_strlwr(token), "nov", 3)!= 0)
                              if (strncmp(l_strlwr(token), "dec", 3)!= 0)
                              {
                          	fprintf(ofp, "Error: invalid expire month value '%s'.\n", token);
                          	free(temp);
                          	return (-1);
                              }
    free(temp);	

#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &m, &y, &x, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &m, &y, &x);
#endif
	if (t)
		sprintf(tbuf, ":%s", t);
	else
		*tbuf = 0;

	sprintf(buf, "%d-%s-%d:%s:%s%s", d,
		m == 0 ? "jan"  :
		m == 1 ? "feb"  :
		m == 2 ? "mar"  :
		m == 3 ? "apr"  :
		m == 4 ? "may"  :
		m == 5 ? "jun"  :
		m == 6 ? "jul"  :
		m == 7 ? "aug"  :
		m == 8 ? "sep"  :
		m == 9 ? "oct"  :
		m == 10 ? "nov"  : "dec", 1900+y, vendor, expire, tbuf);
	if (l_set_registry(lm_job, "LM_BORROW", buf, 0, 1))
	{
		fprintf(ofp, "Borrow: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	else
	{
		fprintf(ofp, "Setting LM_BORROW=%s\n", buf);
	}
	return 0;
}

int
borrow_usage(void)
{
	fprintf(ofp, "\n\
Usage:  lmborrow {all|vendorname} dd-mmm-yyyy [hh:mm]     (To borrow)\n\
        lmborrow -status           (Report features borrowed to this node)\n\
        lmborrow -clear            (Changed your mind -- do not borrow)\n\
	    lmborrow -return [-c licfile] [-d display_name] feature\n\t\t\t\t\t(Return feature early)\n\
	    lmborrow -help             (Display usage information)\n\n");
	return 1;
}
