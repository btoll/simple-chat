CC      	= gcc
WARN    	= -W -Wall
SIMPLE_CHAT	= simple_chat.c
TARGET		= simple_chat

.PHONY: build clean

$(TARGET): $(SIMPLE_CHAT)
	$(CC) $(WARN) -o $(TARGET) $(SIMPLE_CHAT)

build: $(TARGET)

clean:
	rm -f $(TARGET) *.o

