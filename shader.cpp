#include "shader.hpp"

#include <fstream>

#include "log.hpp"

namespace shader {

// TODO: Improve
auto load( const std::string _name ) -> bgfx::ShaderHandle {
    bgfx::ShaderHandle l_returnValue = BGFX_INVALID_HANDLE;

    {
        std::ifstream l_file( _name, ( std::ios::binary | std::ios::ate ) );

        if ( !l_file.is_open() ) {
            log::error( "TODO" );

            goto EXIT;
        }

        const std::streamsize l_size = l_file.tellg();

        l_file.seekg( 0, std::ios::beg );

        const bgfx::Memory* l_mem = bgfx::alloc( uint32_t( l_size + 1 ) );

        if ( !l_file.read( reinterpret_cast< char* >( l_mem->data ),
                           l_size ) ) {
            log::error( "TODO" );

            goto EXIT;
        }

        // NUL is required
        l_mem->data[ l_mem->size - 1 ] = '\0';

        l_returnValue = bgfx::createShader( l_mem );
    }

EXIT:
    return ( l_returnValue );
}

} // namespace shader
