//--------------------------------------------------
// Honeybee Democracy
// hive.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <Eigen/Dense>
#include <array>
#include <cstdint>
#include <random>
#include <sim/bee.hpp>
#include <vector>

namespace sim {

class NestBox;

class Hive {
  public:
    Hive(Eigen::Vector2f position, Eigen::Vector3f color, std::array<double, 4> gene, size_t num_bees, uint32_t seed);

    void reset(Eigen::Vector2f position, std::array<double, 4> gene);
    void set_nest_boxes(const std::vector<NestBox>* nest_boxes);

    void update_consensus();
    void step_bees(size_t steps);

    Eigen::Vector2f position() const { return _position; }
    Eigen::Vector3f color() const { return _color; }
    const std::array<double, 4>& gene() const { return _gene; }
    const std::vector<Bee>& bees() const { return _bees; }
    const std::vector<int>& consensus() const { return _consensus; }
    float fitness() const { return _fitness; }

  private:
    void spawn_bees();

    Eigen::Vector2f _position;
    Eigen::Vector3f _color;
    std::array<double, 4> _gene;
    std::vector<Bee> _bees;

    const std::vector<NestBox>* _nest_boxes{nullptr};

    std::vector<int> _consensus;     // bees-in-Dance counts per nest box
    std::vector<float> _choice_prob; // consensus fraction used by Rest→Follow draws
    float _fitness{0.0f};            // running max of consensus_frac * goodness

    std::mt19937 _rng;
};

} // namespace sim
