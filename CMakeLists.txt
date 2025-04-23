# File: CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Curses REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/GameMap.cpp
    src/GameObject.cpp
    src/GameEngine/LoadSave.cpp
    src/GameEngine/Rendering.cpp
    src/GameEngine/Logic.cpp  # 新增
    src/main.cpp
)

# Executable
add_executable(GameEngine ${SOURCES})

# Link libraries
target_link_libraries(GameEngine ${CURSES_LIBRARIES})

# Installation (optional)
install(TARGETS GameEngine DESTINATION bin)