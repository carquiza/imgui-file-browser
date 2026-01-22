vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO carquiza/imgui-scaling
    REF v${VERSION}
    SHA512 0  # Update with actual SHA512 after first release
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/ImGuiScaling)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
