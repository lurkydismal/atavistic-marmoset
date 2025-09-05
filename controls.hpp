#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace controls {

enum class direction_t : uint8_t {
    none = 0,
    up = 0b1,
    down = 0b10,
    left = 0b100,
    right = 0b1000,
};

[[nodiscard]] constexpr auto operator|( direction_t _lhs, direction_t _rhs )
    -> direction_t {
    using directionType_t = std::underlying_type_t< direction_t >;

    return (
        static_cast< direction_t >( static_cast< directionType_t >( _lhs ) |
                                    static_cast< directionType_t >( _rhs ) ) );
}

[[nodiscard]] constexpr auto operator&( direction_t _lhs, direction_t _rhs )
    -> direction_t {
    using directionType_t = std::underlying_type_t< direction_t >;

    return (
        static_cast< direction_t >( static_cast< directionType_t >( _lhs ) &
                                    static_cast< directionType_t >( _rhs ) ) );
}

constexpr void operator|=( direction_t& _lhs, direction_t _rhs ) {
    _lhs = ( _lhs | _rhs );
}

enum class button_t : uint8_t {
    none = 0,
    zoom = 0b1,
    dilate = 0b10,
};

[[nodiscard]] constexpr auto operator|( button_t _lhs, button_t _rhs )
    -> button_t {
    using buttonType_t = std::underlying_type_t< button_t >;

    return ( static_cast< button_t >( static_cast< buttonType_t >( _lhs ) |
                                      static_cast< buttonType_t >( _rhs ) ) );
}

[[nodiscard]] constexpr auto operator&( button_t _lhs, button_t _rhs )
    -> button_t {
    using buttonType_t = std::underlying_type_t< button_t >;

    return ( static_cast< button_t >( static_cast< buttonType_t >( _lhs ) &
                                      static_cast< buttonType_t >( _rhs ) ) );
}

constexpr void operator|=( button_t& _lhs, button_t _rhs ) {
    _lhs = ( _lhs | _rhs );
}

using input_t = struct input {
    input( direction_t _direction, button_t _button = button_t::none )
        : direction( _direction ), button( _button ) {}
    input( button_t _button, direction_t _direction = direction_t::none )
        : direction( _direction ), button( _button ) {}
    input() = default;
    input( const input& ) = default;
    input( input&& ) = default;
    ~input() = default;
    auto operator=( const input& ) -> input& = default;
    auto operator=( input&& ) -> input& = default;

    direction_t direction = direction_t::none;
    button_t button = button_t::none;
};

using control_t = struct control {
    control( SDL_Scancode _scancode, input_t _input )
        : scancode( _scancode ), input( _input ) {}
    control() = default;
    control( const control& ) = default;
    control( control&& ) = default;
    ~control() = default;
    auto operator=( const control& ) -> control& = default;
    auto operator=( control&& ) -> control& = default;

    [[nodiscard]] inline constexpr auto check(
        const SDL_Scancode _scancode ) const -> bool {
        return ( scancode == _scancode );
    }

    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    input_t input;
};

// All available controls
using controls_t = struct controls {
    controls( control_t _up,
              control_t _down,
              control_t _left,
              control_t _right,
              control_t _zoom,
              control_t _dilate )
        : up( _up ),
          down( _down ),
          left( _left ),
          right( _right ),
          zoom( _zoom ),
          dilate( _dilate ) {}
    controls() = default;
    controls( const controls& ) = default;
    controls( controls&& ) = default;
    ~controls() = default;
    auto operator=( const controls& ) -> controls& = default;
    auto operator=( controls&& ) -> controls& = default;

    [[nodiscard]] inline constexpr auto get(
        const SDL_Scancode _scancode ) const -> const control_t {
        for ( auto& _control : { up, down, left, right, zoom, dilate } ) {
            if ( _control.check( _scancode ) ) {
                return ( _control );
            }
        }

        return {};
    }

    // Directions
    control_t up;
    control_t down;
    control_t left;
    control_t right;

    // Actions
    control_t zoom;
    control_t dilate;
};

} // namespace controls
