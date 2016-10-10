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


//global condition Vars
pthread_cond_t condA;
pthread_cond_t condB;

//global needed vars
// FILE* file;
pthread_mutex_t* queueQ;
pthread_mutex_t* fileWrite;
queue* Q;
// int* status





void* writeQueue(FILE* fileName){
  char hostname[MAX_NAME_LENGTH]; //
  char * attachment; //contains name and ip
  int written = 0; //determine if writing was a success
  int error = 0;
  while(fscanf(fileName, INPUTFS, hostname) > 0){

    while(!written){ //while the file hasnt been written to
      error = pthread_mutex_lock(queueQ); //lock queue and check for error
      if(error){
        fprintf(stderr, "Mutex error when locking queue %d\n", error);
      }
      if(queue_is_full(Q)){ //check the status of the queue
        // if queue is full we dont want to write
         pthread_cond_wait(&condA, queueQ);//wait if the queue is full

      //wait for resolver thread to wake us up
      }
      else {
        attachment = malloc(MAX_NAME_LENGTH); //allocate space for payload (ip & address)

        attachment=strncpy(attachment, hostname, MAX_NAME_LENGTH); //copys line from file into the attachment
        if(queue_push(Q, attachment)== QUEUE_FAILURE){ //push attachment onto queue
          fprintf(stderr, "Unable to push to the queue\n");
        }
        //now we want to release our mutex lock
        error = pthread_mutex_unlock(queueQ);
        if (error){
          fprintf(stderr, "Mutex error when unlocking the queue\n");
        }
        written = 1;
        //now we have pushed on the queue
            }
    }
    written = 0; //
  }

  return NULL;

}
//
// void* readQueue(void*pn){
//   reader* parameter = pn;
//   char hostname[MAX_NAME_LENGTH];
//
//
// }


int main(int argc, char* argv[]){

  int threadNum = argc -2; //argv ends at argc -1, and we only care about input files


  /* Setup Local Vars */
  pthread_t qthreads[threadNum];  //set array of threads
  int rc;
  int t;


  /* Spawn NUM_THREADS threads */
  for(t=0;t<threadNum;t++){
    printf("In main: creating thread %ld\n", t);
    rc = pthread_create(&(qthreads[t]), NULL, writeQueue, &argv[t]);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(EXIT_FAILURE);
    }
    for (t=0; t<threadNum; t++){
      pthread_join(qthreads[t], NULL);
    }
  }
  return 0;
}
