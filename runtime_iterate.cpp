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

            // simple model rotation
#if 0
            static double t = 0.0;
            t += 0.016; // ~60fps step
            float model[ 16 ];
            bx::mtxRotateY( model, float( t ) );

            // submit all meshes
            for ( const auto& mesh : meshes ) {
                if ( !bgfx::isValid( mesh.vbh ) || !bgfx::isValid( mesh.ibh ) )
                    continue;

                // set model transform (per-mesh you could compute different
                // transforms)
                bgfx::setTransform( model );

                bgfx::setVertexBuffer( 0, mesh.vbh );
                bgfx::setIndexBuffer( mesh.ibh );
                if ( bgfx::isValid( mesh.texture ) ) {
                    bgfx::setTexture( 0, s_texColor, mesh.texture );
                }
                bgfx::setState( BGFX_STATE_DEFAULT );
                bgfx::submit( 0, shaderProgram );
            }
#endif

            // TODO: Background
            // TODO: Scene

            // End frame
            bgfx::frame();
        }

        // TODO: Logic
        // TODO: Handle application current input

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace runtime
