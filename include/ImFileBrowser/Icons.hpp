// Icons.hpp
// Icon system for ImFileBrowser library
// Standalone ImGui-based file browser

#pragma once

namespace ImFileBrowser {

/**
 * @brief Icon set for file browser UI
 *
 * Provides icon strings for various UI elements. By default uses
 * text fallbacks that work without any special fonts. Call SetIcons()
 * with FontAwesome() preset to enable proper icons when FontAwesome
 * font is loaded.
 */
struct IconSet {
    // Navigation
    const char* arrowUp = "^";
    const char* home = "H";
    const char* refresh = "R";
    const char* newFolder = "+";
    const char* drives = "D";

    // Files and folders
    const char* folder = "[D]";
    const char* file = "[F]";
    const char* hdd = "HD";

    // Actions
    const char* save = "S";
    const char* close = "X";
    const char* check = "v";

    // Sorting
    const char* sortAlphaDown = "A-Z";   // Name ascending
    const char* sortAlphaUp = "Z-A";     // Name descending
    const char* sortAmountDown = "9-1";  // Size/date descending
    const char* sortAmountUp = "1-9";    // Size/date ascending

    // Dialog icons
    const char* info = "i";
    const char* warning = "!";
    const char* errorIcon = "X";  // Named errorIcon to avoid conflict with macro
    const char* question = "?";

    /**
     * @brief Get text fallback icon set (works without special fonts)
     */
    static IconSet TextFallback() {
        return IconSet();  // Default values are text fallbacks
    }

    /**
     * @brief Get FontAwesome icon set
     *
     * These are FontAwesome Free solid icons. To use them:
     * 1. Download FontAwesome Free from https://fontawesome.com/download
     * 2. Load fa-solid-900.ttf with ImGui font merging
     * 3. Call SetIcons(IconSet::FontAwesome())
     */
    static IconSet FontAwesome() {
        IconSet icons;

        // Navigation
        icons.arrowUp = "\xEF\x81\xA2";     // U+F062 - arrow-up
        icons.home = "\xEF\x80\x95";        // U+F015 - home
        icons.refresh = "\xEF\x80\xA1";     // U+F021 - sync
        icons.newFolder = "\xEF\x99\x9E";   // U+F65E - folder-plus
        icons.drives = "\xEF\x82\xA0";      // U+F0A0 - hdd (for drives dropdown)

        // Files and folders
        icons.folder = "\xEF\x81\xBB";      // U+F07B - folder
        icons.file = "\xEF\x85\x9B";        // U+F15B - file
        icons.hdd = "\xEF\x82\xA0";         // U+F0A0 - hdd

        // Actions
        icons.save = "\xEF\x83\x87";        // U+F0C7 - save
        icons.close = "\xEF\x80\x8D";       // U+F00D - times
        icons.check = "\xEF\x80\x8C";       // U+F00C - check

        // Sorting
        icons.sortAlphaDown = "\xEF\x85\x9D";  // U+F15D - sort-alpha-down (A-Z)
        icons.sortAlphaUp = "\xEF\x85\x9E";    // U+F15E - sort-alpha-up (Z-A)
        icons.sortAmountDown = "\xEF\x85\xA0";  // U+F160 - sort-amount-down
        icons.sortAmountUp = "\xEF\x85\xA1";    // U+F161 - sort-amount-up

        // Dialog icons
        icons.info = "\xEF\x81\x9A";        // U+F05A - info-circle
        icons.warning = "\xEF\x81\xB1";     // U+F071 - exclamation-triangle
        icons.errorIcon = "\xEF\x81\x97";   // U+F057 - times-circle
        icons.question = "\xEF\x81\x99";    // U+F059 - question-circle

        return icons;
    }
};

/**
 * @brief Get the global icon set
 * @return Reference to current icon set
 */
IconSet& GetIcons();

/**
 * @brief Set the global icon set
 * @param icons New icon set to use
 */
void SetIcons(const IconSet& icons);

} // namespace ImFileBrowser
