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

struct graphHeader {
    int vertexNum;
    int outNeighborNum;
};

class subGraph {
public:
    char *buf1;
    char *buf2;
    vector<node> vertexes;
    graphHeader header;
    vector<int> outNeighbors;
    uint64_t edgeNum;
    bool deserialized_nghbrs_ = false;

    subGraph() {
        edgeNum = 0;
        buf1 = newA(char, MAX_SUB_GRAPH_STRUCT_SIZE);
        buf2 = newA(char, MAX_SUB_GRAPH_STRUCT_SIZE);
    }

    ~subGraph() {
        clearSubgraph();
        free(buf1);
        free(buf2);
    }

    int addEdge(uint a, uint b) {
        edgeNum++;
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
        return edgeNum;
    }

    void setOutDegree(void) {
        cout << "vertexes.size()" << vertexes.size() << endl;
        int sz = vertexes.size(), szo = outNeighbors.size();
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
        vertexes.clear();
        outNeighbors.clear();
    }

    void printSubgraph(void) {
        cout << "vertexes array size:" << vertexes.size() << endl;
        cout << "edgeNum:" << edgeNum << endl;
        // /*
        for (node &n : vertexes) {
            cout << "id:" << n.id << " offset:" << n.offset << " outDegree:" \
                    << n.outDegree << endl;
        }
        // */
        cout << "outNeighbors array size:" << outNeighbors.size() << endl;
        // /*
        int count = 0;
        for (auto &o : outNeighbors) {
            if (count <= 46)
                cout << o << " ";
            // if (count == 100)
            //     break;
            count++;
        }
        // */
        cout << endl;
    }

    int serializeOutDegreeHelper(vector<int> &data, int &pos, int blen, char *ptr) {
        int dlen = data.size();
        int sz = sizeof(data[pos]);
        while(pos < dlen && blen < MAX_BUF_SIZE) {
            memcpy(ptr, &data[pos], sz);
            ptr += sz;
            pos++;
            blen += sz;
        }
        return blen;
    }

    int serializeVertexesHelper(vector<node> &data, int &pos, int blen, char *ptr) {
        int dlen = data.size();
        int sz = sizeof(data[pos]);
        while(pos < dlen && blen < MAX_BUF_SIZE) {
            memcpy(ptr, &vertexes[pos], sz);
            ptr += sz;
            pos++;
            blen += sz;
        }
        return blen;
    }

    void serializeAndAppendBinToDisk(string &fileName, string &nghbr_fileName) {
        cout << "f: " << fileName << " " << nghbr_fileName << endl;
        header.vertexNum = vertexes.size();
        header.outNeighborNum = outNeighbors.size();
        char *ptr = buf1;
        

        int pos = 0, blen = 0;
        memcpy(ptr, &header, sizeof(graphHeader));
        ptr += sizeof(graphHeader);
        cout << "vertexesSize:" << header.vertexNum << endl;
        cout << "outNeighborsSize:" << header.outNeighborNum << endl;
        std::fstream outputFile, nghbr_outputFile;
        outputFile.open(fileName, ios::app | ios::binary);
        outputFile.write(buf1, sizeof(graphHeader));
        while (pos < header.vertexNum ) {
            blen = serializeVertexesHelper(vertexes, pos, blen, buf1);
            // outputFile.write(reinterpret_cast<const char*>(&blen), sizeof(int));
            outputFile.write(buf1, blen);
            blen = 0;
        }
        outputFile.close();
        outputFile.flush();
        nghbr_outputFile.open(nghbr_fileName, ios::app | ios::binary);
        pos = 0;
        while (pos < header.outNeighborNum) {
            blen = serializeOutDegreeHelper(outNeighbors, pos, blen, buf2);
            // nghbr_outputFile.write(reinterpret_cast<const char*>(&blen), sizeof(int));
            nghbr_outputFile.write(buf2, blen);
            blen = 0;
        }
        nghbr_outputFile.close();
        nghbr_outputFile.flush();
#ifdef ENABLE_DEBUG
        cout << "serializeAndAppendBinToDisk success" << endl;
#endif
    }

    void deserialize(string fileName, string nghbr_fileName) {
        std::ifstream* filePtr = new std::ifstream(fileName, std::ios::binary);
        if (!filePtr->is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        filePtr->seekg(0, std::ios::end);
        int len = (int)filePtr->tellg();;
        filePtr->seekg(0, std::ios::beg);
        filePtr->read(buf1, sizeof(graphHeader));
        // std::cout << "Deserialize file length:" << len << endl;
        // Get the number of bytes read
        std::streamsize bytesRead = filePtr->gcount();

        //std::cout << "Read " << bytesRead << " bytes." << std::endl;
        
        memcpy(&header, buf1, sizeof(graphHeader));
        // cout << "In deserialize vertexesSize:" << header.vertexNum << endl;
        // cout << "In deserialize outNeighborsSize:" << header.outNeighborNum << endl;
        vertexes.resize(header.vertexNum);
        outNeighbors.resize(header.outNeighborNum);
        len -= sizeof(graphHeader);
        int vertexIdx = 0, outNeighIdx = 0;
        int plen;
        while (len > 0) {
            // filePtr->read(reinterpret_cast<char*>(&plen), sizeof(int));
            // int bytesRead = (int)filePtr->gcount();
            // filePtr->read(buf1, plen);
            filePtr->read(buf1, MAX_BUF_SIZE);
            char *ptr1 = buf1;
            char *ptr2 = buf2;
            // Get the number of bytes read
            int bytesRead = (int)filePtr->gcount();
            len -= bytesRead;
            //std::cout << "Read " << bytesRead << " bytes." << std::endl;
            while (bytesRead > 0) {
                if (vertexIdx < header.vertexNum) {
                    memcpy(&vertexes[vertexIdx], ptr1, sizeof(node));
                    //cout << "vertexes[vertexIdx]:" << vertexes[vertexIdx].id << endl;
                    ptr1 += sizeof(node);
                    vertexIdx++;
                    bytesRead -= sizeof(node);
                } else {
                    cout <<  "No" << endl;
                }
                //cout << "bytesRead:" << bytesRead << endl;
            }
        }
        edgeNum = header.outNeighborNum;
        delete filePtr;
        filePtr = new std::ifstream(nghbr_fileName, std::ios::binary);
        if (!filePtr->is_open()) {
            std::cout << "Unable to open file:" << nghbr_fileName << std::endl;
            abort();
        }
        filePtr->seekg(0, std::ios::end);
        len = (int)filePtr->tellg();;
        filePtr->seekg(0, std::ios::beg);
        while (len > 0) {
            // filePtr->read(reinterpret_cast<char*>(&plen), sizeof(int));
            // int bytesRead = (int)filePtr->gcount();
            // filePtr->read(buf1, plen);
            filePtr->read(buf1, MAX_BUF_SIZE);
            char *ptr1 = buf1;
            // Get the number of bytes read
            int bytesRead = (int)filePtr->gcount();
            len -= bytesRead;
            //std::cout << "Read " << bytesRead << " bytes." << std::endl;
            while (bytesRead > 0) {
                if (outNeighIdx < header.outNeighborNum) {
                    memcpy(&outNeighbors[outNeighIdx], ptr1, sizeof(int));
                    //cout << "outNeighbors[outNeighIdx]:" << outNeighbors[outNeighIdx] << endl;
                    ptr1 += sizeof(int);
                    outNeighIdx++;
                    bytesRead -= sizeof(int);
                } else {
                    cout << " OutNghbr: should not reach here" << endl;
                }
                //cout << "bytesRead:" << bytesRead << endl;
            }
        }
        delete filePtr;
        // cout << "Deserialize success" << endl;
        // printSubgraph();
    }

    void deserialize_vertex(string fileName) {
        std::ifstream* filePtr = new std::ifstream(fileName, std::ios::binary);
        if (!filePtr->is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        filePtr->seekg(0, std::ios::end);
        int len = (int)filePtr->tellg();;
        filePtr->seekg(0, std::ios::beg);
        filePtr->read(buf1, sizeof(graphHeader));
        // std::cout << "Deserialize file length:" << len << endl;
        // Get the number of bytes read
        std::streamsize bytesRead = filePtr->gcount();

        //std::cout << "Read " << bytesRead << " bytes." << std::endl;
        
        memcpy(&header, buf1, sizeof(graphHeader));
        // cout << "In deserialize vertexesSize:" << header.vertexNum << endl;
        // cout << "In deserialize outNeighborsSize:" << header.outNeighborNum << endl;
        vertexes.resize(header.vertexNum);
        len -= sizeof(graphHeader);
        int vertexIdx = 0, outNeighIdx = 0;
        int plen;
        while (len > 0) {
            // filePtr->read(reinterpret_cast<char*>(&plen), sizeof(int));
            // int bytesRead = (int)filePtr->gcount();
            // filePtr->read(buf1, plen);
            filePtr->read(buf1, MAX_BUF_SIZE);
            char *ptr1 = buf1;
            char *ptr2 = buf2;
            // Get the number of bytes read
            int bytesRead = (int)filePtr->gcount();
            len -= bytesRead;
            //std::cout << "Read " << bytesRead << " bytes." << std::endl;
            while (bytesRead > 0) {
                if (vertexIdx < header.vertexNum) {
                    memcpy(&vertexes[vertexIdx], ptr1, sizeof(node));
                    //cout << "vertexes[vertexIdx]:" << vertexes[vertexIdx].id << endl;
                    ptr1 += sizeof(node);
                    vertexIdx++;
                    bytesRead -= sizeof(node);
                } else {
                    cout << "Shouldn't reach here" << endl;
                }
                //cout << "bytesRead:" << bytesRead << endl;
            }
        }
        edgeNum = header.outNeighborNum;
        delete filePtr;
        // cout << "Deserialize success" << endl;
        // printSubgraph();
    }

    void deserialize_nghbr(string fileName, string nghbr_fileName) {
        std::ifstream* filePtr = new std::ifstream(fileName, std::ios::binary);
        if (!filePtr->is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        filePtr->seekg(0, std::ios::end);
        int len = (int)filePtr->tellg();;
        filePtr->seekg(0, std::ios::beg);
        filePtr->read(buf1, sizeof(graphHeader));
        delete filePtr;

        memcpy(&header, buf1, sizeof(graphHeader));
        // cout << "In deserialize vertexesSize:" << header.vertexNum << endl;
        // cout << "In deserialize outNeighborsSize:" << header.outNeighborNum << endl;
        outNeighbors.resize(header.outNeighborNum);

        filePtr = new std::ifstream(nghbr_fileName, std::ios::binary);
        if (!filePtr->is_open()) {
            std::cout << "Unable to open file:" << nghbr_fileName << std::endl;
            abort();
        }
        filePtr->seekg(0, std::ios::end);
        len = (int)filePtr->tellg();;
        filePtr->seekg(0, std::ios::beg);
        int outNeighIdx = 0;
        while (len > 0) {
            // filePtr->read(reinterpret_cast<char*>(&plen), sizeof(int));
            // int bytesRead = (int)filePtr->gcount();
            // filePtr->read(buf1, plen);
            filePtr->read(buf1, MAX_BUF_SIZE);
            char *ptr1 = buf1;
            // Get the number of bytes read
            int bytesRead = (int)filePtr->gcount();
            len -= bytesRead;
            //std::cout << "Read " << bytesRead << " bytes." << std::endl;
            while (bytesRead > 0) {
                if (outNeighIdx < header.outNeighborNum) {
                    memcpy(&outNeighbors[outNeighIdx], ptr1, sizeof(int));
                    //cout << "outNeighbors[outNeighIdx]:" << outNeighbors[outNeighIdx] << endl;
                    ptr1 += sizeof(int);
                    outNeighIdx++;
                    bytesRead -= sizeof(int);
                } else {
                    cout << " OutNghbr: should not reach here" << endl;
                }
                //cout << "bytesRead:" << bytesRead << endl;
            }
        }
        delete filePtr;
        deserialized_nghbrs_ = true;
        // cout << "Deserialize success" << endl;
        // printSubgraph();
    }

    // Todo: when the vertexes is sorted according to id, use binary search.
    int search(uint targetId) {
        int lo = 0, hi = vertexes.size() -1, mid = 0;
        // cout << “Doing search ” << targetId << endl;
        while (lo <= hi) {
            mid = lo + (hi - lo) / 2;
            if (vertexes[mid].id == targetId) {
                return mid;
            }
            if (vertexes[mid].id < targetId) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        //cout << "targetId:" << targetId << endl;
        // for (int i = 0; i < vertexes.size(); i++) {
        //     //cout << "id" << vertexes[i].id << endl;
        //     if (vertexes[i].id == targetId) {
        //         return i;
        //     }
        // }
        // cout << “Done search ” << targetId << endl;
        return -1;
        /*
        while (lo <= hi) {
            mid = lo + (hi - lo) / 2;
            cout << “id:” << vertexes[mid].id << endl;
            if (vertexes[mid].id == targetId) {
                return mid;
            } else if (vertexes[mid].id < targetId) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        */
        // return -1;
    }
    bool getAllNeighbors(int idx, vector<uint> &neighbors) {
        //int idx = search(targetId);
        if (idx == -1) {
            //cout << "fail to get targetId" << targetId << endl;
            return false;
        }
        int neighborNum = vertexes[idx].outDegree;

        // cout << neighborNum << " " << idx << std::endl;

        //vector<uint> new_vect(neighborNum);
        //cout << "idx:" << idx << " neighborNum:" << neighborNum << endl;
        //neighbors.resize(neighborNum);
        int endPos = vertexes[idx].offset + neighborNum;
        for (int i = vertexes[idx].offset, cnt = 0; i < endPos; ++i, ++cnt) {
            neighbors[cnt] = outNeighbors[i];
            // cout << "outNeighbor" << i << " " << outNeighbors[i] << std::endl;
        }
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
        // cout << "fileLen:" << fileLen << endl;
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