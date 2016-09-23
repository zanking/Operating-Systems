# Operating-Systems Homework 1
CSCI 3753

##Personal Information:
Nicholas Clement  
Email: Nicholas.clement@colorado.edu  
  
## Information on Files:
arch/x86/kernel/Simpleadd.c:  This is where I built the actual system call, defined its parameters and steps to completion.  
  
arch/x86/kernel/Makefile:  Here I added my simpleadd.o so that it could be compiled and added to the kernel.  
  
arch/x86/syscalls/syscall_64.tbl:  This is where I added the system call to the system table of the kernel.  
  
include/linux/syscalls.h:  Contains the function prototype of the simpleadd syscall.  
  
/var/log/syslog:  The last few entries in the syslog show that the syscall is functional and working.  
  
source code (implement.c):  This is where I actuall call the system function that I built and pass in two numbers to be added.  
  
##Testing  
To test my source code replace all files in the specified directories.  The run the compiled implement.c (a.out if you dont want to compile it)
