// ConfirmationDialog.cpp
// Generic ImGui-based confirmation dialog with touch support
// Part of ImFileBrowser standalone library

#include "ImFileBrowser/ConfirmationDialog.hpp"
#include "ImFileBrowser/Config.hpp"
#include "imgui.h"
#include <algorithm>

namespace ImFileBrowser {

void ConfirmationDialog::OnScaleChanged() {
    // Update sizes with new scale
    const float scale = GetScale();
    if (m_config.touchMode) {
        m_buttonHeight = BaseSize::TOUCH_BUTTON_HEIGHT * scale;
        m_buttonWidth = BaseSize::TOUCH_BUTTON_WIDTH * scale;
        m_iconSize = BaseSize::TOUCH_CONFIRM_ICON_SIZE * scale;
        m_fontSize = BaseSize::TOUCH_FONT_SIZE * scale;
    } else {
        m_buttonHeight = BaseSize::BUTTON_HEIGHT * scale;
        m_buttonWidth = BaseSize::BUTTON_WIDTH * scale;
        m_iconSize = BaseSize::CONFIRM_ICON_SIZE * scale;
        m_fontSize = BaseSize::FONT_SIZE * scale;
    }
}

void ConfirmationDialog::Show(const ConfirmationConfig& config) {
    m_config = config;
    m_isShown = true;
    m_result = DialogResult::None;
    m_shouldOpen = true;

    // Apply scale from config (0 = keep current scale set via SetScale())
    if (config.scale > 0.0f) {
        SetScale(config.scale);
    }

    // Update sizing based on mode and scale
    OnScaleChanged(); // Ensure sizes are updated for the new mode if scale didn't change
}

void ConfirmationDialog::Hide() {
    m_isShown = false;
    m_result = DialogResult::None;
}

DialogResult ConfirmationDialog::Render() {
    if (!m_isShown) {
        return DialogResult::None;
    }

    if (HasScaleChanged()) {
        AcknowledgeScaleChange();
    }

    // Open popup on first frame
    if (m_shouldOpen) {
        ImGui::OpenPopup(m_config.title.c_str());
        m_shouldOpen = false;
    }

    // Center the popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // Calculate width based on content (with scaled constraints)
    float scaledMinWidth = m_config.minWidth * GetScale();
    float scaledMaxWidth = m_config.maxWidth * GetScale();
    float padding = 40.0f * GetScale();

    float contentWidth = ImGui::CalcTextSize(m_config.message.c_str()).x + padding;
    if (!m_config.detailMessage.empty()) {
        float detailWidth = ImGui::CalcTextSize(m_config.detailMessage.c_str()).x + padding;
        contentWidth = (std::max)(contentWidth, detailWidth);
    }
    contentWidth = (std::max)(contentWidth, scaledMinWidth);
    contentWidth = (std::min)(contentWidth, scaledMaxWidth);

    // Add icon width if present (with scaled spacing)
    if (m_config.icon != DialogIcon::None) {
        contentWidth += m_iconSize + 16.0f * GetScale();
    }

    ImGui::SetNextWindowSize(ImVec2(contentWidth, 0), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoMove;

    if (ImGui::BeginPopupModal(m_config.title.c_str(), nullptr, flags)) {
        // Content area with icon and message
        if (m_config.icon != DialogIcon::None) {
            ImGui::BeginGroup();
            RenderIcon();
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            RenderMessage();
            ImGui::EndGroup();
        } else {
            RenderMessage();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        RenderButtons();

        // Handle escape key
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if (HasButton(m_config.buttons, DialogButton::Cancel)) {
                HandleButtonClick(DialogButton::Cancel);
            } else if (HasButton(m_config.buttons, DialogButton::No)) {
                HandleButtonClick(DialogButton::No);
            }
        }

        // Handle enter key (activates default button)
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
            HandleButtonClick(m_config.defaultButton);
        }

        ImGui::EndPopup();
    } else {
        // Popup was closed externally
        m_isShown = false;
    }

    return m_result;
}

void ConfirmationDialog::RenderIcon() {
    ImVec4 iconColor;
    const char* iconText = "";

    switch (m_config.icon) {
        case DialogIcon::Info:
            iconColor = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);  // Blue
            iconText = "i";
            break;
        case DialogIcon::Warning:
            iconColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);  // Yellow
            iconText = "!";
            break;
        case DialogIcon::Error:
            iconColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red
            iconText = "X";
            break;
        case DialogIcon::Question:
            iconColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);  // Green
            iconText = "?";
            break;
        default:
            return;
    }

    // Draw icon as colored text in a circle
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float radius = m_iconSize / 2.0f;
    ImVec2 centerPos = ImVec2(pos.x + radius, pos.y + radius);

    // Draw circle background
    drawList->AddCircleFilled(centerPos, radius, ImGui::ColorConvertFloat4ToU32(iconColor));

    // Draw icon text centered
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImVec2 textSize = ImGui::CalcTextSize(iconText);
    ImGui::SetCursorScreenPos(ImVec2(centerPos.x - textSize.x / 2, centerPos.y - textSize.y / 2));
    ImGui::Text("%s", iconText);
    ImGui::PopStyleColor();

    // Reserve space
    ImGui::Dummy(ImVec2(m_iconSize, m_iconSize));
}

void ConfirmationDialog::RenderMessage() {
    float scaledMaxWidth = m_config.maxWidth * GetScale();
    ImGui::PushTextWrapPos(scaledMaxWidth - 80.0f * GetScale());

    ImGui::Text("%s", m_config.message.c_str());

    if (!m_config.detailMessage.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("%s", m_config.detailMessage.c_str());
    }

    ImGui::PopTextWrapPos();
}

void ConfirmationDialog::RenderButtons() {
    // Count buttons to calculate spacing
    int buttonCount = 0;
    if (HasButton(m_config.buttons, DialogButton::Ok)) buttonCount++;
    if (HasButton(m_config.buttons, DialogButton::Cancel)) buttonCount++;
    if (HasButton(m_config.buttons, DialogButton::Yes)) buttonCount++;
    if (HasButton(m_config.buttons, DialogButton::No)) buttonCount++;
    if (HasButton(m_config.buttons, DialogButton::Save)) buttonCount++;
    if (HasButton(m_config.buttons, DialogButton::DontSave)) buttonCount++;
    if (HasButton(m_config.buttons, DialogButton::Retry)) buttonCount++;

    // Calculate right alignment (scaled spacing)
    float buttonWidth = GetButtonWidth();
    float spacing = BaseSize::BUTTON_SPACING * GetScale();
    float totalWidth = buttonCount * buttonWidth + (buttonCount - 1) * spacing;
    float startX = ImGui::GetContentRegionAvail().x - totalWidth;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + startX);

    // Render buttons in order
    bool first = true;

    auto RenderButton = [&](DialogButton btn) {
        if (!HasButton(m_config.buttons, btn)) return;

        if (!first) {
            ImGui::SameLine(0, spacing);
        }
        first = false;

        bool isDefault = (btn == m_config.defaultButton);

        if (isDefault) {
            // Highlight default button
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        if (ImGui::Button(GetButtonLabel(btn), ImVec2(buttonWidth, m_buttonHeight))) {
            HandleButtonClick(btn);
        }

        if (isDefault) {
            ImGui::PopStyleColor();
            // Focus default button
            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere(-1);
            }
        }
    };

    // Render in logical order (positive actions first, cancel last)
    RenderButton(DialogButton::Save);
    RenderButton(DialogButton::Ok);
    RenderButton(DialogButton::Yes);
    RenderButton(DialogButton::Retry);
    RenderButton(DialogButton::No);
    RenderButton(DialogButton::DontSave);
    RenderButton(DialogButton::Cancel);
}

float ConfirmationDialog::GetButtonWidth() const {
    // Calculate based on longest button label (with scaled padding)
    float maxWidth = m_buttonWidth;
    float buttonPadding = 20.0f * GetScale();

    auto CheckLabel = [&](DialogButton btn) {
        if (HasButton(m_config.buttons, btn)) {
            float labelWidth = ImGui::CalcTextSize(GetButtonLabel(btn)).x + buttonPadding;
            maxWidth = (std::max)(maxWidth, labelWidth);
        }
    };

    CheckLabel(DialogButton::Ok);
    CheckLabel(DialogButton::Cancel);
    CheckLabel(DialogButton::Yes);
    CheckLabel(DialogButton::No);
    CheckLabel(DialogButton::Save);
    CheckLabel(DialogButton::DontSave);
    CheckLabel(DialogButton::Retry);

    return maxWidth;
}

const char* ConfirmationDialog::GetButtonLabel(DialogButton button) const {
    switch (button) {
        case DialogButton::Ok: return "OK";
        case DialogButton::Cancel: return "Cancel";
        case DialogButton::Yes: return "Yes";
        case DialogButton::No: return "No";
        case DialogButton::Save: return "Save";
        case DialogButton::DontSave: return "Don't Save";
        case DialogButton::Retry: return "Retry";
        default: return "";
    }
}

void ConfirmationDialog::HandleButtonClick(DialogButton button) {
    switch (button) {
        case DialogButton::Ok: m_result = DialogResult::Ok; break;
        case DialogButton::Cancel: m_result = DialogResult::Cancel; break;
        case DialogButton::Yes: m_result = DialogResult::Yes; break;
        case DialogButton::No: m_result = DialogResult::No; break;
        case DialogButton::Save: m_result = DialogResult::Save; break;
        case DialogButton::DontSave: m_result = DialogResult::DontSave; break;
        case DialogButton::Retry: m_result = DialogResult::Retry; break;
        default: m_result = DialogResult::None; break;
    }

    if (m_result != DialogResult::None) {
        m_isShown = false;
        ImGui::CloseCurrentPopup();
        NotifyResult(m_result);
    }
}

void ConfirmationDialog::NotifyResult(DialogResult result) {
#ifdef IMFILEBROWSER_USE_SIGSLOT
    onResult(result);
#endif
    if (m_onResult) {
        m_onResult(result);
    }
}

DialogResult ConfirmationDialog::ShowMessage(
    const std::string& title,
    const std::string& message,
    DialogButton buttons,
    DialogIcon icon)
{
    // This is a simplified static version
    // For full functionality, use the instance methods
    ConfirmationConfig config;
    config.title = title;
    config.message = message;
    config.buttons = buttons;
    config.icon = icon;

    // Note: This doesn't actually block - caller needs to handle in render loop
    // This is just a convenience for setting up the config
    return DialogResult::None;
}

} // namespace ImFileBrowser
