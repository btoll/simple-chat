CC      = gcc
WARN    = -W -Wall

.PHONY: all simple_chat clean

all: simple_chat

simple_chat:
	$(CC) $(WARN) -o simple_chat simple_chat.c

clean:
	rm simple_chat *.o

