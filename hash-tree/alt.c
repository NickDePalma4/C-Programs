#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>     // for EINTR
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <string.h>


/*
 *
num Threads = 4
Blocks per Thread = 16384
tnum 3 hash computed 3830420435
tnum 3 hash sent to parent 3830420435
tnum 2 hash computed 1891069755
tnum 2 hash sent to parent 1891069755
tnum 1 hash computed 2738683681
tnum 1 hash from left child 3830420435
tnum 1 concat string 27386836813830420435
tnum 1 hash sent to parent 2230974001
tnum 0 hash computed 4081559069
tnum 0 hash from left child 2230974001
tnum 0 hash from right child 1891069755
tnum 0 concat string 408155906922309740011891069755
tnum 0 hash sent to parent 2365678750
hash value = 2365678750
 */

/*
 *
num Threads = 1
Blocks per Thread = 262144
hash value = 2806494268
-----------------------
num Threads = 16
Blocks per Thread = 16384
hash value = 2238003738
-----------------------
num Threads = 64
Blocks per Thread = 4096
hash value = 4048116138
-----------------------
num Threads = 256
Blocks per Thread = 1024
hash value = 1081711728
-----------------------
num Threads = 1024
Blocks per Thread = 256
hash value = 759834141
-----------------------
num Threads = 4096
Blocks per Thread = 64
hash value = 2648775596
-----------------------
 */

#define BSIZE 4096
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length);
void* htree(void* args);
void Pthread_create(pthread_t *t, const pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg);
void Pthread_join(pthread_t thread, void **value_ptr);
void Pthread_mutex_lock(pthread_mutex_t *mutex);
void Pthread_mutex_unlock(pthread_mutex_t *mutex);
void Pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
void Spin(int howlong);
double GetTime();



struct TreeInfo {   //a struct to pass everything into the pthread
    off_t fileSize;
    int currNodeNum;
    int numOfNodes;
    off_t numOfCharToHash;
    char* fileInMemory;
};

int main(int argc, char *argv[]) {
    int32_t fd;
    if (argc != 3)
    {   //if the input isn't right exit
        perror("input check failed");
        EXIT_FAILURE;
    }

    fd = open(argv[1], O_RDWR); //open file

    if (fd == -1)
    {   //if file open fails exit program
        perror("file couldn't be opened");
        EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {   //if the fstat fails exit program
        perror("error exit");
        EXIT_FAILURE;
    }

    off_t file_size;        //fill in the struct for the tree info
    file_size = st.st_size;
    char* file_in_memory = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    int numOfNodes = 4;//atoi(argv[2]);
    off_t blocksPerFile = file_size / BSIZE;
    off_t numOfCharToHash = 0;
    if (numOfNodes > 0)
    {
        numOfCharToHash = blocksPerFile / numOfNodes;
    }
    struct TreeInfo info;   //fill in the struct for the tree info
    info.fileSize = file_size;
    info.numOfNodes = numOfNodes - 1;
    info.fileInMemory = file_in_memory;
    info.numOfCharToHash = numOfCharToHash * BSIZE;
    info.currNodeNum = 0;
    char* hash = malloc(sizeof(char) * 50);

    pthread_t p1;
    time_t start, end;
    time(&start); // get time before thread is created
    pthread_create(&p1, NULL, htree, &info);
    pthread_join(p1, (void**)&hash);
    time(&end); // get time after the thread is done
    double diffTime = (double)(start - end);
    printf("Time elapsed: %f\n", diffTime); //print the diff of time
    printf("Blocks per Thread = %d\n", numOfNodes); //print num of threads
    if (numOfNodes > 0) {
        printf("Blocks per Thread = %lld\n", (blocksPerFile / numOfNodes)); //print blocks per thread
    }
    printf("Hash = %s\n", hash); // print the hash
    free(hash);
    return 0;

}

void* htree(void* arg) {

    struct TreeInfo* infoBuffer = (struct TreeInfo*)(arg);  //cast the variable passed
    struct TreeInfo info = *infoBuffer;
    int leftSide = (info.currNodeNum * 2) + 1;
    int rightSide = leftSide + 1;
    pthread_t p1;
    pthread_t p2;
    char* retLeft = malloc(sizeof(char)*50); // allocate space for both the left hash and the right hash
    char* retRight = malloc(sizeof(char)*50);

//    uint32_t* hash;
//    hash = malloc(sizeof(*hash)); //allocate memory
    char* hash = malloc(sizeof(char)*32);
    off_t offset = (info.currNodeNum) * info.numOfCharToHash;
    info.fileInMemory += offset;        //offset the file
    uint32_t uHash;
    uHash = jenkins_one_at_a_time_hash((const uint8_t *)info.fileInMemory, info.numOfCharToHash); // get hash code
    info.fileInMemory -= offset;        //reset the file
    sprintf(hash, "%u", uHash);         //conver that hash to string

    if (leftSide <= info.numOfNodes && rightSide <= info.numOfNodes)
    {
        struct TreeInfo leftInfo;           //make new struct to be passed in the threads
        leftInfo.currNodeNum = leftSide;
        leftInfo.numOfNodes = info.numOfNodes;
        leftInfo.numOfCharToHash = info.numOfCharToHash;
        leftInfo.fileInMemory = info.fileInMemory;
        leftInfo.fileSize = info.fileSize;
        struct TreeInfo rightInfo;          //make new struct to be passed in the threads
        rightInfo.currNodeNum = rightSide;
        rightInfo.numOfNodes = info.numOfNodes;
        rightInfo.numOfCharToHash = info.numOfCharToHash;
        rightInfo.fileInMemory = info.fileInMemory;
        rightInfo.fileSize = info.fileSize;
        pthread_create(&p1, NULL, htree, &leftInfo);
        pthread_create(&p2, NULL, htree, &rightInfo);
        pthread_join(p1, (void **) &retLeft);
        pthread_join(p2, (void **) &retRight);  //wait for the threads for subtrees
        strcat(hash, retLeft);      //cat the hash values from the subtrees to the root hash val
        strcat(hash, retRight);
        int length = strlen(hash);
        uHash = jenkins_one_at_a_time_hash((const uint8_t *)hash, length);  //take the hash of that new hash value
        sprintf(hash, "%u", uHash);
    }
    else if (leftSide == info.numOfNodes)
    {
        struct TreeInfo leftInfo;           //make new struct to be passed in the threads
        leftInfo.currNodeNum = leftSide;
        leftInfo.numOfNodes = info.numOfNodes;
        leftInfo.numOfCharToHash = info.numOfCharToHash;
        leftInfo.fileInMemory = info.fileInMemory;
        leftInfo.fileSize = info.fileSize;
        pthread_create(&p1, NULL, htree, &leftInfo);
        pthread_join(p1, (void **) &retLeft); //wait for the threads for subtrees
        strcat(hash, retLeft); //cat the hash values from the subtrees to the root hash val

        int length = strlen(hash);
        uHash = jenkins_one_at_a_time_hash((const uint8_t *)hash, length);  //take the hash of that new hash value
        sprintf(hash, "%u", uHash);
    }

    free(retLeft);      //free allocated memory
    free(retRight);
    pthread_exit((void *)hash);

}


uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) {
    size_t i = 0;
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

void Pthread_create(pthread_t *t, const pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg) {
    int rc = pthread_create(t, attr, start_routine, arg);
    assert(rc == 0);
}

void Pthread_join(pthread_t thread, void **value_ptr) {
    int rc = pthread_join(thread, value_ptr);
    assert(rc == 0);
}


void Pthread_mutex_lock(pthread_mutex_t *mutex) {
    int rc = pthread_mutex_lock(mutex);
    assert(rc == 0);
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int rc = pthread_mutex_unlock(mutex);
    assert(rc == 0);
}

void Pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr) {
    int rc = pthread_mutex_init(mutex, attr);
    assert(rc == 0);
}

double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, 0);
    assert(rc == 0);
    return (double)t.tv_sec + (double)t.tv_usec/1e6;
}

void Spin(int howlong) {
    double t = GetTime();
    while ((GetTime() - t) < (double)howlong)
        ; // do nothing in loop
}
