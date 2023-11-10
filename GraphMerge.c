#include "GraphMerge.h"

// Test function for GraphMerge.h
int main() {
    // Graph edges array.
    graphEdge edges[] = {
        // (x, y, w) -> edge from x to y with weight w
        {0, 1, 2}, {0, 2, 4}, {1, 4, 3}, {2, 3, 2}, {3, 1, 4}, {4, 3, 3}
    };
    int numVertices = 6;  // Number of vertices in the graph
    int numEdges = sizeof(edges) / sizeof(edges[0]);

    // Construct graph
    DiaGraph diaGraph(edges, numEdges, numVertices);

    // Convert to CSR format
    subGraph csrGraph = convertToCSR(diaGraph);
    printCSRGraph(csrGraph);

    // Merge CSR graph with itself and print the result
    subGraph mergedGraph = mergeGraphs(csrGraph, csrGraph);
    printCSRGraph(mergedGraph);

    // Check for correctness
    bool correct = true;
    for (size_t i = 0; i < csrGraph.vertexes.size(); ++i) {
        if (mergedGraph.vertexes[i].outDegree != 2 * csrGraph.vertexes[i].outDegree) {
            correct = false;
            break;
        }
    }
    cout << "Merge correctness: " << (correct ? "Passed" : "Failed") << endl;

    return 0;
}