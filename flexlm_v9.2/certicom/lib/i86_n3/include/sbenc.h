/*+
 * sbenc.h - The Application Programmer's Interface
 *           -- Elliptic Curve Encryption and Decryption Module
 *
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface definitions
 *   for the SB ENC module.
 *
 *        sb_getSessionKeyLength()
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
#ifndef SBENC_H
#define SBENC_H    1

#include "sbrc.h"
#include "sbeces.h"
#include "sbecaes.h"

/*--sb_getSessionKeyLength()
 * An error occurs whenever the function terminates with an error return code
 * other than SB_SUCCESS. This function returns the length of the session key.
 *
 * Notes: None.
 *
 * Returns:
 *    SB_NO_GLOBAL_DATA     --The global data is null.
 *    SB_NOT_INITIALIZED    --The global data is not initialized.
 *    SB_NO_LEN_PTR         --The session key length pointer is null.
 *    SB_FAILURE            --The operation failed.
 *    SB_SUCCESS            --The operation completed successfully.
 */
int
sb_getSessionKeyLength(
    void *globalData,
    /* [input]
     * Global data.
     */
    unsigned int *sessionKeyLength
    /* [output]
     * Length of the session key in octets.
     */
);


#endif /*SBENC_H*/
