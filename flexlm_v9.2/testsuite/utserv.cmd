/******************************************************************************

	    COPYRIGHT (c) 1994, 1998 by Globetrotter Software Inc.
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
 *	Module: $Id: utserv.cmd,v 1.10.26.1 2003/07/08 15:27:13 brolland Exp $
 *
 *	D. Birns
 *	3/22/95
 *
 *	Last changed:  10/29/98
 *
 */
	
SETENV	FLEXLM_INTERVAL_OK=1
SETENV	LM_TSUITE=1
CMD	servtest 0 -o junk.out
CMD	lmgrd -c . -l _log3  -background

IFWIN
SLEEP 	15
ENDIF

IFUNIX
SLEEP 	30
ENDIF


DELIM	
PRINT	general status:
CMD	lmutil lmstat -c . -o utsstat1.out
CHECK	utsstat1.out stat1 MATCH

DELIM
PRINT	-a with a feature checked out
CHECKOUT f1
CMD	lmutil lmstat -c . -a -o utsstat2.out
CHECK	utsstat2.out stat2 MATCH


DELIM
PRINT	-f for feature checked out
CMD	lmutil lmstat -c . -f f1 -o utsstat3.out
CHECK	utsstat3.out stat3 MATCH

DELIM
PRINT	Check nonexistent feature
CMD	lmutil lmstat -c . -f foobar -o utsstat4.out
CHECK	utsstat4.out stat4 MATCH

DELIM
SLEEP	15
CMD	lmremove -c . -h f1 $HOSTNAME 27000 101  -o utrem1.out
CHECK	utrem1.out lmremove1 MATCH

DELIM
CMD	lmutil lmstat -c . -f f1 -o utsstat5.out
CHECK	utsstat5.out stat5 MATCH

DELIM
CHECKOUT f2
SLEEP	15
CMD	lmremove -c . f2 $USERNAME $HOSTNAME $DISPLAY -o utrem2.out
CHECK	utrem2.out lmremove2 MATCH

DELIM
CHECKOUT f3
PRINT	Testing not enough time!
CMD	lmremove -c . f3 $USERNAME $HOSTNAME $DISPLAY -o utrem3.out
CHECK	utrem3.out lmremove3 MATCH

DELIM
PRINT	Testing not checked out!
CMD	lmremove -c . f1 $USERNAME $HOSTNAME $DISPLAY -o utrem4.out
CHECK	utrem4.out lmremove4 MATCH

DELIM
CMD	lmutil lmstat -c . -f f1 -o utsstat6.out
CHECK	utsstat6.out stat6 MATCH

DELIM
CMD	lmdiag f3 -c . -n -o utsdiag.out
CHECK	utsdiag.out diag MATCH

DELIM
CMD	lmdiag -n -c alt2.lic -n -o uts100.out
CHECK	uts100.out uts100 MATCH

DELIM
CMD	lmreread -c . -o uts101.out
SLEEP	15
CHECK	uts101.out uts101 MATCH

DELIM
CMD	lmutil lmstat -c . -i f1 -o uts102.out
CHECK	uts102.out uts102 MATCH

DELIM
CMD	lmutil lmstat -c . -A -o uts103.out
CHECK	uts103.out uts103 MATCH

DELIM
CMD	lmutil lmstat -c . -S demof -o uts104.out
CHECK	uts104.out uts104 MATCH

DELIM
CMD	lmutil lmstat -c . -s $HOSTNAME -o uts105.out
CHECK	uts105.out uts105 MATCH

DELIM
CMD	lmutil lmstat -c . -t 10 -o uts106.out
CHECK	uts106.out uts106 MATCH

DELIM
CMD	lmborrow demo $TODAY -o $CMDCNT.1.out
CHECKOUT f15-1a
CMD	lmborrow -status -o $CMDCNT.out
CMD	lmborrow -clear -o $CMDCNT.2.out
FREEJOB
CHECKOUT f15-1a
CMD	lmutil lmstat -c . -f f15-1a -o $CMDCNT.3.out
CLEARBORROW
FREEJOB
CHECK	$CMDCNT.out $CMDCNT.log MATCH
CHECK	$CMDCNT.3.out $CMDCNT.3.log MATCH
CHECK	$CMDCNT.1.out $CMDCNT.1.log MATCH

DELIM
CMD	lmswitch -c . demo demodebug.log -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH

CMD	lmdown -q -n -force -c . -o utslmdown.out 
CHECK	utslmdown.out lmdown MATCH

DELIM


CMD	lmgrd -c p6177.dat -l $CMDCNT.log  -background
SETENV  DEMO_LICENSE_FILE=p6177.dat
SLEEP	30
CMD	lmdown -q -n -force -c . -o $CMDCNT.out 
CHECK	$CMDCNT.log $CMDCNT MATCH
