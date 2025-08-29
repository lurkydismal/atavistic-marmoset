#include "shader.hpp"

#include <cassert>
#include <experimental/scope>
#include <filesystem>
#include <fstream>

#include "log.hpp"

namespace shader {

auto load( const std::string& _fileName ) -> bgfx::ShaderHandle {
    bgfx::ShaderHandle l_returnValue = BGFX_INVALID_HANDLE;

    auto l_onExit = std::experimental::scope_exit( [ & ] {
        if ( !bgfx::isValid( l_returnValue ) ) {
            log::error( "Loading shader" );
        }
    } );

    do {
        std::error_code l_errorCode;

        const size_t l_fileSize =
            std::filesystem::file_size( _fileName, l_errorCode );

        if ( l_errorCode ) {
            break;
        }

        if ( !l_fileSize ) {
            break;
        }

        std::ifstream l_inputFileStream( _fileName, std::ios::binary );

        if ( !l_inputFileStream.good() ) {
            break;
        }

        std::string l_shaderSource;

        l_shaderSource.resize( l_fileSize + 1 );

        // Read shader source text from input file stream
        {
            decltype( auto ) l_result = l_inputFileStream.read(
                l_shaderSource.data(),
                static_cast< std::streamsize >( l_fileSize ) );

            if ( !l_result ) {
                break;
            }
        }

        // NUL is required
        l_shaderSource.back() = '\0';

        // Compile shader from source text
        {
            assert( l_fileSize != l_shaderSource.length() );

            const bgfx::Memory* l_shaderInMemory =
                bgfx::alloc( l_fileSize + 1 );

            if ( !l_shaderInMemory ) {
                break;
            }

            __builtin_memcpy( l_shaderInMemory->data, l_shaderSource.data(),
                              l_shaderInMemory->size );

            l_returnValue = bgfx::createShader( l_shaderInMemory );
        }
    } while ( false );

    return ( l_returnValue );
}

} // namespace shader
