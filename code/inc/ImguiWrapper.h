//
// Created by Radek on 30.10.2024.
//

#ifndef IMGUIWRAPPER_H
#define IMGUIWRAPPER_H
#include <vulkan/vulkan_core.h>

#include "imgui.h"
#include "State.h"

class App;


class ImguiWrapper {
public:
    void Init(App* app);
    void StartFrame();
    void Render();
    void EndFrame(VkCommandBuffer& command_buffer) const;
    void CleanUp();

private:
    ImGuiIO io;
    int size;
    int init_active;
    bool adv_states;
    int s_duration;
    int g_duration;
};



#endif //IMGUI_H
