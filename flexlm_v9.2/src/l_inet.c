/******************************************************************************

	    COPYRIGHT (c) 1992, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_inet.c,v 1.5 2003/04/18 23:48:03 sluu Exp $
 *	Last changed:  10/23/96
 *
 *	Function:	l_inet_to_addr(), l_addr_to_inet(), l_inet_cmp()
 *
 *	Description: 	Internet address to string conversion routines
 *
 *	Parameters:	(short) addr[] - Internet address
 *			(char) type    - Internet address type
 *			(char *) string - String representation
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	5/21/92
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>
#ifdef WINNT 
#include <stdlib.h>
#endif /* WINNT */
#ifdef ANSI
#include <string.h>
#else
extern char *strchr();
#endif

/*
 *	Internet address as string to internal representation
 */
void API_ENTRY
l_inet_to_addr(
	char *	string,
	char *	type,
	short	addr[])
{
  int parts = 0;
  int i;
  long data[4];
  char *c = string, *s = string;




	while ((c = strchr(s, '.')) != (char *) NULL)
	{
		*c = '\0';
		if (*s == '*')
			data[parts] = -1;
		else
			data[parts] = atoi(s);
		parts++;
		s = c+1;
		*c = '.';	/* Put it back (P464) */
	}
	if (*s == '*')
		data[parts] = -1;
	else
		data[parts] = atoi(s);
	parts++;
	*type = (char) parts;
	for (i = 0; i < parts-1; i++)
	{
		addr[i] = (short) data[i];
	}
/*
 *	Now, the last part
 */
	switch (parts)
	{
	  case 2:
		if (data[1] == -1)
		{
			addr[1] = addr[2] = addr[3] = (short) -1;
		}
		else
		{
			addr[1] = (short) ((data[1] & 0xff0000) >> 16);
			addr[2] = (short) ((data[1] & 0xff00) >> 8);
			addr[3] = (short) (data[1] & 0xff);
		}
		break;
	  case 3:
		if (data[2] == -1)
		{
			addr[2] = addr[3] = (short) -1;
		}
		else
		{
			addr[2] = (short) ((data[2] & 0xff00) >> 8);
			addr[3] = (short) (data[2] & 0xff);
		}
		break;
	  case 4:
		addr[3] = (short) data[3];
		break;
	  default:
		addr[0] = addr[1] = addr[2] = addr[3] = (short) 0;
		break;
	}
}

/*
 *	Internal internet address to printable string
 */
void API_ENTRY
l_addr_to_inet(
	short	addr[],
	int		type,		/* promoted */
	char *	string)
{
  unsigned char a[4][10];
  int i;

	if (type == 4)
	{
		for (i=0; i<4; i++)
		{
			if ((int) addr[i] == -1) 
			{
				a[i][0] = '*';
				a[i][1] = '\0';
			}
			else
				(void) sprintf((char *)&a[i][0], "%d", 
								(int) addr[i]);
		}
		(void) sprintf(string, "%s.%s.%s.%s", &a[0][0], &a[1][0], 
						&a[2][0], &a[3][0]);
	}
	else if (type == 3)
	{
		for (i=0; i<3; i++)
		{
			if ((int) addr[i] == -1) 
			{
				a[i][0] = '*';
				a[i][1] = '\0';
			}
			else
				(void) sprintf((char *)&a[i][0], "%d", 
							(int) addr[i]);
		}
		if (a[2][0] != '*')
		{
			(void) sprintf((char *)&a[2][0], "%d", (int)
					((addr[2] << 8) + addr[3]));
		}
		(void) sprintf(string, "%s.%s.%s", &a[0][0], &a[1][0], 
							     &a[2][0]);
	}
	else
	{
		for (i=0; i<2; i++)
		{
			if ((int) addr[i] == -1) 
			{
				a[i][0] = '*';
				a[i][1] = '\0';
			}
			else
				(void) sprintf((char *)&a[i][0], "%d", 
							(int) addr[i]);
		}
		if (a[1][0] != '*')
		{
			(void) sprintf((char *)&a[1][0], "%d", 
						(int) ((addr[1] << 16) + 
						 (addr[2] << 8) + addr[3]));
		}
		(void) sprintf(string, "%s.%s", &a[0][0], &a[1][0]);
	}
}

/*
 *	Compare two internet addresses, if addr1 matches addr2, return 0
 */

API_ENTRY
l_inet_cmp(
	short	addr1[],
	short	addr2[])
{
  int i;

	for (i = 0; i <= 3; i++)
	{
		if ((addr2[i] == (short) -1) || (addr1[i] == (short) -1))
			;
		else if (addr2[i] > addr1[i])
			return 1;
		else if (addr2[i] < addr1[i])
			return -1;
	}
	return 0;
}

int
l_is_inet(char *s)
{
  char *cp;
  int dots = 0;
  int digits = 0;
 
	for (cp = s; *cp; cp++)
	{
		if (*cp == '.')
			dots++;
		else if (isdigit(*cp ) || (*cp == '*'))
			digits++;
	}
	if ((dots == 3) && (digits >= 4) && (digits <= 12))
		return 1;
	return 0;
}
