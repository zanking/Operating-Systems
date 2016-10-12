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
 #include <pthread.h>
 #include <unistd.h>

 #include "util.h"
 #include "queue.h"
 /* all constraints defined by handout*/

 #define MINARGS 3
 #define MAXARGS 10
 #define MAX_RESOLVER_THREADS 11 //set to 11 because up to 10 input files
 #define MIN_RESOLVER_THREADS 2
 #define MAX_NAME_LENGTH 1025
 #define SBUFSIZE 1025
 #define USAGE "<inputFilePath> <outputFilePath>"
 // #define SBUFSIZE 1025
 #define INPUTFS "%1024s"
 #define QUEUE_SIZE 40 //10 less than max


//global condition Vars
pthread_cond_t writers;
pthread_cond_t readers;
pthread_mutex_t lockqueue;

//global needed vars
// FILE* file;

// int* status


typedef struct writeQueueParams{
  FILE* fileWrite;
  queue* Q;
} writeQueueP;


void* writeQueue(void *p){

  printf("CALLED");
  writeQueueP* parameter = p; //point parameter to our input p
  char hostname[MAX_NAME_LENGTH]; //
  char* attachment; //contains name and ip
  int written = 0; //determine if writing was a success
  int error = 0;


  queue* queueQ = parameter->Q;
    FILE* file = parameter->fileWrite;
  while(fscanf(file, INPUTFS, hostname) > 0){

    while(!written){ //while the file hasnt been written to
      error = pthread_mutex_lock(&lockqueue); //lock queue and check for error
      if(error){
        fprintf(stderr, "Mutex error when locking queue %d\n", error);
      }
      if(queue_is_full(queueQ)){ //check the status of the queue
        // if queue is full we dont want to write
         pthread_cond_wait(&writers, &lockqueue);//wait if the queue is full

      //wait for resolver thread to wake us up
      }
      else {
        attachment = malloc(MAX_NAME_LENGTH); //allocate space for payload (ip & address)

        attachment=strncpy(attachment, hostname, MAX_NAME_LENGTH); //copys line from file into the attachment
        if(queue_push(queueQ, attachment)== QUEUE_FAILURE){ //push attachment onto queue
          fprintf(stderr, "Unable to push to the queue\n");
        }
        //now we want to release our mutex lock
        error = pthread_mutex_unlock(&lockqueue);
        if (error){
          fprintf(stderr, "Mutex error when unlocking the queue\n");
        }

        pthread_cond_signal(&readers); //wake up the readers if the queue was empty

        written = 1;
        //now we have pushed on the queue
            }
    }
    written = 0; //
  }

  return NULL;

}


int main(int argc, char* argv[]){

  int threadNum = argc -2; //argv ends at argc -1, and we only care about input files

  /*make an array of file pointers*/
  FILE* inputFiles[threadNum];

  // Check to see how many arguments were passed
  if(argc < MIN_RESOLVER_THREADS){
    fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
    fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
    return EXIT_FAILURE;
  }

  /* Setup Local Vars */


  queue bigqueue;

  int rc;
  int t, errorc, j;

  writeQueueP inputParams[threadNum];
  // fprintf("Number of threads = %p\n", argv[1]);
  printf("Can you see me\n");

  //initalize the queue
  if(queue_init(&bigqueue, QUEUE_SIZE)==QUEUE_FAILURE){
    fprintf(stderr,"Error creating the Queue\n");
    return EXIT_FAILURE;
  }


  //create the mutex for queuelock
  errorc=pthread_mutex_init(&lockqueue, NULL);

  //check to see that the mutex was sucessful
  if(errorc){
    fprintf(stderr, "Error creating the Queue Mutex\n");
    fprintf(stderr, "Error No: %d\n",errorc);
    return EXIT_FAILURE;
  }
  /* open all files*/
  for(t=1;t<threadNum+1;t++){
    printf("opening file");

    inputFiles[t] = fopen(argv[t], "r");
      if (!inputFiles[t]){
        printf("Error opening input file");
        return EXIT_FAILURE;
      }
  }



  /* make the reading/ queuepushing threads*/
    pthread_t qthreads[threadNum];  //set array of threads for writing to the queue
  for(j = 0; j<threadNum; j++){
    inputParams = malloc(sizeof(struct writeQueueParams));
    FILE* currentFile = inputFiles[j];
    inputParams[j].fileWrite = currentFile;
    inputParams[j].Q = &bigqueue;

    printf("In main: creating thread %d\n", j);
    rc = pthread_create(&(qthreads[j]), NULL, writeQueue, &inputParams[j]);
    printf("called thread");
    if (rc){
        printf("supossed to call thread");
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(EXIT_FAILURE);
    }
}
  for (t=0; t<threadNum; t++){
    pthread_join(qthreads[t], NULL);
  }
  return 0;
}
