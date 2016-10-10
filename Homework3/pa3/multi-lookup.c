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
 #define queueSize = 40 //10 less than max


//set up a struct to contain inputs of functions

typedef struct writer{
  FILE* file; //pass the file
  pthread_mutex_t* queueQ; //points to the mtuex that locks the shared queue resorce
  queue* Q //points to our queue
}writer;

//struct to contain outputs of functions
typedef struct reader{
  FILE* file;
  pthread_mutex_t* queueQ;
  pthread_mutex_t* out;
  queue *Q;
  int* status
}reader;


//////////////////////////////////////////////////////////////

void* writeQueue(void* pn){
  char hostname[MAX_NAME_LENGTH]; //
  writer* parameter = pn;

  FILE* fileName = parameter -> file; //read from input struct
  pthread_mutex_t* lockQueue = parameter->queueQ; //set the queue lock
  queue* bigQueue = parameter->Q; //define the main queue

  char * attachment; //the payload

  int success = 0; //determine if writing was a success
  int error = 0;

  while(fscanf(fileName, INPUTFS, hostname) > 0){
    while(!success){ //while the file hasnt been written to
      error = pthread_mutex_lock(lockQueue); //lock queue and check for error
      if(error){
        fprintf(stderr, "Mutex error when locking queue %d\n", error);
      }
      if(queue_is_full(bigQueue)){ //check the status of the queue
        // if queue is full we dont want to write
        error = pthread_mutex_unlock(lockQueue); //release the mutex lock so that other threads can execute
        if (error){
          fprintf(stderr, "Mutex error when unlocking queue %d\n", error);
        }
      usleep(1000); //suspend processing for 1000 microseconds
      }
      else {
        attachment = malloc(MAX_NAME_LENGTH); //allocate space for payload (ip & address)
        if (attachment == NULL){
          fprintf(stderr, "Unable to allocate space for payload\n ");
        }
        attachment=strncpy(attachment, hostname, MAX_NAME_LENGTH); //copys line from file into the attachment
        if(queue_push(bigQueue, attachment)== QUEUE_FAILURE){ //push attachment onto queue
          fprintf(stderr, "Unable to push to the queue\n");
        }
        //now we want to release our mutex lock
        error = pthread_mutex_unlock(lockQueue);
        if (error){
          fprintf(stderr, "Mutex error when unlocking the queue\n");
        }
        success = 1;
        //now we have pushed on the queue
            }
    }
    success = 0; //
  }
  //
  if(fclose(fileName)){
    fprintf(stderr, "Error closing file after pushing to queue");
  }

  return NULL;

}

void* readQueue(void*pn){
  reader* parameter = pn;
  char hostname[MAX_NAME_LENGTH];


}
