#include "ring_buffer.h"
#include <stdatomic.h>
#include <stdint.h>

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
    uint32_t prod_head, prod_next;
    bool success = false;

    while (!success){
        prod_head = r->p_head;
        prod_next = (r->p_head + 1) % RING_SIZE;
        while (prod_head == r->c_tail) {
        }
        success = atomic_compare_exchange_strong(&r->p_head, &prod_head, prod_next);
    }
    r->buffer[prod_head] = *bd;
    r->p_tail = prod_next;
}

void ring_get(struct ring *r, struct buffer_descriptor *bd){
    uint32_t cons_next, cons_head;
    bool success = false;

    while (!success) {
        cons_head = r->c_head;
        cons_next = (r->c_head + 1) % RING_SIZE;
        while (cons_head == r->p_tail){
        }

        success = atomic_compare_exchange_strong(&r->c_head, &cons_head, cons_next);
    }
    r->buffer[cons_head] = *bd;
    r->p_tail = prod_next;
}



