#pragma once

namespace camera {

// TODO: Implement
using camera_t = struct camera {
    camera() = default;
    camera( const camera& ) = default;
    camera( camera&& ) = default;
    ~camera() = default;
    auto operator=( const camera& ) -> camera& = default;
    auto operator=( camera&& ) -> camera& = default;
};

} // namespace camera
