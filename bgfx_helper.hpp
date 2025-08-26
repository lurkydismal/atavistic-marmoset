#pragma once

#include <bgfx/bgfx.h>

#include <ostream>
#include <sstream>

[[nodiscard]] auto operator<<( std::ostream& _outputStream,
                               const bgfx::ProgramHandle& _programHandle )
    -> std::ostream&;

template <>
struct std::formatter< bgfx::ProgramHandle, char > {
    constexpr auto parse( auto& _context ) { return ( _context.begin() ); }

    auto format( const bgfx::ProgramHandle& _value, auto& _context ) {
        std::string l_returnValue;

        // Build return value
        {
            std::ostringstream l_returnValueStream;

            l_returnValueStream << "{ ";

            bool l_isNotFirstFormatterIteration = false;

            l_returnValueStream
                << std::format( "{} {}: {} : {}", "uint16_t", "idx", _value.idx,
                                sizeof( _value.idx ) );

            l_returnValueStream << "}";
        }

        return ( std::format_to( _context.out(), "{}", l_returnValue ) );
    }
};
