/*+
 * sbmd5.h - The Application Programmer's Interface -- MD5 Module
 *
 * Description: This header file holds the interface definitions
 * for the SB MD5 module.  This is an automatically generated file.
 * Do not edit.
 *
 * 
 * 
 *
 * This software contains trade secrets and confidential information of
 * Certicom Corp.
 * (c) Copyright Certicom Corp. 1999
-*/
#ifndef SBMD5_H
#define SBMD5_H


/*=============include files==================================*/

#include "sbhash.h"

/*=============definitions====================================*/

#define sb_Md5HashContext sb_HashContext
#define sb_Md5MessageDigest sb_MessageDigest

/*=============function prototypes============================*/

/*---sb_md5Begin()
 * Initialize an MD5 context from the MD5 request.
 *       
 * Notes:  
 *
 * Return: 
 *    SB_NO_CONTEXT      -- The context is NULL.
 *    SB_SUCCESS         -- The operation completed successfully.
 */
int
sb_md5Begin(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    sb_HashContext *hashContext
        /* [input/output]
         * The Hash Context.
         * Hash context must be non-NULL. 
         */
    );


/*---sb_md5Hash()
 * Encrypt data using an MD5 context.
 * This function can be called repeatedly as needed.
 *
 * Notes:
 *
 * Return: 
 *    SB_NO_CONTEXT    -- The context is NULL.
 *    SB_BAD_CONTEXT   -- The context is not initialized.
 *    SB_NO_INBUF      -- The message buffer is NULL.
 *    SB_SUCCESS       -- The operation completed successfully.
 */
int
sb_md5Hash(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    const unsigned sbint32 msgBlkLen,
        /* [input]
         * Length of the message block (in bytes).
         */
    const unsigned char msgBlk[],
        /* [input]
         * The message block.
         */
    sb_HashContext *hashContext
        /* [input/output]
         * The Hash Context.
         * Hash context must be non-NULL.
         */
    );


/*---sb_md5End()
 * Close the MD5 context.
 *
 * Return: 
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not initialized.
 *    SB_NO_OUTBUF           -- The message digest is NULL.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_md5End(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    sb_HashContext *hashContext,
        /* [input/output]
         * The Hash Context.
         * Hash context must be non-NULL.
         */
    sb_MessageDigest *msgDig
        /* [output]
         * The message digest.
         * This parameter must be non-NULL.
         */
    );


#endif /*sbmd5.h*/
