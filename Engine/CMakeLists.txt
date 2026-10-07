# 1. 指定最低 CMake 版本（必须放在最前面）
cmake_minimum_required(VERSION 3.15)

# 2. 定义项目名称、版本、支持的语言
project(EngineDemo
    VERSION     1.0.0
    DESCRIPTION "A demo of openGL Engine"
    LANGUAGES   CXX C
)

# 3. 设置 C++ 标准（全局默认值，但下游仍可覆盖）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#   设置游戏资源路径
set(assets_dir ${CMAKE_SOURCE_DIR}/assets)

#   设置配置文件
configure_file(
    ${PROJECT_SOURCE_DIR}/config.h.in
    ${PROJECT_BINARY_DIR}/config.h
)

#   设置头文件接口库
add_library(include_dir INTERFACE)
target_include_directories(include_dir INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#   设置源文件
set(engine_source_files
    src/engine/Engine.cpp
    src/engine/Application.cpp
    src/inputManager/inputManager.cpp
    src/IO_file/file_system.cpp
    src/shaderAPI/shaderProgram.cpp
    src/shaderAPI/texture.cpp
    src/shaderAPI/shaderAPI.cpp
    src/shaderAPI/FBO.cpp
    src/shaderAPI/FBOManager.cpp
    src/render/material.cpp
    src/render/mesh.cpp
    src/render/renderQueue.cpp
    src/render/sceneRenderer.cpp
    src/render/sceneRenderer.cpp
    src/render/renderPipeline.cpp
    src/scene/gameObject.cpp
    src/scene/scene.cpp
    src/scene/component.cpp
    src/scene/components/meshComponent.cpp
    src/scene/components/cameraComponent.cpp
    src/scene/components/playerControlComponent.cpp
    src/scene/components/lightComponent.cpp
    src/scene/components/animationComponent.cpp
    src/scene/components/physicsComponent.cpp
    src/scene/components/audioComponent.cpp
    src/scene/components/audioListenerComponent.cpp
    src/scene/components/spriteComponent.cpp
    src/scene/components/ui/UIElementComponent.cpp
    src/scene/components/ui/canvasComponent.cpp
    src/scene/components/ui/textComponent.cpp
    src/scene/components/ui/buttonComponent.cpp
    src/scene/components/ui/uiInputSystem.cpp
    src/physics/physicsManager.cpp
    src/physics/collider.cpp
    src/physics/rigidBody.cpp
    src/physics/kinematicCharacterControl.cpp
    src/physics/colliderObject.cpp
    src/audio/audioManager.cpp
    src/audio/audio.cpp
    src/font/font.cpp
    src/font/fontManager.cpp
)

#   显式列出 imgui 的核心源文件和后端文件
set(imgui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp      # 可选，如果你需要示例
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_impl_opengl3.cpp
)
#   合并
list(APPEND engine_source_files ${imgui_sources})

#   编译引擎为库
add_library(${PROJECT_NAME} STATIC ${engine_source_files})

#   为glad创建一个STATIC库目标 添加一个普通的静态库来处理glad.c
add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/src/glad.c)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include
)

#   为nlohmann/json创建一个INTERFACE库目标
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nlohmann
)

#   为cgltf创建一个INTERFACE库目标
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cgltf
)

#   为miniaudio创建一个INTERFACE库目标
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniaudio
)
#   根据平台设置miniaudio
if(UNIX AND NOT APPLE)
    target_link_libraries(miniaudio INTERFACE dl pthread m)
endif()
if(WIN32)
    target_link_libraries(miniaudio INTERFACE winmm user32 ole32)
endif()
if(APPLE)
    find_Library(COREAUDIO_FRAMEWORK CoreAudio)
    find_Library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
    find_Library(AUDIOUNIT_FRAMEWORK Audiounit)
    find_Library(COREFOUNDATION_FRAMEWORK COREfOUNDATION)
    target_link_libraries(miniaudio INTERFACE
        ${COREAUDIO_FRAMEWORK}
        ${AUDIOTOOLBOX_FRAMEWORK}
        ${AUDIOUNIT_FRAMEWORK}
        ${COREFOUNDATION_FRAMEWORK}
    )
endif()


#   glfw、glm库源码引入
add_subdirectory(thirdparty/glfw-3.4 ${CMAKE_CURRENT_BINARY_DIR}/glfw_build)
add_subdirectory(thirdparty/glm ${CMAKE_CURRENT_BINARY_DIR}/glm_build)

#   将bullet作为子目录包含
option(BUILD_UNIT_TESTS OFF)
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_SHARED_LIBS OFF)
add_subdirectory(thirdparty/bullet3-3.25)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/bullet3-3.25/src)

#   freetype库
add_subdirectory(thirdparty/freetype-2.14.3 ${CMAKE_CURRENT_BINARY_DIR}/freetype_build)


#   链接依赖
target_link_libraries(${PROJECT_NAME} PUBLIC
    include_dir
    glfw
    glad
    glm
    nlohmann_json
    cgltf
    BulletDynamics
    BulletCollision
    LinearMath
    miniaudio
    freetype
)

#   在生成路径找config.h
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

#   自动拷贝assets目录到build路径下
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${assets_dir}
    ${CMAKE_BINARY_DIR}/assets
)