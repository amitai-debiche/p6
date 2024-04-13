#include "../starter_code/ring_buffer.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>



struct ring ring_buffer;


void *producer_thread(){
    struct buffer_descriptor bd;
    bd.req_type = PUT;
    bd.k = 123;
    bd.v = 456;
    
    while (1){
        //printf("About to submit\n");
        ring_submit(&ring_buffer, &bd);
        //printf("A submission has been completed\n");
        usleep(10000);
    }

    return NULL;
}

void *consumer_thread(){
    struct buffer_descriptor bd;
    
    while (1) {
        //printf("About to consume\n");
        ring_get(&ring_buffer, &bd);
        //printf("A submission has been consumed\n");

        printf("Consumed: req_type=%d, k=%u, v=%u\n", bd.req_type, bd.k, bd.v);
        usleep(20000);
    }

    return NULL;
}


int main() {

    printf("TESTS STARTING\n");
    init_ring(&ring_buffer);
    printf("Ring buff initialized: p_head=%u, p_tail=%u, c_head=%u, c_tail=%u\n",ring_buffer.p_head, ring_buffer.p_tail, ring_buffer.c_head, ring_buffer.c_tail);

    pthread_t producer[4], consumer[2];

    printf("Creating Threads\n");
    for (int i = 0; i < 4; i++){
        pthread_create(&producer[i], NULL, producer_thread,NULL);
    }

    for (int i = 0; i < 2; i++){
        pthread_create(&consumer[i], NULL, consumer_thread,NULL);
    } 

    printf("Threads Created, waiting\n");
    while(1){
        sleep(1);
    }

    return 0;
}
