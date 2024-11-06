#include "Cell.h"

Cell::Cell(State* _state, glm::vec3 _position) :
    state(_state), previousState(_state), position(_position) {

}

void Cell::changeState(State* _newState) {
    state = _newState;
}

void Cell::changeNextState(State* _newState) {
    nextState = _newState;
}

void Cell::changePrevState(State* _newState) {
    previousState = _newState;
}