CC = gcc
CFLAGS = -Wall -Werror -std=c99 -D_GNU_SOURCE
TARGET = output
SRCS = a2.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)