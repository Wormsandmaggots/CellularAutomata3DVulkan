# CPM - package manager
include(CPM)


# adding packages via CPM
CPMAddPackage("gh:glfw/glfw#3.3.8")
CPMAddPackage("gh:g-truc/glm#0.9.9.8")
CPMAddPackage("gh:ocornut/imgui@1.91.3")
CPMAddPackage("gh:gabime/spdlog@1.10.0")


# glad
set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")

add_library(glad STATIC
        ${GLAD_DIR}/src/glad.c
)

target_include_directories(glad PRIVATE
        ${GLAD_DIR}/include
)

find_package(Vulkan REQUIRED)

set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "")
set(imgui_impl_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/imgui_impl CACHE INTERNAL "")

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        imgui_impl/imgui_impl_vulkan.cpp
        imgui_impl/imgui_impl_glfw.cpp
)

target_include_directories(imgui PRIVATE ${imgui_SOURCE_DIR})
target_link_libraries(imgui glfw Vulkan::Vulkan)


set(stb_DIR ${CMAKE_CURRENT_LIST_DIR}/stb CACHE INTERNAL "")

add_library(stb INTERFACE ${stb_DIR}/stb_image.h)

target_include_directories(stb INTERFACE ${stb_DIR})


set(tiny_obj_DIR ${CMAKE_CURRENT_LIST_DIR}/tiny_obj CACHE INTERNAL "")

add_library(tiny_obj INTERFACE ${tiny_obj_DIR}/tiny_obj_loader.h)

target_include_directories(tiny_obj INTERFACE ${tiny_obj_DIR})


set_target_properties(
        glfw
        imgui
        glad
        glm
        spdlog
        stb
        tiny_obj
        PROPERTIES FOLDER "thirdparty"
)


if (TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "thirdparty")
endif()