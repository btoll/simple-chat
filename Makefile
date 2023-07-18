CC      	= gcc
WARN    	= -W -Wall
PROGRAM		= simple-chat
prefix		= /usr

.PHONY: build clean distclean install uninstall

all: build

build: $(PROGRAM)

$(PROGRAM): simple_chat.c
	$(CC) $(WARN) -o $(PROGRAM) simple_chat.c

clean:
	rm -f $(PROGRAM) *.o

distclean: clean

# https://www.gnu.org/software/make/manual/html_node/DESTDIR.html
install:
	install -D -m 0755 $(PROGRAM) $(DESTDIR)$(prefix)/bin/$(PROGRAM)

uninstall:
	-rm -f $(DESTDIR)$(prefix)/bin/$(PROGRAM)

