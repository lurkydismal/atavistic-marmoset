#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace {

// Control
static inline constexpr const std::string_view g_controlAsStringUnknown =
    "UNKNOWN";

} // namespace

namespace controls {

enum class direction_t : uint8_t {
    none = 0,
    up = 0b1,
    down = 0b10,
    left = 0b100,
    right = 0b1000,
};

inline constexpr auto operator|=( direction_t& _lhs, direction_t _rhs )
    -> direction_t& {
    using directionType_t = std::underlying_type_t< direction_t >;

    _lhs = static_cast< direction_t >( static_cast< directionType_t >( _lhs ) |
                                       static_cast< directionType_t >( _rhs ) );

    return ( _lhs );
}

inline constexpr auto operator|( direction_t _lhs, direction_t _rhs )
    -> direction_t {
    using directionType_t = std::underlying_type_t< direction_t >;

    return (
        static_cast< direction_t >( static_cast< directionType_t >( _lhs ) |
                                    static_cast< directionType_t >( _rhs ) ) );
}

inline constexpr auto operator&( direction_t _lhs, direction_t _rhs )
    -> direction_t {
    using directionType_t = std::underlying_type_t< direction_t >;

    return (
        static_cast< direction_t >( static_cast< directionType_t >( _lhs ) &
                                    static_cast< directionType_t >( _rhs ) ) );
}

enum class button_t : uint8_t {
    none = 0,
};

inline constexpr auto operator|=( button_t& _lhs, button_t _rhs ) -> button_t& {
    using buttonType_t = std::underlying_type_t< button_t >;

    _lhs = static_cast< button_t >( static_cast< buttonType_t >( _lhs ) |
                                    static_cast< buttonType_t >( _rhs ) );

    return ( _lhs );
}

inline constexpr auto operator|( button_t _lhs, button_t _rhs ) -> button_t {
    using buttonType_t = std::underlying_type_t< button_t >;

    return ( static_cast< button_t >( static_cast< buttonType_t >( _lhs ) |
                                      static_cast< buttonType_t >( _rhs ) ) );
}

inline constexpr auto operator&( button_t _lhs, button_t _rhs ) -> button_t {
    using buttonType_t = std::underlying_type_t< button_t >;

    return ( static_cast< button_t >( static_cast< buttonType_t >( _lhs ) &
                                      static_cast< buttonType_t >( _rhs ) ) );
}

using input_t = struct input {
    input() = default;
    input( const input& ) = default;
    input( input&& ) = default;
    ~input() = default;
    auto operator=( const input& ) -> input& = default;
    auto operator=( input&& ) -> input& = default;

    direction_t direction = direction_t::none;
    button_t button = button_t::none;
    size_t duration = 0;
};

using control_t = struct control {
    control() = default;
    control( const control& ) = default;
    control( control&& ) = default;
    ~control() = default;
    auto operator=( const control& ) -> control& = default;
    auto operator=( control&& ) -> control& = default;

    inline constexpr auto check( const SDL_Scancode _scancode ) -> bool {
        return ( scancode == _scancode );
    }

    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    input_t input;
};

// All available controls
using controls_t = struct controls {
    controls() = default;
    controls( const controls& ) = default;
    controls( controls&& ) = default;
    ~controls() = default;
    auto operator=( const controls& ) -> controls& = default;
    auto operator=( controls&& ) -> controls& = default;

    inline constexpr auto get( const SDL_Scancode _scancode )
        -> const control_t {
        if ( up.check( _scancode ) ) {
            return ( up );

        } else if ( down.check( _scancode ) ) {
            return ( down );

        } else if ( left.check( _scancode ) ) {
            return ( left );

        } else if ( right.check( _scancode ) ) {
            return ( right );
        }

        return {};
    }

    // Directions
    control_t up;
    control_t down;
    control_t left;
    control_t right;
};

} // namespace controls
