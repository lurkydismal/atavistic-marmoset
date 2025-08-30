#include "runtime.hpp"

namespace runtime {

// TODO: Implement
auto applicationState_t::unload() -> bool {
    // Destroy mesh resources
#if 0
        for ( auto& mesh : meshes ) {
            if ( bgfx::isValid( mesh.vbh ) ) {
                bgfx::destroy( mesh.vbh );
                mesh.vbh = BGFX_INVALID_HANDLE;
            }
            if ( bgfx::isValid( mesh.ibh ) ) {
                bgfx::destroy( mesh.ibh );
                mesh.ibh = BGFX_INVALID_HANDLE;
            }
            if ( bgfx::isValid( mesh.texture ) ) {
                bgfx::destroy( mesh.texture );
                mesh.texture = BGFX_INVALID_HANDLE;
            }
        }
        meshes.clear();

        if ( bgfx::isValid( shaderProgram ) ) {
            bgfx::destroy( shaderProgram );
            shaderProgram = BGFX_INVALID_HANDLE;
        }
        if ( bgfx::isValid( s_texColor ) ) {
            bgfx::destroy( s_texColor );
            s_texColor = BGFX_INVALID_HANDLE;
        }

        // vertexLayout has no destroy func; it's just an object. Reset it.
        vertexLayout = bgfx::VertexLayout();
#endif

    return true;
}

} // namespace runtime
