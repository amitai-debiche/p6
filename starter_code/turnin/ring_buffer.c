#include "ring_buffer.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int init_ring(struct ring *r){
    if (r == NULL) {
        return -1;
    }

    r->c_head = 0;
    r->c_tail = 0;
    r->p_head = 0;
    r->p_tail = 0;
    return 0;


}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void ring_submit(struct ring *r, struct buffer_descriptor *bd) {
    pthread_mutex_lock(&lock);
    uint32_t prod_head, prod_next, cons_tail;
    bool success = false;
    while (!success) {
        prod_head = r->p_head;
        cons_tail = r->c_tail;
        prod_next = (prod_head + 1);

	if(prod_head - cons_tail < 1024) {
            success = atomic_compare_exchange_strong(&r->p_head, &prod_head, prod_next);
        }
    }
    r->buffer[prod_head % RING_SIZE] = *bd;
    success = false;
    while(!success){
        success = atomic_compare_exchange_strong(&r->p_tail, &prod_head, prod_next);
    }
    pthread_mutex_unlock(&lock);
}

void ring_get(struct ring *r, struct buffer_descriptor *bd){
    pthread_mutex_lock(&lock);
    uint32_t cons_next, cons_head, prod_tail;
    bool success = false;

    while (!success) {
        cons_head = r->c_head;
        prod_tail = r->p_tail;
        cons_next = (cons_head + 1);
        if (cons_head < prod_tail) {
            success = atomic_compare_exchange_strong(&r->c_head, &cons_head, cons_next);
        }
    }
    *bd = r->buffer[cons_head % RING_SIZE];
    success = false;
    while (!success) {
        success = atomic_compare_exchange_strong(&r->c_tail, &cons_head, cons_next);
    }
    pthread_mutex_unlock(&lock);
}



