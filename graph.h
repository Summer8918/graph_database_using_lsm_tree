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

// caution! let node be 8 byte aligned.
struct node {
    uint id;
    uint offset;
    uint outDegree;
    uint reserved;
    node () {
        id = 0;
        offset = 0;
        outDegree = 0;
        reserved = 0;
    }
};

struct graphHeader {
    int vertexNum;
    int outNeighborNum;
};

class Graph {
public:
    char *buf;
    vector<node> vertexes;
    vector<uint> outNeighbors;
    uint64_t edgeNum;
    uint off;
    uint read_from_start_offset;
    int posV;
    int posO;
    int v_read_bytes;
    int o_read_bytes;
    int vertexCnt;
    int neighborsCnt;

    Graph(uint off_) : off(off_) {
        buf = newA(char, MAX_SUB_GRAPH_STRUCT_SIZE);
        edgeNum = 0;
        read_from_start_offset = 0;
        posV = 0;
        posO = 0;
        v_read_bytes = 0;
        o_read_bytes = 0;
        vertexCnt = 0;
        neighborsCnt = 0;
        vertexes.clear();
        outNeighbors.clear();
    }

    ~Graph() {
        free(buf);
        vertexes.clear();
        outNeighbors.clear();
    }

    uint addVertex(uint preId, uint vertexId, vector<uint> &neighbors) {
        cout << "In addVertex vertexId:" << vertexId << " neighbors size:" <<
                neighbors.size() << " preId" << preId << endl;
        assert(preId <= vertexId);
        node n;
        n.id = vertexId;
        n.outDegree = neighbors.size();
        n.offset = off;
        off += neighbors.size();
        vertexes.push_back(n);
        /*
        cout << "In addVertex vertexes.size():" << vertexes.size() << endl;
        cout << "In addVertex neighbors.size():" << neighbors.size() << endl;
        for (auto x : neighbors) {
            cout << x << " ";
        }
        cout << endl;
        */
        outNeighbors.insert(outNeighbors.end(), neighbors.begin(), neighbors.end());
        return off;
    }

    int serializeVertexesHelper(vector<node> &data, int &pos, int blen, char *ptr) {
        int dlen = data.size();
        int sz = sizeof(data[pos]);
        cout << "vertexes.size():" << vertexes.size() << endl;
        while(pos < dlen && blen < MAX_BUF_SIZE) {
            memcpy(ptr, &vertexes[pos], sz);
            cout << "vertexes[pos].id" << data[pos].id << " od" << data[pos].outDegree << endl;
            ptr += sz;
            pos++;
            blen += sz;
        }
        return blen;
    }

    int serializeOutDegreeHelper(vector<uint> &data, int &pos, int blen, char *ptr) {
        int dlen = data.size();
        int sz = sizeof(data[pos]);
        while(pos < dlen && blen < MAX_BUF_SIZE) {
            memcpy(ptr, &data[pos], sz);
            //cout << "od: " << data[pos] << " ";
            ptr += sz;
            pos++;
            blen += sz;
        }
        //cout << endl;
        return blen;
    }

    void flushToDisk(string &fileName) {
        std::fstream outputFile;
        string vfile = fileName + "v";
        outputFile.open(vfile, ios::app | ios::binary);
        int pos = 0, len = vertexes.size(), blen = 0;  ////caution! init
        while (pos < len) {
            blen = serializeVertexesHelper(vertexes, pos, blen, buf);
            outputFile.write(buf, blen);
            blen = 0;
        }
        outputFile.close();
        outputFile.flush();

        string ofile = fileName + "o";
        outputFile.open(ofile, ios::app | ios::binary);
        pos = 0, blen = 0, len = outNeighbors.size();
        while (pos < len) {
            blen = serializeOutDegreeHelper(outNeighbors, pos, blen, buf);
            outputFile.write(buf, blen);
            blen = 0;
        }
        outputFile.close();
        outputFile.flush();
        vertexes.clear();  //caution!
        outNeighbors.clear();   //caution!
        cout << "flushToDisk success" << endl;
    }

    // reach end, return false;
    // not reach end, return true;
    bool readFileFromStart(node &vertex, vector<uint> &neighbors, string &fileName, 
            int vertex_num, int neighbors_num, uint minId, uint maxId) {
                cout << "read file:" << fileName << " vertex_num:" << vertex_num << 
            "neighbors_num:" << neighbors_num  << " minId:" << minId << " maxId:" <<
            maxId << endl;
        // read to the end
        if (vertexCnt >= vertex_num) {
            cout << "read to end" << endl;
            return false;
        }
        if (posV == vertexes.size()) {
            string vfile = fileName + "v";
            std::ifstream* vertexesPtr = new std::ifstream(vfile, std::ios::binary);
            if (!vertexesPtr->is_open()) {
                std::cout << "Unable to open file:" << vfile << std::endl;
                delete vertexesPtr;
                abort();
            }
            vertexesPtr->seekg(v_read_bytes, std::ios::beg);
            vertexesPtr->read(buf, MAX_BUF_SIZE);
            int bytesRead = (int)vertexesPtr->gcount();
            cout << "read_bytes:" << bytesRead << "vertexCnt" << vertexCnt << endl;
            v_read_bytes += bytesRead;
            cout << "v_read_bytes:" << v_read_bytes << endl;
            int sz = sizeof(node);
            int len = vertex_num - vertexCnt;
            if (len > (int)MAX_BUF_SIZE / sz) {
                len = (int)MAX_BUF_SIZE / sz;
            }
            vertexes.clear();  //caution!
            vertexes.reserve(len);
            char *ptr = buf;
            cout << "len:" << len << endl;
            node tmp_n;
            int cnt = 0;
            for (int i = 0; i < len; i++) {
                memcpy(&tmp_n, ptr, sz);
                vertexes.push_back(tmp_n);
                ptr += sz;
                if (vertexes[i].id > maxId || vertexes[i].outDegree > neighbors_num ||
                        vertexes[i].id < minId) { // == 174232
                    cout << "id: " << vertexes[i].id << " maxId:" << maxId \
                        << " minId:" << minId << " outDegree:"
                        << vertexes[i].outDegree << " neighbors_num:" << neighbors_num << endl;
                    abort();
                }
                //cout << "od:" << vertexes[i].outDegree << "id:" << vertexes[i].id << endl;
                cnt++;
            }
            cout << "cnt:" << cnt << endl;
            posV = 0;
            vertexesPtr->close();
            delete vertexesPtr;
        }
        neighbors.clear();
        neighbors.reserve(vertexes[posV].outDegree);
        neighborsCnt = 0;
        do {
            if (posO == outNeighbors.size()) {
                cout << "read outNeighbors from disk" << "neighborsCnt:" << neighborsCnt \
                        << "posO" << posO << "outNeighbors.size()"  << outNeighbors.size()\
                        << endl;
                string ofile = fileName + "o";
                std::ifstream* neighborsPtr = new std::ifstream(ofile, std::ios::binary);
                if (!neighborsPtr->is_open()) {
                    std::cout << "Unable to open file:" << ofile << std::endl;
                    abort();
                }
                neighborsPtr->seekg(o_read_bytes, std::ios::beg);
                neighborsPtr->read(buf, MAX_BUF_SIZE);
                int bytesRead = (int)neighborsPtr->gcount();
                o_read_bytes += bytesRead;
                int sz = sizeof(uint);
                int len = vertexes[posV].outDegree - neighborsCnt;
                if (len > (int)MAX_BUF_SIZE / sz) {
                    len = (int)MAX_BUF_SIZE / sz;
                }
                cout << "len:" << len << endl;
                outNeighbors.reserve(len);
                char *ptr = buf;
                for (int i = 0; i < len; i++) {
                    uint tmp_neighbor;
                    memcpy(&tmp_neighbor, ptr, sz);
                    outNeighbors.push_back(tmp_neighbor);
                    ptr += sz;
                }
                posO = 0;
                neighborsPtr->close();
                delete neighborsPtr;
            }
            cout << "outdegree" << vertexes[posV].outDegree << endl;
            cout << "neighborsCnt:" << neighborsCnt \
                        << "posO" << posO << "outNeighbors.size()"  << outNeighbors.size()\
                        << endl;
            while (posO < outNeighbors.size() && neighborsCnt < vertexes[posV].outDegree) {
                neighbors.push_back(outNeighbors[posO++]);
                neighborsCnt++;
            }
        } while (neighborsCnt < vertexes[posV].outDegree);

        vertex = vertexes[posV];
        vertexCnt++;
        posV++;
        cout << "vertexes.size():" << vertexes.size() << " posV" << posV << endl;
        cout << "outNeighbors.size()" << outNeighbors.size() << endl;
        cout << " vertexCnt" << vertexCnt << " neighborsCnt" << neighborsCnt 
                << " read_from_start_offset:" \
                << read_from_start_offset << " neighbors.size()" 
                << neighbors.size() << endl;
        return true;
    }
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

