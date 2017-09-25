/*+
 * sbrng.h - The Application Programmer's Interface
 *           Random Number Generator (RNG) module
 *
 * Description: This header file holds the interface definitions
 * for the RNG.
 * This is an automatically generated file.  Do not edit.
 *
 * 
 * 
 *
 * This software contains trade secrets and confidential information of
 * Certicom Corp.
 * (c) Copyright Certicom Corp. 1999
-*/
#ifndef SBRNG_H
#define SBRNG_H

/*=============include files==================================*/

#include "sbini.h"



/*=============constants======================================*/

    /*
     * The following constants must be used to identify the type
     * of RNG.
     * The types SB_BBS_RNG and SB_FIPS_RNG are defined in "sbini.h".
     */
#define SB_FIPS_140_1   2
    /*
     * Self-test according to FIPS 140-1 with the FIPS 186-1 RNG.
     */
#define SB_FIPS_186_1   3
    /*
     * Random byte string generation based on FIPS 186-1 (alternative).
     */
#define SB_EXTERNAL     50
    /*
     * External RNG (usually not implemented).
     */



/*=============function prototypes============================*/

/*---sb_rngFIPS186Session()
 * The generates the random byte string without optional user input.
 *
 * Notes:
 *
 * Return: 
 *    SB_SUCCESS         -- The operation completed successfully.
 *    SB_NO_GLOBAL_DATA  -- The global data is NULL.
 *    SB_NOT_INITIALIZED -- The global data is not initialized.
 *    SB_NO_OUTBUF       -- The random byte string is NULL.
 *    SB_FAILURE         -- The operation failed.
 */
int
sb_rngFIPS186Session(
    void *globalData,
        /* [input]
         * The global data.
         * The global data must be initialized.
         */
    const unsigned int length,
        /* [input]
         * The length of the required random byte string.
         */
    unsigned char randomBytes[]
        /* [output]
         * The random byte string.
         * Must not be NULL.
         */
    );


/*---sb_rngFIPS186Private()
 * The generates the random byte string with optional user input.
 *
 * Notes:
 *
 * Return: 
 *    SB_SUCCESS          -- The operation completed successfully.
 *    SB_NO_GLOBAL_DATA   -- The global data is NULL.
 *    SB_NOT_INITIALIZED  -- The global data is not initialized.
 *    SB_NO_INBUF         -- The optional user input is NULL when it is used.
 *    SB_BAD_RANDOM_INPUT -- The optional user input is all 0 or all 1 when
 *                           it is used.
 *    SB_NO_OUTBUF        -- The random byte string is NULL.
 *    SB_FAILURE          -- The operation failed.
 */
int
sb_rngFIPS186Private(
    void *globalData,
        /* [input]
         * The global data.
         * The global data must be initialized.
         */
    const unsigned int optLength,
        /* [input]
         * The length of the optional user input.
         * Must be greater than zero if the optional user input is used.
         */
    const unsigned char optInput[],
        /* [input]
         * The optional user input
         * Must be NULL if it is not used.
         */
    const unsigned int length,
        /* [input]
         * The length of the required random byte string.
         */
    unsigned char randomBytes[]
        /* [output]
         * The random byte string.
         * Must not be NULL.
         */
    );


/*---sb_rngSeedFIPS186()
 * Provides the optional user input to the RNG.
 *
 * Notes:
 *
 * Return:
 *    SB_SUCCESS          -- The operation completed successfully.
 *    SB_NO_GLOBAL_DATA   -- The global data is NULL.
 *    SB_NOT_INITIALIZED  -- The global data is not initialized.
 *    SB_NO_INBUF         -- The seed value is NULL.
 *    SB_BAD_RANDOM_INPUT -- The optional user input is all 0 or all 1 when
 *                           it is used.
 *    SB_BAD_INBUF_LEN    -- The seed value length is zero.
 *    SB_FAILURE          -- The operation failed.
 */
int 
sb_rngSeedFIPS186(
    void *globalData,
        /* [input]
         * The global data.
         */
    const unsigned int seedLength,
        /* [input]
         * The length of the seed.
         * Must be more than zero.
         */
    const unsigned char seedData[]
        /* [output]
         * The seed value.
         * Must not be NULL.
         */
    );


/*---sb_fipsRngOptionalInput()
 * Provides the optional user input to the RNG.
 *
 * Notes:
 *
 * Return:
 *    SB_SUCCESS          -- The operation completed successfully.
 *    SB_NO_GLOBAL_DATA   -- The global data is NULL.
 *    SB_NOT_INITIALIZED  -- The global data is not initialized.
 *    SB_NO_INBUF         -- The seed value is NULL.
 *    SB_BAD_INBUF_LEN    -- The seed value length is zero.
 *    SB_BAD_RANDOM_INPUT -- The optional user input is all 0 or all 1 when
 *                           it is used.
 *    SB_FAILURE          -- The operation failed.
 */
int 
sb_fipsRngOptionalInput(
    void *globalData,
        /* [input]
         * The global data.
         */
    const unsigned int length,
        /* [input]
         * The length of the seed.
         * Must be more than zero.
         */
    const unsigned char byteString[]
        /* [output]
         * The seed value.
         * Must not be NULL.
         */
    );




/*---sb_rngSize()
 * Provides the size of the RNG data space.
 *
 * Notes:
 *
 * Return: 
 *    SB_SUCCESS         -- The operation completed successfully.
 *    SB_NO_OUTBUF       -- The rng size variable is NULL.
 */
int
sb_rngSize(
    size_t *rngSize
        /* [output]
         * The size of the RNG data in bytes.
         * Must not be NULL.
         */
    );


/*---sb_rngInit()
 * Initializes the RNG.
 *
 * Notes:
 *
 * Return: 
 *    SB_SUCCESS          -- The operation completed successfully.
 *    SB_NO_GLOBAL_DATA   -- The global data is NULL.
 *    SB_NOT_IMPLEMENTED  -- The required RNG algorithm is not implemented.
 *    SB_BAD_INBUF_LEN    -- The initial seed length is zero.
 *    SB_NO_INBUF         -- The initial seed is NULL.
 *    SB_BAD_RANDOM_INPUT -- The optional user input is all 0 or all 1 when
 *                           it is used.
 *    SB_FAILURE          -- The operation failed.
 */
int
sb_rngInit(
    void *globalData,
        /* [input]
         * The global data.
         */
    int rngType,
        /* [input]
         * The type of RNG.
         */
    unsigned int seedLen,
        /* [input]
         * The length of the initial seed.
         * Must be more than zero.
         */
    const unsigned char seedValue[]
        /* [input]
         * The initial seed.
         * It is restricted to a non-NULL value.
         */
    );


#endif /*sbrng.h*/
