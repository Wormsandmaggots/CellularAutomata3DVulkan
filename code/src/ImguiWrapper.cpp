//
// Created by Radek on 30.10.2024.
//

#include "ImguiWrapper.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "App.h"
#include "Types.h"
#include "VulkanUtils.h"

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
    static bool show_another_window = false;

    static int f = 3;
    static int a = 1;
    static int T = 0;
    static int t = 0;
    static bool as = false;
    static ImColor active_color;
    static ImColor inactive_color;
    static ImColor activating_color;
    static ImColor deactivating_color;

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
           int ctrl_max = (f*f*f)-1;
           if(ctrl_max < a)
               a = ctrl_max;
           ImGui::Begin("Cellular automata 3D");                          // Create a window called "Hello, world!" and append into it.
           ImGui::Text("Use this panel to adjust simulation parameters.");  // Display some text (you can use a format strings too)
           ImGui::SliderInt("Cube edge size", &f, 3, 100);
           ImGui::SliderInt("Initially active cells", &a, 1, ctrl_max);
           ImGui::Checkbox("Use advanced states", &as);
           ImGui::ColorEdit3("Active color", (float*)&active_color);
           ImGui::ColorEdit4("Inactive color", (float*)&inactive_color);
           if(as){
               ImGui::ColorEdit3("Activating color", (float*)&activating_color);
               ImGui::ColorEdit3("Deactivating color", (float*)&deactivating_color);
           }
           ImGui::SliderInt("Simulation duration (s)", &T, 1, 60);
           ImGui::SliderInt("Generation duration (frame)", &t, 1,6);
           if (ImGui::Button("Start simulation")){
               //starting the simulation
           }
           if (ImGui::Button("Stop simulation")){
               //stopping the simulation
           }
           ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
           ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
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
