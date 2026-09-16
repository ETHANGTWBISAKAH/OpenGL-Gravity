cmake_minimum_required(VERSION 3.10)
project(Test-Lat2)

# Tell CMake where header folders live
include_directories(include)

# Compile main.cpp together with glad.c
add_executable(main src/main.cpp src/glad.c)

# Link GLFW and system graphics libraries
target_link_directories(main PRIVATE lib)
target_link_libraries(main glfw3 opengl32 gdi32)