#! /bin/sh


d=10 
t=0
while [ $d -ne 17 ]
do
    t=0
    while [ $t - ne 8 ]
    do
    ./preProcess -s rm_rokka19_200803$d__s_000$t.kvsml -outname 200803$d_s_000$t.kvsml
    t=`expr $t + 1`
    done
d=`expr $d + 1`
done

d=10 
t=0
step=00
while [ $d -ne 17 ]
do
    t=0
    while [ $t - ne 8 ]
    do
    ./compresion -f 200803$d_s_000$t.kvsml -t 0315s.kvsml -sp 3 -PBVR -Bspline -b 2 -outpoint s_$step.kvsml
    t=`expr $t + 1`
    step=`expr $step + 1`
    done
d=`expr $d + 1`
done