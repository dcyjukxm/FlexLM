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
 *	Module: $Id: lm_ver.c,v 1.8 2003/04/18 23:48:11 sluu Exp $
 *
 *	Function:
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  03/22/96
 *
 */
#include "lmutil.h"
#include "flex_file.h"
#define REMAINING (sizeof(copyright_buf) - (cp - copyright_buf) - 1)

int 
lmver_main( path)
char *path;
{
  static char copyright_buf[1024]; /* 1024 is big enough */
  char *cp;
  int c;
  int rc;
  int max, i;
  FILE *fp;
  char *exepath;

	if(!(fp = l_flexFopen(lm_job, path, "rb")))
	{
		exepath = l_malloc(lm_job, strlen(path) + 10);
		sprintf(exepath, "%s.exe", path);
		if(!(fp = l_flexFopen(lm_job, exepath, "rb")))
		{
			(void) fprintf(ofp,  "Failed to open file '%s' for read.\n", path );
			return 1;
		}
	}
	
	while (get_first_char(fp) != EOF)
	{
		cp = copyright_buf;
		memset(copyright_buf, 0, sizeof(copyright_buf));
/*
 *	First fill buffer
 */
		*cp++ = 'F';

		rc = get_string("LEXlm", fp, &cp, 7);
		if (rc == EOF) break;
		else if (rc == 0) continue;

		rc = get_string("Copyright", fp, &cp, REMAINING);
		if (rc == EOF) break;
		else if (rc == 0) continue;

		rc = get_string("19", fp, &cp, REMAINING);
		if (rc == EOF) break;
		else if (rc== 0) continue;

/* 
 *	Found it, now fill to end of line
 */
		max = REMAINING;
		i = 0;
		while (((c = getc(fp)) != EOF) && (c != '\n') && 
			c && (i < max))
		{
			*cp++ = c;
			i++;
		}
/*
 *	Print result
 */
		*cp = 0;
		{
		  char *t;
			if ((t = strchr(copyright_buf, '%')) &&
				t[1] == 's')
			continue;
		}

		fprintf(ofp, "%s\n", copyright_buf);
	} 
	return 0;
}

int 
get_first_char(file)
FILE *file;
{
  int c;
	
	while ((c = getc(file)) != EOF)
	{
		if (c == 'F')
			break;

	}
	return c;
}
int
get_string(str, file, cpp, max)
char *str;
FILE *file;
char **cpp;
int max;
{
  int c;
  int i, j;
  int len;
  char *cp = *cpp;

/*
 *	Skip to start of string
 */
	c = getc(file);
	*cp++ = c;
	for (i=0;
		(i < max) && (c != *str) && (c != '\n') && (c != EOF) && 
			(c <= 127) && c;
		i++)
	{
		c = getc(file);
		*cp++ = c;
	}
	if (c == EOF) return c;
	else if (c != *str) return 0; 

	len = strlen(str); 
	for ( j=1; j < len && i < max; i++, j++)
	{
		c = getc(file);
		if (c == EOF) break;
		else if (c != str[j]) return 0;
		*cp++ = c;
	}
	*cpp = cp;
	if (c == EOF) return c;
	else if (j == len) return 1;
	else return 0;
}
