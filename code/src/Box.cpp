#include "Box.h"

namespace Constants {
    constexpr int VON_NEUMANN = 0;
}

int INSTANCE_COUNT = 27;

Box::Box(int _size): size(_size) {
    createCells();
}
Box::~Box() {
    for (auto& cell : cells) {
        delete cell;
    }
}

void Box::createCells() {
    for(int x=0; x<size; x++){
        for(int y=0; y<size; y++){
            for(int z=0; z<size; z++){
                //cells[glm::vec3(x,y,z)] = new Cell(&inactive, glm::vec3(x,y,z)); //creating inactive cells
                cells.push_back(new Cell(&inactive, glm::vec3(x,y,z)));
            }
        }
    }
}

void Box::deleteCells() {
    for(const auto& cell : cells) { delete cell; }
    
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

void Box::updateCells() {
    if(additional_cells){
        for (const auto& cell : cells) {
            glm::vec3 pos =  cell->position;
            int state_id = cell->state->id;
            if(state_id == 2){ //activating
                if(cell->generations_in_state == temporary_state_frames){
                    cell->changeNextState(&active);
                    cell->generations_in_state = 0;
                }
                else{
                    cell->generations_in_state++;
                }
            }
            else if(state_id == 3){ //deactivating
                if(cell->generations_in_state == temporary_state_frames){
                    cell->changeNextState(&inactive);
                    cell->generations_in_state = 0;
                }
                else{
                    cell->changeNextState(getCell(pos)->state);
                    cell->generations_in_state++;
                }
            }
            else{ //active and inactive
                glm::vec3 pos =  cell->position;
                int activeNeighbours = findNeighbours(pos, neighborhood);
                //if cell was active and condition is met
                if(cell->state->id == 1 && activeNeighbours >=n_to_inactive) {
                    getCell(pos)->changeNextState(&deactivating);
                }
                    //if cell was inactive and condition is met
                else if (cell->state->id == 0 && activeNeighbours >=n_to_active){
                    getCell(pos)->changeNextState(&activating);
                }
                    //conditions not met
                else{
                    cell->changeNextState(getCell(pos)->state);
                }
            }
        }
    }
    else{
        for (auto& cell : cells) {
            glm::vec3 pos =  cell->position;
            int activeNeighbours = findNeighbours(pos, neighborhood);
            //if cell was active and condition is met
            if(cell->state->id == 1 && activeNeighbours >=n_to_inactive) {
                getCell(pos)->changeNextState(&inactive);
            }
                //if cell was inactive and condition is met
            else if (cell->state->id == 0 && activeNeighbours >=n_to_active){
                getCell(pos)->changeNextState(&active);
            }
                //conditions not met
            else{
                cell->changeNextState(getCell(pos)->state);
            }
        }
    }

    for (auto& cell : cells) {
        cell->changePrevState(cell->state);
        cell->changeState(cell->nextState);
    }
}

Cell* Box::getCell(const glm::vec3& _position) const{
    // auto it = cells.find(_position);
    // if (it != cells.end()) {
    //     return it->second;
    // }
    const int index = _position.x * size * size + _position.y * size + _position.z;

    if(index < 0 || index >= cells.size()) return nullptr;

    return cells[index];
}

void Box::disableCells() {
    for (auto& cell : cells) {
        cell->changeState(&inactive);
        cell->generations_in_state = 0;
    }
}

int Box::findNeighbours(glm::vec3 pos, int n){
    int activeNeighbours = 0;
    if(n == Constants::VON_NEUMANN){ //Von Neumann
        //how many neighbours are active
        if (const auto cell = getCell(glm::vec3(pos.x - 1, pos.y, pos.z))) {activeNeighbours += cell->state->id;}
        if (const auto cell = getCell(glm::vec3(pos.x + 1, pos.y, pos.z))) {activeNeighbours += cell->state->id;}
        if (const auto cell = getCell(glm::vec3(pos.x, pos.y - 1, pos.z))) {activeNeighbours += cell->state->id;}
        if (const auto cell = getCell(glm::vec3(pos.x, pos.y + 1, pos.z))) {activeNeighbours += cell->state->id;}
        if (const auto cell = getCell(glm::vec3(pos.x, pos.y, pos.z - 1))) {activeNeighbours += cell->state->id;}
        if (const auto cell = getCell(glm::vec3(pos.x, pos.y, pos.z + 1))) {activeNeighbours += cell->state->id;}
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

void Box::start(int _n, int _n_to_active, int _n_to_inactive, int _temporary_state_frames, bool _additional_cells, int _amount, int _size) {
    neighborhood = _n;
    n_to_active = _n_to_active;
    n_to_inactive = _n_to_inactive;
    temporary_state_frames = _temporary_state_frames;
    additional_cells = _additional_cells;
    size = _size;
    INSTANCE_COUNT = size * size * size;
    deleteCells();
    createCells();
    enableCells(_amount);
}

void Box::stop(){
    disableCells();
}