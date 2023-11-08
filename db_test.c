#include "vertex.h"
#include "graph.h"

using namespace std;

int main() {
    cout << "hello db" << endl;

    /* Code to test class initGraphFile and subGraph begin*/
    InitGraphFile initGraphFile("partOfsocLiveJournal1.txt");
    uint a, b;
    subGraph sg;
    int cnt = 0;
    while (initGraphFile.getLine(a, b)) {
        int sgLen = sg.addEdge(a, b);
        if (sgLen >= MAX_ARRAYS_SIZE) {
            sg.setOutDegree();
            sg.printSubgraph();
            cout << "file name:" << to_string(cnt) << " cnt:" << cnt << endl;
            sg.flushSubgraphToDisk(to_string(cnt));
            sg.clearSubgraph();
            cnt += 1;
        }
    }
    sg.setOutDegree();
    cout << "file name:" << to_string(cnt) << " cnt:" << cnt << endl;
    sg.printSubgraph();
    sg.flushSubgraphToDisk(to_string(cnt));
    sg.clearSubgraph();
    cout << "test deserialize" << endl;
    sg.deserialize(to_string(cnt));
    sg.printSubgraph();
    /* Code to test clas initGraphFile and subGraph end*/

    return 0;
}