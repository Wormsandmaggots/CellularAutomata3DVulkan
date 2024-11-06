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
                cells[glm::vec3(x,y,z)] = new Cell(0, glm::vec3(x,y,z), glm::vec4(1,0,0,0)); //creating invisible, inactive cells
            }
        }
    }
}

void Box::enableCells(int _amount) {
    Cell* temp;
    disableCells();
    for(int i = 0; i<_amount; i++){
        float x = static_cast <float> (rand()) / static_cast <float> (size);
        float y = static_cast <float> (rand()) / static_cast <float> (size);
        float z = static_cast <float> (rand()) / static_cast <float> (size);
        temp = getCell(glm::vec3(x,y,z));
        if(temp->state == 0){
            temp->changeState(1,glm::vec4(1,1,1,1));
        }
        else{
            i--;
        }
    }
}

void Box::updateCells(int _n) {
    int activeNeighbours;
    if(_n == 0){
        
    }
    else{

    }
}

Cell* Box::getCell(glm::vec3 _position){
    auto it = cells.find(_position);
    if (it != cells.end()) {
        return it->second;
    }
    return nullptr;
}

void Box::disableCells() {
    for (auto& cell : cells) {
        cell.second->changeState(0,glm::vec4(0,0,0,0));
    }
}