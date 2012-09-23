#! /bin/sh


d=10 
while [ $d -ne 17 ]
do
    t=0
    while [ $t -ne 8 ]
    do
./preProcess -s rm_rokka19_200803${d}__s_000$t.kvsml -outname pre/200803${d}_s_000$t.kvsml
    t=`expr $t + 1`
    done
d=`expr $d + 1`
done

d=10 
step=00
while [ $d -ne 17 ]
do
    t=0
    while [ $t -ne 8 ]
    do
./compresion -f pre/200803${d}_s_000$t.kvsml -t 0315s.kvsml -sp 3 -PBVR -Bspline -b 2 -outpoint PBVR/s_$step.kvsml
    t=`expr $t + 1`
    step=`expr $step + 1`
    done
d=`expr $d + 1`
done