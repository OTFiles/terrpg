cmake_minimum_required(VERSION 3.15)
project(GameEngine LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录配置
include_directories(
    include
    include/Commands
    include/Commands/ConcreteCommands
)

# 源文件配置
file(GLOB_RECURSE SOURCE_FILES
    "src/*.cpp"
    "src/Commands/*.cpp"
    "src/Commands/ConcreteCommands/*.cpp"
    "src/GameEngine/*.cpp"
)

# 可执行文件配置
add_executable(GameEngine 
    ${SOURCE_FILES}
    src/main.cpp
)

# 编译器选项
if(MSVC)
    target_compile_options(GameEngine PRIVATE /W4 /WX)
else()
    target_compile_options(GameEngine PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

# 输出目录配置
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)