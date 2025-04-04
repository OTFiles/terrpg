# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)
project(TerminalRPG 
    VERSION 1.0
    LANGUAGES CXX
    DESCRIPTION "A terminal-based RPG game engine"
)

# 设置C++标准为17（解决optional等问题）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找ncurses库
find_package(Curses REQUIRED)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 创建动态库
add_library(GameEngine SHARED
    src/GameEngine.cpp
    src/GameEngine.h
)

# 设置目标属性
target_compile_definitions(GameEngine PRIVATE 
    GAME_ENGINE_BUILD
    $<$<BOOL:${WIN32}>:NOMINMAX>
)

target_include_directories(GameEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
    ${CURSES_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(GameEngine PRIVATE
    ${CURSES_LIBRARIES}
)

# 创建可执行文件
add_executable(rpg 
    src/main.cpp
)

# 可执行文件配置
target_include_directories(rpg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(rpg PRIVATE
    GameEngine
    ${CURSES_LIBRARIES}
)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 通用警告选项
    target_compile_options(GameEngine PRIVATE 
        -Wall
        -Wextra
        -Wpedantic
        -Werror=return-type
    )
    
    # 动态库符号控制
    target_compile_options(GameEngine PRIVATE
        -fvisibility=hidden
        -fvisibility-inlines-hidden
    )
    
    # 优化选项
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(GameEngine PRIVATE
            -O2
            -flto
        )
    endif()
    
    # 设置目标属性
    set_target_properties(GameEngine PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
    )
    
    # 可执行文件选项
    target_compile_options(rpg PRIVATE
        -Wall
        -Wextra
    )
elseif(MSVC)
    target_compile_options(GameEngine PRIVATE
        /W4
        /WX
        /EHsc
    )
endif()

# 安装规则
install(TARGETS GameEngine
    EXPORT GameEngineTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES src/GameEngine.h
    DESTINATION include
)

# 导出目标
export(EXPORT GameEngineTargets
    FILE "${CMAKE_BINARY_DIR}/GameEngineTargets.cmake"
)

# 包配置
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_BINARY_DIR}/GameEngineConfig.cmake"
    INSTALL_DESTINATION lib/cmake/GameEngine
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/GameEngineConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(EXPORT GameEngineTargets
    FILE GameEngineTargets.cmake
    DESTINATION lib/cmake/GameEngine
)

install(FILES
    "${CMAKE_BINARY_DIR}/GameEngineConfig.cmake"
    "${CMAKE_BINARY_DIR}/GameEngineConfigVersion.cmake"
    DESTINATION lib/cmake/GameEngine
)