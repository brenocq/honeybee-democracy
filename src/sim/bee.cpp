//--------------------------------------------------
// Honeybee Democracy
// bee.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <sim/bee.hpp>

#include <cmath>
#include <sim/nest_box.hpp>

namespace sim {

namespace {
constexpr float kInHome = 0.06f;
constexpr float kDanceRadius = 0.005f;
constexpr float kMaxRotation = 20.0f;
constexpr float kPi = 3.14159265358979323846f;

float deg2rad(float deg) { return deg * kPi / 180.0f; }
float rad2deg(float rad) { return rad * 180.0f / kPi; }
} // namespace

Bee::Bee(Eigen::Vector2f position, float theta) : _position(position), _theta(theta) {}

void Bee::set_gene(const std::array<double, 4>& gene) {
    _random_chance = gene[0];
    _follow_chance = gene[1];
    _linear_decay = gene[2];
    _dance_force_exponent = gene[3];
}

void Bee::step(std::mt19937& rng, Eigen::Vector2f hive_pos, const std::vector<NestBox>& nest_boxes, const std::vector<float>& choice_prob) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    const float random = dist(rng);
    const float rand_x = dist(rng);
    const float rand_y = dist(rng);

    const Eigen::Vector2f from_hive = _position - hive_pos;
    const float angle_to_hive = rad2deg(std::atan2(from_hive.y(), from_hive.x()));
    const float dist_to_hive = from_hive.norm();
    const float step_len = _size * _velocity;

    switch (_state) {
        case State::Rest: {
            if (dist_to_hive > kInHome) {
                _theta = angle_to_hive - 180.0f;
                _position.x() += step_len * std::cos(deg2rad(_theta));
                _position.y() += step_len * std::sin(deg2rad(_theta));
            } else {
                _theta += random * kMaxRotation - kMaxRotation / 2.0f;
                _position.x() += 0.03f * step_len * std::cos(deg2rad(_theta));
                _position.y() += 0.03f * step_len * std::sin(deg2rad(_theta));
            }

            if (random < _random_chance) {
                _state = State::SearchNewNestBox;
            } else if (random < _random_chance + _follow_chance) {
                float sum = 0.0f;
                for (size_t i = 0; i < nest_boxes.size(); i++) {
                    sum += choice_prob[i];
                    if (random <= sum) {
                        _choice = static_cast<int>(i);
                        break;
                    }
                }
                if (_choice == -1)
                    break;
                _state = State::FindNestBox;
            }
            break;
        }
        case State::SearchNewNestBox: {
            if (_position.x() > 1.0f || _position.x() < -1.0f || _position.y() > 1.0f || _position.y() < -1.0f)
                _theta = angle_to_hive - 180.0f;
            _theta += random * kMaxRotation - kMaxRotation / 2.0f;
            _position.x() += step_len * std::cos(deg2rad(_theta));
            _position.y() += step_len * std::sin(deg2rad(_theta));

            for (size_t i = 0; i < nest_boxes.size(); i++) {
                const Eigen::Vector2f nb_pos = nest_boxes[i].position();
                const float nb_size = nest_boxes[i].size();
                const float dist_to_nb = (_position - nb_pos).norm();
                if (dist_to_nb <= nb_size * 2.0f) {
                    _state = State::BackToHome;
                    _choice = static_cast<int>(i);
                    _choice_goodness = nest_boxes[i].goodness();
                    _dance_force = _choice_goodness;
                }
            }
            break;
        }
        case State::FindNestBox: {
            const Eigen::Vector2f nb_pos = nest_boxes[_choice].position();
            const float nb_size = nest_boxes[_choice].size();
            const Eigen::Vector2f from_nb = _position - nb_pos;
            const float angle_to_nb = rad2deg(std::atan2(from_nb.y(), from_nb.x()));
            const float dist_to_nb = from_nb.norm();

            _theta = angle_to_nb - 180.0f;
            _theta += random * kMaxRotation - kMaxRotation / 2.0f;
            _position.x() += step_len * std::cos(deg2rad(_theta));
            _position.y() += step_len * std::sin(deg2rad(_theta));

            if (dist_to_nb < nb_size * 2.0f) {
                _choice_goodness = nest_boxes[_choice].goodness();
                _dance_force = std::pow(_choice_goodness, static_cast<float>(_dance_force_exponent) * 10.0f);
                _state = State::BackToHome;
            }
            break;
        }
        case State::BackToHome: {
            _theta = angle_to_hive - 180.0f;
            _position.x() += step_len * std::cos(deg2rad(_theta));
            _position.y() += step_len * std::sin(deg2rad(_theta));

            if (dist_to_hive < kInHome)
                _state = State::Dance;
            break;
        }
        case State::Dance: {
            if (dist_to_hive > kInHome)
                _theta = angle_to_hive - 180.0f;
            else
                _theta += random * kMaxRotation - kMaxRotation / 2.0f;

            _position.x() += 0.3f * step_len * std::cos(deg2rad(_theta));
            _position.y() += 0.3f * step_len * std::sin(deg2rad(_theta));
            _position.x() += 2.0f * rand_x * kDanceRadius - kDanceRadius;
            _position.y() += 2.0f * rand_y * kDanceRadius - kDanceRadius;

            _dance_force -= static_cast<float>(_linear_decay);

            if (_dance_force < 0.0f) {
                _choice = -1;
                _choice_goodness = 0.0f;
                _dance_force = 0.0f;
                _state = State::Rest;
            }
            break;
        }
    }
}

} // namespace sim
