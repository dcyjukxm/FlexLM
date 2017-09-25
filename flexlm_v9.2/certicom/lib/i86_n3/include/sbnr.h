/*+
 * sbnr.h - The Application Programmer's Interface
 *            -- Nyberg-Rueppel Elliptic Curve Signature Scheme Module
 * 
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB NR module.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes: None.
 *
* 
* 
* Copyright Certicom Corp., 1996. All rights reserved.
-*/
#ifndef SBNR_H
#define SBNR_H    1

/*=== Include Files =========================================================*/
#include "sbrc.h"
#include "sbkey.h"
#include "sbglb.h"

/*=== Constant Values =======================================================*/
    /*---SB_ECNR_SIGN_CONTEXT_LENGTH
     * The maximum length of a signing context.
     */
#define SB_ECNR_SIGN_CONTEXT_LENGTH 100
    /*---SB_ECNR_SIGNATURE_LENGTH
     * The maximum length of a signature.
     */
#define SB_ECNR_SIGNATURE_LENGTH (2 * SB_FIELD_SIZE_OCTETS)
    /*---SB_ECNR_VERIFY_CONTEXT_LENGTH
     * The maximum length of a verification context.
     */
#define SB_ECNR_VERIFY_CONTEXT_LENGTH 100

/*=== Data Type Definitions =================================================*/
    /*---SignContextECNR
     * The signing context. Do not modify the context.
     */
typedef struct sb_SignContextECNR {
    unsigned char context[ SB_ECNR_SIGN_CONTEXT_LENGTH ];
} sb_SignContextECNR;
    /*---sb_SignatureECNR
     * Defines a Nyberg-Rueppel signature. The size is the number of valid
     * bytes in the signature.
     */
typedef struct sb_SignatureECNR {
    unsigned int size;
    unsigned char signature[ SB_ECNR_SIGNATURE_LENGTH ];
} sb_SignatureECNR;
    /*---sb_VerifyContextECNR
     * Defines a verification context. Do not modify the context.
     */
typedef struct sb_VerifyContextECNR {
    unsigned char context[ SB_ECNR_VERIFY_CONTEXT_LENGTH ];
} sb_VerifyContextECNR;

/*=== Function Prototypes ===================================================*/
/*---sb_ecnrSignBegin()
 * Begin signing with EC-NR by establishing a signing context.
 *
 * Notes:
 *   (1) The signing context is zero-filled whenever an error occurs.
 *
 * Returns:
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_CONTEXT     --The signing context is null.
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecnrSignBegin(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    sb_SignContextECNR *signContext
        /* [output]
         * This is the signing context. It is restricted to non-null values.
         */
);
/*---sb_ecnrSign()
 * Continue signing with EC-NR within the current signing context.
 *
 * Notes:
 *   (1) The signing context is zero-filled whenever an error occurs.
 *
 * Returns:
 *    SB_BAD_BUF_LEN    --The message buffer length is zero.
 *    SB_BAD_CONTEXT    --The signing context is invalid.
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_BUF         --The message buffer is null.
 *    SB_NO_CONTEXT     --The signing context is null.
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecnrSign(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    const unsigned int msgBlkLen,
        /* [input]
         * Length of the message block. It must be greater than zero.
         */
    const unsigned char msgBlk[],
        /* [input]
         * Message block that contributes to the signature. It is restricted to
         * non-null values.
         */
    sb_SignContextECNR *signContext
        /* [input/output]
         * This is the signing context. It is restricted to initialized
         * non-null values.
         */
);
/*---sb_ecnrSignEnd()
 * End signing with EC-NR within the current signing context returning the
 * signature formed with the passed private key.
 *
 * Notes:
 *   (1) The signing context is zero-filled on return from this function.
 *   (2) The private key is in the clear.
 *   (3) The signature is undefined whenever an error occurs.
 *   (4) Using a signing key created using global data different from the
 *       global data provide to the function creates an unverifiable signature.
 *
 * Returns:
 *    SB_BAD_CONTEXT      --The signing context is invalid.
 *    SB_BAD_PRIVATE_KEY  --The private key is invalid.
 *    SB_FAILURE          --The operation failed.
 *    SB_NO_CONTEXT       --The signing context is null.
 *    SB_NO_GLOBAL_DATA   --The global data is null.
 *    SB_NO_PRIVATE_KEY   --The signing key is null.
 *    SB_NO_ECNR_SIGNATURE--The signature is null.
 *    SB_NOT_INITIALIZED  --The global data is not initialized.
 *    SB_SUCCESS          --The operation completed successfully.
 */
int
sb_ecnrSignEnd(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    const sb_PrivateKey *signingKey,
        /* [input]
         * The unprotected signing key. It is restricted to non-null values.
         */
    sb_SignContextECNR *signContext,
        /* [input/output]
         * This is the signing context. It is restricted to initialized
         * non-null values.
         */
    sb_SignatureECNR *signatureNR
        /* [output]
         * The resulting signature. It is restricted to non-null values.
         */
);
/*---sb_ecnrPwdSignEnd()
 * End signing with EC-NR. Return a signature calculated with the given
 * password-protected private key. (The password must be passed into the
 * function.)
 *
 * Notes:
 *   (1) The signing context is zero-filled on return from this function.
 *   (2) The private key is password-protected.
 *   (3) The signature is undefined whenever an error occurs.
 *   (4) Using a signing key created using global data different from the
 *       global data provide to the function creates an unverifiable signature.
 *
 * Returns:
 *    SB_BAD_CONTEXT         --The signing context is invalid.
 *    SB_BAD_PROT_PRIVATE_KEY--The protected signing key is bad.
 *    SB_BAD_PWD_LEN         --The length of the password is out-of-range.
 *    SB_FAILURE             --The operation failed.
 *    SB_NO_CONTEXT          --The signing context is null.
 *    SB_NO_GLOBAL_DATA      --The global data is null.
 *    SB_NO_PROT_PRIVATE_KEY --The protected signing key is null.
 *    SB_NO_PWD              --The password is null.
 *    SB_NO_ECNR_SIGNATURE   --The signature is null.
 *    SB_NOT_INITIALIZED     --The global data is not initialized.
 *    SB_SUCCESS             --The operation completed successfully.
 */
int
sb_ecnrPwdSignEnd(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
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
    const sb_ProtectedKey *protectedSigningKey,
        /* [input]
         * The protected signing key. It is restricted to non-null values.
         */
    sb_SignContextECNR *signContext,
        /* [input/output]
         * This is the signing context. It is restricted to initialized
         * non-null values.
         */
    sb_SignatureECNR *signatureNR
        /* [output]
         * The resulting signature. It is restricted to non-null values.
         */
);
/*---sb_ecnrNoHashSign()
 *
 * Notes:
 *   (1) The private key is in the clear.
 *   (2) The signature is undefined whenever an error occurs.
 *   (3) Using a signing key created using global data different from the
 *       global data provide to the function creates an unverifiable signature.
 *
 * Returns:
 *    SB_BAD_BUF_LEN      --The message digest buffer length is zero or greater than
 *                        -- SB_DIGEST_LENGTH.
 *    SB_BAD_PRIVATE_KEY  --The private key is invalid.
 *    SB_FAILURE          --The operation failed.
 *    SB_NO_BUF           --The message digest buffer is null.
 *    SB_NO_GLOBAL_DATA   --The global data is null.
 *    SB_NO_PRIVATE_KEY   --The signing key is null.
 *    SB_NO_ECNR_SIGNATURE--The signature is null.
 *    SB_NOT_INITIALIZED  --The global data is not initialized.
 *    SB_SUCCESS          --The operation completed successfully.
 */
int
sb_ecnrNoHashSign(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    const sb_PrivateKey *signingKey,
        /* [input]
         * The unprotected signing key. It is restricted to non-null values.
         */
    const unsigned int msgDigLen,
        /* [input]
         * Length of the message digest. It must be greater than zero and less
         * than or equal to SB_DIGEST_LENGTH.
         */
    const unsigned char msgDig[],
        /* [input]
         * Message digest that contributes to the signature. It is restricted to
         * non-null values and less than or equal to SB_DIGEST_LENGTH.
         */
    sb_SignatureECNR *signatureNR
        /* [output]
         * The resulting signature. It is restricted to non-null values.
         */
);
/*---sb_ecnrNoHashPwdSign()
 * Return a signature calculated with the given password-protected private key.
 * (The password must be passed into the function.)
 *
 * Notes:
 *   (1) The private key is password-protected.
 *   (2) The signature is undefined whenever an error occurs.
 *   (3) Using a signing key created using global data different from the
 *       global data provide to the function creates an unverifiable signature.
 *
 * Returns:
 *    SB_BAD_BUF_LEN         --The message digest length is zero or greater than
 *                           -- SB_DIGEST_LENGTH.
 *    SB_BAD_PROT_PRIVATE_KEY--The protected signing key is bad.
 *    SB_BAD_PWD_LEN         --The length of the password is out-of-range.
 *    SB_FAILURE             --The operation failed.
 *    SB_NO_BUF              --The message digest buffer is null.
 *    SB_NO_GLOBAL_DATA      --The global data is null.
 *    SB_NO_PROT_PRIVATE_KEY --The protected signing key is null.
 *    SB_NO_PWD              --The password is null.
 *    SB_NO_ECNR_SIGNATURE   --The signature is null.
 *    SB_NOT_INITIALIZED     --The global data is not initialized.
 *    SB_SUCCESS             --The operation completed successfully.
 */
int
sb_ecnrNoHashPwdSign(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
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
    const sb_ProtectedKey *protectedSigningKey,
        /* [input]
         * The protected signing key. It is restricted to non-null values.
         */
    const unsigned int msgDigLen,
        /* [input]
         * Length of the message digest. It must be greater than zero and less
         * than or equal to SB_DIGEST_LENGTH.
         */
    const unsigned char msgDig[],
        /* [input]
         * Message digest that contributes to the signature. It is restricted to
         * non-null values.
         */
    sb_SignatureECNR *signatureNR
        /* [output]
         * The resulting signature. It is restricted to non-null values.
         */
);
/*---sb_ecnrVerifyBegin()
 * Verify a signature with EC-NR. Establish a verification context.
 *
 * Notes:
 *   (1) The verification context is zero-filled whenever an error occurs.
 *
 * Returns:
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_CONTEXT     --The context is null.
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecnrVerifyBegin(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    sb_VerifyContextECNR *verifyContext
        /* [output]
         * This is the verification context. It is restricted to non-null
         * values.
         */
);
/*---sb_ecnrVerify()
 *  Verify with EC-NR within the current verification context.
 *
 * Notes:
 *   (1) The verification context is zero-filled whenever an error occurs.
 *
 * Returns:
 *    SB_BAD_BUF_LEN    --The message buffer length is zero.
 *    SB_BAD_CONTEXT    --The verification context is not initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_NO_CONTEXT     --The verification context is null.
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NO_BUF         --The message buffer is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecnrVerify(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    const unsigned int msgBlkLen,
        /* [input]
         * Length of the message block. It must be greater than zero.
         */
    const unsigned char msgBlk[],
        /* [input]
         * Message block that contributes to the verification of the signature.
         * It is restricted to non-null values.
         */
    sb_VerifyContextECNR *verifyContext
        /* [input/output]
         * This is the verification context. It is restricted to initialized
         * non-null values.
         */
);
/*---sb_ecnrVerifyEnd()
 * End verification with EC-NR indicating whether the verification passed
 * or succeeded with the passed public key and clears the context.
 *
 * Notes:
 *   (1) The verification context is zero-filled on return from this function.
 *   (2) The verification result is undefined whenever an error occurs; it is
 *       non-zero whenever the signature verifies; otherwise it is zero.
 *   (3) Verification of signatures created with a crypto-system other than the
 *       one provided to this function are not verifiable.
 *
 * Returns:
 *    SB_BAD_CONTEXT       --The verification context is not initialized.
 *    SB_BAD_ECNR_SIGNATURE--The signature is invalid.
 *    SB_FAILURE           --The operation failed.
 *    SB_NO_CONTEXT        --The verification context is null.
 *    SB_NO_GLOBAL_DATA    --The global data is null.
 *    SB_NO_FLAG           --The verification result is null.
 *    SB_NO_PUBLIC_KEY     --The verification key is null.
 *    SB_BAD_PUBLIC_KEY    --The wrong verification key.
 *    SB_NO_ECNR_SIGNATURE --The signature is null.
 *    SB_NOT_INITIALIZED   --The global data is not initialized.
 *    SB_SUCCESS           --The operation completed successfully.
 */
int
sb_ecnrVerifyEnd(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    const sb_PublicKey *verificationKey,
        /* [input]
         * This is the verification key. It is restricted to non-null values.
         */
    const sb_SignatureECNR *signatureNR,
        /* [input]
         * Signature to be verified. It is restricted to non-null values.
         */
    sb_VerifyContextECNR *verifyContext,
        /* [input/output]
         * This is the verification context. It is restricted to initialized
         * non-null values.
         */
    int *verificationResult
        /* [output]
         * This is the result of the verifying the signature. It is restricted
         * to non-null values.
         */
);
/*---sb_ecnrNoHashVerify()
 * Verification with EC-NR indicating whether the verification passed
 * or succeeded with the passed public key.  The message digest is used.
 *
 * Notes:
 *   (1) The verification result is undefined whenever an error occurs; it is
 *       non-zero whenever the signature verifies; otherwise it is zero.
 *   (2) Verification of signatures created with a crypto-system other than the
 *       one provided to this function are not verifiable.
 *
 * Returns:
 *    SB_BAD_BUF_LEN       --The message digest length is zero or greater than
 *                         -- SB_DIGEST_LENGTH.
 *    SB_BAD_ECNR_SIGNATURE--The signature is invalid.
 *    SB_BAD_PUBLIC_KEY    --The verification key is invalid.
 *    SB_FAILURE           --The operation failed.
 *    SB_NO_BUF            --The message digest is null.
 *    SB_NO_GLOBAL_DATA    --The global data is null.
 *    SB_NO_FLAG           --The verification result is null.
 *    SB_NO_PUBLIC_KEY     --The verification key is null.
 *    SB_BAD_PUBLIC_KEY    --The wrong verification key.
 *    SB_NO_ECNR_SIGNATURE --The signature is null.
 *    SB_NOT_INITIALIZED   --The global data is not initialized.
 *    SB_SUCCESS           --The operation completed successfully.
 */
int
sb_ecnrNoHashVerify(
    void *globalData,
        /* [input]
         * This is the global data. It is restricted to initialized non-null
         * values.
         */
    const sb_PublicKey *verificationKey,
        /* [input]
         * This is the verification key. It is restricted to non-null values.
         */
    const sb_SignatureECNR *signatureNR,
        /* [input]
         * Signature to be verified. It is restricted to non-null values.
         */
    const unsigned int msgDigLen,
        /* [input]
         * Length of the message digest. It must be greater than zero and less
         * than or equal to SB_DIGEST_LENGTH.
         */
    const unsigned char msgDig[],
        /* [input]
         * Message digest that contributes to the signature. It is restricted to
         * non-null values and less than or equal to SB_DIGEST_LENGTH.
         */
    int *verificationResult
        /* [output]
         * This is the result of the verifying the signature. It is restricted
         * to non-null values.
         */
);

#endif /*SBNR_H*/
