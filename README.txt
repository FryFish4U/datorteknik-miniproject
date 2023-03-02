Rewritten Lab 3 file
Rewritten by Alvin Pettersson and David Haendler

to compile we used mcb32tools with the console commands:
. /opt/mcb32tools/environmnet
make && make install TTDEV=/dev/ttyS# 

NOTE: 
you need to path to where the folder is stored on you computer before running make

ttyS# should be the port where the chipKIT is connected in 
(might need to use trial and error to find the correct port)