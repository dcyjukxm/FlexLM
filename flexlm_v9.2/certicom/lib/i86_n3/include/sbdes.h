/*+
 * sbdes.h - The Application Programmer's Interface -- DES Module
 *
 * Description: This header file holds the interface definitions
 * for the SB DES module.  This is an automatically generated file.
 * Do not edit.
 *
 * 
 * 
 *
 * This software contains trade secrets and confidential information of
 * Certicom Corp.
 * (c) Copyright Certicom Corp. 1999
-*/
#ifndef SBDES_H
#define SBDES_H


/*=============constants and data structures==================*/

/*---SB_DES_BLOCK_SIZE
 * Size of a DES block in bytes.
 */
#define SB_DES_BLOCK_SIZE 8


/*
 * Key array selector constants
 * Each constant defines which key of an array of keys is used for
 * what purpose for a specific DES algorithm.
 */  
#define SB_DES_DES_KEY          0x0
#define SB_DES_DES_IV           0x1
#define SB_DES_TDES_KEY1        0x0
#define SB_DES_TDES_KEY2        0x1
#define SB_DES_TDES_KEY3        0x2
#define SB_DES_TDES_IV          0x3
#define SB_DES_DESX_KEY         0x0
#define SB_DES_DESX_PREWHITEN   0x1
#define SB_DES_DESX_POSTWHITEN  0x2
#define SB_DES_DESX_IV          0x3


    /* 
     * Data Type Definitions
     */ 
/*---sb_DesAlgorithm
 * DES algorithms supported by Security Builder.
 */
typedef enum {
    SB_DES_ECB,     /* DES, ECB mode */
    SB_DES_CBC,     /* DES, CBC mode */
    SB_TDES_ECB,    /* triple DES, ECB mode */
    SB_TDES_CBC,    /* triple DES, CBC mode */
    SB_DESX_ECB,    /* DESX, ECB mode */
    SB_DESX_CBC,    /* DESX, CBC mode */
    SB_DES_CFB,     /* DES, CFB mode */
    SB_DES_OFB,     /* DES, OFB mode */
    SB_TDES_CFB,    /* triple DES, CFB mode */
    SB_TDES_OFB,    /* triple DES, OFB mode */
    SB_DESX_CFB,    /* DESX, CFB mode */
    SB_DESX_OFB     /* DESX, OFB mode */
} sb_DesAlgorithm;


/*---sb_DesKeyParityOption
 * This enumeration lists the possible choices for whether key parity
 * is to be enforced or ignored.
 */
typedef enum {
    SB_DES_IGNORE_KEY_PARITY,   /* ignore the parity of all DES keys */
    SB_DES_ENFORCE_KEY_PARITY   /* ensure that DES keys have correct parity */
} sb_DesKeyParityOption;


/*---sb_DesWeakKeyOption
 * This enumeration lists the possible choices for whether weak keys are
 * to be detected or ignored 
 */ 
typedef enum {
    SB_DES_IGNORE_WEAK_KEYS,    /* weak keys can be generated or used */
    SB_DES_DETECT_WEAK_KEYS     /* weak keys cannot be generated or used */
} sb_DesWeakKeyOption; 


/*---sb_DesKey
 * An sb_DesKey holds the data for a single length DES key.
 */
#define SB_DES_KEY_SIZE 8

typedef struct {
    unsigned char data[SB_DES_KEY_SIZE];
} sb_DesKey;


/*---sb_DesRequest
 * The parameters needed to create a DES context using sb_desBegin().
 * Set keysPresent to the number of valid keys that are present in keys.
 */ 
#define SB_MAX_DES_KEYS 4

typedef struct {
    sb_DesAlgorithm         algorithm;          /* DES algorithm to use ? */
    sb_DesKeyParityOption   enforceParity;      /* keys need parity? */
    sb_DesWeakKeyOption     allowWeakKeys;      /* allow weak DES keys */
    unsigned                keysPresent;        /* how many valid keys ? */
    sb_DesKey               keys[SB_MAX_DES_KEYS]; /* key data */
} sb_DesRequest;


/*---sb_DesContext
 * The DES intermediate data.
 */
#define SB_DES_CONTEXT_LENGTH 408

typedef struct {
    char context[SB_DES_CONTEXT_LENGTH];
} sb_DesContext;



/*=============function prototypes============================*/

/*---sb_desGenerateKeys()
 * Generate keys for a DES session. 
 * 
 * Return:
 *    SB_NO_DES_CONTEXT_REQ  -- The request is NULL.
 *    SB_NO_GLOBAL_DATA      -- The global data is null.
 *    SB_NOT_INITIALIZED     -- The global data area has not been initialized.
 *    SB_NO_DES_CONTEXT_REQ  -- The DES request is null.
 *    SB_BAD_DES_ALG         -- The DES algorithm requested is invalid.
 *    SB_BAD_DES_CONTEXT_REQ -- The DES context request is invalid
 *    SB_FAILURE             -- The operation failed.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int 
sb_desGenerateKeys(
    void * globalData,
        /* [input]
         * This is the global data.
         */
    sb_DesRequest *request
        /* [input/output]
         * On input, describes the keys to be generated.
         * On output, contains the generated keys such that the request
         * can be passed directly to sb_desBegin().
         */
);


/*---sb_desBegin()
 * Create a DES context from the DES request.
 *
 * Notes:
 *   (1) If an error occurs, the resulting DES context is zero filled.
 *    
 * Return:
 *    SB_NO_DES_CONTEXT_REQ  -- The DES request is null.
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_NUM_DES_KEYS    -- The DES number of keys provided is incorrect.
 *    SB_BAD_DES_ALG         -- The DES algorithm requested is invalid.
 *    SB_BAD_DES_KEY_PARITY  -- One or more of the supplied DES keys have
 *                              incorrect parity.
 *    SB_BAD_DES_KEY         -- One or more of the supplied DES keys are weak.
 *    SB_FAILURE             -- The operation failed.
 *    SB_SUCCESS             -- The operation completed successfully.
 */
int 
sb_desBegin(
    void * globalData,
        /* [input]
         * This is the global data.
         */
    const sb_DesRequest * request,
        /* [input]
         * Contains generated DES keys and requested algorithm.
         */
    sb_DesContext * context
           /* [input/output]
            * The DES intermediate data.
            */
);


/*---sb_desEncrypt()
 * Encrypt data using a DES context.
 *    
 * Notes:
 *   (1) The DES context is zero filled if an error occurs.
 *  
 * Return:  
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not recognizable.
 *    SB_NO_PLAINTEXT_BUF    -- plaintextBuffer is NULL.
 *    SB_BAD_BUF_LEN         -- buffer size is not multiple of SB_DES_KEY_SIZE.
 *    SB_NO_CIPHERTEXT_BUF   -- ciphertextBuffer is NULL.
 *    SB_FAILURE             -- The operation failed.
 *    SB_SUCCESS             -- The operation completed successfully.
 */ 
int   
sb_desEncrypt(
    void * globalData,
        /* [input]
         * This is the global data.
         */
    sb_DesContext * context,
           /* [input/output]
            * The DES intermediate data.
            */
    const unsigned char plaintextBuffer[],
        /* [input]
         * The plaintext to be encrypted.
         */
    const unsigned sbint32 bufferSize,
            /* [input]
             * Length of the plaintext and ciphertext buffers (in bytes).
             */
    unsigned char ciphertextBuffer[]
        /* [output]
         * The resulting ciphertext
         */
);


/*---sb_desDecrypt()
 * Decrypt data using a DES context.
 *
 * Notes:
 *   (1) The DES context is zero filled if an error occurs.
 *
 * Return:  
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not recognizable.
 *    SB_NO_CIPHERTEXT_BUF   -- ciphertextBuffer is NULL.
 *    SB_BAD_BUF_LEN         -- buffer size is not multiple of SB_DES_KEY_SIZE.
 *    SB_NO_PLAINTEXT_BUF    -- plaintextBuffer is NULL.
 *    SB_FAILURE             -- The operation failed. 
 *    SB_SUCCESS             -- The operation completed successfully.
 */ 
int 
sb_desDecrypt(
    void * globalData,
        /* [input]
         * This is the global data.
         */
    sb_DesContext * context,
           /* [input/output]
            * The DES intermediate data.
            */
    const unsigned char ciphertextBuffer[],
        /* [input]
         * The ciphertext to be decrypted
         */
    const unsigned sbint32 bufferSize,
            /* [input]       
             * Length of the plaintext and ciphertext buffers (in bytes).
             */
    unsigned char plaintextBuffer[] 
        /* [output]
         * The resulting plaintext
         */
);


/*---sb_desEnd()
 * Destroy a DES context once it is no longer needed.
 *       
 * Notes:
 *   (1) The DES context is always zero filled.
 *          
 * Return:  
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not recognizable.
 *    SB_FAILURE             -- The operation failed.
 *    SB_SUCCESS             -- The operation completed successfully.
 */ 
int
sb_desEnd(
    void * globalData,
        /* [input]
         * This is the global data.
         */
    sb_DesContext * context
           /* [input/output]
            * The DES intermediate data.
            */  
);


/*---sb_desSetIV()
 * Reset the initialization value.
 *
 * Notes: 
 *   (1) If an error occurs, the IV stays unchanged.
 *    
 * Return:
 *    SB_NO_CONTEXT          -- The context is NULL.
 *    SB_BAD_CONTEXT         -- The context is not recognizable.
 *    SB_BAD_DES_ALG         -- This algorithm is not supported.
 *    SB_NO_INBUF            -- IV buffer is NULL.
 *    SB_FAILURE             -- The operation failed.
 *    SB_SUCCESS             -- The operation completed successfully.
 */      
int      
sb_desSetIV(
    void *globalData,
        /* [input]
         * This is the global data.
         */
    const sb_DesKey *desIV,
        /* [input]
         * DES Initialization Value.
         */
    sb_DesContext *context
        /* [input/output]
         * The DES intermediate data.
         */
);


#endif /*sbdes.h*/
