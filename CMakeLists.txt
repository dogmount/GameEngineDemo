# 1. 指定最低 CMake 版本（必须放在最前面）
cmake_minimum_required(VERSION 3.15)

# 2. 定义项目名称、版本、支持的语言
project(GameDemo
    VERSION     1.0.0
    DESCRIPTION "A demo of game by openGL Engine"
    LANGUAGES   CXX C
)

# 3. 设置 C++ 标准（全局默认值，但下游仍可覆盖）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 6. 设置源代码列表
set(src_sources
    src/main.cpp
    src/game.h
    src/game.cpp
    src/player.h
    src/player.cpp
    src/bullet.h
    src/bullet.cpp
    src/jumpPlatform.h
    src/jumpPlatform.cpp
    src/spawner.h
    src/spawner.cpp
)

# 7. 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 8. 可执行文件：主程序
# add_executable(${PROJECT_NAME} WIN32 ${src_sources})
add_executable(${PROJECT_NAME} ${src_sources})

# 添加子目录
add_subdirectory(Engine ${CMAKE_CURRENT_BINARY_DIR}/engine_build)

# 链接引擎库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    EngineDemo
)