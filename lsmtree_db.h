#include "graph.h"
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <queue>
#include <chrono>
using namespace std;

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

void test_bfs(subGraph& G, commandLine& P) {
  long src = P.getOptionLongValue("-src",-1);
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
  vector<uint> neighbors;
  int visitedNeighborNum = 0;
  while (!q.empty()) {
    uint id = q.front();
    //cout << "BFS visited id" << id << endl;
    q.pop();
    visitedNeighborNum++;
    if (visitedNeighborNum % 10 == 0) {
      cout << "visitedNeighborNum:" << visitedNeighborNum << endl;
    }
    if(G.getAllNeighbors(id, neighbors)) {
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

long execute(subGraph& G, commandLine& P, std::string testname) {
  // Record the start time
  auto startTime = std::chrono::high_resolution_clock::now();
  if (testname == "BFS") {
    test_bfs(G, P);
  } else {
    std::cout << "Unknown test: " << testname << ". Quitting." << std::endl;
  }
  // Record the end time
  auto endTime = std::chrono::high_resolution_clock::now();
  // Calculate the duration in microseconds
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
  return duration.count();
}