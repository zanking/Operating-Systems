/*
 * File: pi.c
 * Author: Andy Sayler
 * Revised: Shivakant Mishra
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: 2012/03/07
 * Modify Date: 2012/03/09
 * Modify Date: 2016/31/10
 * Description:
 * 	This file contains a simple program for statistically
 *      calculating pi.
 */

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

/* Local Defines */
#define DEFAULT_ITERATIONS 1000000
#define RADIUS (RAND_MAX / 2)
#define FORKS 1

/* Local Functions */
static double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

static double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

int main(int argc, char* argv[]){

    long i;
    long iterations;
    int forks; //number of times to fork parent process
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;

    /* Process program arguments to select iterations */
    /* Set default iterations if not supplied */
    if(argc < 2){
	iterations = DEFAULT_ITERATIONS;
    }
    /* Set iterations if supplied */
    else{
	iterations = atol(argv[1]);
	if(iterations < 1){
	    fprintf(stderr, "Bad iterations value\n");
	    exit(EXIT_FAILURE);
	}
    }
    ///////////////////////////////////////////////////////////////////////
    /*set number of forks if passed in argv*/
    if (argc < 3){
      forks = FORKS;
      printf("Using default number of forks\n");
    }
    else{
      forks = atoi(argv[2]);
    //  printf("Using user specified forks\n");
      if(forks <= 1){
        fprintf(stderr, "Bad fork value\n");
        exit(EXIT_FAILURE);
      }
    }
/////////////////////////////////////////////////////////////////////////////
    /* begin forks*/

    // printf( "[dad] pid %d\n", getpid() );

    for ( int i = 0; i < forks; i++ ){
      if ( fork() == 0 )
        {

        /* Process

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



        /*wait for the kids*/

        //printf( "[son] pid %d from pid %d | ", getpid(), getppid() );
      //  printf("pi = %f \n", piCalc);
        exit( 0 );
      }
    }

    /*Parent waits for children*/
    for ( int i = 0; i < forks; i++ ){
        wait( NULL );
    }


    return 0;

}
