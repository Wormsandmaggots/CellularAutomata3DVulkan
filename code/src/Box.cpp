#include "Box.h"

#include "Types.h"

#include <omp.h>

namespace Constants {
    constexpr int VON_NEUMANN = 0;
    constexpr int INACTIVE = 0;
    constexpr int ACTIVE = 1;
    constexpr int ACTIVATING = 2;
    constexpr int DEACTIVATING = 3;
}

int INSTANCE_COUNT = 27;

Box::Box(int _size): size(_size) {
    createCells();
}
Box::~Box() {

}

void Box::createCells() {
    for(int x=0; x<size; x++){
        for(int y=0; y<size; y++){
            for(int z=0; z<size; z++){
                cells.push_back(new Cell(&inactive, glm::vec3(x,y,z)));
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

void Box::updateCells() {
    if(additional_cells){
        for (const auto& cell : cells) {
            const glm::vec3& pos =  cell->getPosition();
            const int state_id = cell->state->id;
            if(state_id == Constants::ACTIVATING){ //activating
                if(cell->generations_in_state == temporary_state_frames){
                    cellsToChange.emplace_back(cell, &active);
                    cell->generations_in_state = 0;
                }
                else{
                    cell->generations_in_state++;
                }
            }
            else if(state_id == Constants::DEACTIVATING){ //deactivating
                if(cell->generations_in_state == temporary_state_frames){
                    cellsToChange.emplace_back(cell, &inactive);
                    cell->generations_in_state = 0;
                }
                else{
                    //cellsToChange.emplace_back(cell, getCell(pos)->state);
                    cell->generations_in_state++;
                }
            }
            else{ //active and inactive
                const glm::vec3& pos1 =  cell->getPosition();
                const int activeNeighbours = findNeighbours(pos1, neighborhood);
                //if cell was active and condition is met
                if(cell->state->id == Constants::ACTIVE && activeNeighbours >=n_to_inactive) {
                    cellsToChange.emplace_back(cell, &deactivating);
                }
                else if (cell->state->id == Constants::INACTIVE && activeNeighbours >=n_to_active){
                    cellsToChange.emplace_back(cell, &activating);
                }
                    //conditions not met
                // else{
                //     //cellsToChange.emplace_back(cell, cell->state);
                // }
            }
        }
    }
    else{

        for (const auto& cell : cells) {
            const glm::vec3& pos =  cell->getPosition();
            const int activeNeighbours = findNeighbours(pos, neighborhood);
            //if cell was active and condition is met
            if(cell->state->id == Constants::ACTIVE && activeNeighbours >=n_to_inactive) {
                cellsToChange.emplace_back(cell, &inactive);
            }
                //if cell was inactive and condition is met
            else if (cell->state->id == Constants::INACTIVE && activeNeighbours >=n_to_active){
                cellsToChange.emplace_back(cell, &active);
            }
                //conditions not met
            // else{
            //     //cellsToChange.emplace_back(cell, cell->state);
            // }
        }
    }

    Cell* c;
    State* s;

    for(auto cellStatePair : cellsToChange){

        c = cellStatePair.first;
        s = cellStatePair.second;

        c->changeNextState(s);
        c->changePrevState(c->state);
        c->changeState(c->nextState);
    }

}

Cell* Box::getCell(const glm::ivec3& _position) const{
    if(_position.x >= size || _position.x < 0 ||
        _position.y >= size || _position.y < 0 ||
        _position.z >= size || _position.z < 0)
        return null;

    const int index = _position.x * size * size + _position.y * size + _position.z;

    if(index < 0 || index >= cells.size())
        return null;

    return cells[index];
}

void Box::disableCells() {
    for (auto& cell : cells) {
        cell->changeState(&inactive);
        cell->generations_in_state = 0;
    }
}

int Box::findNeighbours(const glm::ivec3& pos, int n) const {
    int activeNeighbours = 0;
    if(n == Constants::VON_NEUMANN){ //Von Neumann
        //how many neighbours are active
        auto cell = getCell(glm::vec3(pos.x - 1, pos.y, pos.z));
        if(cell)
            activeNeighbours += cell->state->id;

        cell = getCell(glm::vec3(pos.x + 1, pos.y, pos.z));
        if(cell)
            activeNeighbours += cell->state->id;

        cell = getCell(glm::vec3(pos.x, pos.y - 1, pos.z));
        if(cell)
            activeNeighbours += cell->state->id;

        cell = getCell(glm::vec3(pos.x, pos.y + 1, pos.z));
        if(cell)
            activeNeighbours += cell->state->id;

        cell = getCell(glm::vec3(pos.x, pos.y, pos.z - 1));
        if(cell)
            activeNeighbours += cell->state->id;

        cell = getCell(glm::vec3(pos.x, pos.y, pos.z + 1));
        if(cell)
            activeNeighbours += cell->state->id;
    }
    else{ //Moore
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                for (int k = -1; k <= 1; ++k) {
                    // Skip the current cell itself
                    if (i == 0 && j == 0 && k == 0) continue;

                    // Calculate the neighbor position
                    glm::ivec3 offset(i, j, k);
                    if (auto cell = getCell(glm::ivec3(pos) + offset)) {
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
    omp_set_num_threads(10);
    INSTANCE_COUNT = size * size * size;
    deleteCells();
    createCells();
    enableCells(_amount);
}

void Box::stop(){
    disableCells();
}