#include "vertex.h"
#include "graph.h"
#include <filesystem>
#include <chrono>

using namespace std;

int main() {
    cout << "hello db" << endl;

    /* Code to test class initGraphFile and subGraph begin*/
    //partOfsocLiveJournal1.txt or soc-LiveJournal1.txt
    //string fileName = "partOfsocLiveJournal1.txt";
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
            std::cerr << "Failed to delete the existing directory: " << \
                dirPath << std::endl;
            abort();
        }
    }
    if (std::filesystem::create_directory(dirPath)) {
        std::cout << "Created new directory: " << dirPath << std::endl;
    } else {
        std::cerr << "Failed to create the new directory: " << dirPath << std::endl;
        return 1;
    }

    // Record the start time
    auto startTime = std::chrono::high_resolution_clock::now();
    while (initGraphFile.getLine(a, b)) {
        //cout << "a" << a << " b" << b << endl; 
        int sgLen = sg.addEdge(a, b);
        if (sgLen >= MAX_ARRAYS_SIZE) {
            sg.setOutDegree();
            //sg.printSubgraph();
            cout << "file name 1:" << to_string(cnt) << " cnt:" << cnt << endl;
            sg.flushSubgraphToDisk(dirPath + "/" + to_string(cnt));
            sg.clearSubgraph();
            cnt += 1;
        }
    }
    sg.setOutDegree();
    cout << "file name 2:" << to_string(cnt) << " cnt:" << cnt << endl;
    //sg.printSubgraph();
    sg.flushSubgraphToDisk(dirPath+"/"+to_string(cnt));
    sg.clearSubgraph();

    // Record the end time
    auto endTime = std::chrono::high_resolution_clock::now();
    // Calculate the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    // Print the duration
    std::cout << "Time taken by function: " << duration.count() << " microseconds" << std::endl;

    cout << "test deserialize" << endl;
    sg.deserialize(dirPath+"/"+to_string(cnt));

    initGraphFile.testReadFile();
    sg.printSubgraph();
    /* Code to test clas initGraphFile and subGraph end*/

    return 0;
}