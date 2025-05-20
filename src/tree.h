#ifndef TREE_PARALLEL_H
#define TREE_PARALLEL_H

#include <array>
#include <vector>
#include "object.h"

class QuadTree {
public:
    enum EQuadrant {
        SW = 0,
        NW = 1, 
        NE = 2,
        SE = 3
    };

    // Constructor and destructor
    QuadTree(const std::array<float, 2> &min, const std::array<float, 2> &max, QuadTree *parent);
    ~QuadTree();

    // Tree properties and methods
    bool IsRoot() const;
    bool IsExternal() const;
    bool WasTooClose() const;
    const std::array<float, 2>& GetMin() const;
    const std::array<float, 2>& GetMax() const;
    const std::array<float, 2>& GetCenterOfMass() const;
    double GetTheta() const;
    void SetTheta(double theta);
    int GetNum() const;
    
    // Tree operations
    void Reset(const std::array<float, 2> &min, const std::array<float, 2> &max);
    void Insert(const Object &new_obj, int level);
    void ComputeMassDistribution();
    std::array<float, 2> CalcForce(const Object &obj1) const;

    // Public quadrant information for visualization
    QuadTree* _quadNode[4];
    QuadTree* _parent = nullptr;

private:
    // Helper methods
    EQuadrant GetQuadrant(double x, double y) const;
    QuadTree* CreateQuadNode(EQuadrant eQuad);
    std::array<float, 2> CalcTreeForce(const Object &obj1) const;
    std::array<float, 2> CalcAcc(const Object &obj1, const Object &obj2) const;

    // Node data
    Object _obj;                // Particle for leaf nodes
    double _mass;               // Total mass in this node
    std::array<float, 2> _cm;   // Center of mass
    std::array<float, 2> _min;  // Bounding box min
    std::array<float, 2> _max;  // Bounding box max
    std::array<float, 2> _center; // Center of the quadrant
    mutable bool _bSubdivided;  // Used for visualization
    int _num = 0;               // Number of particles in this node

    // Static parameters
    static double s_theta;      // Barnes-Hut opening angle
    static double s_gamma;      // Gravitational constant
    static double s_soft;       // Softening parameter
    static std::vector<Object> s_renegades; // Particles with identical positions
};

#endif // TREE_PARALLEL_H