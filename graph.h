#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "utils.h"
using namespace std;

class InitGraphFile {
public:
    InitGraphFile(char *fileName, uint chukSize_) {
        offset = 0;
        fName = fileName;
        chunkSize = chukSize_;
        buffer = newA(char, chunkSize);
        inputFile.open(fileName, ios::in | ios::binary | ios::ate);
        if (!inputFile.is_open()) {
            std::cout << "Unable to open file:" << fileName << std::endl;
            abort();
        }
        // Move the file pointer to the end of the file
        inputFile.seekg(0, std::ios::end);

        // Get the current position of the file pointer, which is the file's size
        end = inputFile.tellg();
    }

    long readChunk() {
        inputFile.seekg(offset, ios::beg);
        long n = end - inputFile.tellg();
        if (n > chunkSize) {
            n = chunkSize;
        }
        cout << "end:" << end << " n:" << n << " offset" << offset << endl;
        inputFile.read(buffer, n);
        offset += n;
        return n;
    }

    char *getBuffer(void) {
        return buffer;
    }

    ~InitGraphFile() {
        if (inputFile.is_open()) {
            inputFile.close();
        }
    }
private:
    uint offset;
    char * fName;
    uint chunkSize;
    char *buffer;
    std::ifstream inputFile;
    long end;
};
