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
                cells[glm::vec3(x,y,z)] = new Cell(&inactive, glm::vec3(x,y,z)); //creating inactive cells
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
        if(temp->state->id == 0){
            temp->changeState(&active);
        }
        else{
            i--;
        }
    }
}

void Box::updateCells(int _n) {
    int activeNeighbours;
    for (auto& cell : cells) {
        cell.second->changeState(&inactive);
    }
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
        cell.second->changeState(&inactive);
    }
}