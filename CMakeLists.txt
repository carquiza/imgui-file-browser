# ImFileBrowser - Standalone ImGui File Browser Library
# https://github.com/user/imgui-file-browser (update with actual URL)

cmake_minimum_required(VERSION 3.14)
project(ImFileBrowser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(IMFILEBROWSER_ENABLE_SIGNALS "Enable sigslot signal support" OFF)

# Library sources
set(IMFILEBROWSER_SOURCES
    src/FileBrowserDialog.cpp
    src/ConfirmationDialog.cpp
    src/Config.cpp
)

# Library headers (for IDE integration)
set(IMFILEBROWSER_HEADERS
    include/ImFileBrowser/ImFileBrowser.hpp
    include/ImFileBrowser/Types.hpp
    include/ImFileBrowser/FileFilter.hpp
    include/ImFileBrowser/FileSystemHelper.hpp
    include/ImFileBrowser/Config.hpp
    include/ImFileBrowser/Icons.hpp
    include/ImFileBrowser/FileBrowserDialog.hpp
    include/ImFileBrowser/ConfirmationDialog.hpp
)

# Create the library
add_library(ImFileBrowser STATIC
    ${IMFILEBROWSER_SOURCES}
    ${IMFILEBROWSER_HEADERS}
)

# Create alias for namespaced usage
add_library(ImFileBrowser::ImFileBrowser ALIAS ImFileBrowser)

# Include directories
target_include_directories(ImFileBrowser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# =============================================================================
# ImGui dependency
# =============================================================================
# The user must provide imgui. There are several ways:
#
# Option 1: Provide an 'imgui' target (recommended)
#   add_subdirectory(imgui)  # or FetchContent
#   add_subdirectory(imgui-file-browser)
#
# Option 2: Set IMGUI_INCLUDE_DIR before adding this subdirectory
#   set(IMGUI_INCLUDE_DIR "/path/to/imgui")
#   add_subdirectory(imgui-file-browser)
#
# Option 3: Link imgui manually after
#   add_subdirectory(imgui-file-browser)
#   target_include_directories(ImFileBrowser PUBLIC /path/to/imgui)
# =============================================================================

if(TARGET imgui)
    target_link_libraries(ImFileBrowser PUBLIC imgui)
    message(STATUS "ImFileBrowser: Using 'imgui' target")
elseif(DEFINED IMGUI_INCLUDE_DIR)
    target_include_directories(ImFileBrowser PUBLIC ${IMGUI_INCLUDE_DIR})
    message(STATUS "ImFileBrowser: Using IMGUI_INCLUDE_DIR=${IMGUI_INCLUDE_DIR}")
else()
    message(STATUS "ImFileBrowser: No imgui target or IMGUI_INCLUDE_DIR found.")
    message(STATUS "  Make sure imgui headers are available when compiling.")
    message(STATUS "  See README.md for integration options.")
endif()

# =============================================================================
# Optional sigslot support
# =============================================================================
if(IMFILEBROWSER_ENABLE_SIGNALS)
    if(TARGET Pal::Sigslot)
        target_link_libraries(ImFileBrowser PUBLIC Pal::Sigslot)
        target_compile_definitions(ImFileBrowser PUBLIC IMFILEBROWSER_USE_SIGSLOT)
        message(STATUS "ImFileBrowser: sigslot signal support enabled")
    else()
        message(WARNING "ImFileBrowser: IMFILEBROWSER_ENABLE_SIGNALS=ON but Pal::Sigslot not found")
    endif()
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(ImFileBrowser PRIVATE /W4)
else()
    target_compile_options(ImFileBrowser PRIVATE -Wall -Wextra)
endif()

# Set folder for IDEs
set_target_properties(ImFileBrowser PROPERTIES FOLDER "Libraries")

# Source groups for Visual Studio
source_group("Header Files" FILES ${IMFILEBROWSER_HEADERS})
source_group("Source Files" FILES ${IMFILEBROWSER_SOURCES})
