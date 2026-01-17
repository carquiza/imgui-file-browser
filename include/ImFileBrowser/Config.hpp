// Config.hpp
// Configuration system for ImFileBrowser library
// Standalone ImGui-based file browser

#pragma once

#include "imgui.h"

namespace ImFileBrowser {

/**
 * @brief Color configuration for file browser
 */
struct ColorConfig {
    // File list area
    ImU32 listBackground   = IM_COL32(25, 25, 30, 255);    // Dark, slightly blue
    ImU32 listBorder       = IM_COL32(80, 80, 90, 255);    // Subtle border

    // Text colors
    ImU32 directoryText    = IM_COL32(100, 200, 255, 255); // Cyan for directories
    ImU32 fileText         = IM_COL32(220, 220, 220, 255); // Light gray for files
    ImU32 secondaryText    = IM_COL32(180, 180, 180, 255); // Medium gray for size/date
    ImU32 selectedText     = IM_COL32(255, 255, 255, 255); // White when selected

    // Selection
    ImU32 selectedRow      = IM_COL32(0, 100, 180, 180);   // Blue highlight
    ImU32 hoveredRow       = IM_COL32(60, 60, 70, 255);    // Subtle hover

    // Path bar
    ImU32 pathBackground   = IM_COL32(40, 40, 45, 255);
    ImU32 pathText         = IM_COL32(180, 180, 180, 255);
};

/**
 * @brief Size configuration for file browser
 *
 * All values in pixels. Desktop mode uses smaller values for compact UI,
 * Touch mode uses larger values for finger-friendly targets.
 */
struct SizeConfig {
    float rowHeight = 24.0f;        // Height of file list rows
    float buttonHeight = 28.0f;     // Height of toolbar/dialog buttons
    float buttonWidth = 80.0f;      // Width of dialog buttons
    float inputHeight = 26.0f;      // Height of input fields
    float pathBarHeight = 32.0f;    // Height of path breadcrumb bar
    float iconSize = 18.0f;         // Size of file/folder icons
    float fontSize = 14.0f;         // Base font size
    float scrollbarWidth = 16.0f;   // Width of scrollbars
    float grabMinSize = 16.0f;      // Minimum scrollbar grab size

    /**
     * @brief Get default desktop sizing
     */
    static SizeConfig Desktop() {
        return SizeConfig();  // Default values are desktop
    }

    /**
     * @brief Get touch-optimized sizing
     */
    static SizeConfig Touch() {
        SizeConfig config;
        config.rowHeight = 52.0f;
        config.buttonHeight = 48.0f;
        config.buttonWidth = 120.0f;
        config.inputHeight = 48.0f;
        config.pathBarHeight = 56.0f;
        config.iconSize = 28.0f;
        config.fontSize = 16.0f;
        config.scrollbarWidth = 40.0f;
        config.grabMinSize = 40.0f;
        return config;
    }
};

/**
 * @brief Main library configuration
 */
struct LibraryConfig {
    ColorConfig colors;
    SizeConfig sizes;
    bool touchMode = false;   // When true, uses touch-optimized behavior

    /**
     * @brief Get default configuration
     */
    static LibraryConfig Default() {
        return LibraryConfig();
    }

    /**
     * @brief Get touch-optimized configuration
     */
    static LibraryConfig Touch() {
        LibraryConfig config;
        config.touchMode = true;
        config.sizes = SizeConfig::Touch();
        return config;
    }
};

/**
 * @brief Get the global library configuration
 * @return Reference to current configuration
 */
LibraryConfig& GetConfig();

/**
 * @brief Set the global library configuration
 * @param config New configuration to use
 */
void SetConfig(const LibraryConfig& config);

} // namespace ImFileBrowser
