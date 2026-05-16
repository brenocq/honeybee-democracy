//--------------------------------------------------
// Honeybee Democracy
// simulation_window.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <memory>
#include <sim/environment.hpp>

namespace ui {

class SimulationWindow {
  public:
    void render();

    const sim::Environment* environment() const { return _environment.get(); }

  private:
    enum class State { Idle, Running, Paused };

    void render_config();
    void render_controls();
    void render_status();
    void render_environment_plot();
    void render_fitness_plot();
    void render_consensus_plot();
    void step_env();

    State _state{State::Idle};
    sim::Config _config{};
    int _steps_offline{100}; // smaller than the old code's 1000 so the main-thread loop stays responsive
    bool _realtime{false};
    double _last_realtime_step{0.0};
    bool _reset_env_axes{false}; // one-shot: forces the env plot's Y axis to [-1, 1] on the next frame
    std::unique_ptr<sim::Environment> _environment;
};

} // namespace ui
