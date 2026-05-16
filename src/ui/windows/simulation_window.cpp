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
    ImGui::Separator();
    render_controls();
    step_env();
    ImGui::Separator();
    render_status();

    ImGui::End();
}

void SimulationWindow::step_env() {
    if (_state != State::Running || !_environment)
        return;
    _environment->step(static_cast<size_t>(_steps_offline));
}

void SimulationWindow::render_config() {
    if (!ImGui::CollapsingHeader("Configuration", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    const bool editable = (_state == State::Idle);
    ImGui::BeginDisabled(!editable);

    int num_colonies = static_cast<int>(_config.num_colonies);
    int bees_per_colony = static_cast<int>(_config.bees_per_colony);
    int num_nest_boxes = static_cast<int>(_config.num_nest_boxes);
    int steps_per_repetition = static_cast<int>(_config.steps_per_repetition);
    int repetitions_per_generation = static_cast<int>(_config.repetitions_per_generation);
    int seed = static_cast<int>(_config.seed);

    ImGui::SeparatorText("Colony");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Colonies", &num_colonies);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Bees per colony", &bees_per_colony);

    ImGui::SeparatorText("Environment");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Nest boxes", &num_nest_boxes);

    ImGui::SeparatorText("Optimization");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Steps per repetition", &steps_per_repetition);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Repetitions per generation", &repetitions_per_generation);

    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Steps offline", &_steps_offline);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Seed", &seed);

    if (num_colonies < 1)
        num_colonies = 1;
    if (bees_per_colony < 1)
        bees_per_colony = 1;
    if (num_nest_boxes < 1)
        num_nest_boxes = 1;
    if (steps_per_repetition < 1)
        steps_per_repetition = 1;
    if (repetitions_per_generation < 1)
        repetitions_per_generation = 1;
    if (_steps_offline < 1)
        _steps_offline = 1;

    _config.num_colonies = static_cast<size_t>(num_colonies);
    _config.bees_per_colony = static_cast<size_t>(bees_per_colony);
    _config.num_nest_boxes = static_cast<size_t>(num_nest_boxes);
    _config.steps_per_repetition = static_cast<size_t>(steps_per_repetition);
    _config.repetitions_per_generation = static_cast<size_t>(repetitions_per_generation);
    _config.seed = static_cast<uint32_t>(seed);

    ImGui::EndDisabled();
}

void SimulationWindow::render_controls() {
    const ImVec2 button_size(90, 0);

    // Start / Resume — Idle creates a fresh sim, Paused resumes.
    const char* start_label = (_state == State::Paused) ? "Resume" : "Start";
    const bool can_start = (_state != State::Running);
    ImGui::BeginDisabled(!can_start);
    if (ImGui::Button(start_label, button_size)) {
        if (_state == State::Idle)
            _environment = std::make_unique<sim::Environment>(_config);
        _state = State::Running;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(_state != State::Running);
    if (ImGui::Button("Pause", button_size))
        _state = State::Paused;
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(_state == State::Idle);
    if (ImGui::Button("Stop", button_size)) {
        _environment.reset();
        _state = State::Idle;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(_state == State::Idle);
    if (ImGui::Button("Reset", button_size)) {
        // Recreate from current config, keep current play state.
        _environment = std::make_unique<sim::Environment>(_config);
    }
    ImGui::EndDisabled();
}

void SimulationWindow::render_status() {
    if (!_environment) {
        ImGui::TextDisabled("Idle — press Start to begin.");
        return;
    }

    ImGui::Text("Generation: %zu", _environment->current_generation());
    ImGui::SameLine(220);
    ImGui::Text("Repetition: %zu / %zu", _environment->current_repetition() + 1, _config.repetitions_per_generation);
    ImGui::SameLine(440);
    ImGui::Text("Step: %zu / %zu", _environment->current_step(), _config.steps_per_repetition);

    ImGui::Spacing();

    if (ImGui::BeginTable("hive_fitness", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Hive");
        ImGui::TableSetupColumn("Color");
        ImGui::TableSetupColumn("Fitness");
        ImGui::TableHeadersRow();

        size_t i = 0;
        for (const auto& hive : _environment->hives()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", i);
            ImGui::TableSetColumnIndex(1);
            const Eigen::Vector3f c = hive.color();
            ImGui::ColorButton("##color", ImVec4(c.x(), c.y(), c.z(), 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.4f", static_cast<double>(hive.fitness()));
            i++;
        }
        ImGui::EndTable();
    }
}

} // namespace ui
