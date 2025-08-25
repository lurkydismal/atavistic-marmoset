#pragma once

#include <cstdint>
#include <string_view>

#include "vsync.hpp"

namespace window {

using window_t = struct window {
    window() = default;
    window( const window& ) = default;
    window( window&& ) = default;
    ~window() = default;
    auto operator=( const window& ) -> window& = default;
    auto operator=( window&& ) -> window& = default;

    static inline constexpr const std::string_view name = "atavistic-marmoset";
    size_t width = 640;
    size_t height = 480;
    size_t desiredFPS = 60;
    vsync::vsync_t vsync = vsync::vsync_t::off;
};

} // namespace window
