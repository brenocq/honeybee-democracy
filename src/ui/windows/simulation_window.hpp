//--------------------------------------------------
// Honeybee Democracy
// simulation_window.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

namespace ui {

class SimulationWindow {
  public:
    void render();

  private:
    void render_config();

    // Simulation parameters
    int _num_colonies = 10;
    int _bees_per_colony = 100;
    int _num_nest_boxes = 20;
    int _steps_per_repetition = 5000;
    int _repetitions_per_generation = 5;
    int _steps_offline = 1000;
    int _seed = 42;
};

} // namespace ui
