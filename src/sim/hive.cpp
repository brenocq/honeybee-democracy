//--------------------------------------------------
// Honeybee Democracy
// hive.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <sim/hive.hpp>

#include <algorithm>
#include <sim/nest_box.hpp>

namespace sim {

Hive::Hive(Eigen::Vector2f position, Eigen::Vector3f color, std::array<double, 4> gene, size_t num_bees, uint32_t seed)
    : _position(position), _color(color), _gene(gene), _bees(num_bees), _rng(seed) {
    spawn_bees();
}

void Hive::reset(Eigen::Vector2f position, std::array<double, 4> gene) {
    _position = position;
    _gene = gene;
    _fitness = 0.0f;
    spawn_bees();
}

void Hive::set_nest_boxes(const std::vector<NestBox>* nest_boxes) {
    _nest_boxes = nest_boxes;
    _consensus.assign(nest_boxes->size(), 0);
    _choice_prob.assign(nest_boxes->size(), 0.0f);
}

void Hive::update_consensus() {
    std::fill(_consensus.begin(), _consensus.end(), 0);

    for (const auto& bee : _bees) {
        const int choice = bee.choice();
        if (choice >= 0 && bee.state() == Bee::State::Dance)
            _consensus[choice]++;
    }

    const float total_bees = static_cast<float>(_bees.size());
    for (size_t i = 0; i < _consensus.size(); i++) {
        const float frac = static_cast<float>(_consensus[i]) / total_bees;
        const float g = (*_nest_boxes)[i].goodness();
        _fitness = std::max(_fitness, g * frac);
        _choice_prob[i] = frac;
    }
}

void Hive::step_bees(size_t steps) {
    update_consensus();
    while (steps--) {
        for (auto& bee : _bees)
            bee.step(_rng, _position, *_nest_boxes, _choice_prob);
    }
}

void Hive::spawn_bees() {
    std::normal_distribution<float> gauss_x(_position.x(), 0.03f);
    std::normal_distribution<float> gauss_y(_position.y(), 0.03f);
    std::uniform_real_distribution<float> theta_dist(0.0f, 360.0f);

    for (auto& bee : _bees) {
        bee = Bee(Eigen::Vector2f(gauss_x(_rng), gauss_y(_rng)), theta_dist(_rng));
        bee.set_gene(_gene);
    }
}

} // namespace sim
