/*+
 * sbecaes.h - The Application Programmer's Interface
 *           -- ECAES Encryption and Decryption Module
 *
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB ENC module.
 *
 *        sb_ecaesEncrypt()
 *        sb_ecaesBREncrypt()
 *        sb_ecaesDecrypt()
 *        sb_ecaesBRDecrypt()
 *        sb_ecaesPwdDecrypt()
 *        sb_ecaesBRPwdDecrypt()
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
#ifndef SBECAES_H
#define SBECAES_H    1

#include "sbglb.h"
#include "sbkey.h"
#include "sbsha.h"

/*--sb_ecaesEncrypt()
 * The ECAES encryption algorithm.
 * If an error occurs, the ciphertext and ciphertext length are undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PUBLIC_KEY  --The public key pointer is null.
 *    SB_BAD_PUBLIC_KEY --The public key is invalid.
 *    SB_NO_CNTL_INFO   --The control information pointer is null.
 *    SB_BAD_INBUF_LEN  --The plaintext length is zero.
 *    SB_NO_INBUF       --The plaintext pointer is null.
 *    SB_BAD_OUTBUF_LEN --The ciphertext buffer length is too short.
 *    SB_NO_LEN_PTR     --The ciphertext length pointer is null.
 *    SB_NO_OUTBUF      --The ciphertext pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecaesEncrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * Recipient's public key.
     */
    const unsigned int controlInfoLength,
    /* [input]
     * Length of the optional control information.
     */
    const unsigned char *controlInfo,
    /* [input]
     * Optional control information.
     */
    const unsigned int plaintextLength,
    /* [input]
     * Length of the plaintext in octets.
     */
    const unsigned char *plaintext,
    /* [input]
     * The plaintext to be encrypted.
     */
    unsigned int *ciphertextLength,
    /* [input/output]
     * On input it gives the length of the ciphertext buffer in octets.
     * On output it gives the length of the ciphertext in octets.
     */
    unsigned char *ciphertext
    /* [output]
     * The encrypted message.
     */
);

/*--sb_ecaesDecrypt()
 * Use the ECAES decryption algorithm to decrypt a message.
 * If an error occurs, the plaintext and plaintext length are undefined.
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
 *    SB_NO_CNTL_INFO   --The control information pointer is null.
 *    SB_BAD_INBUF_LEN  --The ciphertext length is zero.
 *    SB_NO_INBUF       --The ciphertext pointer is null.
 *    SB_BAD_OUTBUF_LEN --The plaintext buffer length is too short.
 *    SB_NO_LEN_PTR     --The plaintext length pointer is null.
 *    SB_NO_OUTBUF      --The plaintext pointer is null.
 *    SB_NO_FLAG        --The authentication flag pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecaesDecrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * An unprotected private key.
     */
    const unsigned int controlInfoLength,
    /* [input]
     * Length of the optional control information.
     */
    const unsigned char *controlInfo,
    /* [input]
     * Optional control information.
     */
    const unsigned int ciphertextLength,
    /* [input]
     * Length of the ciphertext in octets.
     */
    const unsigned char *ciphertext,
    /* [input]
     * The ciphertext to be decrypted.
     */
    unsigned int *plaintextLength,
    /* [input/output]
     * On input it gives the length of the plaintext buffer in octets.
     * On output it gives the length of the plaintext in octets.
     */
    unsigned char *plaintext,
    /* [output]
     * The decrypted message.
     */
    unsigned int *authenticate
    /* [output]
     * 0 - authentication failed.
     * 1 - authentication passed.
     */
);

/*--sb_ecaesPwdDecrypt()
 * Use the ECAES decryption algorithm to decrypt a message.
 * If an error occurs, the plaintext and plaintext length are undefined.
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
 *    SB_NO_CNTL_INFO        --The control information pointer is null.
 *    SB_BAD_INBUF_LEN       --The ciphertext length is zero.
 *    SB_NO_INBUF            --The ciphertext pointer is null.
 *    SB_BAD_OUTBUF_LEN      --The plaintext buffer length is too short.
 *    SB_NO_LEN_PTR          --The plaintext length pointer is null.
 *    SB_NO_OUTBUF           --The plaintext pointer is null.
 *    SB_NO_FLAG             --The authentication flag pointer is null.
 *    SB_FAILURE             --The operation failed.
 *    SB_SUCCESS             --The operation completed successfully.
 */
int
sb_ecaesPwdDecrypt(
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
    const unsigned int controlInfoLength,
    /* [input]
     * Length of the optional control information.
     */
    const unsigned char *controlInfo,
    /* [input]
     * Optional control information.
     */
    const unsigned int ciphertextLength,
    /* [input]
     * Length of the ciphertext in octets.
     */
    const unsigned char *ciphertext,
    /* [input]
     * The ciphertext to be decrypted.
     */
    unsigned int *plaintextLength,
    /* [input/output]
     * On input it gives the length of the plaintext buffer in octets.
     * On output it gives the length of the plaintext in octets.
     */
    unsigned char *plaintext,
    /* [output]
     * The decrypted message.
     */
    unsigned int *authenticate
    /* [output]
     * 0 - authentication failed.
     * 1 - authentication passed.
     */
);

/*--sb_ecaesBREncrypt()
 * The ECAES encryption algorithm.
 * If an error occurs, the ciphertext and ciphertext length are undefined.
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PUBLIC_KEY  --The public key pointer is null.
 *    SB_BAD_PUBLIC_KEY --The public key is invalid.
 *    SB_NO_CNTL_INFO   --One of the additioal information pointer is null.
 *    SB_BAD_INBUF_LEN  --The plaintext length is zero.
 *    SB_NO_INBUF       --The plaintext pointer is null.
 *    SB_BAD_OUTBUF_LEN --The ciphertext buffer length is too short.
 *    SB_NO_LEN_PTR     --The ciphertext length pointer is null.
 *    SB_NO_OUTBUF      --The ciphertext pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecaesBREncrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * Recipient's public key.
     */
    const unsigned int sharedData1Length,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData1,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int sharedData2Length,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData2,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int plaintextLength,
    /* [input]
     * Length of the plaintext in octets.
     */
    const unsigned char *plaintext,
    /* [input]
     * The plaintext to be encrypted.
     */
    unsigned int *ciphertextLength,
    /* [input/output]
     * On input it gives the length of the ciphertext buffer in octets.
     * On output it gives the length of the ciphertext in octets.
     */
    unsigned char *ciphertext
    /* [output]
     * The encrypted message.
     */
);

/*--sb_ecaesBRDecrypt()
 * Use the ECAES decryption algorithm to decrypt a message.
 * If an error occurs, the plaintext and plaintext length are undefined.
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
 *    SB_NO_CNTL_INFO   --One of the additional information pointer is null.
 *    SB_BAD_INBUF_LEN  --The ciphertext length is zero.
 *    SB_NO_INBUF       --The ciphertext pointer is null.
 *    SB_BAD_OUTBUF_LEN --The plaintext buffer length is too short.
 *    SB_NO_LEN_PTR     --The plaintext length pointer is null.
 *    SB_NO_OUTBUF      --The plaintext pointer is null.
 *    SB_NO_FLAG        --The authentication flag pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecaesBRDecrypt(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * An unprotected private key.
     */
    const unsigned int sharedData1Length,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData1,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int sharedData2Length,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData2,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int ciphertextLength,
    /* [input]
     * Length of the ciphertext in octets.
     */
    const unsigned char *ciphertext,
    /* [input]
     * The ciphertext to be decrypted.
     */
    unsigned int *plaintextLength,
    /* [input/output]
     * On input it gives the length of the plaintext buffer in octets.
     * On output it gives the length of the plaintext in octets.
     */
    unsigned char *plaintext,
    /* [output]
     * The decrypted message.
     */
    unsigned int *authenticate
    /* [output]
     * 0 - authentication failed.
     * 1 - authentication passed.
     */
);

/*--sb_ecaesBRPwdDecrypt()
 * Use the ECAES decryption algorithm to decrypt a message using 
 * Bellare-Rogaway scheme.
 * If an error occurs, the plaintext and plaintext length are undefined.
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
 *    SB_NO_CNTL_INFO        --One of the additional information pointer is null.
 *    SB_BAD_INBUF_LEN       --The ciphertext length is zero.
 *    SB_NO_INBUF            --The ciphertext pointer is null.
 *    SB_BAD_OUTBUF_LEN      --The plaintext buffer length is too short.
 *    SB_NO_LEN_PTR          --The plaintext length pointer is null.
 *    SB_NO_OUTBUF           --The plaintext pointer is null.
 *    SB_NO_FLAG             --The authentication flag pointer is null.
 *    SB_FAILURE             --The operation failed.
 *    SB_SUCCESS             --The operation completed successfully.
 */
int
sb_ecaesBRPwdDecrypt(
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
    const unsigned int sharedData1Length,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData1,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int sharedData2Length,
    /* [input]
     * Additional info length.
     */
    const unsigned char *sharedData2,
    /* [input]
     * Optional additional info shared by
     * the sender and the recipient.
     */
    const unsigned int ciphertextLength,
    /* [input]
     * Length of the ciphertext in octets.
     */
    const unsigned char *ciphertext,
    /* [input]
     * The ciphertext to be decrypted.
     */
    unsigned int *plaintextLength,
    /* [input/output]
     * On input it gives the length of the plaintext buffer in octets.
     * On output it gives the length of the plaintext in octets.
     */
    unsigned char *plaintext,
    /* [output]
     * The decrypted message.
     */
    unsigned int *authenticate
    /* [output]
     * 0 - authentication failed.
     * 1 - authentication passed.
     */
);


#endif /*SBECAES_H*/
