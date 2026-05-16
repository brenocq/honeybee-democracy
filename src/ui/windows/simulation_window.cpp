//--------------------------------------------------
// Honeybee Democracy
// simulation_window.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <ui/windows/simulation_window.hpp>

#include <imgui.h>

namespace ui {

void SimulationWindow::render() {
    if (!ImGui::Begin("Simulation")) {
        ImGui::End();
        return;
    }

    render_config();

    if (ImGui::Button("Optimize", ImVec2(120, 0))) {
        // TODO: run optimization
    }

    ImGui::End();
}

void SimulationWindow::render_config() {
    if (!ImGui::CollapsingHeader("Configuration", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::SeparatorText("Colony");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Colonies", &_num_colonies);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Bees per colony", &_bees_per_colony);

    ImGui::SeparatorText("Environment");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Nest boxes", &_num_nest_boxes);

    ImGui::SeparatorText("Optimization");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Steps per repetition", &_steps_per_repetition);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Repetitions per generation", &_repetitions_per_generation);

    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Steps offline", &_steps_offline);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Seed", &_seed);

    if (_num_colonies < 1)
        _num_colonies = 1;
    if (_bees_per_colony < 1)
        _bees_per_colony = 1;
    if (_num_nest_boxes < 1)
        _num_nest_boxes = 1;
    if (_steps_per_repetition < 1)
        _steps_per_repetition = 1;
    if (_repetitions_per_generation < 1)
        _repetitions_per_generation = 1;
    if (_steps_offline < 1)
        _steps_offline = 1;
}

} // namespace ui
