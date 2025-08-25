#pragma once

#include "controls.hpp"
#include "window.hpp"

namespace settings {

// All available customization
using settings_t = struct settings {
    settings() = default;
    settings( const settings& ) = default;
    settings( settings&& ) = default;
    ~settings() = default;
    auto operator=( const settings& ) -> settings& = default;
    auto operator=( settings&& ) -> settings& = default;

    window::window_t window;
    controls::controls_t controls;
    static inline constexpr const std::string_view version = "0.1";
    static inline constexpr const std::string_view identifier =
        window::window_t::name;
};

} // namespace settings
