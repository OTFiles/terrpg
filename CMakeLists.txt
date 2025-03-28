cmake_minimum_required(VERSION 3.12)
project(TerminalRPG CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找ncurses库
find_package(Curses REQUIRED)

# 添加可执行文件
add_executable(rpg
    src/main.cpp
    src/GameEngine.cpp
    src/GameEngine.h
)

# 包含头文件目录
target_include_directories(rpg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURSES_INCLUDE_DIRS}
)

# 链接ncurses库（Termux特殊处理）
target_link_libraries(rpg
    PRIVATE ${CURSES_LIBRARIES}
)

# 添加编译警告
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rpg PRIVATE -Wall -Wextra)
endif()