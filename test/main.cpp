// ImFileBrowser Scaling Test
// Tests UI scaling with CTRL+PLUS and CTRL+MINUS

// Prevent Windows min/max macros from conflicting with std::min/max
#define NOMINMAX

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include "ImFileBrowser/ImFileBrowser.hpp"
#include <ImGuiScaling/ImGuiScaling.hpp>

#include <cstdio>
#include <algorithm>

// Global scale config using ImGuiScaling
static ImGuiScaling::ScaleConfig g_scaleConfig;
static bool g_scaleChanged = false;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)window;

    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;

    if (ctrl) {
        if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
            // Ctrl+Plus: Increase scale
            g_scaleConfig.userScale += 0.1f;
            g_scaleConfig.userScale = std::min(g_scaleConfig.userScale, 3.0f);
            g_scaleChanged = true;
            printf("Scale: %.1f\n", g_scaleConfig.GetEffectiveScale());
        }
        else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
            // Ctrl+Minus: Decrease scale
            g_scaleConfig.userScale -= 0.1f;
            g_scaleConfig.userScale = std::max(g_scaleConfig.userScale, 0.5f);
            g_scaleChanged = true;
            printf("Scale: %.1f\n", g_scaleConfig.GetEffectiveScale());
        }
        else if (key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) {
            // Ctrl+0: Reset scale
            g_scaleConfig.userScale = 1.0f;
            g_scaleChanged = true;
            printf("Scale reset to: %.1f\n", g_scaleConfig.GetEffectiveScale());
        }
    }
}

int main(int, char**) {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImFileBrowser Scaling Test", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Set key callback for CTRL+PLUS/MINUS
    glfwSetKeyCallback(window, key_callback);

    // Get DPI scale
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    g_scaleConfig.dpiScale = xscale;
    printf("DPI Scale: %.2f\n", g_scaleConfig.dpiScale);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Scale ImGui style for DPI
    ImGui::GetStyle().ScaleAllSizes(g_scaleConfig.dpiScale);

    // Create file browser
    ImFileBrowser::FileBrowserDialog fileBrowser;
    ImFileBrowser::ConfirmationDialog confirmDialog;

    bool showFileBrowser = false;
    bool showConfirmDialog = false;
    std::string lastSelectedPath;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Handle scale changes
        if (g_scaleChanged) {
            float effectiveScale = g_scaleConfig.GetEffectiveScale();

            // Update ImGui font scale
            io.FontGlobalScale = g_scaleConfig.userScale;

            // Update dialogs with new scale
            fileBrowser.SetScale(effectiveScale);
            confirmDialog.SetScale(effectiveScale);

            g_scaleChanged = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Control panel
        {
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("Scaling Test Controls");

            ImGui::Text("DPI Scale: %.2f", g_scaleConfig.dpiScale);
            ImGui::Text("User Scale: %.2f", g_scaleConfig.userScale);
            ImGui::Text("Effective Scale: %.2f", g_scaleConfig.GetEffectiveScale());
            ImGui::Separator();

            ImGui::TextWrapped("Controls:");
            ImGui::BulletText("CTRL+PLUS: Increase scale");
            ImGui::BulletText("CTRL+MINUS: Decrease scale");
            ImGui::BulletText("CTRL+0: Reset scale");
            ImGui::Separator();

            // Manual scale slider
            if (ImGui::SliderFloat("User Scale", &g_scaleConfig.userScale, 0.5f, 3.0f, "%.1f")) {
                g_scaleChanged = true;
            }

            ImGui::Separator();

            if (ImGui::Button("Open File Browser", ImVec2(-1, 0))) {
                ImFileBrowser::DialogConfig config;
                config.mode = ImFileBrowser::Mode::Open;
                config.title = "Select a File";
                config.scale = g_scaleConfig.GetEffectiveScale();
                config.filters = {
                    {"All Files", "*.*"},
                    {"Text Files", "*.txt"},
                    {"Images", "*.png;*.jpg;*.jpeg;*.bmp"}
                };
                fileBrowser.Open(config);
                showFileBrowser = true;
            }

            if (ImGui::Button("Open Confirmation Dialog", ImVec2(-1, 0))) {
                ImFileBrowser::ConfirmationConfig config;
                config.title = "Confirm Action";
                config.message = "This is a test confirmation dialog.";
                config.detailMessage = "The dialog should scale with CTRL+PLUS/MINUS.";
                config.buttons = ImFileBrowser::DialogButton::YesNoCancel;
                config.icon = ImFileBrowser::DialogIcon::Question;
                config.scale = g_scaleConfig.GetEffectiveScale();
                confirmDialog.Show(config);
                showConfirmDialog = true;
            }

            ImGui::Separator();

            if (!lastSelectedPath.empty()) {
                ImGui::Text("Last selected:");
                ImGui::TextWrapped("%s", lastSelectedPath.c_str());
            }

            ImGui::End();
        }

        // Render file browser
        if (showFileBrowser && fileBrowser.IsOpen()) {
            auto result = fileBrowser.Render();
            if (result == ImFileBrowser::Result::Selected) {
                lastSelectedPath = fileBrowser.GetSelectedPath();
                showFileBrowser = false;
            } else if (result == ImFileBrowser::Result::Cancelled) {
                showFileBrowser = false;
            }
        }

        // Render confirmation dialog
        if (showConfirmDialog && confirmDialog.IsShown()) {
            auto result = confirmDialog.Render();
            if (result != ImFileBrowser::DialogResult::None) {
                showConfirmDialog = false;
            }
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
