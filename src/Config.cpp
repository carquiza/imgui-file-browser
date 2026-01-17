// Config.cpp
// Global configuration implementation for ImFileBrowser library
// Standalone ImGui-based file browser

#include "ImFileBrowser/Config.hpp"
#include "ImFileBrowser/Icons.hpp"

namespace ImFileBrowser {

// Global configuration instance
static LibraryConfig g_config;

// Global icons instance
static IconSet g_icons;

LibraryConfig& GetConfig() {
    return g_config;
}

void SetConfig(const LibraryConfig& config) {
    g_config = config;
}

IconSet& GetIcons() {
    return g_icons;
}

void SetIcons(const IconSet& icons) {
    g_icons = icons;
}

} // namespace ImFileBrowser
