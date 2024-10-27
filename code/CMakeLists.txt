file(GLOB_RECURSE SOURCE_FILES
        src/*.c
        src/*.cpp)


file(GLOB_RECURSE HEADER_FILES
        inc/*.h
        inc/*.hpp)

file(GLOB_RECURSE dlls
        ${CMAKE_CURRENT_BINARY_DIR}/../_deps/*.dll
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})

add_executable(CellularAutomata3DVulkan
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${HEADER_FILES}
        ${SOURCE_FILES}
)


#Vulkan
find_package(Vulkan REQUIRED)


target_include_directories(CellularAutomata3DVulkan PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${glad_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
        ${imgui_impl_SOURCE_DIR}
        ${Vulkan_INCLUDE_DIRS}
)


target_link_libraries(CellularAutomata3DVulkan
        glad
        spdlog
        glm::glm
        glfw
        imgui
        Vulkan::Vulkan
        stb
        tiny_obj
)

target_link_libraries(CellularAutomata3DVulkan Vulkan::Vulkan)


add_custom_command(TARGET CellularAutomata3DVulkan POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/res
        ${CMAKE_CURRENT_BINARY_DIR}/res
)


if(MSVC)
    target_compile_definitions(CellularAutomata3DVulkan PUBLIC NOMINMAX)
endif()


function(copy_dll_dependencies DESTINATION_DIR DEPENDENCIES)

    if (NOT DESTINATION_DIR)
        message(FATAL_ERROR "copy_dll_dependencies: DESTINATION_DIR not specified")
    endif()

    if (NOT DEPENDENCIES)
        message(STATUS "copy_dll_dependencies: DEPENDENCIES not specified or nothing to copy")
    endif()

    foreach (DLL_PATH ${DEPENDENCIES})

        get_filename_component(DLL_NAME ${DLL_PATH} NAME)
        set(DLL_DESTINATION ${DESTINATION_DIR}/${DLL_NAME})

        if (NOT EXISTS ${DLL_DESTINATION})
            message(STATUS "${DLL_NAME} not found in destination directory, copying...")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_PATH}" "${DLL_DESTINATION}")
        else()
            message(STATUS "${DLL_NAME} has been found in destination directory, no need for copying")
        endif()

    endforeach()

endfunction()


copy_dll_dependencies( ${CMAKE_CURRENT_BINARY_DIR} "${dlls}")
