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
 *	Module: $Id: ts_hostid.c,v 1.8 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Function: 	vendor-defined hostid
 *
 *	D. Birns
 *	7/19/95
 *
 *	Last changed:  10/10/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "string.h"
#include "testsuite.h"
#include "l_privat.h"

#define OURTYPE HOSTID_VENDOR+1
#define OURTYPE2 HOSTID_VENDOR+2
#define OURTYPE3 HOSTID_VENDOR+3
#define OURTYPE4 HOSTID_VENDOR+4
#define OURSTRING ( LM_CHAR_PTR ) "EXAMPLE_HOSTID" /* len = 14 */
#define OURSTRING2 ( LM_CHAR_PTR ) "EXAMPLE_HOSTID2" 
#define OURSTRING3  ( LM_CHAR_PTR ) "EXAMPLE_HOSTID3" 
#define OURSTRING4 ( LM_CHAR_PTR ) "FILE_HOSTID" 
/* This example returns only 1 hostid */
/* make it MAX_HOSTID_LEN  = 41 characters */
#define OUR_FIXED_ID ( LM_CHAR_PTR ) "12345678901234567890123456" /* 41 - 15 = 26 chars */
#define OUR_FIXED_ID2 ( LM_CHAR_PTR ) "VENDORID2" 
#define OUR_FIXED_ID3 ( LM_CHAR_PTR ) "ID3" 
#define OUR_FIXED_ID3_1 ( LM_CHAR_PTR ) "ID3_1" 

HOSTID * CB_LOAD_DATA LM_CALLBACK_TYPE x_flexlm_gethostid lm_args((short idtype)); 


/*
 *	Print a vendor-defined hostid
 */

char *
CB_LOAD_DATA
LM_CALLBACK_TYPE
x_flexlm_printid(id)
HOSTID *id;
{
  static char ser[MAX_HOSTID_LEN+1];

	if (id->type == OURTYPE)
	{
		(void) sprintf(ser, "%s=%s", OURSTRING, id->id.vendor);
		return(ser);
	}
	return("");
}

/*
 *	Parse a vendor-defined hostid for FLEXlm
 */
CB_LOAD_DATA
LM_CALLBACK_TYPE
x_flexlm_parseid(id, hostid)
HOSTID *id;
char *hostid;
{
  int nflds;
  char hid[MAX_HOSTID_LEN+1], *idtype, *val;
  int ret = 1;

	strncpy(hid, hostid, MAX_HOSTID_LEN);
	hid[MAX_HOSTID_LEN] = '\0';
	idtype = hid;
	val = strchr(idtype, '=');
	if (val)
	{
		*val = '\0';
		val++;
		if (!strcmp(idtype, OURSTRING)) 
		{
			id->type = OURTYPE;
			strncpy(id->id.vendor, val, MAX_HOSTID_LEN);
			id->id.vendor[MAX_HOSTID_LEN] = '\0';
			ret = 0;
		}
	}
	return(ret);
}

/*
 *	x_flexlm_checkid() - Compare two vendor-defined hostids
 */

LM_CALLBACK_TYPE
x_flexlm_checkid(id1, id2)
HOSTID *id1;
HOSTID *id2;
{
  int type1, type2;
  char *val1, *val2;

	if ((id1->type == OURTYPE) && (id1->type == id2->type) && 
		!strncmp(id1->id.vendor, id2->id.vendor, MAX_HOSTID_LEN))
		return(1);
	else
		return(0);
}

/*
 *	x_flexlm_gethostid() - Get the vendor-defined hostid.
 */

#ifdef ANSI
HOSTID *
CB_LOAD_DATA
LM_CALLBACK_TYPE
x_flexlm_gethostid(short idtype)
#else
HOSTID *
CB_LOAD_DATA
LM_CALLBACK_TYPE
x_flexlm_gethostid(idtype)
short idtype;
#endif
{
  HOSTID id, id2;
  HOSTID *p1 = 0, *p2 = 0;
  FILE *fp;
  char buf[512];

	memset(&id, 0, sizeof(id));
	memset(&id2, 0, sizeof(id));
	switch(idtype)
	{
	case OURTYPE:
		id.type = OURTYPE;
		strncpy(id.id.vendor, OUR_FIXED_ID, MAX_HOSTID_LEN);
		id.id.vendor[MAX_HOSTID_LEN] = 0;
		break;
	case OURTYPE2:
		id.type = OURTYPE2;
		strncpy(id.id.vendor, OUR_FIXED_ID2, MAX_HOSTID_LEN);
		id.id.vendor[MAX_HOSTID_LEN] = 0;
		break;
	case OURTYPE3:
		id.type = OURTYPE3;
		strncpy(id.id.vendor, OUR_FIXED_ID3, MAX_HOSTID_LEN);
		id.id.vendor[MAX_HOSTID_LEN] = 0;
		id.next = &id2;
		id2.type = OURTYPE3;
		strncpy(id2.id.vendor, OUR_FIXED_ID3_1, MAX_HOSTID_LEN);
		id2.id.vendor[MAX_HOSTID_LEN] = 0;
		break;
	case OURTYPE4:
		if (!(fp = fopen("hostid.txt", "r")))
		{
			break; /* failure */
		}
		if (!fgets(buf, 512, fp))
		{
			fclose(fp);
			break; /* failure */
		}
		fclose(fp);
		buf[strlen(buf)-1] = 0; /* remove trailing '\n'*/
					  
		id.type = OURTYPE4;
		strncpy(id.id.vendor, buf, MAX_HOSTID_LEN);
		id.id.vendor[MAX_HOSTID_LEN] = 0;
		break;
	}
	if (id.type)
	{
		if (!(p1 = l_new_hostid()))
			return 0;
		memcpy(p1, &id, sizeof(id));
	}
	if (id2.type)
	{
		if (!(p2 = l_new_hostid()))
			return 0;
		memcpy(p2, &id2, sizeof(id2));
		if (id.type) p1->next = p2;
	}
	return(p1);
}
HOSTID *
LM_CALLBACK_TYPE
x1_flexlm_gethostid(idtype) short idtype; { return x_flexlm_gethostid(idtype); }
HOSTID *
LM_CALLBACK_TYPE
x2_flexlm_gethostid(idtype) short idtype; { return x_flexlm_gethostid(idtype); }

/*
 *	x_flexlm_newid() - Install the vendor-defined hostid as a known 
 *			   FLEXlm hostid type.
 *
 *	Place a call to this function after the call to lm_init() in 
 *	makekey, in your client code, and in your vendor_init1
 *	routine in your daemon.
 */

#define PRINT_ERR 	/* Define this to get error printouts */

typedef HOSTID * (LM_CALLBACK_TYPE *PHOSTID)lm_args((short idtype));
extern LM_HANDLE *lm_job;
void
LM_CALLBACK_TYPE
x_flexlm_newid()
{
  int rc;
  LM_VENDOR_HOSTID h;

	rc = lc_set_attr(lm_job, LM_A_HOSTID_PARSE, 
				(LM_A_VAL_TYPE) x_flexlm_parseid);
	if (rc) lc_perror( lm_job, 
	  "lm_set_attr(lm_job, LM_A_HOSTID_PARSE, x_flexlm_parseid) FAILED");
	rc = lc_set_attr(lm_job, LM_A_VENDOR_GETHOSTID, 
					(LM_A_VAL_TYPE) x_flexlm_gethostid);
	if (rc) lc_perror( lm_job, 
	       "lm_set_attr(LM_A_VENDOR_GETHOSTID, x_flexlm_gethostid) FAILED");
	rc = lc_set_attr(lm_job, LM_A_VENDOR_PRINTHOSTID, 
					(LM_A_VAL_TYPE ) x_flexlm_printid);
	if (rc) 
		lc_perror( lm_job, 
	       "lm_set_attr(LM_A_VENDOR_PRINTHOSTID, x_flexlm_printid) FAILED");
	rc = lc_set_attr(lm_job, LM_A_VENDOR_CHECKID, 
					(LM_A_VAL_TYPE) x_flexlm_checkid);
	if (rc) lc_perror( lm_job, 
		"lm_set_attr(LM_A_VENDOR_CHECKID, x_flexlm_checkid) FAILED");
	memset(&h, 0, sizeof (h));
	h.label = OURSTRING2;
	h.hostid_num = OURTYPE2;
	h.case_sensitive = 0;
	h.get_vendor_id = (PHOSTID) x1_flexlm_gethostid;
	rc = lc_set_attr(lm_job, LM_A_VENDOR_ID_DECLARE, (LM_A_VAL_TYPE) &h);
	if (rc) lc_perror( lm_job, "LM_A_VENDOR_ID_DECLARE FAILED");
	memset(&h, 0, sizeof (h));
	h.label = OURSTRING3;
	h.hostid_num = OURTYPE3;
	h.case_sensitive = 0;
	h.get_vendor_id = (PHOSTID) x2_flexlm_gethostid;
	rc = lc_set_attr(lm_job, LM_A_VENDOR_ID_DECLARE, (LM_A_VAL_TYPE) &h);
	if (rc) lc_perror( lm_job, "LM_A_VENDOR_ID_DECLARE FAILED");
	memset(&h, 0, sizeof (h));
	h.label = OURSTRING4;
	h.hostid_num = OURTYPE4;
	h.case_sensitive = 0;
	h.get_vendor_id = (PHOSTID) x_flexlm_gethostid;
	if (lc_set_attr(lm_job, LM_A_VENDOR_ID_DECLARE, (LM_A_VAL_TYPE) &h))
		lc_perror( lm_job, "LM_A_VENDOR_ID_DECLARE FAILED");

}
