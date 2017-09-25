servtest 0
lmgrd -c nosuch.dat -l log1
lmgrd -c pathtest.dat -l log2
sleep 5
lmgrd -c . -l log3 -local
sleep 5
                                      


