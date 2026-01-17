// FileSystemHelper.hpp
// Cross-platform filesystem utilities for ImFileBrowser library
// Standalone ImGui-based file browser

#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ImFileBrowser {

/**
 * @brief Information about a file or directory
 */
struct FileEntry {
    std::string name;           // Filename only
    std::string path;           // Full path
    bool isDirectory = false;
    uint64_t size = 0;          // Size in bytes (0 for directories)
    std::time_t modifiedTime = 0;

    // For sorting
    bool operator<(const FileEntry& other) const {
        // Directories first, then alphabetical
        if (isDirectory != other.isDirectory) {
            return isDirectory > other.isDirectory;
        }
        // Case-insensitive comparison
        std::string lowerName = name;
        std::string lowerOther = other.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::transform(lowerOther.begin(), lowerOther.end(), lowerOther.begin(), ::tolower);
        return lowerName < lowerOther;
    }
};

/**
 * @brief Cross-platform filesystem utilities
 *
 * Provides abstraction over std::filesystem with additional
 * platform-specific features like drive enumeration on Windows.
 */
class FileSystemHelper {
public:
    /**
     * @brief List contents of a directory
     * @param path Directory path to list
     * @param sortOrder How to sort the results
     * @return Vector of file entries (directories first by default)
     */
    static std::vector<FileEntry> ListDirectory(
        const std::string& path,
        SortOrder sortOrder = SortOrder::NameAsc)
    {
        std::vector<FileEntry> entries;

        try {
            namespace fs = std::filesystem;

            for (const auto& entry : fs::directory_iterator(path)) {
                FileEntry fe;
                fe.name = entry.path().filename().string();
                fe.path = entry.path().string();
                fe.isDirectory = entry.is_directory();

                if (!fe.isDirectory) {
                    try {
                        fe.size = entry.file_size();
                    } catch (...) {
                        fe.size = 0;
                    }
                }

                try {
                    auto ftime = entry.last_write_time();
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    fe.modifiedTime = std::chrono::system_clock::to_time_t(sctp);
                } catch (...) {
                    fe.modifiedTime = 0;
                }

                entries.push_back(fe);
            }

            SortEntries(entries, sortOrder);
        }
        catch (const std::exception&) {
            // Return empty vector on error
        }

        return entries;
    }

    /**
     * @brief List contents of a directory with extension filter
     * @param path Directory path to list
     * @param extensions Vector of extensions to include (with dots, e.g., ".jml")
     * @param sortOrder How to sort the results
     * @return Vector of filtered file entries
     */
    static std::vector<FileEntry> ListDirectoryFiltered(
        const std::string& path,
        const std::vector<std::string>& extensions,
        SortOrder sortOrder = SortOrder::NameAsc)
    {
        auto entries = ListDirectory(path, sortOrder);

        if (extensions.empty()) {
            return entries;
        }

        // Filter to only include directories and files with matching extensions
        std::vector<FileEntry> filtered;
        for (const auto& entry : entries) {
            if (entry.isDirectory) {
                filtered.push_back(entry);
                continue;
            }

            std::string ext = GetExtension(entry.name);
            for (const auto& allowedExt : extensions) {
                if (CompareExtension(ext, allowedExt)) {
                    filtered.push_back(entry);
                    break;
                }
            }
        }

        return filtered;
    }

    /**
     * @brief Get available drives (Windows) or mount points (Unix)
     * @return Vector of root paths
     */
    static std::vector<std::string> GetDrives() {
        std::vector<std::string> drives;

#ifdef _WIN32
        DWORD driveMask = GetLogicalDrives();
        for (char letter = 'A'; letter <= 'Z'; ++letter) {
            if (driveMask & 1) {
                std::string drive = std::string(1, letter) + ":\\";
                drives.push_back(drive);
            }
            driveMask >>= 1;
        }
#else
        // Unix: common mount points
        drives.push_back("/");

        namespace fs = std::filesystem;

        // Check for /home
        if (fs::exists("/home")) {
            drives.push_back("/home");
        }

        // Check for common mount directories
        std::vector<std::string> mountDirs = {"/mnt", "/media", "/run/media"};
        for (const auto& mountDir : mountDirs) {
            try {
                if (fs::exists(mountDir) && fs::is_directory(mountDir)) {
                    for (const auto& entry : fs::directory_iterator(mountDir)) {
                        if (entry.is_directory()) {
                            drives.push_back(entry.path().string());
                        }
                    }
                }
            } catch (...) {}
        }
#endif

        return drives;
    }

    /**
     * @brief Get user's home directory
     * @return Home directory path
     */
    static std::string GetHomeDirectory() {
#ifdef _WIN32
        const char* userProfile = std::getenv("USERPROFILE");
        if (userProfile) {
            return std::string(userProfile);
        }
        const char* homeDrive = std::getenv("HOMEDRIVE");
        const char* homePath = std::getenv("HOMEPATH");
        if (homeDrive && homePath) {
            return std::string(homeDrive) + std::string(homePath);
        }
        return "C:\\";
#else
        const char* home = std::getenv("HOME");
        return home ? std::string(home) : "/";
#endif
    }

    /**
     * @brief Get user's documents directory
     * @return Documents directory path
     */
    static std::string GetDocumentsDirectory() {
#ifdef _WIN32
        // Try USERPROFILE/Documents
        std::string home = GetHomeDirectory();
        std::string docs = home + "\\Documents";
        if (std::filesystem::exists(docs)) {
            return docs;
        }
        return home;
#else
        std::string home = GetHomeDirectory();
        std::string docs = home + "/Documents";
        if (std::filesystem::exists(docs)) {
            return docs;
        }
        return home;
#endif
    }

    /**
     * @brief Get parent directory of a path
     * @param path Path to get parent of
     * @return Parent directory path
     */
    static std::string GetParentDirectory(const std::string& path) {
        namespace fs = std::filesystem;
        fs::path p(path);

        if (p.has_parent_path()) {
            return p.parent_path().string();
        }
        return path;
    }

    /**
     * @brief Check if a path exists
     * @param path Path to check
     * @return true if exists
     */
    static bool Exists(const std::string& path) {
        return std::filesystem::exists(path);
    }

    /**
     * @brief Check if a path is a directory
     * @param path Path to check
     * @return true if directory
     */
    static bool IsDirectory(const std::string& path) {
        return std::filesystem::is_directory(path);
    }

    /**
     * @brief Check if a path is a regular file
     * @param path Path to check
     * @return true if file
     */
    static bool IsFile(const std::string& path) {
        return std::filesystem::is_regular_file(path);
    }

    /**
     * @brief Create a directory
     * @param path Directory path to create
     * @return true on success
     */
    static bool CreateDirectory(const std::string& path) {
        try {
            return std::filesystem::create_directories(path);
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Get file extension (lowercase, with dot)
     * @param path File path or name
     * @return Extension string (e.g., ".jml")
     */
    static std::string GetExtension(const std::string& path) {
        namespace fs = std::filesystem;
        std::string ext = fs::path(path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    /**
     * @brief Get filename without extension
     * @param path File path or name
     * @return Stem (filename without extension)
     */
    static std::string GetStem(const std::string& path) {
        return std::filesystem::path(path).stem().string();
    }

    /**
     * @brief Get filename from path
     * @param path Full path
     * @return Filename only
     */
    static std::string GetFilename(const std::string& path) {
        return std::filesystem::path(path).filename().string();
    }

    /**
     * @brief Combine path components
     * @param base Base path
     * @param child Child path or filename
     * @return Combined path
     */
    static std::string CombinePath(const std::string& base, const std::string& child) {
        namespace fs = std::filesystem;
        return (fs::path(base) / fs::path(child)).string();
    }

    /**
     * @brief Format file size for display
     * @param bytes Size in bytes
     * @return Human-readable size string
     */
    static std::string FormatFileSize(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }

        char buffer[32];
        if (unitIndex == 0) {
            snprintf(buffer, sizeof(buffer), "%d %s", static_cast<int>(size), units[unitIndex]);
        } else {
            snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unitIndex]);
        }
        return std::string(buffer);
    }

    /**
     * @brief Format date for display
     * @param time Time value
     * @return Formatted date string
     */
    static std::string FormatDate(std::time_t time) {
        if (time == 0) return "";

        char buffer[32];
        std::tm tmBuf;
#ifdef _WIN32
        if (localtime_s(&tmBuf, &time) == 0) {
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &tmBuf);
            return std::string(buffer);
        }
#else
        std::tm* tm = std::localtime(&time);
        if (tm) {
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm);
            return std::string(buffer);
        }
#endif
        return "";
    }

private:
    /**
     * @brief Compare extensions case-insensitively
     */
    static bool CompareExtension(const std::string& ext1, const std::string& ext2) {
        std::string lower1 = ext1;
        std::string lower2 = ext2;
        std::transform(lower1.begin(), lower1.end(), lower1.begin(), ::tolower);
        std::transform(lower2.begin(), lower2.end(), lower2.begin(), ::tolower);
        return lower1 == lower2;
    }

    /**
     * @brief Sort file entries based on sort order
     */
    static void SortEntries(std::vector<FileEntry>& entries, SortOrder order) {
        auto compareName = [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            std::string la = a.name, lb = b.name;
            std::transform(la.begin(), la.end(), la.begin(), ::tolower);
            std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
            return la < lb;
        };

        auto compareSize = [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return a.size < b.size;
        };

        auto compareDate = [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return a.modifiedTime < b.modifiedTime;
        };

        switch (order) {
            case SortOrder::NameAsc:
                std::sort(entries.begin(), entries.end(), compareName);
                break;
            case SortOrder::NameDesc:
                std::sort(entries.begin(), entries.end(), [&](const auto& a, const auto& b) {
                    return !compareName(a, b) && (a.isDirectory == b.isDirectory || compareName(a, b));
                });
                break;
            case SortOrder::SizeAsc:
                std::sort(entries.begin(), entries.end(), compareSize);
                break;
            case SortOrder::SizeDesc:
                std::sort(entries.begin(), entries.end(), [&](const auto& a, const auto& b) {
                    if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
                    return a.size > b.size;
                });
                break;
            case SortOrder::DateAsc:
                std::sort(entries.begin(), entries.end(), compareDate);
                break;
            case SortOrder::DateDesc:
                std::sort(entries.begin(), entries.end(), [&](const auto& a, const auto& b) {
                    if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
                    return a.modifiedTime > b.modifiedTime;
                });
                break;
        }
    }
};

} // namespace ImFileBrowser
