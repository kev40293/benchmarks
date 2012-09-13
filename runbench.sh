#!/bin/bash

if [ $# -gt "1" ]
then LOGFILE=$1
else
   LOGFILE=stdout
fi

function disk {
for i in B K M G
do
   ./diskbench -r -t -b$i | grep "diskbench"
   echo ./diskbench -r -b$i | grep "diskbench"
   echo ./diskbench -t -b$i | grep "diskbench"
   echo ./diskbench -b$i | grep "diskbench"
done
}

disk
