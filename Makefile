CC = gcc
CFLAGS = -Wall -Werror -std=c99 -D_GNU_SOURCE
TARGET = output
A2 = a2.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(A2)

clean:
	rm -f $(TARGET)