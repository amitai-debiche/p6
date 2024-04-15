#include "common.h"
#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

value_type* arr;

void init(uint size) {
    arr = (value_type*)malloc(sizeof(value_type) * size);
    for(int i = 0; i < size; i++) {
	*(arr + i) = 0;
    }
}

void put(key_type k, value_type v) {
    arr[hash_function(k, 1024)] = v;
}


value_type get(key_type k) {
    return arr[hash_function(k, 1024)];
}

int main(int argc, char** argv) {
    init(1024);
    return 0;
}
