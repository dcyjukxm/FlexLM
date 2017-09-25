/*+
 * sbiniold.h - The Application Programmer's Interface
 *              -- Old Initialization Module
 * 
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB INI module.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes: None.
 *
* 
* 
* This software contains trade secrets and confidential information  of Certicom Corp.
* (c) Copyright Certicom Corp. 1997-1998
-*/
#ifndef SBINIOLD_H
#define SBINIOLD_H    1

/*=== Include Files =========================================================*/

#include "sbini.h"


/*=== Function Prototypes ===================================================*/
/*---sb_initializeOld()
 * Initialize Security Builder.
 *
 * Notes:
 *   (1) The heap space must contain the number of bytes specified by the
 *       function sb_heapSize() or it must be null. If it is null, the heap
 *       size must equal zero; if it is not null the heap size must contain the
 *       number of bytes specified by the function sb_heapSize(). Note that the
 *       function sb_heapSize() returns a zero heap size whenever an error
 *       occurs in it. 
 *   (2) The data space must contain the number of bytes specified by the
 *       function sb_dataSize(), and the data size parameter must specify the
 *       number of bytes specified by the function sb_dataSize().
 *
 * Returns:
 *   SB_FAILURE              -- Initialization failed.
 *   SB_BAD_COMPRESSION_FLAG -- Initialization options specifies out-of-range
 *                              point compression flag value.
 *   SB_BAD_ECES_COMPRESSION_FLAG  -- Initialization options specifies out-of-range
 *                                    point compression style flag
 *   SB_BAD_DATA_SIZE        -- Data size indicates data space is insufficient.
 *   SB_BAD_EC_PARAMS        -- Unrecognized elliptic curve parameters.
 *   SB_BAD_HEAP_SIZE        -- Heap size indicates heap space is insufficient.
 *   SB_BAD_RANDOM_INPUT     -- Initialization options specifies a seed containing
 *                              an unacceptable value.
 *   SB_BAD_RNG_TYPE         -- Initialization options specifies an unknown random
 *                              number generator type.
 *   SB_BAD_SEED_LEN         -- Initialization options specifies a seed length
 *                              which is not equal to SB_MAX_SEED_LEN.
 *   SB_NO_EC_PARAMS         -- Null curve object provided.
 *   SB_NO_DATA_SPACE        -- Null data space provided.
 *   SB_NO_HEAP_SPACE        -- Null heap space provided with non-zero heap space.
 *   SB_NO_OPTIONS_LIST      -- Null list of initialization options.
 *   SB_SUCCESS              -- Initialization succeeded.
 *   SB_NOT_IMPLEMENTED      -- This function not implemented
 */
int
sb_initializeOld(
    const ellipticCurveParameters *ecParams,
        /* [input]
         * The elliptic curve parameters. It is restricted to non-null values.
         */
    const sb_OptionsList *optionsList,
        /* [input]
         * The list of options to include with the elliptic curve parameters. It
         * is restricted to non-null values.
         */
    const unsigned int dataSize,
        /* [input]
         * The size of the data space. It is restricted to the value returned by
         * the function sb_dataSize() when called with the elliptic curve
         * parameters provided to this function.
         */
    const unsigned int heapSize,
        /* [input]
         * The size of the heap space. It is restricted to the value returned by
         * the function sb_heapSize() when called with the elliptic curve
         * parameters provided to this function, or it must be zero.
         */
    void *dataSpace,
        /* [input/output]
         * The data space. It is restricted to non-null values.
         */
    void *heapSpace
        /* [input/output]
         * The heap space. It is restricted to non-null values whenever the
         * heap size is non-zero; otherwise is is restricted to null.
         */
);

#endif /*SBINIOLD_H*/
