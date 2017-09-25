/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

/*****************************************************************************/
/*	
 *	Module:	%M% v%R%.%L%.%B%.%S%
 *
 *	Description:	This is a sample application to illustrate 
 *			the use of FLEXlm-ultralite
 *
 *	Last changed:  %G%
 *	M. Christiano
 *
 */
#include "lmclient.h" 
#include "lm_code.h"
#include <stdio.h>

#define FEATURE "570B"

#define HOSTID "1234567890"
#define GOODKEY "B0749E494111"

#define BADRESULT  { badresults++; printf("***ERROR***\n\n"); }

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

main()
{
  int err;
  int i, j[12];
  int badresults = 0;
  char key[20];
  int pass = 0;

	printf("start at ");
	fflush(stdout);
	system("date");
	err = lc_checkit(VENDOR_NAME, FEATURE, GOODKEY, HOSTID, &code);
	if (err) 
	{
		BADRESULT
		printf("With license key (%s), error is %d\n", GOODKEY, err);
	}
	else
		printf("Checkout of good license (%s/%s/%s) OK\n",
				FEATURE, HOSTID, GOODKEY);

	for (i=0; i<12; i++) j[i] = 0;
	strcpy(key, "000000000000");

	for (j[0] = 0; j[0] < 16; j[0]++)
	{
	 setkey(key, 0, j[0]);
	 for (j[1] = 0; j[1] < 16; j[1]++)
	 {
	  setkey(key, 1, j[1]);
	  for (j[2] = 0; j[2] < 16; j[2]++)
	  {
	   setkey(key, 2, j[2]);
	   for (j[3] = 0; j[3] < 16; j[3]++)
	   {
	    setkey(key, 3, j[3]);
	    for (j[4] = 0; j[4] < 16; j[4]++)
	    {
	     setkey(key, 4, j[4]);
	     for (j[5] = 0; j[5] < 16; j[5]++)
	     {
	      setkey(key, 5, j[5]);
	      for (j[6] = 0; j[6] < 16; j[6]++)
	      {
	       setkey(key, 6, j[6]);
	       for (j[7] = 0; j[7] < 16; j[7]++)
	       {
	        setkey(key, 7, j[7]);
	        for (j[8] = 0; j[8] < 16; j[8]++)
	        {
	         setkey(key, 8, j[8]);
	         for (j[9] = 0; j[9] < 16; j[9]++)
	         {
	          setkey(key, 9, j[9]);
	          for (j[10] = 0; j[10] < 16; j[10]++)
	          {
	           setkey(key, 10, j[10]);
	           for (j[11] = 0; j[11] < 16; j[11]++)
	           {
	           	setkey(key, 11, j[11]);
		   	err = lc_checkit(VENDOR_NAME, FEATURE, key, HOSTID, 
								&code);
		 	if (err != LM_BADCODE) 
			{
				BADRESULT
				printf("With license key (%s), error is %d\n", 
								key, err);
			}
			pass++;
			if ((pass % 50000) == 0) 
			{
				printf("%10d: testing %s\n", pass, key);
				fflush(stdout);
			}
		   }
		  }
		 }
		}
	       }
	      }
	     }
	    }
	   }
	  }
	 }
	}
	printf("done at ");
	fflush(stdout);
	system("date");
}

setkey(key, pos, val)
char *key;
int pos, val;
{
	if (val > 9) key[pos] = 'a' + val - 10;
	else key[pos] = '0' + val;
}
