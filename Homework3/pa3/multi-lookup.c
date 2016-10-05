/*
 * File: multi-lookup.c
 * Author: Nicholas Clement
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 10/4/2016
 * Modify Date: 10/4/2016
 * Description:
 * 	threaded solotuion to dnslookup
 *
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <errno.h>

 #include "util.h"

 #define MINARGS 3
 #define MAXARGS 11
 #define MAX_RESOLVER_THREADS 10
 #define MIN_RESOLVER_THREADS 2
 #define MAX_NAME_LENGTH 1025
 #define USAGE "<inputFilePath> <outputFilePath>"
 // #define SBUFSIZE 1025
 #define INPUTFS "%1024s"

int main(int argc, char* argv[]){

  /* Local Vars */
  pthread_t *th1, *th2, *th3;

  FILE* inputfp = NULL;
  FILE* outputfp = NULL;
  char hostname[SBUFSIZE];
  char errorstr[SBUFSIZE];
  char firstipstr[INET6_ADDRSTRLEN];
  int i;

  // init the different threads

  th1 = (pthread_t *) malloc(sizeof(pthread_t));
  if (pthread_create(th1, NULL, odd, (void *)&MAX_NAME_LENGTH)) {
    perror("error creating the first thread");
    exit(1);
  }
  printf("created the first thread\n");

  /* Check Arguments */
  if(argc < MINARGS || argc > MAXARGS){
fprintf(stderr, "Invalid number of arguments: %d\n", (argc - 1));
fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
return EXIT_FAILURE;
  }


  /* Open Output File */
  outputfp = fopen(argv[(argc-1)], "w");
  if(!outputfp){
    perror("Error Opening Output File");
    return EXIT_FAILURE;
  }





}
