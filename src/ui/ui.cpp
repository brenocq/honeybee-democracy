//--------------------------------------------------
// Honeybee Democracy
// ui.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <ui/ui.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

namespace ui {

void UI::startup() {}

void UI::shutdown() {}

void UI::update() {
    render_menu_bar();
    setup_dockspace();

    if (_show_imgui_demo)
        ImGui::ShowDemoWindow(&_show_imgui_demo);
    if (_show_implot_demo)
        ImPlot::ShowDemoWindow(&_show_implot_demo);
}

void UI::render_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Demos")) {
            ImGui::MenuItem("ImGui Demo", nullptr, &_show_imgui_demo);
            ImGui::MenuItem("ImPlot Demo", nullptr, &_show_implot_demo);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UI::setup_dockspace() {
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    if (_first_render) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
        ImGui::DockBuilderFinish(dockspace_id);
        _first_render = false;
    }
}

} // namespace ui
