#include <stdio.h>     
#include <stdlib.h>   
#include <stdint.h>  
#include <inttypes.h>  
#include <errno.h>     
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
    int32_t fd; // file descriptor
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
struct stat initialzeFileData(int32_t fd);
struct thread_data initializeChild(struct thread_data* params, int id);
struct thread_data initializeRoot(struct stat file, int numThreads, int32_t fd);

// block size
#define BSIZE 4096

int main(int argc, char** argv) {
  int32_t fd;
  
  int numThreads = atoi(argv[2]);

  // input checking 
  if (argc != 3)
    Usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDWR);

  char *hash = malloc(sizeof(char) * 50); 
  pthread_t root; 
  struct stat file = initialzeFileData(fd); // initialize thread_data struct
  struct thread_data data = initializeRoot(file, numThreads, fd); // initialize root thread_data struct
  double start = GetTime(); // start timer
  pthread_create(&root, NULL, treeHash, &data); // create root thread
  pthread_join(root, (void**)&hash); // join root thread
  

  double end = GetTime();
  printf("hash value = %s \n", hash);
  printf("time taken = %f \n", (float)(end - start));
  free (hash);
  close(data.fd);
  return EXIT_SUCCESS;
}

/**
 * Initialize thread_data struct
 * @param filename - name of the file
 * @param numThreads - number of threads
 * @return data - thread_data struct
*/
struct stat initialzeFileData(int32_t fd) {
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
  return file;
} 

struct thread_data initializeRoot(struct stat file, int numThreads, int32_t fd) {
  uint32_t nblocks;
  char *addr = mmap(NULL, file.st_size, PROT_READ, MAP_PRIVATE, fd, 0); // mmap the file
  if (addr == MAP_FAILED) { // check for errors
      printf("ERROR: mmap\n");
      exit(EXIT_FAILURE);
  }
  nblocks = file.st_size / BSIZE; // calculate number of blocks

  printf(" no. of blocks = %u \n", nblocks);
  off_t numToParse = 0; 
  off_t fileSize = file.st_size; // get file size
  if (numThreads > 0) { // if there are more than 1 threads
    numToParse = nblocks / numThreads; // calculate number of blocks to parse
  }

  struct thread_data threadInfo; // create thread_data struct
  threadInfo.size = fileSize; // set size
  threadInfo.numThreads = numThreads - 1; // set number of threads
  threadInfo.numToParse = numToParse; // set number of blocks to parse
  threadInfo.data = addr; 
  threadInfo.id = 0; 

  return threadInfo;
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
    struct thread_data* params = (struct thread_data*)arg; 
    int left = (params->id * 2) + 1; // calculate left child id
    int right = (params->id * 2) + 2; // calculate right child id
    pthread_t leftSide;
    pthread_t rightSide;
    char* leftHash = malloc(sizeof(char) * 50);
    char* rightHash = malloc(sizeof(char) * 50);

    char* hash = malloc(sizeof(char) * 32);
    off_t offset = (params->id * params->numToParse); // calculate offset
    params->data += (offset) * BSIZE; // move pointer to the correct location
    uint32_t unsignedHash;
    unsignedHash = jenkins_one_at_a_time_hash((const uint8_t*)params->data, params->numToParse * BSIZE);
    params->data -= (offset) * BSIZE; // move pointer back to the beginning
    sprintf(hash, "%u", unsignedHash); // convert hash value to string

    if (left <= params->numThreads && right <= params->numThreads) { // if there are 2 children
        struct thread_data leftData = initializeChild(params, left); // initialize left child
        pthread_create(&leftSide, NULL, treeHash, &leftData); // create left child
        struct thread_data rightData = initializeChild(params, right); // initialize right child
        pthread_create(&rightSide, NULL, treeHash, &rightData); // create right child
        pthread_join(leftSide, (void**)&leftHash); 
        pthread_join(rightSide, (void**)&rightHash);
        strcat(hash, leftHash);
        strcat(hash, rightHash);
        uint64_t length = strlen(hash);
        unsignedHash = jenkins_one_at_a_time_hash((const uint8_t*)hash, length); // calculate hash value
        sprintf(hash, "%u", unsignedHash); // convert hash value to string
    } else if (left == params->numThreads) { // if there is only 1 child
        struct thread_data leftData = initializeChild(params, left); // initialize left child
        pthread_create(&leftSide, NULL, treeHash, &leftData); // create left child
        pthread_join(leftSide, (void**)&leftHash);
        strcat(hash, leftHash);
        uint64_t length = strlen(hash);
        unsignedHash = jenkins_one_at_a_time_hash((const uint8_t*)hash, length); // calculate hash value
        sprintf(hash, "%u", unsignedHash); // convert hash value to string
    }

    free(leftHash); // free memory
    free(rightHash); // free memory
    pthread_exit((void*)hash); // exit thread
}


struct thread_data initializeChild(struct thread_data* params, int id) {
    struct thread_data thread; // create left child thread_data struct
    thread.size = params->size;
    thread.numThreads = params->numThreads;
    thread.data = params->data;
    thread.id = id;
    thread.numToParse = params->numToParse; 
    return thread;
}

void Usage(char* s) {
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}

