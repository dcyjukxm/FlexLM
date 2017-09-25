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
 *	Module: $Id: ls_lock.h,v 1.4 2003/01/13 22:22:33 kmaclean Exp $
 *
 *	D. Birns
 *	10/2000
 *
 */

/*
 *	Here's the idea:
 *	1) We have 1 required lockfile, and a list of lockfiles that we want
 *	   we want to make sure aren't used by any other vendor daemons.
 *	   We use the standard (pre-v8) method for the required lockfile, and
 *	   for the others, we simply remove them if they exist, and give
 *	   a failure only if they exist and can't be removed.
 *	2) We want to hide the strings for the paths to the lockfiles.
 *	3) We want ISVs to be able to add to the list of historic lockfiles
 *	   via ls_user_init1().
 */
#define LS_MAX_LOCKFILES 30
char *l_locks[LS_MAX_LOCKFILES];

#define L_USR_TMP "/usr/tmp"
#define L_USR_TMP_LOCK 0
static unsigned int l_usr_tmp[sizeof(L_USR_TMP) * sizeof(int)] =
	{
	('/' - ' ') ^ 0x12345100,
	('u' - ' ')^ 0x13843200,
	('s'- ' ') ^ 0x1584a300,
	('r'- ' ') ^ 0x3284d400,
	('/' - ' ')^ 0x3284f500,
	('t' - ' ')^ 0xa2741600,
	('m' - ' ')^ 0x52d41700,
	('p' - ' ')^ 0x72e41800, 0 };
#define L_INIT_LOCKNAME(x, c) \
	{ int i; \
		for (i = 0; x[i]; i++) \
			c[i]= (char)(x[i] & 0xff) + ' '; \
		c[i] = 0; \
	}
#define L_USR_TMP_FLEXLM "/usr/tmp/.flexlm"
#define L_USR_TMP_FLEXLM_LOCK 1
static unsigned int l_usr_tmp_flexlm[sizeof(L_USR_TMP_FLEXLM) * sizeof(int)] =
	{
	('/' - ' ') ^0xa21f5100,
	('u' - ' ')^ 0xac3f5200,
	('s'- ' ') ^ 0xac145300,
	('r'- ' ') ^ 0xa2145400,
	('/' - ' ')^ 0xac1f5500,
	('t' - ' ')^ 0xbc145600,
	('m' - ' ')^ 0xb23f5700,
	('p' - ' ')^ 0xbc145700,
	('/' - ' ')^ 0xbc1f5700,
	('.' - ' ')^ 0x32345700,
	('f' - ' ')^ 0x3c3f5700,
	('l' - ' ')^ 0x3c1a5700,
	('e' - ' ')^ 0x321a5700,
	('x' - ' ')^ 0x8c3a5700,
	('l' - ' ')^ 0x8c145700,
	('m' - ' ')^ 0x829a5800, 0 };

#define L_VAR_TMP "/var/tmp"
#define L_VAR_TMP_LOCK 2
static unsigned int l_var_tmp[sizeof(L_VAR_TMP) * sizeof(int)] =
	{
	('/' - ' ') ^ 0x12345100,
	('v' - ' ')^ 0x12345200,
	('a'- ' ') ^ 0x12345300,
	('r'- ' ') ^ 0x12345400,
	('/' - ' ')^ 0x12345500,
	('t' - ' ')^ 0x12345600,
	('m' - ' ')^ 0x12345700,
	('p' - ' ')^ 0x12345800, 0 };
static char ls_var_tmp[sizeof(L_VAR_TMP)];


#define L_PC_LOC "C:\\flexlm"
#define L_PC_LOC_LOCK 0
static unsigned int l_pc_loc[sizeof(L_PC_LOC) * sizeof(int)] =
	{
	('C' - ' ') ^ 0x12345100,
	(':' - ' ')^ 0x12345200,
	('\\'- ' ') ^ 0x12345300,
	('f'- ' ') ^ 0x12345400,
	('l' - ' ')^ 0x12345500,
	('e' - ' ')^ 0x12345600,
	('x' - ' ')^ 0x12345700,
	('l' - ' ')^ 0x12345700,
	('m' - ' ')^ 0x12345800, 0 };
static char ls_pc_loc[sizeof(L_USR_TMP)];

#define L_PC_LOCPRE "LM_"
#define L_PC_LOC_LOCKPRE 0
static unsigned int l_pc_locpre[sizeof(L_PC_LOCPRE) * sizeof(int)] =
	{
	('L' - ' ') ^ 0x12345100,
	('M' - ' ')^ 0x12345200,
	('_'- ' ') ^ 0x12345300, 0};

#define L_PC_LOCPOS "_SEMAPHORE"
#define L_PC_LOC_LOCKPOS 0
static unsigned int l_pc_locpos[sizeof(L_PC_LOCPOS) * sizeof(int)] =
	{
	('_' - ' ') ^ 0x12345100,
	('S' - ' ')^ 0x12345200,
	('E'- ' ') ^ 0x12345300,
	('M'- ' ') ^ 0x12345400,
	('A' - ' ')^ 0x12345500,
	('P' - ' ')^ 0x12345600,
	('H' - ' ')^ 0x12345700,
	('O' - ' ')^ 0x12345700,
	('R' - ' ')^ 0x12345800,
	('E' - ' ')^ 0x12345900, 0 };

