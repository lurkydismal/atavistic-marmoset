#pragma once

#include <bgfx/bgfx.h>

#include <format>
#include <ostream>
#include <sstream>

[[nodiscard]] auto operator<<( std::ostream& _outputStream,
                               const bgfx::ProgramHandle& _programHandle )
    -> std::ostream&;

template <>
struct std::formatter< bgfx::ProgramHandle, char > {
    constexpr auto parse( std::format_parse_context& _context ) {
        return ( _context.begin() );
    }

    auto format( const bgfx::ProgramHandle& _value,
                 std::format_context& _context ) const {
        return ( std::format_to( _context.out(), "{{ uint16_t {}: {} : {} }}",
                                 "idx", _value.idx, sizeof( _value.idx ) ) );
    }
};
