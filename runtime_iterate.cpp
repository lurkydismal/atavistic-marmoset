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
                float l_model[ 16 ];
                bx::mtxRotateY( l_model, float( l_t ) );

                // submit all meshes
                for ( const auto& l_mesh : _applicationState.meshes ) {
                    if ( !bgfx::isValid( l_mesh.vbh ) ||
                         !bgfx::isValid( l_mesh.ibh ) )
                        continue;

                    // set model transform (per-mesh you could compute different
                    // transforms)
                    bgfx::setTransform( l_model );

                    bgfx::setVertexBuffer( 0, l_mesh.vbh );
                    bgfx::setIndexBuffer( l_mesh.ibh );
#if 0
                    if ( bgfx::isValid( l_mesh.texture ) ) {
                        bgfx::setTexture( 0, _applicationState.s_texColor,
                                          l_mesh.texture );
                    }
#endif
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
