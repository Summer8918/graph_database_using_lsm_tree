#ifndef GRAPHMERGE_H
#define GRAPHMERGE_H

// #include "ligra.h"
#include "graph.h"
#include "adj_list_graph.h"
#include "utils.h"
#include <unordered_map>
#include <algorithm>
#include <vector>

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
    csrGraph.totalLen = csrGraph.vertexes.size() * sizeof(node) + csrGraph.outNeighbors.size() * sizeof(uint);
    csrGraph.vertexesSize = csrGraph.vertexes.size();
    csrGraph.outNeighborsSize = csrGraph.outNeighbors.size();
    
    return csrGraph;
}

// Merge function for two CSR graphs that handles overlapping vertices
subGraph mergeGraphs(const Graph& G1, const Graph& G2) {
    subGraph newGraph;

    // Initialize the newGraph with appropriate size.
    // Note: Depending on the original `graph.h` implementation, you may need to initialize more fields here.
    newGraph.vertexes.reserve(G1.vertexes.size() + G2.vertexes.size());
    newGraph.outNeighbors.reserve(G1.outNeighbors.size() + G2.outNeighbors.size());

    // Insert all vertices and edges from G1 into newGraph
    for (const auto& v : G1.vertexes) {
        newGraph.vertexes.push_back(v);
    }
    newGraph.outNeighbors.insert(newGraph.outNeighbors.end(), G1.outNeighbors.begin(), G1.outNeighbors.end());

    // Mapping from G2's vertex id to the index in the newGraph's vertexes
    std::unordered_map<uintT, size_t> vertexMapping;

    // Append vertices from G2
    for (const auto& v : G2.vertexes) {
        auto it = vertexMapping.find(v.id);
        if (it != vertexMapping.end()) {
            // Vertex already exists in G1, so merge adjacency lists
            node& existingNode = newGraph.vertexes[it->second];
            size_t originalDegree = existingNode.outDegree;
            existingNode.outDegree += v.outDegree;
            size_t newOffset = newGraph.outNeighbors.size();
            
            // Update offset if the node already had neighbors
            if (originalDegree > 0) {
                existingNode.offset = newOffset;
            }
            
            // Append G2's adjacency list for the vertex to newGraph
            newGraph.outNeighbors.insert(newGraph.outNeighbors.end(),
                                         G2.outNeighbors.begin() + v.offset,
                                         G2.outNeighbors.begin() + v.offset + v.outDegree);
        } else {
            // Vertex is unique to G2
            node newNode = v;
            newNode.offset = newGraph.outNeighbors.size(); // Update offset to new position
            newGraph.vertexes.push_back(newNode);
            vertexMapping[v.id] = newGraph.vertexes.size() - 1;
            newGraph.outNeighbors.insert(newGraph.outNeighbors.end(),
                                         G2.outNeighbors.begin() + v.offset,
                                         G2.outNeighbors.begin() + v.offset + v.outDegree);
        }
    }

    // Update the totalLen and other necessary fields for the newGraph
    newGraph.totalLen = newGraph.vertexes.size() * sizeof(node) + newGraph.outNeighbors.size() * sizeof(uint);
    // You would also want to update vertexesSize and outNeighborsSize if they are used in the `subGraph` class
    newGraph.vertexesSize = newGraph.vertexes.size();
    newGraph.outNeighborsSize = newGraph.outNeighbors.size();

    return newGraph;
}

#endif  // GRAPHMERGE_H