CC      	= gcc
WARN    	= -W -Wall
SIMPLE_CHAT	= simple_chat.c
TARGET		= simple-chat
PREFIX		= /usr

.PHONY: build clean install

$(TARGET): $(SIMPLE_CHAT)
	$(CC) $(WARN) -o $(TARGET) $(SIMPLE_CHAT)

build: $(TARGET)

clean:
	rm -f $(TARGET) *.o

# https://www.gnu.org/software/make/manual/html_node/DESTDIR.html
install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

