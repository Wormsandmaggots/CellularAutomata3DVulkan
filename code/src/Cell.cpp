#include "Cell.h"

Cell::Cell(int _state, glm::vec3 _position, glm::vec4 _color) :
    state(_state), position(_position), color(_color) {

}

void Cell::changeState(int _newState, glm::vec4 _newColor) {
    state = _newState;
    color = _newColor;
}

