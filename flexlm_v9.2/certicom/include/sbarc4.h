/*+
 * sbarc4.h - The Application Programmer's Interface -- ARC4 Module
 *
 * Description: This header file holds the interface definitions
 * for the SB ARC4 module.  This is an automatically generated file.
 * Do not edit.
 *
 * 
 * 
 *

-*/
#ifndef SBARC4_H
#define SBARC4_H      1



    /*
     * This is the request structure used by SB ARC4 functions.
     */
#if defined(LIMITED_EXPORT) || defined(UNLIMITED_EXPORT)
#define SB_MAX_ARC4_KEY_SIZE 7
#else
#define SB_MAX_ARC4_KEY_SIZE 256
#endif

typedef struct {
    unsigned sbint32 keySize;
    unsigned char key[SB_MAX_ARC4_KEY_SIZE];
} sb_Arc4Request;


    /*
     * This is the context structure used by SB ARC4 functions.
     */
#define SB_ARC4_CONTEXT_LENGTH    264

typedef struct {
    char context[SB_ARC4_CONTEXT_LENGTH];
} sb_Arc4Context;



    /*
     * The following are the function prototypes.
     */
/*---sb_arc4GenerateKey()
 * Generate a key for an ARC4 session.
 *
 * Return:
 *    SB_NO_GLOBAL_DATA      -- The global data is NULL.
 *    SB_NOT_INITIALIZED     -- The global data area has not been initialized.
 *    SB_NO_ARC4_CONTEXT_REQ -- The request is NULL.
 *    SB_BAD_ARC4_KEY_SIZE   -- The ARC4 key size is incorrect.
 *                              The size must be larger than 0 byte and at most
 *                              SB_MAX_ARC4_KEY_SIZE bytes.
 *    SB_FAILURE             -- The operation failed.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_arc4GenerateKey(
    void *globalData,
        /* [input]
         * This is the global data. 
         * Preconditions: globalData points to a correctly initialized
         *                global area.
         */
    sb_Arc4Request *arc4Request
        /* [input/output]    
         * On input, describes the size of the key to be generated in the
         * keySize member of the request structure.
         * On output, contains the generated keys such that the request
         * can be passed directly to sb_arc4Begin().
         * Preconditions: ARC4 request is non-NULL.
         */
    );


/*---sb_arc4Begin()
 * Initialize an ARC4 context from the ARC4 request.
 *       
 * Notes:  
 *   (1) If an error occurs, the resulting ARC4 context is zero filled.
 *
 * Return: 
 *    SB_NO_ARC4_CONTEXT_REQ -- The ARC4 request is NULL.
 *    SB_BAD_ARC4_KEY_SIZE   -- The ARC4 key size is incorrect.
 *                              The size must be larger than 0 byte and at most
 *                              SB_MAX_ARC4_KEY_SIZE bytes.
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_arc4Begin(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    const sb_Arc4Request *arc4Request,
        /* [input]
         * Preconditions: ARC4 request is non-NULL.
         *                The key is set in the ARC4 request.
         */
    sb_Arc4Context *arc4Context
        /* [input/output]
         * The ARC4 Context.
         * Preconditions: ARC4 context is non-NULL. 
         */
    );


/*---sb_arc4Encrypt()
 * Encrypt data using an ARC4 context.
 * This function can be called repeatedly as needed.
 *
 * Notes:
 *   (1) The ARC4 context is zero filled if an error occurs.
 *
 * Return: 
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not initialized.
 *    SB_NO_PLAINTEXT_BUF    -- The plaintext buffer is NULL.
 *    SB_NO_CIPHERTEXT_BUF   -- The ciphertext buffer is NULL.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_arc4Encrypt(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    sb_Arc4Context *arc4Context,
        /* [input/output]
         * The ARC4 Context.
         * Preconditions: ARC4 context is non-NULL.
         */
    const unsigned char plaintextBuffer[],
        /* [input]
         * The plaintext to be encrypted.
         * Preconditions: plaintext is non-NULL.
         */
    const unsigned sbint32 bufferSize,
        /* [input]
         * Length of the plaintext and ciphertext buffers (in bytes).
         */
    unsigned char ciphertextBuffer[]
        /* [output]
         * The resulting ciphertext
         * Preconditions: ciphertext is non-NULL.
         */
    );


/*---sb_arc4Decrypt()
 * Decrypt data using an ARC4 context.
 * This function can be called repeatedly as needed.
 *
 * Notes:
 *   (1) The ARC4 context is zero filled if an error occurs.
 *
 * Return:
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not initialized.
 *    SB_NO_CIPHERTEXT_BUF   -- The ciphertext buffer is NULL.
 *    SB_NO_PLAINTEXT_BUF    -- The plaintext buffer is NULL.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_arc4Decrypt(
    void * globalData,
        /* [input]
         * This is the global data.
         */
    sb_Arc4Context * arc4Context,
        /* [input/output]
         * The ARC4 Context.
         * Preconditions: ARC4 context is non-NULL.
         */
    const unsigned char ciphertextBuffer[],
        /* [input]
         * The ciphertext to be decrypted
         * Preconditions: ciphertext is non-NULL.
         */
    const unsigned sbint32 bufferSize,
        /* [input]
         * Length of the plaintext and ciphertext buffers (in bytes).
         */
    unsigned char plaintextBuffer[]
        /* [output]
         * The resulting plaintext
         * Preconditions: plaintext is non-NULL.
         */
    );


/*---sb_arc4End()
 * Close the ARC4 context.
 *
 * Return: 
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not initialized.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int
sb_arc4End(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    sb_Arc4Context *arc4Context
        /* [input/output]
         * The ARC4 Context.
         * Preconditions: ARC4 context is non-NULL.
         */
    );


#endif /*sbarc4.h*/
