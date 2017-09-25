#!/bin/sh
if [ "$FLEXLM_INTERVAL_OK" = "" ]
then
	echo "FLEXLM_INTERVAL_OK not set, exiting"
	exit 1
fi

rm -f $HOME/.flexlmrc
rm -f demo.lic

servtest 0
lmgrd -c nosuch.dat -l log1; lmgrd -c pathtest.dat -l log2; lmgrd -c . -l log3 
