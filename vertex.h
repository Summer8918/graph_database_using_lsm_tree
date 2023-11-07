#ifndef VERTEX_H
#define VERTEX_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

typedef unsigned int uintE;
typedef unsigned int uintT;

// Suppose the graph is unweighted and asummetric
struct asymmetricVertex {
    uintE* inNeighbors;
    uintE* outNeighbors;
    uintT outDegree;
    uintT inDegree;

    asymmetricVertex(uintE *iN, uintE *oN, uintT id, uintT od)
        : inNeighbors(iN),
          outNeighbors(oN),
          inDegree(id),
          outDegree(od) {}
    
    uintE* getInNeighbors () {
        return inNeighbors;
    }

    const uintE* getInNeighbors () const {
        return inNeighbors;
    }

    uintE* getOutNeighbors () {
        return outNeighbors;
    }

    const uintE* getOutNeighbors () const {
        return outNeighbors;
    }
    
    uintE getInNeighbor(uintT j) const {
        return inNeighbors[j];
    }

    uintE getOutNeighbor(uintT j) const {
        return outNeighbors[j];
    }

    void setInNeighbor(uintT j, uintE ngh) {
        inNeighbors[j] = ngh;
    }

    void setOutNeighbor(uintT j, uintE ngh) {
        outNeighbors[j] = ngh;
    }

    void setInNeighbors(uintE* _i) {
        inNeighbors = _i;
    }

    void setOutNeighbors(uintE* _i) {
        outNeighbors = _i;
    }

    uintT getInDegree() const {
        return inDegree;
    }

    uintT getOutDegree() const {
        return outDegree;
    }

    void setInDegree(uintT _d) {
        inDegree = _d;
    }

    void setOutDegree(uintT _d) {
        outDegree = _d;
    }

    void flipEdges() {
        swap(inNeighbors,outNeighbors);
        swap(inDegree,outDegree); 
    }
};

#endif