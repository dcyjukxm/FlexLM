/*+
 * sbdsa.h - The Application Programmer's Interface
 *              -- Digital Signature Algorithm Module
 *
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB DSA module.
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
#ifndef SBDSA_H
#define SBDSA_H    1

/* include files */
#include "sbglb.h"
#include "sbrc.h"
#include "sbkey.h"
#include "sbsha.h"

/*
 * Constant Values
 */
/*
 * SB_ECDSA_CONTEXT_SIZE
 *   --This macro defines the maximum length of an ECDSA context.
 */
#define SB_ECDSA_CONTEXT_SIZE sizeof(sb_HashContext)

/*
 * SB_ECDSA_SIGNATURE_SIZE
 *   --This macro defines the maximum length of a signature.
 *
 * SB_FIELD_ELEMENT_OCTETS is defined in sbglb.h
 */

#define SB_ECDSA_SIGNATURE_SIZE (2 * SB_FIELD_ELEMENT_OCTETS)


/* Public Types Definitions */

/*--sb_SignContextECDSA and sb_VerifyContextECDSA
 *   --This data type defines an ECDSA context. The size is
 *     the number of bytes used in the signature.
 *
 *
 * The ECDSA contexts sb_SignContextECDSA and sb_VerifyContextECDSA
 * supports multipart operations,
 * that is operations in manner Begin-Continue-End.
 * It contains intermediate results for multipart operations.  The size of
 * the ECDSA context
 * is taken long enough to accomodate the intermediate data required by
 * message hashing mechanism used internally.
 * The length is used to indicate the  the fact that the operation has been
 * properly initialized. Whenever any multipart operation
 * fails the context data structure gets zerofilled.
 *
 */
typedef struct sb_SignContextECDSA
{
    unsigned int length;
    unsigned char context[SB_ECDSA_CONTEXT_SIZE];
}sb_SignContextECDSA;

typedef sb_SignContextECDSA sb_VerifyContextECDSA;

/*
 * --sb_SignatureECDSA
 *   --This data type defines an ECDSA signature. The size is
 *     the number of bytes used in the signature.

 *
 * The ECDSA signature is created from a message and a private key and serves
 * authentication purpose in EC cryptosystem. It is verified via calculation
 * involving an original message and a public key.
 *
 * The length of the
 * array signature is twice as long as
 * SB_FIELD_ELEMENT_OCTETS, a maximum ( curve independent) size
 * to accomodate  two signature components.
 * However, the actual size of data is found in size member.
 * The size of every component is fixed to size/2.
 *
 */
typedef struct sb_SignatureECDSA {
    unsigned int size;
    unsigned char signature[SB_ECDSA_SIGNATURE_SIZE];
} sb_SignatureECDSA;

/* Function Prototypes */

/*---sb_ecdsaSignBegin()
 * initialize signing with ECDSA by establishing an ECDSA space  context.
 *
 * Notes:
 *     (1) global structure has to be initialized.
 *     (2) ECDSA context is not supposed to be parsed by the user.
 *     (3) This function has to be called before sb_ecdsaSign or
 *         sb_ecdsaSignEnd
 *
 * Returns:
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NO_CONTEXT NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaSignBegin(
    void * globalData,
        /* [input]
         * The global data area. Must not be NULL.
         * Has to be initialized.
         */
    sb_SignContextECDSA *context
        /* [output]
         * The current ECDSA context. Must not be NULL.
         */
);
/*---sb_ecdsaSign()
 * Continue signing with ECDSA within the current ECDSA context.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *     (2) Function  sb_ecdsaSignBegin() must be called first.
 *
 * Returns:
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NO_CONTEXT NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_BAD_CONTEXT context actually does not contain necessary information.
 * SB_NO_BUF NULL pointer passed for message buffer.
 * SB_BAD_BUF_LEN zero message buffer length is passed.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 */
int
sb_ecdsaSign(
    void * globalData,
        /* [input]
         * The global data area. Must not be NULL
         * Has to be initialized at the library init time.
         */
    unsigned int  msgBlockLength,
        /* [input]
         * Length (in bytes) of the message block
         * Not allowed to be zero.
         */
    const unsigned char msgBlock[],
        /* [input]
         * Message block that contributes to the signature.
         * Must not be NULL.
         * The pointer has to be
         * incremented with respect to
         * previously processed block length.
         */
    sb_SignContextECDSA *context
        /* [input output]
         * The current ECDSA context. Must not be NULL.
         * It gets updated so to keep track of function calls
         * proper order.
         */
)
;
/*---sb_ecdsaSignEnd()
 * End signing with ECDSA within the current ECDSA context
 * by returning the signature calculated with the given private key
 * and clearing the context.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *     (2) sb_ecdsaSignBegin() call has to precede.
 *     (3) No intermediate operation part can be called after this function
 *         call unless another operation is initialized via call to Begin part
 *
 * Returns:
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NO_CONTEXT NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_BAD_CONTEXT context actually does not contain necessary information.
 * SB_NO_ECDSA_SIGNATURE NULL pointer passed for signature
 * SB_BAD_PRIVATE_KEY wrong private key format
 * SB_NO_PRIVATE_KEY NULL pointer is passed for private key
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaSignEnd(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    const sb_PrivateKey *privateKey,
        /* [input]
         * Pointer to an unprotected private key.
         * Must not be NULL.
         */
    sb_SignatureECDSA *signature,
        /* [output]
         * The resulting signature. Must not be NULL.
         */
    sb_SignContextECDSA *context
        /* [input output]
         * The current ECDSA context. Must not be NULL.
         */
);
/*---sb_ecdsaPwdSignEnd()
 * Password-Protected version of sb_ecdsaSignEnd(). It
 * only differs from sb_ecdsaSignEnd() in the way the Private Key is
 * passed.
 * It completes signing with ECDSA within the current ECDSA context
 * by returning the signature calculated with the given PASSWORD-PROTECTED private key
 * and clearing the context.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *     (2) sb_ecdsaSignBegin() call has to precede.
 *     (3) User must provide a correct password.
 *     (4) No intermediate operation part can be called after this function
 *         call unless another operation is initialized via call to Begin part
 *
 * Returns:
 *
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NO_CONTEXT NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_BAD_CONTEXT context actually does not contain necessary information.
 * SB_NO_ECDSA_SIGNATURE NULL pointer passed for signature.
 * SB_BAD_PROT_PRIVATE_KEY wrong password protected private key format.
 * SB_NO_PROT_PRIVATE_KEY  NULL pointer passed as password protected private key
 * SB_NO_PWD NULL pointer passed for password.
 * SB_BAD_PWD_LEN the length of the password is out-of-range.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaPwdSignEnd(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    unsigned int pwdLength,
        /* [input]
         * Length of the password.
         * Not allowed to be zero.
         * The  length is limited to SB_MAX_PWD_LEN bytes.
         *
         */
    const unsigned char password[],
        /* [input]
         * Password (assumed of length pwdLength).
         * Must not be NULL.
         */
    const sb_ProtectedKey *protectedPrivateKey,
        /* [input]
         * Pointer to a protected private key.
         * Must not be NULL.
         */
    sb_SignatureECDSA *signature,
        /* [output]
         * The resulting signature. Must not be NULL.
         */
    sb_SignContextECDSA *context
        /* [input output]
         * The current ECDSA context. Must not be NULL.
         */
);
/*---sb_ecdsaNoHashSign()
 * ECDSA signature generation.  The input message digest is used in
 * the signature generation.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *
 * Returns:
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_NO_ECDSA_SIGNATURE NULL pointer passed for signature
 * SB_BAD_PRIVATE_KEY wrong private key format
 * SB_NO_PRIVATE_KEY NULL pointer is passed for private key
 * SB_NO_BUF NULL pointer passed for message digest buffer.
 * SB_BAD_BUF_LEN message digest buffer length is zero or greater than
 *                SB_SHA1_MSG_DIG_LEN.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaNoHashSign(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    const sb_PrivateKey *privateKey,
        /* [input]
         * Pointer to an unprotected private key.
         * Must not be NULL.
         */
    const unsigned int msgDigestLength,
        /* [input]
         * Length (in bytes) of the message digest.
         * Not allowed to be zero or greater than SB_SHA1_MSG_DIG_LEN.
         */
    const unsigned char msgDigest[],
        /* [input]
         * Message digest that contributes to the signature.
         * Must not be NULL.
         */
    sb_SignatureECDSA *signature
        /* [output]
         * The resulting signature. Must not be NULL.
         */
);
/*---sb_ecdsaNoHashPwdSign()
 * Password-Protected version of sb_ecdsaNoHashSign(). It
 * only differs from sb_ecdsaNoHashSign() in that the private key is
 * password protected.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *     (2) User must provide a correct password.
 *
 * Returns:
 *
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_NO_ECDSA_SIGNATURE NULL pointer passed for signature.
 * SB_BAD_PROT_PRIVATE_KEY wrong password protected private key format.
 * SB_NO_PROT_PRIVATE_KEY  NULL pointer passed as password protected private key
 * SB_NO_PWD NULL pointer passed for password.
 * SB_BAD_PWD_LEN the length of the password is out-of-range.
 * SB_NO_BUF NULL pointer passed for message digest buffer.
 * SB_BAD_BUF_LEN message digest buffer length is zero or greater than
 *                SB_SHA1_MSG_DIG_LEN.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaNoHashPwdSign(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    const unsigned int pwdLength,
        /* [input]
         * Length of the password.
         * Not allowed to be zero.
         * The  length is limited to SB_MAX_PWD_LEN bytes.
         *
         */
    const unsigned char password[],
        /* [input]
         * Password (assumed of length pwdLength).
         * Must not be NULL.
         */
    const sb_ProtectedKey *protectedPrivateKey,
        /* [input]
         * Pointer to a protected private key.
         * Must not be NULL.
         */
    const unsigned int msgDigestLength,
        /* [input]
         * Length (in bytes) of the message digest.
         * Not allowed to be zero or greater than SB_SHA1_MSG_DIG_LEN.
         */
    const unsigned char msgDigest[],
        /* [input]
         * Message digest that contributes to the signature.
         * Must not be NULL.
         */
    sb_SignatureECDSA *signature
        /* [output]
         * The resulting signature. Must not be NULL.
         */
);

/*---sb_ecdsaVerifyBegin()
 * Verify a signature with ECDSA.
 * Establish a ECDSA context.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *     (2) function  sb_ecdsaVerifyBegin() must be called first
 *     (3) ECDSA context is not supposed to be parsed by the user.
 *
 * Returns:
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NO_CONTEXT NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaVerifyBegin(
    void * globalData,
        /* [input]
         * The global data area. Must not be NULL
         * Has to be initialized.
         */
    sb_VerifyContextECDSA *context
        /* [input output]
         * The current ECDSA context. Must not be NULL.
         * It gets updated so to keep track of function calls
         * proper order.
         */
);
/*---sb_ecdsaVerify()
 * Continue verification with ECDSA within the current ECDSA context.
 *
 * Notes:
 *     (1) ECDSA context is not supposed to be parsed by the user.
 *     (2) This function has to be called before sb_ecdsaVerify
 *
 * Returns:
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA NULL pointer passed for global data.
 * SB_NO_CONTEXT the NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_BAD_CONTEXT context actually does not contain necessary information.
 * SB_NO_BUF the NULL pointer passed for message buffer
 * SB_BAD_BUF_LEN zero message buffer length is passed
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 *
 */
int
sb_ecdsaVerify(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    unsigned int msgBlockLength,
        /* [input]
         * Length (in bytes) of the message block processed.
         * Not allowed to be zero.
         */
    const unsigned char msgBlock[],
        /* [input]
         * Message block that contributes to the signature.
         * Must not be NULL.
         * The pointer has to be
         * incremented with respect to
         * previously processed block length.
         */
    sb_VerifyContextECDSA *context
        /* [input output]
         * The current ECDSA context. Must not be NULL.
         * It gets updated so to keep track of function calls
         * proper order.
         */
);
/*---sb_ecdsaVerifyEnd()
 * End verification with ECDSA indicating whether the verification passed
 * or succeeded with the given public key and clear the context.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *     (2) sb_ecdsaVerifyBegin() call has to precede.
 *     (3) No intermediate operation part can be called after this function
 *         call unless another operation is initialized via call to Begin part
 *
 *
 * Returns: zero on success or error message otherwise
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA the NULL pointer passed for global data.
 * SB_NO_CONTEXT NULL pointer passed for context structure.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_BAD_CONTEXT context actually does not contain necessary information.
 * SB_NO_PUBLIC_KEY the NULL pointer passed for public key.
 * SB_NO_ECDSA_SIGNATURE the NULL pointer passed for signature
 * SB_BAD_PUBLIC_KEY wrong public key format.
 * SB_BAD_ECDSA_SIGNATURE wrong signature format
 * SB_NO_FLAG the NULL is passed for pointer to result
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 */
int
sb_ecdsaVerifyEnd(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    const sb_PublicKey *publicKey,
        /* [input]
         * Pointer to a  public key.
         * Must not be NULL.
         */
    const sb_SignatureECDSA *signature,
        /* [input]
         * Pointer to signature to be verified.
         * Must not be NULL.
         */
    sb_VerifyContextECDSA *context,
        /* [input output]
         * The current ECDSA context. Must not be NULL.
         */
    int *result
        /* [output]
         * Pointer to result of the verification.
         * Must not be NULL.
         */
   )

;
/*---sb_ecdsaNoHashVerify()
 * ECDSA signature verification.  The input message digest is used in
 * the signature generation.
 *
 * Notes:
 *     (1) Global Data has to be initialized.
 *
 *
 * Returns: zero on success or error message otherwise
 * SB_SUCCESS operation completed successfully.
 * SB_FAILURE ECS module operation completed unsuccessfully.
 * SB_NO_GLOBAL_DATA the NULL pointer passed for global data.
 * SB_NOT_INITIALIZED the library has not been initialized.
 * SB_NO_PUBLIC_KEY the NULL pointer passed for public key.
 * SB_NO_ECDSA_SIGNATURE the NULL pointer passed for signature
 * SB_BAD_PUBLIC_KEY wrong public key format.
 * SB_BAD_ECDSA_SIGNATURE wrong signature format
 * SB_NO_FLAG the NULL is passed for pointer to result
 * SB_NO_BUF NULL pointer passed for message digest buffer.
 * SB_BAD_BUF_LEN message digest buffer length is zero or greater than
 *                SB_SHA1_MSG_DIG_LEN.
 * SB_NOT_IMPLEMENTED the function is excluded from the library.
 */
int
sb_ecdsaNoHashVerify(
    void * globalData,
        /* [input]
         * The global data area.
         * Has to be initialized.
         */
    const sb_PublicKey *publicKey,
        /* [input]
         * Pointer to a  public key.
         * Must not be NULL.
         */
    const sb_SignatureECDSA *signature,
        /* [input]
         * Pointer to signature to be verified.
         * Must not be NULL.
         */
    const unsigned int msgDigestLength,
        /* [input]
         * Length (in bytes) of the message digest.
         * Not allowed to be zero or greater than SB_SHA1_MSG_DIG_LEN.
         */
    const unsigned char msgDigest[],
        /* [input]
         * Message digest that contributes to the signature.
         * Must not be NULL.
         */
    int *result
        /* [output]
         * Pointer to result of the verification.
         * Must not be NULL.
         */
   )

;


#endif /*SBDSA_H*/
