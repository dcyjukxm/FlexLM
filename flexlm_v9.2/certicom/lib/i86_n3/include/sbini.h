/*+
 * sbini.h - The Application Programmer's Interface
 *              -- Initialization Module
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
#ifndef SBINI_H
#define SBINI_H    1

/*=== Include Files =========================================================*/
#include "curves.h"
#include <stddef.h>
#include "sbrc.h"

/*=== Constant Values =======================================================*/
    /*---SB_MAX_SEED_LEN
     * The maximum length of the seed.
     */
#define SB_MAX_SEED_LEN 32

/*=== Data Type Definitions =================================================*/
    /*---sb_OptionsList
     * The list of options to be added to the elliptic curve parameters upon
     * initialization.
     */
enum sb_rngType {
    SB_BBS_RNG, /* Blum-Blum-Shubb Random Number Generator */
    SB_FIPS_RNG /* FIPS-186 Random Number Generator        */
};

typedef struct sb_OptionsList {
    enum pointCompressionFlag {
        SB_OFF,     /* Point compression is off. */
        SB_ON,      /* Point compression is on.  */
        SB_HYBRID   /* It is a hybrid. */
    } pointCompressionFlag;
    enum ecesCompressionFlag {
        SB_P1363_22AUG1996,  /* Old style of point compression. */
        SB_P1363_13FEB1998   /* New style of point compression. */
    } ecesCompressionFlag;
    struct rngInfo {
        enum sb_rngType type;
        struct seed {
            unsigned int size;                      /* Seed size (in bytes). */
            unsigned char value[ SB_MAX_SEED_LEN ]; /* The seed.             */
        } seed;
    } rngInfo;
} sb_OptionsList;

/*=== Function Prototypes ===================================================*/
/*---sb_dataSize()
 * Returns the size (in bytes) of the data space needed for the elliptic curve
 * parameters.
 *
 * Notes:
 *   (1) The data size is zero-filled whenever an error occurs.
 *
 * Returns:
 *   SB_BAD_EC_PARAMS    -- Unrecognized elliptic curve parameters.
 *   SB_FAILURE          -- Failed to complete operation.
 *   SB_NO_EC_PARAMS     -- Null elliptic curve paramaters.
 *   SB_NO_OUTBUF        -- Null data size buffer.
 *   SB_SUCCESS          -- Successfully completed operation.
 */
int
sb_dataSize(
    const ellipticCurveParameters *ecParams,
        /* [input]
         * The elliptic curve parameters. It is restricted to non-null values.
         */
    size_t *dataSize
        /* [output]
         * The data space needed (in bytes) to use the curve object. It is
         * restricted to non-null values.
         */
);
/*---sb_heapSize()
 * Returns the size (in bytes) of the heap space needed for the elliptic curve
 * parameters.
 *
 * Notes:
 *   (1) The heap size is zero-filled whenever an error occurs. Note that a
 *       heap size of zero and a null heap pointer are valid initialization
 *       values.
 *
 * Returns:
 *   SB_BAD_EC_PARAMS    -- Unrecognized elliptic curve parameters.
 *   SB_FAILURE          -- Failed to complete operation.
 *   SB_NO_EC_PARAMS     -- No elliptic curve parameters.
 *   SB_NO_OUTBUF        -- No buffer for the heap size.
 *   SB_SUCCESS          -- Successfully completed operation.
 *   SB_NOT_IMPLEMENTED  -- This function not implemented
 */
int
sb_heapSize(
    const ellipticCurveParameters *ecParams,
        /* [input]
         * The elliptic curve parameters. It is restricted to non-null values.
         */
    size_t *heapSize
        /* [output]
         * The heap space needed (in bytes) to use the curve object. It is
         * restricted to non-null values.
         */
);
/*---sb_getOID()
 * The location and length of the Object Identifier (OID) of the elliptic 
 * curve used is set in the output.
 *
 * Notes:
 *       None.
 *
 * Returns:
 *   SB_NO_GLOBAL_DATA   -- Null global data is provided.
 *   SB_NOT_INITIALIZED  -- Security Builder is not initialized.
 *   SB_SUCCESS          -- Successfully completed operation.
 */
int
sb_getOID(
    void *globalData,
    /* [input]
     * The global data.
     */
    unsigned int *oidLength,
    /* [output]
     * The size of the OID. 
     * Restricted to non-null values.
     */
    const unsigned char **oid 
    /* [output]
     * The location of the OID as a pointer to the unsigned char array. 
     */
);
/*---sb_initializeCurve()
 * Initialize Security Builder.
 *
 * Notes:
 *   (1) 
 *   (2) 
 *
 * Returns:
 *   SB_FAILURE              -- Initialization failed.
 *   SB_BAD_EC_PARAMS        -- Unrecognized elliptic curve parameters.
 *   SB_NO_EC_PARAMS         -- Null curve object provided.
 *   SB_SUCCESS              -- Initialization succeeded.
 */
int
sb_initializeCurve(
    ellipticCurveParameters *ecParams
    /* [input/output]
     * As input:  the parameters provided by the user. 
     * As output: the complete curve object to be used by SB.
     */
);
/*---sb_initialize()
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
sb_initialize(
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
/*---sb_end()
 * Zero-fill the global data space.
 *
 * Notes: None.
 *
 * Returns:
 *   SB_NO_GLOBAL_DATA--Null global data space.
 *   SB_SUCCESS       --Operation succeeded.
 */
int
sb_end(
    void *globalData
        /* [input/output]
         * The global data space to be zero-filled. It is restricted to non-null
         * values.
         */
);

#endif /*SBINI_H*/
