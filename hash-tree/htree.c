#include <stdio.h>     
#include <stdlib.h>   
#include <stdint.h>  
#include <inttypes.h>  
#include <errno.h>     // for EINTR
#include <fcntl.h>     
#include <unistd.h>    
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include "common.h"

struct thread_data {
    off_t size; // size of the file
    int id; // id of the thread
    int numThreads; // number of threads
    off_t numToParse; // number of blocks to parse
    char* data; // data to parse
};

// Print out the usage of the program and exit.
void Usage(char*);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* , uint64_t );
void* treeHash(void* arg);

// block size
#define BSIZE 4096

/**
 * Main function
*/
int main(int argc, char** argv) {
  int32_t fd;
  uint32_t nblocks;
  int numThreads = atoi(argv[2]);

  // input checking 
  if (argc != 3)
    Usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  // use fstat to get file size
  struct stat file;
    if (fstat(fd, &file) == -1) {
        printf("ERROR: file statistics\n");
        exit(EXIT_FAILURE);
    }
  // calculate nblocks 
  char *addr = mmap(NULL, file.st_size, PROT_READ, MAP_PRIVATE, fd, 0); // mmap the file
  if (addr == MAP_FAILED) { // check for errors
      printf("ERROR: mmap\n");
      exit(EXIT_FAILURE);
  }
  nblocks = file.st_size / BSIZE; // calculate number of blocks
  printf(" no. of blocks = %u \n", nblocks);

  double start = GetTime(); 

  //calculate hash value of the input file
  off_t numToParse = 0; 
  off_t fileSize = file.st_size; // get file size
  if (numThreads > 0) { // if there are more than 1 threads
    numToParse = nblocks / numThreads; // calculate number of blocks to parse
  }

  struct thread_data data; // create thread_data struct
  data.size = fileSize; // set size
  data.numThreads = numThreads - 1; 
  data.numToParse = numToParse; 
  data.data = addr; 
  data.id = 0; 
  char *hash = malloc(sizeof(char) * 50); 
  pthread_t root; 
  pthread_create(&root, NULL, treeHash, &data); // create root thread
  pthread_join(root, (void**)&hash); // join root thread
  

  double end = GetTime();
  printf("hash value = %s \n", hash);
  printf("time taken = %f \n", (float)(end - start));
  free (hash);
  close(fd);
  return EXIT_SUCCESS;
}

uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, uint64_t length) {
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

/**
 * Recursive function to calculate the hash value of the file
 * @param arg - thread_data struct
 * @return hash - hash value of the file
*/
void* treeHash(void* arg) {
    struct thread_data *params = (struct thread_data *) arg; // cast arg to thread_data struct
    int left = (params->id * 2) + 1; // calculate left child id
    int right = (params->id * 2) + 2; // calculate right child id
    pthread_t leftSide; // create left child thread
    pthread_t rightSide; // create right child thread
    char *leftHash = malloc(sizeof(char) * 50); 
    char *rightHash = malloc(sizeof(char) * 50);

    char *hash = malloc(sizeof(char) * 32); // create hash string
    off_t offset = (params->id * params->numToParse); 
    params->data += (offset) * BSIZE; // move pointer to correct location
    uint32_t unsignedHash; // create unsigned hash
    unsignedHash = jenkins_one_at_a_time_hash((const uint8_t *)params->data, params->numToParse * BSIZE); // calculate hash
    params->data -= (offset) * BSIZE; // move pointer back to beginning
    sprintf(hash, "%u", unsignedHash); // convert hash to string

    if (left <= params->numThreads && right <= params->numThreads) { // if there are more than 1 threads
        struct thread_data leftData; // create left child thread_data struct
        leftData.id = params->id; 
        leftData.numThreads = params->numThreads;
        leftData.data = params->data;
        leftData.id = left;
        leftData.numToParse = params->numToParse; 
        pthread_create(&leftSide, NULL, treeHash, &leftData); // create left child thread
        struct thread_data rightData; // create right child thread_data struct
        rightData.size = params->size; 
        rightData.numThreads = params->numThreads;
        rightData.data = params->data;
        rightData.id = right;
        rightData.numToParse = params->numToParse;
        pthread_create(&rightSide, NULL, treeHash, &rightData); // create right child thread
        pthread_join(leftSide, (void**)&leftHash); // join left child thread
        pthread_join(rightSide, (void**)&rightHash); // join right child thread
        strcat(hash, leftHash); // concatenate left hash to hash
        strcat(hash, rightHash); // concatenate right hash to hash
        uint64_t length = strlen(hash); // get length of hash
        unsignedHash = jenkins_one_at_a_time_hash((const uint8_t *)hash, length); // calculate parent hash
        sprintf(hash, "%u", unsignedHash);  // convert hash to string
    } else if (left == params -> numThreads) { // if there is only 1 thread
        struct thread_data leftData; // create left child thread_data struct
        leftData.size = params->size;
        leftData.numThreads = params->numThreads;
        leftData.data = params->data;
        leftData.id = left;
        leftData.numToParse = params->numToParse; 
        pthread_create(&leftSide, NULL, treeHash, &leftData); // create left child thread
        pthread_join(leftSide, (void**)&leftHash); // join left child thread
        strcat(hash, leftHash); // concatenate left hash to hash
        uint64_t length = strlen(hash); 
        unsignedHash = jenkins_one_at_a_time_hash((const uint8_t *)hash, length); // calculate parent hash
        sprintf(hash, "%u", unsignedHash); // convert hash to string
    }

    free(leftHash); 
    free(rightHash);
    pthread_exit((void*)hash); // exit thread
}

void Usage(char* s) 
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}