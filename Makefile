ifdef D
   CXXFLAGS=-Wall -std=c++11 -g -pg -DDEBUG
else
   CXXFLAGS=-Wall -std=c++11 -g -O3 
endif

#CXXFLAGS=-Wall -std=c++11 -g -pg

CC=g++

all: db_test

vertex.o: vertex.h

utils.o: utils.h

graph.o: graph.h

clean:
	$(RM) *.o db_test