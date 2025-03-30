# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)
project(TerminalRPG CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找ncurses库
find_package(Curses REQUIRED)

# 创建动态库
add_library(GameEngine SHARED
    src/GameEngine.cpp
    src/GameEngine.h
)

# 设置动态库属性
target_include_directories(GameEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURSES_INCLUDE_DIRS}
)

# 创建可执行文件
add_executable(rpg src/main.cpp)

# 链接库文件
target_link_libraries(rpg
    PRIVATE 
    GameEngine
    ${CURSES_LIBRARIES}
)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(GameEngine PRIVATE 
        -Wall 
        -Wextra 
        -fvisibility=hidden
        -fvisibility-inlines-hidden
    )
    # 添加动态库符号导出
    set_target_properties(GameEngine PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()