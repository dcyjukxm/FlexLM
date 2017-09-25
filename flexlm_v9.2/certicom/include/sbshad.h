/*+
 * sbshad.h - The Application Programmer's Interface -- Key Shadowing Functions
 *
 * This is an automatically generated file.  >>> Do not edit! <<<
 *
 * Description: This header file holds the interface definitions
 *   for the SB SHADOW module.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes: None.
 *
 * 
 * $Revision: 1.13 $
 * This software contains trade secrets and confidential information of Certicom Corp.
 * (c) Copyright Certicom Corp. 1997-1998
-*/
#ifndef SBSHAD_H
#define SBSHAD_H    1

/*===== Included files =========================================*/
#include "sbglb.h"
#include "sbkey.h"

/*===== Constants ==============================================*/
    /*---SB_MAX_SHADOW
     * The largest ordinal counting the shadows.
     */
#define SB_MAX_SHADOW   3

/*===== Macro definitions ======================================*/
/* None */

/*===== Data type definitions ==================================*/
#define SB_MAX_SHADOW_LEN            128

typedef struct  sb_KeyShadow {
    unsigned int size;
    unsigned char data[SB_MAX_SHADOW_LEN];
}  sb_KeyShadow;

/*===== Functions prototypes ===================================*/
/*---sb_shadowCombine()
 * Combine two key-shadow withins the ECC specified at the time of initialisation.
 *
 * Returns:
 * SB_SUCCESS         -- Operation succeeded.
 * SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 * SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 * SB_NO_PRIVATE_KEY  -- NULL pointer passed for the resulting private key.
 * SB_NO_INBUF    -- NULL pointer passed for key-shadow array.
 * SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_shadowCombine(
    void *globalData,
   /*[input]
    * The global data area.
    * Restricted to non-NULL values and must be initialised.
    */
    const sb_KeyShadow shadow[],
   /*[input]
    * The array holding the two key-shadows.
    * Restricted to non-NULL values.
    */
    sb_PrivateKey *decryptionKey
   /*[output]
    * The private key calculated by combining the two shadows.
    * Restricted to non-NULL values.
    */
);
/*---sb_shadowGenerate()
 * Generate a key-shadow within the ECC specified at the time of initialisation.
 *
 * Returns:
 * SB_SUCCESS         -- Operation succeeded.
 * SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 * SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 * SB_NO_PUBLIC_KEY   -- NULL pointer passed for the public key.
 * SB_NO_OUTBUF      -- NULL pointer passed for key-shadow array.
 * SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_shadowGenerate(
    void *globalData,
   /*[input]
    * The global data area.
    * Restricted to non-NULL values and must be initialised.
    */
    sb_PublicKey *encryptionKey,
   /*[output]
    * The public key corresponding the private key to be
    * found within the shadows.
    * Restricted to non-NULL values.
    */
    sb_KeyShadow shadow[]
   /*[output]
    * The array holding the key-shadows.
    * The array must be large enough to hold SB_MAX_SHADOW shadows.
    * Restricted to non-NULL values.
    */
);

#endif /*SBSHAD_H*/
