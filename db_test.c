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
    // test class Graph begin
    /*
    vector<uint> neighbors = {5,6,3,1,2};
    Graph *graph = new Graph(0);
    string tmp = "tmp/tmp_file";
    int vn = 0, on = 0;
    for (int i = 0; i < 10; i++) {
        vn++;
        on += neighbors.size();
        graph->addVertex(i, neighbors);
        graph->flushToDisk(tmp);
        neighbors.push_back(i);
    }
    node n;
    while (graph->readFileFromStart(n, neighbors, tmp, vn, on)) {
        cout << "vid:" << n.id << " neighbors:" << endl;
        for (int i = 0; i < neighbors.size(); i++) {
            cout << neighbors[i] << " ";
        }
        cout << endl;
    }
    */
    // test class Graph end

    cout << "hello db" << endl;
    string fileName = "soc-LiveJournal1.txt";
    InitGraphFile initGraphFile(fileName.c_str());
    int edgeCnt = 0;
    uint a, b;
    std::vector<uint32_t> srcs;
    std::vector<uint32_t> dests;

    LSMTree *lsmtree = new LSMTree(dirPath);
    while (initGraphFile.getLine(a, b)) {
        lsmtree->addEdge(a, b);
        srcs.push_back(a);
        dests.push_back(b);
        edgeCnt++;
        if (edgeCnt >= MAX_EDGE_NUM) {
            break;
        }
    }
    return 0;

    /*

    
    subGraph sg;
    int cnt = 0;
    
    Graph* graph = new Graph();
    int maxVertexId = 0;
    LSMTree *lsmtree = new LSMTree(dirPath);
    uint offset = 0;
    string test_graph_file = "tmp/graph";
    int edgeCnt = 0;
    while (initGraphFile.getLine(a, b)) {
        // cout << "a" << a << " b" << b << endl;
        //edgeNum = sg.addEdge(a, b);
        graph->addEdge(a, b);
        //lsmtree->addEdge(a, b);
        srcs.push_back(a);
        dests.push_back(b);
        edgeCnt++;
        if (edgeCnt % 100 == 0) {
            offset += graph->serializeAndAppendBinToDisk(test_graph_file, offset);
            cout << "offset:" << offset << endl;
        }
    }
    //
    graph->serializeAndAppendBinToDisk(test_graph_file, offset);
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
    */
}
