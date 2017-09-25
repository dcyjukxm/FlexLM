/*+
 * sbeces.h - The Application Programmer's Interface
 *           -- ECES Encryption and Decryption Module
 *
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB ENC module.
 *
 *        sb_ecesEncryptBegin()
 *        sb_ecesBREncryptBegin()
 *        sb_ecesEncrypt()
 *        sb_ecesBREncrypt()
 *        sb_ecesEncryptEnd()
 *        sb_ecesBREncryptEnd()
 *        sb_ecesDecryptBegin()
 *        sb_ecesBRDecryptBegin()
 *        sb_ecesPwdDecryptBegin()
 *        sb_ecesBRPwdDecryptBegin()
 *        sb_ecesDecrypt()
 *        sb_ecesBRDecrypt()
 *        sb_ecesDecryptEnd()
 *        sb_ecesBRDecryptEnd()
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
#ifndef SBECES_H
#define SBECES_H    1

#include "sbglb.h"
#include "sbkey.h"
#include "sbsha.h"


#define ENC_CONTEXT_SIZE 88


typedef struct {
    unsigned char sbArray[ENC_CONTEXT_SIZE];
} sb_EncContext;

/*--sb_ecesEncryptBegin()
 * The ECES encryption algorithm.
 * If an error occurs, the key buffer and key buffer length are undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PUBLIC_KEY  --The public key buffer is null.
 *    SB_BAD_PUBLIC_KEY --The public key is invalid.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_OUTBUF_LEN --The key buffer length is too short.
 *    SB_NO_LEN_PTR     --The key buffer length pointer is null.
 *    SB_NO_OUTBUF      --The key buffer pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesEncryptBegin(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * Recipient's public key.
     */
    sb_EncContext *ecesContext,
    /* [output]
     * The ECES context.
     */
    unsigned int *keyBufferLength,
    /* [input/output]
     * On input it gives the length of the key buffer in octets.
     * On output it gives the length of the session key in octets.
     */
    unsigned char *keyBuffer
    /* [output]
     * A buffer to hold the session key.
     */
);

/*--sb_ecesBREncryptBegin()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * If an error occurs, the key buffer and key buffer length are undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PUBLIC_KEY  --The public key buffer is null.
 *    SB_BAD_PUBLIC_KEY --The public key is invalid.
 *    SB_NO_CNTL_INFO   --The additional information pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_OUTBUF_LEN --The key buffer length is too short.
 *    SB_NO_LEN_PTR     --The key buffer length pointer is null.
 *    SB_NO_OUTBUF      --The key buffer pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesBREncryptBegin(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * Recipient's public key.
     */
    const unsigned int sharedDataLength,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    sb_EncContext *ecesContext,
    /* [output]
     * The ECES context.
     */
    unsigned int *keyBufferLength,
    /* [input/output]
     * On input it gives the length of the key buffer in octets.
     * On output it gives the length of the session key in octets.
     */
    unsigned char *keyBuffer
    /* [output]
     * A buffer to hold the session key.
     */
);

/*--sb_ecesEncrypt()
 * The ECES encryption algorithm.
 * If an error occurs, the ciphertext is undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_BAD_INBUF_LEN  --The plaintext length is zero.
 *    SB_NO_INBUF       --The plaintext pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_NO_OUTBUF      --The ciphertext pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesEncrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const unsigned int textLength,
    /* [input]
     * Length of the plaintext and ciphertext blocks in octets.
     */
    const unsigned char *plaintextBlock,
    /* [input]
     * A block of plaintext to be encrypted.
     */
    sb_EncContext *ecesContext,
    /* [input/output]
     * The ECES context.
     */
    unsigned char *ciphertextBlock
    /* [output]
     * The encrypted message block, must be at least textLength.
     */
);

/*--sb_ecesBREncrypt()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * If an error occurs, the ciphertext is undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_BAD_INBUF_LEN  --The plaintext length is zero.
 *    SB_NO_INBUF       --The plaintext pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_NO_OUTBUF      --The ciphertext pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesBREncrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const unsigned int textLength,
    /* [input]
     * Length of the plaintext and ciphertext blocks in octets.
     */
    const unsigned char *plaintextBlock,
    /* [input]
     * A block of plaintext to be encrypted.
     */
    sb_EncContext *ecesContext,
    /* [input/output]
     * The ECES context.
     */
    unsigned char *ciphertextBlock
    /* [output]
     * The encrypted message block, must be at least textLength.
     */
);

/*--sb_ecesEncryptEnd()
 * The ECES encryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesEncryptEnd(
    void *globalData,
    /* [input]
     * Global data.
     */
    sb_EncContext *ecesContext
    /* [input/output]
     * The ECES context.
     */
);

/*--sb_ecesBREncryptEnd()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesBREncryptEnd(
    void *globalData,
    /* [input]
     * Global data.
     */
    sb_EncContext *ecesContext
    /* [input/output]
     * The ECES context.
     */
);

/*--sb_ecesDecryptBegin()
 * The ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PRIVATE_KEY --The private key pointer is null.
 *    SB_BAD_PRIVATE_KEY--The private key is outside the allowed range.
 *    SB_BAD_INBUF_LEN  --The key length is not equal to the session key length.
 *    SB_NO_INBUF       --The key pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesDecryptBegin(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * The unprotected private key.
     */
    const unsigned int keyLength,
    /* [input]
     * The length of the session key in octets.
     */
    const unsigned char *keyBuffer,
    /* [input]
     * The session key buffer.
     */
    sb_EncContext *ecesContext
    /* [output]
     * The ECES context.
     */
);

/*--sb_ecesBRDecryptBegin()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PRIVATE_KEY --The private key pointer is null.
 *    SB_BAD_PRIVATE_KEY--The private key is outside the allowed range.
 *    SB_NO_CNTL_INFO   --The additional information pointer is null.
 *    SB_BAD_INBUF_LEN  --The key length is not equal to the session key length.
 *    SB_NO_INBUF       --The key pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesBRDecryptBegin(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * The unprotected private key.
     */
    const unsigned int sharedDataLength,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int keyLength,
    /* [input]
     * The length of the session key in octets.
     */
    const unsigned char *keyBuffer,
    /* [input]
     * The session key buffer.
     */
    sb_EncContext *ecesContext
    /* [output]
     * The ECES context.
     */
);

/*--sb_ecesDecrypt()
 * The ECES decryption algorithm.
 * If an error occurs, the plaintext is undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_BAD_INBUF_LEN  --The ciphertext length is zero.
 *    SB_NO_INBUF       --The ciphertext pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_NO_OUTBUF      --The plaintext pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesDecrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const unsigned int textLength,
    /* [input]
     * Length of the ciphertext and plaintext blocks in octets.
     */
    const unsigned char *ciphertextBlock,
    /* [input]
     * A buffer containing a block of ciphertext.
     */
    sb_EncContext *ecesContext,
    /* [input/output]
     * The ECES context.
     */
    unsigned char *plaintextBlock
    /* [output]
     * A buffer containing a block of plaintext, must be at least textLength.
     */
);

/*--sb_ecesBRDecrypt()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * If an error occurs, the plaintext is undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_BAD_INBUF_LEN  --The ciphertext length is zero.
 *    SB_NO_INBUF       --The ciphertext pointer is null.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_NO_OUTBUF      --The plaintext pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesBRDecrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const unsigned int textLength,
    /* [input]
     * Length of the ciphertext and plaintext blocks in octets.
     */
    const unsigned char *ciphertextBlock,
    /* [input]
     * A buffer containing a block of ciphertext.
     */
    sb_EncContext *ecesContext,
    /* [input/output]
     * The ECES context.
     */
    unsigned char *plaintextBlock
    /* [output]
     * A buffer containing a block of plaintext, must be at least textLength.
     */
);

/*--sb_ecesDecryptEnd()
 * The ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesDecryptEnd(
    void *globalData,
    /* [input]
     * Global data.
     */
    sb_EncContext *ecesContext
    /* [input/output]
     * The ECES context.
     */
);

/*--sb_ecesBRDecryptEnd()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_CONTEXT     --The context pointer is null.
 *    SB_BAD_CONTEXT    --The context has not been initialized.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesBRDecryptEnd(
    void *globalData,
    /* [input]
     * Global data.
     */
    sb_EncContext *ecesContext
    /* [input/output]
     * The ECES context.
     */
);

/*--sb_ecesPwdDecryptBegin()
 * The ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA      --The global data is null.
 *    SB_NOT_INITIALIZED     --The global data is not initialized.
 *    SB_BAD_PWD_LEN         --The password length is zero or greater than SB_MAX_PWD_LEN.
 *    SB_NO_PWD              --The password pointer is null.
 *    SB_NO_PROT_PRIVATE_KEY --The protected private key pointer is null.
 *    SB_BAD_PROT_PRIVATE_KEY--The protected private key size is wrong.
 *    SB_BAD_PRIVATE_KEY     --The private key is outside the allowed range.
 *    SB_BAD_INBUF_LEN       --The key length is zero.
 *    SB_NO_INBUF            --The key buffer pointer is null.
 *    SB_NO_CONTEXT          --The context pointer is null.
 *    SB_FAILURE             --The operation failed.
 *    SB_SUCCESS             --The operation completed successfully.
 */
int
sb_ecesPwdDecryptBegin(
    void *globalData,
    /* [input]
     * Global data.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password used to decrypt the protected private key.
     */
    const unsigned char password[],
    /* [input]
     * Password used to decrypt the protected private key.
     */
    const sb_ProtectedKey *protectedKey,
    /* [input]
     * A password protected private key.
     */
    const unsigned int keyLength,
    /* [input]
     * The length of the session key in octets.
     */
    const unsigned char *keyBuffer,
    /* [input]
     * The session key buffer.
     */
    sb_EncContext *ecesContext
    /* [output]
     * The ECES context.
     */
);

/*--sb_ecesBRPwdDecryptBegin()
 * The Bellaire-Rogaway ECES decryption algorithm.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA      --The global data is null.
 *    SB_NOT_INITIALIZED     --The global data is not initialized.
 *    SB_BAD_PWD_LEN         --The password length is zero or greater than SB_MAX_PWD_LEN.
 *    SB_NO_PWD              --The password pointer is null.
 *    SB_NO_PROT_PRIVATE_KEY --The protected private key pointer is null.
 *    SB_BAD_PROT_PRIVATE_KEY--The protected private key size is wrong.  
 *    SB_BAD_PRIVATE_KEY     --The private key is outside the allowed range.
 *    SB_NO_CNTL_INFO        --The additional information pointer is null.
 *    SB_BAD_INBUF_LEN       --The key length is zero.
 *    SB_NO_INBUF            --The key buffer pointer is null.
 *    SB_NO_CONTEXT          --The context pointer is null.
 *    SB_FAILURE             --The operation failed.
 *    SB_SUCCESS             --The operation completed successfully.
 */
int
sb_ecesBRPwdDecryptBegin(
    void *globalData,
    /* [input]
     * Global data.
     */
    const unsigned int pwdLength,
    /* [input]
     * Length of the password used to decrypt the protected private key.
     */
    const unsigned char password[],
    /* [input]
     * Password used to decrypt the protected private key.
     */
    const sb_ProtectedKey *protectedKey,
    /* [input]
     * A password protected private key.
     */
    const unsigned int sharedDataLength,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int keyLength,
    /* [input]
     * The length of the session key in octets.
     */
    const unsigned char *keyBuffer,
    /* [input]
     * The session key buffer.
     */
    sb_EncContext *ecesContext
    /* [output]
     * The ECES context.
     */
);


#endif /*SBECES_H*/
