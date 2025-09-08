#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bgfx/bgfx.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <print>
#include <vector>

#include "assimp_helder.hpp"

auto loadModel( const std::string& _modelPath ) -> const aiScene* {
    const aiScene* l_scene = nullptr;

    static Assimp::Importer l_importer;

    l_scene = l_importer.ReadFile(
        _modelPath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                        aiProcess_GenSmoothNormals |
                        aiProcess_ImproveCacheLocality | aiProcess_FlipUVs );

    if ( !l_scene ) {
        std::println( "Assimp ReadFile failed: {}",
                      l_importer.GetErrorString() );

        return nullptr;
    }

    // Verify counts
    {
        std::println(
            "Assimp: meshes = {}, materials = {}, "
            "embedded textures = {}",
            l_scene->mNumMeshes, l_scene->mNumMaterials,
            l_scene->mNumTextures );

        // Print embedded textures if any
        for ( unsigned l_t = 0; l_t < l_scene->mNumTextures; ++l_t ) {
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

        return nullptr;
    }

    return l_scene;
}

void loadMaterial( runtime::mesh_t& _mesh,
                   const aiScene* _scene,
                   const aiMesh* _aiMesh ) {
    auto l_createBgfxTextureFromRgba =
        [ & ]( const unsigned char* _rgba, int _w, int _h,
               bool _srgb = true ) -> bgfx::TextureHandle {
        if ( nullptr == _rgba || _w <= 0 || _h <= 0 ) {
            std::println( "l_createBgfxTextureFromRgba: invalid inputs {}x{}",
                          _w, _h );
            return BGFX_INVALID_HANDLE;
        }

        const uint32_t l_size = uint32_t( _w ) * uint32_t( _h ) * 4u;
        const bgfx::Memory* l_mem = bgfx::alloc( l_size );
        memcpy( l_mem->data, _rgba, l_size );

        uint64_t flags = 0;
        if ( _srgb ) {
            flags |= BGFX_TEXTURE_SRGB;
        }
        // Keep wrap/filter defaults. If you want clamp: flags |=
        // BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

        bgfx::TextureHandle th =
            bgfx::createTexture2D( ( uint16_t )_w, ( uint16_t )_h,
                                   false, // hasMips = false (safer). Turn true
                                          // if providing mips.
                                   1,     // layers
                                   bgfx::TextureFormat::RGBA8, flags, l_mem );

        if ( !bgfx::isValid( th ) ) {
            std::println( "bgfx::createTexture2D failed for {}x{} (srgb={})",
                          _w, _h, _srgb );
        } else {
            std::println( "bgfx texture created {}x{} (srgb={})", _w, _h,
                          _srgb );
        }
        return th;
    };

    // helper to create bgfx texture from raw RGBA pixels
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
                reinterpret_cast< const unsigned char* >( _at->pcData );
            const int blobSize = static_cast< int >( _at->mWidth );

            stbi_uc* decoded =
                stbi_load_from_memory( blob, blobSize, &l_w, &l_h, &l_comp, 4 );
            if ( !decoded ) {
                std::println(
                    "stbi_load_from_memory failed for embedded "
                    "texture (bytes = {})",
                    blobSize );
                return BGFX_INVALID_HANDLE;
            }

            std::println( "Decoded embedded texture: {}x{} comp={}", l_w, l_h,
                          l_comp );
            // Optional: write to disk for inspection (requires
            // stb_image_write) stbi_write_png("embedded_dump.png",
            // l_w, l_h, 4, decoded, l_w * 4);

            bgfx::TextureHandle th =
                l_createBgfxTextureFromRgba( decoded, l_w, l_h, true /*srgb*/ );
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
            return l_createBgfxTextureFromRgba( l_rgba.data(), l_w, l_h,
                                                true /*srgb*/ );
        }

        std::println( "aiTexture had no usable data (mWidth={} mHeight={})",
                      _at->mWidth, _at->mHeight );
        return BGFX_INVALID_HANDLE;
    };

    if ( _scene->HasMaterials() ) {
        const aiMaterial* l_mat = _scene->mMaterials[ _aiMesh->mMaterialIndex ];

        if ( l_mat ) {
            aiString l_texPath;
            if ( l_mat->GetTextureCount( aiTextureType_DIFFUSE ) > 0 &&
                 l_mat->GetTexture( aiTextureType_DIFFUSE, 0, &l_texPath ) ==
                     AI_SUCCESS ) {
                std::string l_tpath = l_texPath.C_Str();

                std::println( "Texture path: {}", l_tpath );

                std::println( "Loading texture from memory" );

                // get embedded texture index
                int l_idx = std::atoi( l_tpath.c_str() + 1 );
                std::println( "Loading texture from memory 2: {} < {}", l_idx,
                              static_cast< int >( _scene->mNumTextures ) );

                if ( l_idx >= 0 &&
                     l_idx < static_cast< int >( _scene->mNumTextures ) ) {
                    const aiTexture* l_at = _scene->mTextures[ l_idx ];
                    std::println( "Loading texture from memory 3" );
                    _mesh.textureHandle = l_createTextureFromAiTexture( l_at );
                    if ( !bgfx::isValid( _mesh.textureHandle ) ) {
                        std::println(
                            "Failed to create texture from "
                            "embedded index {}",
                            l_idx );
                    }
                }
            }
        }
    }
}

void loadMesh( runtime::mesh_t& _mesh,
               bgfx::VertexLayout& _vertexLayout,
               const std::string& _scenePath ) {
    const aiScene* l_scene = loadModel( _scenePath );

    unsigned l_mi = 0;
    const aiMesh* l_am = l_scene->mMeshes[ l_mi ];

    if ( !l_am->HasPositions() ) {
        std::println( "Mesh[{}] has no positions, skipping", l_mi );

        return;
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
        if ( l_am->HasTextureCoords( 0 ) ) {
            l_v.u = l_am->mTextureCoords[ 0 ][ l_i ].x;
            l_v.v = l_am->mTextureCoords[ 0 ][ l_i ].y;
        } else {
            l_v.u = 0.0f;
            l_v.v = 0.0f;
        }
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
        std::println( "Mesh[{}] empty verts or indices, skipping", l_mi );

        return;
    }

    // create vertex/index buffers
    const bgfx::Memory* l_vbMem = bgfx::makeRef(
        l_verts.data(), uint32_t( l_verts.size() * sizeof( vertex ) ) );
    const bgfx::Memory* l_ibMem = bgfx::makeRef(
        l_indices.data(), uint32_t( l_indices.size() * sizeof( uint32_t ) ) );

    _mesh.vertexCount = uint32_t( l_verts.size() );
    _mesh.indexCount = uint32_t( l_indices.size() );

    _mesh.vertexBufferHandle =
        bgfx::createVertexBuffer( l_vbMem, _vertexLayout );
    _mesh.indexBufferHandle =
        bgfx::createIndexBuffer( l_ibMem, BGFX_BUFFER_INDEX32 );

    // texture: prefer embedded in material or in
    // scene->mTextures
    _mesh.textureHandle = BGFX_INVALID_HANDLE;

    loadMaterial( _mesh, l_scene, l_am );

    // White texture fallback
    if ( !bgfx::isValid( _mesh.textureHandle ) ) {
        std::println( "Loaded white texture" );

        static const std::array< uint8_t, 4 > l_white = { 0xFF, 0xFF, 0xFF,
                                                          0xFF };

        const bgfx::Memory* l_whiteAllocated =
            bgfx::makeRef( l_white.data(), 4 );

        _mesh.textureHandle =
            bgfx::createTexture2D( 1, 1, false, 1, bgfx::TextureFormat::RGBA8,
                                   BGFX_TEXTURE_NONE, l_whiteAllocated );
    }

    std::println( "Loaded mesh[{}]: verts={}, indices={}, tex={}", l_mi,
                  _mesh.vertexCount, _mesh.indexCount,
                  bgfx::isValid( _mesh.textureHandle ) );
}
