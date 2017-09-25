servtest 0
lmgrd -c nosuch.dat -l log1
lmgrd -c pathtest.dat -l log2
sleep 2
if exist demo.lic del demo.lic
lmgrd -c . -l log3 

