/*+
 * sbexc.h - The Application Programmer's Interface -- Key-exchange Module.
 *
 * This is an automatically generated file.  >>> Do not edit! <<<
 *
 * Description: This header file holds the interface definitions
 *   for the SB EXCH module.
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
#ifndef SBEXC_H
#define SBEXC_H    1


/*===== Included files =========================================*/

#include "sbkey.h"



/*===== Constants ==============================================*/

#define SB_MAX_ADD_INFO_LEN 2048



/*===== Macro definitions ======================================*/

/* None */



/*===== Data type definitions ==================================*/

/* None */



/*===== Functions prototypes ===================================*/

    /*
     * DH Functions.
     */

/*---sb_dhGenerateValues()
 * Generate the local and remote values required by the DH key-exchange.
 *
 * Returns:
 *  SB_SUCCESS         -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY    -- NULL pointer passed for the local private key.
 *  SB_NO_REMOTE_KEY   -- NULL pointer passed for the remote public key.
 *  SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_dhGenerateValues(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    sb_PublicKey *remoteKey,
    /* [output]
     * The remote value for the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    sb_PrivateKey *localKey
    /* [output]
     * The local value for the DH key-exchange.
     * It is restricted to non-NULL values.
     */
);


/*---sb_dhPwdGenerateValues()
 * Generate the remote and local values for the Diffie-Hellman key-exchange
 * whose  local value is protected via a password.
 * The local value is protected via DESX.
 *
 * Returns:
 *      SB_SUCCESS           -- Function succeeded.
 *      SB_NO_REMOTE_KEY     -- NULL pointer passed to hold the remote value.
 *      SB_NOT_INITIALIZED   -- The global data has not been initialised.
 *      SB_NO_PROT_LOCAL_KEY -- NULL pointer passed to hold the protected
 *                              local value.
 *      SB_BAD_PWD_LEN       -- Zero password length specified.
 *      SB_NO_PWD            -- NULL pointer passed for the password.
 *      SB_NO_GLOBAL_DATA    -- NULL pointer passed for the global data.
 *      SB_FAILURE           -- Failure in the underlying cryptosystem.
 */
int
sb_dhPwdGenerateValues(
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
    sb_ProtectedKey *protectedLocalValue,
    /* [output]
     * The resulting local value encrypted with the password.
     * Restricted to non-NULL values.
     */
    sb_PublicKey *remoteValue
    /* [output]
     * The resulting remote value.
     * Restricted to non-NULL values.
     */
);


/*---sb_dhSharedSecret()
 * Generate the shared secret from the local key and the remote key using the DH
 * key-exchange algorithm.
 *
 * Returns:
 *  SB_SUCCESS         -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY    -- NULL pointer passed for the local private key.
 *  SB_NO_REMOTE_KEY   -- NULL pointer passed for the remote public key.
 *  SB_NO_OUTBUF       -- NULL pointer passed for the shared secret buffer.
 *  SB_BAD_INBUF_LEN   -- Bad value specified for length of the shared secret.
 *  SB_BAD_LOCAL_KEY   -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY  -- Remote key in incorrect format.
 *  SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_dhSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * Remote contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * Local contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * It is restricted to non-NULL values.
     */
);


/*---sb_dhSharedSecretWithAddInfo()
 * Generate the shared secret from the local key, the remote key and the
 * additional information using the DH key-exchange algorithm.
 *
 * Returns:
 *  SB_SUCCESS         -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY    -- NULL pointer passed for the local private key.
 *  SB_NO_REMOTE_KEY   -- NULL pointer passed for the remote public key.
 *  SB_NO_OUTBUF       -- NULL pointer passed for the shared secret buffer.
 *  SB_NO_BUF          -- NULL pointer passed for the additional information
 *                        buffer.
 *  SB_BAD_BUF_LEN     -- Greater than SB_MAX_ADD_INFO_LEN bytes passed for
 *                        the additional information length.
 *  SB_BAD_INBUF_LEN   -- Bad value specified for length of the shared secret.
 *  SB_BAD_LOCAL_KEY   -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY  -- Remote key in incorrect format.
 *  SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_dhSharedSecretWithAddInfo(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * Remote contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * Local contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const unsigned int addInfoLength,
    /* [input]
     * The length (in bytes) of addInfo.
     * Must be less than or equal to SB_MAX_ADD_INFO_LEN bytes.
     */
    const unsigned char addInfo[],
    /* [input]
     * Additional information of length addInfoLength.
     * Restricted to non-NULL values.
     */
    const unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * It is restricted to non-NULL values.
     */
);


/*---sb_dhRawSharedSecret()
 * Generate the shared secret from the local key and the remote key using the DH
 * key-exchange algorithm without hashing the result.
 *
 * Returns:
 *  SB_SUCCESS         -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY    -- NULL pointer passed for the local private key.
 *  SB_NO_REMOTE_KEY   -- NULL pointer passed for the remote public key.
 *  SB_NO_OUTBUF       -- NULL pointer passed for the shared secret buffer.
 *  SB_NO_OUTBUF_LEN   -- The shared secret buffer length is NULL.
 *  SB_BAD_OUTBUF_LEN  -- The shared secret buffer length is wrong.
 *  SB_BAD_LOCAL_KEY   -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY  -- Remote key in incorrect format.
 *  SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_dhRawSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * Remote contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * Local contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    unsigned int *secretLength,
    /* [input/output]
     * The length of the raw shared secret buffer as input.
     * The length of the raw shared secret as output.
     * The shared secret buffer must be equal to or larger than the length of
     * the raw shared secret, which is the x-coordinate of an elliptic curve
     * point.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * It is restricted to non-NULL values.
     */
);


/*---sb_dhPwdSharedSecret()
 * Generate the shared secret from the local key and the remote key using the DH
 * key-exchange algorithm.
 *
 * Returns:
 *    SB_SUCCESS            -- Operation succeeded.
 *    SB_NO_GLOBAL_DATA     -- NULL pointer passed for global data.
 *    SB_NOT_INITIALIZED    -- Global data structure has not been initialised.
 *    SB_NO_PROT_LOCAL_KEY  -- NULL pointer passed for the local protected key.
 *    SB_BAD_PROT_LOCAL_KEY -- protected local key in incorrect format.
 *    SB_BAD_PWD_LEN        -- The length of the password is out-of-range.
 *    SB_NO_PWD             -- NULL pointer passed for the password
 *    SB_NO_OUTBUF          -- NULL pointer passed for the shared secret buffer.
 *    SB_BAD_INBUF_LEN      -- Bad value specified for the length of the
 *                             shared secret.
 *    SB_BAD_REMOTE_KEY     -- Remote key in incorrect format.
 *    SB_NO_REMOTE_KEY      -- NULL pointer passed for the remote public key.
 *    SB_FAILURE            -- Failure in the underlying cryptosystem.
 */
int
sb_dhPwdSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * Remote contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const sb_ProtectedKey *protectedLocalKey,
    /* [input]
     * Local contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * This is the length of the password and it must be greater than zero
     * and less than or equal to SB_MAX_PWD_LEN.
     */
    const unsigned char password[],
    /* [input]
     * This is the password protecting the signing key. It is restricted to
     * non-null values.
     */
    unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * It is restricted to non-NULL values.
     */
);


/*---sb_dhPwdSharedSecretWithAddInfo()
 * Generate the shared secret from the local key, the remote key and the
 * additional information using the DH key-exchange algorithm.
 *
 * Returns:
 *    SB_SUCCESS            -- Operation succeeded.
 *    SB_NO_GLOBAL_DATA     -- NULL pointer passed for global data.
 *    SB_NOT_INITIALIZED    -- Global data structure has not been initialised.
 *    SB_NO_PROT_LOCAL_KEY  -- NULL pointer passed for the protected local key.
 *    SB_BAD_PROT_LOCAL_KEY -- Protected local key in incorrect format.
 *    SB_BAD_PWD_LEN        -- The length of the password is out-of-range.
 *    SB_NO_PWD             -- NULL pointer passed for the password
 *    SB_NO_OUTBUF          -- NULL pointer passed for the shared secret buffer.
 *    SB_NO_BUF             -- NULL pointer passed for the additional
 *                             information buffer.
 *    SB_BAD_BUF_LEN        -- Greater than SB_MAX_ADD_INFO_LEN bytes passed
 *                             for the additional information length.
 *    SB_BAD_INBUF_LEN      -- Bad value specified for the length of the shared
 *                             secret.
 *    SB_NO_REMOTE_KEY      -- NULL pointer passed for the remote public key.
 *    SB_BAD_REMOTE_KEY     -- Remote key in incorrect format.
 *    SB_FAILURE            -- Failure in the underlying cryptosystem.
 */
int
sb_dhPwdSharedSecretWithAddInfo(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * Remote contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const sb_ProtectedKey *protectedLocalKey,
    /* [input]
     * Local contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * This is the length of the password and it must be greater than zero
     * and less than or equal to SB_MAX_PWD_LEN.
     */
    const unsigned char password[],
    /* [input]
     * This is the password protecting the signing key. It is restricted to
     * non-null values.
     */
    const unsigned int addInfoLength,
    /* [input]
     * The length (in bytes) of addInfo.
     * Must be less than or equal to SB_MAX_ADD_INFO_LEN bytes.
     */
    const unsigned char addInfo[],
    /* [input]
     * Additional information of length addInfoLength.
     * Restricted to non-NULL values.
     */
    unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * It is restricted to non-NULL values.
     */
);


/*---sb_dhPwdRawSharedSecret()
 * Generate the shared secret from the local key and the remote key using
 * the Raw DH key-exchange algorithm.
 *
 * Returns:
 *    SB_SUCCESS            -- Operation succeeded.
 *    SB_NO_GLOBAL_DATA     -- NULL pointer passed for global data.
 *    SB_NOT_INITIALIZED    -- Global data structure has not been initialised.
 *    SB_NO_PROT_LOCAL_KEY  -- NULL pointer passed for the local protected key.
 *    SB_BAD_PROT_LOCAL_KEY -- protected local key in incorrect format.
 *    SB_BAD_PWD_LEN        -- The length of the password is out-of-range.
 *    SB_NO_PWD             -- NULL pointer passed for the password
 *    SB_NO_OUTBUF          -- NULL pointer passed for the shared secret buffer.
 *    SB_NO_OUTBUF_LEN      -- The shared secret buffer length is NULL.
 *    SB_BAD_INBUF_LEN      -- Bad value specified for the length of the
 *                             shared secret.
 *    SB_BAD_REMOTE_KEY     -- Remote key in incorrect format.
 *    SB_NO_REMOTE_KEY      -- NULL pointer passed for the remote public key.
 *    SB_FAILURE            -- Failure in the underlying cryptosystem.
 */
int
sb_dhPwdRawSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * Remote contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const sb_ProtectedKey *protectedLocalKey,
    /* [input]
     * Local contribution to the DH key-exchange.
     * It is restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * This is the length of the password and it must be greater than zero
     * and less than or equal to SB_MAX_PWD_LEN.
     */
    const unsigned char password[],
    /* [input]
     * This is the password protecting the signing key. It is restricted to
     * non-null values.
     */
    unsigned int *secretLength,
    /* [input/output]
     * The length of the raw shared secret buffer as input.
     * The length of the raw shared secret as output.
     * The shared secret buffer must be equal to or larger than the length of
     * the raw shared secret, which is the x-coordinate of an elliptic curve
     * point.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * It is restricted to non-NULL values.
     */
);



    /*
     * MQV Functions.
     */

/*---sb_mqvGenerateValues()
 * Generate the local and remote values required by the MQV key-exchange.
 *
 * Returns:
 *  SB_SUCCESS         -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA  -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY    -- NULL pointer passed for the local private key.
 *  SB_NO_REMOTE_KEY   -- NULL pointer passed for the remote public key.
 *  SB_FAILURE         -- Failure in the underlying cryptosystem.
 */
int
sb_mqvGenerateValues(
    void *globalData,
    /* [input]
     * The global data area.
     * It is restricted to non-NULL values.
     */
    sb_PublicKey *remoteKey,
    /* [output]
     * The remote value for the MQV key-exchange.
     * It is restricted to non-NULL values.
     */
    sb_PrivateKey *localKey
    /* [output]
     * The local value for the MQV key-exchange.
     * It is restricted to non-NULL values.
     */
);


/*---sb_mqvSharedSecret()
 * Generate the shared secret from the local key and the remote key using
 * the MQV key-exchange algorithm.
 *
 * Returns:
 *  SB_SUCCESS               -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA        -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED       -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY          -- NULL pointer passed for the local key.
 *  SB_NO_REMOTE_KEY         -- NULL pointer passed for the remote key.
 *  SB_NO_PRIVATE_KEY        -- NULL pointer passed for the local private key.
 *  SB_NO_PUBLIC_KEY         -- NULL pointer passed for the local public key.
 *  SB_NO_REMOTE_PUBLIC_KEY  -- NULL pointer passed for the remote public key.
 *  SB_NO_OUTBUF             -- NULL pointer passed for the shared secret
 *                              buffer.
 *  SB_BAD_INBUF_LEN         -- Bad value specified for length of the shared
 *                              secret.
 *  SB_BAD_LOCAL_KEY         -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY        -- Remote key in incorrect format.
 *  SB_BAD_REMOTE_PUBLIC_KEY -- Remote public key in incorrect format.
 *  SB_BAD_PUBLIC_KEY        -- Local external key in incorrect format.
 *  SB_BAD_LOCAL_PRIVATE_KEY -- Local private key in incorrect format.
 *  SB_FAILURE               -- Failure in the underlying cryptosystem.
 */
int
sb_mqvSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * The remote contribution generated by the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * The local contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remotePublicKey,
    /* [input]
     * The public key of the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localPrivateKey,
    /* [input]
     * The private key of the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *localExternalKey,
    /* [input]
     * The remote contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * Restricted to non-NULL values.
     */
);


/*---sb_mqvSharedSecretWithAddInfo()
 * Generate the shared secret from the local key, the remote key, and the
 * additional information using the MQV key-exchange algorithm.
 *
 * Returns:
 *  SB_SUCCESS              -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA       -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED      -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY         -- NULL pointer passed for the local key.
 *  SB_NO_REMOTE_KEY        -- NULL pointer passed for the remote key.
 *  SB_NO_PRIVATE_KEY       -- NULL pointer passed for the local private key.
 *  SB_NO_PUBLIC_KEY        -- NULL pointer passed for the local public key.
 *  SB_NO_REMOTE_PUBLIC_KEY -- NULL pointer passed for the remote public key.
 *  SB_NO_OUTBUF            -- NULL pointer passed for the shared secret buffer.
 *  SB_BAD_BUF_LEN          -- Bad value specified for length of the shared
 *                             secret.
 *  SB_NO_INBUF             -- NULL pointer passed for the additional
 *                             information.
 *  SB_BAD_INBUF_LEN        -- Greater than SB_MAX_ADD_INFO_LEN bytes passed
 *                             for the additional information length.
 *  SB_BAD_LOCAL_KEY        -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY       -- Remote key in incorrect format.
 *  SB_BAD_REMOTE_PUBLIC_KEY    -- Remote public key in incorrect format.
 *  SB_BAD_PUBLIC_KEY           -- Local external key in incorrect format.
 *  SB_BAD_LOCAL_PRIVATE_KEY    -- Local private key in incorrect format.
 *  SB_FAILURE              -- Failure in the underlying cryptosystem.
 */
int
sb_mqvSharedSecretWithAddInfo(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * The remote contribution generated by the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * The local contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remotePublicKey,
    /* [input]
     * The public key of the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localPrivateKey,
    /* [input]
     * The private key of the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *localExternalKey,
    /* [input]
     * The remote contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const unsigned int addInfoLength,
    /* [input]
     * The length of addInfo in bytes.
     * Must be less than or equal to SB_MAX_ADD_INFO_LEN.
     */
    const unsigned char addInfo[],
    /* [input]
     * Additional information of length addInfoLength.
     * Restricted to non-NULL values.
     */
    const unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * Restricted to non-NULL values.
     */
);


/*---sb_mqvRawSharedSecret()
 * Generate the raw shared secret from the local key and the remote key using
 * the MQV key-exchange algorithm.
 *
 * Returns:
 *  SB_SUCCESS               -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA        -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED       -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY          -- NULL pointer passed for the local key.
 *  SB_NO_REMOTE_KEY         -- NULL pointer passed for the remote key.
 *  SB_NO_PRIVATE_KEY        -- NULL pointer passed for the local private key.
 *  SB_NO_PUBLIC_KEY         -- NULL pointer passed for the local public key.
 *  SB_NO_REMOTE_PUBLIC_KEY  -- NULL pointer passed for the remote public key.
 *  SB_NO_OUTBUF_LEN         -- The shared secret buffer length is NULL.
 *  SB_BAD_OUTBUF_LEN        -- The shared secret buffer length is wrong.
 *  SB_NO_OUTBUF             -- NULL pointer passed for the shared secret
 *                              buffer.
 *  SB_NO_BUF                -- NULL pointer passed for the secret length.
 *  SB_BAD_BUF_LEN           -- Bad value specified for length of the shared
 *                              secret.
 *  SB_BAD_LOCAL_KEY         -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY        -- Remote key in incorrect format.
 *  SB_BAD_REMOTE_PUBLIC_KEY -- Remote public key in incorrect format.
 *  SB_BAD_PUBLIC_KEY        -- Local external key in incorrect format.
 *  SB_BAD_LOCAL_PRIVATE_KEY -- Local private key in incorrect format.
 *  SB_FAILURE               -- Failure in the underlying cryptosystem.
 */
int
sb_mqvRawSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * The remote contribution generated by the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * The local contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remotePublicKey,
    /* [input]
     * The public key of the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localPrivateKey,
    /* [input]
     * The private key of the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *localExternalKey,
    /* [input]
     * The remote contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    unsigned int *secretLength,
    /* [input/output]
     * The length of the raw shared secret buffer as input.
     * The length of the raw shared secret as output.
     * The shared secret buffer must be equal to or larger than the length of
     * the raw shared secret, which is the x-coordinate of an elliptic curve
     * point.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * Restricted to non-NULL values.
     */
);


/*---sb_mqvPwdSharedSecret()
 * Generate the shared secret from the local key and the remote key using
 * the MQV key-exchange algorithm with a password-protected private key.
 *
 * Returns:
 *  SB_SUCCESS               -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA        -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED       -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY          -- NULL pointer passed for the local key.
 *  SB_NO_REMOTE_KEY         -- NULL pointer passed for the remote key.
 *  SB_NO_PROT_PRIVATE_KEY   -- NULL pointer passed for the protected private
 *                              key.
 *  SB_BAD_PWD_LEN           -- Password length is zero or too large.
 *  SB_NO_PWD                -- NULL pointer passed for the password.
 *  SB_NO_PUBLIC_KEY         -- NULL pointer passed for the public key.
 *  SB_NO_OUTBUF             -- NULL pointer passed for the shared secret
 *                              buffer.
 *  SB_BAD_INBUF_LEN         -- Bad value specified for length of the shared
 *                              secret.
 *  SB_BAD_LOCAL_KEY         -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY        -- Remote key in incorrect format.
 *  SB_BAD_REMOTE_PUBLIC_KEY -- Remote public key in incorrect format.
 *  SB_BAD_PUBLIC_KEY        -- Local external key in incorrect format.
 *  SB_BAD_PROT_PRIVATE_KEY  -- Protected private key in incorrect format.
 *  SB_BAD_LOCAL_PRIVATE_KEY -- Local private key in incorrect format.
 *  SB_FAILURE               -- Failure in the underlying cryptosystem.
 */
int
sb_mqvPwdSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * The remote contribution generated by the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * The local contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remotePublicKey,
    /* [input]
     * The public key of the remote party.
     * Restricted to non-NULL values.
     */
    const sb_ProtectedKey *localProtectedPrivateKey,
    /* [input]
     * The password-protected private key of the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *localExternalKey,
    /* [input]
     * The remote contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password used to decrypt the protected private key.
     */
    const unsigned char password[],
    /* [input]
     * Password used to decrypt the protected private key.
     */
    const unsigned int secretLength,
    /* [input]
     * The length (in bytes) of the desired secret value.
     * Restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * Restricted to non-NULL values.
     */
);


/*---sb_mqvPwdSharedSecretWithAddInfo()
 * Generate the shared secret from the local key, the remote key, and the
 * additional information using the MQV key-exchange algorithm using a
 * password-protected private key.
 *
 * Returns:
 *  SB_SUCCESS               -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA        -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED       -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY          -- NULL pointer passed for the local key.
 *  SB_NO_REMOTE_KEY         -- NULL pointer passed for the remote key.
 *  SB_NO_PROT_PRIVATE_KEY   -- NULL pointer passed for the protected private
 *                              key.
 *  SB_BAD_PWD_LEN           -- Password length is zero or too large.
 *  SB_NO_PWD                -- NULL pointer passed for the password.
 *  SB_NO_PUBLIC_KEY         -- NULL pointer passed for the public key.
 *  SB_NO_OUTBUF             -- NULL pointer passed for the shared secret
 *                              buffer.
 *  SB_NO_BUF                -- NULL pointer passed for the shared secret
 *                              length.
 *  SB_BAD_BUF_LEN           -- Bad value specified for length of the shared
 *                              secret.
 *  SB_NO_INBUF              -- NULL pointer passed for the additinal
 *                              information.
 *  SB_BAD_INBUF_LEN         -- Greater than SB_MAX_ADD_INFO_LEN passed for
 *                              length of the additional information.
 *  SB_BAD_LOCAL_KEY         -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY        -- Remote key in incorrect format.
 *  SB_BAD_REMOTE_PUBLIC_KEY -- Remote public key in incorrect format.
 *  SB_BAD_PUBLIC_KEY        -- Local external key in incorrect format.
 *  SB_BAD_PROT_PRIVATE_KEY  -- Protected private key in incorrect format.
 *  SB_BAD_LOCAL_PRIVATE_KEY -- Local private key in incorrect format.
 *  SB_FAILURE               -- Failure in the underlying cryptosystem.
 */
int
sb_mqvPwdSharedSecretWithAddInfo(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * The remote contribution generated by the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * The local contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remotePublicKey,
    /* [input]
     * The public key of the remote party.
     * Restricted to non-NULL values.
     */
    const sb_ProtectedKey *localProtectedPrivateKey,
    /* [input]
     * The password-protected private key of the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *localExternalKey,
    /* [input]
     * The remote contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password used to decrypt the protected private key.
     */
    const unsigned char password[],
    /* [input]
     * Password used to decrypt the protected private key.
     */
    const unsigned int addInfoLength,
    /* [input]
     * The length of addInfo in bytes.
     * Must be less than or equal to SB_MAX_ADD_INFO_LEN.
     */
    const unsigned char addInfo[],
    /* [input]
     * Additional information of length addInfoLength.
     * Restricted to non-NULL values.
     */
    const unsigned int secretLength,
    /* [input]
     * The desired length (in bytes) of the shared secret.
     * It is restricted to non-zero values.
     * Must be shorter than or equal to SB_SHA1_MSG_DIG_LEN.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * The buffer must be able to hold at least secretLength bytes.
     * Restricted to non-NULL values.
     */
);


/*---sb_mqvPwdRawSharedSecret()
 * Generate the raw shared secret from the local key and the remote key using
 * the MQV key-exchange algorithm using a password-protected private key.
 *
 * Returns:
 *  SB_SUCCESS               -- Operation succeeded.
 *  SB_NO_GLOBAL_DATA        -- NULL pointer passed for global data.
 *  SB_NOT_INITIALIZED       -- Global data structure has not been initialised.
 *  SB_NO_LOCAL_KEY          -- NULL pointer passed for the local key.
 *  SB_NO_REMOTE_KEY         -- NULL pointer passed for the remote key.
 *  SB_NO_PROT_PRIVATE_KEY   -- NULL pointer passed for the protected private
 *                              key.
 *  SB_BAD_PWD_LEN           -- Password length is zero or too large.
 *  SB_NO_PWD                -- NULL pointer passed for the password.
 *  SB_NO_PUBLIC_KEY         -- NULL pointer passed for the public key.
 *  SB_NO_OUTBUF_LEN         -- The shared secret buffer length is NULL.
 *  SB_BAD_OUTBUF_LEN        -- The shared secret buffer length is wrong.
 *  SB_NO_OUTBUF             -- NULL pointer passed for the shared secret
 *                              buffer.
 *  SB_NO_BUF                -- NULL pointer passed for the shared secret
 *                              length.
 *  SB_BAD_BUF_LEN           -- Bad value specified for length of the shared
 *                              secret.
 *  SB_BAD_LOCAL_KEY         -- Local key in incorrect format.
 *  SB_BAD_REMOTE_KEY        -- Remote key in incorrect format.
 *  SB_BAD_REMOTE_PUBLIC_KEY -- Remote public key in incorrect format.
 *  SB_BAD_PUBLIC_KEY        -- Local external key in incorrect format.
 *  SB_BAD_PROT_PRIVATE_KEY  -- Protected private key in incorrect format.
 *  SB_BAD_LOCAL_PRIVATE_KEY -- Local private key in incorrect format.
 *  SB_FAILURE               -- Failure in the underlying cryptosystem.
 */
int
sb_mqvPwdRawSharedSecret(
    void *globalData,
    /* [input]
     * The global data area.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remoteKey,
    /* [input]
     * The remote contribution generated by the remote party.
     * Restricted to non-NULL values.
     */
    const sb_PrivateKey *localKey,
    /* [input]
     * The local contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *remotePublicKey,
    /* [input]
     * The public key of the remote party.
     * Restricted to non-NULL values.
     */
    const sb_ProtectedKey *localProtectedPrivateKey,
    /* [input]
     * The password-protected private key of the local party.
     * Restricted to non-NULL values.
     */
    const sb_PublicKey *localExternalKey,
    /* [input]
     * The remote contribution generated by the local party.
     * Restricted to non-NULL values.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password used to decrypt the protected private key.
     */
    const unsigned char password[],
    /* [input]
     * Password used to decrypt the protected private key.
     */
    unsigned int *secretLength,
    /* [input/output]
     * The length of the raw shared secret buffer as input.
     * The length of the raw shared secret as output.
     * The shared secret buffer must be equal to or larger than the length of
     * the raw shared secret, which is the x-coordinate of an elliptic curve
     * point.
     */
    unsigned char sharedSecret[]
    /* [output]
     * The shared secret.
     * Restricted to non-NULL values.
     */
);


#endif /*SBEXC_H*/
