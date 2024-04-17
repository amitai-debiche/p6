CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

TARGETS = ring_buf_tests single_op_test

ring_buf_tests_SOURCES = ring_buf_tests.c ../starter_code/ring_buffer.c
single_op_test_SOURCES = single_op_test.c ../starter_code/ring_buffer.c

ring_buf_tests_OBJECTS = $(ring_buf_tests_SOURCES:.c=.o)
single_op_test_OBJECTS = $(single_op_test_SOURCES:.c=.o)

all: $(TARGETS)

ring_buf_tests: $(ring_buf_tests_OBJECTS)
	$(CC) $(CFLAGS) $(ring_buf_tests_OBJECTS) -o $@

single_op_test: $(single_op_test_OBJECTS)
	$(CC) $(CFLAGS) $(single_op_test_OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) $(ring_buf_tests_OBJECTS) $(single_op_test_OBJECTS)

.PHONY: all clean
