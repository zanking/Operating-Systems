CS3753 (Operating Systems)
Spring 2016
University of Colorado Boulder
Programming Assignment 3


Nicholas Clement


Adopted from previous code by
 Chris Wailes <chris.wailes@gmail.com> - 2010
 Wei-Te Chen <weite.chen@colorado.edu> - 2011
 Blaise Barney - pthread-hello.c

---Folders---
input - names*.txt input files
handout - Assignment description and documentation

---Executables---
lookup - A basic non-threaded DNS query-er
queueTest - Unit test program for queue
pthread-hello ; A simple threaded "Hello World" program
multi-lookup - Threaded DNS query-er

---Examples---
Build:
 make

Clean:
 make clean

Lookup DNS info for all names files in input folder:
 ./multi-lookup input/names*.txt results.txt

Check queue for memory leaks:
 valgrind ./multi-lookup input/names*.txt results.txt

Run pthread-hello
 ./pthread-hello
