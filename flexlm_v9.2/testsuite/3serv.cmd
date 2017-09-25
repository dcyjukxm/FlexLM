/******************************************************************************

	    COPYRIGHT (c) 2001 by Globetrotter Software Inc.
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
 *	Module: $Id: 3serv.cmd,v 1.6 2003/01/23 18:57:28 jwong Exp $
 *
 *	D. Birns
 *	August 2001
 *
 */

/*
 *	OPERATIONAL NOTES FOR WINDOWS (1/23/03):
 *
 *		Add to .\winnt\system32\drivers\etc\hosts file the following entries.
 * 		These entries are needed in order to successfully activate servers on 
 *		a single box:
 *			127.0.0.1 localhost
 *			127.0.0.1 loghost
 *			127.0.0.1 flexlm_one
 * 			127.0.0.1 flexlm_two
 *			127.0.0.1 flexlm_three
 *			192.156.198.254 flexlm_nowhere
 *		These settings can be verified with the PING utility.
 *
 *		Also make sure that your box reads the hosts file by making sure that the
 *		"Enable LMHOSTS lookup" is checked in the tcp/ip's advanced settings in the
 *		network configrations area. Also use the "Import LMHOSTS..." button to
 *		register the updates into windows lookup tables. REBOOT is manatory after this
 *		change!! 
 *		
 *		The DEBUG_LOCKFILE will change the default lockfile location and filename.
 *		This code change is located in the .\app\ls_app_init.c.
 *		
 *		The DEBUG_LOCKFILE settings are also going to apply to renaming of the
 *		semaphore in the demo vendor daemon. 
 *		(.\testsuite\demo.c & .\testsuite\pc.mak[demo_r.exe macro])
 *
 *		Need to update strings for major version changes in order to avoid false
 *		positive failures in testsuite. (3serv.exp)
 */

SETENV  LM_TSUITE=1
SETENV	FLEXLM_INTERVAL_OK=1
CMD	red_s_tests 0
SETENV	NO_FLEXLM_HOSTNAME_ALIAS=1
SETENV	DEBUG_HOSTNAME=flexlm_one
SETENV	DEBUG_LOCKFILE=lockone
CMD	lmgrd_r -c rlicense.dat1 -l rlog1 -exit 15 -background
SETENV	DEBUG_HOSTNAME=flexlm_two
SETENV 	DEBUG_LOCKFILE=locktwo
CMD	lmgrd_r -c rlicense.dat2 -l rlog2 -background
SETENV	DEBUG_HOSTNAME=flexlm_three
SETENV	DEBUG_LOCKFILE=lockthree
CMD	lmgrd_r -c rlicense.dat3 -l rlog3 -background
SETENV  DEMO_LICENSE_FILE=rlicense.dat1
SLEEP	10

DELIM
PRINT	Status
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH

DELIM
CHECKOUT f1
PRINT	Status with license checked out
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH

DELIM
PRINT	master exits
SLEEP	120
CHECKOUT f1
PRINT	Status with license checked out
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH

DELIM
PRINT	restart first server
SETENV	DEBUG_HOSTNAME=flexlm_one
SETENV	DEBUG_LOCKFILE=lockone
CMD	lmgrd_r -c rlicense.dat1 -l rlog1 -background
SLEEP	10
CHECKOUT f1
PRINT	Status with license checked out
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECKIN f1

DELIM
SLEEP	120
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECKOUT f1

DELIM
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH


DELIM
PRINT	Shutdown
CMD	lmdown -q -c rlicense.dat1 -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH
SLEEP	60

DELIM
PRINT	Status with server down
CMD	lmstat -a -c rlicense.dat1 -o $CMDCNT.out
CHECK	$CMDCNT.out $CMDCNT MATCH

