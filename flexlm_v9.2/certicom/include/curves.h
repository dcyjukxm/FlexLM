/*+
 * curves.h - The Curve Object List
 *
 * This is an automatically generated file. Do not edit.
 *
 * Description: This header file holds the interface
 *   definitions for the predefined elliptic curves.
 *-------------------------------------------------------------
 * Conditional compilations: None.
 *-------------------------------------------------------------
 * Notes: None
 *   
-*/
#ifndef CURVES_H
#define CURVES_H    1


/*=== Include Files =========================================================*/
#include <limits.h>



/*=== Module Preconditions ==================================================*/
#if CHAR_BIT != 8
#error CHAR_BIT must be equal 8.
#endif



/*=== Constant Values =======================================================*/
    /*---Basis Types
     * CURVE_BASIS_TYPE_ONB_I     Optimal normal basis type I.
     * CURVE_BASIS_TYPE_ONB_II    Optimal normal basis type II.
     * CURVE_BASIS_TYPE_UMBRAL    Umbral basis type.
     * CURVE_BASIS_TYPE_POLY      Polynomial basis type.
     * CURVE_BASIS_TYPE_FP        Fp basis type.
     * CURVE_BASIS_TYPE_ANSI_POLY Polynomial basis ANSI curve type.
     */
#define CURVE_BASIS_TYPE_ONB_I     ((unsigned char)0)
#define CURVE_BASIS_TYPE_ONB_II    ((unsigned char)1)
#define CURVE_BASIS_TYPE_UMBRAL    ((unsigned char)2)
#define CURVE_BASIS_TYPE_POLY      ((unsigned char)3)
#define CURVE_BASIS_TYPE_FP        ((unsigned char)4)
#define CURVE_BASIS_TYPE_ANSI_POLY ((unsigned char)5)
    /* 
     * MAXIM_OID_CHARS - Total numbers of OID characters
     */
#define MAXIM_OID_CHARS   31 



/*=== Data Type Definitions =================================================*/
    /*---ellipticCurveParameters
     * Defines the elliptic curve parameters.
     */
typedef struct ellipticCurveParameters {
    unsigned char oid[ MAXIM_OID_CHARS + 1 ];
    struct {
        unsigned char major[ 1 ];
        unsigned char minor[ 1 ];
    } version;
    unsigned char checksum[ 4 ];
    unsigned char fieldSize[ 2 ]; /* bits */
    unsigned char fieldSizeOctets[ 1 ]; /* octets */
    unsigned char basisType[ 1 ];
    unsigned char modulus[ 32 ];
    const unsigned char *ident1;
    const unsigned char *ident2;
    struct {
        unsigned char a[ 32 ];
        unsigned char b[ 32 ];
    } curveParameter;
    struct {
        unsigned char value[ 64 ];
    } generatingPoint;
    struct {
        unsigned char size[ 2 ]; /* bits */
        unsigned char value[ 32 ];
    } pointOrder;
    struct {
        unsigned char size[ 2 ]; /* bits */
        unsigned char value[ 32 ];
    } cofactor;
    struct {
        unsigned char size[ 2 ]; /* bits */
        unsigned char value[ 32 ];
    } curveOrder;
    struct {
        unsigned char A[ 32 ];
        unsigned char B[ 32 ];
    } reserved;
} ellipticCurveParameters;



/*=== Curves Definitions ====================================================*/

#define sect163k1 ec163a02  
#define sect239k1 ec239a03  
#define sect163r1 ec163b02



    /*---ec163a02 (a.k.a. sect163k1)
     * Elliptic curve parameters for the field size 163.
     */
extern const ellipticCurveParameters ec163a02;


    /*---ec239a03 (a.k.a. sect239k1)
     * Elliptic curve parameters for the field size 239.
     */
extern const ellipticCurveParameters ec239a03;


    /*---ec163b02 (a.k.a. sect163r1)
     * Elliptic curve parameters for the field size 163.
     */
extern const ellipticCurveParameters ec163b02;


    /*---sect233k1
     * Elliptic curve parameters for the field size 233.
     */
extern const ellipticCurveParameters sect233k1;


    /*---sect113r1
     * Elliptic curve parameters for the field size 113.
     */
extern const ellipticCurveParameters sect113r1;


    /*---sect163r2
     * Elliptic curve parameters for the field size 163.
     */
extern const ellipticCurveParameters sect163r2;


    /*---sect233r1
     * Elliptic curve parameters for the field size 233.
     */
extern const ellipticCurveParameters sect233r1;


    /*---secp112r1
     * Elliptic curve parameters for the field size 112/Fp
     */
extern const ellipticCurveParameters secp112r1;


    /*---secp160r1
     * Elliptic curve parameters for the field size 160/Fp
     */
extern const ellipticCurveParameters secp160r1;


    /*---secp192r1
     * Elliptic curve parameters for the field size 192/Fp
     */
extern const ellipticCurveParameters secp192r1;


    /*---secp224r1
     * Elliptic curve parameters for the field size 224/Fp
     */
extern const ellipticCurveParameters secp224r1;


    /*---secp256r1
     * Elliptic curve parameters for the field size 256/Fp
     */
extern const ellipticCurveParameters secp256r1;



#endif /*CURVES_H*/
