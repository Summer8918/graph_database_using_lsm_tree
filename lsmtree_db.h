
#include "GraphMerge.h"
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <queue>
#include <chrono>
#include <map>

using namespace std;

#define MAX_EDGE_NUM_IN_MEMTABLE 1024 * 512
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

struct FileMetaData {
  string fileName;
  uint minNodeId;
  uint maxNodeId;
  uint edgeNum;
  void printDebugInfo() {
    cout << "fileName:" << fileName << " minNodeId:" << minNodeId \
        << " maxNodeId" << maxNodeId << " edgeNum" << edgeNum << endl;
  }
};

class MemTable{
public:
  uint64_t edgeNum;
  map<uint, vector<uint>> memTable;

  int addEdge(int a, int b) {
    memTable[a].push_back(b);
    edgeNum++;
    if (edgeNum >= MAX_EDGE_NUM_IN_MEMTABLE) {
      cout << " neet to convertToCSR" << endl;
      return 1;
    }
    return 0;
  }

  void clearMemT() {
    edgeNum = 0;
    memTable.clear();
  }
};


void removeFile(string &fileDir) {
  if(std::system(("rm " + fileDir).c_str()) == 0) {
    std::cout << "Deleted existing directory: " << fileDir << std::endl;
  } else {
    std::cerr << "Failed to delete the existing directory: " << fileDir << std::endl;
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
    edgeNumLimitOfLevels.resize(MAX_LEVEL_NUM);
    lsmtreeOnDiskData.resize(MAX_LEVEL_NUM);
    edgeNumLimitOfLevels[0] = LEVEL_0_CSR_FILE_NUM * MAX_EDGE_NUM_IN_MEMTABLE;
    for (int i = 1; i < MAX_LEVEL_NUM; i++) {
      edgeNumLimitOfLevels[i] = MULTIPLE_BETWEEN_LEVEL * edgeNumLimitOfLevels[i - 1];
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
    subGraph *graph = new subGraph;
    int sz = memt.memTable.size();
    graph->vertexes.resize(sz);
    uint64_t offset = 0;
    int idx = 0;
    for (auto it = memt.memTable.begin(); it != memt.memTable.end(); it++) {
      //cout << "id in map:" << it->first << endl;
      node n;
      n.id = it->first;
      n.offset = offset;
      n.outDegree = it->second.size();
      graph->vertexes[idx++] = n;
      // note: the neighbors of a node is not sorted.
      for (auto & o : it->second) {
        graph->outNeighbors.push_back(o);
      }
      offset += it->second.size();
    }
    // Update totalLen and other necessary fields for csrGraph
    graph->header.vertexNum = graph->vertexes.size();
    graph->header.outNeighborNum = graph->outNeighbors.size();

    string name = getFileName();
    graph->edgeNum = memt.edgeNum;
    graph->serializeAndAppendBinToDisk(name);
    //cout << "file name:" << name << endl;

    // Add file meta data to level 0;
    FileMetaData file;
    file.edgeNum = memt.edgeNum;
    file.minNodeId = graph->vertexes.front().id;
    file.maxNodeId = graph->vertexes.back().id;
    file.fileName = name;
    lsmtreeOnDiskData[0].push_back(file);

    memt.clearMemT();

    // To verify
    //graph->clearSubgraph();
    //graph->deserialize(name);
    //graph->printSubgraph();

    delete graph;
    mayScheduleMerge();
  }

  // check lsmtreeOnDiskData to find if should do merge operation on each level.
  void mayScheduleMerge() {
    cout << "In mayScheduleMerge lsmtreeOnDiskData.size()" << lsmtreeOnDiskData.size() << endl;
    for (int i = 0; i < MAX_LEVEL_NUM - 1; i++) {
      vector<FileMetaData> level = lsmtreeOnDiskData[i];
      uint TotalEdgeNum = 0;
      for (auto &f : level) {
        TotalEdgeNum += f.edgeNum;
      }
      cout << "TotalEdgeNum:" << TotalEdgeNum << "edgeNumLimitOfLevels[i]" \
            << "i: " << i << " " << edgeNumLimitOfLevels[i] << endl;
      if (TotalEdgeNum >= edgeNumLimitOfLevels[i]) {
        cout << "defore merge:" << endl;
        debugInfo();
        implMerge(i, i+1);
        cout << "after merge:" << endl;
        debugInfo();
      }
    }
  }

  // Do merge operation on CSR file in levela and levelb.
  void implMerge(int levela, int levelb) {
    cout << "Merging CSR files from Level " << levela << " to Level " << levelb << endl;

    // Assuming each level has a list of CSR files
    auto& levelAFiles = lsmtreeOnDiskData[levela];
    auto& levelBFiles = lsmtreeOnDiskData[levelb];

    // Check if there are files to merge from levela to levelb
    if (levelAFiles.empty()) {
        cout << "No files to merge from Level " << levela << endl;
        return;
    }
    // levelb is empty
    if (levelBFiles.empty()) {
      cout << "levelBFiles is empty " << endl;
      lsmtreeOnDiskData[levelb].push_back(lsmtreeOnDiskData[levela].front());
      lsmtreeOnDiskData[levela].clear();
      return;
    }
    FileMetaData fa = levelAFiles.front();
    FileMetaData fb = levelBFiles.front();
    subGraph *gA = new subGraph();
    subGraph *gB = new subGraph();

    gA->deserialize(fa.fileName);
    gB->deserialize(fb.fileName);
    subGraph *mergedGraph = mergeGraphs(*gA, *gB); // Merge operation

    cout << "---gA----" << endl;
    gA->printSubgraph();
    cout << "---gB----" << endl;
    gB->printSubgraph();
    cout << "---mergedGraph----" << endl;
    mergedGraph->printSubgraph();

    delete gA;
    delete gB;
    FileMetaData mergedFile;
    string mergedFileName = getFileName();
    mergedGraph->serializeAndAppendBinToDisk(mergedFileName);
    mergedFile.fileName = mergedFileName;
    mergedFile.minNodeId = mergedGraph->vertexes.front().id;
    mergedFile.maxNodeId = mergedGraph->vertexes.back().id;
    mergedFile.edgeNum = mergedGraph->header.outNeighborNum;
    // Clear the files from the lower level after merging
    levelAFiles.clear();
    lsmtreeOnDiskData[levelb].pop_back();
    lsmtreeOnDiskData[levelb].push_back(mergedFile);

    removeFile(fa.fileName);
    removeFile(fb.fileName);
    delete mergedGraph;
    cout << "Merging CSR file: " << fa.fileName << " and " << fb.fileName \
      << "Merge res:" << mergedFile.fileName << endl;
  }

  void debugInfo() {
    for (int i = 0; i < lsmtreeOnDiskData.size(); ++i) {
      if (!lsmtreeOnDiskData[i].empty()) {
        cout << "Files in level i:" << i << endl;
        lsmtreeOnDiskData[i].front().printDebugInfo();
      }
    }
  }

  void bfs(uint src) {

  }
};

void test_bfs(subGraph& G, commandLine& P, LSMTree *lsmtree) {
/*
Concern: implement BFS on lsm-tree is very complex, as for each node,
we need to search its neighbors in all levels of lsm-tree.
The IO cost is extremely high, which equals to level_num * bfs_steps * csr_file_size.
*/
  long src = P.getOptionLongValue("-src", -1);
  if (src == -1) {
    std::cout << "Please specify a source vertex to run the BFS from" << std::endl;
    exit(0);
  }
  cout << "test bfs" << endl;
  bitset<MAX_VERTEX_ID + 1> visitedBitMap;
  visitedBitMap.reset();
  queue<uint> q;
  q.push(src);
  visitedBitMap.set(src);
  int visitedNeighborNum = 0;
  while (!q.empty()) {
    uint id = q.front();
    //cout << "BFS visited id" << id << endl;
    q.pop();
    visitedNeighborNum++;
    if (visitedNeighborNum % 10 == 0) {
      cout << "visitedNeighborNum:" << visitedNeighborNum << endl;
    }
    int index = G.search(id);
    if(id != -1){
      int numNeighbor = G.vertexes[index].outDegree;
      vector<uint> neighbors(numNeighbor);
      G.getAllNeighbors(index, neighbors);
    //if(neighbors.size()) {
      //cout << "Neighbors size" << neighbors.size() << endl;
      for (auto & neighbor : neighbors) {
        if (!visitedBitMap[neighbor]) {
          visitedBitMap.set(neighbor);
          q.push(neighbor);
        }
      }
    }
  }
}

long execute(subGraph& G, commandLine& P, std::string testname, LSMTree *lsmtree) {
  // Record the start time
  auto startTime = std::chrono::high_resolution_clock::now();
  if (testname == "BFS") {
    test_bfs(G, P, lsmtree);
  } else {
    std::cout << "Unknown test: " << testname << ". Quitting." << std::endl;
  }
  // Record the end time
  auto endTime = std::chrono::high_resolution_clock::now();
  // Calculate the duration in microseconds
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
  return duration.count();
}