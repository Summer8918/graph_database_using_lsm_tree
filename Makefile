ifdef D
   CXXFLAGS=-Wall -std=c++11 -g -pg -DDEBUG
else
   CXXFLAGS=-Wall -std=c++11 -g -O3 
endif

#CXXFLAGS=-Wall -std=c++11 -g -pg

CC=g++

all: db_test GraphMerge

GraphMerge: adj_list_graph.h GraphMerge.h

vertex.o: vertex.h

utils.o: utils.h

graph.o: graph.h

GraphMerge.o: GraphMerge.h

adj_list_graph.o: adj_list_graph.h

clean:
	$(RM) *.o db_test