CPP=g++
CPPFLAGS=-Iincludes -Wall -Wextra -ggdb -std=c++23 
LDLIBS=-lcrypto
VPATH=src
.INTERMEDIATE: hash.o parser_server.o server.o

all: server

server: hash.o parser_server.o server.o
	$(CPP) $^ $(LDLIBS) -o $@

clean:
	rm -rf *~ server

.PHONY : clean all