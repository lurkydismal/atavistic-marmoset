#pragma once

#include <bx/math.h>

namespace camera {

// TODO: Implement
using camera_t = struct camera {
    camera() = default;
    camera( const camera& ) = default;
    camera( camera&& ) = default;
    ~camera() = default;
    auto operator=( const camera& ) -> camera& = default;
    auto operator=( camera&& ) -> camera& = default;

    bx::Vec3 position = { 0.0f, 1.0f, -3.0f };
    bx::Vec3 at = { 0.0f, 1.0f, 0.0f };
    bx::Vec3 up = { 0.0f, 1.0f, 0.0f };
};

} // namespace camera
