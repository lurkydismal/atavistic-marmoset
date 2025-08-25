#pragma once

#include <atomic>

namespace FPS {

void init( std::atomic< size_t >& _frameCount );
void quit();

} // namespace FPS
