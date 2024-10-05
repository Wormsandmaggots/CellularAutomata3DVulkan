#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "Logger.h"

int initVulkan();
void cleanup();
void drawFrame();

VkInstance instance;
VkDevice device;
VkSwapchainKHR swapChain;
VkRenderPass renderPass;
VkPipeline pipeline;
VkCommandBuffer commandBuffer;
VkFramebuffer* framebuffers;
VkQueue graphicsQueue;

int main() {
    // Inicjalizacja GLFW (opcjonalnie, jeśli chcesz korzystać z okien)
    if (!glfwInit()) {
        ERRORLOG("Failed to initialized GLFW");
        return -1;
    }
    INFOLOG("GLFW initialized succesfully");

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    if (!window) {
        ERRORLOG("Failed to create window");
        glfwTerminate();
        return -1;
    }
    INFOLOG("Window initialized succesfully");

    if(initVulkan())
    {
        ERRORLOG("Failed to initialized Vulkan");
        glfwTerminate();
        glfwDestroyWindow(window);
        return -1;
    }

    INFOLOG("Vulkan initialized succesfully");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        drawFrame();

        glfwSwapBuffers(window);
    }

    cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int initVulkan() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        return 1;
    }

    return 0;
}

void cleanup() {
    vkDestroyInstance(instance, nullptr);
}

void drawFrame() {
}