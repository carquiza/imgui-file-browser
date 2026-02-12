// FileBrowserDialog.cpp
// Custom ImGui-based file browser with touch support
// Part of ImFileBrowser standalone library

#include "ImFileBrowser/FileBrowserDialog.hpp"
#include "ImFileBrowser/Config.hpp"
#include "ImFileBrowser/Icons.hpp"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <cstdio>

namespace ImFileBrowser {

FileBrowserDialog::FileBrowserDialog() {
    m_drives = FileSystemHelper::GetDrives();
}

void FileBrowserDialog::OnScaleChanged() {
    UpdateSizing();
}

void FileBrowserDialog::Open(const DialogConfig& config) {
    m_config = config;
    m_isOpen = true;
    m_result = Result::None;
    m_selectedIndex = -1;
    m_selectedPath.clear();
    m_selectedFilterIndex = config.selectedFilterIndex;
    m_sortOrder = SortOrder::NameAsc;
    m_showNewFolderPopup = false;
    m_showOverwriteConfirm = false;
    m_pendingActivateIndex = -1;
    m_pendingScrollToIndex = -1;

    // Apply scale from config (0 = keep current scale set via SetScale())
    if (config.scale > 0.0f) {
        SetScale(config.scale);
    }

    // Set initial path (priority: config.initialPath > persisted lastPath > documents)
    if (!config.initialPath.empty() && FileSystemHelper::IsDirectory(config.initialPath)) {
        m_currentPath = config.initialPath;
    } else {
        // Try persisted last path (safely - it may no longer exist)
        const std::string& lastPath = GetLastPath();
        if (!lastPath.empty() && FileSystemHelper::IsDirectory(lastPath)) {
            m_currentPath = lastPath;
        } else {
            m_currentPath = FileSystemHelper::GetDocumentsDirectory();
        }
    }

    // Set initial filename
    if (!config.initialFilename.empty()) {
        strncpy(m_filenameBuffer, config.initialFilename.c_str(), sizeof(m_filenameBuffer) - 1);
        m_filenameBuffer[sizeof(m_filenameBuffer) - 1] = '\0';
    } else {
        m_filenameBuffer[0] = '\0';
    }

    m_newFolderBuffer[0] = '\0';
    m_filenameInputActive = false;

    // Refresh drives
    m_drives = FileSystemHelper::GetDrives();

    // Update sizing based on mode
    UpdateSizing();

    // Load directory contents
    RefreshDirectory();
}

void FileBrowserDialog::Close() {
    m_isOpen = false;
    m_result = Result::Cancelled;
    NotifyCancelled();
}

void FileBrowserDialog::UpdateSizing() {
    // Use base constants and apply scale
    const float scale = GetScale();

    if (m_config.touchMode) {
        // Touch mode: use touch-optimized sizes, scaled
        m_rowHeight = BaseSize::TOUCH_ROW_HEIGHT * scale;
        m_buttonHeight = BaseSize::TOUCH_BUTTON_HEIGHT * scale;
        m_buttonWidth = BaseSize::TOUCH_BUTTON_WIDTH * scale;
        m_iconSize = BaseSize::TOUCH_ICON_SIZE * scale;
        m_fontSize = BaseSize::TOUCH_FONT_SIZE * scale;
        m_pathBarHeight = BaseSize::TOUCH_PATH_BAR_HEIGHT * scale;
        m_inputHeight = BaseSize::TOUCH_INPUT_HEIGHT * scale;
        // Touch mode fills screen, no scaling on dialog size
        m_dialogWidth = ImGui::GetIO().DisplaySize.x * 0.9f;
        m_dialogHeight = ImGui::GetIO().DisplaySize.y * 0.85f;
    } else {
        // Desktop mode: use desktop sizes, scaled
        m_rowHeight = BaseSize::ROW_HEIGHT * scale;
        m_buttonHeight = BaseSize::BUTTON_HEIGHT * scale;
        m_buttonWidth = BaseSize::BUTTON_WIDTH * scale;
        m_iconSize = BaseSize::ICON_SIZE * scale;
        m_fontSize = BaseSize::FONT_SIZE * scale;
        m_pathBarHeight = BaseSize::PATH_BAR_HEIGHT * scale;
        m_inputHeight = BaseSize::INPUT_HEIGHT * scale;
        m_dialogWidth = BaseSize::DIALOG_WIDTH * scale;
        m_dialogHeight = BaseSize::DIALOG_HEIGHT * scale;
    }
}

Result FileBrowserDialog::Render() {
    if (!m_isOpen) {
        return m_result;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Check if scale changed since last frame
    bool scaleChanged = HasScaleChanged();
    if (scaleChanged) {
        // UpdateSizing() is called automatically via OnScaleChanged()
        AcknowledgeScaleChange();
    }

    // Touch mode: fullscreen dialog for maximum usability
    // Desktop mode: centered dialog with scaled size and constraints
    if (m_config.touchMode) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
    } else {
        // Apply window size constraints (scaled) - always apply
        ImVec2 minSize(BaseSize::DIALOG_MIN_WIDTH * GetScale(), BaseSize::DIALOG_MIN_HEIGHT * GetScale());
        ImGui::SetNextWindowSizeConstraints(minSize, ImVec2(FLT_MAX, FLT_MAX));

        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // Force resize when scale changes, otherwise only on first appearance
        if (scaleChanged) {
            ImGui::SetNextWindowSize(ImVec2(m_dialogWidth, m_dialogHeight), ImGuiCond_Always);
        } else {
            ImGui::SetNextWindowSize(ImVec2(m_dialogWidth, m_dialogHeight), ImGuiCond_Appearing);
        }
    }

    // Window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (m_config.touchMode) {
        // Fullscreen: no moving, no resizing, no scrolling the outer window
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse;
    }
    if (m_showNewFolderPopup || m_showOverwriteConfirm) {
        flags |= ImGuiWindowFlags_NoInputs;  // Block input while popup is open
    }

    bool windowOpen = true;
    if (ImGui::Begin(m_config.title.c_str(), &windowOpen, flags)) {
        if (!windowOpen) {
            Close();
        } else {
            RenderToolbar();
            RenderPathBar();
            RenderFileList();

            if (m_config.mode != Mode::SelectFolder) {
                RenderFilenameAndFilter();
            }

            RenderButtons();

            // Popups
            RenderNewFolderPopup();
            RenderOverwriteConfirmPopup();
        }
    }
    ImGui::End();

    return m_result;
}

void FileBrowserDialog::RenderToolbar() {
    const auto& icons = GetIcons();

    // Touch mode: use wider buttons with icon + text for clarity
    // Desktop mode: use compact icon-only buttons (all scaled)
    float buttonHeight = m_buttonHeight;

    // Build button labels with icons
    char backLabel[32], homeLabel[32], refreshLabel[32], newFolderLabel[32];
    if (m_config.touchMode) {
        snprintf(backLabel, sizeof(backLabel), "%s Back", icons.arrowUp);
        snprintf(homeLabel, sizeof(homeLabel), "%s Home", icons.home);
        snprintf(refreshLabel, sizeof(refreshLabel), "%s Refresh", icons.refresh);
        snprintf(newFolderLabel, sizeof(newFolderLabel), "%s New", icons.newFolder);
    } else {
        snprintf(backLabel, sizeof(backLabel), "%s", icons.arrowUp);
        snprintf(homeLabel, sizeof(homeLabel), "%s", icons.home);
        snprintf(refreshLabel, sizeof(refreshLabel), "%s", icons.refresh);
        snprintf(newFolderLabel, sizeof(newFolderLabel), "%s", icons.newFolder);
    }

    // Calculate button width: in touch mode, auto-size from text to avoid truncation;
    // in desktop mode, use fixed icon-only width
    float iconButtonWidth;
    if (m_config.touchMode) {
        const auto& style = ImGui::GetStyle();
        float maxTextWidth = ImGui::CalcTextSize(backLabel).x;
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(homeLabel).x);
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(refreshLabel).x);
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(newFolderLabel).x);
        iconButtonWidth = maxTextWidth + style.FramePadding.x * 2.0f;
    } else {
        iconButtonWidth = BaseSize::ICON_BUTTON_WIDTH * GetScale();
    }

    // Back/Up button
    if (ImGui::Button(backLabel, ImVec2(iconButtonWidth, buttonHeight))) {
        NavigateUp();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Go to parent folder");
    }

    ImGui::SameLine();

    // Home button
    if (ImGui::Button(homeLabel, ImVec2(iconButtonWidth, buttonHeight))) {
        NavigateTo(FileSystemHelper::GetHomeDirectory());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Go to home folder");
    }

    ImGui::SameLine();

    // Drives dropdown with icon (scaled)
    char drivesLabel[32];
    snprintf(drivesLabel, sizeof(drivesLabel), "%s", icons.hdd);
    float drivesWidth = m_config.touchMode
        ? BaseSize::TOUCH_DRIVES_COMBO_WIDTH * GetScale()
        : BaseSize::DRIVES_COMBO_WIDTH * GetScale();
    ImGui::SetNextItemWidth(drivesWidth);
    if (ImGui::BeginCombo("##drives", drivesLabel)) {
        for (const auto& drive : m_drives) {
            // Show drive icon next to each drive letter
            char driveItem[64];
            snprintf(driveItem, sizeof(driveItem), "%s %s", icons.hdd, drive.c_str());
            if (ImGui::Selectable(driveItem)) {
                NavigateTo(drive);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // Refresh button
    if (ImGui::Button(refreshLabel, ImVec2(iconButtonWidth, buttonHeight))) {
        RefreshDirectory();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Refresh directory");
    }

    // New Folder button (if allowed)
    if (m_config.allowCreateFolder) {
        ImGui::SameLine();
        if (ImGui::Button(newFolderLabel, ImVec2(iconButtonWidth, buttonHeight))) {
            m_showNewFolderPopup = true;
            m_newFolderBuffer[0] = '\0';
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Create new folder");
        }
    }

    // Sort dropdown (right-aligned, auto-sized from labels)
    char sortLabels[6][64];
    snprintf(sortLabels[0], sizeof(sortLabels[0]), "Name %s", icons.sortAlphaDown);
    snprintf(sortLabels[1], sizeof(sortLabels[1]), "Name %s", icons.sortAlphaUp);
    snprintf(sortLabels[2], sizeof(sortLabels[2]), "Size %s", icons.sortAmountUp);
    snprintf(sortLabels[3], sizeof(sortLabels[3]), "Size %s", icons.sortAmountDown);
    snprintf(sortLabels[4], sizeof(sortLabels[4]), "Date %s", icons.sortAmountUp);
    snprintf(sortLabels[5], sizeof(sortLabels[5]), "Date %s", icons.sortAmountDown);

    // Auto-size from longest label + combo arrow
    float sortWidth = 0;
    for (int i = 0; i < 6; ++i) {
        float w = ImGui::CalcTextSize(sortLabels[i]).x;
        if (w > sortWidth) sortWidth = w;
    }
    sortWidth += ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 4;

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - sortWidth);
    ImGui::SetNextItemWidth(sortWidth);

    // Build items string for Combo (null-separated, double-null terminated)
    const char* sortItems[6] = { sortLabels[0], sortLabels[1], sortLabels[2], sortLabels[3], sortLabels[4], sortLabels[5] };
    int sortIndex = static_cast<int>(m_sortOrder);
    const char* currentLabel = sortItems[sortIndex];
    if (ImGui::BeginCombo("##sort", currentLabel)) {
        for (int i = 0; i < 6; ++i) {
            bool isSelected = (sortIndex == i);
            if (ImGui::Selectable(sortItems[i], isSelected)) {
                m_sortOrder = static_cast<SortOrder>(i);
                RefreshDirectory();
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();
}

void FileBrowserDialog::RenderPathBar() {
    // Simple path display with breadcrumb-style navigation
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, (m_pathBarHeight - m_fontSize) / 2));

    // Split path into components
    std::vector<std::string> pathParts;
    std::string remaining = m_currentPath;

#ifdef _WIN32
    // Handle Windows drive letter
    if (remaining.length() >= 2 && remaining[1] == ':') {
        pathParts.push_back(remaining.substr(0, 3));  // "C:\"
        remaining = remaining.substr(3);
    }
#else
    if (!remaining.empty() && remaining[0] == '/') {
        pathParts.push_back("/");
        remaining = remaining.substr(1);
    }
#endif

    // Split remaining path
    size_t pos = 0;
    while ((pos = remaining.find_first_of("/\\")) != std::string::npos) {
        std::string part = remaining.substr(0, pos);
        if (!part.empty()) {
            pathParts.push_back(part);
        }
        remaining = remaining.substr(pos + 1);
    }
    if (!remaining.empty()) {
        pathParts.push_back(remaining);
    }

    // Render breadcrumbs
    std::string buildPath;
    for (size_t i = 0; i < pathParts.size(); ++i) {
        if (i > 0) {
            ImGui::SameLine(0, 0);
            ImGui::TextDisabled("/");
            ImGui::SameLine(0, 4);
        }

#ifdef _WIN32
        if (i == 0) {
            buildPath = pathParts[i];
        } else {
            buildPath += pathParts[i] + "\\";
        }
#else
        if (i == 0 && pathParts[i] == "/") {
            buildPath = "/";
        } else {
            buildPath += pathParts[i] + "/";
        }
#endif

        std::string pathAtThisPoint = buildPath;

        // Make each part clickable
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::SmallButton(pathParts[i].c_str())) {
            NavigateTo(pathAtThisPoint);
        }
        ImGui::PopID();
    }

    ImGui::PopStyleVar();
    ImGui::Separator();
}

void FileBrowserDialog::RenderFileList() {
    const auto& colors = GetConfig().colors;
    const auto& icons = GetIcons();

    // Calculate available height for file list using actual ImGui style metrics
    // so the reserved space exactly matches what RenderFilenameAndFilter
    // + RenderButtons will consume (separator, item spacing, frame padding, etc.)
    const float itemSpacing = ImGui::GetStyle().ItemSpacing.y;
    const float separatorHeight = itemSpacing * 2 + 1.0f;  // Separator: spacing above + 1px line + spacing below
    float reservedHeight = separatorHeight + m_buttonHeight + itemSpacing;  // Separator + buttons + bottom pad
    if (m_config.mode != Mode::SelectFolder) {
        reservedHeight += m_inputHeight + itemSpacing;   // Filename + filter (single row)
    }

    float listHeight = ImGui::GetContentRegionAvail().y - reservedHeight;

    // Style the file list area with distinct background for visual separation
    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors.listBackground);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.listBorder);

    // Apply selection/hover colors from config (used by ImGui::Selectable)
    ImGui::PushStyleColor(ImGuiCol_Header, colors.selectedRow);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colors.hoveredRow);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, colors.selectedRow);

    // Touch mode: widen scrollbar for fat fingers (scaled)
    if (m_config.touchMode) {
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, BaseSize::TOUCH_SCROLLBAR_WIDTH * GetScale());
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, BaseSize::TOUCH_GRAB_MIN_SIZE * GetScale());
    }

    ImGui::BeginChild("FileList", ImVec2(0, listHeight), ImGuiChildFlags_Borders);

    // Touch mode: scale font for better readability
    float fontScale = m_config.touchMode ? 1.3f : 1.0f;
    if (m_config.touchMode) {
        ImGui::SetWindowFontScale(fontScale);
    }

    // Table for file list
    // NoSavedSettings prevents persisted column widths from causing layout instability
    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                 ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                                 ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable("Files", 3, tableFlags)) {
        // Column widths (scaled)
        float sizeColWidth = m_config.touchMode
            ? BaseSize::TOUCH_SIZE_COLUMN_WIDTH * GetScale()
            : BaseSize::SIZE_COLUMN_WIDTH * GetScale();
        float dateColWidth = m_config.touchMode
            ? BaseSize::TOUCH_DATE_COLUMN_WIDTH * GetScale()
            : BaseSize::DATE_COLUMN_WIDTH * GetScale();

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, sizeColWidth);
        ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed, dateColWidth);
        ImGui::TableSetupScrollFreeze(0, 1);  // Freeze header row
        ImGui::TableHeadersRow();

        // Use floored row height to ensure consistent calculations
        // Floating-point precision issues can cause the clipper to oscillate between
        // showing different rows each frame (hysteresis)
        float rowHeight = floorf(m_rowHeight);

        // Handle pending scroll from incremental search - must be inside table context
        if (m_pendingScrollToIndex >= 0 && m_pendingScrollToIndex < static_cast<int>(m_entries.size())) {
            float targetY = m_pendingScrollToIndex * rowHeight;
            ImGui::SetScrollY(targetY);
            m_pendingScrollToIndex = -1;
        }

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_entries.size()), rowHeight);

        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                const auto& entry = m_entries[row];

                ImGui::TableNextRow(0, rowHeight);

                // Name column
                ImGui::TableNextColumn();

                bool isSelected = (row == m_selectedIndex);

                // Make the whole row selectable
                ImGui::PushID(row);

                // Touch mode: single-click enters directories (double-click unreliable on touch)
                // Desktop mode: double-click to enter/open
                ImGuiSelectableFlags selectFlags = ImGuiSelectableFlags_SpanAllColumns;
                if (!m_config.touchMode) {
                    selectFlags |= ImGuiSelectableFlags_AllowDoubleClick;
                }

                if (ImGui::Selectable("##row", isSelected, selectFlags, ImVec2(0, rowHeight)))
                {
                    SelectEntry(row);

                    // Touch mode: single-click enters directories immediately
                    // Desktop mode: require double-click
                    if (m_config.touchMode && entry.isDirectory) {
                        m_pendingActivateIndex = row;  // Defer directory navigation
                    } else if (!m_config.touchMode && ImGui::IsMouseDoubleClicked(0)) {
                        m_pendingActivateIndex = row;  // Defer activation
                    }
                }
                ImGui::PopID();

                // Determine text colors based on selection state
                ImVec4 nameColor, secondaryColor;
                if (isSelected) {
                    nameColor = ImGui::ColorConvertU32ToFloat4(colors.selectedText);
                    secondaryColor = nameColor;
                } else {
                    nameColor = entry.isDirectory
                        ? ImGui::ColorConvertU32ToFloat4(colors.directoryText)
                        : ImGui::ColorConvertU32ToFloat4(colors.fileText);
                    secondaryColor = ImGui::ColorConvertU32ToFloat4(colors.secondaryText);
                }

                // Draw name with icon
                ImGui::SameLine(0, 0);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
                ImGui::TextColored(nameColor, "%s %s",
                    entry.isDirectory ? icons.folder : icons.file,
                    entry.name.c_str());

                // Size column
                ImGui::TableNextColumn();
                if (!entry.isDirectory) {
                    ImGui::TextColored(secondaryColor, "%s", FileSystemHelper::FormatFileSize(entry.size).c_str());
                }

                // Modified column
                ImGui::TableNextColumn();
                ImGui::TextColored(secondaryColor, "%s", FileSystemHelper::FormatDate(entry.modifiedTime).c_str());
            }
        }

        clipper.End();
        ImGui::EndTable();
    }

    // Reset font scale
    if (m_config.touchMode) {
        ImGui::SetWindowFontScale(1.0f);
    }

    ImGui::EndChild();

    // Pop touch mode styles
    if (m_config.touchMode) {
        ImGui::PopStyleVar(2);  // ScrollbarSize, GrabMinSize
    }

    // Pop file list style colors
    ImGui::PopStyleColor(5);  // ChildBg, Border, Header, HeaderHovered, HeaderActive

    // Process deferred activation AFTER table iteration is complete
    if (m_pendingActivateIndex >= 0) {
        int indexToActivate = m_pendingActivateIndex;
        m_pendingActivateIndex = -1;
        ActivateEntry(indexToActivate);
    }
}

void FileBrowserDialog::RenderFilenameAndFilter() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, (m_inputHeight - m_fontSize) / 2));

    // Measure label widths and filter combo width
    float labelWidth = ImGui::CalcTextSize("File name:").x;
    float filterComboWidth = 0.0f;
    float filterLabelWidth = 0.0f;
    bool hasFilters = !m_config.filters.empty();

    if (hasFilters) {
        filterLabelWidth = ImGui::CalcTextSize("Type:").x;
        // Size the filter combo from the longest filter string
        for (const auto& f : m_config.filters) {
            float w = ImGui::CalcTextSize(f.ToDisplayString().c_str()).x;
            filterComboWidth = (std::max)(filterComboWidth, w);
        }
        // Add padding for the combo dropdown arrow and frame
        filterComboWidth += ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 4;
    }

    // Layout: [File name:] [====input====] [Type:] [==combo==]
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float availWidth = ImGui::GetContentRegionAvail().x;
    float filterTotalWidth = hasFilters ? (filterLabelWidth + spacing + filterComboWidth) : 0.0f;
    float inputWidth = availWidth - labelWidth - spacing - filterTotalWidth - (hasFilters ? spacing : 0.0f);
    if (inputWidth < 100.0f) inputWidth = 100.0f;

    // Filename label + input
    ImGui::AlignTextToFramePadding();
    ImGui::Text("File name:");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(inputWidth);

    // Restore focus to filename input if it was active and user hasn't clicked elsewhere
    if (m_filenameInputActive && !ImGui::IsMouseClicked(0)) {
        ImGui::SetKeyboardFocusHere();
    }

    if (ImGui::InputText("##filename", m_filenameBuffer, sizeof(m_filenameBuffer))) {
        if (m_config.mode == Mode::Open && strlen(m_filenameBuffer) > 0) {
            int matchIndex = FindMatchingEntryIndex(m_filenameBuffer);
            if (matchIndex >= 0) {
                m_selectedIndex = matchIndex;
                m_pendingScrollToIndex = matchIndex;
            }
        }
    }
    m_filenameInputActive = ImGui::IsItemActive();

    // Filter combo on the same line
    if (hasFilters) {
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Type:");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(filterComboWidth);
        std::string currentFilter = m_config.filters[m_selectedFilterIndex].ToDisplayString();
        if (ImGui::BeginCombo("##filter", currentFilter.c_str())) {
            for (size_t i = 0; i < m_config.filters.size(); ++i) {
                bool isSelected = (static_cast<int>(i) == m_selectedFilterIndex);
                if (ImGui::Selectable(m_config.filters[i].ToDisplayString().c_str(), isSelected)) {
                    m_selectedFilterIndex = static_cast<int>(i);
                    RefreshDirectory();
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::PopStyleVar();
}

void FileBrowserDialog::RenderButtons() {
    ImGui::Separator();

    // Right-align buttons (scaled)
    float buttonWidth = m_buttonWidth;
    float spacing = BaseSize::BUTTON_SPACING * GetScale();
    float totalWidth = buttonWidth * 2 + spacing;

    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - totalWidth + ImGui::GetCursorPosX());

    // Cancel button
    if (ImGui::Button("Cancel", ImVec2(buttonWidth, m_buttonHeight))) {
        Close();
    }

    ImGui::SameLine(0, spacing);

    // OK button (label depends on mode)
    const char* okLabel = "Open";
    switch (m_config.mode) {
        case Mode::Open: okLabel = "Open"; break;
        case Mode::Save: okLabel = "Save"; break;
        case Mode::SelectFolder: okLabel = "Select"; break;
    }

    bool canSelect = IsValidSelection();

    ImGui::BeginDisabled(!canSelect);
    if (ImGui::Button(okLabel, ImVec2(buttonWidth, m_buttonHeight))) {
        if (canSelect) {
            std::string fullPath = BuildFullPath();

            // Check for overwrite in Save mode
            if (m_config.mode == Mode::Save &&
                FileSystemHelper::Exists(fullPath) &&
                FileSystemHelper::IsFile(fullPath))
            {
                m_overwritePath = fullPath;
                m_showOverwriteConfirm = true;
            } else {
                m_selectedPath = fullPath;
                m_result = Result::Selected;
                m_isOpen = false;
                SetLastPath(m_currentPath);  // Persist for next time
                NotifyFileSelected(m_selectedPath);
            }
        }
    }
    ImGui::EndDisabled();
}

void FileBrowserDialog::RenderNewFolderPopup() {
    if (!m_showNewFolderPopup) return;

    ImGui::OpenPopup("New Folder");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("New Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter folder name:");
        ImGui::SetNextItemWidth(BaseSize::POPUP_INPUT_WIDTH * GetScale());

        bool enterPressed = ImGui::InputText("##newfolder", m_newFolderBuffer, sizeof(m_newFolderBuffer),
                                             ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::Separator();

        float buttonW = m_buttonWidth;

        if (ImGui::Button("Cancel", ImVec2(buttonW, m_buttonHeight))) {
            m_showNewFolderPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool canCreate = strlen(m_newFolderBuffer) > 0;
        ImGui::BeginDisabled(!canCreate);
        if (ImGui::Button("Create", ImVec2(buttonW, m_buttonHeight)) || (enterPressed && canCreate)) {
            std::string newPath = FileSystemHelper::CombinePath(m_currentPath, m_newFolderBuffer);
            if (FileSystemHelper::CreateDirectory(newPath)) {
                RefreshDirectory();
            }
            m_showNewFolderPopup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}

void FileBrowserDialog::RenderOverwriteConfirmPopup() {
    if (!m_showOverwriteConfirm) return;

    ImGui::OpenPopup("Confirm Overwrite");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm Overwrite", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("File already exists:");
        ImGui::TextWrapped("%s", FileSystemHelper::GetFilename(m_overwritePath).c_str());
        ImGui::Spacing();
        ImGui::Text("Do you want to replace it?");

        ImGui::Separator();

        float buttonW = m_buttonWidth;

        if (ImGui::Button("No", ImVec2(buttonW, m_buttonHeight))) {
            m_showOverwriteConfirm = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Yes", ImVec2(buttonW, m_buttonHeight))) {
            m_selectedPath = m_overwritePath;
            m_result = Result::Selected;
            m_isOpen = false;
            m_showOverwriteConfirm = false;
            ImGui::CloseCurrentPopup();
            SetLastPath(m_currentPath);  // Persist for next time
            NotifyFileSelected(m_selectedPath);
        }

        ImGui::EndPopup();
    }
}

void FileBrowserDialog::NavigateTo(const std::string& path) {
    if (FileSystemHelper::IsDirectory(path)) {
        m_currentPath = path;
        m_selectedIndex = -1;
        RefreshDirectory();
    }
}

void FileBrowserDialog::NavigateUp() {
    std::string parent = FileSystemHelper::GetParentDirectory(m_currentPath);
    if (parent != m_currentPath) {
        NavigateTo(parent);
    }
}

void FileBrowserDialog::NavigateToParent() {
    NavigateUp();
}

void FileBrowserDialog::RefreshDirectory() {
    auto extensions = GetCurrentExtensions();

    if (m_config.mode == Mode::SelectFolder || extensions.empty()) {
        m_entries = FileSystemHelper::ListDirectory(m_currentPath, m_sortOrder);
    } else {
        m_entries = FileSystemHelper::ListDirectoryFiltered(m_currentPath, extensions, m_sortOrder);
    }

    // Filter hidden files
    if (!m_config.showHiddenFiles) {
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(), [](const FileEntry& e) {
                return !e.name.empty() && e.name[0] == '.';
            }),
            m_entries.end()
        );
    }

    m_selectedIndex = -1;
}

void FileBrowserDialog::SelectEntry(int index) {
    if (index < 0 || index >= static_cast<int>(m_entries.size())) {
        m_selectedIndex = -1;
        return;
    }

    m_selectedIndex = index;
    const auto& entry = m_entries[index];

    // Update filename buffer for files (not directories)
    if (!entry.isDirectory && m_config.mode != Mode::SelectFolder) {
        strncpy(m_filenameBuffer, entry.name.c_str(), sizeof(m_filenameBuffer) - 1);
        m_filenameBuffer[sizeof(m_filenameBuffer) - 1] = '\0';
    }
}

void FileBrowserDialog::ActivateEntry(int index) {
    if (index < 0 || index >= static_cast<int>(m_entries.size())) {
        return;
    }

    const auto& entry = m_entries[index];

    if (entry.isDirectory) {
        // Navigate into directory
        NavigateTo(entry.path);
    } else {
        // Select file and close (if in Open mode)
        if (m_config.mode == Mode::Open) {
            m_selectedPath = entry.path;
            m_result = Result::Selected;
            m_isOpen = false;
            SetLastPath(m_currentPath);  // Persist for next time
            NotifyFileSelected(m_selectedPath);
        }
    }
}

std::vector<std::string> FileBrowserDialog::GetCurrentExtensions() const {
    if (m_config.filters.empty() || m_selectedFilterIndex < 0 ||
        m_selectedFilterIndex >= static_cast<int>(m_config.filters.size())) {
        return {};
    }

    return m_config.filters[m_selectedFilterIndex].GetExtensionList();
}

bool FileBrowserDialog::IsValidSelection() const {
    switch (m_config.mode) {
        case Mode::Open:
            // Need a file selected
            return m_selectedIndex >= 0 &&
                   m_selectedIndex < static_cast<int>(m_entries.size()) &&
                   !m_entries[m_selectedIndex].isDirectory;

        case Mode::Save:
            // Need a filename entered
            return strlen(m_filenameBuffer) > 0;

        case Mode::SelectFolder:
            // Current directory is always valid
            return true;
    }
    return false;
}

std::string FileBrowserDialog::BuildFullPath() const {
    switch (m_config.mode) {
        case Mode::Open:
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_entries.size())) {
                return m_entries[m_selectedIndex].path;
            }
            break;

        case Mode::Save: {
            std::string filename = m_filenameBuffer;

            // Add extension if not present
            if (!m_config.filters.empty()) {
                auto extensions = GetCurrentExtensions();
                if (!extensions.empty()) {
                    std::string currentExt = FileSystemHelper::GetExtension(filename);
                    bool hasValidExt = false;
                    for (const auto& ext : extensions) {
                        if (currentExt == ext) {
                            hasValidExt = true;
                            break;
                        }
                    }
                    if (!hasValidExt) {
                        filename += extensions[0];
                    }
                }
            }

            return FileSystemHelper::CombinePath(m_currentPath, filename);
        }

        case Mode::SelectFolder:
            return m_currentPath;
    }
    return "";
}

void FileBrowserDialog::NotifyFileSelected(const std::string& path) {
#ifdef IMFILEBROWSER_USE_SIGSLOT
    onFileSelected(path);
#endif
    if (m_onFileSelected) {
        m_onFileSelected(path);
    }
}

void FileBrowserDialog::NotifyCancelled() {
#ifdef IMFILEBROWSER_USE_SIGSLOT
    onCancelled();
#endif
    if (m_onCancelled) {
        m_onCancelled();
    }
}

int FileBrowserDialog::FindMatchingEntryIndex(const char* prefix) const {
    if (!prefix || prefix[0] == '\0') {
        return -1;
    }

    size_t prefixLen = strlen(prefix);

    // Find first entry that starts with prefix (case-insensitive)
    for (size_t i = 0; i < m_entries.size(); ++i) {
        const auto& entry = m_entries[i];
        if (entry.name.length() >= prefixLen) {
            bool match = true;
            for (size_t j = 0; j < prefixLen && match; ++j) {
                char c1 = static_cast<char>(tolower(static_cast<unsigned char>(entry.name[j])));
                char c2 = static_cast<char>(tolower(static_cast<unsigned char>(prefix[j])));
                if (c1 != c2) {
                    match = false;
                }
            }
            if (match) {
                return static_cast<int>(i);
            }
        }
    }

    return -1;
}

} // namespace ImFileBrowser
