#ifndef CELLULARAUTOMATA3DVULKAN_BOX_H
#define CELLULARAUTOMATA3DVULKAN_BOX_H
#include <map>
#include "Cell.h"

class Box {
public:
    std::unordered_map<glm::vec3, Cell*, Vec3Hash, Vec3Equal> cells;
    int size;

    Box(int _size);
    ~Box();
    void createCells();
    void enableCells(int _amount);
    void disableCells();
    void updateCells(int _n ,int _n_to_active,int _n_to_inactive); //sasiedztwo, 0 - vonNeumann, 1 - Moore
    Cell* getCell(glm::vec3 _position);
};


#endif //CELLULARAUTOMATA3DVULKAN_BOX_H
