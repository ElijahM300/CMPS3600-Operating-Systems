#!/bin/sh

if test $# -eq 0
   then echo "Usage: cleanup.sh username"
   exit 1
fi

ME=$1

for semid in `ipcs -s | grep $ME | cut -f2 -d" "`; do ipcrm -s $semid; done
for msqid in `ipcs -q | grep $ME | cut -f2 -d" "`; do ipcrm -q $msqid; done
for shmid in `ipcs -m | grep $ME | cut -f2 -d" "`; do ipcrm -m $shmid; done
