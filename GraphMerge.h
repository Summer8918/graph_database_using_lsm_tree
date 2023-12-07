#ifndef GRAPHMERGE_H
#define GRAPHMERGE_H

// #include "ligra.h"
#include "graph.h"
#include "adj_list_graph.h"
#include "utils.h"
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <omp.h>
#include <unordered_set>

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

// Merge function for two CSR graphs that handles overlapping vertices
subGraph* mergeGraphs(const subGraph& G1, const subGraph& G2) {
    std::unordered_map<uintT, size_t> vertexMapping;
    subGraph *newGraph = new subGraph;
    newGraph->vertexes.reserve(G1.vertexes.size() + G2.vertexes.size());
    newGraph->outNeighbors.reserve(G1.outNeighbors.size() + G2.outNeighbors.size());

    // Merging vertices from G1
    for (const auto& vertex : G1.vertexes) {
        newGraph->vertexes.push_back(vertex);
        vertexMapping[vertex.id] = newGraph->vertexes.size() - 1;
        newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), 
                                      G1.outNeighbors.begin() + vertex.offset, 
                                      G1.outNeighbors.begin() + vertex.offset + vertex.outDegree);
    }

    // Merging vertices from G2
    for (const auto& vertex : G2.vertexes) {
        auto it = vertexMapping.find(vertex.id);
        if (it != vertexMapping.end()) {
            // Handle overlapping vertices
            node& existingNode = newGraph->vertexes[it->second];
            std::vector<uintT> uniqueNeighbors(newGraph->outNeighbors.begin() + existingNode.offset,
                                               newGraph->outNeighbors.begin() + existingNode.offset + existingNode.outDegree);

            uniqueNeighbors.insert(uniqueNeighbors.end(),
                                   G2.outNeighbors.begin() + vertex.offset,
                                   G2.outNeighbors.begin() + vertex.offset + vertex.outDegree);

            std::sort(uniqueNeighbors.begin(), uniqueNeighbors.end());
            uniqueNeighbors.erase(std::unique(uniqueNeighbors.begin(), uniqueNeighbors.end()), uniqueNeighbors.end());

            existingNode.offset = newGraph->outNeighbors.size();
            existingNode.outDegree = uniqueNeighbors.size();
            newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), uniqueNeighbors.begin(), uniqueNeighbors.end());
        } else {
            // Handle unique vertices
            node newNode = vertex;
            newNode.offset = newGraph->outNeighbors.size();
            std::vector<uintT> tempNeighbors(G2.outNeighbors.begin() + vertex.offset, 
                                             G2.outNeighbors.begin() + vertex.offset + vertex.outDegree);
            std::sort(tempNeighbors.begin(), tempNeighbors.end());

            newGraph->vertexes.push_back(newNode);
            vertexMapping[vertex.id] = newGraph->vertexes.size() - 1;
            newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), tempNeighbors.begin(), tempNeighbors.end());
        }
    }

    // Sort the vertices in newGraph by their IDs
    std::sort(newGraph->vertexes.begin(), newGraph->vertexes.end(), 
              [](const node& a, const node& b) { return a.id < b.id; });

    // Update vertex offsets after sorting
    size_t offset = 0;
    for (auto& vertex : newGraph->vertexes) {
        vertex.offset = offset;
        offset += vertex.outDegree;
    }

    newGraph->header.vertexNum = newGraph->vertexes.size();
    newGraph->header.outNeighborNum = newGraph->outNeighbors.size();
    newGraph->edgeNum = newGraph->header.outNeighborNum;

    return newGraph;
}


// Merge function for two CSR graphs that handles overlapping vertices
subGraph* mergeGraphsV2(const subGraph &G1, const subGraph & G2) {
    subGraph *newGraph = new subGraph;
    int len1 = G1.vertexes.size();
    int len2 = G2.vertexes.size();
    int p1 = 0, p2 = 0;
    uint offset = 0;
    node tmp;
    while (p1 < len1 && p1 < len2) {
        uint id1 = G1.vertexes[p1].id;
        uint id2 = G2.vertexes[p2].id;
        if (id1 < id2) {
            offset = newGraph->outNeighbors.size();
            newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), 
                G1.outNeighbors.begin() + G1.vertexes[p1].offset,
                G1.outNeighbors.begin() + G1.vertexes[p1].offset + G1.vertexes[p1].outDegree);
            // tmp = G1.vertexes[p1];
            tmp.id = G1.vertexes[p1].id;
            tmp.outDegree = G1.vertexes[p1].outDegree;
            tmp.offset = offset;
            newGraph->vertexes.push_back(tmp);
            p1++;
        } else if (id1 > id2) {
            offset = newGraph->outNeighbors.size();
            newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), 
                G2.outNeighbors.begin() + G2.vertexes[p2].offset,
                G2.outNeighbors.begin() + G2.vertexes[p2].offset + G2.vertexes[p2].outDegree);
            // tmp = G2.vertexes[p2];
            tmp.id = G2.vertexes[p2].id;
            tmp.outDegree = G2.vertexes[p2].outDegree;
            tmp.offset = offset;
            newGraph->vertexes.push_back(tmp);
            p2++;
        } else {
            vector<uint> newNeighbors(G1.outNeighbors.begin() + G1.vertexes[p1].offset,
                G1.outNeighbors.begin() + G1.vertexes[p1].offset + G1.vertexes[p1].outDegree);
            std::unordered_set<uintT> uniqueNeighbors(G1.outNeighbors.begin() + G1.vertexes[p1].offset,
                G1.outNeighbors.begin() + G1.vertexes[p1].offset + G1.vertexes[p1].outDegree);

            int len = G2.vertexes[p2].offset + G2.vertexes[p2].outDegree;
            for (int i = G2.vertexes[p2].offset; i < len; i++) {
                if (uniqueNeighbors.find(G2.outNeighbors[i]) == uniqueNeighbors.end()) {
                    newNeighbors.push_back(G2.outNeighbors[i]);
                }
            }
            offset = newGraph->outNeighbors.size();
            newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), 
                    newNeighbors.begin(), newNeighbors.end());
            p1++;
            p2++;
            tmp.offset = offset;
            tmp.outDegree = newNeighbors.size();
            tmp.id = id1;
            newGraph->vertexes.push_back(tmp);
        }
    }
    while (p1 < len1) {
        offset = newGraph->outNeighbors.size();
        newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), 
                G1.outNeighbors.begin() + G1.vertexes[p1].offset,
                G1.outNeighbors.begin() + G1.vertexes[p1].offset + G1.vertexes[p1].outDegree);
        // tmp = G1.vertexes[p1];
        tmp.id = G1.vertexes[p1].id;
        tmp.outDegree = G1.vertexes[p1].outDegree;
        tmp.offset = offset;
        newGraph->vertexes.push_back(tmp);
        p1++;
    }

    while (p2 < len2) {
        offset = newGraph->outNeighbors.size();
        newGraph->outNeighbors.insert(newGraph->outNeighbors.end(), 
                G2.outNeighbors.begin() + G2.vertexes[p2].offset,
                G2.outNeighbors.begin() + G2.vertexes[p2].offset + G2.vertexes[p2].outDegree);
        // tmp = G2.vertexes[p2];
        tmp.id = G2.vertexes[p2].id;
        tmp.outDegree = G2.vertexes[p2].outDegree;
        tmp.offset = offset;
        newGraph->vertexes.push_back(tmp);
        p2++;
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

#endif  // GRAPHMERGE_H