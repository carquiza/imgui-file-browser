# Test application for ImFileBrowser scaling
# This is a standalone test - NOT built by default

cmake_minimum_required(VERSION 3.14)
project(ImFileBrowserTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Get imgui include directory from the imported target
# We need to pass this to ImFileBrowser before adding it as a subdirectory
get_target_property(IMGUI_INCLUDE_DIRS imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
set(IMGUI_INCLUDE_DIR "${IMGUI_INCLUDE_DIRS}" CACHE PATH "ImGui include directory")

message(STATUS "ImGui include dir: ${IMGUI_INCLUDE_DIR}")

# Add ImGuiScaling (required by ImFileBrowser)
# https://github.com/carquiza/imgui-scaling
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../imgui-scaling ${CMAKE_CURRENT_BINARY_DIR}/ImGuiScaling)

# Add parent directory for ImFileBrowser (it will use IMGUI_INCLUDE_DIR and ImGuiScaling)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/ImFileBrowser)

# Create test executable
add_executable(ImFileBrowserTest main.cpp)

target_link_libraries(ImFileBrowserTest PRIVATE
    ImFileBrowser
    imgui::imgui
    glfw
    OpenGL::GL
)

# Set output directory
set_target_properties(ImFileBrowserTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
