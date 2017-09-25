/*+
 * sbmd2.h - The Application Programmer's Interface -- MD2 Module
 *
 * Description: This header file holds the interface definitions
 * for the SB MD2 module.  This is an automatically generated file.
 * Do not edit.
 *
 * 
 * 
 *
 * This software contains trade secrets and confidential information of
 * Certicom Corp.
 * (c) Copyright Certicom Corp. 1999
-*/
#ifndef SBMD2_H
#define SBMD2_H



/*=============include files==================================*/

#include "sbhash.h"



/*=============function prototypes============================*/

/*---sb_md2Begin()
 * Initialize an MD2 context from the MD2 request.
 *       
 * Notes:  
 *   (1) If an error occurs, the resulting MD2 context is zero filled.
 *
 * Return: 
 *    SB_NO_CONTEXT      -- The context is NULL.
 *    SB_SUCCESS         -- The operation completed successfully.
 */
int
sb_md2Begin(
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


/*---sb_md2Hash()
 * Encrypt data using an MD2 context.
 * This function can be called repeatedly as needed.
 *
 * Notes:
 *      The context is cleared if an error is detected.
 *
 * Return: 
 *    SB_NO_CONTEXT    -- The context is NULL.
 *    SB_BAD_CONTEXT   -- The context is not initialized.
 *    SB_NO_INBUF      -- The message buffer is NULL.
 *    SB_SUCCESS       -- The operation completed successfully.
 */
int
sb_md2Hash(
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


/*---sb_md2End()
 * Close the MD2 context.
 *
 * Return: 
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not initialized.
 *    SB_NO_OUTBUF           -- The message digest is NULL.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_md2End(
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


#endif /*sbmd2.h*/
