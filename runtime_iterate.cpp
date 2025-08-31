#include <bx/math.h>

#include <array>

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
                std::array< float, 16 > l_model{};
                bx::mtxRotateY( l_model.data(), static_cast< float >( l_t ) );

                {
                    const bx::Vec3 l_at = { 0.0f, 1.0f, 0.0f };
                    const bx::Vec3 l_eye = { 0.0f, 1.0f, -2.5f };

                    std::array< float, 16 > l_view{};
                    bx::mtxLookAt( l_view.data(), l_eye, l_at );

                    std::array< float, 16 > l_proj{};
                    bx::mtxProj(
                        l_proj.data(), 60.0f,
                        _applicationState.width / _applicationState.height,
                        0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth );
                    bgfx::setViewTransform( 0, l_view.data(), l_proj.data() );

                    // Set view 0 default viewport.
                    bgfx::setViewRect( 0, 0, 0, _applicationState.width,
                                       _applicationState.height );
                }

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
