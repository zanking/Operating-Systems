#/!/bin/bash

#File: testscript
#Author: Andy Sayler
#Revised: Shivakant Mishra
#Project: CSCI 3753 Programming Assignment 4
#Create Date: 2016/30/10
#Modify Date: 2016/30/10
#Description:
#	A simple bash script to run a signle copy of each test case
#	and gather the relevent data.

ITERATIONS=100000000
FORKA=5
FORKB=20
FORKC=100
PRIORITY=1
BYTESTOCOPY=102400
BLOCKSIZE=1024
TIMEFORMAT="wall=%e user=%U system=%S CPU=%P i-switched=%c v-switched=%w"
MAKE="make -s"

echo Building code...
$MAKE clean
$MAKE

echo ----------------------------------------------------------------------------------1
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_OTHER with 5 simultaneous process...
/usr/bin/time -f "$TIMEFORMAT" ./rw $BYTESTOCOPY $BLOCKSIZE $FORKA

echo using SCHED_OTHER with 10 simultaneous process...
/usr/bin/time -f "$TIMEFORMAT" ./rw $BYTESTOCOPY $BLOCKSIZE $FORKB

echo using SCHED_OTHER with 100 simultaneous process...
/usr/bin/time -f "$TIMEFORMAT" ./rw $BYTESTOCOPY $BLOCKSIZE $FORKC
