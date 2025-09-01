#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <array>

#include "controls.hpp"
#include "log.hpp"
#include "runtime.hpp"

namespace {

bgfx::VertexLayout vertexLayout2;

}

namespace runtime {

auto iterate( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        // TODO: Camera

        // Render
        {
            // Begin frame
            {
                // Background
                bgfx::touch( 0 );
            }

            {
                // simple model rotation
                static double l_t = 0.0;
                l_t += 0.016; // ~60fps step
                std::array< float, 16 > l_model{};
                // bx::mtxRotateY( l_model.data(), static_cast< float >( l_t )
                // );
                using mat4x4_t = std::array< float, 16 >;

#if 1
                {
#if 0
                    const bx::Vec3 l_at = { 0.0f, 1.0f, 0.0f };
                    const bx::Vec3 l_eye = { 0.0f, 1.0f, -2.5f };

                    mat4x4_t l_view{};
                    bx::mtxLookAt( l_view.data(), l_eye, l_at );
#endif
                    mat4x4_t l_view{};
                    bx::mtxLookAt( l_view.data(), _applicationState.cameraPos,
                                   _applicationState.cameraAt,
                                   _applicationState.cameraUp );

                    mat4x4_t l_proj{};
                    bx::mtxProj(
                        l_proj.data(), 60.0f,
                        _applicationState.width / _applicationState.height,
                        0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth );
                    bgfx::setViewTransform( 0, l_view.data(), l_proj.data() );

#if 1
                    // Set view 0 default viewport.
                    bgfx::setViewRect( 0, 0, 0, _applicationState.width,
                                       _applicationState.height );
#endif
                }
#endif

                // Create vb/ib once (position + uv)
                struct Vertex {
                    float x, y, z, u, v;
                };
                static const Vertex l_quadV[] = {
                    { .x = -1.f, .y = 1.f, .z = 0.0f, .u = 0.0f, .v = 0.0f },
                    { .x = 1.f, .y = 1.f, .z = 0.0f, .u = 1.0f, .v = 0.0f },
                    { .x = 1.f, .y = -1.f, .z = 0.0f, .u = 1.0f, .v = 1.0f },
                    { .x = -1.f, .y = -1.f, .z = 0.0f, .u = 0.0f, .v = 1.0f },
                };
                static const uint16_t l_quadI[] = { 0, 1, 2, 0, 2, 3 };

                static bool l_x = false;
                static bgfx::VertexBufferHandle l_quadVbh;
                static bgfx::IndexBufferHandle l_quadIbh;

                if ( !l_x ) {
                    vertexLayout2.begin()
                        .add( bgfx::Attrib::Position, 3,
                              bgfx::AttribType::Float )
                        .add( bgfx::Attrib::TexCoord0, 2,
                              bgfx::AttribType::Float )
                        .end();

                    // create once:
                    l_quadVbh = bgfx::createVertexBuffer(
                        bgfx::makeRef( l_quadV, sizeof( l_quadV ) ),
                        vertexLayout2 );
                    l_quadIbh = bgfx::createIndexBuffer(
                        bgfx::makeRef( l_quadI, sizeof( l_quadI ) ) );
                }

                const uint64_t l_state = BGFX_STATE_WRITE_RGB |
                                         BGFX_STATE_WRITE_A |
                                         BGFX_STATE_WRITE_Z |
                                         BGFX_STATE_MSAA
                                         // remove culling while debugging:
                                         // | BGFX_STATE_CULL_CCW
                                         | BGFX_STATE_DEPTH_TEST_LESS;

                mat4x4_t l_identity{};
                bx::mtxIdentity( l_identity.data() );
                bgfx::setTransform( l_identity.data() );
                bgfx::setVertexBuffer( 0, l_quadVbh );
                bgfx::setIndexBuffer( l_quadIbh );
                bgfx::setTexture( 0, _applicationState.s_texColor,
                                  _applicationState.meshes.front().texture );
                bgfx::setState( l_state );
                bgfx::submit( 0, _applicationState.shaderProgram );

                // submit all meshes
                for ( const auto& l_mesh : _applicationState.meshes ) {
                    if ( !bgfx::isValid( l_mesh.vbh ) ||
                         !bgfx::isValid( l_mesh.ibh ) ) {
                        continue;
                    }

                    // set model transform (per-mesh you could compute different
                    // transforms)
                    bgfx::setTransform( l_model.data() );

                    bgfx::setVertexBuffer( 0, l_mesh.vbh );
                    bgfx::setIndexBuffer( l_mesh.ibh );
#if 1
                    if ( bgfx::isValid( l_mesh.texture ) ) {
                        bgfx::setTexture( 0, _applicationState.s_texColor,
                                          l_mesh.texture );
                    }
#endif
                    bgfx::setState( BGFX_STATE_DEFAULT | BGFX_STATE_CULL_CCW |
                                    BGFX_STATE_BLEND_FUNC(
                                        BGFX_STATE_BLEND_SRC_ALPHA,
                                        BGFX_STATE_BLEND_INV_SRC_ALPHA ) );
                    bgfx::submit( 0, _applicationState.shaderProgram );
                }

                // TODO: Scene
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
                _applicationState.cameraPos.y += l_speed;
                _applicationState.cameraAt.y += l_speed;
            }
            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::down ) {
                _applicationState.cameraPos.y -= l_speed;
                _applicationState.cameraAt.y -= l_speed;
            }
            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::left ) {
                _applicationState.cameraPos.x -= l_speed;
                _applicationState.cameraAt.x -= l_speed;
            }
            if ( _applicationState.currentInput.direction ==
                 controls::direction_t::right ) {
                _applicationState.cameraPos.x += l_speed;
                _applicationState.cameraAt.x += l_speed;
            }
            if ( _applicationState.currentInput.button ==
                 controls::button_t::zoom ) {
                _applicationState.cameraPos.z += l_speed;
                _applicationState.cameraAt.z += l_speed;
            }
            if ( _applicationState.currentInput.button ==
                 controls::button_t::dilate ) {
                _applicationState.cameraPos.z -= l_speed;
                _applicationState.cameraAt.z -= l_speed;
            }
        }

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace runtime
