CC=gcc
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)

CFLAGS=-c -fPIC -Wall -I./libat/headers -I/home/csgrad/bavva/installed/zeromq/include
LDFLAGS=-L./libat/ -lat -L/home/csgrad/bavva/installed/zeromq/lib -lzmq

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
