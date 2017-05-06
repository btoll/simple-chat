CC      = gcc
WARN    = -W -Wall

.PHONY: all simple_chat clean

all: simple_chat

simple_chat: hash.o hash.h
	$(CC) $(WARN) -o simple_chat simple_chat.c

clean:
	rm simple_chat *.o

