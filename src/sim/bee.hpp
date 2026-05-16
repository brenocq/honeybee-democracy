//--------------------------------------------------
// Honeybee Democracy
// bee.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <Eigen/Dense>
#include <array>
#include <random>
#include <vector>

namespace sim {

class NestBox;

class Bee {
  public:
    enum class State { Rest, SearchNewNestBox, FindNestBox, BackToHome, Dance };

    Bee() = default;
    Bee(Eigen::Vector2f position, float theta);

    void set_gene(const std::array<double, 4>& gene);

    void step(std::mt19937& rng, Eigen::Vector2f hive_pos, const std::vector<NestBox>& nest_boxes, const std::vector<float>& choice_prob);

    Eigen::Vector2f position() const { return _position; }
    float theta() const { return _theta; }
    State state() const { return _state; }
    int choice() const { return _choice; }
    float choice_goodness() const { return _choice_goodness; }

  private:
    Eigen::Vector2f _position{0.0f, 0.0f};
    float _theta{0.0f};
    float _velocity{3.0f};
    float _size{0.005f};
    State _state{State::Rest};
    int _choice{-1};
    float _choice_goodness{0.0f};
    float _dance_force{0.0f};

    // Gene (unpacked from the hive's gene[4])
    double _random_chance{0.0};         // Chance to start searching for a new nest box
    double _follow_chance{0.0};         // Chance to follow another dancing bee
    double _linear_decay{0.0};          // Per-step linear decay of dance force in [0, 1]
    double _dance_force_exponent{0.0};  // Maps to (0, 10): dance_force = goodness^(exp*10)
};

} // namespace sim
