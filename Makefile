SHELL := /bin/sh
CC=clang
CFLAGS=-Wall -Wextra -Werror -Wpedantic -Wshadow -gdwarf-4
SRCFILES=trie.c word.c io.c helpers.c 
OBJFILES=trie.o word.o io.o helpers.o 
HEADERS=helpers.h trie.h word.h io.h

all: encode decode

decode: decode.o $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

encode: encode.o $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

helpers.o: helpers.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

trie.o: trie.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

word.o: word.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

io.o: io.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f *.o decode encode

format:
	clang-format -i -style=file *.[ch]
