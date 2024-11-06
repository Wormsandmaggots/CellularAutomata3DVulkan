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
        int x = static_cast <int> (rand()) % static_cast <int> (size);
        int y = static_cast <int> (rand()) % static_cast <int> (size);
        int z = static_cast <int> (rand()) % static_cast <int> (size);
        temp = getCell(glm::vec3(x,y,z));
        if(temp->state->id == 0){
            temp->changeState(&active);
        }
        else{
            i--;
        }
    }
}

void Box::updateCells(int _n, int _n_to_active,int _n_to_inactive) {
    if(_n == 0){ //Von Neumann
        for (auto& cell : cells) {
            int activeNeighbours = 0;
            glm::vec3 pos =  cell.second->position;

            //how many neighbours are active
            if (auto cell = getCell(glm::vec3(pos.x - 1, pos.y, pos.z))) {activeNeighbours += cell->state->id;}
            if (auto cell = getCell(glm::vec3(pos.x + 1, pos.y, pos.z))) {activeNeighbours += cell->state->id;}
            if (auto cell = getCell(glm::vec3(pos.x, pos.y - 1, pos.z))) {activeNeighbours += cell->state->id;}
            if (auto cell = getCell(glm::vec3(pos.x, pos.y + 1, pos.z))) {activeNeighbours += cell->state->id;}
            if (auto cell = getCell(glm::vec3(pos.x, pos.y, pos.z - 1))) {activeNeighbours += cell->state->id;}
            if (auto cell = getCell(glm::vec3(pos.x, pos.y, pos.z + 1))) {activeNeighbours += cell->state->id;}

            //if cell was active and condition is met
            if(getCell(pos)->state->id == 1 && activeNeighbours >=_n_to_inactive) {
                getCell(pos)->changeNextState(&inactive);
            }
            //if cell was inactive and condition is met
            else if (getCell(pos)->state->id == 0 && activeNeighbours >=_n_to_active){
                getCell(pos)->changeNextState(&active);
            }
            //conditions not met
            else{
                getCell(pos)->changeNextState(getCell(pos)->state);
            }
        }
    }
    else{ //Moore

    }

    for (auto& cell : cells) {
        cell.second->changePrevState(cell.second->state);
        cell.second->changeState(cell.second->nextState);
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