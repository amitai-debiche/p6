#include "common.h"
#include "ring_buffer.h"
#include <bits/getopt_core.h>
#include <bits/pthreadtypes.h>
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

struct node {
    key_type k;
    value_type v;
    struct node* next;
};

struct hashTable {
    pthread_mutex_t* arr_lock;
    struct node** arr;
};

struct hashTable* map;
int hashtable_size = 1000;
struct ring *ring;
char *shmem_area;
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

    pthread_mutex_lock(&(map->arr_lock[index]));

    if(map->arr[index] == NULL) {
        map->arr[index] = newNode;
        pthread_mutex_unlock(&(map->arr_lock[index]));
        return;
    }

    struct node* cur = map->arr[index];
    struct node* prev = map->arr[index];


    while(cur) {
        prev = cur;
        if(cur->k == k) {
            cur->v = v;
            pthread_mutex_unlock(&(map->arr_lock[index]));
            return;
        }
        cur = cur->next;
    }
    prev->next = newNode;
    pthread_mutex_unlock(&(map->arr_lock[index]));
}

value_type get(key_type k) {

    int index = hash_function(k, hashtable_size);
    struct node* tmp = map->arr[index];

    while(tmp != NULL) {
        if(tmp->k == k) {
            return tmp->v;
        }
        tmp = tmp->next;
    }
    return 0;
}

void server_init() {
    int fd = open(shm_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0)
        perror("open");

    struct stat file_stat;
    if (stat(shm_file, &file_stat) < 0) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    int shm_size = file_stat.st_size;

    char *mem = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);

    ring = (struct ring *)mem;
    shmem_area = mem;
}

void *thread_function() {
    struct buffer_descriptor bd;
    while(true) {
        ring_get(ring, &bd);
        if(bd.req_type == PUT) {
            put(bd.k, bd.v);
        } else {
            bd.v = get(bd.k);
        }
        struct buffer_descriptor* window = (struct buffer_descriptor *)(shmem_area + bd.res_off);
        memcpy(window, &bd, sizeof(struct buffer_descriptor));
    	window->ready = READY;
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


static int parse(int argc, char **argv)
{
    int op;

    while ((op = getopt(argc, argv, "n:s:v")) != -1) {
        switch(op) {
            case 'n':
                num_threads = atoi(optarg);
                break;

            case 's':
                hashtable_size = atoi(optarg);
                break;
        }
    }
    return 0;
}



// ./server -n serverThreads -s hashtableSize
int main(int argc, char** argv) {
    if (parse(argc, argv) != 0)
        exit(EXIT_FAILURE);

    if(num_threads <= 0 || hashtable_size <= 0) {
        fprintf(stderr, "Invalid argument(s).\n");
        return EXIT_FAILURE;
    }

    server_init();
    hashtable_init();
    start_threads();
    wait_for_threads();

    return EXIT_SUCCESS;
}
