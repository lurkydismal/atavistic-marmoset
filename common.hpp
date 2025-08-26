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
[[nodiscard]] static inline constexpr auto millisecondsToNanoseconds(
    const T _milliseconds ) -> size_t {
    return ( _milliseconds * g_oneMillisecondInNanoseconds );
}

// TODO: Implement
#if 0
template < typename T >
struct std::formatter< T, char > {
    constexpr auto parse( auto& _context ) { return ( _context.begin() ); }

    auto format( const T& _value, auto& _context ) {
        std::string l_returnValue;

        // Build return value
        {
            std::ostringstream l_returnValueStream;

            l_returnValueStream << "{ ";

            bool l_isNotFirstFormatterIteration = false;

            iterate_struct(
                std::ref( _value ),
                [ & ]( std::string_view _name, std::string_view _type,
                       const auto& _field, size_t, size_t _size ) {
                    if ( l_isNotFirstFormatterIteration ) {
                        l_returnValueStream << ", ";

                    } else {
                        l_isNotFirstFormatterIteration = true;
                    }

                    l_returnValueStream << std::format( "{} {}: {} : {}", _type,
                                                        _name, _field, _size );
                } );

            l_returnValueStream << "}";
        }

        return ( std::format_to( _context.out(), "{}", l_returnValue ) );
    }
};
#endif
