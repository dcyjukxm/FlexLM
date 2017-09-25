/*+
 * sbkey.h - The Application Programmer's Interface -- Key-Generation Module
 *
 * This is an automatically generated file.  >>> Do not edit! <<<
 *
 * Description: This header file holds the interface definitions
 *   for the SB KEY module.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes: None.
 *
 * 
 * 
 * Copyright Certicom Corp., 1996. All rights reserved.
-*/
#ifndef SBKEY_H
#define SBKEY_H


/*===== Included files =========================================*/

#include "sbglb.h"



/*===== Constants and Data Structures ==========================*/

    /*---sb_PublicKey
     * ECC public key.
     */
#define SB_PUB_KEY_LEN                  ((2 * SB_FIELD_SIZE_OCTETS) + 1)

typedef struct sb_PublicKey {
    unsigned int size;                          /* Number of bytes used. */
    unsigned char key[ SB_PUB_KEY_LEN ];
} sb_PublicKey;


    /*---sb_PrivateKey
     * ECC private key.
     */
#define SB_PRIV_KEY_LEN         (8* ((SB_FIELD_SIZE_OCTETS + (1 + 7)) / 8))

typedef struct sb_PrivateKey {
    unsigned int size;                  /* Number of bytes used. */
    unsigned char key[ SB_PRIV_KEY_LEN ];
} sb_PrivateKey;


    /*--sb_ProtectedKey
     * ECC password-protected private key.
     */
typedef struct sb_ProtectedKey {
    unsigned int size;
    unsigned char protectedKey[ SB_PRIV_KEY_LEN ];
} sb_ProtectedKey;


    /*---SB_MAX_PWD_LEN
     * The maximal number of octets permitted in a password.
     */
#define SB_MAX_PWD_LEN          128



/*===== Functions prototypes ===================================*/

/*---sb_genKeyPair()
 * Generate a key-pair using the random-number generation
 * specified at the time of initialisation.
 *
 * Returns:
 *      SB_SUCCESS         -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *      SB_NO_PRIVATE_KEY  -- NULL pointer passed for the private key.
 *      SB_NO_PUBLIC_KEY   -- NULL pointer passed for the public key.
 *      SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_genKeyPair(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values and must be initialised.
     */
    sb_PrivateKey *privateKey,
    /* [output]
     * The resulting private key.
     * Restricted to non-NULL values.
     */
    sb_PublicKey *publicKey
    /* [output]
     * The resulting public key.
     * Restricted to non-NULL values.
     */
);


/*---sb_genPwdKeyPair()
 * Generate a public key-pair whose private key is protected via a password.
 * The private key is protected via DESX.
 *
 * Returns:
 *      SB_SUCCESS              -- Function succeeded.
 *      SB_NO_PUBLIC_KEY        -- NULL pointer passed to hold the public key.
 *      SB_NOT_INITIALIZED      -- The global data has not been initialised.
 *      SB_NO_PROT_PRIVATE_KEY  -- NULL pointer passed to hold the protected
 *                                 private key.
 *      SB_BAD_PWD_LEN          -- Zero password length specified.
 *      SB_NO_PWD               -- NULL pointer passed for the password.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for the global data.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_genPwdKeyPair(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values and must be initialised.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password;
     * the maximum length is SB_MAX_PWD_LEN bytes and
     * the minimal length is 1.
     */
    const unsigned char password[],
    /* [input]
     * Password (assumed of length pwdLength).
     * Restricted to non-NULL values.
     */
    sb_ProtectedKey *protectedKey,
    /* [output]
     * The resulting private key encrypted with the password.
     * Restricted to non-NULL values.
     */
    sb_PublicKey *publicKey
    /* [output]
     * The resulting public key.
     * Restricted to non-NULL values.
     */
);


/*---sb_changePwdKey()
 *
 * Returns:
 *      SB_SUCCESS              -- Function succeeded.
 *      SB_NO_PUBLIC_KEY        -- NULL pointer passed to hold the public key.
 *      SB_NOT_INITIALIZED      -- The global data has not been initialised.
 *      SB_NO_PROT_PRIVATE_KEY  -- NULL pointer passed to hold the protected
 *                                 private key.
 *      SB_BAD_PROT_PRIVATE_KEY -- Zero length specified for the protected
 *                                 private key.
 *      SB_BAD_PWD_LEN          -- Zero old password length specified.
 *      SB_BAD_NEW_PWD_LEN      -- Zero new password length specified.
 *      SB_NO_PWD               -- NULL pointer passed for the old password.
 *      SB_NO_NEW_PWD           -- NULL pointer passed for the new password.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for the global data.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_changePwdKey(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values and must be initialised.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the existing password;
     * the maximum length is SB_MAX_PWD_LEN bytes and
     * the minimal length is 1.
     */
    const unsigned char password[],
    /* [input]
     * Existing password (assumed of length pwdLength).
     * Restricted to non-NULL values.
     */
    const unsigned int newPwdLength,
    /* [input]
     * Length of the new password;
     * the maximum length is SB_MAX_PWD_LEN bytes and
     * the minimal length is 1.
     */
    const unsigned char newPassword[],
    /* [input]
     * New password (assumed of length pwdLength).
     * Restricted to non-NULL values.
     */
    sb_ProtectedKey *protectedKey
    /* [input output]
     * On input, the private key protected with the old password.
     * On output, the private key protected with the new password.
     * Restricted to non-NULL values.
     */
);


/*---sb_genPublicKey()
 * Generate a public key from a private key.
 *
 * Returns:
 *      SB_SUCCESS         -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *      SB_NO_PRIVATE_KEY  -- NULL pointer passed for the private key.
 *      SB_NO_PUBLIC_KEY   -- NULL pointer passed for the public key.
 *      SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_genPublicKey(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values and must be initialised.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * The private key.
     * Restricted to non-NULL values.
     */
    sb_PublicKey *publicKey
    /* [output]
     * The resulting public key.
     * Restricted to non-NULL values.
     */
);


/*---sb_expandPublicKey()
 * Expands a public key by attaching a table.
 * The user has to allocate the memory for the table.
 * In the expanded public key the size is set to 1 and the key array 
 * stores the address of the table.
 *
 * Returns:
 *      SB_SUCCESS         -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *      SB_NO_PUBLIC_KEY   -- NULL pointer passed for the public key.
 *      SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_expandPublicKey(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values and must be initialised.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * The public key.
     * Restricted to non-NULL values.
     */
    void *table, 
    /* [input]
     * The address of the allocated memory. 
     */ 
    sb_PublicKey *publicKeyExpanded
    /* [output]
     * The expanded public key. 
     * It has the size set to 1 and the key field stores the address of the 
     * table. 
     */
    
);


/*--sb_old2newPublicKey()
 * Convert a public key from old-style format to new-style format.
 *
 * Notes:
 *  (1) The contents of the new-style key are undefined if an error
 *      occurs.
 *  (3) The compression format of the new-style key is determined by
 *          the compression option set at initialization time.
 *
 * Returns:
 *      SB_SUCCESS            -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA     -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED    -- Global data structure has not been initialised.
 *      SB_NO_OLD_PUBLIC_KEY  -- NULL pointer passed for the old-sytle public
 *                               key.
 *      SB_BAD_OLD_PUBLIC_KEY -- Old-style key is not in the correct format.
 *      SB_NO_NEW_PUBLIC_KEY  -- NULL pointer passed for the new-style public
 *                               key.
 *      SB_FAILURE            -- Failure in the underlying cryptosystem.
 */
int
sb_old2newPublicKey(
    void *globalData,
        /*[input]
         * The global data area.  It is restricted to non-null values.
         */
    const sb_PublicKey *oldStyleKey,
        /*[input]
         * The old-style public key to be converted.
         * It is restricted to non-null values.
         */
    sb_PublicKey *newStyleKey
        /*[output]
         * The resulting public key in the new-style format as specified by the
         * compression options.  It is restricted to non-null values.
         */
);


/*---sb_new2oldPublicKey()
 * Convert a public key from new-style format to old-style uncompressed format.
 *
 * Notes:
 *  (1) The contents of the old-style key are undefined if an error
 *      occurs.
 *
 * Returns:
 *      SB_SUCCESS            -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA     -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED    -- Global data structure has not been initialised.
 *      SB_NO_NEW_PUBLIC_KEY  -- NULL pointer passed for the new-style public
 *                               key.
 *      SB_BAD_NEW_PUBLIC_KEY -- New-style key is not in the correct format.
 *      SB_NO_OLD_PUBLIC_KEY  -- NULL pointer passed for the old-sytle public
 *                               key.
 *      SB_FAILURE            -- Failure in the underlying cryptosystem.
 */
int
sb_new2oldPublicKey(
    void *globalData,
        /*[input]
         * The global data area.  It is restricted to non-null values.
         */
    const sb_PublicKey *newStyleKey,
        /*[input]
         * The new-style public key to be converted. It is restricted to
         * non-null values.
         */
    sb_PublicKey *oldStyleKey
        /*[output]
         * The resulting public key in the old-style uncompressed format.
         * It is restricted to non-null values.
         */
);


#endif /*SBKEY_H*/
