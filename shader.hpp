#pragma once

#include <bgfx/bgfx.h>

#include <string>

namespace shader {

[[nodiscard]] auto load( const std::string _name ) -> bgfx::ShaderHandle;

} // namespace shader
