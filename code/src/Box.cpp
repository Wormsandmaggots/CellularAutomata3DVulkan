#include "Box.h"

Box::Box(int _size): size(_size) {
    createCells();
}
Box::~Box() {
    for (auto& cell : cells) {
        delete cell.second;
    }
}

void Box::createCells() {
    for(int x=0; x<size; x++){
        for(int y=0; y<size; y++){
            for(int z=0; z<size; z++){
                cells[glm::vec3(x,y,z)] = new Cell(0, glm::vec3(x,y,z), glm::vec4(0,0,0,0)); //creating invisible, inactive cells
            }
        }
    }
}

void Box::enableCells(int _amount) {
    for(int i = 0; i<_amount; i++){
        float x = static_cast <float> (rand()) / static_cast <float> (size);
        float y = static_cast <float> (rand()) / static_cast <float> (size);
        float z = static_cast <float> (rand()) / static_cast <float> (size);
        getCell(glm::vec3(x,y,z))->changeState(1,glm::vec4(1,1,1,1));
    }
}

void Box::iterateCells() {

}

Cell* Box::getCell(glm::vec3 _position){
    auto it = cells.find(_position);
    if (it != cells.end()) {
        return it->second;
    }
    return nullptr;
}