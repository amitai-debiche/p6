#include "ring_buffer.h"
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

    for (int i = 0; i < RING_SIZE; i++){
        r->buffer[i].req_type = PUT;
        r->buffer[i].k = 0;
        r->buffer[i].v = 0;
        r->buffer[i].res_off = 0;
        r->buffer[i].ready = 0;
    }
    return 0;
}

void ring_submit(struct ring *r, struct buffer_descriptor *bd){
  //  printf("ring_sub called\n");
    uint32_t prod_head, prod_next, cons_tail;
    bool success = false;

    while (!success){
        prod_head = r->p_head;
        cons_tail = r->c_tail;
        prod_next = (r->p_head + 1);
         if ((prod_head >= cons_tail) && (prod_head - cons_tail <= 1024)) {
            success = atomic_compare_exchange_strong(&r->p_head, &prod_head, prod_next);
        }
    }
    //printf("EXITS\n");
    r->buffer[prod_head % RING_SIZE] = *bd;
    success = false;
    while(!success){
        success = atomic_compare_exchange_strong(&r->p_tail, &prod_head, prod_next);
    }
 //   printf("UPDATED TAIL, %u\n", r->p_tail);

}

void ring_get(struct ring *r, struct buffer_descriptor *bd){
    uint32_t cons_next, cons_head, prod_tail;
    bool success = false;

    while (!success) {
        cons_head = r->c_head;
        prod_tail = r->p_tail;
        cons_next = (r->c_head + 1);
        if (cons_head < prod_tail) {
            success = atomic_compare_exchange_strong(&r->c_head, &cons_head, cons_next);
        }
    }
    *bd = r->buffer[cons_head % RING_SIZE];
    success = false;
    while (!success) {
        success = atomic_compare_exchange_strong(&r->c_tail, &cons_head, cons_next);
    }
//   printf("ring_get exits");
  //  printf("UPDATED CONSUMER TAIL, %u\n", r->c_tail);
}



