/*
 * File: pi-sched.c
 * Author: Andy Sayler
 * Revised: Dhivakant Mishra
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: 2012/03/07
 * Modify Date: 2012/03/09
 * Modify Date: 2016/31/10
 * Description:
 * 	This file contains a simple program for statistically
 *      calculating pi using a specific scheduling policy.
 */

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>

#define FORKS 1

#define DEFAULT_ITERATIONS 100000000
#define RADIUS (RAND_MAX / 2)

static double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

static double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}


int main(int argc, char* argv[]){

    long i;
    long iterations;
    struct sched_param param;
    int policy;
    int forks;
    int niceness;
    int niceNumb;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;

    /* Process program arguments to select iterations and policy */
    /* Set default iterations if not supplied */
    if(argc < 2){
	iterations = DEFAULT_ITERATIONS;
    }
    /* Set default policy if not supplied */
    if(argc < 3){
	policy = SCHED_OTHER;
    }

    if (argc < 4){
      forks = FORKS;
      printf("Using default number of forks\n");
    }
    else{
      forks = atoi(argv[3]);
    //  printf("Using user specified forks\n");
      if(forks <= 1){
        fprintf(stderr, "Bad fork value\n");
        exit(EXIT_FAILURE);
      }
    }

    if (argc < 5){
      niceness = 0;
      printf("Not changing priority");
    }
    else{
      niceness = atoi(argv[4]);
      if (niceness != 1){
        printf("Not changing priority");
        niceness = 0;
      }
    }


    /* Set iterations if supplied */
    if(argc > 1){
	iterations = atol(argv[1]);
	if(iterations < 1){
	    fprintf(stderr, "Bad iterations value\n");
	    exit(EXIT_FAILURE);
	}
}

    /* Set policy if supplied */
    if(argc > 2){
	if(!strcmp(argv[2], "SCHED_OTHER")){
	    policy = SCHED_OTHER;
	}
	else if(!strcmp(argv[2], "SCHED_FIFO")){
	    policy = SCHED_FIFO;
	}
	else if(!strcmp(argv[2], "SCHED_RR")){
	    policy = SCHED_RR;
	}

    }

  param.sched_priority = sched_get_priority_max(policy);
    for ( int i = 0; i < forks; i++ ){
      if ( fork() == 0 )
        {
          if (niceness == 1){
            printf("niceness == 1");
            niceNumb ++;
            int test = nice(niceNumb);
            //error handling after decreasing priority
            if (test == -1){
              fprintf(stderr, "Unable to change niceness\n");
        	    exit(EXIT_FAILURE);
            }
          }

        if(sched_setscheduler(0, policy, &param)){
    	      perror("Error setting scheduler policy");
    	      exit(EXIT_FAILURE);
        }

        /* Calculate pi using statistical methode across all iterations*/
      for(i=0; i<iterations; i++){
    	x = (random() % (RADIUS * 2)) - RADIUS;
    	y = (random() % (RADIUS * 2)) - RADIUS;
    	if(zeroDist(x,y) < RADIUS){
    	    inCircle++;
    	}
    	inSquare++;
        }

        /* Finish calculation */
        pCircle = inCircle/inSquare;
        piCalc = pCircle * 4.0;

        /* Print result */
        fprintf(stdout, "pi = %f\n", piCalc);

        exit( 0 );
      }
    }

    for ( int i = 0; i < forks; i++ ){
        wait( NULL );
    }

    return 0;
}
