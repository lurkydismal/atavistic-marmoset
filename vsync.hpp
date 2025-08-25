#pragma once

#include <cstdint>

// Software vsync implementation
namespace vsync {

enum class vsync_t : uint8_t {
    off = 0,
    unknownVsync,
};

auto init( const vsync_t _vsyncType, const float _desiredFPS ) -> bool;
void quit();

void begin();
void end();

} // namespace vsync
