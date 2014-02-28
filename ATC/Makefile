CC=gcc
SOURCES=main.c
HEADERS=./libat/headers
OBJECTS=$(SOURCES:.c=.o)

CFLAGS=-c -fPIC -Wall -I$(HEADERS)
LDFLAGS=-L./libat/ -lat

all: $(SOURCES) driver install
	rm -rf $(OBJECTS)

clean:
	rm -rf $(OBJECTS)
	rm -rf bin

driver: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

install:
	rm -rf bin
	mkdir bin
	mv driver bin
	cp libat/libat.so bin
