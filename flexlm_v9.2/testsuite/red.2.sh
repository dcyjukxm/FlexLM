#. ~/.profile
export NO_FLEXLM_HOSTNAME_ALIAS
export DEBUG_HOSTNAME
export DEBUG_LOCKFILE
NO_FLEXLM_HOSTNAME_ALIAS=""
export LM_LICENSE_FILE
DEBUG_HOSTNAME=flexlm_two
DEBUG_LOCKFILE=/usr/tmp/locktwo
lmgrd_r -c rlicense.dat2 >> rlog2

