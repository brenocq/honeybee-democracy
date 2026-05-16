//--------------------------------------------------
// Honeybee Democracy
// color.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <Eigen/Dense>
#include <cstddef>

namespace sim {

// Returns palette[index % palette.size()] from the shared 64-color table
// (copied from state-estimation-playground's ui/color.cpp).
Eigen::Vector3f palette_color(size_t index);

} // namespace sim
