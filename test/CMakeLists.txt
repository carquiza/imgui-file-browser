# Test application for ImFileBrowser scaling
# This is a standalone test - NOT built by default

cmake_minimum_required(VERSION 3.14)
project(ImFileBrowserTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Try to find imgui via vcpkg first
find_package(imgui CONFIG QUIET)

if(imgui_FOUND)
    message(STATUS "Found imgui via vcpkg")
    # Get imgui include directory from the imported target
    get_target_property(IMGUI_INCLUDE_DIRS imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
    set(IMGUI_INCLUDE_DIR "${IMGUI_INCLUDE_DIRS}" CACHE PATH "ImGui include directory")
    set(IMGUI_TARGET imgui::imgui)
else()
    message(STATUS "imgui not found via vcpkg, fetching from GitHub...")

    # Fetch imgui from GitHub
    include(FetchContent)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
    )
    FetchContent_MakeAvailable(imgui)

    # Create imgui library with GLFW+OpenGL3 backends
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

    # For Linux, we may need additional libraries
    if(UNIX AND NOT APPLE)
        find_package(X11 QUIET)
        if(X11_FOUND)
            target_link_libraries(imgui PUBLIC ${X11_LIBRARIES})
        endif()
    endif()

    set(IMGUI_INCLUDE_DIR "${imgui_SOURCE_DIR}" CACHE PATH "ImGui include directory")
    set(IMGUI_TARGET imgui)
endif()

message(STATUS "ImGui include dir: ${IMGUI_INCLUDE_DIR}")

# Add ImGuiScaling (required by ImFileBrowser)
# https://github.com/carquiza/imgui-scaling
# ImGuiScaling will be fetched automatically by ImFileBrowser if not found

# Add parent directory for ImFileBrowser (it will use IMGUI_INCLUDE_DIR and ImGuiScaling)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/ImFileBrowser)

# Create test executable
add_executable(ImFileBrowserTest main.cpp)

target_link_libraries(ImFileBrowserTest PRIVATE
    ImFileBrowser
    ${IMGUI_TARGET}
    glfw
    OpenGL::GL
)

# Set output directory
set_target_properties(ImFileBrowserTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
