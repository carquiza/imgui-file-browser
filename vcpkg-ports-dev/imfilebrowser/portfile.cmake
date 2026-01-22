# Development portfile - uses local source directory
# For production, use the vcpkg-ports version instead
#
# Set the environment variable IMFILEBROWSER_SOURCE_PATH to your local checkout:
#   Windows: set IMFILEBROWSER_SOURCE_PATH=D:\Source\AIResearch\imgui-file-browser
#   Linux:   export IMFILEBROWSER_SOURCE_PATH=/home/user/src/imgui-file-browser

if(DEFINED ENV{IMFILEBROWSER_SOURCE_PATH})
    set(SOURCE_PATH "$ENV{IMFILEBROWSER_SOURCE_PATH}")
else()
    message(FATAL_ERROR "Environment variable IMFILEBROWSER_SOURCE_PATH is not set.\n"
        "Set it to your local imgui-file-browser source directory.")
endif()

# Normalize path separators
file(TO_CMAKE_PATH "${SOURCE_PATH}" SOURCE_PATH)

# Verify the path exists
if(NOT EXISTS "${SOURCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Local source not found at ${SOURCE_PATH}.\n"
        "Check that IMFILEBROWSER_SOURCE_PATH points to a valid imgui-file-browser checkout.")
endif()

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        signals IMFILEBROWSER_ENABLE_SIGNALS
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/ImFileBrowser)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Create a simple copyright file if LICENSE doesn't exist
if(EXISTS "${SOURCE_PATH}/LICENSE")
    vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
else()
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "Copyright (c) 2024. All rights reserved.")
endif()
