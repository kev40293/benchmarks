#!/bin/bash

if [ $# -gt "1" ]
then LOGFILE=$1
else
   LOGFILE=stdout
fi

function net {
   for i in 1B 1K 64K
       do
          ./netbench -b$i | grep "NETBENCH"
          ./netbench -b$i -p | grep "NETBENCH"
          sleep 1
       done
   for i in 1B 1K 64K
       do
          ./netbench -b$i -u | grep "NETBENCH"
          ./netbench -b$i -u -p| grep "NETBENCH"
          sleep 1
       done
}

net
