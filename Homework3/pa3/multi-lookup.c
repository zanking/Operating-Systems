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
 #define MAX_RESOLVER_THREADS 10 //set to 11 because up to 10 input files
 #define MIN_RESOLVER_THREADS 2
 #define MAX_NAME_LENGTH 1025
 #define SBUFSIZE 1025
 #define USAGE "<inputFilePath> <outputFilePath>"
 // #define SBUFSIZE 1025
 #define INPUTFS "%1024s"
 #define QUEUE_SIZE 50 //max size


 // char errorstr[SBUFSIZE];
 // char firstipstr[INET6_ADDRSTRLEN];
 // FILE* inputfp = NULL;
 // FILE* outputfp = NULL;
 // char hostname[SBUFSIZE];

 pthread_cond_t writer; //requester is writer
 pthread_cond_t reader; //resolver is reader
 pthread_mutex_t queueLock;
 pthread_mutex_t fileLock;
 queue* critical;



 int *alive;


 void* request(void* file){
   printf("hello please from request\n");
   char hostname[SBUFSIZE];
   char errorstr[SBUFSIZE];
   char* payload;
   payload = (char*)malloc(SBUFSIZE*sizeof(char));

   FILE* inputfp = NULL;

  	/* Open Input File */
  	inputfp = fopen((char*)file, "r");
  	  if(!inputfp){
  	    printf("Error Opening Input File: %s\n");
  	    perror(errorstr);
        return NULL;
  	  }



   while(fscanf(inputfp, INPUTFS, hostname) > 0){
     pthread_mutex_lock(&queueLock);
     if(queue_is_full(critical)){
       pthread_cond_wait(&reader,&queueLock);
     }

      //  payload = malloc(SBUFSIZE);
       payload = strdup(hostname);
       queue_push(critical,payload);
       pthread_mutex_unlock(&queueLock);
       pthread_cond_signal(&reader);
   }
 fclose(inputfp);
 return NULL;
 }

 void* resolve(void* OutFile){
   printf("in resolve \n");

   char firstipstr[INET6_ADDRSTRLEN];
   FILE* outputfp = fopen((char*)OutFile, "w");
   if(!outputfp){
   perror("Error Opening Output File\n");
  //  return EXIT_FAILURE;
    return NULL;
    }
    printf("about to lock queue\n");
   while(*alive || !(queue_is_empty(critical))){
     char* hostname;
     printf("about to lock queue in while loop\n");
   pthread_mutex_lock(&queueLock);
   if(queue_is_empty(critical)){
     pthread_cond_wait(&writer,&queueLock);
   }
   printf("about to pop off queue\n");
   hostname = queue_pop(critical);
   printf("popped off queue\n" );
   pthread_mutex_unlock(&queueLock);
   pthread_cond_signal(&writer);
   printf("unlocked queue and signaled writer\n");
   printf("about to run DNS lookup\n");
   if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
      == UTIL_FAILURE){
 fprintf(stderr, "dnslookup error: %s\n", hostname);
 strncpy(firstipstr, "", sizeof(firstipstr));
   }
   printf("about to write to the file");
   pthread_mutex_lock(&fileLock);
   fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
   pthread_mutex_unlock(&fileLock);
   free(hostname); //free our memory
 }
  fclose(outputfp);
   return NULL;
 }

 int main(int argc, char* argv[]){
  //  printf("A");
   alive = malloc(sizeof(int));
  //  printf("B");
   critical = malloc(sizeof(queue));

  //  printf("hello can u see me");
   *alive = 1;
   int i;
  //  printf("hello can u see me 2");
   int proc = sysconf (_SC_NPROCESSORS_ONLN);

  //  printf("hello can u see me");
   if(argc < MINARGS){
  	  fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
   	fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
     return EXIT_FAILURE;
   }

   if(queue_init(critical, QUEUE_SIZE) == QUEUE_FAILURE){
   fprintf(stderr,
   "error: queue_init failed!\n");
   }

   int ifiles = argc -2;
   pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t)*ifiles);
   pthread_t *outThread = (pthread_t *) malloc(sizeof(pthread_t)* proc);
   printf("allocated pthread memory about to launch request\n");
  //  char errorstr[SBUFSIZE];
   for(i = 0; i < ifiles; i++){
     printf("about to call pthread create requester\n");
     int newRequestTh = pthread_create(&thread[i], NULL, request, argv[i+1]);
     printf("called requester in for loop\n");
     //error check
   }

   for(i = 0; i < proc; i++){
     printf("about to call pthread create resolver\n");
     int resolvers = pthread_create(&outThread[i], NULL, resolve, argv[argc-1]);
     printf("called resolvers\n");
//error check this too

   }

   for(i = 0; i < ifiles; i++){

     pthread_join(thread[i],NULL);

   }
   *alive = 0;

   for(i = 0; i < proc; i++){

     pthread_join(outThread[i],NULL);

   }



   return 0;




 }








//global condition Vars
// pthread_cond_t writers;
// pthread_cond_t readers;
// pthread_mutex_t* lockqueue;
// queue Q;
//
// void* request(FILE* file, queue* Q){
//     char hostname[SBUFSIZE];
//     char errorstr[SBUFSIZE];
//     char* attachment;
//
//     FILE* inputfp = fopen(argv[i], "r");
//
//     if(!inputfp){
//        sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
//        perror(errorstr);
//        break;
//      }
//
//
//   	while(fscanf(inputfp, INPUTFS, hostname) > 0){
//       pthread_mutex_lock(&lockqueue);
//       if(queue_is_full(&Q)){
//   	     fprintf(stderr, "Queue is full\n");
//          pthread_cond_wait(&readers); ////////////////////////////fix
//     }
//
//     attachment = strdup(hostname);
//     queue_push(&Q, &attachment);
//     pthread_mutex_unlock(&lockqueue);
//     pthread_cond_signal(&readers)
//   }
//
// return NULL;
// }
//
// void * resolve(FILE* file, queue* Q){
//   char* payload;
//   char firstipstr[INET6_ADDRSTRLEN];
//   outputfp = fopen(argv[(argc-1)], "w");
//   if(!outputfp){
//     perror("Error Opening Output File");
//     return EXIT_FAILURE;
//   }
//   pthread_mutex_lock(&Q);
//
//   if queue_is_empty(&Q){
//     pthread_cond_wait(&writers)
//   }
//   payload = queue_pop(&Q);
//   pth
// }
//
//  int main(int argc, char* argv[]){
//
//    FILE* inputfp = NULL;
//    FILE* outputfp = NULL;
//    char hostname[SBUFSIZE];
//    char errorstr[SBUFSIZE];
//    char firstipstr[INET6_ADDRSTRLEN];
//    int i;
//    /* Loop Through Input Files */
//    for(i=1; i<(argc-1); i++){
//
//      /* Open Input File */
//
//
//  }
//  }


//global needed vars
// FILE* file;

// int* status

//
// typedef struct writeQueueParams{
//   FILE* fileWrite;
//   queue* Q;
// } writeQueueP;
//
//
// void* writeQueue(void *p){
//
//   printf("CALLED");
//   writeQueueP* parameter = p; //point parameter to our input p
//   char hostname[MAX_NAME_LENGTH]; //
//   char* attachment; //contains name and ip
//   int written = 0; //determine if writing was a success
//   int error = 0;
//
//
//   queue* queueQ = parameter->Q;
//     FILE* file = parameter->fileWrite;
//   while(fscanf(file, INPUTFS, hostname) > 0){
//
//     while(!written){ //while the file hasnt been written to
//       error = pthread_mutex_lock(&lockqueue); //lock queue and check for error
//       if(error){
//         fprintf(stderr, "Mutex error when locking queue %d\n", error);
//       }
//       if(queue_is_full(queueQ)){ //check the status of the queue
//         // if queue is full we dont want to write
//          pthread_cond_wait(&writers, &lockqueue);//wait if the queue is full
//
//       //wait for resolver thread to wake us up
//       }
//       else {
//         attachment = malloc(MAX_NAME_LENGTH); //allocate space for payload (ip & address)
//
//         attachment=strncpy(attachment, hostname, MAX_NAME_LENGTH); //copys line from file into the attachment
//         if(queue_push(queueQ, attachment)== QUEUE_FAILURE){ //push attachment onto queue
//           fprintf(stderr, "Unable to push to the queue\n");
//         }
//         //now we want to release our mutex lock
//         error = pthread_mutex_unlock(&lockqueue);
//         if (error){
//           fprintf(stderr, "Mutex error when unlocking the queue\n");
//         }
//
//         pthread_cond_signal(&readers); //wake up the readers if the queue was empty
//
//         written = 1;
//         //now we have pushed on the queue
//             }
//     }
//     written = 0; //
//   }
//
//   return NULL;
//
// }
//
//
// int main(int argc, char* argv[]){
//
//   int threadNum = argc -2; //argv ends at argc -1, and we only care about input files
//
//   /*make an array of file pointers*/
//   FILE* inputFiles[threadNum];
//
//   // Check to see how many arguments were passed
//   if(argc < MIN_RESOLVER_THREADS){
//     fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
//     fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
//     return EXIT_FAILURE;
//   }
//
//   /* Setup Local Vars */
//
//
//   queue bigqueue;
//
//   int rc;
//   int t, errorc, j;
//
//   writeQueueP inputParams[threadNum];
//   // fprintf("Number of threads = %p\n", argv[1]);
//   printf("Can you see me\n");
//
//   //initalize the queue
//   if(queue_init(&bigqueue, QUEUE_SIZE)==QUEUE_FAILURE){
//     fprintf(stderr,"Error creating the Queue\n");
//     return EXIT_FAILURE;
//   }
//
//
//   //create the mutex for queuelock
//   errorc=pthread_mutex_init(&lockqueue, NULL);
//
//   //check to see that the mutex was sucessful
//   if(errorc){
//     fprintf(stderr, "Error creating the Queue Mutex\n");
//     fprintf(stderr, "Error No: %d\n",errorc);
//     return EXIT_FAILURE;
//   }
//   /* open all files*/
//   for(t=1;t<threadNum+1;t++){
//     printf("opening file");
//
//     inputFiles[t] = fopen(argv[t], "r");
//       if (!inputFiles[t]){
//         printf("Error opening input file");
//         return EXIT_FAILURE;
//       }
//   }
//
//
//
//   /* make the reading/ queuepushing threads*/
//     pthread_t qthreads[threadNum];  //set array of threads for writing to the queue
//   for(j = 0; j<threadNum; j++){
//     // inputParams = malloc(sizeof(struct writeQueueParams));
//     FILE* currentFile = inputFiles[j];
//     inputParams[j].fileWrite = currentFile;
//     inputParams[j].Q = &bigqueue;
//
//     printf("In main: creating thread %d\n", j);
//     malloc(sizeof(pthread))
//     rc = pthread_create(&(qthreads[j]), NULL, writeQueue, &inputParams[j]);
//     printf("called thread");
//     if (rc){
//         printf("supossed to call thread");
//       printf("ERROR; return code from pthread_create() is %d\n", rc);
//       exit(EXIT_FAILURE);
//     }
// }
//   for (t=0; t<threadNum; t++){
//     pthread_join(qthreads[t], NULL);
//   }
//   return 0;
// }
