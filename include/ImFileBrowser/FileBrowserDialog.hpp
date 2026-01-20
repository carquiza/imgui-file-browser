// FileBrowserDialog.hpp
// Custom ImGui-based file browser with touch support
// Part of ImFileBrowser standalone library

#pragma once

#include "Types.hpp"
#include "FileFilter.hpp"
#include "FileSystemHelper.hpp"
#include <ImGuiScaling/ImGuiScaling.hpp>
#include <string>
#include <vector>
#include <functional>
#include <optional>

#ifdef IMFILEBROWSER_USE_SIGSLOT
#include <sigslot/signal.hpp>
#endif

namespace ImFileBrowser {

/**
 * @brief Configuration for file browser dialog
 */
struct DialogConfig {
    Mode mode = Mode::Open;
    std::string title = "Open File";
    std::string initialPath = "";           // Starting directory
    std::string initialFilename = "";       // Initial filename (for Save mode)
    std::vector<FileFilter> filters;        // File type filters
    int selectedFilterIndex = 0;            // Default filter
    bool showHiddenFiles = false;           // Show hidden files/folders
    bool allowCreateFolder = true;          // Show "New Folder" button
    bool touchMode = false;                 // Use touch-optimized sizing
    float scale = 1.0f;                     // UI scale factor (dpi * user scale)
};

/**
 * @brief Touch-friendly file browser dialog
 *
 * Provides a custom ImGui-based file browser that works well with
 * both mouse and touch input. Features large touch targets, clear
 * visual feedback, and responsive layout.
 *
 * Usage:
 * @code
 * FileBrowserDialog browser;
 *
 * // Configure and show
 * DialogConfig config;
 * config.mode = Mode::Save;
 * config.title = "Save Document";
 * config.filters = { {"JML Files", "*.jml"}, {"All Files", "*.*"} };
 * browser.Open(config);
 *
 * // In render loop
 * if (browser.IsOpen()) {
 *     auto result = browser.Render();
 *     if (result == Result::Selected) {
 *         std::string path = browser.GetSelectedPath();
 *         // Use path...
 *     }
 * }
 * @endcode
 */
class FileBrowserDialog : public ImGuiScaling::Scalable {
public:
    FileBrowserDialog();
    ~FileBrowserDialog() = default;

    // Non-copyable
    FileBrowserDialog(const FileBrowserDialog&) = delete;
    FileBrowserDialog& operator=(const FileBrowserDialog&) = delete;

    // ==================== Dialog Control ====================

    /**
     * @brief Open the file browser dialog
     * @param config Dialog configuration
     */
    void Open(const DialogConfig& config);

    /**
     * @brief Close the dialog without selection
     */
    void Close();

    /**
     * @brief Check if dialog is currently open
     */
    bool IsOpen() const { return m_isOpen; }

    /**
     * @brief Render the dialog
     * @return Dialog result (None while open, Selected or Cancelled when closed)
     */
    Result Render();

    // ==================== Results ====================

    /**
     * @brief Get the selected file/folder path
     * @return Full path to selected item, or empty if none
     */
    const std::string& GetSelectedPath() const { return m_selectedPath; }

    /**
     * @brief Get the selected filter index
     * @return Index of selected filter in config.filters
     */
    int GetSelectedFilterIndex() const { return m_selectedFilterIndex; }

    // ==================== Signals (optional, requires sigslot) ====================

#ifdef IMFILEBROWSER_USE_SIGSLOT
    /// Emitted when a file is selected (double-click or OK)
    sigslot::signal<const std::string&> onFileSelected;

    /// Emitted when dialog is cancelled
    sigslot::signal<> onCancelled;
#endif

    // ==================== Callback-based API (always available) ====================

    using FileSelectedCallback = std::function<void(const std::string&)>;
    using CancelledCallback = std::function<void()>;

    void SetOnFileSelected(FileSelectedCallback callback) { m_onFileSelected = std::move(callback); }
    void SetOnCancelled(CancelledCallback callback) { m_onCancelled = std::move(callback); }

protected:
    // ==================== Scaling ====================

    void OnScaleChanged() override;

private:
    // ==================== Rendering ====================

    void RenderToolbar();
    void RenderPathBar();
    void RenderFileList();
    void RenderFilenameInput();
    void RenderFilterSelector();
    void RenderButtons();
    void RenderNewFolderPopup();
    void RenderOverwriteConfirmPopup();

    // ==================== Navigation ====================

    void NavigateTo(const std::string& path);
    void NavigateUp();
    void NavigateToParent();
    void RefreshDirectory();
    void SelectEntry(int index);
    void ActivateEntry(int index);  // Double-click or Enter

    // ==================== Helpers ====================

    std::vector<std::string> GetCurrentExtensions() const;
    bool IsValidSelection() const;
    std::string BuildFullPath() const;
    void UpdateSizing();
    void NotifyFileSelected(const std::string& path);
    void NotifyCancelled();
    int FindMatchingEntryIndex(const char* prefix) const;

    // ==================== State ====================

    bool m_isOpen = false;
    DialogConfig m_config;
    Result m_result = Result::None;

    // Current state
    std::string m_currentPath;
    std::vector<FileEntry> m_entries;
    int m_selectedIndex = -1;
    std::string m_selectedPath;
    int m_selectedFilterIndex = 0;
    SortOrder m_sortOrder = SortOrder::NameAsc;

    // Input state
    char m_filenameBuffer[256] = {0};
    char m_newFolderBuffer[256] = {0};
    bool m_filenameInputActive = false;  // Track if filename input has focus

    // Popup state
    bool m_showNewFolderPopup = false;
    bool m_showOverwriteConfirm = false;
    std::string m_overwritePath;

    // Deferred action (to avoid modifying entries during table iteration)
    int m_pendingActivateIndex = -1;
    int m_pendingScrollToIndex = -1;  // For incremental search scrolling

    // Drives/roots (cached)
    std::vector<std::string> m_drives;

    // Sizing (computed based on touch mode and scale)
    float m_rowHeight = 32.0f;
    float m_buttonHeight = 32.0f;
    float m_buttonWidth = 80.0f;
    float m_iconSize = 20.0f;
    float m_fontSize = 14.0f;
    float m_dialogWidth = 600.0f;
    float m_dialogHeight = 450.0f;
    float m_pathBarHeight = 36.0f;
    float m_inputHeight = 32.0f;

    // Callbacks
    FileSelectedCallback m_onFileSelected;
    CancelledCallback m_onCancelled;
};

} // namespace ImFileBrowser
