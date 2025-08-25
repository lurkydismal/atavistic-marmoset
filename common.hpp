#pragma once

#include <cstddef>

// Function-like macros
// Universal
#define STRINGIFY( _value ) #_value
#define MACRO_TO_STRING( _macro ) STRINGIFY( _macro )

// Constants
// Universal
static inline constexpr const size_t g_oneSecondInMilliseconds = 1000;
static inline constexpr const size_t g_oneMillisecondInNanoseconds = 1000000;

template < typename T >
static inline constexpr auto millisecondsToNanoseconds( const T _milliseconds )
    -> size_t {
    return ( _milliseconds * g_oneMillisecondInNanoseconds );
}
