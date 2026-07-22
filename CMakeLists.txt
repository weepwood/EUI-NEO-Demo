cmake_minimum_required(VERSION 3.20)

project(EuiNeoDemo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# 只构建当前演示程序，避免同时编译框架自带示例。
set(EUI_BUILD_APPS OFF CACHE BOOL "" FORCE)
set(EUI_BUILD_USER_APPS OFF CACHE BOOL "" FORCE)
set(EUI_BUILD_TEST_FIXTURES OFF CACHE BOOL "" FORCE)
set(EUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(EUI_DEPS_MODE bundled CACHE STRING "" FORCE)
set(EUI_WINDOW_BACKEND glfw CACHE STRING "" FORCE)
set(EUI_RENDER_BACKEND opengl CACHE STRING "" FORCE)

FetchContent_Declare(
    eui_neo
    GIT_REPOSITORY https://github.com/sudoevolve/EUI-NEO.git
    GIT_TAG v0.5.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(eui_neo)

add_executable(eui_neo_demo
    "${eui_neo_SOURCE_DIR}/core/app/glfw_app_main.cpp"
    app.cpp
)

eui_neo_configure_app(eui_neo_demo)

set_target_properties(eui_neo_demo PROPERTIES
    OUTPUT_NAME "EUI-NEO-Demo"
)
