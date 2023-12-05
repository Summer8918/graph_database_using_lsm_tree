# graph database using lsm tree

## Compilation
compliers: g++

compile command: make

clean compile files: make clean

## Initialization of graph
The graph is initialize with input file(soc-LiveJournal1.txt.gz) download from https://snap.stanford.edu/data/soc-LiveJournal1.html.

The soc-LiveJournal1.txt.gz is too large to be uploaded to github, you can downlowd it from above website.

The first 10 lines of soc-LiveJournal1.txt is shown as below:
```
# Directed graph (each unordered pair of nodes is saved once): soc-LiveJournal1.txt 
# Directed LiveJournal friednship social network
# Nodes: 4847571 Edges: 68993773
# FromNodeId	ToNodeId
0	1
0	2
0	3
0	4
0	5
0	6
0	7
0	8
0	9
0	10
```

# Test of the Database
./db_test -src 87 -type 1 -rounds 4
use -src to indicate the start source of BFS application
use -type to designate test type: lsm_tree_graph(in memory graph database) or lsm_tree_graph(lsm tree graph database)
use -rounds to indicate the rounds of test BFS application