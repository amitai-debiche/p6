#include "common.h"
#include "ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_THREADS 128
#define READY 1
#define NOT_READY 0

struct node{
    key_type k;
    value_type v;
    struct node* next;
};

struct hashTable {
    pthread_mutex_t* arr_lock;
    struct node** arr;
};

struct hashTable* map;
uint hashtable_size;
struct ring *ring = NULL;
char *shmem_area = NULL;
char shm_file[] = "shmem_file";
int num_threads = 4;
pthread_t threads[MAX_THREADS];


void hashtable_init() {
    map = (struct hashTable*) malloc(sizeof(struct hashTable));
    map->arr = (struct node**) malloc(sizeof(struct node*) * hashtable_size);
    map->arr_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * hashtable_size);
    for(int i = 0; i < hashtable_size; i++) {
        map->arr[i] = NULL;
        pthread_mutex_init(&(map->arr_lock[i]), NULL);
    }
}

void put(key_type k, value_type v) {
    uint index = hash_function(k, hashtable_size);
    struct node* newNode = (struct node*) malloc(sizeof(struct node));

    newNode->k = k;
    newNode->v = v;
    newNode->next = NULL;

    if(map->arr[index] == NULL) {
        map->arr[index] = newNode;
    } else {
	pthread_mutex_lock(&(map->arr_lock[index]));
	struct node* tmp = map->arr[index];
	if(tmp->k != k) {
            while(tmp != NULL) {
	        if(tmp->k == k) break;
                tmp = tmp->next;
	    }
	}
	if(tmp->k == k) {
          tmp->v = v;
	  pthread_mutex_unlock(&(map->arr_lock[index]));
	} else { // must be end of list
          tmp = map->arr[index];
	  while(tmp->next != NULL) tmp = tmp->next; // finding spots
	  tmp->next = newNode;
	  pthread_mutex_unlock(&(map->arr_lock[index]));
	}
    }
}

value_type get(key_type k) {
    int index = hash_function(k, hashtable_size);
    pthread_mutex_lock(&(map->arr_lock[index]));
    struct node* tmp = map->arr[index];
    
    while(tmp != NULL) {
        if(tmp->k == k){
	    return tmp->v;
	}
	tmp = tmp->next;
    }
    pthread_mutex_unlock(&(map->arr_lock[index]));

    return 0;
}

void server_init() {
    int fd = open(shm_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0)
        perror("open");

    struct stat* file_stat = (struct stat*)malloc(sizeof(struct stat));
    int shm_size = (int) file_stat->st_size;

    char *mem = mmap(NULL, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
        if (mem == (void *)-1) 
	    perror("mmap");
    close(fd);

    ring = (struct ring *)mem;
    shmem_area = mem;
}

void *thread_function() {
    while(true) {
        struct buffer_descriptor bd;
        ring_get(ring, &bd);
        if(bd.req_type == PUT) {
            put(bd.k, bd.v);
        } else {
            bd.v = get(bd.k);
	    bd.ready = READY;
	    memcpy((void*)(shmem_area + bd.res_off), &bd, sizeof(struct buffer_descriptor));
        }
    }
}

void start_threads() {
    for (int i = 0; i < num_threads; i++) {
        if(pthread_create(&threads[i], NULL, &thread_function, NULL))
            perror("pthread_create");
    }
}

void wait_for_threads() {
    for (int i = 0; i < num_threads; i++)
        if (pthread_join(threads[i], NULL))
            perror("pthread_join");
}

// ./server -n serverThreads -s hashtableSize
int main(int argc, char** argv) {
    if(argc < 4) return -1;
    if(argv[2] < 0 || argv[4] < 0) return -1;

    hashtable_size = *argv[4];
    num_threads = *argv[2];
    
    server_init();

    hashtable_init();
    
    start_threads();

    wait_for_threads();
    

    return 0;
}
