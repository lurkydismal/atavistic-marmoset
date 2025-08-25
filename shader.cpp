#include "shader.hpp"

#include <cstdint>
#include <fstream>
#include <span>

#include "log.hpp"

namespace shader {

auto load( const std::string _name ) -> bgfx::ShaderHandle {
    bgfx::ShaderHandle l_returnValue = BGFX_INVALID_HANDLE;

    {
        std::ifstream l_inputFileStream( _name,
                                         ( std::ios::binary | std::ios::ate ) );

        bool l_result = l_inputFileStream.good();

        if ( !l_result ) {
            log::error( "TODO" );

            goto EXIT;
        }

        const std::streamsize l_fileSize = l_inputFileStream.tellg();

        l_inputFileStream.seekg( 0, std::ios::beg );

        const bgfx::Memory* l_shaderInMemory = bgfx::alloc( l_fileSize + 1 );

        // Build memory
        {
            const auto l_shaderInMemoryView =
                std::span( reinterpret_cast< char* >( l_shaderInMemory->data ),
                           l_shaderInMemory->size );

            l_result = !!( l_inputFileStream.read( l_shaderInMemoryView.data(),
                                                   l_fileSize ) );

            if ( !l_result ) {
                log::error( "TODO" );

                goto EXIT;
            }

            // NUL is required
            l_shaderInMemoryView.back() = '\0';
        }

        l_returnValue = bgfx::createShader( l_shaderInMemory );
    }

EXIT:
    return ( l_returnValue );
}

} // namespace shader
