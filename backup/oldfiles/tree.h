#ifndef BH_TREE_H
#define BH_TREE_H

#include <vector>
#include "object.h"
#include "threadpool.h"

class HighPerformanceThreadPool;

// QuadTree class representing the Barnes-Hut tree structure
class QuadTree {
public:
    // Enumeration for the quadrants
    enum EQuadrant {
        NE = 0, // Northeast
        NW,     // Northwest
        SW,     // Southwest
        SE,     // Southeast
        NONE    // No quadrant
    };

    // Constructor and Destructor
    QuadTree(const std::vector<float> &min, const std::vector<float> &max, QuadTree *parent = nullptr);
    ~QuadTree();

    // Reset the QuadTree bounds
    void Reset(const std::vector<float> &min, const std::vector<float> &max);

    // Utility methods to check node properties
    bool IsRoot() const;
    bool IsExternal() const; // Leaf node check
    bool WasTooClose() const;

    // Accessor methods
    int GetNum() const;
    const std::vector<float> &GetCenterOfMass() const;
    const std::vector<float> &GetMin() const;
    const std::vector<float> &GetMax() const;

    // Theta threshold for approximation
    double GetTheta() const;
    void SetTheta(double theta);

    // Insert an object into the tree
    void Insert(const Object &newObject, int level);

    // Methods for handling quadrants
    EQuadrant GetQuadrant(double x, double y) const;
    QuadTree *CreateQuadNode(EQuadrant eQuad);

    // Compute the mass distribution
    void ComputeMassDistribution();

    QuadTree *_quadNode[4];
    std::vector<float> CalcForce(const Object &obj) const;
    std::vector<float> CalcAcc(const Object &obj1, const Object &obj2) const;
std::vector<float> CalcTreeForce(const Object &obj) const;
double getMass() const { return _mass; };
const std::vector<float>& getCom() const { return _cm; };
void BatchInsert(const std::vector<Object*>& bodies);
void ComputeMassDistributionParallel(HighPerformanceThreadPool& pool);
private:
    // Force and acceleration calculations


    // Member variables
    Object _obj;                     // Object stored in this node (if a leaf)
    double _mass;                    // Total mass of all objects in the node
    std::vector<float> _cm;          // Center of mass of the objects in the node
    std::vector<float> _min;         // Minimum bounds of the node (top-left corner)
    std::vector<float> _max;         // Maximum bounds of the node (bottom-right corner)
    std::vector<float> _center;      // Geometric center of the node
    QuadTree *_parent;               // Pointer to the parent node
    int _num = 0;                         // Number of objects in the node
    mutable bool _bSubdivided;               // Whether the node is subdivided into quadrants

    // Static members for global settings
    static double s_theta;           // Threshold for Barnes-Hut approximation
    static std::vector<Object> s_renegades; // Objects that couldn't be placed in the tree
    static double s_gamma;           // Damping or scaling factor
    static double s_soft;            // Softening factor to avoid singularities
};

#endif // BH_TREE_H