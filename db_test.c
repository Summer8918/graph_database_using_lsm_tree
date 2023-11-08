#include "vertex.h"
#include "graph.h"

using namespace std;

int main() {
    cout << "hello db" << endl;

    /* Code to test clas initGraphFile begin*/
    InitGraphFile initGraphFile("partOfsocLiveJournal1.txt", 1024);
    uint a, b;
    while (initGraphFile.getLine(a, b)) {
    }
    /* Code to test clas initGraphFile end*/

    return 0;
}