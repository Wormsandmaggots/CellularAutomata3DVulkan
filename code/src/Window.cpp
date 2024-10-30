//
// Created by Radek on 30.10.2024.
//

#include "Window.h"

#include "App.h"
#include "Settings.h"

void Window::Init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    width = WIDTH;
    height = HEIGHT;
}

void Window::CleanUp()
{
    glfwDestroyWindow(window);
}

GLFWwindow* Window::GetWindow()
{
    return window;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    app->SetFramebufferResized(true);
}
