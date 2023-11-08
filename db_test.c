#include "vertex.h"
#include "graph.h"

using namespace std;

int main() {
    cout << "hello db" << endl;

    /* Code to test class initGraphFile and subGraph begin*/
    InitGraphFile initGraphFile("partOfsocLiveJournal1.txt");
    uint a, b;
    subGraph sg;
    while (initGraphFile.getLine(a, b)) {
        int sgLen = sg.addEdge(a, b);
        if (sgLen >= MAX_SUB_GRAPH_SIZE) {
            sg.setOutDegree();
            sg.printSubgraph();
            sg.clearSubgraph();
        }
    }
    /* Code to test clas initGraphFile and subGraph end*/

    return 0;
}