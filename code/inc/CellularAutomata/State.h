#ifndef CELLULARAUTOMATA3DVULKAN_STATE_H
#define CELLULARAUTOMATA3DVULKAN_STATE_H

#include "glm/vec4.hpp"

struct State{
    int id;
    glm::vec4 color;
};
extern State inactive;
extern State active;
extern State activating;
extern State deactivating;



#endif //CELLULARAUTOMATA3DVULKAN_STATE_H
