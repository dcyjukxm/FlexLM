/******************************************************************************

	    COPYRIGHT (c) 1998 by Globetrotter Software Inc.
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
 *	Module: $Id: utborr.cmd,v 1.1.1.1 1999/01/13 19:33:49 daniel Exp $
 *
 *	D. Birns
 *	10/98
 *
 *	Last changed:  5/27/98
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

SETENV	FLEXLM_INTERVAL_OK=1
DELIM	
CMD	servtest 0 -o junk.out
CMD	borrtest 0
BKG_CMD	lmgrd -c borrow.dat -l borrow.log
SLEEP 	5
CMD	borrtest -o $CMDCNT.out
CMD	lmdown -q -c borrow.dat -o lmdown.out
CHECK	$CMDCNT.out $CMDCNT.log
