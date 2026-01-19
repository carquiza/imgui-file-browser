// ConfirmationDialog.hpp
// Generic ImGui-based confirmation dialog with touch support
// Part of ImFileBrowser standalone library

#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <functional>

#ifdef IMFILEBROWSER_USE_SIGSLOT
#include <sigslot/signal.hpp>
#endif

namespace ImFileBrowser {

/**
 * @brief Configuration for confirmation dialog
 */
struct ConfirmationConfig {
    std::string title = "Confirm";
    std::string message = "";
    std::string detailMessage = "";         // Optional secondary message
    DialogButton buttons = DialogButton::OkCancel;
    DialogButton defaultButton = DialogButton::Ok;  // Focused by default
    DialogIcon icon = DialogIcon::None;
    bool touchMode = false;
    float minWidth = 300.0f;
    float maxWidth = 500.0f;
    float scale = 1.0f;                     // UI scale factor (dpi * user scale)
};

/**
 * @brief Touch-friendly confirmation dialog
 *
 * Provides a generic modal dialog for confirmations, warnings,
 * and simple user prompts. Supports various button configurations
 * and works well with both mouse and touch input.
 *
 * Usage:
 * @code
 * ConfirmationDialog dialog;
 *
 * // Show save changes prompt
 * ConfirmationConfig config;
 * config.title = "Unsaved Changes";
 * config.message = "Do you want to save changes to 'document.jml'?";
 * config.buttons = DialogButton::SaveDontSaveCancel;
 * config.icon = DialogIcon::Warning;
 * dialog.Show(config);
 *
 * // In render loop
 * auto result = dialog.Render();
 * if (result == DialogResult::Save) {
 *     // Save document...
 * } else if (result == DialogResult::DontSave) {
 *     // Discard changes...
 * } else if (result == DialogResult::Cancel) {
 *     // Return to editing...
 * }
 * @endcode
 */
class ConfirmationDialog {
public:
    ConfirmationDialog() = default;
    ~ConfirmationDialog() = default;

    // Non-copyable
    ConfirmationDialog(const ConfirmationDialog&) = delete;
    ConfirmationDialog& operator=(const ConfirmationDialog&) = delete;

    // ==================== Dialog Control ====================

    /**
     * @brief Show the dialog
     * @param config Dialog configuration
     */
    void Show(const ConfirmationConfig& config);

    /**
     * @brief Hide the dialog without result
     */
    void Hide();

    /**
     * @brief Check if dialog is currently shown
     */
    bool IsShown() const { return m_isShown; }

    /**
     * @brief Render the dialog
     * @return Dialog result (None while open, button result when closed)
     */
    DialogResult Render();

    /**
     * @brief Get the last result
     */
    DialogResult GetResult() const { return m_result; }

    /**
     * @brief Set the UI scale factor
     * @param scale Combined scale (dpiScale * userScale), must be > 0
     */
    void SetScale(float scale);

    /**
     * @brief Get the current UI scale factor
     */
    float GetScale() const { return m_scale; }

    // ==================== Signals (optional, requires sigslot) ====================

#ifdef IMFILEBROWSER_USE_SIGSLOT
    /// Emitted when a button is clicked with the result
    sigslot::signal<DialogResult> onResult;
#endif

    // ==================== Callback-based API (always available) ====================

    using ResultCallback = std::function<void(DialogResult)>;

    void SetOnResult(ResultCallback callback) { m_onResult = std::move(callback); }

    // ==================== Convenience Static Methods ====================

    /**
     * @brief Show a simple message box (non-blocking)
     * @param title Dialog title
     * @param message Message to display
     * @param buttons Button configuration
     * @param icon Icon to display
     * @return None (use Render() to get result)
     *
     * @note This is a convenience for simple cases. For blocking
     *       dialogs, use the instance methods instead.
     */
    static DialogResult ShowMessage(
        const std::string& title,
        const std::string& message,
        DialogButton buttons = DialogButton::Ok,
        DialogIcon icon = DialogIcon::Info);

private:
    void RenderIcon();
    void RenderMessage();
    void RenderButtons();

    float GetButtonWidth() const;
    const char* GetButtonLabel(DialogButton button) const;
    void HandleButtonClick(DialogButton button);
    void NotifyResult(DialogResult result);

    bool m_isShown = false;
    ConfirmationConfig m_config;
    DialogResult m_result = DialogResult::None;
    bool m_shouldOpen = false;  // Flag to open popup on next frame

    // Scale factor
    float m_scale = 1.0f;

    // Sizing (computed based on touch mode and scale)
    float m_buttonHeight = 32.0f;
    float m_buttonWidth = 80.0f;
    float m_iconSize = 32.0f;
    float m_fontSize = 14.0f;

    // Callback
    ResultCallback m_onResult;
};

// ============================================================================
// Inline convenience functions
// ============================================================================

/**
 * @brief Create a save changes confirmation config
 * @param filename Name of file with unsaved changes
 * @param touchMode Use touch-optimized sizing
 * @return Config ready to pass to Show()
 */
inline ConfirmationConfig MakeSaveChangesConfig(
    const std::string& filename = "",
    bool touchMode = false)
{
    ConfirmationConfig config;
    config.title = "Unsaved Changes";
    if (filename.empty()) {
        config.message = "Do you want to save changes?";
    } else {
        config.message = "Do you want to save changes to '" + filename + "'?";
    }
    config.buttons = DialogButton::SaveDontSaveCancel;
    config.defaultButton = DialogButton::Save;
    config.icon = DialogIcon::Warning;
    config.touchMode = touchMode;
    return config;
}

/**
 * @brief Create an overwrite confirmation config
 * @param filename Name of file to overwrite
 * @param touchMode Use touch-optimized sizing
 * @return Config ready to pass to Show()
 */
inline ConfirmationConfig MakeOverwriteConfig(
    const std::string& filename,
    bool touchMode = false)
{
    ConfirmationConfig config;
    config.title = "Confirm Overwrite";
    config.message = "'" + filename + "' already exists.";
    config.detailMessage = "Do you want to replace it?";
    config.buttons = DialogButton::YesNo;
    config.defaultButton = DialogButton::No;
    config.icon = DialogIcon::Warning;
    config.touchMode = touchMode;
    return config;
}

/**
 * @brief Create an error message config
 * @param title Error title
 * @param message Error message
 * @param touchMode Use touch-optimized sizing
 * @return Config ready to pass to Show()
 */
inline ConfirmationConfig MakeErrorConfig(
    const std::string& title,
    const std::string& message,
    bool touchMode = false)
{
    ConfirmationConfig config;
    config.title = title;
    config.message = message;
    config.buttons = DialogButton::Ok;
    config.icon = DialogIcon::Error;
    config.touchMode = touchMode;
    return config;
}

} // namespace ImFileBrowser
