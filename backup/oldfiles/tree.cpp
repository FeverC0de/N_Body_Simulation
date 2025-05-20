#include <iostream>
#include <sstream>
#include <C:\Users\Nick-Uni\Desktop\Sim Methods\GLFW\glfw3.h>
#include <cmath>
#include <vector>
#include <random>
#include <cassert>
#include "tree.h"
#include "threadpool.h"


double QuadTree::s_theta = 1;
std::vector<Object> QuadTree::s_renegades;
double QuadTree::s_gamma = 10;
double QuadTree::s_soft = 0.1*0.1;

QuadTree::QuadTree(const std::vector<float> &min, const std::vector<float> &max, QuadTree *parent)
    : _obj(), // Initialize _object (empty by default)
      _mass(0),  // Initialize mass to 0
      _cm({0.0f, 0.0f}), // Center of mass starts as zero
      _min(min),         // Minimum bounds
      _max(max),         // Maximum bounds
      _center({min[0] + (max[0] - min[0]) / 2.0f, min[1] + (max[1] - min[1]) / 2.0f}),
      _bSubdivided(false) // Initially not subdivided
{
    _quadNode[0] = _quadNode[1] = _quadNode[2] = _quadNode[3] = nullptr;
}

bool QuadTree::IsRoot() const{
    return _parent == nullptr;
}

bool QuadTree::IsExternal() const{
    return  _quadNode[0] == nullptr &&
            _quadNode[1] == nullptr &&
            _quadNode[2] == nullptr &&
            _quadNode[3] == nullptr;
}

bool QuadTree::WasTooClose() const{
    return _bSubdivided;
}

const std::vector<float>& QuadTree::GetMin() const{
    return _min;
}
const std::vector<float>& QuadTree::GetMax() const{
    return _max;
}
const std::vector<float>& QuadTree::GetCenterOfMass() const{
    return _cm;
}

double QuadTree::GetTheta() const{
    return s_theta;
}

void QuadTree::SetTheta(double theta){
    s_theta = theta;
} 

// int QuadTree::GetNumRenegades() const
// {
//     return s_renegades.size();
// }

int QuadTree::GetNum() const{
    return _num;
}

void QuadTree::Reset(const std::vector<float> &min, const std::vector<float> &max){

    if(!IsRoot())
        throw std::runtime_error("Only the root nood may reset the tree.");

    for(int i = 0; i < 4; ++i){
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

QuadTree::EQuadrant QuadTree::GetQuadrant(double x, double y) const{
    if(x <= _center[0] && y <= _center[1]){
        return SW;
    }
    if(x <= _center[0] && y > _center[1]){
        return NW;
    }
    if(x > _center[0] && y > _center[1]){
        return NE;
    }
    if(x > _center[0] && y <= _center[1]){
        return SE;
    }


    else if (x > _max[0] || y > _max[1] || x < _min[0] || y < _min[1]){
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
    else{
        throw std::runtime_error("Can't determine quadrant!");
    }
}

QuadTree *QuadTree::CreateQuadNode(EQuadrant eQuad){
    switch (eQuad){
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

void QuadTree::Insert(const Object &new_obj, int level){

    if (_num > 1){
        EQuadrant eQuad = GetQuadrant(new_obj.position[0], new_obj.position[1]);
        if (!_quadNode[eQuad]){
            _quadNode[eQuad] = CreateQuadNode(eQuad);
        }
        _quadNode[eQuad]->Insert(new_obj, level + 1);
    }
    else if (_num == 1){
        assert(IsExternal() || IsRoot());

        if (new_obj.position[0] == _obj.position[0] && new_obj.position[1] == _obj.position[1]){
            s_renegades.push_back(new_obj);
        }
        else{

            EQuadrant eQuad = GetQuadrant(_obj.position[0], _obj.position[1]);
            if(_quadNode[eQuad] == nullptr){
                _quadNode[eQuad] = CreateQuadNode(eQuad);
            }
            _quadNode[eQuad]->Insert(_obj, level + 1);
            _obj = Object(); // Reset _obj to an empty state

            eQuad = GetQuadrant(new_obj.position[0], new_obj.position[1]);
            if(!_quadNode[eQuad]){
                _quadNode[eQuad] = CreateQuadNode(eQuad);

            _quadNode[eQuad]->Insert(new_obj, level + 1);
            }
        }
    }
    else if (_num == 0){
        _obj = new_obj;
    }

    _num++;
        }

void QuadTree::ComputeMassDistribution() {
    if (_num == 1){
        _mass = _obj.mass;
        _cm = {_obj.position[0], _obj.position[1]};
    }
    else{
        _mass = 0;
        _cm = {0,0};

        for (int i = 0; i < 4 ;++i){
            if(_quadNode[i]){
                _quadNode[i] -> ComputeMassDistribution();
                _mass += _quadNode[i]-> _mass;
                _cm[0] += _quadNode[i]->_cm[0] * _quadNode[i]->_mass;
                _cm[1] += _quadNode[i]->_cm[1] * _quadNode[i]->_mass;

            }
        }

        _cm[0] /= _mass;
        _cm[1] /= _mass;
    }
}

std::vector<float> QuadTree::CalcAcc(const Object &obj1, const Object &obj2) const {
    std::vector<float> acc(2, 0.0f);

    if(&obj1 == &obj2){
        return acc;
    }

    float x1 = obj1.position[0];
    float y1 = obj1.position[1];
    float x2 = obj2.position[0];
    float y2 = obj2.position[1];

    float m1 = obj1.mass;
    float m2 = obj2.mass;

    double r = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + s_soft);

    if (r > 0){
        double k = s_gamma * m2 / (r*r);
        acc[0] += k * (x2-x1);
        acc[1] += k * (y2-y1);
    }
    else{
        acc[0] = acc[1] = 0;
    }
    return acc;
}

std::vector<float> QuadTree::CalcTreeForce(const Object &obj1) const{
    std::vector<float> acc = {0, 0};

    double r(0), k(0), d(0);
    if (_num == 1){
        acc = CalcAcc(obj1, _obj);
    }
    else{
        r = sqrt((obj1.position[0] - _cm[0]) * (obj1.position[0] - _cm[0]) +
                 (obj1.position[1] - _cm[1]) * (obj1.position[1] - _cm[1]));
        
        d = _max[0] - _min[0];

        if (d/r <= s_theta){
            _bSubdivided = false;
            k = s_gamma * _mass /(r * r );
            acc[0] = k * (_cm[0] - obj1.position[0]);
            acc[1] = k * (_cm[1] - obj1.position[1]);
        }    
        else {
            _bSubdivided = true;
            std::vector<float> buf = {0, 0};

            for (int q =0; q < 4; ++q){
                if (_quadNode[q]){
                    buf = _quadNode[q] -> CalcTreeForce(obj1);
                    acc[0] += buf[0];
                    acc[1] += buf[1];
                }
            }
        }
    }

    return acc;
}

std::vector<float> QuadTree::CalcForce(const Object &obj1) const{

    std::vector<float> acc = CalcTreeForce(obj1);

    if (s_renegades.size()){
        for (std::size_t i = 0; i < s_renegades.size(); ++i){
            std::vector<float> buf = CalcAcc(obj1, s_renegades[i]);  // Missing <float> template parameter
            acc[0] += buf[0];
            acc[1] += buf[1];
        }
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

// Implementation of new QuadTree methods
void QuadTree::BatchInsert(const std::vector<Object*>& bodies) {
    // Use mutex to ensure thread safety during insertion
    static std::mutex tree_mutex;
    
    for (const auto& body : bodies) {
        // For thread safety, lock only when modifying the tree structure
        std::lock_guard<std::mutex> lock(tree_mutex);
        Insert(*body, 0);
    }
}

void QuadTree::ComputeMassDistributionParallel(HighPerformanceThreadPool& pool) {
    // Base case: leaf node with no children
    if (_quadNode[0] == nullptr && _quadNode[1] == nullptr && 
        _quadNode[2] == nullptr && _quadNode[3] == nullptr) {
        ComputeMassDistribution();
        return;
    }
    
    // Process child nodes in parallel
    std::atomic<int> completed_nodes(0);
    
    // Launch tasks for each child
    for (int i = 0; i < 4; i++) {
        if (_quadNode[i] != nullptr) {
            pool.enqueue([this, i, &pool, &completed_nodes] {
                pool.increment_active();
                _quadNode[i]->ComputeMassDistributionParallel(pool);
                completed_nodes++;
                pool.decrement_active();
            });
        } else {
            completed_nodes++;
        }
    }
    
    // Wait for all child nodes to complete their computation
    while (completed_nodes < 4) {
        std::this_thread::yield();
    }
    
    // Compute mass distribution for this node
    float total_mass = 0.0f;
    std::vector<float> com_pos = {0.0f, 0.0f};
    
    // Aggregate child results
    for (int i = 0; i < 4; i++) {
        if (_quadNode[i] != nullptr) {
            float child_mass = _quadNode[i]->getMass();
            const std::vector<float>& child_com = _quadNode[i]->getCom();
            
            total_mass += child_mass;
            com_pos[0] += child_mass * child_com[0];
            com_pos[1] += child_mass * child_com[1];
        }
    }
    
    // Add this node's body if it exists
    if (_num == 1) {
        float body_mass = _obj.mass;
        const std::vector<float>& body_pos = _obj.position;
        
        total_mass += body_mass;
        com_pos[0] += body_mass * body_pos[0];
        com_pos[1] += body_mass * body_pos[1];
    }
    
    // Calculate final center of mass
    if (total_mass > 0) {
        com_pos[0] /= total_mass;
        com_pos[1] /= total_mass;
        _mass = total_mass;
        _cm = com_pos;
    }
}
