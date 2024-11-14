#include "Box.h"
int INSTANCE_COUNT = 27;
int BOX_SIDE = 3;

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

void Box::deleteCells() {
    cells.clear();
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

void Box::updateCells(int _n, int _n_to_active,int _n_to_inactive, int _temporary_state_frames, bool _additional_cells) {
    if(_additional_cells){
        for (auto& cell : cells) {
            glm::vec3 pos =  cell.second->position;
            int state_id = cell.second->state->id;
            if(state_id == 2){ //activating
                if(cell.second->frames_hold == _temporary_state_frames){
                    cell.second->changeNextState(&active);
                    cell.second->frames_hold = 0;
                }
                else{
                    cell.second->frames_hold++;
                }
            }
            else if(state_id == 3){ //deactivating
                if(cell.second->frames_hold == _temporary_state_frames){
                    cell.second->changeNextState(&inactive);
                    cell.second->frames_hold = 0;
                }
                else{
                    cell.second->changeNextState(getCell(pos)->state);
                    cell.second->frames_hold++;
                }
            }
            else{ //active and inactive
                glm::vec3 pos =  cell.second->position;
                int activeNeighbours = findNeighbours(pos, _n);
                //if cell was active and condition is met
                if(cell.second->state->id == 1 && activeNeighbours >=_n_to_inactive) {
                    getCell(pos)->changeNextState(&deactivating);
                }
                    //if cell was inactive and condition is met
                else if (cell.second->state->id == 0 && activeNeighbours >=_n_to_active){
                    getCell(pos)->changeNextState(&activating);
                }
                    //conditions not met
                else{
                    cell.second->changeNextState(getCell(pos)->state);
                }
            }
        }
    }
    else{
        for (auto& cell : cells) {
            glm::vec3 pos =  cell.second->position;
            int activeNeighbours = findNeighbours(pos, _n);
            //if cell was active and condition is met
            if(cell.second->state->id == 1 && activeNeighbours >=_n_to_inactive) {
                getCell(pos)->changeNextState(&inactive);
            }
                //if cell was inactive and condition is met
            else if (cell.second->state->id == 0 && activeNeighbours >=_n_to_active){
                getCell(pos)->changeNextState(&active);
            }
                //conditions not met
            else{
                cell.second->changeNextState(getCell(pos)->state);
            }
        }
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

void Box::update(int amount){
    if(size != BOX_SIDE){
        size = BOX_SIDE;
        deleteCells();
        createCells();
        enableCells(amount);
    }
}

int Box::findNeighbours(glm::vec3 pos, int n){
    int activeNeighbours = 0;
    if(n == 0){ //Von Neumann
        //how many neighbours are active
        if (auto cell = getCell(glm::vec3(pos.x - 1, pos.y, pos.z))) {activeNeighbours += cell->state->id;}
        if (auto cell = getCell(glm::vec3(pos.x + 1, pos.y, pos.z))) {activeNeighbours += cell->state->id;}
        if (auto cell = getCell(glm::vec3(pos.x, pos.y - 1, pos.z))) {activeNeighbours += cell->state->id;}
        if (auto cell = getCell(glm::vec3(pos.x, pos.y + 1, pos.z))) {activeNeighbours += cell->state->id;}
        if (auto cell = getCell(glm::vec3(pos.x, pos.y, pos.z - 1))) {activeNeighbours += cell->state->id;}
        if (auto cell = getCell(glm::vec3(pos.x, pos.y, pos.z + 1))) {activeNeighbours += cell->state->id;}
    }
    else{ //Moore
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                for (int k = -1; k <= 1; ++k) {
                    // Skip the current cell itself
                    if (i == 0 && j == 0 && k == 0) continue;

                    // Calculate the neighbor position
                    glm::vec3 offset(i, j, k);
                    if (auto cell = getCell(pos + offset)) {
                        activeNeighbours += cell->state->id;
                    }
                }
            }
        }
    }
    return activeNeighbours;
}