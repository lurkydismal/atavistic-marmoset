#include <bgfx/defines.h>

#include <print>

#include "assimp_helder.hpp"
#include "bgfx_helper.hpp"
#include "log.hpp"
#include "runtime.hpp"
#include "shader.hpp"

namespace runtime {

namespace {

bgfx::VertexLayout g_vertexLayout;

} // namespace

// TODO: Implement
auto applicationState_t::load() -> bool {
    bool l_returnValue = false;

    do {
        if ( vertexShaderPath.empty() ) {
            log::variable( vertexShaderPath );

            log::error( "Vertex shader path is empty" );

            break;
        }

        if ( fragmentShaderPath.empty() ) {
            log::variable( fragmentShaderPath );

            log::error( "Fragment shader path is empty" );

            break;
        }

        if ( bgfx::isValid( shaderProgramHandle ) ) {
            log::variable( shaderProgramHandle );

            log::error( "Shader program is already loaded" );

            break;
        }

        {
            // Build shader program
            {
                log::info( "Loading vertex shader" );

                bgfx::ShaderHandle l_vertexShaderHandle =
                    shader::load( vertexShaderPath );

                if ( !bgfx::isValid( l_vertexShaderHandle ) ) {
                    log::error( "Loading vertex shader" );

                    break;
                }

                log::info( "Loading fragment shader" );

                bgfx::ShaderHandle l_fragmentShaderHandle =
                    shader::load( fragmentShaderPath );

                if ( !bgfx::isValid( l_fragmentShaderHandle ) ) {
                    log::error( "Loading fragment shader" );

                    bgfx::destroy( l_vertexShaderHandle );

                    break;
                }

                log::info( "Compiling shaders" );

                shaderProgramHandle = bgfx::createProgram(
                    l_vertexShaderHandle, l_fragmentShaderHandle, true );

                if ( !bgfx::isValid( shaderProgramHandle ) ) {
                    log::error( "Compiling shaders" );

                    break;
                }
            }

            // TODO: Describe scope
            {
                g_vertexLayout.begin()
                    .add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
                    .add( bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float )
                    .end();

                textureColorSampler = bgfx::createUniform(
                    "s_texColor", bgfx::UniformType::Sampler );
            }

            // Load model
            {
                loadMesh( mesh, g_vertexLayout, modelPath );
            }
        }

        l_returnValue = true;
    } while ( false );

    return ( l_returnValue );
}

} // namespace runtime
