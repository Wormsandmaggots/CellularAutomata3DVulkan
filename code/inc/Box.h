#ifndef CELLULARAUTOMATA3DVULKAN_BOX_H
#define CELLULARAUTOMATA3DVULKAN_BOX_H
#include <map>
#include "Cell.h"
extern int INSTANCE_COUNT;
class Box {
public:
    std::unordered_map<glm::vec3, Cell*, Vec3Hash, Vec3Equal> cells;


    Box(int _size);
    ~Box();
    void createCells();
    void deleteCells();
    void enableCells(int _amount);
    void disableCells();
    void updateCells();
    Cell* getCell(glm::vec3 _position);
   // void update(int amount);
    int findNeighbours(glm::vec3 pos, int n);
    void start(int _n ,int _n_to_active,int _n_to_inactive, int _temporary_state_frames, bool _additional_cells, int _amount, int _size); //sasiedztwo, 0 - vonNeumann, 1 - Moore
    void stop();
private:
    int neighborhood;
    int n_to_active;
    int n_to_inactive;
    int temporary_state_frames;
    bool additional_cells;
    int size;

};


#endif //CELLULARAUTOMATA3DVULKAN_BOX_H
