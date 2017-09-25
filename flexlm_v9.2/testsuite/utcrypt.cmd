/******************************************************************************

	    COPYRIGHT (c) 1994,1998 by Globetrotter Software Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Globetrotter Software Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Globetrotter Software Inc.

 *****************************************************************************/
/*	
 *	Module: $Id: utcrypt.cmd,v 1.9 2000/07/10 22:57:31 daniel Exp $
 *	D. Birns
 *	3/22/95
 *
 *	Last changed:  11/2/98
 *
 */
Format is 
	CMD  FLEXlm command
	CHECK  args are:
		1) file, against
		2) expected result in utiltest.exp
		3) optional -- which test type to use
	PRINT	msg
	DELIM	Prints "--------------------------" and increments counter

	Everything else is comment

DELIM
PRINT	Compare 11000 license-keys with base
CMD	cryptest -s -outfile utcrypt1.out
CHECK	utcrypt1.out crypt1 EXACT

DELIM
PRINT 	Verify hostid sort order.
CMD	cryptest -o -outfile utcrypt2.out
CHECK	utcrypt2.out crypt2 EXACT

DELIM
PRINT	Encrypt with varying ethernet hostids
CMD	cryptest -h -outfile utcrypt3.out
CHECK	utcrypt3.out crypt3 EXACT

DELIM
PRINT	Version 3 test
CMD	cryptest -3 -outfile utcrypt4.out
CHECK	utcrypt4.out crypt4 EXACT

DELIM
PRINT	Version 3 test, using lc_cryptstr()
CMD	cryptest -3+ -outfile utcrypt5.out
CHECK	utcrypt5.out crypt5 EXACT

DELIM
PRINT	Version 4 test
CMD	cryptest -4 -outfile utcrypt6.out
CHECK	utcrypt6.out crypt6 EXACT

DELIM
PRINT	Version 5 test
CMD	cryptest -5 -outfile utcrypt7.out
CHECK	utcrypt7.out crypt7 EXACT


DELIM
PRINT	Rerun tests with short keys (v6)
PRINT	Compare 11000 license-keys with base
CMD	cryptest -s -outfile utcryp61.out -verfmt6
CHECK	utcryp61.out cryp61 EXACT

DELIM
PRINT 	Verify hostid sort order.
CMD	cryptest -o -outfile utcryp62.out -verfmt6
CHECK	utcryp62.out cryp62 EXACT

DELIM
PRINT	Encrypt with varying ethernet hostids
CMD	cryptest -h -outfile utcryp63.out -verfmt6
CHECK	utcryp63.out cryp63 EXACT

DELIM
PRINT	Version 3 test
CMD	cryptest -3 -outfile utcryp64.out -verfmt6
CHECK	utcryp64.out cryp64 EXACT

DELIM
PRINT	Version 3 test, using lc_cryptstr()
CMD	cryptest -3+ -outfile utcryp65.out -verfmt6
CHECK	utcryp65.out cryp65 EXACT

DELIM
PRINT	Version 4 test
CMD	cryptest -4 -outfile utcryp66.out -verfmt6
CHECK	utcryp66.out cryp66 EXACT

DELIM
PRINT	Version 5 test
CMD	cryptest -5 -outfile utcryp67.out -verfmt6
CHECK	utcryp67.out cryp67 EXACT

DELIM
PRINT	Version 6 test
CMD	cryptest -6 -outfile utcrypt68.out 
CHECK	utcrypt68.out crypt68 EXACT

DELIM
PRINT	Rerun tests with user_crypt_func (v6)
PRINT	Compare 11000 license-keys with base
CMD	cryptest -s -filter -outfile utcryptx1.out
CHECK	utcryptx1.out cryptx1 EXACT

DELIM
CMD	cryptest -s -filter -outfile utcryptx2.out -verfmt6
CHECK	utcryptx2.out cryptx2 EXACT

DELIM
PRINT 	Verify hostid sort order.
CMD	cryptest -o -filter -outfile utcrypx62.out -verfmt6
CHECK	utcrypx62.out crypx62 EXACT

DELIM
PRINT	Encrypt with varying ethernet hostids
CMD	cryptest -h -filter -outfile utcrypx63.out -verfmt6
CHECK	utcrypx63.out crypx63 EXACT

DELIM
PRINT	Version 3 test
CMD	cryptest -3 -filter -outfile utcrypx64.out -verfmt6
CHECK	utcrypx64.out crypx64 EXACT

DELIM
PRINT	Version 3 test, using lc_cryptstr()
CMD	cryptest -3+ -filter -outfile utcrypx65.out -verfmt6
CHECK	utcrypx65.out crypx65 EXACT

DELIM
PRINT	Version 4 test
CMD	cryptest -4 -filter -outfile utcrypx66.out -verfmt6
CHECK	utcrypx66.out crypx66 EXACT

DELIM
PRINT	Version 5 test
CMD	cryptest -5 -filter -outfile utcrypx67.out -verfmt6
CHECK	utcrypx67.out crypx67 EXACT

DELIM
PRINT	Version 6 test
CMD	cryptest -6 -filter -outfile utcryptx68.out 
CHECK	utcryptx68.out cryptx68 EXACT

DELIM
PRINT	Version 7 test
CMD	cryptest -7.1 -outfile $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT EXACT

DELIM
CMD	crotest -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT REPLOG

