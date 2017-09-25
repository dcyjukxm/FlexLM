/*+
 * sbrc.h - The Application Programmer's Interface
 *              -- Global Data Module Error Return Codes
 *
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB GLB module.
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
#ifndef SBRC_H
#define SBRC_H    1

    /*
     * Error Return Codes
     *   -- Common error return codes.
     */
#define SB_SUCCESS 0x0000
#define SB_NOT_INITIALIZED 0x0001
#define SB_NO_GLOBAL_DATA 0x0002
#define SB_FAILURE 0x0003
#define SB_BAD_BUF_LEN 0x0004
#define SB_NO_CONTEXT 0x0005
#define SB_BAD_INBUF_LEN 0x0006
#define SB_BAD_OUTBUF_LEN 0x0007
#define SB_NO_BUF 0x0008
#define SB_NO_BUF_LEN 0x0009
#define SB_BAD_CONTEXT 0x000a
#define SB_NO_INBUF 0x000b
#define SB_NO_INBUF_LEN 0x000c
#define SB_NO_OUTBUF 0x0000d
#define SB_NO_OUTBUF_LEN 0x000e
#define SB_NO_FLAG 0x000f
#define SB_NOT_IMPLEMENTED 0x0010
    /*
     * Error Return Codes
     *   -- CA related error return codes.
     */
#define SB_BAD_CERT 0x0100
#define SB_BAD_CERT_LEN 0x0101
#define SB_BAD_CERT_INFO_LEN 0x0102
#define SB_BAD_SIGNATURE_LEN 0x0103
#define SB_BAD_USER_DATA_LEN 0x0104
#define SB_NO_CERT 0x0105
#define SB_NO_CERT_INFO 0x0106
#define SB_NO_USER_DATA 0x0107
    /*
     * Error Return Codes
     *   -- DES related error return codes.
     */
#define SB_BAD_DES_ALG 0x0200
#define SB_BAD_DES_KEY 0x0201
#define SB_BAD_DES_KEY_PARITY 0x0202
#define SB_BAD_DES_CONTEXT_REQ 0x0203
#define SB_BAD_NUM_DES_KEYS 0x0204
#define SB_NO_DES_CONTEXT_REQ 0x0205
#define SB_NO_PLAINTEXT_BUF 0x0206
#define SB_NO_CYPHERTEXT_BUF 0x0207
#define SB_NO_CIPHERTEXT_BUF 0x0207
    /*
     * Error Return Codes
     *   -- ENC related error return codes.
     */
#define SB_NO_CNTL_INFO 0x0300
#define SB_NO_LEN_PTR 0x0301
    /*
     * Error Return Codes
     *   -- EXCH related error return codes.
     */
#define SB_BAD_REMOTE_KEY 0x0400
#define SB_BAD_LOCAL_KEY 0x0401
#define SB_BAD_LOCAL_PRIVATE_KEY 0x0402
#define SB_BAD_REMOTE_PUBLIC_KEY 0x0403
#define SB_NO_LOCAL_KEY 0x0404
#define SB_NO_REMOTE_KEY 0x0405
#define SB_NO_REMOTE_PUBLIC_KEY 0x0406
#define SB_BAD_PROT_LOCAL_KEY 0x0407
#define SB_NO_PROT_LOCAL_KEY 0x0408
    /*
     * Error Return Codes
     *   -- GLB related error return codes.
     */
#define SB_BAD_ERROR_RETURN_CODE 0x0500
    /*
     * Error Return Codes
     *   -- INI related error return codes.
     */
#define SB_BAD_DATA_SIZE 0x0600
#define SB_BAD_EC_PARAMS 0x0601
#define SB_BAD_HEAP_SIZE 0x0602
#define SB_BAD_OPTIONS_LIST 0x0603
#define SB_BAD_COMPRESSION_FLAG 0x0604
#define SB_NO_DATA_SPACE 0x0605
#define SB_NO_HEAP_SPACE 0x0606
#define SB_NO_OPTIONS_LIST 0x0607
#define SB_NO_EC_PARAMS 0x0608
#define SB_BAD_ECES_COMPRESSION_FLAG    0x0609
    /*
     * Error Return Codes
     *   -- KEY related error return codes.
     */
#define SB_BAD_NEW_PWD_LEN 0x0700
#define SB_BAD_PWD_LEN 0x0701
#define SB_BAD_PRIVATE_KEY 0x0702
#define SB_BAD_PROT_PRIVATE_KEY 0x0703
#define SB_BAD_PUBLIC_KEY 0x0704
#define SB_NO_PRIVATE_KEY 0x0705
#define SB_NO_PROT_PRIVATE_KEY 0x0706
#define SB_NO_PUBLIC_KEY 0x0707
#define SB_NO_NEW_PWD 0x0708
#define SB_NO_PWD 0x0709
#define SB_NO_OLD_PUBLIC_KEY    0x070a
#define SB_NO_NEW_PUBLIC_KEY    0x070b
#define SB_BAD_OLD_PUBLIC_KEY   0x070c
#define SB_BAD_NEW_PUBLIC_KEY   0x070d
    /*
     * Error Return Codes
     *   -- RNG related error return codes.
     */
#define SB_SEED_NOT_INITIALIZED 0x0800
#define SB_BAD_RANDOM_INPUT 0x0801
#define SB_BAD_RNG_TYPE 0x0802
#define SB_BAD_SEED_LEN 0x0803
#define SB_NO_RNG_TYPE 0x0804
#define SB_NO_SEED 0x0805
    /*
     * Error Return Codes
     *   -- ARC4 related error return codes.
     */
#define SB_NO_ARC4_CONTEXT_REQ 0x0900
#define SB_BAD_ARC4_KEY_SIZE 0x0901
    /*
     * Error Return Codes
     *   -- SIG-DSA related error return codes.
     */
#define SB_BAD_ECDSA_SIGNATURE 0x0a00
#define SB_NO_ECDSA_SIGNATURE 0x0a01
    /*
     * Error Return Codes
     *   -- SIG-NR related error return codes.
     */
#define SB_BAD_ECNR_SIGNATURE 0x0b00
#define SB_NO_ECNR_SIGNATURE 0x0b01
    /*
     * Error Return Codes
     *   -- ICS related error return codes.
     */
#define SB_BAD_PUBLIC_PARAM 0x0e00
#define SB_NO_PUBLIC_PARAM 0x0e01
    /*
     * Error Return Codes
     *   -- ICS-DSA related error return codes.
     */
#define SB_BAD_DSA_SIGNATURE 0x0f00
#define SB_NO_DSA_SIGNATURE 0x0f01
    /*
     * Error Return Codes
     *   -- HMAC related error return codes.
     */
#define SB_BAD_HMAC_KEY 0x1100
#define SB_NO_HMAC_KEY 0x1101
    /*
     * Error Return Codes
     *   -- Bullet Certificate related error return codes.
     */
#define SB_NO_USER_PRI_CONT 0x1200
#define SB_NO_USER_PUB_CONT 0x1201
#define SB_NO_CA_PRI_CONT 0x1202
#define SB_NO_CA_PUB_CONT 0x1203
#define SB_NO_CA_PRI_KEY 0x1205
#define SB_NO_CA_PUB_KEY 0x1205
#define SB_NO_PRI_RECON 0x1205
#define SB_NO_PUB_RECON 0x1205

#endif /*SBRC_H*/
