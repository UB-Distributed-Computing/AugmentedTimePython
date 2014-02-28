CC=gcc
SOURCES=clock.c event.c stack.c
HEADERS=./headers
OBJECTS=$(SOURCES:.c=.o)

CFLAGS=-c -fPIC -Wall -I$(HEADERS)
LDFLAGS=-shared

all: $(SOURCES) libat.so
	rm -rf $(OBJECTS)

clean:
	rm -rf $(OBJECTS)
	rm -rf libat.so

libat.so: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
