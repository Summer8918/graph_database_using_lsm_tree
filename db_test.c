#include "vertex.h"
#include "graph.h"
#include <filesystem>
#include <chrono>

using namespace std;

int main()
{
    cout << "hello db" << endl;
    string fileName = "soc-LiveJournal1.txt";

    InitGraphFile initGraphFile(fileName.c_str());

    uint a, b;
    subGraph sg;
    int cnt = 0;
    std::string dirPath = "tmp";
    if (filesystem::exists(dirPath)) {
        if (filesystem::remove_all(dirPath)) {
            std::cout << "Deleted existing directory: " << dirPath << std::endl;
        } else {
            std::cerr << "Failed to delete the existing directory: " << dirPath << std::endl;
            abort();
        }
    }
    if (std::filesystem::create_directory(dirPath)) {
        std::cout << "Created new directory: " << dirPath << std::endl;
    } else {
        std::cerr << "Failed to create the new directory: " << dirPath << std::endl;
        return 1;
    }
    int edgeNum = 0;
    while (initGraphFile.getLine(a, b))
    {
        // cout << "a" << a << " b" << b << endl;
        edgeNum = sg.addEdge(a, b);
        if (edgeNum % 1000000 == 0) {
            cout << "edgeNum:" << edgeNum << endl;
        }
        if (edgeNum >= MAX_EDGE_NUM) {
            break;
        }
    }
    sg.setOutDegree();
    cout << "edgeNum:" << edgeNum << endl;
    sg.serializeAndAppendBinToDisk(dirPath + "/l" + to_string(0));
    sg.deserialize(dirPath + "/l" + to_string(0));
    //sg.printSubgraph();
    // sg.flushSubgraphToDisk(dirPath+"/l"+to_string(0));
    sg.clearSubgraph();
    return 0;
}