// Config.cpp
// Global configuration implementation for ImFileBrowser library
// Standalone ImGui-based file browser

#include "ImFileBrowser/Config.hpp"
#include "ImFileBrowser/Icons.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstring>

namespace ImFileBrowser {

// Global configuration instance
static LibraryConfig g_config;

// Global icons instance
static IconSet g_icons;

// Persisted settings
static std::string g_lastPath;
static bool g_settingsHandlerRegistered = false;

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

const std::string& GetLastPath() {
    return g_lastPath;
}

void SetLastPath(const std::string& path) {
    if (g_lastPath != path) {
        g_lastPath = path;
    }
}

// ImGui settings handler callbacks
static void* SettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name) {
    // We only have one entry named "Data"
    if (strcmp(name, "Data") == 0) {
        return (void*)1;  // Non-null to indicate valid entry
    }
    return nullptr;
}

static void SettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
    if (!entry) return;

    // Parse "LastPath=<path>" format
    const char* prefix = "LastPath=";
    size_t prefixLen = strlen(prefix);
    if (strncmp(line, prefix, prefixLen) == 0) {
        g_lastPath = line + prefixLen;
    }
}

static void SettingsHandler_WriteAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
    if (g_lastPath.empty()) return;

    buf->appendf("[%s][Data]\n", handler->TypeName);
    buf->appendf("LastPath=%s\n", g_lastPath.c_str());
    buf->append("\n");
}

void RegisterSettingsHandler() {
    if (g_settingsHandlerRegistered) return;

    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return;

    ImGuiSettingsHandler handler;
    handler.TypeName = "ImFileBrowser";
    handler.TypeHash = ImHashStr("ImFileBrowser");
    handler.ReadOpenFn = SettingsHandler_ReadOpen;
    handler.ReadLineFn = SettingsHandler_ReadLine;
    handler.WriteAllFn = SettingsHandler_WriteAll;
    ctx->SettingsHandlers.push_back(handler);

    g_settingsHandlerRegistered = true;
}

} // namespace ImFileBrowser
