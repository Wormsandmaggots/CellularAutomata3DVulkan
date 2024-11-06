#ifndef CELLULARAUTOMATA3DVULKAN_CELL_H
#define CELLULARAUTOMATA3DVULKAN_CELL_H


#include "glm/detail/type_vec3.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <unordered_map>
#include <memory>
#include "State.h"

class Cell {
public:
    State* previousState;
    State* state;
    State* nextState;
    glm::vec3 position;


    Cell(State* _state, glm::vec3 _position);
    void changeState(State* _newState);
    void changeNextState(State* _newState);
    void changePrevState(State* _newState);
};

struct Vec3Hash {
    std::size_t operator()(const glm::vec3& v) const {
        std::hash<float> floatHash;
        // Combine the hashes of the x, y, and z components
        return floatHash(v.x) ^ (floatHash(v.y) << 1) ^ (floatHash(v.z) << 2);
    }
};

struct Vec3Equal {
    bool operator()(const glm::vec3& v1, const glm::vec3& v2) const {
        return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
    }
};


#endif //CELLULARAUTOMATA3DVULKAN_CELL_H
