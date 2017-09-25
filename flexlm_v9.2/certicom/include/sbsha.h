/*+
 * sbsha.h - The Application Programmer's Interface
 *              -- Secure Hashing Algorithm Module
 * 
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB SHA module.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes:
 *   (1) The functions sb_sha1Begin(), sb_sha1Hash(), and sb_sha1End() are
 *       an implementation of FIPS 180-1, Secure Hashing Standard,
 *       Federal Information Processing Standards Publication,
 *       U.S Department of Commerce/N.I.S.T, Computer Systems Laboratory, 
 *       Gaithersburg, Maryland. April 17, 1995.
 *
* 
* 
* Copyright Certicom Corp., 1996. All rights reserved.
-*/
#ifndef SBSHA_H
#define SBSHA_H    1

    /*
     * Include Files
     */
#include "sbhash.h"
#include "sbrc.h"


        /*
         * SHA-1 specific constant
         */
    /*---SB_SHA1_MSG_DIG_LEN
     * This value defines the length of the message digest for SHA-1.
     */
#define SB_SHA1_MSG_DIG_LEN 20



    /*
     * Function Prototypes
     */
/*---sb_sha1Begin()
 * Initiate the hashing context.
 *
 * Notes:
 *   (1) The hashing context is zero-filled whenever an error occurs.
 *
 * Return:
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_CONTEXT     --The hash context is null.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_sha1Begin(
    void *globalData,
        /* [input]
         * This is the global data. It is unused.
         */
    sb_HashContext *hashContext
        /* [input/output]
         * This is the hashing context. It is restricted to non-null
         * values.
         */
);
/*---sb_sha1Hash()
 *  Add a message block to the hashing context.
 *
 * Notes:
 *   (1) The hashing context is zero-filled whenever an error occurs.
 *
 * Return:
 *    SB_BAD_CONTEXT    --The hash context is not initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_INBUF       --The message buffer is null.
 *    SB_NO_CONTEXT     --The hash context is null.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_sha1Hash(
    void *globalData,
        /* [input]
         * This is the global data. It is unused.
         */
    const unsigned int msgBlkLen,
        /* [input]
         * This is the length of the message block. It must be greater
         * than zero.
         */
    const unsigned char msgBlk[],
        /* [input]
         * This is the message block to add to the hashing context. It is
         * restricted to non-null values.
         */
    sb_HashContext *hashContext
        /* [input/output]
         * This is the hashing context. It is restricted to initialized
         * non-null values.
         */
);
/*---sb_sha1End()
 * Create the message digest.
 * 
 * Notes:
 *   (1) The hashing context is zero-filled whenever an error occurs.
 *   (2) The contents of the message digest are undefined whenever an
 *       error occurs.
 *
 * Return:
 *    SB_BAD_CONTEXT    --The hash context is not initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_CONTEXT     --The hash context is null.
 *    SB_NO_OUTBUF      --The message digest is null.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_sha1End(
    void *globalData,
        /* [input]
         * This is the global data. It is unused.
         */
    sb_HashContext *hashContext,
        /* [input/output]
         * This is the hashing context. It is restricted to non-null
         * values.
         */
    sb_MessageDigest *msgDig
        /* [output]
         * This is the message digest. It is restricted to non-null
         * values.
         */
);

#endif /*SBSHA_H*/
