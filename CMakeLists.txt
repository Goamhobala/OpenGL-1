cmake_minimum_required(VERSION 3.16)
project(Assignment1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force CMake to look in Homebrew's default folders on Mac
if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local" ${CMAKE_PREFIX_PATH})
endif()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW)

# 1. Grab files from BOTH the src folder AND their provided build folder
file(GLOB SOURCES "src/*.cpp")
file(GLOB PRAC_SOURCES "build/*.cpp") 

# Compile everything together
add_executable(Assignment1 ${SOURCES} ${PRAC_SOURCES})

# 2. Tell CMake to look for header files (.h) in both folders
target_include_directories(Assignment1 PRIVATE src build)

# Link SDL2 and OpenGL
target_link_libraries(Assignment1 PRIVATE SDL2::SDL2 OpenGL::GL)

if(GLEW_FOUND)
    target_include_directories(Assignment1 PRIVATE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(Assignment1 PRIVATE GLEW::GLEW)
endif()