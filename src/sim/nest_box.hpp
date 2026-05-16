//--------------------------------------------------
// Honeybee Democracy
// nest_box.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <Eigen/Dense>

namespace sim {

class NestBox {
  public:
    NestBox() = default;
    NestBox(Eigen::Vector2f position, float goodness) : _position(position), _goodness(goodness) {}

    Eigen::Vector2f position() const { return _position; }
    float size() const { return _size; }
    float goodness() const { return _goodness; }

  private:
    Eigen::Vector2f _position{0.0f, 0.0f};
    float _size{0.01f};
    float _goodness{0.0f};
};

} // namespace sim
