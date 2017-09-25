/*+
 * sbdul.h - The Application Programmer's Interface
 *              -- Dual Functions Module
 *
 * This is an automatically generated file. >>> Do not edit! <<<
 *
 * Description: This header file holds the interface definitions
 *   for the SB dual-functions module.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes: None.
 *
* 
* 
* This software contains trade secrets and confidential information of Certicom Corp.
* (c) Copyright Certicom Corp. 1997-1998
-*/
#ifndef SBDUL_H
#define SBDUL_H    1

/*===== Included files =========================================*/

#include <limits.h>

#include "sbglb.h"
#include "sbkey.h"
#include "sbdsa.h"
#include "sbenc.h"


/*===== Constants ==============================================*/

/* None */

/*===== Macro definitions ======================================*/

/* None */

/*===== Data type definitions ==================================*/

    /*---sb_EncSignContext
     * Encrypt-while-signing context holds context specific to this process.
     */
typedef struct sb_EncSignContext {
    sb_SignContextECDSA ecdsaContext;
    sb_EncContext       ecesContext;
} sb_EncSignContext;

    /*---sb_DecVerifyContext
     * Decrypt-while-verifying context holds context specific to this process.
     */
typedef struct sb_DecVerifyContext {
    sb_VerifyContextECDSA ecdsaContext;
    sb_EncContext         ecesContext;
} sb_DecVerifyContext;


/*===== Functions prototypes ===================================*/

/*---sb_encryptSignBegin()
 * Begin the encrypt-while-signing process by establishing both a signing and
 * an encryption context.
 *
 * Note: The plaintext is to be signed.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_NO_PUBLIC_KEY        -- The public key buffer is null.
 *      SB_NO_CONTEXT           -- The context pointer is null.
 *      SB_BAD_OUTBUF_LEN       -- The key buffer length is zero.
 *      SB_NO_OUTBUF            -- The key buffer pointer is null.
  *     SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_encryptSignBegin(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * The public key employed for encryption.
     * It is restricted to non-NULL values.
     */
    sb_EncSignContext *context,
    /* [output]
     * The context for the encrypt-while-signing process.
     * It is restricted to non-NULL values.
     */
    unsigned int *keyBufferLength,
    /* [input/output]
     * On input it gives the length of the key buffer in octets.
     * On output it gives the length of the session key in octets.
     * It is restricted to non-NULL values and must point to a non-zero value.
     */
    unsigned char keyBuffer[]
    /* [output]
     * A buffer to hold the session key.
     * It is restricted to non-NULL values.
     */
);

/*---sb_encryptSign()
 * Continue the encrypt-while-signing process by signing the plaintext and then
 * encrypting it, passing back the encrypted plaintext.
 *
 * Notes: None.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_BAD_BUF_LEN          -- The plaintext buffer length is zero.
 *      SB_NO_BUF               -- The plaintext buffer pointer is NULL.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_BAD_CONTEXT          -- The context has not been initialized.
 *      SB_NO_OUTBUF            -- The cyphertext buffer pointer is NULL.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_encryptSign(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const unsigned int textLength,
    /* [input]
     * The length of the plaintext to encrypt and the resulting cyphertext length.
     * It is restricted to non-zero values.
     */
    const unsigned char plaintext[],
    /* [input]
     * The plaintext message.
     * It is restricted to non-NULL values.
     */
    sb_EncSignContext *context,
    /* [input output]
     * The context for the encrypt-while-signing process from before.
     * It is restricted to non-NULL values.
     */
    unsigned char cyphertext[]
    /* [output]
     * The resulting cyphertext.
     * It is restricted to non-NULL values and must hold at least textLength bytes.
     */
);

/*---sb_encryptSignEnd()
 * End the encrypt-while-signing process by returning the signature computed with
 * the given private key..
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_BAD_CONTEXT          -- The context has not been initialised.
 *      SB_NO_PRIVATE_KEY       -- The signing key is NULL.
 *      SB_NO_ECDSA_SIGNATURE   -- The signature is NULL.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_encryptSignEnd(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * The private key employed for signing the encrypted message.
     * It is restricted to non-NULL values.
     */
    sb_EncSignContext *context,
    /* [input output]
     * The context for the encrypt-while-signing process from before.
     * It is restricted to non-NULL values.
     */
    sb_SignatureECDSA *signature
    /* [output]
     * The signature of the cyphertext.
     * It is restricted to non-NULL values.
     */
);

/*---sb_encryptSignPwdEnd()
 * End the encrypt-while-signing process with a password-protected private key.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_BAD_PWD_LEN          -- The password length is zero.
 *      SB_NO_PWD               -- The password pointer is NULL.
 *      SB_NO_PROT_PRIVATE_KEY  -- The protected private key pointer is NULL.
 *      SB_BAD_PRIVATE_KEY      -- The private key is outside the allowed range.
 *      SB_BAD_INBUF_LEN        -- The key length is zero.
 *      SB_NO_INBUF             -- The key buffer pointer is NULL.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_BAD_CONTEXT          -- The context has not been initialised.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_encryptSignPwdEnd(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password.
     * The maximum length is SB_MAX_PWD_LEN bytes and the minimal length is 1.
     */
    const unsigned char password[],
    /* [input]
     * Password (assumed to be of length pwdLength).
     * Restricted to non-NULL values.
     */
    const sb_ProtectedKey *protectedKey,
    /* [input]
     * The password-protected private key employed for signing the encrypted message.
     * It is restricted to non-NULL values.
     */
    sb_EncSignContext *context,
    /* [input output]
     * The context for the encrypt-while-signing process from before.
     * It is restricted to non-NULL values.
     */
    sb_SignatureECDSA *signature
    /* [output]
     * The signature of the cyphertext.
     * It is restricted to non-NULL values.
     */
);

/*---sb_decryptVerifyBegin()
 * Begin the decrypt-while-verifying process.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_NO_PRIVATE_KEY       -- The private key buffer is NULL.
 *      SB_BAD_PRIVATE_KEY      -- The private key is outside the allowed range.
 *      SB_BAD_INBUF_LEN        -- The key length is not equal to the session key length.
 *      SB_NO_INBUF             -- The key buffer pointer is NULL.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_decryptVerifyBegin(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * The private key employed for decryption.
     * It is restricted to non-NULL values.
     */
    const unsigned int sessionKeyLength,
    /* [input]
     * The length of the key buffer in octets.
     * It is restricted to non-zero values.
     */
    const unsigned char sessionKey[],
    /* [input]
     * A buffer that holds the session key.
     * It is restricted to non-NULL values.
     */
    sb_DecVerifyContext *context
    /* [output]
     * The context for the decrypt-while-verifying process.
     * It is restricted to non-NULL values.
     */
);

/*---sb_decryptVerifyPwdBegin()
 * Begin the decrypt-while-verifying process.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.

 *      SB_BAD_INBUF_LEN        -- The key length is not equal to the session key length.
 *      SB_NO_INBUF             -- The key buffer pointer is NULL.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_decryptVerifyPwdBegin(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * The length of the password used to decrypt the protected private key.
     * The maximum length is SB_MAX_PWD_LEN bytes and the minimal length is 1.
     */
    const unsigned char password[],
    /* [input]
     * Password used to decrypt the protected private key.
     * It is restricted to non-NULL values.
     */
    const sb_ProtectedKey *protectedKey,
    /* [input]
     * The password protected private key.
     * It is restricted to non-NULL values.
     */
    const unsigned int sessionKeyLength,
    /* [input]
     * The length of the key buffer in octets.
     * It is restricted to non-zero values.
     */
    const unsigned char sessionKey[],
    /* [input]
     * A buffer that holds the session key.
     * It is restricted to non-NULL values.
     */
    sb_DecVerifyContext *context
    /* [output]
     * The context for the decrypt-while-verifying process.
     * It is restricted to non-NULL values.
     */
);

/*---sb_decryptVerify()
 * Continue the decrypt-while-verifying process.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_BAD_INBUF_LEN        -- The cyphertext length is zero.
 *      SB_NO_INBUF             -- The cyphertext pointer is NULL.
 *      SB_NO_OUTBUF            -- The plaintext pointer is NULL.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_BAD_CONTEXT          -- The context has not been initialised.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_decryptVerify(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const unsigned int textLength,
    /* [input]
     * The length of the cyphertext to decrypt;
     * the resulting plaintext length is the same size.
     * It is restricted to non-zero values.
     */
    const unsigned char cyphertext[],
    /* [input]
     * The cyphertext to decrypt.
     * It is restricted to non-NULL values.
     */
    sb_DecVerifyContext *context,
    /* [input output]
     * The context for the decrypt-while-verifying process from before.
     * It is restricted to non-NULL values.
     */
    unsigned char plaintext[]
    /* [output]
     * The resulting plaintext.
     * It is restricted to non-NULL values and must hold at least textLength bytes.
     */
);

/*---sb_decryptVerifyEnd()
 * End the decrypt-while-verifying process.
 *
 * Returns:
 *      SB_SUCCESS              -- Operation succeeded.
 *      SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *      SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *      SB_NO_FLAG              -- The flag pointer is NULL.
 *      SB_NO_CONTEXT           -- The context pointer is NULL.
 *      SB_BAD_CONTEXT          -- The context has not been initialised.
 *      SB_NO_PUBLIC_KEY        -- NULL pointer passed for public key.
 *      SB_NO_ECDSA_SIGNATURE   -- NULL pointer passed for signature.
 *      SB_BAD_PUBLIC_KEY       -- Incorrect public key format.
 *      SB_BAD_ECDSA_SIGNATURE  -- Incorrect signature format.
 *      SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_decryptVerifyEnd(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * The private key employed for signing the encrypted message.
     * It is restricted to non-NULL values.
     */
    const sb_SignatureECDSA *signature,
    /* [input]
     * Pointer to the signature to be verified.
     * It is restricted to non-NULL values.
     */
    sb_DecVerifyContext *context,
    /* [input output]
     * The context for the decrypt-while-verifying process from before.
     * It is restricted to non-NULL values.
     */
    int *result
    /* [output]
     * Pointer to the result of the verification.
     * Returns 1 if verification passed; returns 0 if verification failed.
     * It is restricted to non-NULL values.
     */
);


#endif /*SBDUL_H*/
