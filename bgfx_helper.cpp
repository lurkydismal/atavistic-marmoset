#include "bgfx_helper.hpp"

auto operator<<( std::ostream& _outputStream,
                 const bgfx::ProgramHandle& _programHandle ) -> std::ostream& {
    return ( _outputStream << _programHandle.idx );
}
