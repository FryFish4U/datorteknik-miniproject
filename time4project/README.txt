Rewritten Lab 3 file
Rewritten by Alvin Pettersson and David Haendler

to compile we used mcb32tools with the console commands:
. /opt/mcb32tools/environmnet
cd '{path to}/jammedspace'
make && make install TTDEV=/dev/ttyS# 

NOTE: ttyS# should be the port where the chipKIT is connected in 
(might need to use trial and error to find the correct port)