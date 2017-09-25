/******************************************************************************

	    COPYRIGHT (c) 1994, 1999 by Globetrotter Software Inc.
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
 *	Module: $Id: utnoserv.cmd,v 1.12 2003/01/20 23:43:01 sluu Exp $
 *
 *	D. Birns
 *	3/22/95
 *
 *	Last changed:  1/9/99
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

SETENV LM_TSUITE=1
DELIM
PRINT 	Expect lmcrypt errors to stderr  
PRINT		(creating utiltest.dat and rm at end)
RM-F	utiltest.dat
RM-F	license.dat
FPRINT	utiltest.dat SERVER noserv 08002b32b161 2837
FPRINT	utiltest.dat DAEMON demo demo
FPRINT	utiltest.dat FEATURE f1 demo 1.050 01-jan-2010 9 start:1-jan-94 "Vendor String Goes Here"
FPRINT	utiltest.dat FEATURE f0.a demo 1.050 01-jan-2010 0 start:1-jan-94 VENDOR_STRING="Vendor String Goes Here" FOO=bar ck=1
CMD	lmcrypt -i utiltest.dat -o utn1.out -e utn1.err
CHECK	utn1.out $CMDCNT MATCH
CHECK	utn1.err $CNDCNT.err MATCH
RM	utiltest.dat

DELIM
CMD	lmdiag -n -c utn1.out -o utn3.out
CHECK	utn3.out 3 MATCH

DELIM
CMD	lmhostid -o utn4a.out
PRINT 	Expect 2 different hostids
CHECK	utn4a.out 4a MATCH

DELIM
CMD	lmhostid -n -o utn4b.out
PRINT 	Expect 2 different hostids
CHECK	utn4b.out 4b MATCH

DELIM
PRINT	Expect the correct hostid
CMD	lmhostid -n

DELIM
PRINT	Expect an error msg to stderr:
CMD	lmdown -q -c utn1.out -o utn5.out
CHECK	utn5.out 5 MATCH

DELIM
PRINT	no server test 
CMD	lmremove -c utn1.out f1 daniel utiltest /dev/tty0 -o utn6a.out
CHECK	utn6a.out 6a MATCH

DELIM
CMD	lmremove -c utn1.out -h f1 utiltest 2837 101 -o utn6b.out
CHECK	utn6b.out 6b MATCH

DELIM
CMD	lmremove -c utn1.out f1 -o utn6c.out
CHECK	utn6c.out 6c MATCH

DELIM 
PRINT	no server test 
CMD	lmreread -c utn1.out -o utn7.out
CHECK	utn7.out 7 MATCH

DELIM
PRINT	no server test 
CMD	lmstat -c utn1.out -o utnstat1.out
CHECK	utnstat1.out lmstat1 MATCH

DELIM
CMD	lmutil lmver demo -o utnver.out
CHECK	utnver.out lmver MATCH

DELIM
PRINT	Simple test
CMD	makekey createl1.dat -i create1.inp -redirect
CHECK	createl1.dat create1 MATCH

DELIM
PRINT	Increment, Upgrade, and long daemon path
CMD	makekey createl2.dat -i create2.inp -redirect
CHECK	createl2.dat create2 MATCH

DELIM
PRINT	long feature name, funny version
CMD	makekey create3.dat -i create3.inp -redirect
CHECK	create3.dat $CMDCNT.log MATCH

DELIM
PRINT	Repeat tests on valid file with server down (alt2.lic is smaller)
CMD	lmdiag -n -c alt.2.lic -o utr3.out
IFUNIX
CHECK	utr3.out r3 MATCH
ENDIF

DELIM
PRINT	Expect an error msg to stderr:
CMD	lmdown -q -c . -o utr5.out
CHECK	utr5.out r5 MATCH

DELIM
PRINT	no server test 
CMD	lmremove -c . f1 daniel utiltest /dev/tty0 -o utr6a.out
CHECK	utr6a.out r6a MATCH

DELIM
CMD	lmremove -c . -h f1 utiltest 2837 101 -o utr6b.out
CHECK	utr6b.out r6b MATCH

DELIM
CMD	lmremove -c . f1 -o utr6c.out
CHECK	utr6c.out r6c MATCH

DELIM 
PRINT	no server test 
CMD	lmreread -c . -o utr7.out
CHECK	utr7.out r7 MATCH

DELIM
PRINT	no server test 
CMD	lmstat -c . -o utrstat1.out
CHECK	utrstat1.out rlmstat1 MATCH

DELIM
CMD	lmstat -a -c . -o utrstat2.out
CHECK	utrstat2.out rlmstat2 MATCH

DELIM
CMD	lmstat -c . -f f1 -o utrstat3.out
CHECK	utrstat3.out rlmstat3 MATCH

DELIM
CMD	lmstat -c . -i f1 -o utrstat4.out
CHECK	utrstat4.out rlmstat4 MATCH

DELIM
CMD	lmstat -c . -A -o utrstat5.out
CHECK	utrstat5.out rlmstat5 MATCH

DELIM
CMD	lmstat -c . -S demo -o utrstat6.out
CHECK	utrstat6.out rlmstat6 MATCH

DELIM
CMD	lmstat -c . -s utiltest -o utrstat7.out
CHECK	utrstat7.out rlmstat7 MATCH

DELIM
CMD	lmstat -c . -t 10 -o utrstat8.out
CHECK	utrstat8.out rlmstat8 MATCH

DELIM
CMD	lmstat -c @nosuchhost -o utrstat9.out
CHECK	utrstat9.out rlmstat9 MATCH

DELIM
CMD	lmstat -c @nosuchhost -o utrstata.out -verbose
CHECK	utrstata.out rlmstata MATCH

DELIM
PRINT	Test lmcrypt decimal conversion
CMD	lmcrypt -i dec.tst -o dec1.out -decimal -e dec1.err
CHECK	dec1.out dec1 MATCH
CHECK	dec1.err $CMDCNT.err MATCH

DELIM
CMD	lmcrypt -i dec1.out -o dec2.out 
CHECK	dec2.out dec2 MATCH

DELIM
PRINT	lminstall decimal conversion
CMD	lminstall -i dec.tst -o dec3.out  -odecimal
CHECK	dec3.out dec3 MATCH

DELIM
CMD	lminstall -i dec1.out -o dec4.out 
CHECK	dec4.out dec4 MATCH

DELIM
CMD	lminstall -i dec1.out -o $CMDCNT.out  -e $CMDCNT.err -overfmt 5.1
CHECK	$CMDCNT.out $CMDCNT.log MATCH

DELIM
CMD	lminstall -i dec.tst -o $CMDCNT.out  -e $CMDCNT.err -overfmt 4
CHECK	$CMDCNT.out $CMDCNT.log MATCH

DELIM
CMD	lminstall -i dec.tst -o $CMDCNT.out  -e $CMDCNT.err 
CHECK	$CMDCNT.out $CMDCNT.log MATCH

DELIM
PRINT	Expect some differences, but not with 'demo'
CMD	lmpath -o $CMDCNT1.out -override demo ifthisisdifferentitisanerror 
CMD	lmpath -status -o $CMDCNT2.out  -e $CMDCNT.err 
CHECK	$CMDCNT1.out $CMDCNT1.log MATCH
CHECK	$CMDCNT2.out $CMDCNT2.log MATCH
