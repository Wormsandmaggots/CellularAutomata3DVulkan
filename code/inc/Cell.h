#ifndef CELLULARAUTOMATA3DVULKAN_CELL_H
#define CELLULARAUTOMATA3DVULKAN_CELL_H


#include "glm/detail/type_vec3.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <unordered_map>
#include <memory>

class Cell {
public:
    int state; //0 - inactive, 1 - active, 2 - deactivating, 3 - activating
    glm::vec3 position;
    glm::vec4 color;


    Cell(int _state, glm::vec3 _position, glm::vec4 _color);
    void changeState(int _newState, glm::vec4 _newColor);
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
