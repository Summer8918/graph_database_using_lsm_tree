#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdlib.h>
#include "utils.h"

using namespace std;

class InitGraphFile {
public:
    InitGraphFile(char *fileName, uint chukSize_) {
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
                cout << "vertex1: " << a << " vertex2: " << b << endl;
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
    uint pos;
    uint offset;
    char * fName;
    uint chunkSize;
    char *buffer;
    std::ifstream inputFile;
    long end;
};
