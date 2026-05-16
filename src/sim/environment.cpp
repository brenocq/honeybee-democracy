//--------------------------------------------------
// Honeybee Democracy
// environment.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <sim/environment.hpp>

#include <algorithm>
#include <utility>

namespace sim {

namespace {
constexpr float kBorder = 0.9f;
constexpr size_t kPredationInterval = 15;
constexpr size_t kPredationDivisor = 10; // re-randomize worst 1/N of hives
} // namespace

Environment::Environment(const Config& config) : _config(config), _rng(config.seed) {
    _nest_boxes.resize(config.num_nest_boxes);
    randomize_nest_boxes();

    _hives.reserve(config.num_colonies);
    for (size_t i = 0; i < config.num_colonies; i++) {
        _hives.emplace_back(random_position(), random_color(), random_gene(), config.bees_per_colony, static_cast<uint32_t>(_rng()));
        _hives.back().set_nest_boxes(&_nest_boxes);
    }
}

void Environment::step(size_t steps) {
    for (auto& hive : _hives)
        hive.step_bees(steps);
    _step += steps;

    if (_step >= _config.steps_per_repetition)
        end_repetition();
}

void Environment::end_repetition() {
    _step = 0;
    _repetition++;

    randomize_nest_boxes();

    // Capture each hive's accumulated fitness from the just-finished repetition.
    // Hive::reset (below) will clear it back to 0.
    std::vector<float> rep_fitness;
    rep_fitness.reserve(_hives.size());
    for (const auto& hive : _hives)
        rep_fitness.push_back(hive.fitness());
    _repetition_fitness.push_back(std::move(rep_fitness));

    // Reset hives onto a new random spawn position; keep their genes for now.
    for (auto& hive : _hives)
        hive.reset(random_position(), hive.gene());

    if (_repetition >= _config.repetitions_per_generation)
        end_generation();
}

void Environment::end_generation() {
    // Mean fitness per hive across the generation's repetitions.
    std::vector<float> gen_fitness(_hives.size(), 0.0f);
    for (const auto& rep : _repetition_fitness)
        for (size_t i = 0; i < _hives.size(); i++)
            gen_fitness[i] += rep[i];
    const float n = static_cast<float>(_repetition_fitness.size());
    for (auto& f : gen_fitness)
        f /= n;
    _repetition_fitness.clear();
    _generation_fitness.push_back(gen_fitness);

    _generation++;
    _repetition = 0;

    // Crossover + mutation: pull every non-best hive halfway toward the best
    // gene, with a per-coordinate uniform jitter scaled by the best gene.
    size_t best_index = 0;
    for (size_t i = 1; i < _hives.size(); i++)
        if (gen_fitness[i] > gen_fitness[best_index])
            best_index = i;

    const std::array<double, 4> best_gene = _hives[best_index].gene();
    std::uniform_real_distribution<double> mut_delta(-0.5, 0.5);

    for (size_t i = 0; i < _hives.size(); i++) {
        if (i == best_index)
            continue;

        std::array<double, 4> gene = _hives[i].gene();
        for (int j = 0; j < 4; j++) {
            do {
                gene[j] = best_gene[j] * 0.5 + gene[j] * 0.5 + best_gene[j] * mut_delta(_rng);
            } while (gene[j] < 0.0 || gene[j] > 1.0);
        }
        _hives[i].reset(random_position(), gene);
    }

    // Predation: every Nth generation, replace the worst 1/N of hives with fresh random genes.
    if (_generation % kPredationInterval == 0) {
        std::vector<std::pair<float, size_t>> ranked;
        ranked.reserve(_hives.size());
        for (size_t i = 0; i < _hives.size(); i++)
            ranked.emplace_back(gen_fitness[i], i);
        std::sort(ranked.begin(), ranked.end());

        const size_t kill_count = _hives.size() / kPredationDivisor;
        for (size_t k = 0; k < kill_count; k++)
            _hives[ranked[k].second].reset(random_position(), random_gene());
    }
}

void Environment::randomize_nest_boxes() {
    std::uniform_real_distribution<float> pos_dist(-kBorder, kBorder);
    std::uniform_real_distribution<float> good_dist(0.0f, 1.0f);

    for (size_t i = 0; i < _nest_boxes.size(); i++) {
        const Eigen::Vector2f pos(pos_dist(_rng), pos_dist(_rng));
        // The first box is always the "ideal" nest with maximum goodness.
        const float goodness = (i == 0) ? 1.0f : good_dist(_rng);
        _nest_boxes[i] = NestBox(pos, goodness);
    }
}

Eigen::Vector2f Environment::random_position() {
    std::uniform_real_distribution<float> dist(-kBorder, kBorder);
    return {dist(_rng), dist(_rng)};
}

Eigen::Vector3f Environment::random_color() {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return {dist(_rng), dist(_rng), dist(_rng)};
}

std::array<double, 4> Environment::random_gene() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return {dist(_rng), dist(_rng), dist(_rng), dist(_rng)};
}

} // namespace sim
