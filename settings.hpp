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
    controls::controls_t controls{
        { SDL_SCANCODE_K, { controls::direction_t::up } },
        { SDL_SCANCODE_J, { controls::direction_t::down } },
        { SDL_SCANCODE_H, { controls::direction_t::left } },
        { SDL_SCANCODE_L, { controls::direction_t::right } },
        { SDL_SCANCODE_I, { controls::button_t::zoom } },
        { SDL_SCANCODE_O, { controls::button_t::dilate } } };
    static inline constexpr std::string_view version = "0.1";
    static inline constexpr std::string_view identifier =
        window::window_t::name;
};

} // namespace settings
