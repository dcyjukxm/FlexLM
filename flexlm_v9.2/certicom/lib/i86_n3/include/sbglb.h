/*+
 * sbglb.h - The Application Programmer's Interface
 *              -- Global Data Module
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
#ifndef SBGLB_H
#define SBGLB_H    1

    /*
     * Include Files
     */
#include <limits.h>
#include "sbrc.h"

    /*
     * Constant Values
     */
    /*
     * SB_ERROR_MESSAGE_LENGTH
     *   --This macro defines the number of octets needed to contain an
     *     error message.
     */
#define SB_ERROR_MESSAGE_LENGTH 64
    /*---SB_FIELD_ELEMENT_OCTETS
     * The maximum number of octets needed to contain a field element.
     */
#define SB_FIELD_ELEMENT_OCTETS 32
    /*---SB_MAXIMAL_FIELD_SIZE
     * The maximum field size in bits.
     */
#define SB_MAXIMAL_FIELD_SIZE 256
    /*---SB_FIELD_SIZE_OCTETS
     * The number of octets needed to contain the maximal field size.
     */
#define SB_FIELD_SIZE_OCTETS (((SB_MAXIMAL_FIELD_SIZE - 1) / CHAR_BIT) + 1)
    /*---SB_DATA_SIZE
     * The maximum number of octets needed to a represent an integral length.
     */
#define SB_DATA_SIZE sizeof(unsigned int)
    /*---SB_TRUE and SB_FALSE
     * The values for true and false.
     */
#define SB_FALSE (1 == 0)
#define SB_TRUE (0 == 0)
    /*---SB_MANUFACTURER_LEN
     * The maximum length of the manufacturer's identity buffer.
     */
#define SB_MANUFACTURER_LEN 32
    /*---SB_BUILD_NUMBER_LEN
     * The maximum length of the build number's identity buffer.
     */
#define SB_BUILD_NUMBER_LEN 40

    /*
     * Macros Definitions
     */
/*---SB_PUT_DATA_SIZE()
 * A macro to marshal the length of an API data structure. It converts the
 * non-portable numeric data size in the structure to an octet string. This
 * macro is useful for moving API data from the application level to the
 * transport level.
 *
 * Notes:
 *  (1) The contents of the octet string are undefined whenever an error occurs.
 *  (2) API structures available for marshalling all have a size member. This
 *      macro is useful for marshalling this size (the other members in an API
 *      data structure are suitable for data transfer as is).
 *
 * Returns:
 *   SB_BAD_OUTBUF_LEN--Octet string is too short.
 *   SB_FAILURE       --Failed to complete operation.
 *   SB_NO_OUTBUF     --Null octet string buffer.
 *   SB_SUCCESS       --Successfully completed operation.
 *
 * Parameters:
 *   ptr2struct [input]
 *     This is a pointer to the structure whose size is to be converted to an
 *     octet string. It is restricted to non-null values.
 *   bufLen [input]
 *     This is the length of the buffer to contain the marshalled value. It
 *     must be greater than or equal to the length of an unsigned integer.
 *   buf [output]
 *     This is the buffer to contain the marshalled value. It is restricted to
 *     non-null values.
 */
#define SB_PUT_DATA_SIZE( ptr2struct, bufLen, buf ) \
sb_uint2os( (ptr2struct)->size, bufLen, buf )
/*---SB_GET_DATA_SIZE()
 * A macro to marshal the portable representation of the length of a data
 * element into an API structure. It converts the portable length into a the
 * machine specific representation used by the API structure. This macro is
 * useful for moving data at the transporation level to the application level.
 *
 * Notes:
 *  (1) The contents of the unsigned integer are undefined whenever an error
 *      occurs.
 *  (2) Octet strings containing more than SB_DATA_SIZE octets are
 *      trucated.
 *  (3) API structures available all have a size member. This macro is useful
 *      for marshalling values into this data member.
 *
 * Returns:
 *   SB_FAILURE      --Failed to complete operation.
 *   SB_BAD_INBUF_LEN--Octet string has length equal to zero or has length
 *                     greater than SB_DATA_SIZE.
 *   SB_NO_INBUF     --Null octet string buffer.
 *   SB_NO_OUTBUF    --Null length buffer.
 *   SB_SUCCESS      --Successfully completed operation.
 *
 * Parameters:
 *   bufLen [input]
 *     This is the length of the buffer containing the value to convert. It
 *     must be greater than zero and less than or equal to SB_DATA_SIZE.
 *   buf [input]
 *     This is the buffer to containing the value to convert. It is restricted
 *     to non-null values.
 *   ptr2struct [output]
 *     This is a pointer to the structure whose size is to contain the
 *     converted value. It is restricted to non-null values.
 */
#define SB_GET_DATA_SIZE( bufLen, buf, ptr2struct ) \
sb_os2uint( bufLen, buf, &(ptr2struct)->size )

    /*
     * Data Type Definitions
     */
    /*---sb_Info
     * Implementation information.
     */
typedef struct sb_Info {
    struct {
        unsigned int major;
        unsigned int minor;
    } version;
    unsigned char manufacturer[ SB_MANUFACTURER_LEN ];
    unsigned char buildNumber [ SB_BUILD_NUMBER_LEN ];
} sb_Info;

    /*
     * Function Prototypes
     */
/*---sb_errorMessage()
 * Returns the error message associated with an error return code.
 *
 * Notes:
 *   (1) The error message buffer is undefined whenever an error occurs.
 *
 * Returns:
 *   SB_BAD_ERROR_RETURN_CODE--Unrecognized error return code.
 *   SB_BAD_OUTBUF_LEN       --Error message buffer too small.
 *   SB_NO_OUTBUF            --Null error message buffer.
 *   SB_SUCCESS              --Successfully completed operation.
 */
int
sb_errorMessage(
    const int errorReturnCode,
    /* [input]
     * The error return code.
     */
    const int errorMessageLength,
    /* [input]
     * The length of the error message buffer. It must be large enough to
     * hold the entire error message.
     */
    char *errorMessage
    /* [ouput]
     * The buffer for error message associated with the error return code.
     * It is restricted to non-null values.
     */
);
/*---sb_os2uint()
 * Convert an octet string to an unsigned integer.
 *
 * Notes:
 *  (1) The contents of the unsigned integer are undefined whenever an error
 *      occurs.
 *
 * Returns:
 *   SB_FAILURE      --Failed to complete operation.
 *   SB_BAD_INBUF_LEN--Octet string has length equal to zero or has length
 *                     greater than SB_DATA_SIZE.
 *   SB_NO_INBUF     --Null octet string buffer.
 *   SB_NO_OUTBUF    --Null length buffer.
 *   SB_SUCCESS      --Successfully completed operation.
 */
int
sb_os2uint(
    const unsigned int nOctets,
    /* [input]
     * The number of octets in the octet string to be converted. It must be
     * greater than zero.
     */
    const unsigned char octetString[],
    /* [input]
     * The octet string to be converted. The octet string is restricted to
     * non-null values.
     */
    unsigned int *length
    /* [output]
     * The converted length. The length is restricted to non-null values.
     */
);
/*---sb_uint2os()
 * Convert an unsigned integer to an octet string.
 *
 * Notes:
 *  (1) The contents of the octet string are undefined whenever an error occurs.
 *
 * Returns:
 *   SB_BAD_OUTBUF_LEN--Octet string is too short.
 *   SB_FAILURE       --Failed to complete operation.
 *   SB_NO_OUTBUF     --Null octet string buffer.
 *   SB_SUCCESS       --Successfully completed operation.
 */
int
sb_uint2os(
    const unsigned int length,
    /* [input]
     * The length to be converted.
     */
    const unsigned int nOctets,
    /* [input]
     * The number of octets in the octet string to be converted. There must
     * be enough elements in the octet string to contain the entire array.
     */
    unsigned char octetString[]
    /* [output]
     * The octet string to be converted. The octet string is restricted to
     * non-null values.
     */
);
/*---sb_getInfo()
 * Returns the implementation information.
 *
 * Notes:
 *   (1) Implementation information undefined whenever an error occurs.
 *
 * Returns:
 *   SB_NO_OUTBUF    --No buffer for implementation information.
 *   SB_SUCCESS      --Successfully completed operation.
 */
int
sb_getInfo(
    sb_Info *info
        /* [output]
         * The implementation information. It is restricted to non-null values.
         */
);

#endif /*SBGLB_H*/
