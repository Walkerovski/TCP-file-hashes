CPP=g++
CPPFLAGS=-Iincludes -Wall -Wextra -ggdb -std=c++23 
LDLIBS=-lcrypto
VPATH=src
.INTERMEDIATE: hash.o parser_server.o server.o parser_client.o client.o

all: server client

server: hash.o parser_server.o server.o
	$(CPP) $^ $(LDLIBS) -o $@

client: hash.o parser_client.o client.o
	$(CPP) $^ $(LDLIBS) -o $@

clean:
	rm -rf *~ server client

.PHONY : clean all