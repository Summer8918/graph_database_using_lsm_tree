
#include "GraphMerge.h"
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <queue>
#include <chrono>
#include <map>

using namespace std;

#define MAX_EDGE_NUM_IN_MEMTABLE 1024 * 1024 * 4
#define LEVEL_0_CSR_FILE_NUM 1
#define MULTIPLE_BETWEEN_LEVEL 10
#define MAX_LEVEL_NUM 7

struct commandLine {
    int argc;
    char** argv;
    std::string comLine;
    commandLine(int _c, char** _v, std::string _cl)
            : argc(_c), argv(_v), comLine(_cl) {
        if (getOption("-h") || getOption("-help")) {
	        badArgument();
        }
    }

  commandLine(int _c, char** _v)
    : argc(_c), argv(_v), comLine("bad arguments") { }

  void badArgument() {
    std::cout << "usage: " << argv[0] << " " << comLine << std::endl;
    exit(0);
  }

  // get an argument
  // i is indexed from the last argument = 0, second to last indexed 1, ..
  char* getArgument(int i) {
    if (argc < 2 + i) {
        badArgument();
    }
    return argv[argc - 1 - i];
  }

  // looks for two filenames
  std::pair<char*,char*> IOFileNames() {
      if (argc < 3) {
          badArgument();
      }
      return std::pair<char*,char*>(argv[argc-2], argv[argc-1]);
  }

  std::pair<size_t,char*> sizeAndFileName() {
    if (argc < 3) {
        badArgument();
    }
    return std::pair<size_t,char*>(std::atoi(argv[argc-2]),(char*) argv[argc-1]);
  }

  bool getOption(std::string option) {
    for (int i = 1; i < argc; i++) {
      if ((std::string) argv[i] == option) {
        return true;
      }
    }
    return false;
  }

  char* getOptionValue(std::string option) {
    for (int i = 1; i < argc-1; i++) {
      if ((std::string) argv[i] == option) return argv[i+1];
    }
    return NULL;
  }

  std::string getOptionValue(std::string option, std::string defaultValue) {
    for (int i = 1; i < argc-1; i++) {
      if ((std::string) argv[i] == option) {
        return (std::string) argv[i+1];
      }
    }
    return defaultValue;
  }

  long getOptionLongValue(std::string option, long defaultValue) {
    for (int i = 1; i < argc-1; i++) {
      if ((std::string) argv[i] == option) {
	    long r = atol(argv[i+1]);
	    if (r < 0) {
            badArgument();
        }
	    return r;
      }
    }
    return defaultValue;
  }

  int getOptionIntValue(std::string option, int defaultValue) {
    for (int i = 1; i < argc-1; i++) {
      if ((std::string) argv[i] == option) {
	    int r = atoi(argv[i+1]);
	    if (r < 0) {
            badArgument();
        }
	    return r;
      }
    }
    return defaultValue;
  }

  double getOptionDoubleValue(std::string option, double defaultValue) {
    for (int i = 1; i < argc-1; i++) {
      if ((std::string) argv[i] == option) {
	    double val;
	    if (sscanf(argv[i+1], "%lf",  &val) == EOF) {
	        badArgument();
	    }
	    return val;
      }
    }
    return defaultValue;
  }
};

typedef struct _pair_uint {
  uint32_t x;
  uint32_t y;
} pair_uint;

std::vector<uint32_t> get_random_permutation(uint32_t num) {
	std::vector<uint32_t> perm(num);
	std::vector<uint32_t> vec(num);

	for (uint32_t i = 0; i < num; i++) {
		vec[i] = i;
  }

	uint32_t cnt = 0, n = 0;
	while (vec.size()) {
		n = vec.size();
		srand(time(NULL));
		uint32_t idx = rand() % n;
		uint32_t val = vec[idx];
		std::swap(vec[idx], vec[n-1]);
		vec.pop_back();
		perm[cnt++] = val;
	}
	return perm;
}

class MemTable{
public:
  uint64_t edgeNum;
  map<uint, vector<uint>> memTable;

  MemTable() {
    edgeNum = 0;
  }

  bool addEdge(int a, int b) {
    memTable[a].push_back(b);
    edgeNum++;
    if (edgeNum >= MAX_EDGE_NUM_IN_MEMTABLE) {
      return true;
    }
    return false;
  }

  void clearMemT() {
    edgeNum = 0;
    memTable.clear();
  }

  bool getNeighbors(uint targetId, vector<uint> &neighbors) {
    if (memTable.find(targetId) == memTable.end()) {
      neighbors.clear();
      return false;
    } else {
      neighbors.resize(memTable[targetId].size());
      neighbors = memTable[targetId];
      return true;
    }
  }
};

void removeFile(string &fileDir) {
  if(std::system(("rm " + fileDir + "o " + fileDir + "v").c_str()) == 0) {
    // std::cout << "Deleted existing directory: " << fileDir << std::endl;
  } else {
    std::cerr << "Failed to delete the existing directory: " << fileDir + "o " + fileDir + "v" 
        << std::endl;
    abort();
	}
}

class LSMTree {
public:
  std::vector<std::vector<FileMetaData>> lsmtreeOnDiskData;
  int cnt;  // to name the disk file
  MemTable memt;
  string dirPath;
  vector<__uint64_t> edgeNumLimitOfLevels;

  LSMTree (string dirPath_) : dirPath (dirPath_) {
    cnt = 0;
    // caution! difference between resize and reserve
    edgeNumLimitOfLevels.resize(MAX_LEVEL_NUM);
    lsmtreeOnDiskData.resize(MAX_LEVEL_NUM);
    edgeNumLimitOfLevels[0] = (LEVEL_0_CSR_FILE_NUM * MAX_EDGE_NUM_IN_MEMTABLE);
    for (int i = 1; i < MAX_LEVEL_NUM; i++) {
      edgeNumLimitOfLevels[i] = (MULTIPLE_BETWEEN_LEVEL * edgeNumLimitOfLevels[i - 1]);
    }
  }

  string getFileName() {
    return dirPath + "/" + to_string(cnt++);
  }

  void addEdge(uint a, uint b) {
    if (memt.addEdge(a, b)) {
      convertToCSR();
    }
  }

  void convertToCSR() {
    if (memt.edgeNum == 0) {
      cout << "memt is empty, no need to convert to csr" << endl;
      return;
    }

    Graph *graph = new Graph(0);
    int sz = memt.memTable.size();
    uint64_t offset = 0;
    int idx = 0;
    FileMetaData file;
    for (auto it = memt.memTable.begin(); it != memt.memTable.end(); it++) {
      //cout << "id in map:" << it->first << endl;
      //cout << "it->second.size():" << it->second.size() << endl;
      if (it->second.size() != 0) {
        graph->addVertex(0, it->first, it->second);
      }
    }

    string name = getFileName();
    graph->edgeNum = memt.edgeNum;

    // Add file meta data to level 0;

    file.edgeNum = memt.edgeNum;
    file.minNodeId = graph->vertexes.front().id;
    file.maxNodeId = graph->vertexes.back().id;
    graph->flushToDisk(name);

    file.fileName = name;
    file.vertexNum = sz;
    assert(lsmtreeOnDiskData[0].empty());
    lsmtreeOnDiskData[0].push_back(file);
    cout << "convertToCSR, CSR file info:" << endl;
    file.printDebugInfo();
    memt.clearMemT();

    delete graph;

    mayScheduleMerge();
  }

  // check lsmtreeOnDiskData to find if should do merge operation on each level.
  void mayScheduleMerge() {
    //cout << "In mayScheduleMerge lsmtreeOnDiskData.size()" << lsmtreeOnDiskData.size() << endl;
    for (int i = 0; i < MAX_LEVEL_NUM - 1; i++) {
      vector<FileMetaData> level = lsmtreeOnDiskData[i];
      uint TotalEdgeNum = 0;
      for (auto &f : level) {
        TotalEdgeNum += f.edgeNum;
      }
      // cout << "TotalEdgeNum:" << TotalEdgeNum << "edgeNumLimitOfLevels[i]" \
      //       << "i: " << i << " " << edgeNumLimitOfLevels[i] << endl;
      if (TotalEdgeNum >= edgeNumLimitOfLevels[i]) {
        // cout << "defore merge:" << endl;
        // getFileSizeInEachLevel();
        implMerge(i, i+1);
        // cout << "after merge:" << endl;
        // getFileSizeInEachLevel();
      }
    }
  }

  // Do merge operation on CSR file in levela and levelb.
  void implMerge(int levela, int levelb) {
    // cout << "Merging CSR files from Level " << levela << " to Level " << levelb << endl;

    // Assuming each level has a list of CSR files
    auto& levelAFiles = lsmtreeOnDiskData[levela];
    auto& levelBFiles = lsmtreeOnDiskData[levelb];

    // Check if there are files to merge from levela to levelb
    assert(!levelAFiles.empty());

    // levelb is empty
    if (levelBFiles.empty()) {
      // cout << "levelBFiles is empty " << endl;
      lsmtreeOnDiskData[levelb].push_back(lsmtreeOnDiskData[levela].front());
      lsmtreeOnDiskData[levela].clear();
      return;
    }
    FileMetaData fa = levelAFiles.front();
    FileMetaData fb = levelBFiles.front();

    FileMetaData fMerged;
    fMerged.fileName = getFileName();

    
    externalMergeSort2(fa, fb, fMerged); // Merge operation

    // Clear the files from the lower level after merging
    levelAFiles.clear();
    lsmtreeOnDiskData[levelb].pop_back();
    lsmtreeOnDiskData[levelb].push_back(fMerged);

    removeFile(fa.fileName);
    removeFile(fb.fileName);
    // cout << "Merging CSR file: " << fa.fileName << " and " << fb.fileName \
    //   << "Merge res:" << fMerged.fileName << endl;
  }

  void getFileSizeInEachLevel(void) {
    cout << "lsmtreeOnDiskData.size():" << lsmtreeOnDiskData.size() << endl;
    for (int i = 0; i < (int) lsmtreeOnDiskData.size(); ++i) {
      if (!lsmtreeOnDiskData[i].empty()) {
        cout << "level:" << i << endl;
        lsmtreeOnDiskData[i].front().printDebugInfo();
      } else {
        cout << "level:" << i << "empty."<< endl;
      }
    }
  }

  void mergeAllLevels() {
    // heap + external merge sort
    FileMetaData fMerged;
    fMerged.fileName = getFileName();
    multipleExternalMergeSort(lsmtreeOnDiskData, fMerged);
  }

  void ConvertCSRFileIntoGraphInMemory() {

  }

  void bfsOnGraphInMemory() {

  }

  void implBfs(uint src) {
    mergeAllLevels();
    ConvertCSRFileIntoGraphInMemory();
    bfsOnGraphInMemory();
  //   cout << "test bfs in LSM-tree" << endl;
  //   bitset<MAX_VERTEX_ID + 1> visitedBitMap;
  //   visitedBitMap.reset();
  //   queue<uint> q;
  //   q.push(src);
  //   visitedBitMap.set(src);
  //   vector<uint> neighbors;
  //   int visitedNodes = 1;
  //   int steps = 0;
  //   while (!q.empty()) {
  //     queue<uint> q1 = q;
  //     queue<uint> q2 = q1;
  //     steps++;
  //     // Traverse graph in Memtable
  //     for (int i = q2.size(); i > 0; i--) {
  //       uint id = q2.front();
  //       q2.pop();
  //       if(memt.getNeighbors(id, neighbors)) {
  //         //cout << "Neighbors size" << neighbors.size() << endl;
  //         for (auto & neighbor : neighbors) {
  //           if (!visitedBitMap[neighbor]) {
  //             visitedBitMap.set(neighbor);
  //             q.push(neighbor);
  //             visitedNodes++;
  //           }
  //         }
  //       }
  //     }

  //     // Traverse graph in each level of LSM-tree
  //     for (int i = 0; i < lsmtreeOnDiskData.size(); i++) {
  //       if (lsmtreeOnDiskData[i].empty()) {
  //         cout << "skip bfs on level:" << i << endl;
  //         continue;
  //       }
  //       cout << "Bfs on level:" << i << endl;
  //       q2 = q1;
  //       for (int j = q2.size(); j > 0; j--) {
  //         uint id = q2.front();
  //         //cout << "BFS visited id" << id << endl;
  //         q2.pop();
  //         Graph *graph = new Graph(0);
  //         graph->deserialize(lsmtreeOnDiskData[i].front().fileName);
  //         if(graph->getAllNeighbors(id, neighbors)) {
  //           //cout << "Neighbors size" << neighbors.size() << endl;
  //           for (auto & neighbor : neighbors) {
  //             if (!visitedBitMap[neighbor]) {
  //               visitedBitMap.set(neighbor);
  //               q.push(neighbor);
  //               visitedNodes++;
  //             }
  //           }
  //         }
  //       }
  //     }
  //     cout << "steps:" << steps << " visitedNodes" << visitedNodes << endl;
  //   }
  }
};


// void test_bfs(commandLine& P, LSMTree *lsmtree) {

//   long src = P.getOptionLongValue("-src", -1);
//   if (src == -1) {
//     std::cout << "Please specify a source vertex to run the BFS from" << std::endl;
//     exit(0);
//   }
//   lsmtree->bfs(src);
// }

// long execute(commandLine& P, std::string testname, LSMTree *lsmtree) {
//   // Record the start time
//   auto startTime = std::chrono::high_resolution_clock::now();
//   if (testname == "BFS") {
//     test_bfs(P, lsmtree);
//   } else {
//     std::cout << "Unknown test: " << testname << ". Quitting." << std::endl;
//   }
//   // Record the end time
//   auto endTime = std::chrono::high_resolution_clock::now();
//   // Calculate the duration in microseconds
//   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
//   return duration.count();
// }

// void test_bfs_on_subGraph(subGraph& G, commandLine& P) {
//   long src = P.getOptionLongValue("-src",-1);
//   if (src == -1) {
//     std::cout << "Please specify a source vertex to run the BFS from" << std::endl;
//     exit(0);
//   }
//   cout << "test bfs on subgraph" << endl;
//   bitset<MAX_VERTEX_ID + 1> visitedBitMap;
//   visitedBitMap.reset();
//   queue<uint> q;
//   q.push(src);
//   visitedBitMap.set(src);
//   vector<uint> neighbors;
//   int visitedNodeNum = 1;
//   int steps = 0;
//   while (!q.empty()) {
//     steps++;
//     for (int i = q.size(); i > 0; i--) {
//       uint id = q.front();
//       //cout << "BFS visited id" << id << endl;
//       q.pop();
//       if(G.getAllNeighbors(id, neighbors)) {
//         //cout << "Neighbors size" << neighbors.size() << endl;
//         for (auto & neighbor : neighbors) {
//           if (!visitedBitMap[neighbor]) {
//             visitedBitMap.set(neighbor);
//             q.push(neighbor);
//             visitedNodeNum++;
//             if (visitedNodeNum % 1000 == 0) {
//               cout << "visitedNodeNum" << visitedNodeNum << endl;
//             }
//           }
//         }
//       }
      
//     }
//     cout << "steps:" << steps << " visitedNodeNum" << visitedNodeNum << endl;
//   }
// }