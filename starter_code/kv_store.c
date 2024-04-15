#include "common.h"
#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

struct node{
    key_type k;
    value_type v;
    struct node* next;
};
struct node** arr;

uint size;

void init() {
    arr = (struct node**)malloc(sizeof(struct node*) * size);
    for(int i = 0; i < size; i++) {
        arr[i] = NULL;
    }
}

void put(key_type k, value_type v) {
    uint index = hash_function(k, size);
    struct node* newNode = (struct node*) malloc(sizeof(struct node));

    newNode->k = k;
    newNode->v = v;
    newNode->next = NULL;

    if(arr[index] == NULL) {
        arr[index] = newNode;
    } else {
	struct node* tmp = arr[index];
	if(tmp->k != k) {
            while(tmp != NULL) {
	        if(tmp->k == k) break;
                tmp = tmp->next;
	    }
	}
	if(tmp->k == k) {
          tmp->v = v;
	} else { // must be end of list
          tmp = arr[index];
	  while(tmp->next != NULL) tmp = tmp->next; // finding spots
	  tmp->next = newNode;
	}
    }
}

value_type get(key_type k) {
    int index = hash_function(k, size);
    struct node* tmp = arr[index];
    
    while(tmp != NULL) {
        if(tmp->k == k) return tmp->v;
	tmp = tmp->next;
    }

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
