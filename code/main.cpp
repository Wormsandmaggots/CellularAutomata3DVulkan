// Dear ImGui: standalone example application for Glfw + Vulkan

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Data
static VkAllocationCallbacks*   g_Allocator = nullptr;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;
static VkPipelineLayout         g_PipelineLayout = VK_NULL_HANDLE;
static VkDescriptorSetLayout    g_DescriptorSetLayout = VK_NULL_HANDLE;
static VkPipeline               g_Pipeline = VK_NULL_HANDLE;
static VkSurfaceKHR             g_Surface = VK_NULL_HANDLE;
static VkRenderPass             g_RenderPass = VK_NULL_HANDLE;
static VkDescriptorSet          g_DescriptorSet = VK_NULL_HANDLE;
static VkFramebuffer            g_Framebuffer = VK_NULL_HANDLE;
static VkCommandPool            g_CommandPool = VK_NULL_HANDLE;
static VkCommandBuffer          g_CommandBuffer = VK_NULL_HANDLE;
static VkSemaphore              g_WaitSemaphore = VK_NULL_HANDLE;
static VkSemaphore              g_RenderCompleteSemaphore = VK_NULL_HANDLE;
static VkFence                  g_Fence = VK_NULL_HANDLE;
static VkSwapchainKHR           g_Swapchain = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;
static int                      g_PresentQueueFamilyIndex = -1;

static int width = 1280;
static int height = 720;

struct Vertex
{
    float Position[3];
    float tex[2];
};

struct UniformBufferObject {
    glm::mat4 model;   // Macierz modelu
    glm::mat4 view;    // Macierz widoku
    glm::mat4 proj;    // Macierz projekcji
};

glm::vec3 pos = {0,0,0};

Vertex vertices[] = {
    // Positions           // Texture Coords
    // Back face
    {-0.5f, -0.5f, -0.5f,   0.0f, 0.0f},  // Back bottom left
     {0.5f, -0.5f, -0.5f,   1.0f, 0.0f},  // Back bottom right
     {0.5f,  0.5f, -0.5f,   1.0f, 1.0f},  // Back top right
     {0.5f,  0.5f, -0.5f,   1.0f, 1.0f},  // Back top right
    {-0.5f,  0.5f, -0.5f,   0.0f, 1.0f},  // Back top left
    {-0.5f, -0.5f, -0.5f,   0.0f, 0.0f},  // Back bottom left

    // Front face
    {-0.5f, -0.5f,  0.5f,   0.0f, 0.0f},  // Front bottom left
     {0.5f, -0.5f,  0.5f,   1.0f, 0.0f},  // Front bottom right
     {0.5f,  0.5f,  0.5f,   1.0f, 1.0f},  // Front top right
     {0.5f,  0.5f,  0.5f,   1.0f, 1.0f},  // Front top right
    {-0.5f,  0.5f,  0.5f,   0.0f, 1.0f},  // Front top left
    {-0.5f, -0.5f,  0.5f,   0.0f, 0.0f},  // Front bottom left

    // Left face
    {-0.5f,  0.5f,  0.5f,   1.0f, 0.0f},  // Top left front
    {-0.5f,  0.5f, -0.5f,   1.0f, 1.0f},  // Top left back
    {-0.5f, -0.5f, -0.5f,   0.0f, 1.0f},  // Bottom left back
    {-0.5f, -0.5f, -0.5f,   0.0f, 1.0f},  // Bottom left back
    {-0.5f, -0.5f,  0.5f,   0.0f, 0.0f},  // Bottom left front
    {-0.5f,  0.5f,  0.5f,   1.0f, 0.0f},  // Top left front

    // Right face
     {0.5f,  0.5f,  0.5f,   1.0f, 0.0f},  // Top right front
     {0.5f,  0.5f, -0.5f,   1.0f, 1.0f},  // Top right back
     {0.5f, -0.5f, -0.5f,   0.0f, 1.0f},  // Bottom right back
     {0.5f, -0.5f, -0.5f,   0.0f, 1.0f},  // Bottom right back
     {0.5f, -0.5f,  0.5f,   0.0f, 0.0f},  // Bottom right front
     {0.5f,  0.5f,  0.5f,   1.0f, 0.0f},  // Top right front

    // Bottom face
    {-0.5f, -0.5f, -0.5f,   0.0f, 1.0f},  // Bottom left back
    {0.5f, -0.5f, -0.5f,   1.0f, 1.0f},  // Bottom right back
    {0.5f, -0.5f,  0.5f,   1.0f, 0.0f},  // Bottom right front
     {0.5f, -0.5f,  0.5f,   1.0f, 0.0f},  // Bottom right front
    {-0.5f, -0.5f,  0.5f,   0.0f, 0.0f},  // Bottom left front
    {-0.5f, -0.5f, -0.5f,   0.0f, 1.0f},  // Bottom left back

    // Top face
    {-0.5f,  0.5f, -0.5f,   0.0f, 1.0f},  // Top left back
     {0.5f,  0.5f, -0.5f,   1.0f, 1.0f},  // Top right back
     {0.5f,  0.5f,  0.5f,   1.0f, 0.0f},  // Top right front
     {0.5f,  0.5f,  0.5f,   1.0f, 0.0f},  // Top right front
    {-0.5f,  0.5f,  0.5f,   0.0f, 0.0f},  // Top left front
    {-0.5f,  0.5f, -0.5f,   0.0f, 1.0f}   // Top left back
};

uint16_t indices[] = {
    // Back face
    0, 1, 2,
    2, 3, 0,

    // Front face
    4, 5, 6,
    6, 7, 4,

    // Left face
    8, 9, 10,
    10, 11, 8,

    // Right face
    12, 13, 14,
    14, 15, 12,

    // Bottom face
    16, 17, 18,
    18, 19, 16,

    // Top face
    20, 21, 22,
    22, 23, 20
};

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(g_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(g_Device, buffer, bufferMemory, 0);
}

void drawCube(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, VkBuffer vertexBuffer, VkBuffer indexBuffer, VkDescriptorSet descriptorSet, uint32_t indexCount) {
    // Bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Bind the vertex buffer
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

    // Bind the index buffer
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // Bind the descriptor set (for uniform buffer, etc.)
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    // Issue the draw call
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

static VkPhysicalDevice SetupVulkan_SelectPhysicalDevice()
{
    uint32_t gpu_count;
    VkResult err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, nullptr);
    check_vk_result(err);
    IM_ASSERT(gpu_count > 0);

    ImVector<VkPhysicalDevice> gpus;
    gpus.resize(gpu_count);
    err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus.Data);
    check_vk_result(err);

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
    // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
    // dedicated GPUs) is out of scope of this sample.
    for (VkPhysicalDevice& device : gpus)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return device;
    }

    // Use first GPU (Integrated) is a Discrete one is not available.
    if (gpu_count > 0)
        return gpus[0];
    return VK_NULL_HANDLE;
}

static void SetupVulkan(ImVector<const char*> instance_extensions)
{
    VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkInitialize();
#endif

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkLoadInstance(g_Instance);
#endif

        // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto f_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    g_PhysicalDevice = SetupVulkan_SelectPhysicalDevice();

    // Select graphics queue family
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, nullptr);
        VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
        for (uint32_t i = 0; i < count; i++)
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                g_QueueFamily = i;
                break;
            }
        free(queues);
        IM_ASSERT(g_QueueFamily != (uint32_t)-1);
    }

    // Create Logical Device (with 1 queue)
    {
        ImVector<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
        check_vk_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    // The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
    // If you wish to load e.g. additional textures you may need to alter pools sizes.
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10}
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 10;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

static void CleanupVulkan()
{
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
    f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}

VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(g_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

static void CreatePipeline()
{
    VkShaderModule vertShaderModule = createShaderModule(readFile("vertex.spv"));
    VkShaderModule fragShaderModule = createShaderModule(readFile("fragment.spv"));

    // Informacje o shaderach
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";  // Nazwa funkcji wejściowej w shaderze

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Opis wierzchołków
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex); // Rozmiar pojedynczego wierzchołka
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[2];
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Pozycja (3 floaty)
    attributeDescriptions[0].offset = offsetof(Vertex, Position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT; // Współrzędne tekstury (2 floaty)
    attributeDescriptions[1].offset = offsetof(Vertex, tex);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; // Ilość viewportów
    viewportState.pViewports = &viewport; // Referencja do viewportu
    viewportState.scissorCount = 1; // Ilość scissorów
    viewportState.pScissors = &scissor; // Referencja do scissorów

    // Ustawienia rasteryzacji
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // Ustawienia multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Ustawienia mieszania kolorów
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Opis layoutu deskryptorów
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0; // Numer punktu wiązania
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Typ deskryptora
    uboLayoutBinding.descriptorCount = 1; // Ilość elementów
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Shader używający deskryptora

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1; // Liczba deskryptorów
    layoutInfo.pBindings = &uboLayoutBinding; // Referencja do deskryptora

    // Tworzenie deskryptora
    if (vkCreateDescriptorSetLayout(g_Device, &layoutInfo, nullptr, &g_DescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // Tworzenie layoutu pipeline
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;  // liczba layoutów deskryptorów
    pipelineLayoutInfo.pSetLayouts = &g_DescriptorSetLayout;

    if (vkCreatePipelineLayout(g_Device, &pipelineLayoutInfo, nullptr, &g_PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Ustawienia assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Trójkąty
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Pobranie dostępnych formatów swap chain
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_PhysicalDevice, g_Surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_PhysicalDevice, g_Surface, &formatCount, surfaceFormats.data());

    VkFormat swapChainImageFormat;
    if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        swapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM; // Użyj formatu 32-bitowego
    } else {
        swapChainImageFormat = surfaceFormats[0].format; // Użyj pierwszego dostępnego formatu
    }

    // Format bufora głębokości
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    // Tworzenie attachmentów
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM; // Format załącznika kolorowego
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT; // Format załącznika głębi
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Tworzenie subpassu
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Indeks attachmentu koloru
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1; // Indeks attachmentu głębokości
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkClearValue clearValues[2]; // Dla dwóch attachmentów: kolorowego i głębokości
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f}; // Kolor czyszczenia (RGB + Alpha)
    clearValues[1].depthStencil = {1.0f, 0}; // Wartość głębokości + wartość szablonu (stencil)

    // Tworzenie render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2; // Kolor + głębokość
    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
    renderPassInfo.pAttachments = attachments; // Referencja do attachmentów
    renderPassInfo.subpassCount = 1; // Liczba subpassów
    renderPassInfo.pSubpasses = &subpass; // Referencja do subpassu

    if (vkCreateRenderPass(g_Device, &renderPassInfo, nullptr, &g_RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE; // Włącz test głębokości
    depthStencil.depthWriteEnable = VK_TRUE; // Włącz zapis głębokości
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // Porównanie głębokości
    depthStencil.depthBoundsTestEnable = VK_FALSE; // Nie włączaj ograniczeń głębokości
    depthStencil.stencilTestEnable = VK_FALSE; // Nie włączaj testu stencila
    depthStencil.front = {}; // Domyślne wartości dla frontu
    depthStencil.back = {}; // Domyślne wartości dla tyłu
    depthStencil.minDepthBounds = 0.0f; // Minimalna głębokość
    depthStencil.maxDepthBounds = 1.0f; // Maksymalna głębokość

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = g_DescriptorPool; // Upewnij się, że używasz poprawnego poola
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &g_DescriptorSetLayout; // Upewnij się, że jest to poprawny layout

    if (vkAllocateDescriptorSets(g_Device, &allocInfo, &g_DescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    // VkImageView attachmentImageView;
    //
    // VkImage image;
    //
    // VkImageViewCreateInfo viewInfo{};
    // viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    // viewInfo.image = ; // The image to create the view for
    // viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    // viewInfo.format = swapChainImageFormat; // Use the format of the swap chain image
    // viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // No swizzling
    // viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    // viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    // viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //
    // viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Color aspect
    // viewInfo.subresourceRange.baseMipLevel = 0; // Mip level
    // viewInfo.subresourceRange.levelCount = 1; // Number of mip levels
    // viewInfo.subresourceRange.baseArrayLayer = 0; // Base array layer
    // viewInfo.subresourceRange.layerCount = 1; // Number of layers
    //
    // if (vkCreateImageView(g_Device, &viewInfo, nullptr, &attachmentImageView) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to create image view!");
    // }
    //
    // VkFramebufferCreateInfo framebufferInfo{};
    // framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    // framebufferInfo.renderPass = g_RenderPass; // Your render pass
    // framebufferInfo.attachmentCount = 1; // Number of attachments
    // framebufferInfo.pAttachments = &attachmentImageView; // Your color image view
    // framebufferInfo.width = width; // Width of the framebuffer
    // framebufferInfo.height = height; // Height of the framebuffer
    // framebufferInfo.layers = 1; // Number of layers (usually 1)
    //
    // if (vkCreateFramebuffer(g_Device, &framebufferInfo, nullptr, &g_Framebuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to create framebuffer!");
    // }

    // Tworzenie pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;  // Vertex + Fragment shader
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = g_PipelineLayout;
    pipelineInfo.renderPass = g_RenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.pDepthStencilState = &depthStencil; // Dodaj ten wiersz

    if (vkCreateGraphicsPipelines(g_Device, g_PipelineCache, 1, &pipelineInfo, nullptr, &g_Pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    VkImage attachmentImage;
    VkImage attachmentImage2;
    VkDeviceMemory attachmentImageMemory;
    VkDeviceMemory attachmentImageMemory2;

    // Tworzenie obrazu
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM; // lub inny format
    imageInfo.extent.width = width; // Szerokość
    imageInfo.extent.height = height; // Wysokość
    imageInfo.extent.depth = 1; // Głębokość
    imageInfo.mipLevels = 1; // Liczba mipmap
    imageInfo.arrayLayers = 1; // Warstwy
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Liczba próbek
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Tiling
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Użycie obrazu
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Tryb współdzielenia

    if (vkCreateImage(g_Device, &imageInfo, nullptr, &attachmentImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;;
    imageInfo.format = VK_FORMAT_D32_SFLOAT;

    if (vkCreateImage(g_Device, &imageInfo, nullptr, &attachmentImage2) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // Przydzielenie pamięci dla obrazu
    VkMemoryRequirements mem;
    VkMemoryRequirements mem2;
    vkGetImageMemoryRequirements(g_Device, attachmentImage, &mem);
    vkGetImageMemoryRequirements(g_Device, attachmentImage, &mem2);

    VkMemoryAllocateInfo allInfo = {};
    allInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allInfo.allocationSize = mem.size;
    allInfo.memoryTypeIndex = FindMemoryType(mem.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(g_Device, &allInfo, nullptr, &attachmentImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    allInfo.allocationSize = mem2.size;
    allInfo.memoryTypeIndex = FindMemoryType(mem2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(g_Device, &allInfo, nullptr, &attachmentImageMemory2) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    // Przypisanie pamięci do obrazu
    vkBindImageMemory(g_Device, attachmentImage, attachmentImageMemory, 0);
    vkBindImageMemory(g_Device, attachmentImage2, attachmentImageMemory2, 0);

    VkImageView attachmentImageView;
    VkImageView attachmentImageView2;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = attachmentImage; // Użyj obrazu, który utworzyłeś wcześniej
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;  // Musi odpowiadać formatowi obrazu
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Użycie jako attachment kolorowy
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1; // Użyj wszystkich mipmap
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1; // Użyj wszystkich warstw

    if (vkCreateImageView(g_Device, &viewInfo, nullptr, &attachmentImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Użycie jako attachment głębokości
    viewInfo.format = VK_FORMAT_D32_SFLOAT;
    viewInfo.image = attachmentImage2;

    if (vkCreateImageView(g_Device, &viewInfo, nullptr, &attachmentImageView2) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    std::vector<VkImageView> attachments2;

    // Dodaj widok obrazu do tablicy
    attachments2.push_back(attachmentImageView); // lub użyj bezpośrednio
    attachments2.push_back(attachmentImageView2);

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = g_RenderPass; // Musi być poprawnie zainicjalizowane
    framebufferInfo.attachmentCount = attachments2.size(); // Liczba attachmentów
    framebufferInfo.pAttachments = attachments2.data(); // Twoja tablica widoków obrazów
    framebufferInfo.width = width; // Szerokość
    framebufferInfo.height = height; // Wysokość
    framebufferInfo.layers = 1; // Warstwy

    if (vkCreateFramebuffer(g_Device, &framebufferInfo, nullptr, &g_Framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }



    // Zwalnianie shaderów po ich użyciu
    vkDestroyShaderModule(g_Device, fragShaderModule, nullptr);
    vkDestroyShaderModule(g_Device, vertShaderModule, nullptr);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }

    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }

    uint32_t index;

    err = vkAcquireNextImageKHR(g_Device, g_Swapchain, UINT64_MAX, g_WaitSemaphore, VK_NULL_HANDLE, &index);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    {
        err = vkWaitForFences(g_Device, 1, &g_Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &g_Fence);
        check_vk_result(err);
    }

    {
        err = vkResetCommandPool(g_Device, g_CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(g_CommandBuffer, &info);
        check_vk_result(err);
    }

    VkClearValue clearValues[2]; // Dla dwóch attachmentów: kolorowego i głębokości
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f}; // Kolor czyszczenia (RGB + Alpha)
    clearValues[1].depthStencil = {1.0f, 0}; // Wartość głębokości + wartość szablonu (stencil)

    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = g_RenderPass;
        info.framebuffer = g_Framebuffer;
        info.renderArea.extent.width = width;
        info.renderArea.extent.height = height;
        info.clearValueCount = 2;
        info.pClearValues = clearValues;
        vkCmdBeginRenderPass(g_CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // --- Renderowanie sześcianu ---

    // Zakładam, że wcześniej masz zdefiniowane zasoby, takie jak vertexBuffer, indexBuffer, pipeline, i inne.
    VkDeviceSize vertexBufferSize = sizeof(vertices);

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

    void* data;
    vkMapMemory(g_Device, vertexBufferMemory, 0, vertexBufferSize, 0, &data);
    memcpy(data, vertices, (size_t) vertexBufferSize);
    vkUnmapMemory(g_Device, vertexBufferMemory);

    VkDeviceSize indexBufferSize = sizeof(indices);

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);

    void* data2;
    vkMapMemory(g_Device, indexBufferMemory, 0, indexBufferSize, 0, &data2);
    memcpy(data2, indices, (size_t) indexBufferSize);
    vkUnmapMemory(g_Device, indexBufferMemory);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindPipeline(g_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Pipeline);
    vkCmdBindVertexBuffers(g_CommandBuffer, 0, 1, &vertexBuffer, offsets);
    vkCmdBindIndexBuffer(g_CommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    //VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = g_DescriptorPool; // Utwórz pulę descriptorów wcześniej
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &g_DescriptorSetLayout; // Layout

    // if (vkAllocateDescriptorSets(g_Device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to allocate descriptor set!");
    // }

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

    size_t bufferSize = sizeof(UniformBufferObject);

    // Tworzenie bufora
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_Device, &bufferInfo, nullptr, &uniformBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create uniform buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_Device, uniformBuffer, &memRequirements);

    VkMemoryAllocateInfo allinfo{};
    allinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allinfo.allocationSize = memRequirements.size;
    allinfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(g_Device, &allinfo, nullptr, &uniformBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate uniform buffer memory!");
    }

    // Wiązanie pamięci z buforem
    vkBindBufferMemory(g_Device, uniformBuffer, uniformBufferMemory, 0);

    VkDescriptorBufferInfo bufferDescInfo{};
    bufferDescInfo.buffer = uniformBuffer; // Twój uniform buffer
    bufferDescInfo.offset = 0;
    bufferDescInfo.range = sizeof(UniformBufferObject); // Rozmiar danych

    UniformBufferObject ubo = {};
    ubo.model = glm::mat4(1.0f); // Macierz identyczności
    ubo.model = glm::translate(ubo.model, pos);
    ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1; // Odwrócenie osi Y, aby uzyskać poprawną projekcję
    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f); // Pozycja kamery
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Punkt, na który patrzy kamera
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);  // Wektor 'góry' kamery

    ubo.view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    // Zaktualizowanie uniform buffer
    void* data3;
    vkMapMemory(g_Device, uniformBufferMemory, 0, sizeof(ubo), 0, &data3);
    memcpy(data3, &ubo, sizeof(ubo));
    vkUnmapMemory(g_Device, uniformBufferMemory);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = g_DescriptorSet; // Descriptor set
    descriptorWrite.dstBinding = 0; // Indeks w layout
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1; // Liczba descriptorów
    descriptorWrite.pBufferInfo = &bufferDescInfo; // Informacje o buforze

    vkUpdateDescriptorSets(g_Device, 1, &descriptorWrite, 0, nullptr);

    // if (vkAllocateDescriptorSets(g_Device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to allocate descriptor set!");
    // }

    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = g_RenderPass;
    info.framebuffer = g_Framebuffer;
    info.renderArea.extent.width = width;
    info.renderArea.extent.height = height;
    info.clearValueCount = 2;
    info.pClearValues = clearValues;

    //vkCmdBeginRenderPass(g_CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

    // Zakładam, że masz deskryptory dla uniformów, więc je wiążesz (np. dla modelu, widoku, projekcji).
    vkCmdBindDescriptorSets(g_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_PipelineLayout, 0, 1, &g_DescriptorSet, 0, nullptr);

    // Rysowanie sześcianu - ilość indeksów zależy od liczby trójkątów w sześcianie (36 dla pełnego sześcianu)
    vkCmdDrawIndexed(g_CommandBuffer, 36, 1, 0, 0, 0);

    vkCmdEndRenderPass(g_CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &g_WaitSemaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &g_CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &g_RenderCompleteSemaphore;

        err = vkEndCommandBuffer(g_CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, g_Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+Vulkan example", nullptr, nullptr);
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }

    ImVector<const char*> extensions;
    uint32_t extensions_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    for (uint32_t i = 0; i < extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);
    SetupVulkan(extensions);

    CreatePipeline();

    // Create Window Surface
    VkResult err = glfwCreateWindowSurface(g_Instance, window, g_Allocator, &g_Surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, g_Surface, w, h);

    // VkExtent2D swapchainExtent = {};
    //
    // VkSurfaceCapabilitiesKHR surfaceCapabilities;
    // vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_PhysicalDevice, g_Surface, &surfaceCapabilities);
    //
    // // Ustawianie swapchainExtent
    // if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
    //     // Użyj wartości z surfaceCapabilities, jeśli są ustalone
    //     swapchainExtent = surfaceCapabilities.currentExtent;
    // } else {
    //     // W przeciwnym razie ustal własny rozmiar (np. maksymalny rozmiar)
    //     swapchainExtent.width = 800;  // Szerokość
    //     swapchainExtent.height = 600;  // Wysokość
    // }
    //
    // VkSwapchainCreateInfoKHR swapchainInfo = {};
    // swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    // swapchainInfo.surface = g_Surface;  // Musisz mieć wcześniej utworzoną powierzchnię
    // swapchainInfo.minImageCount = 2;  // Liczba obrazów w swapchainie
    // swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;  // Format obrazu
    // swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // Przestrzeń kolorów
    // swapchainInfo.imageExtent = swapchainExtent;  // Rozmiar obrazów
    // swapchainInfo.imageArrayLayers = 1;
    // swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //
    // uint32_t queueFamilyCount = 0;
    // vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &queueFamilyCount, nullptr);
    // std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    // vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &queueFamilyCount, queueFamilies.data());
    //
    // // Sprawdź, która rodzina kolejek obsługuje prezentację
    // g_PresentQueueFamilyIndex = -1;  // Domyślnie -1, jeśli nie znaleziono
    // for (uint32_t i = 0; i < queueFamilyCount; i++) {
    //     VkBool32 presentSupport = false;
    //     vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, i, g_Surface, &presentSupport);
    //     if (presentSupport) {
    //         g_PresentQueueFamilyIndex = i;
    //         break; // Znaleziono odpowiednią rodzinę kolejek
    //     }
    // }
    //
    // // Sprawdź, czy znaleziono indeks
    // if (g_PresentQueueFamilyIndex == -1) {
    //     // Obsłuż sytuację, w której nie znaleziono odpowiedniej rodziny kolejek
    // }
    //
    // // Wybór kolejki
    // uint32_t queueFamilyIndices[] = { g_QueueFamily, g_QueueFamily };
    // if (g_QueueFamily != g_PresentQueueFamilyIndex) {
    //     swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    //     swapchainInfo.queueFamilyIndexCount = 2;
    //     swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    // } else {
    //     swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //     swapchainInfo.queueFamilyIndexCount = 0;  // Nie używamy współdzielonych kolejek
    // }
    //
    // // Ustawienie właściwości swapchain
    // swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;  // Transformacja
    // swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // Przezroczystość
    // swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;  // Tryb prezentacji
    // swapchainInfo.clipped = VK_TRUE;  // Przycinanie
    // swapchainInfo.oldSwapchain = VK_NULL_HANDLE;  // Brak starego swapchaina (pierwsze utworzenie)
    //
    // // Tworzenie swapchaina
    // VkResult r = vkCreateSwapchainKHR(g_Device, &swapchainInfo, nullptr, &g_Swapchain);
    // if (r != VK_SUCCESS) {
    //     std::cerr << "Failed to create swapchain, error code: " << r << std::endl;
    //     throw std::runtime_error("failed to create swapchain!");
    // }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.RenderPass = wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    //VARIABLES
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 active_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 inactive_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 activating_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 deactivating_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static int f = 3;
    static int a = 1;
    static bool as = false;
    static int counter = 0;
    static int T = 1;
    static int t = 1;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = 0;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

    // Create the command pool
    if (vkCreateCommandPool(g_Device, &poolInfo, nullptr, &g_CommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = g_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // For primary command buffer
    allocInfo.commandBufferCount = 1; // Allocate a single command buffer

    // Allocate the command buffer
    if (vkAllocateCommandBuffers(g_Device, &allocInfo, &g_CommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffer!");
    }

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    VkResult result = vkCreateSemaphore(g_Device, &semaphoreInfo, nullptr, &g_WaitSemaphore);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    result = vkCreateSemaphore(g_Device, &semaphoreInfo, nullptr, &g_RenderCompleteSemaphore);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;  // Może być VK_FENCE_CREATE_SIGNALED_BIT, jeśli potrzebujesz go początkowo w stanie "signaled"

    result = vkCreateFence(g_Device, &fenceInfo, nullptr, &g_Fence);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    g_Swapchain = g_MainWindowData.Swapchain;


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        if(glfwGetKey(window, GLFW_KEY_W))
        {
            pos.x += 1;
        }

        if(glfwGetKey(window, GLFW_KEY_S))
        {
            pos.x -= 1;
        }

        if(glfwGetKey(window, GLFW_KEY_W))
        {
            pos.z += 1;
        }

        if(glfwGetKey(window, GLFW_KEY_W))
        {
            pos.z -= 1;
        }
        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && (g_SwapChainRebuild || g_MainWindowData.Width != fb_width || g_MainWindowData.Height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, fb_width, fb_height, g_MinImageCount);
            g_MainWindowData.FrameIndex = 0;
            g_SwapChainRebuild = false;
        }
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            int ctrl_max = (f*f*f)-1;

            ImGui::Begin("Cellular automata 3D");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Text("Use this panel to adjust simulation parameters.");               // Display some text (you can use a format strings too)
            ImGui::SliderInt("Cube edge size", &f, 3, 100);
            ImGui::SliderInt("Initially active cells", &a, 1,ctrl_max);
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

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data);

            //VkCommandBuffer commandBuffer = wd->Frames[wd->FrameIndex].CommandBuffer;
            //drawCube(commandBuffer, pipelineLayout, graphicsPipeline, vertexBuffer, indexBuffer, descriptorSet, indexCount);

            FramePresent(wd);
        }
    }

    // Cleanup
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}