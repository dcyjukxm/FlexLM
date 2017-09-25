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
 *	Module:	lmlite.c v1.1.0.0
 *
 *	Description:	This is a sample application to illustrate 
 *			the use of FLEXlm-ultralite
 *
 *	Last changed:  3/9/98
 *	M. Christiano
 *
 */
#include "lmclient.h" 
#include "lm_code.h"


#if 0

#define FEATURE "f1"
#define GOODKEY_ANY "E65A383BD96F"

#define HOSTID2 "12345678"
#define GOODKEY2 "9C9A82E2C3BC"

#define HOSTID3 "12345678a"
#define HOSTID3_ALT "12345678A"
#define GOODKEY3 "28D90A6D0B99"

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

main()
{
  char feature[MAX_FEATURE_LEN+1];
  int err;
  int Approved;

int cmd_string;
#define TRUE 1
#define FALSE 0
#define OK 0


/****************************************************************

 	lc_checkit() takes 5 parameters:

		vendor name (char *)
		feature name (char *)
		license key (char *)
		licensed hostid (char *)
		VENDORCODE struct (LM_CODE *)

	There are 3 possible return values:

	    		    0	Everything OK (license good, correct host)
	    LM_BADFILE     -2	No hostid specified
	    LM_BADCODE     -8	Bad license key
	    LM_BADPARAM	   -42  Missing feature name or LM_CODE struct

****************************************************************/

	(void) strcpy(feature, FEATURE);
/*
 *	Try the checkout with a few errors, then with a good license
 */
	err = lc_checkit(VENDOR_NAME, "", GOODKEY_ANY, "", &code);
	printf("With no feature name, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "", &code);
	printf("With no hostid, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "12345", &code);
	printf("With bad hostid, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, "0", "ANY", &code);
	printf("With bad license key, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "ANY", &code);
	printf("With ANY hostid and license key, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, GOODKEY2, HOSTID2, &code);
	printf("With good license key for 12345678, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, GOODKEY3, HOSTID3, &code);
	printf("With good license key for 12345678a, error is %d\n", err);

	err = lc_checkit(VENDOR_NAME, feature, GOODKEY3, HOSTID3_ALT, &code);
	printf("With good license key for 12345678A, error is %d\n", err);
  Approved=1;

  /* no feature name */
  err = lc_checkit(VENDOR_NAME, "", GOODKEY_ANY, "", &code);
  if (err==LM_BADPARAM)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* no host id */
  (void) strcpy(feature, FEATURE);
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "", &code);
  if (err==LM_BADFILE)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* bad host id */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "12345", &code);
  if (err==LM_BADCODE)
      output_dec_word(Approved, TRUE, cmd_string);
   else

      output_dec_word(err, TRUE, cmd_string);

  /* bad license key */
  err = lc_checkit(VENDOR_NAME, feature, "0", "ANY", &code);
  if (err==LM_BADCODE)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* any hostid and license key */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "ANY", &code);
  if (err==OK)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* good license key for 12345678 */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY2, HOSTID2, &code);
  if (err==OK)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);


  /* good license key for 12345678a */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY3, HOSTID3, &code);
  if (err==OK)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* good license key for 12345678A */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY3, HOSTID3_ALT, &code);
  if (err==OK)
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);


}
#endif

output_dec_word(err, x, y)
int err, x, y;
{
  unsigned z;

	z = (unsigned) err;

	printf("%u\n", z);
}


#define TRUE 1
#define OK 0
#define MAX_CRYPT_LEN 20
#define VENDOR_NAME "demo"

#define FEATURE "f1"
#define GOODKEY_ANY "E65A383BD96F"

#define HOSTID2 "12345678"
#define GOODKEY2 "9C9A82E2C3BC"

#define HOSTID3 "12345678a"
#define HOSTID3_ALT "12345678A"
#define GOODKEY3 "28D90A6D0B99"

LM_CODE_GLOBAL(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
        VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

main()
{
  int err;
  char feature[MAX_FEATURE_LEN+1];
  char Buffer1[100];
  char Buffer2[100];
  char Buffer3[100];
  char Buffer4[100];
  int Approved;
  int cmd_string;

  Approved=1;
  (void) strcpy(feature, FEATURE);

  /* no feature name */
  err = lc_checkit(VENDOR_NAME, "", GOODKEY_ANY, "", &code);
  if (err==LM_BADPARAM) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* no host id */
  (void) strcpy(feature, FEATURE);
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "", &code);
  if (err==LM_BADFILE) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* bad host id */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "12345", &code);
  if (err==LM_BADCODE) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* bad license key */
  err = lc_checkit(VENDOR_NAME, feature, "0", "ANY", &code);
  if (err==LM_BADCODE) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);


  /* any hostid and license key */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY_ANY, "ANY", &code);
  if (err==OK) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* good license key for 12345678 */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY2, HOSTID2, &code);
  if (err==OK) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* good license key for 12345678a */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY3, HOSTID3, &code);
  if (err==OK) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

  /* good license key for 12345678A */
  err = lc_checkit(VENDOR_NAME, feature, GOODKEY3, HOSTID3_ALT, &code);
  if (err==OK) 
      output_dec_word(Approved, TRUE, cmd_string);
   else
      output_dec_word(err, TRUE, cmd_string);

return(916);
}
