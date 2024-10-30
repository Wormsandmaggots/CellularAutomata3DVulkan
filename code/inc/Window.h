//
// Created by Radek on 30.10.2024.
//

#ifndef WINDOW_H
#define WINDOW_H
#include <glm/fwd.hpp>

#include "Types.h"
#include "GLFW/glfw3.h"


class Window {
public:
    void Init();
    void CleanUp();

    GLFWwindow* GetWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
    GLFWwindow* window;

    u32 width, height;
};



#endif //WINDOW_H
