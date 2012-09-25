#!/bin/bash

if [ $# -gt "1" ]
then LOGFILE=$1
else
   LOGFILE=stdout
fi

function disk {
for i in B K M G
do
   ./diskbench -r -t -b$i | grep "DISKBENCH"
   ./diskbench -r -b$i | grep "DISKBENCH"
   ./diskbench -t -b$i | grep "DISKBENCH"
   ./diskbench -b$i | grep "DISKBENCH"
done
}

function net {
   for i in 1B 1K 64K
       do
          ./netbench -b$i | grep "NETBENCH"
          ./netbench -b$i -p | grep "NETBENCH"
          ./netbench -b$i -u | grep "NETBENCH"
          ./netbench -b$i -u -p| grep "NETBENCH"
       done
       }
       
function mem {
    for i in  1B 1K 1M
        do
            for j in 1 2 3
            do
            ./membench -b $i -m $j -t 1 | grep "MEMBENCH"  
            ./membench -b $i -m $j -t 2 | grep "MEMBENCH"           
            done
        done
}        

function cpub {
  ./cpubench | grep "CPUBENCH"
}

#disk
#net
mem
cpub
