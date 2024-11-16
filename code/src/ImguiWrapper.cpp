//
// Created by Radek on 30.10.2024.
//

#include "ImguiWrapper.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "App.h"
#include "Types.h"
#include "VulkanUtils.h"
#include "Settings.h"
#include "Utils.h"
#include <chrono>

void ImguiWrapper::Init(App* app)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(app->GetWindow().GetWindow(), true);

    u32 queueFamily = app->findQueueFamilies(app->GetPhysicalDevice()).presentFamily.value();

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = app->GetInstance();
    init_info.PhysicalDevice = app->GetPhysicalDevice();
    init_info.Device = app->GetDevice();
    init_info.QueueFamily = queueFamily;
    init_info.Queue = app->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = app->GetDescriptorPool();
    init_info.RenderPass = app->GetRenderPass();
    init_info.Subpass = 0;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = app->GetAllocationCallback();
    ImGui_ImplVulkan_Init(&init_info);
}

void ImguiWrapper::StartFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImguiWrapper::Render()
{
    static bool show_demo_window = true;

    static int f = 3;
    static int a = 1;
    static int T = 0;
    static int n = 0;
    static ImColor active_color = glmVec4ToImColor(active.color);
    static ImColor inactive_color = glmVec4ToImColor(inactive.color);
    static ImColor activating_color = glmVec4ToImColor(activating.color);
    static ImColor deactivating_color = glmVec4ToImColor(deactivating.color);
    static int _n_to_inactive = 1;
    static int _n_to_active = 1;

    static auto start_time = std::chrono::high_resolution_clock::now();
    static auto end_time = start_time;
    static double elapsed_time = 0.0;
    static bool count_time = false;

    {
       int ctrl_max = (f*f*f)-1;
       if(ctrl_max < a)
           a = ctrl_max;
       ImGui::Begin("Cellular automata 3D");
        ImGui::SliderFloat("Zoom", &fov, 20.0f, 180.0f);
       ImGui::Text("Use this panel to adjust simulation parameters.");
       ImGui::SliderInt("Cube edge size", &f, 3, 100);
       ImGui::SliderInt("Initially active cells", &a, 1, ctrl_max);
       ImGui::Checkbox("Use advanced states", &additional_states);
       ImGui::ColorEdit4("Active color", (float*)&active_color);
       ImGui::ColorEdit4("Inactive color", (float*)&inactive_color);
       if(additional_states){
           ImGui::ColorEdit4("Activating color", (float*)&activating_color);
           ImGui::ColorEdit4("Deactivating color", (float*)&deactivating_color);
           ImGui::SliderInt("Temp. state duration (frame)", &temporary_state_frames, 1, 10);
       }
       //ImGui::SliderInt("Simulation duration (s)", &T, 1, 60);
       ImGui::SliderInt("Generation duration (frame)", &g_duration, 10,100);
        ImGui::RadioButton("Von Neumann Neighborhood", &n, 0);ImGui::SameLine();
        ImGui::RadioButton("Moore Neighborhood", &n, 1);
        if (ImGui::InputInt("Neighbours to active", &_n_to_active)) {
            if (_n_to_active < 0) {
                _n_to_active = 0;
            }
        }
        if (ImGui::InputInt("Neighbours to inactive", &_n_to_inactive)) {
            if (_n_to_inactive < 0) {
                _n_to_inactive = 0;
            }
        }

           if (ImGui::Button("Start simulation")){
               if(RUNNING){
                    RESTART = true;
                    start_time = std::chrono::high_resolution_clock::now(); // Record start time
                    elapsed_time = 0.0; // Reset elapsed time
               }
               else {
                   RUNNING = true;
                   if (!count_time) {
                       start_time = std::chrono::high_resolution_clock::now(); // Record start time
                       elapsed_time = 0.0; // Reset elapsed time
                       count_time = true;
                   }
               }
           }
           if (ImGui::Button("Stop simulation")){
               RUNNING = false;
               count_time = false;
           }
           if(count_time) {
               elapsed_time = std::chrono::duration<double>(
                       std::chrono::high_resolution_clock::now() - start_time).count();
           }
           ImGui::Text("Simulation elapsed time: %.3f seconds", elapsed_time);
           ImGui::End();

           size = f;
           prev_init_active = init_active;
           init_active = a;
           active.color = imColorToGlmVec4(active_color);
           inactive.color = imColorToGlmVec4(inactive_color);
           activating.color = imColorToGlmVec4(activating_color);
           deactivating.color = imColorToGlmVec4(deactivating_color);
           s_duration = T;
           neighborhood = n;
           n_to_active = _n_to_active;
           n_to_inactive = _n_to_inactive;
    }


    ImGui::Render();
}

void ImguiWrapper::EndFrame(VkCommandBuffer& command_buffer) const
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

void ImguiWrapper::CleanUp()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
