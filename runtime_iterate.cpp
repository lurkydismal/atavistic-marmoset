#include <bx/math.h>

#include "runtime.hpp"

namespace runtime {

auto iterate( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        // TODO: Camera

        // Render
        {
            // Begin frame
            {
                bgfx::touch( 0 );
            }

            {
                // simple model rotation
                static double l_t = 0.0;
                l_t += 0.016; // ~60fps step

                // Build view/proj once per-frame (or cache them if static)
                float l_view[ 16 ];
                float l_proj[ 16 ];
                bx::Vec3 l_eye{ 0.0f, 0.0f, -5.0f };
                bx::Vec3 l_at{ 0.0f, 0.0f, 0.0f };
                bx::mtxLookAt( l_view, l_eye, l_at );
                constexpr float l_FOV = 60.0f;
                const float l_aspect =
                    ( _applicationState.width / _applicationState.height );
                bx::mtxProj( l_proj, l_FOV, l_aspect, 0.1f, 100.0f,
                             bgfx::getCaps()->homogeneousDepth );

                // for each mesh
                for ( const auto& l_mesh : _applicationState.meshes ) {
                    if ( !bgfx::isValid( l_mesh.vbh ) ||
                         !bgfx::isValid( l_mesh.ibh ) )
                        continue;

                    // model rotation
                    float l_model[ 16 ];
                    bx::mtxRotateY( l_model, float( l_t ) );

                    // compute mvp = proj * view * model
                    float l_viewModel[ 16 ];
                    bx::mtxMul( l_viewModel, l_view, l_model ); // view * model
                    float l_mvp[ 16 ];
                    bx::mtxMul( l_mvp, l_proj,
                                l_viewModel ); // proj * (view * model)

                    // upload mvp uniform
                    if ( bgfx::isValid( _applicationState.u_modelViewProj ) ) {
                        bgfx::setUniform( _applicationState.u_modelViewProj,
                                          l_mvp );
                    }

                    bgfx::setTransform(
                        l_model ); // optional (bgfx uses transform stack too)
                    bgfx::setVertexBuffer( 0, l_mesh.vbh );
                    bgfx::setIndexBuffer( l_mesh.ibh );

                    if ( bgfx::isValid( l_mesh.texture ) ) {
                        bgfx::setTexture( 0, _applicationState.s_texColor,
                                          l_mesh.texture );
                    }

                    bgfx::setState( BGFX_STATE_DEFAULT );
                    bgfx::submit( 0, _applicationState.shaderProgram );
                }

                // TODO: Background
                // TODO: Scene
            }

            // End frame
            {
                bgfx::frame();
            }
        }

        // TODO: Logic
        // TODO: Handle application current input

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace runtime
