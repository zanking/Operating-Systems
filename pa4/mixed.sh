

#File: mixed.sh
#Author: Nicholas Clement
#Project: CSCI 3753 Programming Assignment 4
#Create Date: 2016/11/11
#Description:
# Bash script to test mixed CPU-I/O times with different scheduling processes

FORKA=5
FORKB=20
FORKC=100
BYTESTOCOPY=102400
BLOCKSIZE=1024
TIMEFORMAT="wall=%e user=%U system=%S CPU=%P i-switched=%c v-switched=%w"
MAKE="make -s"

echo Building code...
$MAKE clean
$MAKE

echo ----------------------------------------------------------------------------------1
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_OTHER with 5 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKA SCHED_OTHER 0
echo -
echo using SCHED_OTHER with 10 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKB SCHED_OTHER 0
echo -
echo using SCHED_OTHER with 100 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKC SCHED_OTHER 0
echo ----------------------------------------------------------------------------------2
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_OTHER with 5 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKA SCHED_OTHER 1
echo -
echo using SCHED_OTHER with 10 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKB SCHED_OTHER 1
echo -
echo using SCHED_OTHER with 100 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKC SCHED_OTHER 1
echo ----------------------------------------------------------------------------------3
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_FIFO with 5 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKA SCHED_FIFO 0
echo  -
echo using SCHED_FIFO with 10 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKB SCHED_FIFO 0
echo -
echo using SCHED_FIFO with 100 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKC SCHED_FIFO 0
echo ----------------------------------------------------------------------------------4
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_FIFO with 5 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKA SCHED_FIFO 1
echo -
echo using SCHED_FIFO with 20 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKB SCHED_FIFO 1
echo -
echo using SCHED_FIFO with 100 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKC SCHED_FIFO 1
echo ----------------------------------------------------------------------------------5
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_RR with 5 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKA SCHED_RR 0
echo -
echo using SCHED_RR with 20 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKB SCHED_RR 0
echo -
echo using SCHED_RR with 100 simultaneous process PRIORITY 0
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKC SCHED_RR 0
echo ----------------------------------------------------------------------------------6
# echo Copying $BYTESTOCOPY bytes in blocks of $BLOCKSIZE from rwinput to rwoutput
echo using SCHED_RR with 5 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKA SCHED_RR 1
echo -
echo using SCHED_RR with 20 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKB SCHED_RR 1
echo -
echo using SCHED_RR with 100 simultaneous process PRIORITY 1
/usr/bin/time -f "$TIMEFORMAT" ./cpu-io $BYTESTOCOPY $BLOCKSIZE $FORKC SCHED_RR 1
