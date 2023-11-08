#include "vertex.h"
#include "graph.h"

using namespace std;

int main() {
    cout << "hello db" << endl;

    /* Code to test clas initGraphFile begin*/
    InitGraphFile initGraphFile("partOfsocLiveJournal1.txt", 1024);
    long len = initGraphFile.readChunk();
    int i = 0;
    while (len > 0) {
        char * b = initGraphFile.getBuffer();
        cout << "i:" << i << " read length:" << len << endl;
        len = initGraphFile.readChunk();
        i += 1;
    }
    /* Code to test clas initGraphFile end*/

    return 0;
}