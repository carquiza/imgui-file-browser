# ImFileBrowser

A standalone, touch-friendly ImGui file browser dialog library.

## Features

- **File Browser Dialog**: Open, Save, and Select Folder modes
- **Confirmation Dialog**: Generic modal dialogs with configurable buttons
- **Touch Support**: Large touch targets, touch-optimized sizing presets
- **Cross-Platform**: Windows and Linux support (drive enumeration, filesystem operations)
- **Configurable**: Colors, sizes, and icons can be customized
- **FontAwesome Icons**: Optional icon support with text fallbacks
- **Path Persistence**: Automatically remembers the last used directory via imgui.ini

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

### Option 4: vcpkg (Overlay Ports)

For vcpkg integration, use overlay ports via the `VCPKG_OVERLAY_PORTS` environment variable. This library depends on [imgui-scaling](https://github.com/carquiza/imgui-scaling), so your overlay ports registry must include both packages.

#### Setup

1. Set the `VCPKG_OVERLAY_PORTS` environment variable to your overlay ports registry:
```bash
# Windows
set VCPKG_OVERLAY_PORTS=C:\path\to\your\vcpkg-ports

# Linux/macOS
export VCPKG_OVERLAY_PORTS=/path/to/your/vcpkg-ports
```

2. Add to your `vcpkg.json`:
```json
{
  "dependencies": [
    "imfilebrowser",
    {
      "name": "imgui",
      "features": ["glfw-binding", "opengl3-binding"]
    }
  ]
}
```

3. Use in CMake:
```cmake
find_package(ImFileBrowser CONFIG REQUIRED)
target_link_libraries(YourApp PRIVATE ImFileBrowser::ImFileBrowser)
```

The overlay ports registry should contain port definitions for `imfilebrowser` and `imguiscaling`.

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

### Path Persistence (imgui.ini)

ImFileBrowser can automatically save and restore the last browsed directory using ImGui's settings system. This requires registering a settings handler during initialization.

**Important**: The initialization order matters. You must:
1. Call `ImGui::CreateContext()`
2. Call `ImFileBrowser::RegisterSettingsHandler()`
3. Call `ImGui::LoadIniSettingsFromDisk()` to load persisted settings

```cpp
// During application initialization
IMGUI_CHECKVERSION();
ImGui::CreateContext();

// Register settings handler (must be after CreateContext, before LoadIniSettings)
ImFileBrowser::RegisterSettingsHandler();

// Explicitly load settings after handlers are registered
ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);

// Continue with rest of ImGui setup...
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init(glsl_version);
```

Once registered, the file browser automatically:
- Saves the current directory to `imgui.ini` when a file is selected
- Restores the last used directory when opened (if no `initialPath` is specified)

The settings are stored in a `[ImFileBrowser][Data]` section:
```ini
[ImFileBrowser][Data]
LastPath=C:\Users\Documents\Projects
```

**Note**: If you specify `config.initialPath` when opening the dialog, it takes priority over the persisted path. Leave `initialPath` empty to use the persisted path:

```cpp
ImFileBrowser::DialogConfig config;
config.mode = ImFileBrowser::Mode::Open;
config.title = "Open File";
// Don't set config.initialPath - let file browser use persisted path
browser.Open(config);
```

### Custom Colors

```cpp
ImFileBrowser::LibraryConfig config = ImFileBrowser::GetConfig();
config.colors.listBackground = IM_COL32(30, 30, 35, 255);
config.colors.directoryText = IM_COL32(100, 200, 255, 255);
config.colors.selectedRow = IM_COL32(0, 120, 200, 180);
ImFileBrowser::SetConfig(config);
```

### UI Scaling (DPI / User Preference)

ImFileBrowser supports scaling for high-DPI displays and user preferences. The scale factor affects all dialog elements including window size, buttons, icons, row heights, and column widths.

#### Setting Scale When Opening

```cpp
// Get DPI scale from your windowing system (e.g., GLFW)
float dpiScale = 1.0f;  // glfwGetWindowContentScale(window, &dpiScale, nullptr);

// User preference scale (e.g., from Ctrl+Plus/Minus)
float userScale = 1.2f;

// Set combined scale in config
ImFileBrowser::DialogConfig config;
config.mode = ImFileBrowser::Mode::Open;
config.title = "Open File";
config.scale = dpiScale * userScale;  // Combined effective scale
browser.Open(config);
```

#### Updating Scale at Runtime

If the user changes scale while the dialog is open (e.g., via Ctrl+Plus/Minus):

```cpp
// When scale changes, update the dialog
void onScaleChanged(float newDpiScale, float newUserScale) {
    float effectiveScale = newDpiScale * newUserScale;

    // Update open dialogs
    fileBrowser.SetScale(effectiveScale);
    confirmDialog.SetScale(effectiveScale);

    // Also update ImGui's font scale for consistency
    ImGui::GetIO().FontGlobalScale = newUserScale;
}
```

#### Scale Values

| Scale | Use Case |
|-------|----------|
| 0.5 - 0.75 | Compact UI, more content visible |
| 1.0 | Default (96 DPI equivalent) |
| 1.25 - 1.5 | High-DPI displays (125-150% scaling) |
| 2.0 | 4K displays, accessibility |
| 2.0 - 3.0 | Touch devices, large displays |

#### Base Size Constants

All UI elements use base sizes defined in `ImFileBrowser::BaseSize` namespace. These are multiplied by the scale factor at runtime:

```cpp
// Example base sizes (at 1.0x scale)
BaseSize::DIALOG_WIDTH      // 650px - default dialog width
BaseSize::BUTTON_HEIGHT     // 28px  - button height
BaseSize::ROW_HEIGHT        // 24px  - file list row height
BaseSize::ICON_SIZE         // 18px  - icon dimensions

// Touch mode uses larger base sizes
BaseSize::TOUCH_ROW_HEIGHT  // 52px  - touch-friendly rows
BaseSize::TOUCH_BUTTON_HEIGHT // 48px - touch-friendly buttons
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
- `GetLastPath()` / `SetLastPath()` - Access persisted last browsed path
- `RegisterSettingsHandler()` - Register ImGui settings handler for path persistence
- `MakeSaveChangesConfig()` - Create save changes dialog config
- `MakeOverwriteConfig()` - Create overwrite confirmation config
- `MakeErrorConfig()` - Create error message config

## License

MIT License - See LICENSE file for details.
