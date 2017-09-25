#. ~/.profile
export NO_FLEXLM_HOSTNAME_ALIAS
export DEBUG_HOSTNAME
export DEBUG_LOCKFILE
NO_FLEXLM_HOSTNAME_ALIAS=""
DEBUG_HOSTNAME=flexlm_one 
DEBUG_LOCKFILE=/usr/tmp/lockone
lmgrd_r -c rlicense.dat1 > rlog1
