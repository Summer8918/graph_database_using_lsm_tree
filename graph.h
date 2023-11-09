#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <cassert>
#include "utils.h"

using namespace std;

struct node {
    uint id;
    uint offset;
    uint outDegree;
    node () {
        id = 0;
        offset = 0;
        outDegree = 0;
    }
};

class subGraph {
public:
    subGraph() {
        totalLen = 0;
        buf = newA(char, MAX_SUB_GRAPH_STRUCT_SIZE);
    }

    ~subGraph() {
        clearSubgraph();
        free(buf);
    }

    int getLenght() {
        return totalLen;
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
        } else {
            node tmp;
            tmp.id = a;
            outNeighbors.push_back(b);
            tmp.offset = 0;
            vertexes.push_back(tmp);
        }
        totalLen = sizeof(totalLen) + sizeof(vertexesSize) + sizeof(outNeighborsSize) +\
                vertexes.size() * sizeof(node) + outNeighbors.size() * sizeof(int);
        //cout << "totalLen:" << totalLen << endl;
        return totalLen;
    }

    void setOutDegree(void) {
        uint sz = vertexes.size(), szo = outNeighbors.size();
        for (int i = 0; i < sz - 1; i++) {
            vertexes[i].outDegree = vertexes[i+1].offset - vertexes[i].offset;
        }
        //cout << "sz:" << sz << " vertexes[sz - 2].offset" << vertexes[sz - 2].offset << endl;
        if (sz >= 2) {
            vertexes[sz - 1].outDegree = szo - vertexes[sz - 1].offset;
        } else if (sz == 1) {
            vertexes[sz - 1].outDegree = szo;
        }
        //cout << "vertexes[sz - 1].outDegree " << vertexes[sz - 1].outDegree << endl;
    }

    void clearSubgraph(void) {
        totalLen = sizeof(outNeighborsSize) + sizeof(vertexesSize);
        vertexes.clear();
        outNeighbors.clear();
    }

    void printSubgraph(void) {
        //cout << "vertexes array size:" << vertexes.size() << endl;
        for (node &n : vertexes) {
            cout << "id:" << n.id << " offset:" << n.offset << " outDegree:" \
                    << n.outDegree << endl;
        }
        
        cout << "outNeighbors array size:" << outNeighbors.size() << endl;
        for (auto &o : outNeighbors) {
            cout << o << " ";
        }
        cout << endl;
    }

    void serialize() {
        char *ptr = buf;
        if (ptr == NULL) {
            cout << "ptr is NULL" << endl;
        } else {
            //cout << "ptr is valid" << endl;
        }
        assert(totalLen <= MAX_SUB_GRAPH_STRUCT_SIZE);
        if (totalLen > MAX_SUB_GRAPH_STRUCT_SIZE) {
            cout << "error! totalLen " << totalLen << endl;
        }
        memcpy(ptr, &totalLen, sizeof(totalLen));
        ptr += sizeof(totalLen);
        memcpy(ptr, &vertexesSize, sizeof(vertexesSize));
        ptr += sizeof(vertexesSize);
        memcpy(ptr, &outNeighborsSize, sizeof(outNeighborsSize));
        ptr += sizeof(outNeighborsSize);
        //cout << "outNeighborsSize:" << outNeighborsSize << " vertexesSize:" << vertexesSize << endl;
        for (int i = 0; i < vertexesSize; i++) {
            memcpy(ptr, &vertexes[i], sizeof(vertexes[i]));
            //cout << "vertexes: i" << i << " " << vertexes[i].id << endl;
            ptr += sizeof(vertexes[i]);
        }
        for (int i = 0; i < outNeighborsSize; i++) {
            //cout << "outNeighbors: i" << i << " " << outNeighbors[i] << endl;
            memcpy(ptr, &outNeighbors[i], sizeof(outNeighbors[i]));
            ptr += sizeof(outNeighbors[i]);
        }
    }

    void deserialize(string fileName) {
        std::ifstream* filePtr = new std::ifstream(fileName, std::ios::binary);
        if (!filePtr->is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        filePtr->seekg(0, std::ios::end);
        std::streampos fileSize = filePtr->tellg();
        int len = (int)fileSize;
        if (len > MAX_SUB_GRAPH_STRUCT_SIZE) {
            std::cout << "length of file:" << fileName << "is " << len <<\
                    " , it is larger than buffer"<< std::endl;
            //abort();
        }
        filePtr->seekg(0, std::ios::beg);
        filePtr->read(buf, min(len, MAX_SUB_GRAPH_STRUCT_SIZE));
        char *ptr = buf;
        memcpy(&totalLen, ptr, sizeof(totalLen));
        ptr += sizeof(totalLen);
        memcpy(&vertexesSize, ptr, sizeof(vertexesSize));
        ptr += sizeof(vertexesSize);
        memcpy(&outNeighborsSize, ptr, sizeof(outNeighborsSize));
        ptr += sizeof(outNeighborsSize);
        vertexes.resize(vertexesSize);
        outNeighbors.resize(outNeighborsSize);
        for (int i = 0; i < vertexesSize; i++) {
            memcpy(&vertexes[i], ptr, sizeof(vertexes[i]));
            ptr += sizeof(vertexes[i]);
        }
        for (int i = 0; i < outNeighborsSize; i++) {
            memcpy(&outNeighbors[i], ptr, sizeof(outNeighbors[i]));
            ptr += sizeof(outNeighbors[i]);
        }
        delete filePtr;
    }

    bool flushSubgraphToDisk(string fileName) {
        vertexesSize = vertexes.size();
        outNeighborsSize = outNeighbors.size();
        serialize();
        std::fstream outputFile;
        outputFile.open(fileName, ios::out);
        if (!outputFile) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        //cout << "write file length is:" << totalLen << endl;
        outputFile.write(buf, totalLen);
        //cout << "write file:" << fileName << "success!" << endl;
        if (!outputFile) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        outputFile.close();
        //cout << "close file:" << fileName << "success!" << endl;
        return true;
    }

private:
int totalLen;
char *buf;
int vertexesSize;
vector<node> vertexes;
int outNeighborsSize;
vector<int> outNeighbors;
};

class InitGraphFile {
public:
    InitGraphFile(const char *fileName) {
        fName = fileName;
        inputFile.open(fileName, ios::in | ios::binary | ios::ate);
        if (!inputFile.is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        // Move the file pointer to the end of the file
        inputFile.seekg(0, std::ios::end);

        // Get the file length
        fileLen = (int)inputFile.tellg();
        remainLen = fileLen;
        cout << "fileLen:" << fileLen << endl;
        inputFile.seekg(0, std::ios::beg);
        buffer = newA(char, READ_INPUT_FILE_BUFFER_SIZE);
        inputFile.read(buffer, READ_INPUT_FILE_BUFFER_SIZE);
        pos = 0;
    }

    bool getLine(uint &a, uint &b) {
        //cout << "getLine" << endl;
        while (remainLen > 0) {
            string tmp;
            while (pos < READ_INPUT_FILE_BUFFER_SIZE && pos < remainLen && buffer[pos] != '\n') {
                //cout << "buffer[pos]" << buffer[pos] << endl;
                tmp += buffer[pos];
                pos++;
            }
            //cout << "tmp:" << tmp << "pos" << pos << "remainLen" << remainLen << endl;
            if (pos < READ_INPUT_FILE_BUFFER_SIZE  && pos < remainLen && buffer[pos] == '\n') {
                if (tmp.find('#') != string::npos) {
                    pos++;
                    continue;
                }
                istringstream iss(tmp);
                if (iss >> a >> b) {
                    //cout << "vertex1: " << a << " vertex2: " << b << endl;
                    
                    pos++;
                    return true;
                }
                pos++;
            }
            int tmpLen = tmp.length();
            if (pos == READ_INPUT_FILE_BUFFER_SIZE || pos == remainLen) {
                //cout << "remainLen 1:" << remainLen << endl;
                if (pos >= remainLen) {
                    remainLen = 0;
                } else {
                    remainLen -= (READ_INPUT_FILE_BUFFER_SIZE - tmpLen);
                }
                memcpy(buffer, tmp.c_str(), tmpLen);
                inputFile.read(buffer + tmpLen, READ_INPUT_FILE_BUFFER_SIZE - tmpLen);
                pos = 0;
            }
            //cout << "remainLen 2:" << remainLen << endl;
        }
        return false;
    }

    bool testReadFile() {
        int cnt = 0;
        while (inputFile) {
            inputFile.read(buffer, READ_INPUT_FILE_BUFFER_SIZE);
            cout << "cnt:" << cnt << endl;
            cnt += 1;
        }
        return true;
    }

    ~InitGraphFile() {
        if (inputFile.is_open()) {
            inputFile.close();
        }
    }
private:
    const char * fName;
    std::ifstream inputFile;
    char *buffer;
    int pos;
    int fileLen;
    int remainLen;
};