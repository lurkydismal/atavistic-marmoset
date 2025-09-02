#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bgfx/defines.h>
#include <bx/math.h>

#include <print>
#include <vector>

#include "bgfx_helper.hpp"
#include "runtime.hpp"
#include "shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace runtime {

namespace {

bgfx::VertexLayout vertexLayout;

} // namespace

// TODO: Implement
auto applicationState_t::load() -> bool {
    bool l_returnValue = false;

    do {
        if ( vertexShaderPath.empty() ) {
            std::println( "{}", vertexShaderPath );

            std::println( "Vertex shader path is empty" );

            break;
        }

        if ( fragmentShaderPath.empty() ) {
            std::println( "{}", fragmentShaderPath );

            std::println( "Fragment shader path is empty" );

            break;
        }

        if ( bgfx::isValid( shaderProgram ) ) {
            std::println( "{}", shaderProgram );

            std::println( "Shader program is already loaded" );

            break;
        }

        {
            // Build shader program
            {
                std::println( "Loading vertex shader" );

                bgfx::ShaderHandle l_vertexShaderHandle =
                    shader::load( vertexShaderPath );

                if ( !bgfx::isValid( l_vertexShaderHandle ) ) {
                    std::println( "Loading vertex shader" );

                    break;
                }

                std::println( "Loading fragment shader" );

                bgfx::ShaderHandle l_fragmentShaderHandle =
                    shader::load( fragmentShaderPath );

                if ( !bgfx::isValid( l_fragmentShaderHandle ) ) {
                    std::println( "Loading fragment shader" );

                    bgfx::destroy( l_vertexShaderHandle );

                    break;
                }

                std::println( "Compiling shaders" );

                shaderProgram = bgfx::createProgram(
                    l_vertexShaderHandle, l_fragmentShaderHandle, true );

                if ( !bgfx::isValid( shaderProgram ) ) {
                    std::println( "Failed to create program" );

                    break;
                }
            }

            {
                // vertex layout: position (float3) + texcoord0 (float2)
                vertexLayout.begin()
                    .add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
                    .add( bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float )
                    .end();

#if 1
                s_texColor = bgfx::createUniform( "s_texColor",
                                                  bgfx::UniformType::Sampler );
#endif
            }

            // Load FBX using Assimp
            {
                Assimp::Importer l_importer;
                const aiScene* l_scene = l_importer.ReadFile(
                    modelPath,
                    aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                        aiProcess_GenSmoothNormals |
                        aiProcess_ImproveCacheLocality | aiProcess_FlipUVs );

                if ( !l_scene ) {
                    std::println( "Assimp ReadFile failed: {}",
                                  l_importer.GetErrorString() );

                    break;
                }

                // Verify counts
                {
                    std::println(
                        "Assimp: meshes = {}, materials = {}, "
                        "embedded textures = {}",
                        l_scene->mNumMeshes, l_scene->mNumMaterials,
                        l_scene->mNumTextures );

                    // Print embedded textures if any
                    for ( unsigned l_t = 0; l_t < l_scene->mNumTextures;
                          ++l_t ) {
                        const aiTexture* l_at = l_scene->mTextures[ l_t ];
                        const char* l_name = ( l_at && l_at->mFilename.length )
                                                 ? l_at->mFilename.C_Str()
                                                 : "<no name>";
                        std::println(
                            "Embedded texture[{}]: name='{}' width={} "
                            "height={}",
                            l_t, l_name, l_at ? l_at->mWidth : 0,
                            l_at ? l_at->mHeight : 0 );
                    }
                }

                if ( !l_scene->HasMeshes() ) {
                    std::println( "FBX contains no meshes" );

                    break;
                }

                // Clear any existing meshes
                meshes.clear();

#if 1
                auto l_createBgfxTextureFromRgba =
                    [ & ]( const unsigned char* _rgba, int _w, int _h,
                           bool _srgb = true ) -> bgfx::TextureHandle {
                    if ( nullptr == _rgba || _w <= 0 || _h <= 0 ) {
                        std::println(
                            "l_createBgfxTextureFromRgba: invalid inputs {}x{}",
                            _w, _h );
                        return BGFX_INVALID_HANDLE;
                    }

                    const uint32_t l_size =
                        uint32_t( _w ) * uint32_t( _h ) * 4u;
                    const bgfx::Memory* l_mem = bgfx::alloc( l_size );
                    memcpy( l_mem->data, _rgba, l_size );

                    uint64_t flags = 0;
                    if ( _srgb ) {
                        flags |= BGFX_TEXTURE_SRGB;
                    }
                    // Keep wrap/filter defaults. If you want clamp: flags |=
                    // BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

                    bgfx::TextureHandle th = bgfx::createTexture2D(
                        ( uint16_t )_w, ( uint16_t )_h,
                        false, // hasMips = false (safer). Turn true if
                               // providing mips.
                        1,     // layers
                        bgfx::TextureFormat::RGBA8, flags, l_mem );

                    if ( !bgfx::isValid( th ) ) {
                        std::println(
                            "bgfx::createTexture2D failed for {}x{} (srgb={})",
                            _w, _h, _srgb );
                    } else {
                        std::println( "bgfx texture created {}x{} (srgb={})",
                                      _w, _h, _srgb );
                    }
                    return th;
                };

                // helper to create bgfx texture from raw RGBA pixels
#if 0
                auto l_createBgfxTextureFromRgba =
                    [ & ]( const unsigned char* _rgba, int _w,
                           int _h ) -> bgfx::TextureHandle {
                    const auto l_size = uint32_t( _w * _h * 4 );
                    const bgfx::Memory* l_mem = bgfx::alloc( l_size );
                    memcpy( l_mem->data, _rgba, l_size );
                    return bgfx::createTexture2D(
                        ( uint16_t )_w, ( uint16_t )_h, false, 1,
                        bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_SRGB, l_mem );
                };
#endif

                auto l_createTextureFromAiTexture =
                    [ & ]( const aiTexture* _at ) -> bgfx::TextureHandle {
                    if ( !_at ) {
                        std::println( "aiTexture is null" );
                        return BGFX_INVALID_HANDLE;
                    }

                    // Compressed PNG/JPEG (most FBX embed cases): mHeight == 0,
                    // mWidth == byte_size
                    if ( _at->mHeight == 0 && _at->mWidth > 0 ) {
                        int l_w = 0, l_h = 0, l_comp = 0;
                        const unsigned char* blob =
                            reinterpret_cast< const unsigned char* >(
                                _at->pcData );
                        const int blobSize = static_cast< int >( _at->mWidth );

                        stbi_uc* decoded = stbi_load_from_memory(
                            blob, blobSize, &l_w, &l_h, &l_comp, 4 );
                        if ( !decoded ) {
                            std::println(
                                "stbi_load_from_memory failed for embedded "
                                "texture (bytes = {})",
                                blobSize );
                            return BGFX_INVALID_HANDLE;
                        }

                        std::println( "Decoded embedded texture: {}x{} comp={}",
                                      l_w, l_h, l_comp );
                        // Optional: write to disk for inspection (requires
                        // stb_image_write) stbi_write_png("embedded_dump.png",
                        // l_w, l_h, 4, decoded, l_w * 4);

                        bgfx::TextureHandle th = l_createBgfxTextureFromRgba(
                            decoded, l_w, l_h, true /*srgb*/ );
                        stbi_image_free( decoded );
                        return th;
                    }

                    // Raw aiTexel data (rare for your case). Convert aiTexel ->
                    // RGBA
                    if ( _at->mHeight > 0 && _at->mWidth > 0 ) {
                        const int l_w = _at->mWidth;
                        const int l_h = _at->mHeight;
                        std::vector< unsigned char > l_rgba;
                        l_rgba.resize( size_t( l_w ) * size_t( l_h ) * 4u );
                        for ( int i = 0; i < l_w * l_h; ++i ) {
                            l_rgba[ i * 4 + 0 ] = _at->pcData[ i ].r;
                            l_rgba[ i * 4 + 1 ] = _at->pcData[ i ].g;
                            l_rgba[ i * 4 + 2 ] = _at->pcData[ i ].b;
                            l_rgba[ i * 4 + 3 ] = _at->pcData[ i ].a;
                        }
                        return l_createBgfxTextureFromRgba(
                            l_rgba.data(), l_w, l_h, true /*srgb*/ );
                    }

                    std::println(
                        "aiTexture had no usable data (mWidth={} mHeight={})",
                        _at->mWidth, _at->mHeight );
                    return BGFX_INVALID_HANDLE;
                };

                // helper to create texture from aiTexture (embedded)
#if 0
                auto l_createTextureFromAiTexture =
                    [ & ]( const aiTexture* _at ) -> bgfx::TextureHandle {
                    if ( !_at )
                        return BGFX_INVALID_HANDLE;

                    // compressed (mHeight == 0): at->pcData is a compressed
                    // image (PNG/JPEG) of size at->mWidth
                    if ( _at->mHeight == 0 && _at->mWidth > 0 ) {
                        int l_w = 0, l_h = 0, l_comp = 0;
                        stbi_uc* l_decoded = stbi_load_from_memory(
                            reinterpret_cast< const stbi_uc* >( _at->pcData ),
                            static_cast< int >( _at->mWidth ), &l_w, &l_h,
                            &l_comp, 4 );
                        if ( !l_decoded ) {
                            std::println(
                                "Failed to decode embedded compressed "
                                "texture" );
                            return BGFX_INVALID_HANDLE;
                        }
                        bgfx::TextureHandle l_th =
                            l_createBgfxTextureFromRgba( l_decoded, l_w, l_h );
                        stbi_image_free( l_decoded );
                        return l_th;
                    }

                    return BGFX_INVALID_HANDLE;
                };
#endif
#endif

                // iterate all meshes
                {
                    unsigned l_mi = 0;
                    const aiMesh* l_am = l_scene->mMeshes[ l_mi ];

                    if ( !l_am->HasPositions() ) {
                        std::println( "Mesh[{}] has no positions, skipping",
                                      l_mi );
                        continue;
                    }

                    struct vertex {
                        float x, y, z;
                        float u, v;
                    };
                    static std::vector< vertex > l_verts;
                    l_verts.reserve( l_am->mNumVertices );

                    for ( unsigned l_i = 0; l_i < l_am->mNumVertices; ++l_i ) {
                        vertex l_v{};
                        l_v.x = l_am->mVertices[ l_i ].x;
                        l_v.y = l_am->mVertices[ l_i ].y;
                        l_v.z = l_am->mVertices[ l_i ].z;
#if 1
                        if ( l_am->HasTextureCoords( 0 ) ) {
                            l_v.u = l_am->mTextureCoords[ 0 ][ l_i ].x;
                            l_v.v = l_am->mTextureCoords[ 0 ][ l_i ].y;
                        } else {
#endif
                            l_v.u = 0.0f;
                            l_v.v = 0.0f;
#if 1
                        }
#endif
                        l_verts.push_back( l_v );
                    }

                    // indices
                    static std::vector< uint32_t > l_indices;
                    l_indices.reserve( l_am->mNumFaces * 3 );
                    for ( unsigned l_f = 0; l_f < l_am->mNumFaces; ++l_f ) {
                        const aiFace& l_face = l_am->mFaces[ l_f ];
                        if ( l_face.mNumIndices != 3 )
                            continue;
                        l_indices.push_back( l_face.mIndices[ 0 ] );
                        l_indices.push_back( l_face.mIndices[ 1 ] );
                        l_indices.push_back( l_face.mIndices[ 2 ] );
                    }

                    if ( l_verts.empty() || l_indices.empty() ) {
                        std::println(
                            "Mesh[{}] empty verts or indices, skipping", l_mi );
                        continue;
                    }

                    // create vertex/index buffers
                    const bgfx::Memory* l_vbMem = bgfx::makeRef(
                        l_verts.data(),
                        uint32_t( l_verts.size() * sizeof( vertex ) ) );
                    const bgfx::Memory* l_ibMem = bgfx::makeRef(
                        l_indices.data(),
                        uint32_t( l_indices.size() * sizeof( uint32_t ) ) );

                    Mesh l_mesh{};
                    l_mesh.vertexCount = uint32_t( l_verts.size() );
                    l_mesh.indexCount = uint32_t( l_indices.size() );

                    l_mesh.vbh =
                        bgfx::createVertexBuffer( l_vbMem, vertexLayout );
                    l_mesh.ibh =
                        bgfx::createIndexBuffer( l_ibMem, BGFX_BUFFER_INDEX32 );

                    // texture: prefer embedded in material or in
                    // scene->mTextures
                    l_mesh.texture = BGFX_INVALID_HANDLE;

#if 0
                    if ( l_scene->HasMaterials() ) {
                        const aiMaterial* l_mat =
                            l_scene->mMaterials[ l_am->mMaterialIndex ];
                        if ( l_mat ) {
                            aiString l_texPath;
                            if ( l_mat->GetTextureCount(
                                     aiTextureType_DIFFUSE ) > 0 &&
                                 l_mat->GetTexture( aiTextureType_DIFFUSE, 0,
                                                    &l_texPath ) ==
                                     AI_SUCCESS ) {
                                std::string l_tpath = l_texPath.C_Str();

                                std::println( "Texture path: {}", l_tpath );

                                std::println( "Loading texture from memory" );

                                // get embedded texture index
                                int l_idx = std::atoi( l_tpath.c_str() + 1 );
                                if ( l_idx >= 0 &&
                                     l_idx < static_cast< int >(
                                                 l_scene->mNumTextures ) ) {
                                    const aiTexture* l_at =
                                        l_scene->mTextures[ l_idx ];
                                    l_mesh.texture =
                                        l_createTextureFromAiTexture( l_at );
                                    if ( !bgfx::isValid( l_mesh.texture ) ) {
                                        std::println(
                                            "Failed to create texture from "
                                            "embedded index {}",
                                            l_idx );
                                    }
                                }
                            }
                        }
                    }
#endif

#if 1
                    // fallback: if no texture loaded, but scene has embedded
                    // textures (single texture use case)
                    if ( !bgfx::isValid( l_mesh.texture ) &&
                         l_scene->mNumTextures > 0 ) {
                        // try first embedded texture
                        const aiTexture* l_at = l_scene->mTextures[ 0 ];
#if 1
                        l_mesh.texture = l_createTextureFromAiTexture( l_at );
#endif
                        if ( !bgfx::isValid( l_mesh.texture ) ) {
                            std::println(
                                "Failed to create texture from "
                                "scene->mTextures[0]" );
                        }
                    }
#endif

                    // final fallback white texture
                    if ( !bgfx::isValid( l_mesh.texture ) ) {
                        static const uint8_t l_white[ 4 ] = { 0xFF, 0xFF, 0xFF,
                                                              0xFF };
                        const bgfx::Memory* l_mem = bgfx::makeRef( l_white, 4 );
                        l_mesh.texture = bgfx::createTexture2D(
                            1, 1, false, 1, bgfx::TextureFormat::RGBA8,
                            BGFX_TEXTURE_SRGB, l_mem );
                    }

                    meshes.push_back( l_mesh );
                    std::println(
                        "Loaded mesh[{}]: verts={}, indices={}, tex={}", l_mi,
                        l_mesh.vertexCount, l_mesh.indexCount,
                        bgfx::isValid( l_mesh.texture ) );
                } // end for meshes
            }

            // TODO: Implement with camera_t
            // TODO: Reduntant
            {
#if 0
                using mat4x4_t = std::array< float, 16 >;

                mat4x4_t l_view{};
                mat4x4_t l_proj{};

                bx::Vec3 l_eye{ 0.0f, 0.0f, -5.0f };
                bx::Vec3 l_at{ 0.0f, 0.0f, 0.0f };

                bx::mtxLookAt( l_view.data(), l_eye, l_at );

                constexpr const float l_FOV = 60.0f;
                const float l_aspect = ( width / height );

                bx::mtxProj( l_proj.data(), l_FOV, l_aspect, 0.1f, 100.0f,
                             bgfx::getCaps()->homogeneousDepth );

                bgfx::setViewTransform( 0, l_view.data(), l_proj.data() );
#endif

                bgfx::setViewClear( 0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                                    0x303030ff );
            }
        }

        l_returnValue = true;
    } while ( false );

    return ( l_returnValue );
}

} // namespace runtime
