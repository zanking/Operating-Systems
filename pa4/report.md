
#            Nicholas Clement

## CSCI 3753 Operating Systems Fall 2016

## Assignment Four


### Abstract:

The goal of this lab is to analyze the different runtimes and efficiencies of schedulers with different types of processes.  The different schedulers examined were the CFS, Round Robin, and First in First Out.  The different processes examined were CPU intensive, I/O intensive, and a combination of both CPU and I/O intensive tasks.  All of these were then tested with changing priority (otherwise referred to as niceness) and static priority.  The general results showed that CPU bound processes were much more expensive and time consuming compared to I/O processes for each scheduler.  In general, I/O was by far the least time consuming and mixed I/O CPU tests were less time consuming that pure CPU test but more time consuming than pure I/O tests.



### Introduction:

The purpose of this assignment is to investigate the behavior of different scheduling algorithms.  To do this I was tasked with creating different testing benchmarks, all of which I ran with the different scheduling algorithms assigned.  From these different benchmark tests I was able to conclude the corresponding efficiencies for each scheduling algorithm, and the pitfalls associated with that algorithm.

### Method (Experimental Design):

A total of fifty-four tests were preformed to accumulate data on the different scheduling algorithms and their corresponding efficiencies.  These tests were systematically divided into several subcategories including scheduler, priority, structure, and scheduling process.

##### Variable One: Schedulers

The goal of this lab is to investigate three different Unix scheduling algorithms and weigh the pros and cons of each scheduling algorithm.  Therefore this is the primary and first variable I was interested in testing with my different test cases.  

##### Variable Two: Benchmarks

I used three different structures of programs as benchmarks for the tests.  These structures included I/O bound programs (reading/writing), CPU bound programs (computation), and a mix of I/O and CPU bound programs.  These different base tests allowed me to analyze each scheduler and its performance for the basic necessities of computing.  

For the CPU bound program I used the provided statistical computation of pi.  I used one hundred million iterations, which seemed to be enough to thoroughly use the CPU.

    for(i=0; i<iterations; i++){
      x = (random() % (RADIUS * 2)) - RADIUS;
      y = (random() % (RADIUS * 2)) - RADIUS;
      if(zeroDist(x,y) < RADIUS){
          inCircle++;
      }
      inSquare++;
    }
    pCircle = inCircle/inSquare; piCalc = pCircle * 4.0;  

For the I/O bound program I used the provided rw file, and altered it to accept priority and create child processes.  This program read 102400 bytes from a single file and then generated a new random file to write the bytes to.  

    open(outputFilename,
         O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0)


      /* Read from input file and write to output file*/
      do{
    /* Read transfersize bytes from input file*/
    bytesRead = read(inputFD, transferBuffer, buffersize);
    if(bytesRead < 0){
        perror("Error reading input file");
        exit(EXIT_FAILURE);
    }
    else{
        totalBytesRead += bytesRead;
        totalReads++;
    }

The CPU-I/O mixed program first ran the statistical computation of pi over one hundred million iterations and then proceeded to preform the same I/O as described above.

Creating these different benchmark tests allowed me to further investigate how the three CPU schedulers handle different types of processes.

##### Variable Three: Number of Processes

I ran each benchmark test with a different number of child processes using the fork() systemcall.  The different numbers of processes I used were five, twenty, and one hundred.  My goal in changing the number of processes for each benchmark test was to see the scalability of the different schedulers.  In the results section I will discuss in more detail the behavior that is required for a scheduling algorithm to scale well.


##### Variable Four: Priority

Along with the base benchmark tests, I included priority.  When testing priority I analyzed static priority, and dynamic priority.  Static priority consisted of each process preforming the same task with the same priority.  When implementing dynamic priority I used the nice() systemcall to decrement the priority of each child process. The code is as follows:

    if (niceness == 1){
      printf("niceness == 1");
      niceNumb ++;
      int test = nice(niceNumb);
        //error handling
      if (test == -1){
        fprintf(stderr, "Unable to change niceness\n");
        exit(EXIT_FAILURE);
      }
    }

##### Overview

Starting with the schedulers we have three possibilities, followed by three different benchmark cases, followed by three different numbers of processes, followed by two different priorities.

3*3*3*2 = 54
