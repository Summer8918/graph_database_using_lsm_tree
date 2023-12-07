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

void test_in_memory_graph(string &fileName, commandLine &P) {
    auto startTime = std::chrono::high_resolution_clock::now();
    InitGraphFile initGraphFile(fileName.c_str());
    uint a, b;
    subGraph *sg = new subGraph;
    int edgeNum = 0;
    while (initGraphFile.getLine(a, b)) {
        // cout << "a" << a << " b" << b << endl;
        edgeNum = sg->addEdge(a, b);
        
#ifdef ENABLE_DEBUG
        if (edgeNum % 100000 == 0) {
            cout << "edgeNum:" << edgeNum << endl;
        }
#endif
        if (edgeNum >= MAX_EDGE_NUM) {
            break;
        }
    }
    sg->setOutDegree();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    cout << "Time to construct the graph:" << duration.count() << endl;
    // Test the performance of BFS application.
    std::vector<std::string> test_ids = {"BFS"};
    size_t rounds = P.getOptionLongValue("-rounds", 4);
    for (auto test_id : test_ids) {
        double total_time = 0.0;
        for (size_t i = 0; i < rounds; i++) {
            auto tm = execute_on_in_memory_graph(*sg, P, test_id);

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
    delete sg;
}

void test_lsm_tree_graph(string &fileName, commandLine &P, string &dirPath) {
    auto startTime = std::chrono::high_resolution_clock::now();
    InitGraphFile initGraphFile(fileName.c_str());
    uint a, b;
    std::vector<uint32_t> srcs;
    std::vector<uint32_t> dests;
    LSMTree *lsmtree = new LSMTree(dirPath);
    uint64_t num_edges = 0;
    while (initGraphFile.getLine(a, b)) {
        // cout << "a" << a << " b" << b << endl;
        num_edges++;
        lsmtree->addEdge(a, b);
        srcs.push_back(a);
        dests.push_back(b);
        if (num_edges % 1000000 == 0) {
            cout << "num_edges:" << num_edges << endl;
        }
        if (num_edges >= MAX_EDGE_NUM) {
            break;
        }
    }
    cout << "Show the file sizes of each level" << endl;
    lsmtree->getFileSizeInEachLevel();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    cout << "Time to construct the graph:" << duration.count() << endl;

    auto perm = get_random_permutation(num_edges);
    // add edges in batch
    cout << "add edges in batch" << endl;
    startTime = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < num_edges; i++) {
        auto idx = perm[i];
        a = srcs[idx];
        b = dests[idx];
        lsmtree->addEdge(a, b);
    }
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    cout << "Time of adding edges in batch in the graph:" << duration.count() << endl;

    std::vector<std::string> test_ids = {"BFS"};
    size_t rounds = P.getOptionLongValue("-rounds", 4);
    // Test the performance of BFS application.
    for (auto test_id : test_ids) {
        double total_time = 0.0;
        for (size_t i = 0; i < rounds; i++) {
            auto tm = execute_on_lsmtree_graph(lsmtree, P, test_id);

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
}

int main(int argc, char** argv) {
    cout << "hello db" << endl;
    //string fileName = "partOfsocLiveJournal1.txt";
    string fileName = "soc-LiveJournal1.txt";

    InitGraphFile initGraphFile(fileName.c_str());

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
    
    commandLine P(argc, argv, "./graph_bm [-r rounds] [-src \
            a source vertex to run the BFS from] [-type 0: in_memory_graph, 1: lsm_tree_graph]");
    int type = P.getOptionLongValue("-type", 0);
    if (type == 0) {
        test_in_memory_graph(fileName, P);
    } else if (type == 1) {
        test_lsm_tree_graph(fileName, P, dirPath);
    }
    return 0;
}
