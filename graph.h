#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdlib.h>
#include "utils.h"

using namespace std;

struct node {
    uint id;
    uint offset;
};

class subGraph {
public:
    subGraph() {
        length = 0;
    }

    ~subGraph() {
        clearSubgraph();
    }

    int getLenght() {
        return length;
    }

    int addEdge(uint a, uint b) {
        if (!vertexes.empty()) {
            node backNode = vertexes.back();
            if (backNode.id != a) {
                backNode.id = a;
                backNode.offset = outNeighbors.size();
                vertexes.push_back(backNode);
            }
            outNeighbors.push_back(b);
            length += sizeof(b);
        } else {
            node tmp;
            tmp.id = a;
            outNeighbors.push_back(b);
            tmp.offset = 0;
            vertexes.push_back(tmp);
            length += sizeof(a) * 2;
        }
        //cout << "length:" << length << endl;
        return length;
    }

    void clearSubgraph(void) {
        length = 0;
        vertexes.clear();
        outNeighbors.clear();
    }

    void printSubgraph(void) {
        //cout << "vertexes array size:" << vertexes.size() << endl;
        for (node &n : vertexes) {
            cout << "id:" << n.id << " offset:" << n.offset << endl;
        }
        
        cout << "outNeighbors array:" << endl;
        for (auto &o : outNeighbors) {
            cout << o << " ";
        }
        cout << endl;
    }

    // TODO:
    void flushSubgraphToDisk();
private:
int length;
vector<node> vertexes;
vector<int> outNeighbors;
};

class InitGraphFile {
public:
    InitGraphFile(char *fileName) {
        fName = fileName;
        inputFile.open(fileName, ios::in | ios::binary | ios::ate);
        if (!inputFile.is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        inputFile.seekg(0, std::ios::beg);
    }

    bool getLine(uint &a, uint &b) {
        string line;
        while (std::getline(inputFile, line)) {
            if (line.find('#') != string::npos) {
                continue;
            }
            istringstream iss(line);
            if (iss >> a >> b) {
                //cout << "vertex1: " << a << " vertex2: " << b << endl;
                return true;
            }
        }
        cout << "return false" << endl;
        return false;
    }

    ~InitGraphFile() {
        if (inputFile.is_open()) {
            inputFile.close();
        }
    }
private:
    char * fName;
    std::ifstream inputFile;
};