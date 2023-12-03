#include "vertex.h"
#include "lsmtree_db.h"
#include <chrono>
#include <cstdlib>
#include <sys/stat.h>

using namespace std;

bool directoryExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0) && S_ISDIR(info.st_mode);
}

int main(int argc, char** argv) {
    cout << "hello db" << endl;
    string fileName = "soc-LiveJournal1.txt";

    InitGraphFile initGraphFile(fileName.c_str());

    uint a, b;
    subGraph sg;
    int cnt = 0;
    std::string dirPath = "tmp";
    if (directoryExists(dirPath)) {
	if(std::system(("rm -r " + dirPath).c_str()) == 0) {
            std::cout << "Deleted existing directory: " << dirPath << std::endl;
        } else {
            std::cerr << "Failed to delete the existing directory: " << dirPath << std::endl;
            abort();
	    }
    }
    if (std::system(("mkdir " + dirPath).c_str()) == 0) {
        std::cout << "Created new directory: " << dirPath << std::endl;
    } else {
        std::cerr << "Failed to create the new directory: " << dirPath << std::endl;
        return 1;
    }
    int edgeNum = 0;
    std::vector<uint32_t> srcs;
    std::vector<uint32_t> dests;
    int maxVertexId = 0;
    LSMTree *lsmtree = new LSMTree(dirPath);
    while (initGraphFile.getLine(a, b)) {
        // cout << "a" << a << " b" << b << endl;
        edgeNum = sg.addEdge(a, b);

        //lsmtree->addEdge(a, b);
        srcs.push_back(a);
        dests.push_back(b);
        if (edgeNum % 1000000 == 0) {
            cout << "edgeNum:" << edgeNum << endl;
        }
        if (edgeNum >= MAX_EDGE_NUM) {
            break;
        }
    }
    sg.setOutDegree();
    commandLine P(argc, argv, "./graph_bm [-r rounds] [-src \
            a source vertex to run the BFS from]");
    test_bfs_on_subGraph(sg, P);
    return 0;

    lsmtree->convertToCSR();
    maxVertexId = a;
    cout << "maxVertexId:" << maxVertexId << endl;
    cout << "edgeNum:" << edgeNum << endl;
    //sg.printSubgraph();
    string nameTmp = dirPath + "/l" + to_string(0);
    sg.serializeAndAppendBinToDisk(nameTmp);
    sg.clearSubgraph();
    sg.deserialize(nameTmp);
    //sg.printSubgraph();

    uint64_t num_edges = sg.edgeNum;
    auto perm = get_random_permutation(num_edges);

    // add edges in batch
    cout << "add edges in batch" << endl;
    for (uint64_t i = 0; i < num_edges; i++) {
        auto idx = perm[i];
        sg.addEdge(srcs[idx], dests[idx]);
        lsmtree->addEdge(a, b);
    }
    lsmtree->convertToCSR();
    std::vector<std::string> test_ids = {"BFS"};
    size_t rounds = P.getOptionLongValue("-rounds", 4);
    // Skip BFS application currently

    // Test the performance of BFS application.
    for (auto test_id : test_ids) {
        double total_time = 0.0;
        for (size_t i = 0; i < rounds; i++) {
            auto tm = execute(P, test_id, lsmtree);

            // std::cout << "RESULT"  << fixed << setprecision(6)
            std::cout << "\ttest=" << test_id
                    << "\ttime=" << tm
                    << "\titeration=" << i << std::endl;
            total_time += tm;
        }
        // std::cout << "RESULT (AVG)" << fixed << setprecision(6)
        std::cout << "AVG"
            << "\ttest=" << test_id
            << "\ttime=" << (total_time / rounds)
            << "\tgraph=" << fileName << std::endl;
    }
    delete lsmtree;
    return 0;
}
