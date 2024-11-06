#include "State.h"
struct State inactive = {0, glm::vec4(1, 1, 0, 0)};
struct State active = {1, glm::vec4(0, 1, 0, 0)};
struct State activating = {2, glm::vec4(0, 0, 0, 0)};
struct State deactivating = {3, glm::vec4(0, 0, 0, 0)};