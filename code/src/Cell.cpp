#include "Cell.h"

Cell::Cell(State* _state, glm::vec3 _position) :
    state(_state), previousState(_state), position(_position) {

}

void Cell::changeState(State* _newState) {
    state = _newState;
}

