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
 *	Module: $Id: utflex.cmd,v 1.17 2001/08/10 00:17:46 daniel Exp $
 *
 *	D. Birns
 *	10/98
 *
 *	Last changed:  12/8/98
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
SETENV	FLEXLM_BATCH=1
SETENV	LM_TSUITE=1
DELIM
CMD	basic_tests -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT.log EXACT

#DELIM
#
#IFUNIX
#
#PRINT	takes about 2 minutes
#CMD	timers_test -o $CMDCNT.out
#CHECK	$CMDCNT.out $CMDCNT.log EXACT
#
#ENDIF 	#UNIX

DELIM
PRINT	Test without server
CMD	no_s_tests -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT.log EXACT

DELIM
PRINT	Test *with* server
CMD	no_s_tests -liconly 
CMD	lmdown -c license.dat -q -o junk.out
CMD	lmgrd -c license.dat -l _log 
SLEEP 	15
CMD	no_s_tests -php -o $CMDCNT.out
CMD	lmdown -c license.dat -q -o junk.out
CHECK	$CMDCNT.out $CMDCNT.log EXACT
SLEEP 	10

DELIM
CMD	lock_tests -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT.log EXACT
ENDIF


