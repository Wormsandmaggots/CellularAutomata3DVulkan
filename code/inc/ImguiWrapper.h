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

    int prev_init_active = 0;
    int init_active = 0;
    int neighborhood;
    int n_to_inactive;
    int n_to_active;
    float fov = 45.0f;
    int s_duration;
    int g_duration = 100;
    bool additional_states = false;
    int temporary_state_frames = 2;
    int size;
private:
    ImGuiIO io;


};



#endif //IMGUI_H
