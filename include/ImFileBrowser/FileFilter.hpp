// FileFilter.hpp
// File filter specification for ImFileBrowser library
// Standalone ImGui-based file browser

#pragma once

#include <string>
#include <vector>
#include <algorithm>

namespace ImFileBrowser {

/**
 * @brief Filter specification for file dialogs
 *
 * Represents a single filter entry like "JML Files (*.jml)"
 */
struct FileFilter {
    std::string description;    // e.g., "JML Document"
    std::string extensions;     // e.g., "*.jml" or "*.jml;*.jmd"

    FileFilter() = default;
    FileFilter(const std::string& desc, const std::string& ext)
        : description(desc), extensions(ext) {}

    /**
     * @brief Format for Windows-style filter string
     * @return "Description|*.ext" format
     */
    std::string ToFilterString() const {
        return description + "|" + extensions;
    }

    /**
     * @brief Format for display in dialogs
     * @return "Description (*.ext)" format
     */
    std::string ToDisplayString() const {
        return description + " (" + extensions + ")";
    }

    /**
     * @brief Parse extensions into a list
     * @return Vector of extensions with dots (e.g., {".jml", ".jmd"})
     */
    std::vector<std::string> GetExtensionList() const {
        std::vector<std::string> result;

        // Parse extensions from filter (e.g., "*.jml;*.jmd")
        std::string exts = extensions;
        size_t pos = 0;
        while ((pos = exts.find("*.")) != std::string::npos) {
            exts = exts.substr(pos + 2);  // Skip "*."
            size_t endPos = exts.find_first_of(";,");
            std::string ext = "." + (endPos != std::string::npos ? exts.substr(0, endPos) : exts);

            // Lowercase the extension
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            result.push_back(ext);

            if (endPos == std::string::npos) break;
            exts = exts.substr(endPos + 1);
        }

        return result;
    }
};

} // namespace ImFileBrowser
