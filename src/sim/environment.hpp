//--------------------------------------------------
// Honeybee Democracy
// environment.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <sim/hive.hpp>
#include <sim/nest_box.hpp>
#include <vector>

namespace sim {

struct Config {
    size_t num_colonies = 10;
    size_t bees_per_colony = 100;
    size_t num_nest_boxes = 20;

    // Evaluation
    size_t steps_per_repetition = 5000;
    size_t repetitions_per_generation = 5;
    uint32_t seed = 42;

    // Genetic algorithm
    size_t predation_interval = 15;     // generations between predation events
    float predation_fraction = 0.10f;   // fraction of worst hives to re-randomize
    double mutation_amplitude = 0.5;    // uniform jitter half-range applied during crossover
};

class Environment {
  public:
    explicit Environment(const Config& config);

    // Non-copyable, non-movable: each Hive holds a raw pointer to _nest_boxes,
    // which would dangle if the Environment were relocated.
    ~Environment() = default;
    Environment(const Environment&) = delete;
    Environment(Environment&&) = delete;
    Environment& operator=(const Environment&) = delete;
    Environment& operator=(Environment&&) = delete;

    // Advance every hive by `steps` simulation steps. Handles end-of-repetition
    // and end-of-generation transitions internally (resampling nest boxes,
    // crossover/mutation, predation).
    void step(size_t steps);

    const Config& config() const { return _config; }
    const std::vector<NestBox>& nest_boxes() const { return _nest_boxes; }
    const std::vector<Hive>& hives() const { return _hives; }

    size_t current_step() const { return _step; }
    size_t current_repetition() const { return _repetition; }
    size_t current_generation() const { return _generation; }

    // generation_fitness[generation][hive_index] = mean fitness over the generation's repetitions
    const std::vector<std::vector<float>>& generation_fitness() const { return _generation_fitness; }

  private:
    void end_repetition();
    void end_generation();
    void randomize_nest_boxes();

    Eigen::Vector2f random_position();
    std::array<double, 4> random_gene();

    Config _config;
    std::vector<NestBox> _nest_boxes;
    std::vector<Hive> _hives;

    std::vector<std::vector<float>> _repetition_fitness; // cleared at each generation boundary
    std::vector<std::vector<float>> _generation_fitness; // grows once per generation

    size_t _step{0};
    size_t _repetition{0};
    size_t _generation{0};

    std::mt19937 _rng;
};

} // namespace sim
