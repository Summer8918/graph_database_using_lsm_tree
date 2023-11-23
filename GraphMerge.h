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
subGraph mergeGraphs(const subGraph& G1, const subGraph& G2) {
    subGraph newGraph;
    newGraph.vertexes.reserve(G1.vertexes.size() + G2.vertexes.size());
    newGraph.outNeighbors.reserve(G1.outNeighbors.size() + G2.outNeighbors.size());

    std::unordered_map<uintT, size_t> vertexMapping;
    for (const auto& v : G1.vertexes) {
        newGraph.vertexes.push_back(v);
        vertexMapping[v.id] = newGraph.vertexes.size() - 1;
        newGraph.outNeighbors.insert(newGraph.outNeighbors.end(), G1.outNeighbors.begin() + v.offset, G1.outNeighbors.begin() + v.offset + v.outDegree);
    }

    // Parallelize the merging of vertices from G2
    #pragma omp parallel for
    for (size_t i = 0; i < G2.vertexes.size(); ++i) {
        const auto& v = G2.vertexes[i];
        auto it = vertexMapping.find(v.id);
        if (it != vertexMapping.end()) {
            // Handle overlapping vertices
            node& existingNode = newGraph.vertexes[it->second];
            size_t newOffset = newGraph.outNeighbors.size();
            std::vector<uintT> tempNeighbors;
            tempNeighbors.reserve(v.outDegree);
            for (size_t j = 0; j < v.outDegree; ++j) {
                uintT neighbor = G2.outNeighbors[v.offset + j];
                tempNeighbors.push_back(neighbor);
            }
            #pragma omp critical
            {
                newGraph.outNeighbors.insert(newGraph.outNeighbors.end(), tempNeighbors.begin(), tempNeighbors.end());
                existingNode.outDegree += tempNeighbors.size();
                if (existingNode.outDegree > 0) {
                    existingNode.offset = newOffset;
                }
            }
        } else {
            // Handle unique vertices
            node newNode = v;
            newNode.offset = newGraph.outNeighbors.size();
            std::vector<uintT> tempNeighbors(G2.outNeighbors.begin() + v.offset, G2.outNeighbors.begin() + v.offset + v.outDegree);
            #pragma omp critical
            {
                newGraph.vertexes.push_back(newNode);
                vertexMapping[v.id] = newGraph.vertexes.size() - 1;
                newGraph.outNeighbors.insert(newGraph.outNeighbors.end(), tempNeighbors.begin(), tempNeighbors.end());
            }
        }
    }

    //newGraph.totalLen = newGraph.vertexes.size() * sizeof(node) + newGraph.outNeighbors.size() * sizeof(uintT);
    newGraph.header.vertexNum = newGraph.vertexes.size();
    newGraph.header.outNeighborNum = newGraph.outNeighbors.size();

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

#endif  // GRAPHMERGE_H