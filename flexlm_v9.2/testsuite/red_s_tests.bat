#!/bin/sh
#red_s_tests 0
set NO_FLEXLM_HOSTNAME_ALIAS=""
set DEBUG_HOSTNAME=flexlm_one
set DEBUG_LOCKFILE=\temp\lockone
lmgrd_r -c rlicense.dat1 -l rlog1   -exit
set DEBUG_HOSTNAME=flexlm_two
set DEBUG_LOCKFILE=\temp\locktwo
lmgrd_r -c rlicense.dat2 -l rlog2
set DEBUG_HOSTNAME=flexlm_three
set DEBUG_LOCKFILE=\temp\lockthree
lmgrd_r -c rlicense.dat3 -l rlog3

