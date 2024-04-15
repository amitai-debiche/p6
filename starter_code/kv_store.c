#include "common.h"
#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct node{
    key_type k;
    value_type v;
    struct node* next;
};

struct hashTable {
    pthread_mutex_t* lock;
    pthread_mutex_t** arr_lock;
    struct node** arr;
};
struct hashTable* map;

uint size;

void init() {
    map = (struct hashTable*) malloc(sizeof(struct hashTable));
    pthread_mutex_init(map->lock, NULL);
    map->arr = (struct node**) malloc(sizeof(struct node*) * size);
    map->arr_lock = (pthread_mutex_t**) malloc(sizeof(pthread_mutex_t*) * size);
    for(int i = 0; i < size; i++) {
        map->arr[i] = NULL;
        pthread_mutex_init(*(map->arr_lock + i), NULL);
    }
}

void put(key_type k, value_type v) {
    uint index = hash_function(k, size);
    struct node* newNode = (struct node*) malloc(sizeof(struct node));

    newNode->k = k;
    newNode->v = v;
    newNode->next = NULL;

    pthread_mutex_lock(map->lock);
    if(map->arr[index] == NULL) {
        map->arr[index] = newNode;
    } else {
	pthread_mutex_lock(map->arr_lock[index]);
	pthread_mutex_unlock(map->lock);
	struct node* tmp = map->arr[index];
	if(tmp->k != k) {
            while(tmp != NULL) {
	        if(tmp->k == k) break;
                tmp = tmp->next;
	    }
	}
	if(tmp->k == k) {
          tmp->v = v;
	  pthread_mutex_unlock(map->arr_lock[index]);
	} else { // must be end of list
          tmp = map->arr[index];
	  while(tmp->next != NULL) tmp = tmp->next; // finding spots
	  tmp->next = newNode;
	  pthread_mutex_unlock(map->arr_lock[index]);
	}
    }
}

value_type get(key_type k) {
    int index = hash_function(k, size);
    pthread_mutex_lock(map->lock);
    pthread_mutex_lock(map->arr_lock[index]);
    pthread_mutex_unlock(map->lock);
    struct node* tmp = map->arr[index];
    
    while(tmp != NULL) {
        if(tmp->k == k){
	    return tmp->v;
	}
	tmp = tmp->next;
    }
    pthread_mutex_unlock(map->arr_lock[index]);

    return 0;
}

// ./server -n serverThreads -s hashtableSize
int main(int argc, char** argv) {
    if(argc < 4) return -1;
    if(argv[2] < 0 || argv[4] < 0) return -1;

    size = *argv[4];
    init();
    return 0;
}
