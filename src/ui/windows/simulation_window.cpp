//--------------------------------------------------
// Honeybee Democracy
// simulation_window.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <ui/windows/simulation_window.hpp>

#include <imgui.h>
#include <implot.h>
#include <numeric>
#include <string>
#include <vector>

namespace ui {

namespace {
// Match ImGui demo's HelpMarker: render a faint "(?)" that shows the tooltip when hovered.
void help_marker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
} // namespace

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

    if (_environment && ImGui::CollapsingHeader("Plots", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SeparatorText("Environment");
        render_environment_plot();
        ImGui::SeparatorText("Fitness");
        render_fitness_plot();
        ImGui::SeparatorText("Consensus");
        render_consensus_plot();
    }

    ImGui::End();
}

void SimulationWindow::step_env() {
    if (_state != State::Running || !_environment)
        return;
    if (_realtime) {
        constexpr double kRealtimeStepInterval = 0.025; // 25ms between steps when watching bees move
        const double now = ImGui::GetTime();
        if (now - _last_realtime_step >= kRealtimeStepInterval) {
            _environment->step(1);
            _last_realtime_step = now;
        }
    } else {
        _environment->step(static_cast<size_t>(_steps_offline));
    }
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
    int predation_interval = static_cast<int>(_config.predation_interval);
    float predation_fraction = _config.predation_fraction;
    float mutation_amplitude = static_cast<float>(_config.mutation_amplitude);

    ImGui::SeparatorText("Environment");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Colonies", &num_colonies);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Bees per colony", &bees_per_colony);

    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Nest boxes", &num_nest_boxes);

    ImGui::SeparatorText("Evaluation");
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

    ImGui::SeparatorText("Genetic Algorithm");
    ImGui::SetNextItemWidth(160);
    ImGui::InputInt("Predation interval", &predation_interval);
    ImGui::SameLine();
    help_marker("Number of generations between predation events. Every Nth generation, the worst-performing hives are wiped and replaced with fresh random genes — this prevents the population from getting stuck on a local optimum. Set to 0 to disable predation entirely.");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputFloat("Predation fraction", &predation_fraction, 0.01f, 0.1f, "%.2f");
    ImGui::SameLine();
    help_marker("Fraction of the worst-performing hives to re-randomize during a predation event. 0.10 means the bottom 10%. Higher values inject more diversity per event.");

    ImGui::SetNextItemWidth(160);
    ImGui::InputFloat("Mutation amplitude", &mutation_amplitude, 0.05f, 0.5f, "%.2f");
    ImGui::SameLine();
    help_marker("Strength of the random jitter applied to each gene during crossover. Each non-best hive's new gene is computed as 0.5 × best + 0.5 × own + best × U(-m, +m), where m is this value. Higher = more exploration, lower = more exploitation around the current best.");

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
    if (predation_interval < 0)
        predation_interval = 0;
    if (predation_fraction < 0.0f)
        predation_fraction = 0.0f;
    if (predation_fraction > 1.0f)
        predation_fraction = 1.0f;
    if (mutation_amplitude < 0.0f)
        mutation_amplitude = 0.0f;

    _config.num_colonies = static_cast<size_t>(num_colonies);
    _config.bees_per_colony = static_cast<size_t>(bees_per_colony);
    _config.num_nest_boxes = static_cast<size_t>(num_nest_boxes);
    _config.steps_per_repetition = static_cast<size_t>(steps_per_repetition);
    _config.repetitions_per_generation = static_cast<size_t>(repetitions_per_generation);
    _config.seed = static_cast<uint32_t>(seed);
    _config.predation_interval = static_cast<size_t>(predation_interval);
    _config.predation_fraction = predation_fraction;
    _config.mutation_amplitude = static_cast<double>(mutation_amplitude);

    ImGui::EndDisabled();
}

void SimulationWindow::render_controls() {
    const ImVec2 button_size(90, 0);

    // Start / Resume — Idle creates a fresh sim, Paused resumes.
    const char* start_label = (_state == State::Paused) ? "Resume" : "Start";
    const bool can_start = (_state != State::Running);
    ImGui::BeginDisabled(!can_start);
    if (ImGui::Button(start_label, button_size)) {
        if (_state == State::Idle) {
            _environment = std::make_unique<sim::Environment>(_config);
            _reset_env_axes = true;
        }
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
        _reset_env_axes = true;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Checkbox("Real-time", &_realtime);
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

}

void SimulationWindow::render_environment_plot() {
    if (!_environment)
        return;

    if (!ImPlot::BeginPlot("##env", ImVec2(-1, 320), ImPlotFlags_Equal | ImPlotFlags_NoLegend))
        return;
    constexpr ImPlotAxisFlags kBase = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoMenus;
    ImPlot::SetupAxes(nullptr, nullptr, kBase, kBase);
    // Pin both axes to the world bounds on the frame right after a fresh env is created
    // (Start from Idle or Reset). After that the user can pan/zoom freely.
    if (_reset_env_axes) {
        ImPlot::SetupAxesLimits(-1.0, 1.0, -1.0, 1.0, ImPlotCond_Always);
        _reset_env_axes = false;
    }

    // Nest boxes: diamond markers; color encodes goodness via the Jet colormap.
    for (const auto& nb : _environment->nest_boxes()) {
        const ImVec4 color = ImPlot::SampleColormap(nb.goodness(), ImPlotColormap_Jet);
        ImPlotSpec spec;
        spec.Marker = ImPlotMarker_Diamond;
        spec.MarkerSize = 6.0f;
        spec.MarkerFillColor = color;
        spec.MarkerLineColor = color;
        const float x = nb.position().x();
        const float y = nb.position().y();
        ImPlot::PlotScatter("##nb", &x, &y, 1, spec);
    }

    // Per-hive: bees as small dots, hive center as a larger square — both in hive color.
    for (const auto& hive : _environment->hives()) {
        const Eigen::Vector3f c = hive.color();
        const ImVec4 color(c.x(), c.y(), c.z(), 1.0f);

        const auto& bees = hive.bees();
        std::vector<float> xs;
        std::vector<float> ys;
        xs.reserve(bees.size());
        ys.reserve(bees.size());
        for (const auto& bee : bees) {
            xs.push_back(bee.position().x());
            ys.push_back(bee.position().y());
        }
        ImPlotSpec bee_spec;
        bee_spec.Marker = ImPlotMarker_Circle;
        bee_spec.MarkerSize = 2.0f;
        bee_spec.MarkerFillColor = color;
        bee_spec.MarkerLineColor = color;
        ImPlot::PlotScatter("##bees", xs.data(), ys.data(), static_cast<int>(xs.size()), bee_spec);

        const float hx = hive.position().x();
        const float hy = hive.position().y();
        ImPlotSpec hive_spec;
        hive_spec.Marker = ImPlotMarker_Square;
        hive_spec.MarkerSize = 8.0f;
        hive_spec.MarkerFillColor = color;
        hive_spec.MarkerLineColor = color;
        ImPlot::PlotScatter("##hive", &hx, &hy, 1, hive_spec);
    }

    ImPlot::EndPlot();
}

void SimulationWindow::render_fitness_plot() {
    if (!_environment)
        return;

    const auto& gen_fitness = _environment->generation_fitness();
    if (gen_fitness.empty()) {
        ImGui::TextDisabled("(No completed generations yet.)");
        return;
    }

    if (!ImPlot::BeginPlot("##fitness", ImVec2(-1, 220)))
        return;
    // While the sim is running, the data grows every generation. Auto-fit keeps both axes
    // chasing the data; pausing/stopping freezes the limits so the user can pan/zoom.
    const ImPlotAxisFlags fit_flag = (_state == State::Running) ? ImPlotAxisFlags_AutoFit : 0;
    ImPlot::SetupAxes("Generation", "Fitness", fit_flag, fit_flag);

    const auto& hives = _environment->hives();
    const size_t num_hives = hives.size();
    const size_t num_gens = gen_fitness.size();

    std::vector<float> xs(num_gens);
    std::iota(xs.begin(), xs.end(), 0.0f);

    for (size_t h = 0; h < num_hives; h++) {
        std::vector<float> ys;
        ys.reserve(num_gens);
        for (const auto& gen : gen_fitness)
            ys.push_back(gen[h]);

        const Eigen::Vector3f c = hives[h].color();
        ImPlotSpec spec;
        spec.LineColor = ImVec4(c.x(), c.y(), c.z(), 1.0f);
        spec.LineWeight = 2.0f;
        const std::string label = "Hive " + std::to_string(h);
        ImPlot::PlotLine(label.c_str(), xs.data(), ys.data(), static_cast<int>(num_gens), spec);
    }

    ImPlot::EndPlot();
}

void SimulationWindow::render_consensus_plot() {
    if (!_environment)
        return;
    const auto& hives = _environment->hives();
    const auto& nest_boxes = _environment->nest_boxes();
    if (hives.empty())
        return;

    // Each hive lives in its own small "panel" inside a single plot, offset along x.
    // For each nest box:
    //   angle  = atan2(nb.y, nb.x)     (env-origin → nest box direction)
    //   length = ||nb||                (env-origin → nest box distance)
    //   width  = consensus fraction for that box in this hive
    //   color  = Jet(goodness), matching the environment plot
    constexpr float kHiveSpacing = 3.0f;
    constexpr float kMaxRectWidth = 0.4f;
    constexpr float kYHalfRange = 1.5f;
    constexpr ImPlotAxisFlags kAxisFlags =
        ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoMenus;

    // ImPlotFlags_Equal forces equal pixels-per-unit on both axes so the rectangles render
    // with true right angles. Using ImPlotCond_Always for the bounds would override Equal's
    // aspect padding, so we set them once and let Equal pad whichever axis it needs.
    if (!ImPlot::BeginPlot("##consensus", ImVec2(-1, 220), ImPlotFlags_Equal | ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText))
        return;
    ImPlot::SetupAxes(nullptr, nullptr, kAxisFlags, kAxisFlags);
    const double total_x = static_cast<double>(hives.size()) * static_cast<double>(kHiveSpacing);
    ImPlot::SetupAxesLimits(-kHiveSpacing * 0.5, total_x - kHiveSpacing * 0.5, -kYHalfRange, kYHalfRange, ImPlotCond_Once);

    for (size_t h = 0; h < hives.size(); h++) {
        const float cx = static_cast<float>(h) * kHiveSpacing;
        const float cy = 0.0f;
        const auto& hive = hives[h];
        const float total_bees = static_cast<float>(hive.bees().size());

        for (size_t b = 0; b < nest_boxes.size(); b++) {
            const Eigen::Vector2f nb_pos = nest_boxes[b].position();
            const float length = nb_pos.norm();
            if (length < 1e-6f)
                continue;

            const int consensus_count = hive.consensus()[b];
            if (consensus_count == 0)
                continue;
            const float consensus_frac = static_cast<float>(consensus_count) / total_bees;

            const float angle = std::atan2(nb_pos.y(), nb_pos.x());
            const float dx = std::cos(angle);
            const float dy = std::sin(angle);
            const float perp_x = -dy;
            const float perp_y = dx;
            const float half_w = consensus_frac * kMaxRectWidth * 0.5f;

            // CCW around the perimeter: base+perp → base-perp → tip-perp → tip+perp.
            const float xs[4] = {
                cx + perp_x * half_w,
                cx - perp_x * half_w,
                cx + dx * length - perp_x * half_w,
                cx + dx * length + perp_x * half_w,
            };
            const float ys[4] = {
                cy + perp_y * half_w,
                cy - perp_y * half_w,
                cy + dy * length - perp_y * half_w,
                cy + dy * length + perp_y * half_w,
            };

            const ImVec4 color = ImPlot::SampleColormap(nest_boxes[b].goodness(), ImPlotColormap_Jet);
            ImPlotSpec spec;
            spec.FillColor = color;
            spec.FillAlpha = 0.85f;
            spec.LineColor = color;
            ImPlot::PlotPolygon("##d", xs, ys, 4, spec);
        }

        // Hive center marker — colored by hive identity (matches environment plot).
        const Eigen::Vector3f hc = hive.color();
        const ImVec4 hive_color(hc.x(), hc.y(), hc.z(), 1.0f);
        ImPlotSpec center_spec;
        center_spec.Marker = ImPlotMarker_Circle;
        center_spec.MarkerSize = 6.0f;
        center_spec.MarkerFillColor = hive_color;
        center_spec.MarkerLineColor = hive_color;
        ImPlot::PlotScatter("##c", &cx, &cy, 1, center_spec);
    }

    ImPlot::EndPlot();
}

} // namespace ui
