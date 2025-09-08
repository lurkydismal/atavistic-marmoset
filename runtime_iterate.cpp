#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <array>

#include "controls.hpp"
#include "runtime.hpp"

namespace runtime {

auto iterate( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        using mat4x4_t = std::array< float, 16 >;

        // Camera
        {
            mat4x4_t l_view{};
            mat4x4_t l_projection{};

            bx::mtxLookAt( l_view.data(), _applicationState.camera.position,
                           _applicationState.camera.at,
                           _applicationState.camera.up );

            const float l_FOX = 60;

            bx::mtxProj( l_projection.data(), l_FOX,
                         ( _applicationState.width / _applicationState.height ),
                         0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth );

            bgfx::setViewTransform( 0, l_view.data(), l_projection.data() );
        }

        // Render
        {
            // Begin frame
            {
                // Background
                bgfx::touch( 0 );
            }

            // Scene
            {
                std::array< float, 16 > l_model{};

                // Rotate model
                {
                    static double l_t = 0.0;
                    // ~60fps step
                    l_t += 0.016;
                    bx::mtxRotateY( l_model.data(),
                                    static_cast< float >( l_t ) );
                }

                // Submit mesh
                {
                    const auto& l_mesh = _applicationState.mesh;

                    bgfx::setTransform( l_model.data() );

                    bgfx::setVertexBuffer( 0, l_mesh.vertexBufferHandle );
                    bgfx::setIndexBuffer( l_mesh.indexBufferHandle );

                    bgfx::setTexture( 0, _applicationState.textureColorSampler,
                                      l_mesh.textureHandle );

                    bgfx::setState( BGFX_STATE_DEFAULT | BGFX_STATE_CULL_CCW |
                                    BGFX_STATE_BLEND_FUNC(
                                        BGFX_STATE_BLEND_SRC_ALPHA,
                                        BGFX_STATE_BLEND_INV_SRC_ALPHA ) );

                    bgfx::submit( 0, _applicationState.shaderProgramHandle );
                }
            }

            // End frame
            {
                bgfx::frame();
            }
        }

        // Logic
        {
            float l_speed = 0.05f;

            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::up ) {
                _applicationState.camera.position.y += l_speed;
                _applicationState.camera.at.y += l_speed;
            }
            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::down ) {
                _applicationState.camera.position.y -= l_speed;
                _applicationState.camera.at.y -= l_speed;
            }
            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::left ) {
                _applicationState.camera.position.x -= l_speed;
                _applicationState.camera.at.x -= l_speed;
            }
            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::right ) {
                _applicationState.camera.position.x += l_speed;
                _applicationState.camera.at.x += l_speed;
            }
            if ( _applicationState.currentInput.button ==
                 controls::button_t::zoom ) {
                _applicationState.camera.position.z += l_speed;
                _applicationState.camera.at.z += l_speed;
            }
            if ( _applicationState.currentInput.button ==
                 controls::button_t::dilate ) {
                _applicationState.camera.position.z -= l_speed;
                _applicationState.camera.at.z -= l_speed;
            }
        }

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace runtime
