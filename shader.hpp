#pragma once

#include <bgfx/bgfx.h>

#include <string>

namespace shader {

auto load( const std::string _name ) -> bgfx::ShaderHandle;

} // namespace shader
