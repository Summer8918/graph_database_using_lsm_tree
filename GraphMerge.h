#ifndef GRAPHMERGE_H
#define GRAPHMERGE_H

// #include "ligra.h"
#include "graph.h"
#include "adj_list_graph.h"
#include "utils.h"
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <queue>

struct FileMetaData {
  FileMetaData() {
    minNodeId = 0;
    maxNodeId = 0;
    edgeNum = 0;
    vertexNum = 0;
  }

  string fileName;
  uint minNodeId;
  uint maxNodeId;
  uint edgeNum;  // out neighbors size
  uint vertexNum;
  void printDebugInfo() {
    cout << "fileName:" << fileName << " minNodeId:" << minNodeId \
        << " maxNodeId:" << maxNodeId << " edgeNum:" << edgeNum \
        << " vertexNum:" << vertexNum << endl;
    std::ifstream file(fileName + "v", std::ios::binary | std::ios::ate);
    // Check if the file is open
    if (!file.is_open()) {
        std::cerr << "Failed to open the vertexes file." << std::endl;
        return;
    }

    std::streampos fileSize = file.tellg();

    std::cout << "Vertexes File size: " << fileSize / (1024) << " kB" << std::endl;
    file.close();

    std::ifstream neighborFile(fileName + "o", std::ios::binary | std::ios::ate);
    // Check if the file is open
    if (!neighborFile.is_open()) {
        std::cerr << "Failed to open the neighbors file." << std::endl;
        return;
    }

    fileSize = neighborFile.tellg();

    std::cout << "Neighbor File size: " << fileSize / (1024) << " kB" << std::endl;
    file.close();
  }
};

void validate(FileMetaData &file) {
    node nb;
    vector<uint> neighborsB;
    Graph *gB = new Graph(0);
    cout << "Validate after merge" << endl;
    string name = file.fileName;
    while (gB->readFileFromStart(nb, neighborsB, name, 
        file.vertexNum, file.edgeNum, file.maxNodeId, file.minNodeId)) {
        cout << "id:" << nb.id << " neighbors:" << endl;
        for (int i = 0; i < neighborsB.size(); i++) {
            cout << neighborsB[i] << " ";
        }
        cout << endl;
    }
    delete gB;
}

void externalMergeSort(FileMetaData &fA, FileMetaData &fB, FileMetaData &fM) {
    vector<uint> neighborsA, neighborsB;
    Graph *gA = new Graph(0);
    Graph *gB = new Graph(0);
    // cout << "Merge two files" << endl;
    // fA.printDebugInfo();
    // fB.printDebugInfo();
    node na, nb;
    bool graph_a_flag = gA->readFileFromStart(na, neighborsA, fA.fileName, 
            fA.vertexNum, fA.edgeNum, fA.minNodeId, fA.maxNodeId);

    bool graph_b_flag = gB->readFileFromStart(nb, neighborsB, fB.fileName, 
            fB.vertexNum, fB.edgeNum, fB.minNodeId, fB.maxNodeId);

    Graph *gOutput = new Graph(0);
    uint edgeNum = 0;
    fM.minNodeId = min(fA.minNodeId, fB.minNodeId);
    fM.maxNodeId = max(fA.maxNodeId, fB.maxNodeId);

    uint preId = 0;
    while (graph_a_flag == true && graph_b_flag == true) {
        // cout << "na id:" << na.id << " nb id:" << nb.id << endl;
        if (na.id == nb.id) {
            unordered_set<uint> neighborsSet(neighborsA.begin(), neighborsA.end());
            // cout << "neighborsA.size()" << neighborsA.size() << endl;
            // cout << "neighborsB.size()" << neighborsB.size() << endl;
            std::vector<uint> mergedNeighbors = neighborsA;
            for (auto & b : neighborsB) {
                if (neighborsSet.count(b) == 0) {
                    mergedNeighbors.push_back(b);
                }
            }
            assert(neighborsA.size() + neighborsB.size() >= mergedNeighbors.size());
            edgeNum += mergedNeighbors.size();
            fM.edgeNum += mergedNeighbors.size();
            //cout << "mergedNeighbors.size():" << mergedNeighbors.size() << endl;
            gOutput->addVertex(preId, na.id, mergedNeighbors);
            preId = na.id;

            graph_b_flag = gB->readFileFromStart(nb, neighborsB, fB.fileName, 
                    fB.vertexNum, fB.edgeNum, fB.minNodeId, fB.maxNodeId);

            graph_a_flag = gA->readFileFromStart(na, neighborsA, fA.fileName, 
                    fA.vertexNum, fA.edgeNum, fA.minNodeId, fA.maxNodeId);
        } else if (na.id < nb.id) {
            gOutput->addVertex(preId, na.id, neighborsA);
            preId = na.id;
            assert(neighborsA.size() <= fA.edgeNum);
            edgeNum += neighborsA.size();
            fM.edgeNum += neighborsA.size();
            //cout << "merged neighborsA.size():" << neighborsA.size() << endl;
            graph_a_flag = gA->readFileFromStart(na, neighborsA, fA.fileName, 
                    fA.vertexNum, fA.edgeNum, fA.minNodeId, fA.maxNodeId);
        } else {
            edgeNum += neighborsB.size();
            fM.edgeNum += neighborsB.size();
            gOutput->addVertex(preId, nb.id, neighborsB);
            preId = nb.id;
            assert(neighborsB.size() <= fB.edgeNum);
            //cout << "merged neighborsB.size():" << neighborsB.size() << endl;
            graph_b_flag = gB->readFileFromStart(nb, neighborsB, fB.fileName, 
                    fB.vertexNum, fB.edgeNum, fB.minNodeId, fB.maxNodeId);
        }
        if (edgeNum >= FLUSH_EDGE_NUM_LIMIT) {
            gOutput->flushToDisk(fM.fileName);
            edgeNum = 0;
        }
        fM.vertexNum++;
        // cout << "edgeNum:" << edgeNum << endl;
    }

    while (graph_a_flag) {
        gOutput->addVertex(preId, na.id, neighborsA);
        preId = na.id;
        assert(neighborsA.size() <= fA.edgeNum);
        edgeNum += neighborsA.size();
        fM.edgeNum += neighborsA.size();
        //fM.printDebugInfo();
        if (edgeNum >= FLUSH_EDGE_NUM_LIMIT) {
            gOutput->flushToDisk(fM.fileName);
            edgeNum = 0;
        }
        graph_a_flag = gA->readFileFromStart(na, neighborsA, fA.fileName, 
                    fA.vertexNum, fA.edgeNum, fA.minNodeId, fA.maxNodeId);
        fM.vertexNum++;
    }
    while (graph_b_flag) {
        gOutput->addVertex(preId, nb.id, neighborsB);
        preId = nb.id;
        assert(neighborsB.size() <= fB.edgeNum);
        edgeNum += neighborsB.size();
        fM.edgeNum += neighborsB.size();
        if (edgeNum >= FLUSH_EDGE_NUM_LIMIT) {
            gOutput->flushToDisk(fM.fileName);
            edgeNum = 0;
        }
        graph_b_flag = gB->readFileFromStart(nb, neighborsB, fB.fileName, 
                    fB.vertexNum, fB.edgeNum, fB.minNodeId, fB.maxNodeId);
        fM.vertexNum++;
    }
    gOutput->flushToDisk(fM.fileName);
    delete gA;
    delete gB;

    if (fM.vertexNum > fA.vertexNum + fB.vertexNum) {
        fB.printDebugInfo();
        fA.printDebugInfo();
        fM.printDebugInfo();
        cout << "fM.vertexNum > fA.vertexNum + fB.vertexNum" << endl;
        abort(); 
    }
    if (fB.edgeNum + fA.edgeNum < fM.edgeNum) {
        fB.printDebugInfo();
        fA.printDebugInfo();
        fM.printDebugInfo();
        cout << "fB.edgeNum + fA.edgeNum < fM.edgeNum" << endl;
        abort();
    }
    //validate(fM);
    // cout << "External merge file success!" << endl;
    // fM.printDebugInfo();
    delete gOutput;
}

struct pqElement {
    Graph *graph;
    node n;
    FileMetaData file;
    vector<uint> *neighbors;
    void clearElement(void) {
        delete graph;
        delete neighbors;
    }
    ~pqElement() {
        clearElement();
    }
};

struct myCmp{
    bool operator()(pqElement* const a, pqElement* const b) {
        return a->n.id > b->n.id;
    }
};

void multipleExternalMergeSort(std::vector<std::vector<FileMetaData>> &lsmtreeOnDiskData, \
        FileMetaData &fM) {
    priority_queue<pqElement*, vector<pqElement*>, myCmp> pq;
    for (int i = 0; i < lsmtreeOnDiskData.size(); ++i) {
        for (int j = 0; j < lsmtreeOnDiskData[i].size(); j++) {
            pqElement * tmp = new pqElement();
            tmp->graph = new Graph(0);
            tmp->file = lsmtreeOnDiskData[i][j];
            cout << "debug info in multipleExternalMergeSort:" << endl;
            lsmtreeOnDiskData[i][j].printDebugInfo();
            tmp->neighbors = new vector<uint>();
            bool flag = tmp->graph->readFileFromStart(tmp->n, *(tmp->neighbors), tmp->file.fileName, 
                    tmp->file.vertexNum, tmp->file.edgeNum, tmp->file.minNodeId, tmp->file.maxNodeId);
            if (flag) {
                pq.push(tmp);
            } else {
                delete tmp;
            }
        }
    }
    cout << "pq.size()" << pq.size() << endl;
    fM.minNodeId = pq.top()->n.id;
    Graph *gOutput = new Graph(0);
    uint preId = 0;
    while (!pq.empty()) {
        pqElement * top = pq.top();
        if (preId > top->n.id) {
            cout << "addVertex preId:" << preId << "top->n.id" << top->n.id << endl;
        }
        gOutput->addVertex(preId, top->n.id, *(top->neighbors));
        preId = top->n.id;

        pq.pop();
        fM.edgeNum += top->neighbors->size();
        bool flag = top->graph->readFileFromStart(top->n, *(top->neighbors), top->file.fileName,
                    top->file.vertexNum, top->file.edgeNum, top->file.minNodeId,
                    top->file.maxNodeId);
        fM.vertexNum++;
        if (flag) {
            pq.push(top);
        } else {
            delete top;
        }
    }
    fM.maxNodeId = preId;
    gOutput->flushToDisk(fM.fileName);
    cout << "debug info in multipleExternalMergeSort, merged file info:" << endl;
    fM.printDebugInfo();
    delete gOutput;
}

void externalMergeSort2(FileMetaData &fA, FileMetaData &fB, FileMetaData &fM) {
    std::vector<std::vector<FileMetaData>> tmp;
    std::vector<FileMetaData> tmp2;
    tmp2.push_back(fA);
    tmp2.push_back(fB);
    tmp.push_back(tmp2);
    multipleExternalMergeSort(tmp, fM);
}

#endif  // GRAPHMERGE_H
