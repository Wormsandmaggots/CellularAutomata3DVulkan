cmake_minimum_required(VERSION 3.27 FATAL_ERROR)
project(CellularAutomata3DVulkan)

add_compile_options(-fno-rtti)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


include(global_settings)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")


# ---- Dependencies ----
add_subdirectory(thirdparty)


# ---- Main project's files ----
add_subdirectory(code)
