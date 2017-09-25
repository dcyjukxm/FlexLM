#!/bin/sh
#red_s_tests 0
export NO_FLEXLM_HOSTNAME_ALIAS
export DEBUG_HOSTNAME
export DEBUG_LOCKFILE
NO_FLEXLM_HOSTNAME_ALIAS=""
DEBUG_HOSTNAME=flexlm_one 
DEBUG_LOCKFILE=/usr/tmp/lockone
lmgrd_r -c rlicense.dat1 -z -hang > rlog1  2>&1 &
DEBUG_HOSTNAME=flexlm_two
DEBUG_LOCKFILE=/usr/tmp/locktwo
lmgrd_r -c rlicense.dat2 -z > rlog2  2>&1 &
DEBUG_HOSTNAME=flexlm_three
DEBUG_LOCKFILE=/usr/tmp/lockthree
lmgrd_r -c rlicense.dat3 -z > rlog3  2>&1 &

