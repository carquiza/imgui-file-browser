// Types.hpp
// Type definitions for ImFileBrowser library
// Standalone ImGui-based file browser

#pragma once

namespace ImFileBrowser {

/**
 * @brief File browser dialog mode
 */
enum class Mode {
    Open,           ///< Open existing file
    Save,           ///< Save to file (can create new)
    SelectFolder    ///< Select a folder
};

/**
 * @brief Result of file browser dialog
 */
enum class Result {
    None,           ///< Dialog still open
    Selected,       ///< File/folder selected (OK pressed)
    Cancelled       ///< Dialog cancelled
};

/**
 * @brief Sort order for file listings
 */
enum class SortOrder {
    NameAsc,
    NameDesc,
    SizeAsc,
    SizeDesc,
    DateAsc,
    DateDesc
};

/**
 * @brief Standard button types for confirmation dialogs
 */
enum class DialogButton {
    None = 0,
    Ok = 1,
    Cancel = 2,
    Yes = 4,
    No = 8,
    Save = 16,
    DontSave = 32,
    Retry = 64,

    // Common combinations
    OkCancel = Ok | Cancel,
    YesNo = Yes | No,
    YesNoCancel = Yes | No | Cancel,
    SaveDontSaveCancel = Save | DontSave | Cancel
};

// Enable bitwise operations
inline DialogButton operator|(DialogButton a, DialogButton b) {
    return static_cast<DialogButton>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool HasButton(DialogButton buttons, DialogButton test) {
    return (static_cast<int>(buttons) & static_cast<int>(test)) != 0;
}

/**
 * @brief Result of confirmation dialog
 */
enum class DialogResult {
    None,       ///< Dialog still open or not shown
    Ok,
    Cancel,
    Yes,
    No,
    Save,
    DontSave,
    Retry
};

/**
 * @brief Icon type for confirmation dialog
 */
enum class DialogIcon {
    None,
    Info,
    Warning,
    Error,
    Question
};

} // namespace ImFileBrowser
