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
//valgrind --tool=memcheck --leak-check=yes ./multi-lookup input/names1.txt  input/names2.txt input/names3.txt input/names4.txt input/names5.txt tester.txt

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
/*init our resources and pthread items*/
 pthread_cond_t writer; //requester is writer
 pthread_cond_t reader; //resolver is reader
 pthread_mutex_t queueLock;
 pthread_mutex_t fileLock;
 queue* critical;
 int *alive;


 void* request(void* file){
   char hostname[SBUFSIZE];
   char errorstr[SBUFSIZE];
   char* payload;

   FILE* inputfp = NULL;

  	/* Open Input File */
  	inputfp = fopen((char*)file, "r");
       printf("opened input file");
  	  if(!inputfp){
  	    printf("Error Opening Input File:\n");
  	    perror(errorstr);
        return NULL;
  	  }
/*read from input file into queue*/
   while(fscanf(inputfp, INPUTFS, hostname) > 0){
     pthread_mutex_lock(&queueLock); //protect critical section
     if(queue_is_full(critical)){
       pthread_cond_wait(&reader,&queueLock);
     }


       payload = strdup(hostname); //save our attachment
       queue_push(critical,payload);
       pthread_mutex_unlock(&queueLock);
       pthread_cond_signal(&writer);
   }
 fclose(inputfp);
 return NULL;
 }
/* write from queue to output file*/
 void* resolve(void* OutFile){
   char firstipstr[INET6_ADDRSTRLEN];

   while(*alive || !(queue_is_empty(critical))){
     char* hostname;


   pthread_mutex_lock(&queueLock); //lock the queue for when we pop
   if(queue_is_empty(critical)){
     pthread_cond_wait(&writer,&queueLock);
   }
   hostname = queue_pop(critical);
   pthread_mutex_unlock(&queueLock); //unlock queue to allow requesters to continue
   pthread_cond_signal(&reader); //signal requesters to continue workin in the case of queue being previously full


   pthread_mutex_lock(&fileLock); //now open file
 FILE* outputfp = fopen((char*)OutFile, "a"); //open to append
 if(!outputfp){
 perror("Error Opening Output File\n");
  return NULL;
  }


   if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
      == UTIL_FAILURE){
 fprintf(stderr, "dnslookup error: %s\n", hostname);
 strncpy(firstipstr, "", sizeof(firstipstr));
   }
   fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
   free(hostname); //free our memory
     fclose(outputfp);
      pthread_mutex_unlock(&fileLock);
 }

   return NULL;
 }

 int main(int argc, char* argv[]){
   alive = malloc(sizeof(int));
   critical = malloc(sizeof(queue));
   *alive = 1;
   int i;
   int proc = sysconf (_SC_NPROCESSORS_ONLN);
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
   pthread_t thread[ifiles];
   pthread_t outThread[proc];

   for(i = 0; i < ifiles; i++){
     int newRequestTh = pthread_create(&thread[i], NULL, request, argv[i+1]);
     if (newRequestTh){
         printf("ERROR; return code from pthread_create() is %d\n", newRequestTh);
         exit(EXIT_FAILURE);
     }

   }

   for(i = 0; i < proc; i++){
     int resolvers = pthread_create(&outThread[i], NULL, resolve, argv[argc-1]);
     if (resolvers){
         printf("ERROR; return code from pthread_create() is %d\n", resolvers);
         exit(EXIT_FAILURE);
     }
   }
/*join our threads back together*/
   for(i = 0; i < ifiles; i++){
     pthread_join(thread[i],NULL);
   }
   *alive = 0;

   for(i = 0; i < proc; i++){

     pthread_join(outThread[i],NULL);

   }
   //clean up our resources
   queue_cleanup(critical);
   pthread_mutex_destroy(&queueLock);
   pthread_mutex_destroy(&fileLock);
   pthread_cond_destroy(&reader);
   pthread_cond_destroy(&writer);
   return 0;

 }
