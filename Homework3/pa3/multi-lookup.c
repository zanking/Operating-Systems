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
  //  payload = (char*)malloc(SBUFSIZE*sizeof(char));

   FILE* inputfp = NULL;

  	/* Open Input File */
  	inputfp = fopen((char*)file, "r");
       printf("opened input file");
  	  if(!inputfp){
  	    printf("Error Opening Input File:\n");
  	    perror(errorstr);
        return NULL;
  	  }



   while(fscanf(inputfp, INPUTFS, hostname) > 0){
        printf("about to lock queue\n");
     pthread_mutex_lock(&queueLock);
        printf("locked queue\n");
     if(queue_is_full(critical)){
       printf("yo queue full fam\n");
       pthread_cond_wait(&reader,&queueLock);
       printf("signaled reader to wake tf up bruh\n");
     }

      //  payload = malloc(SBUFSIZE);

       payload = strdup(hostname);
          printf("copied hostname about to push on queue\n");
       queue_push(critical,payload);
          printf("pushed on queue\n");
       pthread_mutex_unlock(&queueLock);
          printf("unlocked queue about to signal reader\n");
       pthread_cond_signal(&writer);
          printf("signaled reader\n");
          // free(payload)
   }
      printf("about to close input file\n");
 fclose(inputfp);
   printf("closed input file\n");
 return NULL;
 }

 void* resolve(void* OutFile){
   char firstipstr[INET6_ADDRSTRLEN];
      printf("locked file\n");

    // printf("about to lock queue\n");
   while(*alive || !(queue_is_empty(critical))){
     char* hostname;


    //  printf("about to lock queue in while loop\n");
   pthread_mutex_lock(&queueLock);
   if(queue_is_empty(critical)){
     pthread_cond_wait(&writer,&queueLock);
   }
  //  printf("about to pop off queue\n");
   hostname = queue_pop(critical);
   printf("popped off queue------------------------------------\n" );
   pthread_mutex_unlock(&queueLock);
   pthread_cond_signal(&reader);


   pthread_mutex_lock(&fileLock); //now open file
 FILE* outputfp = fopen((char*)OutFile, "a"); //open to append
 if(!outputfp){
 perror("Error Opening Output File\n");
//  return EXIT_FAILURE;
  return NULL;
  }


//   printf("unlocked queue and signaled writer\n");
//   printf("about to run DNS lookup\n");
   if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
      == UTIL_FAILURE){
 fprintf(stderr, "dnslookup error: %s\n", hostname);
 strncpy(firstipstr, "", sizeof(firstipstr));
   }
   printf("about to write to the file\n");


   fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
   printf("wrote to file but still have lock\n");

   free(hostname); //free our memory
   printf("wrote to the file we gucci\n");
     fclose(outputfp);
      pthread_mutex_unlock(&fileLock);
 }

  //  pthread_mutex_unlock(&fileLock);
  // fclose(OutFile);
   printf("closed file!!!!!!!!!!!!!!!!!!!!\n");
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
   pthread_t thread[ifiles]; //= (pthread_t *) malloc(sizeof(pthread_t)*ifiles);
   pthread_t outThread[proc]; // = (pthread_t *) malloc(sizeof(pthread_t)* proc);
   printf("allocated pthread memory about to launch request\n");
  //  char errorstr[SBUFSIZE];
   for(i = 0; i < ifiles; i++){
     printf("about to call pthread create requester\n");
     int newRequestTh = pthread_create(&thread[i], NULL, request, argv[i+1]);
     if (newRequestTh){
         printf("ERROR; return code from pthread_create() is %d\n", newRequestTh);
         exit(EXIT_FAILURE);
     }
     printf("called requester in for loop\n");
     //error check
   }

   for(i = 0; i < proc; i++){
     printf("about to call pthread create resolver\n");
     int resolvers = pthread_create(&outThread[i], NULL, resolve, argv[argc-1]);
     if (resolvers){
         printf("ERROR; return code from pthread_create() is %d\n", resolvers);
         exit(EXIT_FAILURE);
     }

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
   //free(alive);
   queue_cleanup(critical);
   pthread_mutex_destroy(&queueLock);
   pthread_mutex_destroy(&fileLock);
   pthread_cond_destroy(&reader);
   pthread_cond_destroy(&writer);
   return 0;

 }
