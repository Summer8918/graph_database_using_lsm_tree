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
#include <omp.h>

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

/*
// Function to convert adjacency list graph to CSR graph
subGraph convertToCSR(const DiaGraph& adjListGraph) {
    subGraph csrGraph;
    // Assuming the node ids are 0...N-1
    csrGraph.vertexes.resize(adjListGraph.N);
    csrGraph.outNeighbors.reserve(adjListGraph.N); // Reserve enough space

    for (int i = 0; i < adjListGraph.N; ++i) {
        adjNode* current = adjListGraph.head[i];
        csrGraph.vertexes[i].id = i;
        csrGraph.vertexes[i].offset = csrGraph.outNeighbors.size();
        
        while (current != nullptr) {
            csrGraph.outNeighbors.push_back(current->val);
            current = current->next;
        }
        
        csrGraph.vertexes[i].outDegree = csrGraph.outNeighbors.size() - csrGraph.vertexes[i].offset;
    }
    // Update totalLen and other necessary fields for csrGraph
    csrGraph.header.vertexNum = csrGraph.vertexes.size();
    csrGraph.header.outNeighborNum = csrGraph.outNeighbors.size();

    return csrGraph;
}

void debugInfo(node &a, vector<uint> n) {
    cout << "id:" << a.id << " neighbors:" << endl;
    for (int i = 0; i < n.size(); i++) {
        cout << n[i] << " ";
    }
    cout << endl;
}
*/

/*
// Merge function for two CSR graphs that handles overlapping vertices
subGraph* mergeGraphs(const subGraph& G1, const subGraph& G2) {
    subGraph *newGraph = new subGraph;
    newGraph->vertexes.reserve(G1.vertexes.size() + G2.vertexes.size());
    newGraph->outNeighbors.reserve(G1.outNeighbors.size() + G2.outNeighbors.size());

    std::unordered_map<uintT, size_t> vertexMapping;
    for (const auto& v : G1.vertexes) {
        newGraph->vertexes.push_back(v);
        vertexMapping[v.id] = newGraph->vertexes.size() - 1;
        newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), \
            G1.outNeighbors.begin() + v.offset, \
            G1.outNeighbors.begin() + v.offset + v.outDegree);
    }

    // Parallelize the merging of vertices from G2
    #pragma omp parallel for
    for (size_t i = 0; i < G2.vertexes.size(); ++i) {
        const auto& v = G2.vertexes[i];
        auto it = vertexMapping.find(v.id);
        if (it != vertexMapping.end()) {
            // Handle overlapping vertices
            node& existingNode = newGraph->vertexes[it->second];
            size_t newOffset = newGraph->outNeighbors.size();
            std::vector<uintT> tempNeighbors;
            tempNeighbors.reserve(v.outDegree);
            for (size_t j = 0; j < v.outDegree; ++j) {
                uintT neighbor = G2.outNeighbors[v.offset + j];
                tempNeighbors.push_back(neighbor);
            }
            #pragma omp critical
            {
                newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), tempNeighbors.begin(), tempNeighbors.end());
                existingNode.outDegree += tempNeighbors.size();
                if (existingNode.outDegree > 0) {
                    existingNode.offset = newOffset;
                }
            }
        } else {
            // Handle unique vertices
            node newNode = v;
            newNode.offset = newGraph->outNeighbors.size();
            std::vector<uintT> tempNeighbors(G2.outNeighbors.begin() + v.offset, G2.outNeighbors.begin() + v.offset + v.outDegree);
            #pragma omp critical
            {
                newGraph->vertexes.push_back(newNode);
                vertexMapping[v.id] = newGraph->vertexes.size() - 1;
                newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), tempNeighbors.begin(), tempNeighbors.end());
            }
        }
    }

    //newGraph.totalLen = newGraph.vertexes.size() * sizeof(node) + newGraph.outNeighbors.size() * sizeof(uintT);
    newGraph->header.vertexNum = newGraph->vertexes.size();
    newGraph->header.outNeighborNum = newGraph->outNeighbors.size();
    newGraph->edgeNum = newGraph->header.outNeighborNum;
    return newGraph;
}

// Utility function to print CSR graph
void printCSRGraph(const subGraph& csrGraph) {
    cout << "CSR Graph:" << endl;
    cout << "Vertexes (id, offset, outDegree):" << endl;
    for (const node& n : csrGraph.vertexes) {
        cout << "(" << n.id << ", " << n.offset << ", " << n.outDegree << ") ";
    }
    cout << "\nOutNeighbors:" << endl;
    for (unsigned int n : csrGraph.outNeighbors) {
        cout << n << " ";
    }
    //cout << "\nTotal Length: " << csrGraph.totalLen << endl;
}

// // Test function for GraphMerge.h
// int main() {
//     // Graph edges array.
//     graphEdge edges[] = {
//         // (x, y, w) -> edge from x to y with weight w
//         {0, 1, 2}, {0, 2, 4}, {1, 4, 3}, {2, 3, 2}, {3, 1, 4}, {4, 3, 3}
//     };
//     int numVertices = 6;  // Number of vertices in the graph
//     int numEdges = sizeof(edges) / sizeof(edges[0]);

//     // Construct graph
//     DiaGraph diaGraph(edges, numEdges, numVertices);

//     // Convert to CSR format
//     subGraph csrGraph = convertToCSR(diaGraph);
//     printCSRGraph(csrGraph);

//     // Merge CSR graph with itself and print the result
//     subGraph mergedGraph = mergeGraphs(csrGraph, csrGraph);
//     printCSRGraph(mergedGraph);

//     // Check for correctness
//     bool correct = true;
//     for (size_t i = 0; i < csrGraph.vertexes.size(); ++i) {
//         if (mergedGraph.vertexes[i].outDegree != 2 * csrGraph.vertexes[i].outDegree) {
//             correct = false;
//             break;
//         }
//     }
//     cout << "Merge correctness: " << (correct ? "Passed" : "Failed") << endl;

//     return 0;
// }
*/
#endif  // GRAPHMERGE_H
