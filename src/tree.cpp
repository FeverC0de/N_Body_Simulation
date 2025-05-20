#include "tree.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

// Initialize static members
double QuadTree::s_theta = 1.5;
std::vector<Object> QuadTree::s_renegades;
double QuadTree::s_gamma = 10;
double QuadTree::s_soft = 0.1*0.1;

QuadTree::QuadTree(const std::array<float, 2> &min, const std::array<float, 2> &max, QuadTree *parent)
    : _obj(),
      _mass(0), 
      _cm({0.0f, 0.0f}), 
      _min(min),
      _max(max),         
      _center({min[0] + (max[0] - min[0]) / 2.0f, min[1] + (max[1] - min[1]) / 2.0f}),
      _bSubdivided(false),
      _parent(parent)
{
    _quadNode[0] = _quadNode[1] = _quadNode[2] = _quadNode[3] = nullptr;
}

bool QuadTree::IsRoot() const {
    return _parent == nullptr;
}

bool QuadTree::IsExternal() const {
    return  _quadNode[0] == nullptr &&
            _quadNode[1] == nullptr &&
            _quadNode[2] == nullptr &&
            _quadNode[3] == nullptr;
}

bool QuadTree::WasTooClose() const {
    return _bSubdivided;
}

const std::array<float, 2>& QuadTree::GetMin() const {
    return _min;
}

const std::array<float, 2>& QuadTree::GetMax() const {
    return _max;
}

const std::array<float, 2>& QuadTree::GetCenterOfMass() const {
    return _cm;
}

double QuadTree::GetTheta() const {
    return s_theta;
}

void QuadTree::SetTheta(double theta) {
    s_theta = theta;
} 

int QuadTree::GetNum() const {
    return _num;
}

void QuadTree::Reset(const std::array<float, 2> &min, const std::array<float, 2> &max) {
    if(!IsRoot())
        throw std::runtime_error("Only the root node may reset the tree.");

    for(int i = 0; i < 4; ++i) {
        delete _quadNode[i];
        _quadNode[i] = nullptr;
    }

    _min = min;
    _max = max;
    _center = {min[0] + (max[0] - min[0]) / 2.0f, min[1] + (max[1] - min[1]) / 2.0f};

    _num = 0;
    _mass = 0;
    _cm = {0,0};
    s_renegades.clear();
}

QuadTree::EQuadrant QuadTree::GetQuadrant(double x, double y) const {
    // Check if point is outside the boundaries
    if (x > _max[0] || y > _max[1] || x < _min[0] || y < _min[1]) {
        std::stringstream ss;
        ss << "Can't determine quadrant!\n"
           << "particle  : "
           << "(" << x << ", " << y << ")\n"
           << "quadMin   : "
           << "(" << _min[0] << ", " << _min[1] << ")\n"
           << "quadMax   : "
           << "(" << _max[0] << ", " << _max[1] << ")\n"
           << "quadCenter: "
           << "(" << _center[0] << ", " << _center[1] << ")\n";
        throw std::runtime_error(ss.str().c_str());
    }

    // Determine quadrant
    if (x <= _center[0] && y <= _center[1]) {
        return SW;
    }
    else if (x <= _center[0] && y > _center[1]) {
        return NW;
    }
    else if (x > _center[0] && y > _center[1]) {
        return NE;
    }
    else if (x > _center[0] && y <= _center[1]) {
        return SE;
    }
    
    // This should never happen if boundaries are checked first
    throw std::runtime_error("Can't determine quadrant!");
}

QuadTree *QuadTree::CreateQuadNode(EQuadrant eQuad) {
    switch (eQuad) {
        case SW:
            return new QuadTree(_min, _center, this);
        case NW:
            return new QuadTree({_min[0], _center[1]}, {_center[0], _max[1]}, this);
        case NE:
            return new QuadTree(_center, _max, this);
        case SE:
            return new QuadTree({_center[0], _min[1]}, {_max[0], _center[1]}, this);
        default:
            return nullptr;
    }
}

void QuadTree::Insert(const Object &new_obj, int level) {
    if (_num > 1) {
        // This is an internal node with multiple objects
        EQuadrant eQuad = GetQuadrant(new_obj.position[0], new_obj.position[1]);
        if (!_quadNode[eQuad]) {
            _quadNode[eQuad] = CreateQuadNode(eQuad);
        }
        _quadNode[eQuad]->Insert(new_obj, level + 1);
    }
    else if (_num == 1) {
        // This is a leaf node with exactly one object
        assert(IsExternal() || IsRoot());

        // Check if objects have the same position
        if (new_obj.position[0] == _obj.position[0] && new_obj.position[1] == _obj.position[1]) {
            s_renegades.push_back(new_obj);
        }
        else {
            // Put existing object into appropriate quadrant
            EQuadrant eQuad = GetQuadrant(_obj.position[0], _obj.position[1]);
            if (_quadNode[eQuad] == nullptr) {
                _quadNode[eQuad] = CreateQuadNode(eQuad);
            }
            _quadNode[eQuad]->Insert(_obj, level + 1);
            
            // Clear current object (now moved to child)
            _obj = Object(); 

            // Put new object into appropriate quadrant
            eQuad = GetQuadrant(new_obj.position[0], new_obj.position[1]);
            if (!_quadNode[eQuad]) {
                _quadNode[eQuad] = CreateQuadNode(eQuad);
            }
            _quadNode[eQuad]->Insert(new_obj, level + 1);
        }
    }
    else if (_num == 0) {
        // Empty node, just store the object
        _obj = new_obj;
    }

    _num++;
}

void QuadTree::ComputeMassDistribution() {
    if (_num == 1) {
        _mass = _obj.mass;
        _cm = {_obj.position[0], _obj.position[1]};
    }
    else {
        _mass = 0;
        _cm = {0, 0};

        for (int i = 0; i < 4; ++i) {
            if (_quadNode[i]) {
                _quadNode[i]->ComputeMassDistribution();
                _mass += _quadNode[i]->_mass;
                _cm[0] += _quadNode[i]->_cm[0] * _quadNode[i]->_mass;
                _cm[1] += _quadNode[i]->_cm[1] * _quadNode[i]->_mass;
            }
        }
        
        if (_mass > 0) {
            _cm[0] /= _mass;
            _cm[1] /= _mass;
        }
    }
}

std::array<float, 2> QuadTree::CalcAcc(const Object &obj1, const Object &obj2) const {
    std::array<float, 2> acc{0.0f, 0.0f};

    if (&obj1 == &obj2) {
        return acc;
    }

    float x1 = obj1.position[0];
    float y1 = obj1.position[1];
    float x2 = obj2.position[0];
    float y2 = obj2.position[1];

    float m1 = obj1.mass;
    float m2 = obj2.mass;

    double r = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + s_soft);

    if (r > 0) {
        double k = s_gamma * m2 / (r * r * r);  // Normalize force vector
        acc[0] = k * (x2 - x1);
        acc[1] = k * (y2 - y1);
    }
    else {
        acc[0] = acc[1] = 0;
    }
    
    return acc;
}

std::array<float, 2> QuadTree::CalcTreeForce(const Object &obj1) const {
    std::array<float, 2> acc = {0, 0};

    if (_num == 1) {
        // Leaf node with a single object
        return CalcAcc(obj1, _obj);
    }
    else if (_num > 1) {
        // Internal node with multiple objects
        double r = sqrt((obj1.position[0] - _cm[0]) * (obj1.position[0] - _cm[0]) +
                        (obj1.position[1] - _cm[1]) * (obj1.position[1] - _cm[1]) + s_soft);
        
        double d = _max[0] - _min[0];  // Quadrant size

        if (d / r <= s_theta) {
            // Use approximation - object is far enough away
            _bSubdivided = false;
            double k = s_gamma * _mass / (r * r * r);
            acc[0] = k * (_cm[0] - obj1.position[0]);
            acc[1] = k * (_cm[1] - obj1.position[1]);
        }    
        else {
            // Need to go deeper - object is too close
            _bSubdivided = true;
            
            for (int q = 0; q < 4; ++q) {
                if (_quadNode[q]) {
                    std::array<float, 2> buf = _quadNode[q]->CalcTreeForce(obj1);
                    acc[0] += buf[0];
                    acc[1] += buf[1];
                }
            }
        }
    }

    return acc;
}

std::array<float, 2> QuadTree::CalcForce(const Object &obj1) const {
    // Get force from quadtree structure
    std::array<float, 2> acc = CalcTreeForce(obj1);

    // Add forces from particles at identical positions (renegades)
    for (size_t i = 0; i < s_renegades.size(); ++i) {
        std::array<float, 2> buf = CalcAcc(obj1, s_renegades[i]);
        acc[0] += buf[0];
        acc[1] += buf[1];
    }

    return acc;
}

QuadTree::~QuadTree() {
    // Clean up any resources, especially the dynamically allocated quadrants
    for(int i = 0; i < 4; ++i) {
        delete _quadNode[i];
        _quadNode[i] = nullptr;
    }
}