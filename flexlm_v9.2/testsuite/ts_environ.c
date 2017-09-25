/******************************************************************************

	    COPYRIGHT (c) 1989 by Macrovision Corporation.
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
 *	Module: $Id: ts_environ.c,v 1.12 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Function: setenv(name, val), getenv(name), printenv()
 *
 *	Description: Performs environment functions.
 *
 *	Parameters:	(char *) name - The environment variable name.
 *			(char *) val - The value for name.
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	8/21/89
 *
 *	Last changed:  10/26/98
 *
 */

#include <stdio.h>
#include "lmachdep.h"
#if !defined( convex ) && !defined(LINUX) && !defined(cray) && !defined(BSDI) && !defined(FREEBSD) && !defined (MAC10)  
#include "lmclient.h"
#ifdef RS64
#include "testsuite.h"
#endif
#ifndef OS2
extern char **environ;
#endif /* OS2 */
static int environ_set = 0;
#ifdef VMS
extern char *malloc();
extern void perror(), exit ();
extern int sys$trnlnm();
#endif

setenv(var, equiv)
char *var;
char *equiv;
{
  static char *purify_kludge[1000];
  static int purify_int = 0;
#if !defined(OSF) && !defined(SGI6)
  char **p = 0; 
  char *q = 0;
  int i = strlen(var);
  int j;
#ifdef VMS
	char *_p = var;
	while (*_p) { if (islower(*_p)) *_p = toupper(*_p); _p++; }
#endif
#ifdef __DECC
	/* DEC C's environ is useless - not NULL-terminated */
	environ[0] = (char *) NULL;
#endif /* __DECC */
/*
 *	Create the new environment string
 */
	environ_set = 1;
	q = (char *)malloc(strlen(equiv) + i + 2);
	if (q == (char *) NULL) 
		perror("string malloc (setenv)"), exit(3);
	(void) sprintf(q, "%s=%s", var, equiv);
/*
 *	See if it already exists
 */
	for (j = 0, p = environ; *p; p++, j++)
		if (!strncmp(var, *p, i)) break;
	if (*p)
	{
/*
 *		Replace string in place
 */
		*p = q;		/* Replace the pointer */
	}
	else
	{
/*
 *		Must copy the entire environment, and add this one.
 */
	  char **ee;
	  char **e;
          e =
                (char **) malloc((unsigned) ((j+2) * sizeof(char *)));
          purify_kludge[purify_int++] = (char *) e;

		ee = e;
		if (e == (char **) NULL)
			perror("string malloc (setenv)"), exit(4);
			
		for (p = environ; *p; p++)
			*e++ = *p;
		*e++ = q;
		*e = (char *) NULL;
		environ = ee;
	}
#else
  static char c[10000], *cp;

	/*if (c)	free(c);
	c = (char *)malloc(strlen(var) + strlen(equiv) + 2);
	if (!c) return 0;*/
	sprintf(c, "%s=%s", var, equiv);
	cp = (char *)malloc(strlen(c) + 1);
	strcpy(cp, c);
	return(putenv(cp));
#endif /* NO_PUTENV */
	
}

unsetenv(var)
char *var;
{
  char **p;
  int i = strlen(var);

#ifdef VMS
	char *_p = var;
	while (*_p) { if (islower(*_p)) *_p = toupper(*_p); _p++; }
#endif
/*
 *	See if it exists
 */
	for (p = environ; *p; p++)
		if (!strncmp(var, *p, i)) break;
	if (*p)
	{
/*
 *		Remove string by floating other string pointers up.
 */
		while (*p)
		{
			*p = *(p+1);		/* Replace the pointer */
			p++;
		}
	}
}

printenv()
{
  char **p;

	for (p = environ; *p; p++)
		(void) printf("%s\n", *p);
}
/*
 *	Substitute for getenv() for VMS, so these routines work.

		THIS GETENV() translates logical names, just like the
		VMS getenv().  The reason for this is that the C 
		run-time system calls getenv() before calling main(), 
		in order to set up the environ array.  Thus this 
		getenv must detect this and get the logical name, if any.

 */

#if 1
#ifdef VMS
#include <descrip.h>
#include <lnmdef.h>

extern char *strrchr();

static char equiv[256];

char *
getenv (var)
char *var;
{
  int i = strlen(var);
  int gotit = 0;
  char **p;
  char *e = (char *) NULL;

	if (environ && environ_set)
	{
		for (p = environ; *p; p++)
			if (!strncmp(*p, var, i)) break;
		if (*p)
		{
			e = strrchr(*p, '=');
			if (e) e++;
			gotit = 1;
		}
	}
	if (!gotit)
	{
/*
 *		Try to translate it as a logical name
 */
	  struct dsc$descriptor_s logical;
	  $DESCRIPTOR(tabnam, "lnm$process_table");
	  $DESCRIPTOR(tabnam2, "lnm$system_table");
	  struct itemlist { short len, code; char *bufadr; int rlen; } item[2];
	  int attr = LNM$M_CASE_BLIND;
	  short status;
	  short rlen;

		logical.dsc$w_length = strlen(var);
		logical.dsc$b_dtype = DSC$K_DTYPE_T;
		logical.dsc$b_class = DSC$K_CLASS_S;
		logical.dsc$a_pointer = var;
		item[0].len = sizeof(equiv) - 1;
		item[0].code = LNM$_STRING;
		item[0].bufadr = equiv;
		item[0].rlen = (int) &rlen;
		item[1].len = item[1].code = 0;

		status = sys$trnlnm(&attr, &tabnam, &logical, 0, &item[0].len);
		if (status == 0x1bc)
		{
			status = sys$trnlnm(&attr, &tabnam2, &logical, 0, 
						&item[0].len);
		}
		if (status & 0x1)
		{
			equiv[rlen] = '\0';
			e = equiv;
		}
	}
	return(e);
}
#endif
#endif

#if 0
/*
 *	Code to setenv/unsetenv using VMS logical names.  Couldn't make it work
 */
#include <descrip.h>

#if 0
  struct dsc$descriptor_s logical, value;
  short status;
  char *p = var;
	logical.dsc$w_length = sizeof(var) - 1;
	value.dsc$w_length = sizeof(equiv) - 1;
	value.dsc$b_dtype = logical.dsc$b_dtype = DSC$K_DTYPE_T;
	value.dsc$b_class = logical.dsc$b_class = DSC$K_CLASS_S;
	logical.dsc$a_pointer = var;
	value.dsc$a_pointer = equiv;

	while (*p) { if (islower(*p)) *p = toupper(*p); p++; }

	
	status = lib$set_logical(&logical, &value, 0, 0, 0);
	if (!(status & 0x1))
	{
		printf("Error %x setting environment variable %s\n", 
							status, var);
	}
#endif
	{ char cmd[100];
		sprintf(cmd, "ass %s %s", equiv, var);
		system(cmd);
		printf("After setting %s to %s, we get:\n", var, equiv);
		sprintf(cmd, "sho log %s", var);
		system(cmd);
	}
}

unsetenv(var)
{

#if 0
  struct dsc$descriptor_s logical;
  short status;
  char *p = var;

	while (*p) { if (islower(*p)) *p = toupper(*p); p++; }

	logical.dsc$w_length = sizeof(var) - 1;
	logical.dsc$b_dtype = DSC$K_DTYPE_T;
	logical.dsc$b_class = DSC$K_CLASS_S;
	logical.dsc$a_pointer = var;

	status = lib$delete_logical(&logical, 0);
	if (!(status & 0x1))
	{
		printf("Error %x deleting environment variable %s\n", 
							status, var);
	}
#endif
	{ char cmd[100];
		sprintf(cmd, "deass %s", var);
		system(cmd);
		printf("After deleting %s, we get:\n", var);
		sprintf(cmd, "sho log %s", var);
		system(cmd);
	}
}
#endif
#endif /* convex */

