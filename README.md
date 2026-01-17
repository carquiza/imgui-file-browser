# ImFileBrowser

A standalone, touch-friendly ImGui file browser dialog library.

## Features

- **File Browser Dialog**: Open, Save, and Select Folder modes
- **Confirmation Dialog**: Generic modal dialogs with configurable buttons
- **Touch Support**: Large touch targets, touch-optimized sizing presets
- **Cross-Platform**: Windows and Linux support (drive enumeration, filesystem operations)
- **Configurable**: Colors, sizes, and icons can be customized
- **FontAwesome Icons**: Optional icon support with text fallbacks

## Requirements

- C++17
- ImGui (you provide it)
- Optional: [palacaze/sigslot](https://github.com/palacaze/sigslot) for signal support

## Integration

### Option 1: FetchContent (Recommended)

```cmake
include(FetchContent)

# Fetch ImFileBrowser
FetchContent_Declare(
    ImFileBrowser
    GIT_REPOSITORY https://github.com/user/imgui-file-browser.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(ImFileBrowser)

# Link to your target
target_link_libraries(YourApp PRIVATE ImFileBrowser::ImFileBrowser)
```

### Option 2: Subdirectory

```cmake
# Make sure imgui target exists first, OR set IMGUI_INCLUDE_DIR
set(IMGUI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
add_subdirectory(external/imgui-file-browser)

target_link_libraries(YourApp PRIVATE ImFileBrowser::ImFileBrowser)
```

### Option 3: Copy into project

Copy the `imgui-file-browser` folder into your project and add:

```cmake
add_subdirectory(imgui-file-browser)
target_link_libraries(YourApp PRIVATE ImFileBrowser::ImFileBrowser)
```

## ImGui Dependency

ImFileBrowser requires ImGui but doesn't fetch it (you likely have your own setup).
Provide imgui in one of these ways:

1. **imgui target** (recommended): Create an `imgui` CMake target before adding ImFileBrowser
2. **IMGUI_INCLUDE_DIR**: Set this variable to your imgui include path
3. **Manual linking**: Add imgui includes to ImFileBrowser after adding the subdirectory

## Usage

### Basic File Browser

```cpp
#include <ImFileBrowser/ImFileBrowser.hpp>

// Create dialog instance (persistent across frames)
ImFileBrowser::FileBrowserDialog browser;

// Configure and open
ImFileBrowser::DialogConfig config;
config.mode = ImFileBrowser::Mode::Open;
config.title = "Open File";
config.filters = { {"Text Files", "*.txt"}, {"All Files", "*.*"} };
browser.Open(config);

// In your render loop
if (browser.IsOpen()) {
    auto result = browser.Render();
    if (result == ImFileBrowser::Result::Selected) {
        std::string path = browser.GetSelectedPath();
        // Use the selected file...
    }
}
```

### Save Dialog

```cpp
ImFileBrowser::DialogConfig config;
config.mode = ImFileBrowser::Mode::Save;
config.title = "Save Document";
config.initialFilename = "document.txt";
config.filters = { {"Text Files", "*.txt"} };
browser.Open(config);
```

### Confirmation Dialog

```cpp
#include <ImFileBrowser/ConfirmationDialog.hpp>

ImFileBrowser::ConfirmationDialog dialog;

// Show save changes prompt
auto config = ImFileBrowser::MakeSaveChangesConfig("document.txt");
dialog.Show(config);

// In render loop
if (dialog.IsShown()) {
    auto result = dialog.Render();
    if (result == ImFileBrowser::DialogResult::Save) {
        // Save...
    } else if (result == ImFileBrowser::DialogResult::DontSave) {
        // Discard...
    } else if (result == ImFileBrowser::DialogResult::Cancel) {
        // Cancel...
    }
}
```

### Configuration

```cpp
// Configure for touch mode
ImFileBrowser::LibraryConfig config;
config.touchMode = true;
config.sizes = ImFileBrowser::SizeConfig::Touch();
ImFileBrowser::SetConfig(config);

// Enable FontAwesome icons (if font is loaded)
ImFileBrowser::SetIcons(ImFileBrowser::IconSet::FontAwesome());
```

### Custom Colors

```cpp
ImFileBrowser::LibraryConfig config = ImFileBrowser::GetConfig();
config.colors.listBackground = IM_COL32(30, 30, 35, 255);
config.colors.directoryText = IM_COL32(100, 200, 255, 255);
config.colors.selectedRow = IM_COL32(0, 120, 200, 180);
ImFileBrowser::SetConfig(config);
```

## Signal Support (Optional)

For callback-based event handling without signals:

```cpp
browser.SetOnFileSelected([](const std::string& path) {
    // Handle file selection
});

browser.SetOnCancelled([]() {
    // Handle cancellation
});
```

For sigslot signal support, enable the option and link sigslot:

```cmake
set(IMFILEBROWSER_ENABLE_SIGNALS ON)
# Make sure Pal::Sigslot target exists
add_subdirectory(imgui-file-browser)
```

Then use signals:

```cpp
browser.onFileSelected.connect([](const std::string& path) {
    // Handle file selection
});
```

## API Reference

### Types

- `ImFileBrowser::Mode` - Open, Save, SelectFolder
- `ImFileBrowser::Result` - None, Selected, Cancelled
- `ImFileBrowser::DialogButton` - Ok, Cancel, Yes, No, Save, DontSave, Retry
- `ImFileBrowser::DialogResult` - None, Ok, Cancel, Yes, No, Save, DontSave, Retry
- `ImFileBrowser::DialogIcon` - None, Info, Warning, Error, Question

### Classes

- `FileBrowserDialog` - Main file browser dialog
- `ConfirmationDialog` - Generic confirmation/message dialog
- `FileSystemHelper` - Cross-platform filesystem utilities (static methods)
- `FileFilter` - Filter specification for file dialogs
- `FileEntry` - Information about a file/directory

### Configuration

- `LibraryConfig` - Main configuration (colors, sizes, touchMode)
- `ColorConfig` - Color scheme
- `SizeConfig` - UI element sizes (Desktop/Touch presets)
- `IconSet` - Icon strings (TextFallback/FontAwesome presets)

### Global Functions

- `GetConfig()` / `SetConfig()` - Access global configuration
- `GetIcons()` / `SetIcons()` - Access global icon set
- `MakeSaveChangesConfig()` - Create save changes dialog config
- `MakeOverwriteConfig()` - Create overwrite confirmation config
- `MakeErrorConfig()` - Create error message config

## License

MIT License - See LICENSE file for details.
