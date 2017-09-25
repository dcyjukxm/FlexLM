/*+
 * sbhash.h - The Application Programmer's Interface
 *              -- Common data structure for hash algorithms
 *
 * Description: This header file holds the data structure definitions
 * for the SB hash algorithm  modules.
 * This is an automatically generated file. Do not edit.
 *
 * 
 * 
 *
 * This software contains trade secrets and confidential information of
 * Certicom Corp.
 * (c) Copyright Certicom Corp. 1998
-*/
#ifndef SBHASH_H
#define SBHASH_H



    /*
     * sb_HashContext:
     * The context structure common to all hash algorithms.
     */
#define SB_HASH_CONTEXT_LENGTH 100

typedef struct sb_HashContext{
    unsigned char context[SB_HASH_CONTEXT_LENGTH];
} sb_HashContext;


    /*
     * sb_MessageDigest:
     * The message digest structure common to all hash algorithms.
     * It contains the maximum length buffer of the message digest.
     */
#define SB_DIGEST_LENGTH 20

typedef struct sb_MessageDigest{
    unsigned int size;
    unsigned char digest[SB_DIGEST_LENGTH];
} sb_MessageDigest;


#endif /*sbhash.h*/
