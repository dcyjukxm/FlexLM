/******************************************************************************

            COPYRIGHT (c) 2000, 2003  by Macrovision Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of 
        Macrovision Corporation and is protected by law.  It may 
        not be copied or distributed in any form or medium, disclosed 
        to third parties, reverse engineered or used in any manner not 
        provided for in said License Agreement except with the prior 
        written authorization from Macrovision Corporation.

 *****************************************************************************/
/*
 *	Module: $Id: l_ctype.c,v 1.7 2003/01/13 22:41:50 kmaclean Exp $
 *
 *
 *      D. Birns
 *	Created:  September, 2000
 *      
 *
 */

/*#define AUTODESK 1*/
#include <ctype.h>
#define CHAR_BITS 0xff

int l_isalnum(int c) { return isalnum((unsigned char) (c & CHAR_BITS)); }
int l_isalpha(int c) { return isalpha((unsigned char) (c & CHAR_BITS)); }
int l_iscntrl(int c) { return iscntrl((unsigned char) (c & CHAR_BITS)); }
int l_isdigit(int c) { return isdigit((unsigned char) (c & CHAR_BITS)); }
int l_isgraph(int c) { return isgraph((unsigned char) (c & CHAR_BITS)); }
int l_islower(int c) { return islower((unsigned char) (c & CHAR_BITS)); }
int l_isprint(int c) { return isprint((unsigned char) (c & CHAR_BITS)); }
int l_ispunct(int c) { return ispunct((unsigned char) (c & CHAR_BITS)); }
int l_isupper(int c) { return isupper((unsigned char) (c & CHAR_BITS)); }
int l_isxdigit(int c) { return isxdigit((unsigned char) (c & CHAR_BITS)); }
#ifdef VXWORKS
/* our own is ascii. 
 * If the high bit is on then it is not ascii */
int l_isascii(int c) { return (((c & (unsigned int)0x80) == (unsigned int)0x80) ? 0 : 1); }
#else 
int l_isascii(int c) { return isascii((unsigned char) (c & CHAR_BITS)); }
#endif 
int l_isspace(int c) { 
	if ((c >= 9 && c <= 13) || c == 32) return 1;
	return 0;
}

int l_toupper(int c) 
{ 
	c = c & CHAR_BITS;
	if (!(c & 0x80))
		return toupper(c) ;
	switch (c)
	{
	
	case 131: return 131; /*Ÿ-Ÿ*/
	case 154: return 138; /*s-S*/
	case 156: return 140; /*o-O*/
	case 158: return 142; /*z-Z*/
	case 223: return 223; /*á-á*/
	case 224: return 192; /*…-A*/
	case 225: return 193; /* -A*/
	case 226: return 194; /*ƒ-A*/
	case 227: return 195; /*a-A*/
	case 228: return 196; /*„-Ž*/
	case 229: return 197; /*†-*/
	case 230: return 198; /*‘-’*/
	case 231: return 199; /*‡-€*/
	case 232: return 200; /*Š-E*/
	case 233: return 201; /*‚-*/
	case 234: return 202; /*ˆ-E*/
	case 235: return 203; /*‰-E*/
	case 236: return 204; /*-I*/
	case 237: return 205; /*¡-I*/
	case 238: return 206; /*Œ-I*/
	case 239: return 207; /*‹-I*/
	case 240: return 208; /*d-D*/
	case 241: return 209; /*¤-¥*/
	case 242: return 210; /*•-O*/
	case 243: return 211; /*¢-O*/
	case 244: return 212; /*“-O*/
	case 245: return 213; /*o-O*/
	case 246: return 214; /*”-™*/
	case 248: return 216; /*o-O*/
	case 249: return 217; /*—-U*/
	case 250: return 218; /*£-U*/
	case 251: return 219; /*–-U*/
	case 252: return 220; /*-š*/
	case 253: return 221; /*y-Y*/
	case 254: return 222; /*_-_*/
#ifndef AUTODESK
	case 255: return 159; /*˜-Y*/
#endif /* not AUTODESK */
	}
#ifdef AUTODESK
	if (c == 255) return 159; /*˜-Y*/
#endif /* AUTODESK */
	return c;
}



	
int l_tolower(int c) 
{ 
	c = c & CHAR_BITS;
	if (!(c & 0x80))
		return tolower(c) ;
	switch (c)
	{
	case 138: return 154; /*Š-š*/
	case 140: return 156; /*Œ-œ*/
	case 142: return 158; /*Ž-ž*/
	case 159: return 255; /*Ÿ-ÿ*/
	case 192: return 224; /*À-à*/
	case 193: return 225; /*Á-á*/
	case 194: return 226; /*Â-â*/
	case 195: return 227; /*Ã-ã*/
	case 196: return 228; /*Ä-ä*/
	case 197: return 229; /*Å-å*/
	case 198: return 230; /*Æ-æ*/
	case 199: return 231; /*Ç-ç*/
	case 200: return 232; /*È-è*/
	case 201: return 233; /*É-é*/
	case 202: return 234; /*Ê-ê*/
	case 203: return 235; /*Ë-ë*/
	case 204: return 236; /*Ì-ì*/
	case 205: return 237; /*Í-í*/
	case 206: return 238; /*Î-î*/
	case 207: return 239; /*Ï-ï*/
	case 208: return 240; /*Ð-ð*/
	case 209: return 241; /*Ñ-ñ*/
	case 210: return 242; /*Ò-ò*/
	case 211: return 243; /*Ó-ó*/
	case 212: return 244; /*Ô-ô*/
	case 213: return 245; /*Õ-õ*/
	case 214: return 246; /*Ö-ö*/
	case 216: return 248; /*Ø-ø*/
	case 217: return 249; /*Ù-ù*/
	case 218: return 250; /*Ú-ú*/
	case 219: return 251; /*Û-û*/
	case 220: return 252; /*Ü-ü*/
	case 221: return 253; /*Ý-ý*/
	case 222: return 254; /*Þ-þ*/
	}
	return c;
}

