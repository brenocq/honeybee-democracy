//--------------------------------------------------
// Honeybee Democracy
// os_window.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <ui/os_window.hpp>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <iostream>

namespace ui {

static void glfw_error_callback(int error, const char* description) { std::cerr << "GLFW Error " << error << ": " << description << '\n'; }

OSWindow::OSWindow(const std::string& name, size_t width, size_t height) : _name(name), _width(width), _height(height) {}

bool OSWindow::create() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    _window = glfwCreateWindow(static_cast<int>(_width), static_cast<int>(_height), _name.c_str(), nullptr, nullptr);
    if (!_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    // Load Inter fonts (Regular at index 0, Bold at index 1)
    ImGuiIO& io = ImGui::GetIO();
    namespace fs = std::filesystem;
    std::string font_regular = (fs::path(HONEYBEE_SOURCE_DIR) / "assets" / "Inter-Regular.ttf").string();
    std::string font_bold = (fs::path(HONEYBEE_SOURCE_DIR) / "assets" / "Inter-Bold.ttf").string();
    io.Fonts->AddFontFromFileTTF(font_regular.c_str(), 16.0f);
    io.Fonts->AddFontFromFileTTF(font_bold.c_str(), 16.0f);

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void OSWindow::destroy() {
    if (!_window)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(_window);
    glfwTerminate();
}

bool OSWindow::should_close() { return glfwWindowShouldClose(_window); }

void OSWindow::begin_frame() {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OSWindow::end_frame() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window);
}

} // namespace ui
