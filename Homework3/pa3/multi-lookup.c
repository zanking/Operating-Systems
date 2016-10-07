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
 #include "queue.h"
 #define MINARGS 3
 #define MAXARGS 10
 #define MAX_RESOLVER_THREADS 11 //set to 11 because up to 10 input files
 #define MIN_RESOLVER_THREADS 2
 #define MAX_NAME_LENGTH 1025
 #define SBUFSIZE 1025
 #define USAGE "<inputFilePath> <outputFilePath>"
 // #define SBUFSIZE 1025
 #define INPUTFS "%1024s"


void * openInput(char * argv[], i){
  inputfp = fopen(argv[i], "r");
  if(!inputfp){
      sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
      perror(errorstr);
      break;
  }

  ///////////////////////////////////
  /* Read File and Process*/
  while(fscanf(inputfp, INPUTFS, hostname) > 0){

      /* Lookup hostname and get IP string */
      if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
         == UTIL_FAILURE){
    fprintf(stderr, "dnslookup error: %s\n", hostname);
    strncpy(firstipstr, "", sizeof(firstipstr));
      }

      /* Write to Output File */
    queue_push(&q, payload_in[i]), hostname, firstipstr);
  }

  /* Close Input File */
  fclose(inputfp);
    }
}




int main(int argc, char* argv[]){

  /* Local Vars */
  // pthread_t *th1, *th2, *th3;
  const int qSize = SBUFSIZE; //define the size of our queue
  numReqThreads = argc - 1; //take the number of command line args and subtract 2 for program name and output name

  FILE* inputFile[numReqThreads]; //create an array of pointers the size of the number of INPUTFS
  pthread_t threads[numReqThreads];
  FILE* outputfp = NULL;  //single output file
  char hostname[SBUFSIZE];
  char errorstr[SBUFSIZE];
  char firstipstr[INET6_ADDRSTRLEN];
  int i;
  int rc;
  queue q; //define queue
  /* Initialize Queue */
  if(queue_init(&q, qSize) == QUEUE_FAILURE){
fprintf(stderr,
  "error: queue_init failed!\n");
  }

  /* loop through input args and make threads */
  for (t = 0; t< numReqThreads; t++){
    printf("in main creating thread");
    rc = pthread_create(&(threads[t]), NULL, )
  }

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
