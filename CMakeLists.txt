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
# The user must provide imgui. ImGui doesn't ship with a CMakeLists.txt and
# requires backend configuration, so it cannot be auto-fetched.
#
# Option 1: Provide an 'imgui' target (recommended)
#   # Create imgui target with your chosen backends
#   add_subdirectory(imgui-file-browser)
#
# Option 2: Set IMGUI_INCLUDE_DIR before adding this subdirectory
#   set(IMGUI_INCLUDE_DIR "/path/to/imgui")
#   add_subdirectory(imgui-file-browser)
#
# Option 3: Use vcpkg
#   vcpkg install imgui[core]:x64-windows
# =============================================================================

if(TARGET imgui)
    target_link_libraries(ImFileBrowser PUBLIC imgui)
    message(STATUS "ImFileBrowser: Using 'imgui' target")
elseif(TARGET imgui::imgui)
    target_link_libraries(ImFileBrowser PUBLIC imgui::imgui)
    message(STATUS "ImFileBrowser: Using 'imgui::imgui' target (vcpkg)")
elseif(DEFINED IMGUI_INCLUDE_DIR)
    target_include_directories(ImFileBrowser PUBLIC ${IMGUI_INCLUDE_DIR})
    message(STATUS "ImFileBrowser: Using IMGUI_INCLUDE_DIR=${IMGUI_INCLUDE_DIR}")
else()
    message(WARNING "ImFileBrowser: No imgui target or IMGUI_INCLUDE_DIR found.")
    message(WARNING "  Make sure imgui headers are available when compiling.")
    message(WARNING "  See README.md for integration options.")
endif()

# =============================================================================
# ImGuiScaling dependency
# =============================================================================
# ImGuiScaling provides the unified scaling system.
# https://github.com/carquiza/imgui-scaling
#
# The user must provide ImGuiScaling. Options:
#
# Option 1: Provide an 'ImGuiScaling' target (recommended)
#   add_subdirectory(imgui-scaling)
#   add_subdirectory(imgui-file-browser)
#
# Option 2: Set IMGUISCALING_INCLUDE_DIR before adding this subdirectory
#   set(IMGUISCALING_INCLUDE_DIR "/path/to/imgui-scaling/include")
#   add_subdirectory(imgui-file-browser)
# =============================================================================

if(NOT TARGET ImGuiScaling::ImGuiScaling AND NOT TARGET ImGuiScaling AND NOT DEFINED IMGUISCALING_INCLUDE_DIR)
    message(STATUS "ImFileBrowser: ImGuiScaling not found. Attempting to fetch...")
    include(FetchContent)
    FetchContent_Declare(
        imgui-scaling
        GIT_REPOSITORY https://github.com/carquiza/imgui-scaling.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(imgui-scaling)
endif()

if(TARGET ImGuiScaling::ImGuiScaling)
    target_link_libraries(ImFileBrowser PUBLIC ImGuiScaling::ImGuiScaling)
    message(STATUS "ImFileBrowser: Using 'ImGuiScaling::ImGuiScaling' target")
elseif(TARGET ImGuiScaling)
    target_link_libraries(ImFileBrowser PUBLIC ImGuiScaling)
    message(STATUS "ImFileBrowser: Using 'ImGuiScaling' target")
elseif(DEFINED IMGUISCALING_INCLUDE_DIR)
    target_include_directories(ImFileBrowser PUBLIC ${IMGUISCALING_INCLUDE_DIR})
    message(STATUS "ImFileBrowser: Using IMGUISCALING_INCLUDE_DIR=${IMGUISCALING_INCLUDE_DIR}")
else()
    message(STATUS "ImFileBrowser: No ImGuiScaling target or IMGUISCALING_INCLUDE_DIR found.")
    message(STATUS "  Make sure ImGuiScaling headers are available when compiling.")
    message(STATUS "  See https://github.com/carquiza/imgui-scaling")
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

# =============================================================================
# Installation rules (for vcpkg and find_package support)
# =============================================================================
# Only include install rules when built as a standalone project or via vcpkg,
# not when included as a subdirectory via add_subdirectory/FetchContent.

# Check if we're the top-level project (CMake 3.21+ provides PROJECT_IS_TOP_LEVEL)
if(CMAKE_VERSION VERSION_LESS "3.21")
    get_directory_property(_has_parent PARENT_DIRECTORY)
    if(_has_parent)
        set(PROJECT_IS_TOP_LEVEL OFF)
    else()
        set(PROJECT_IS_TOP_LEVEL ON)
    endif()
endif()

if(PROJECT_IS_TOP_LEVEL OR IMFILEBROWSER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install the library
    install(TARGETS ImFileBrowser
        EXPORT ImFileBrowserTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install headers
    install(DIRECTORY include/ImFileBrowser
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Create and install CMake config files
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/ImFileBrowserConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ImFileBrowserConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ImFileBrowserConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImFileBrowser
    )

    install(EXPORT ImFileBrowserTargets
        FILE ImFileBrowserTargets.cmake
        NAMESPACE ImFileBrowser::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImFileBrowser
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ImFileBrowserConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ImFileBrowserConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImFileBrowser
    )
endif()
