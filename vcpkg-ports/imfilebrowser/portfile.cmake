vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO carquiza/imgui-file-browser
    REF v${VERSION}
    SHA512 0  # Update with actual SHA512 after first release
    HEAD_REF main
)

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

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
