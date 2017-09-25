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

 *****************************************************************************/
/*
 *
 *	Module: $Id: ls_svk.c,v 1.7 2003/01/13 22:22:37 kmaclean Exp $
 *     
 *      Function: 
 *
 *      Description: Generates an encryption value for a feature.
 *
 *      Parameters:     (LM_HANDLE *) job - current job
 *                      (CONFIG *) conf         - config struct
 *                      (char *) sdate          - Start date
 *                      (VENDORCODE *) code     - Vendor's "special" code
 *
 *      Return:         (char *) - The encrypted code, which should be in
 *                                 the license file.
 *
 *      M. Christiano
 *      1/31/90 - Adaped from l_oldcrypt()
 *
 *	Last changed:  12/21/98
 *
 *
 */

 

#ifndef lint
static char *sccsid = "@(#) l_crypt.c v3.71.0.0";
#endif
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include <stdio.h>
#include <errno.h>



#define SIGSIZE 4               

static unsigned long
l_svk(vendor_id, key) 
char *vendor_id;
VENDORCODE *key;
{
  unsigned long keys[4];
  unsigned long signature;
  char sig[SIGSIZE];
  /*- unsigned long x = 0xa8f38730;                   v3.1 */
  /*-unsigned long x = 0x7648b98e;                    v7.0 */
  unsigned long x = 0x6f7330b8;                   /*- v8.x */


        l_key(vendor_id, &(key->keys[0]), keys, 4);



        if (keys == (unsigned long *) NULL)
        {
                return(0);
        }
        else
        {   
          int i = SIGSIZE-1;

                sig[0] = sig[1] = sig[2] = sig[3] = '\0';

                while (*vendor_id)
                {
                        sig[i] ^= *vendor_id++;
                        i--;
                        if (i < 0) i = SIGSIZE-1;
                }
                signature = (long)sig[0] |
                            ((long)sig[1] << 8) |
                            ((long)sig[2] << 16) |
                            ((long)sig[3] << 24);
                signature ^= x;
                signature ^= keys[1];
                signature ^= keys[2];
                signature &= 0xffffffff;
                if (signature == 0) signature = x;      /* 0 invalid */
                return(signature);
        }
}
