ifdef D
   CXXFLAGS=-Wall -std=c++11 -g -pg -DDEBUG
else
   CXXFLAGS=-Wall -std=c++11 -g -O3 
endif

#CXXFLAGS=-Wall -std=c++11 -g -pg

CC=g++

all: db_test GraphMerge

db_test: db_test.c graph.h vertex.h utils.h lsmtree_db.h

GraphMerge: GraphMerge.c GraphMerge.h adj_list_graph.h

vertex.o: vertex.h

utils.o: utils.h

graph.o: graph.h

adj_list_graph.o: adj_list_graph.h

lsmtree_db.o: lsmtree_db.h

clean:
	$(RM) *.o db_test GraphMerge
