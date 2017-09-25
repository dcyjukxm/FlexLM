/*+
 * sbwrap.h - The Application Programmer's Interface
 *              -- Elliptic Curve Key Wrapping Module
 * 
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB WRAP module.
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
#ifndef SBWRAP_H
#define SBWRAP_H    1

#include <string.h>

#include "sbglb.h"
#include "sbenc.h"
#include "sbkey.h"
#include "sbrc.h"

/*--sb_ecesWrap()
 * Use the ECES encryption algorithm to wrap the key.
 *
 * Notes:
 *   The contents of wrappedKey and wrappedKeyLength are undefined if an error
 *   occurs.
 *   An error occurs whenever the function terminates with an error return code
 *   other than SB_SUCCESS.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PUBLIC_KEY  --The public key pointer is null.
 *    SB_BAD_PUBLIC_KEY --The public key is invalid.
 *    SB_BAD_INBUF_LEN  --The key length is zero.
 *    SB_NO_INBUF       --The key pointer is null.
 *    SB_BAD_OUTBUF_LEN --The wrapped key buffer length is zero.
 *    SB_NO_LEN_PTR     --The wrapped key length pointer is null.
 *    SB_NO_OUTBUF      --The wrapped key pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesWrap(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PublicKey *publicKey,
    /* [input]
     * Recipient's public key.
     */
    const unsigned int keyLength,
    /* [input]
     * Length of the key to be wrapped.
     */
    const unsigned char *key,
    /* [input]
     * The key to be wrapped.
     */
    unsigned int *wrappedKeyLength,
    /* [input/output]
     * On input it gives the length of the wrappedKey buffer.
     * On output it gives the length of the wrapped key.
     */
    unsigned char *wrappedKey
    /* [output]
     * The wrapped key.
     */
);

/*--sb_ecesUnwrap()
 * Use the ECES decryption algorithm to unwrap the key.
 *
 * Notes:
 *   The contents of key and keyLength are undefined if an error occurs.
 *   An error occurs whenever the function terminates with an error return code
 *   other than SB_SUCCESS.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PRIVATE_KEY --The private key pointer is null.
 *    SB_BAD_PRIVATE_KEY--The private key is outside the allowed range.
 *    SB_BAD_INBUF_LEN  --The wrapped key length is zero.
 *    SB_NO_INBUF       --The wrapped key pointer is null.
 *    SB_BAD_OUTBUF_LEN --The unwrapped key buffer length is zero.
 *    SB_NO_LEN_PTR     --The unwrapped key length pointer is null.
 *    SB_NO_OUTBUF      --The unwrapped key pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecesUnwrap(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * Pointer to an unprotected private key.
     */
    const unsigned int wrappedKeyLength,
    /* [input]
     * The wrapped key length.
     */
    const unsigned char *wrappedKey,
    /* [input]
     * The key to be unwrapped.
     */
    unsigned int *keyLength,
    /* [input/output]
     * On input it gives the length of the key buffer.
     * On output it gives the length of the unwrapped key.
     */
    unsigned char *key
    /* [output]
     * The unwrapped key.
     */
);

/*--sb_ecesPwdUnwrap()
 * Call api_decryptPasswordProtectedKey() to decrypt the password protected
 * private key.
 * Call sb_ecesUnwrap() to unwrap the key.
 *
 * Notes:
 *   The contents of key and keyLength are undefined if an error occurs.
 *   An error occurs whenever the function terminates with an error return code
 *   other than SB_SUCCESS.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA      --The global data is null.
 *    SB_NOT_INITIALIZED     --The global data is not initialized.
 *    SB_BAD_PWD_LEN         --The password length is 0 or greater than SB_MAX_PWD_LEN.
 *    SB_NO_PWD              --The password pointer is null.
 *    SB_NO_PROT_PRIVATE_KEY --The protected private key pointer is null.
 *    SB_BAD_PROT_PRIVATE_KEY--The protected private key format is wrong.
 *    SB_BAD_PRIVATE_KEY     --The private key is outside the allowed range.
 *    SB_BAD_INBUF_LEN       --The wrapped key length is zero.
 *    SB_NO_INBUF            --The wrapped key pointer is null.
 *    SB_BAD_OUTBUF_LEN      --The unwrapped key buffer length is zero.
 *    SB_NO_LEN_PTR          --The unwrapped key length pointer is null.
 *    SB_NO_OUTBUF           --The unwrapped key pointer is null.
 *    SB_FAILURE             --The operation failed.
 *    SB_SUCCESS             --The operation completed successfully.
 *    SB_NOT_IMPLEMENTED     --The function is not implemented.
 */
int
sb_ecesPwdUnwrap(
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
     * Pointer to a password protected private key.
     */
    const unsigned int wrappedKeyLength,
    /* [input]
     * The wrapped key length.
     */
    const unsigned char *wrappedKey,
    /* [input]
     * The key to be unwrapped.
     */
    unsigned int *keyLength,
    /* [input/output]
     * On input it gives the length of the key buffer.
     * On output it gives the length of the unwrapped key.
     */
    unsigned char *key
    /* [output]
     * The unwrapped key.
     */
);

/*--sb_ecaesWrap()
 * Use the ECAES encryption algorithm to wrap the key.
 *
 * Notes:
 *   The contents of wrappedKey and wrappedKeyLength are undefined if an error
 *   occurs.
 *   An error occurs whenever the function terminates with an error return code
 *   other than SB_SUCCESS.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PUBLIC_KEY  --The public key pointer is null.
 *    SB_NO_CNTL_INFO   --The control information pointer is null.
 *    SB_BAD_INBUF_LEN  --The key length is zero.
 *    SB_NO_INBUF       --The key pointer is null.
 *    SB_BAD_OUTBUF_LEN --The wrapped key buffer length is zero.
 *    SB_NO_LEN_PTR     --The wrapped key length pointer is null.
 *    SB_NO_OUTBUF      --The wrapped key pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecaesWrap(
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
    const unsigned int keyLength,
    /* [input]
     * The length of the key to be wrapped.
     */
    const unsigned char *key,
    /* [input]
     * The key to be wrapped.
     */
    unsigned int *wrappedKeyLength,
    /* [input/output]
     * On input it gives the length of the wrappedKey buffer.
     * On output it gives the the length of the wrapped key.
     */
    unsigned char *wrappedKey
    /* [output]
     * The wrapped key.
     */
);

/*--sb_ecaesUnwrap()
 * Use the ECAES decryption algorithm to unwrap the key.
 *
 * Notes:
 *   The key buffer is cleared if authentication fails.
 *   The contents of key and keyLength are undefined if an error occurs.
 *   An error occurs whenever the function terminates with an error return code
 *   other than SB_SUCCESS.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA --The global data is null.
 *    SB_NOT_INITIALIZED--The global data is not initialized.
 *    SB_NO_PRIVATE_KEY --The private key pointer is null.
 *    SB_BAD_PRIVATE_KEY--The private key is outside the allowed range.
 *    SB_NO_CNTL_INFO   --The control information pointer is null.
 *    SB_BAD_INBUF_LEN  --The wrapped key length is zero.
 *    SB_NO_INBUF       --The wrapped key pointer is null.
 *    SB_BAD_OUTBUF_LEN --The unwrapped key buffer length is zero.
 *    SB_NO_LEN_PTR     --The unwrapped key pointerlength is null.
 *    SB_NO_OUTBUF      --The unwrapped key pointer is null.
 *    SB_NO_FLAG        --The authentication flag pointer is null.
 *    SB_FAILURE        --The operation failed.
 *    SB_SUCCESS        --The operation completed successfully.
 */
int
sb_ecaesUnwrap(
    void *globalData,
    /* [input]
     * Global data.
     */
    const sb_PrivateKey *privateKey,
    /* [input]
     * Pointer to an unprotected private key.
     */
    const unsigned int controlInfoLength,
    /* [input]
     * Length of the optional control information.
     */
    const unsigned char *controlInfo,
    /* [input]
     * Optional control information.
     */
    const unsigned int wrappedKeyLength,
    /* [input]
     * The wrapped key length.
     */
    const unsigned char *wrappedKey,
    /* [input]
     * The key to be unwrapped.
     */
    unsigned int *keyLength,
    /* [input/output]
     * On input it gives the length of the key buffer.
     * On output it gives the length of the unwrapped key.
     */
    unsigned char *key,
    /* [output]
     * The unwrapped key.
     */
    unsigned int *authenticate
    /* [output]
     * 0 - authentication failed.
     * 1 - authentication passed.
     */
);

/*--sb_ecaesPwdUnwrap()
 * Call api_decryptPasswordProtectedKey() to decrypt the password protected
 * private key.
 * Call sb_ecaesUnwrap() to unwrap the key.
 *
 * Notes:
 *   The key buffer is cleared if authentication fails.
 *   The contents of key and keyLength are undefined if an error occurs.
 *   An error occurs whenever the function terminates with an error return code
 *   other than SB_SUCCESS.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA      --The global data is null.
 *    SB_NOT_INITIALIZED     --The global data is not initialized.
 *    SB_BAD_PWD_LEN         --The password length is 0 or greater than SB_MAX_PWD_LEN.
 *    SB_NO_PWD              --The password pointer is null.
 *    SB_NO_PROT_PRIVATE_KEY --The protected private key pointer is null.
 *    SB_BAD_PROT_PRIVATE_KEY--The protected private key format is wrong.
 *    SB_BAD_PRIVATE_KEY     --The private key is outside the allowed range.
 *    SB_NO_CNTL_INFO        --The control information pointer is null.
 *    SB_BAD_INBUF_LEN       --The wrapped key length is zero.
 *    SB_NO_INBUF            --The wrapped key pointer is null.
 *    SB_BAD_OUTBUF_LEN      --The unwrapped key buffer length is zero.
 *    SB_NO_LEN_PTR          --The unwrapped key length pointer is null.
 *    SB_NO_OUTBUF           --The unwrapped key pointer is null.
 *    SB_NO_FLAG             --The authentication flag pointer is null.
 *    SB_FAILURE             --The operation failed.
 *    SB_SUCCESS             --The operation completed successfully.
 *    SB_NOT_IMPLEMENTED     --The function is not implemented.
 */
int
sb_ecaesPwdUnwrap(
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
     * Pointer to a password protected private key.
     */
    const unsigned int controlInfoLength,
    /* [input]
     * Length of the optional control information.
     */
    const unsigned char *controlInfo,
    /* [input]
     * Optional control information.
     */
    const unsigned int wrappedKeyLength,
    /* [input]
     * The wrapped key length.
     */
    const unsigned char *wrappedKey,
    /* [input]
     * The key to be unwrapped.
     */
    unsigned int *keyLength,
    /* [input/output]
     * On input it gives the length of the key buffer.
     * On output it gives the length of the unwrapped key.
     */
    unsigned char *key,
    /* [output]
     * The unwrapped key.
     */
    unsigned int *authenticate
    /* [output]
     * 0 - authentication failed.
     * 1 - authentication passed.
     */
);


#endif /*SBWRAP_H*/
