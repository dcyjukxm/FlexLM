/******************************************************************************

	    COPYRIGHT (c) 2000, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_borrow.h,v 1.15 2003/01/13 22:13:11 kmaclean Exp $
 *
 *	Description: borrowing information
 *
 *	D. Birns
 *	1/2001
 *
 */

#ifndef _L_BORROW_H
#define _L_BORROW_H

#define L_BORROW_DIR ".lmborrow"
void l_borrow(LM_HANDLE *job, char *feature, CONFIG *conf);
CONFIG *l_borrow_conf(LM_HANDLE *job, char *feature, char **borrow_pos);
#define L_DEFAULT_MAX_BORROW_HOURS (7 * 24)
#define L_BORROW_REG "infoborrow"
#define L_BCRYPT_ROUNDS 20
#define L_BORROW_SEPARATOR "-+!-"
#define L_BORROW_ID_SEP "-+#-"
#define L_BORROW_MAGIC 0xd83980a2
#define L_BORROW_MAGIC2 0x9ace36db
#define L_BORROW_SEPARATOR2 "+++"

#define L_BBLOCK_SIZE 32 /* must be < 250 */


#endif /* _L_BORROW_H */
