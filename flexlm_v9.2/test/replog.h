/*	
 *	Module: $Id: replog.h,v 1.6 2003/03/03 23:55:49 sluu Exp $
 *
 *	for test/replog
 *
 *	Last changed:  12/21/98
 *
 */


#define ENCRYPTION_SEED1 0x87654321 
#define ENCRYPTION_SEED2 0x12345678

/*
 *	FLEXlm vendor keys
 */
/*-
 *	Generate these keys with: lmvkey -v demo -d (+3 months) -p ALL -c GSI
 *		(Use a date approx 3 months out)
 */

#define VENDOR_KEY1 0x4054eac3
#define VENDOR_KEY2 0x450077c4
#define VENDOR_KEY3 0x542a4c5b
#define VENDOR_KEY4 0x3951ddd6
#define VENDOR_KEY5 0x0b165dc9
/*
 *	FLEXlm vendor name
 */

#define VENDOR_NAME "demo"

