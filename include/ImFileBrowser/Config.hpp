// Config.hpp
// Configuration system for ImFileBrowser library
// Standalone ImGui-based file browser

#pragma once

#include "imgui.h"
#include <ImGuiScaling/ImGuiScaling.hpp>

namespace ImFileBrowser {

// Import common base sizes from ImGuiScaling
namespace BaseSize {
    // Common sizes from ImGuiScaling
    using namespace ImGuiScaling::BaseSize;

    // Backward compatibility / Missing in some versions
    constexpr float BUTTON_SPACING = ImGuiScaling::BaseSize::SPACING;

    // File browser specific sizes (at 1.0x scale)
    // Dialog dimensions
    constexpr float DIALOG_WIDTH = ImGuiScaling::BaseSize::DIALOG_WIDTH_LARGE;  // 650px
    constexpr float DIALOG_HEIGHT = ImGuiScaling::BaseSize::DIALOG_HEIGHT_LARGE; // 450px
    constexpr float DIALOG_MIN_WIDTH = 400.0f;
    constexpr float DIALOG_MIN_HEIGHT = 300.0f;

    // File browser specific
    constexpr float PATH_BAR_HEIGHT = 32.0f;
    constexpr float ICON_BUTTON_WIDTH = 32.0f;

    // Table column widths
    constexpr float SIZE_COLUMN_WIDTH = 80.0f;
    constexpr float DATE_COLUMN_WIDTH = 120.0f;

    // Confirmation dialog
    constexpr float CONFIRM_MIN_WIDTH = ImGuiScaling::BaseSize::DIALOG_MIN_WIDTH;  // 300px
    constexpr float CONFIRM_MAX_WIDTH = 500.0f;
    constexpr float CONFIRM_ICON_SIZE = 32.0f;

    // Combo widths
    constexpr float DRIVES_COMBO_WIDTH = 90.0f;
    constexpr float SORT_COMBO_WIDTH = 70.0f;
    constexpr float POPUP_INPUT_WIDTH = 300.0f;

    // Touch mode sizes (file browser specific)
    constexpr float TOUCH_ROW_HEIGHT = ImGuiScaling::BaseSize::Touch::ROW_HEIGHT;  // 52px
    constexpr float TOUCH_BUTTON_HEIGHT = ImGuiScaling::BaseSize::Touch::BUTTON_HEIGHT;  // 48px
    constexpr float TOUCH_BUTTON_WIDTH = ImGuiScaling::BaseSize::Touch::BUTTON_WIDTH;  // 120px
    constexpr float TOUCH_ICON_SIZE = ImGuiScaling::BaseSize::Touch::ICON_SIZE;  // 28px
    constexpr float TOUCH_FONT_SIZE = ImGuiScaling::BaseSize::Touch::FONT_SIZE;  // 16px
    constexpr float TOUCH_INPUT_HEIGHT = ImGuiScaling::BaseSize::Touch::INPUT_HEIGHT;  // 48px
    constexpr float TOUCH_SCROLLBAR_WIDTH = ImGuiScaling::BaseSize::Touch::SCROLLBAR_WIDTH;  // 40px
    constexpr float TOUCH_GRAB_MIN_SIZE = ImGuiScaling::BaseSize::Touch::GRAB_MIN_SIZE;  // 40px

    constexpr float TOUCH_PATH_BAR_HEIGHT = 56.0f;
    constexpr float TOUCH_ICON_BUTTON_WIDTH = 100.0f;
    constexpr float TOUCH_SIZE_COLUMN_WIDTH = 100.0f;
    constexpr float TOUCH_DATE_COLUMN_WIDTH = 150.0f;
    constexpr float TOUCH_CONFIRM_ICON_SIZE = 48.0f;
    constexpr float TOUCH_DRIVES_COMBO_WIDTH = 130.0f;
    constexpr float TOUCH_SORT_COMBO_WIDTH = 100.0f;
}

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
    float rowHeight = BaseSize::ROW_HEIGHT;
    float buttonHeight = BaseSize::BUTTON_HEIGHT;
    float buttonWidth = BaseSize::BUTTON_WIDTH;
    float inputHeight = BaseSize::INPUT_HEIGHT;
    float pathBarHeight = BaseSize::PATH_BAR_HEIGHT;
    float iconSize = BaseSize::ICON_SIZE;
    float fontSize = BaseSize::FONT_SIZE;
    float scrollbarWidth = BaseSize::SCROLLBAR_WIDTH;
    float grabMinSize = BaseSize::GRAB_MIN_SIZE;

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
        config.rowHeight = BaseSize::TOUCH_ROW_HEIGHT;
        config.buttonHeight = BaseSize::TOUCH_BUTTON_HEIGHT;
        config.buttonWidth = BaseSize::TOUCH_BUTTON_WIDTH;
        config.inputHeight = BaseSize::TOUCH_INPUT_HEIGHT;
        config.pathBarHeight = BaseSize::TOUCH_PATH_BAR_HEIGHT;
        config.iconSize = BaseSize::TOUCH_ICON_SIZE;
        config.fontSize = BaseSize::TOUCH_FONT_SIZE;
        config.scrollbarWidth = BaseSize::TOUCH_SCROLLBAR_WIDTH;
        config.grabMinSize = BaseSize::TOUCH_GRAB_MIN_SIZE;
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
