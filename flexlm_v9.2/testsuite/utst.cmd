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
 *	Module: $Id: utst.cmd,v 1.1 2000/03/14 02:38:31 daniel Exp $
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
SETENV 	FLEXLM_INTERVAL_OK=1
PRINT	servtest -- allow about 15 minutes
CMD	servtest 0 -o junk.out
CMD	lmgrd -c nosuch.dat -l _log1
CMD	lmgrd -c pathtest.dat -l _log2
SLEEP	20
CMD	lmgrd -c . -l _log3
SLEEP	45
CMD     servtest -o $CMDCNT.st.out
CMD	lmdown -c 27001@localhost -q -o junk.out
CMD	lmdown -c 27002@localhost -q -o junk.out
CMD	lmdown -c 27000@localhost -q -o junk.out
CHECK	$CMDCNT.st.out $CMDCNT.log MATCH

