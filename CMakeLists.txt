# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(GameEngine LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录配置
include_directories(
    include
    include/Commands
    include/GameEngine
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
add_executable(GameEngine ${SOURCE_FILES})

# 设置默认构建类型为Release（如果未指定）
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 构建类型相关配置
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
    target_compile_definitions(GameEngine PRIVATE DEBUG)
endif()

# 查找并链接ncurses宽字符库
find_package(Curses REQUIRED COMPONENTS ncursesw)
if(NOT CURSES_FOUND)
    message(FATAL_ERROR "ncursesw library not found!")
endif()
target_link_libraries(GameEngine PRIVATE 
    ${CURSES_LIBRARIES}
    ${CURSES_NCURSESW_LIBRARIES}
)
target_include_directories(GameEngine PRIVATE ${CURSES_INCLUDE_DIR})
target_compile_definitions(GameEngine PRIVATE
    _XOPEN_SOURCE_EXTENDED
    HAVE_NCURSESW_H
    NCURSES_WIDECHAR=1
)

# Windows平台特殊设置
if(WIN32)
    target_compile_definitions(GameEngine PRIVATE
        _UNICODE
        UNICODE
    )
    target_link_libraries(GameEngine PRIVATE 
        mingw32
        ncursesw
    )
endif()

# 编译器选项
if(MSVC)
    target_compile_options(GameEngine PRIVATE /W4 /WX /utf-8)
else()
    target_compile_options(GameEngine PRIVATE 
        -Wall 
        -Wextra 
        -Wno-error=deprecated-declarations
        -pedantic
        -fexec-charset=UTF-8
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:Debug>:-O0 -g3>
    )
endif()

# 输出目录配置
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# 安装配置
install(TARGETS GameEngine
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)