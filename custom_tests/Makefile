CC = gcc

CFLAGS = -Wall -Wextra -pthread -g

TARGET =  ring_buf_tests

SOURCES = ring_buf_tests.c ../starter_code/ring_buffer.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
