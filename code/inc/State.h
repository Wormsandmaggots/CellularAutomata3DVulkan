#ifndef CELLULARAUTOMATA3DVULKAN_STATE_H
#define CELLULARAUTOMATA3DVULKAN_STATE_H

#include "glm/vec4.hpp"

struct State{
    int id;
    glm::vec4 color;
};
extern struct State inactive;
extern struct State active;
extern struct State activating;
extern struct State deactivating;



#endif //CELLULARAUTOMATA3DVULKAN_STATE_H
