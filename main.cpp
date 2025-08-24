#include <SDL3/SDL.h>
#include <X11/Xlib.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <thread>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace {

struct Mesh {
    bgfx::VertexBufferHandle vbh{ BGFX_INVALID_HANDLE };
    bgfx::IndexBufferHandle ibh{ BGFX_INVALID_HANDLE };
    bgfx::TextureHandle texture{ BGFX_INVALID_HANDLE };
    uint32_t indexCount{ 0 };
    uint32_t vertexCount{ 0 };
};

std::vector< Mesh > meshes;
bgfx::ProgramHandle g_program{ BGFX_INVALID_HANDLE };
bgfx::VertexLayout vertexLayout;
bgfx::UniformHandle s_texColor{ BGFX_INVALID_HANDLE };

// Function-like macros
// Universal
#define STRINGIFY( _value ) #_value
#define MACRO_TO_STRING( _macro ) STRINGIFY( _macro )

// Constants
// Universal
static inline constexpr const size_t g_oneSecondInMilliseconds = 1000;
static inline constexpr const size_t g_oneMillisecondInNanoseconds = 1000000;

// Window
static inline constexpr const std::string_view g_defaultWindowName =
    "atavistic-marmoset";

// Control
static inline constexpr const std::string_view g_controlAsStringUnknown =
    "UNKNOWN";

// Settings
static inline constexpr const std::string_view g_defaultSettingsVersion = "0.1";

template < typename T >
static inline constexpr auto millisecondsToNanoseconds( const T _milliseconds )
    -> size_t {
    return ( _milliseconds * g_oneMillisecondInNanoseconds );
}

namespace log {

namespace {

// Prefixes
#if defined( DEBUG )

inline constexpr const std::string_view g_logDebugPrefix = "DEBUG: ";

#endif

inline constexpr const std::string_view g_logInfoPrefix = "INFO: ";
inline constexpr const std::string_view g_logWarningPrefix = "WARNING: ";
inline constexpr const std::string_view g_logErrorPrefix = "ERROR: ";

} // namespace

void debug( const std::string_view _message ) {
#if defined( DEBUG )

    std::cout << g_logDebugPrefix << _message << "\n";

#endif
}

template < typename T >
void _variable( const std::string_view _message, const T& _variable ) {
    std::ostringstream l_stream;

    l_stream << _message << " = '" << _variable << "'";

    const std::string l_message = l_stream.str();

    debug( l_message );
}

#define variable( _variableToLog )                                    \
    _variable( __FILE_NAME__                                          \
               ":" MACRO_TO_STRING( __LINE__ ) " | " #_variableToLog, \
               _variableToLog );

void info( const std::string_view _message ) {
    std::cout << g_logInfoPrefix << _message << "\n";
}

void warning( const std::string_view _message ) {
    std::cerr << g_logWarningPrefix << _message << "\n";
}

// Function file:line | message
inline void _error( const std::string_view _message,
                    const char* _functionName,
                    const char* _fileName,
                    const char* _lineNumber ) {
    std::cerr << g_logErrorPrefix << "\"" << _functionName << "\"" << " "
              << _fileName << ":" << _lineNumber << " | " << _message << "\n";
}

#define error( _message )                                     \
    _error( ( _message ), __PRETTY_FUNCTION__, __FILE_NAME__, \
            MACRO_TO_STRING( __LINE__ ) )

} // namespace log

enum class vsync_t : uint8_t {
    off = 0,
    unknownVsync,
};

// Software vsync implementation
namespace vsync {

namespace {

vsync_t g_vsyncType = vsync_t::unknownVsync;
float g_desiredFPS = 0;

} // namespace

static struct timespec g_sleepTime, g_startTime, g_endTime;

auto init( const vsync_t _vsyncType, const float _desiredFPS ) -> bool {
    log::variable( _desiredFPS );

    bool l_returnValue = false;

    if ( g_desiredFPS ) {
        log::error( "Alraedy initialized" );

        goto EXIT;
    }

    {
        g_desiredFPS = _desiredFPS;
        g_vsyncType = _vsyncType;

        if ( _vsyncType == vsync_t::off ) {
            g_sleepTime.tv_sec = 0;
            g_sleepTime.tv_nsec = millisecondsToNanoseconds(
                g_oneSecondInMilliseconds /
                static_cast< float >( _desiredFPS ) );
        }

        log::info( std ::format( "Setting vsync to {} FPS", _desiredFPS ) );

        log::debug( std ::format( "Vsync sleep time set to {} nanoseconds",
                                  g_sleepTime.tv_nsec ) );

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

void quit() {
    g_desiredFPS = 0;

    if ( g_vsyncType == vsync_t::off ) {
        g_sleepTime.tv_nsec = 0;
    }
}

void begin() {
    if ( g_vsyncType == vsync_t::off ) {
        clock_gettime( CLOCK_MONOTONIC, &g_startTime );
    }
}

void end() {
    if ( g_vsyncType == vsync_t::off ) {
        clock_gettime( CLOCK_MONOTONIC, &g_endTime );

        struct timespec l_adjustedSleepTime{};

        {
            const size_t l_iterationTimeNano =
                ( g_endTime.tv_nsec - g_startTime.tv_nsec );

            long long l_adjustedSleepNano =
                ( g_sleepTime.tv_nsec - l_iterationTimeNano );

            l_adjustedSleepNano &= -( l_adjustedSleepNano > 0 );

            l_adjustedSleepTime.tv_sec = 0;
            l_adjustedSleepTime.tv_nsec = l_adjustedSleepNano;
        }

        clock_nanosleep( CLOCK_MONOTONIC, 0, &l_adjustedSleepTime, nullptr );
    }
}

} // namespace vsync

using window_t = struct window {
    window() = default;
    window( const window& ) = default;
    window( window&& ) = default;
    ~window() = default;
    auto operator=( const window& ) -> window& = default;
    auto operator=( window&& ) -> window& = default;

    static inline constexpr const std::string_view name = g_defaultWindowName;
    size_t width = 640;
    size_t height = 480;
    size_t desiredFPS = 60;
    vsync_t vsync = vsync_t::off;
};

enum class direction_t : uint8_t {
    none = 0,
    up = 0b1,
    down = 0b10,
    left = 0b100,
    right = 0b1000,
};

inline auto operator|=( direction_t& _lhs, direction_t _rhs ) -> direction_t& {
    using directionType_t = std::underlying_type_t< direction_t >;

    _lhs = static_cast< direction_t >( static_cast< directionType_t >( _lhs ) |
                                       static_cast< directionType_t >( _rhs ) );

    return ( _lhs );
}

inline auto operator|( direction_t _lhs, direction_t _rhs ) -> direction_t {
    using directionType_t = std::underlying_type_t< direction_t >;

    return (
        static_cast< direction_t >( static_cast< directionType_t >( _lhs ) |
                                    static_cast< directionType_t >( _rhs ) ) );
}

inline auto operator&( direction_t _lhs, direction_t _rhs ) -> direction_t {
    using directionType_t = std::underlying_type_t< direction_t >;

    return (
        static_cast< direction_t >( static_cast< directionType_t >( _lhs ) &
                                    static_cast< directionType_t >( _rhs ) ) );
}

enum class button_t : uint8_t {
    none = 0,
};

inline auto operator|=( button_t& _lhs, button_t _rhs ) -> button_t& {
    using buttonType_t = std::underlying_type_t< button_t >;

    _lhs = static_cast< button_t >( static_cast< buttonType_t >( _lhs ) |
                                    static_cast< buttonType_t >( _rhs ) );

    return ( _lhs );
}

inline auto operator|( button_t _lhs, button_t _rhs ) -> button_t {
    using buttonType_t = std::underlying_type_t< button_t >;

    return ( static_cast< button_t >( static_cast< buttonType_t >( _lhs ) |
                                      static_cast< buttonType_t >( _rhs ) ) );
}

inline auto operator&( button_t _lhs, button_t _rhs ) -> button_t {
    using buttonType_t = std::underlying_type_t< button_t >;

    return ( static_cast< button_t >( static_cast< buttonType_t >( _lhs ) &
                                      static_cast< buttonType_t >( _rhs ) ) );
}

using input_t = struct input {
    input() = default;
    input( const input& ) = default;
    input( input&& ) = default;
    ~input() = default;
    auto operator=( const input& ) -> input& = default;
    auto operator=( input&& ) -> input& = default;

    direction_t direction = direction_t::none;
    button_t button = button_t::none;
    size_t duration = 0;
};

using control_t = struct control {
    control() = default;
    control( const control& ) = default;
    control( control&& ) = default;
    ~control() = default;
    auto operator=( const control& ) -> control& = default;
    auto operator=( control&& ) -> control& = default;

    inline constexpr auto check( const SDL_Scancode _scancode ) -> bool {
        return ( scancode == _scancode );
    }

    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    input_t input;
};

// All available controls
using controls_t = struct controls {
    controls() = default;
    controls( const controls& ) = default;
    controls( controls&& ) = default;
    ~controls() = default;
    auto operator=( const controls& ) -> controls& = default;
    auto operator=( controls&& ) -> controls& = default;

    inline constexpr auto get( const SDL_Scancode _scancode )
        -> const control_t {
        if ( up.check( _scancode ) ) {
            return ( up );

        } else if ( down.check( _scancode ) ) {
            return ( down );

        } else if ( left.check( _scancode ) ) {
            return ( left );

        } else if ( right.check( _scancode ) ) {
            return ( right );
        }

        return {};
    }

    // Directions
    control_t up;
    control_t down;
    control_t left;
    control_t right;
};

// All available customization
using settings_t = struct settings {
    settings() = default;
    settings( const settings& ) = default;
    settings( settings&& ) = default;
    ~settings() = default;
    auto operator=( const settings& ) -> settings& = default;
    auto operator=( settings&& ) -> settings& = default;

    window_t window;
    controls_t controls;
    static inline constexpr const std::string_view version =
        g_defaultSettingsVersion;
    static inline constexpr const std::string_view identifier = window_t::name;
};

// TODO: Implement
using camera_t = struct camera {
    camera() = default;
    camera( const camera& ) = default;
    camera( camera&& ) = default;
    ~camera() = default;
    auto operator=( const camera& ) -> camera& = default;
    auto operator=( camera&& ) -> camera& = default;
};

namespace FPS {

namespace {

std::jthread g_loggerThread;

void logger( const std::stop_token& _stopToken,
             std::atomic< size_t >& _frameCount ) {
    using clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    auto l_timeLast = clock::now();

    while ( !_stopToken.stop_requested() ) {
        std::this_thread::sleep_for( 1s );

        const auto l_timeNow = clock::now();

        {
            const auto l_framesCount = _frameCount.exchange( 0 );

            const auto l_frameDuration = ( l_timeNow - l_timeLast );
            const std::chrono::duration< double > l_frameDurationInSeconds =
                l_frameDuration;

            double l_FPS = 0;

            if ( l_frameDurationInSeconds > 0s ) {
                l_FPS = ( l_framesCount / l_frameDurationInSeconds.count() );
            }

            log::info( std::format( "FPS: {:.2f}", l_FPS ) );
        }

        l_timeLast = l_timeNow;
    }

    log::info( "FPS logger stopped." );
}

} // namespace

void init( std::atomic< size_t >& _frameCount ) {
    g_loggerThread = std::jthread( logger, std::ref( _frameCount ) );
}

void quit() {
    g_loggerThread.request_stop();

    if ( g_loggerThread.joinable() ) {
        g_loggerThread.join();
    }
}

} // namespace FPS

namespace shader {

auto load( const std::string _name ) -> bgfx::ShaderHandle {
    std::ifstream l_file( _name, ( std::ios::binary | std::ios::ate ) );

    if ( !l_file.is_open() ) {
        return BGFX_INVALID_HANDLE;
    }

    const std::streamsize l_size = l_file.tellg();

    l_file.seekg( 0, std::ios::beg );

    const bgfx::Memory* l_mem = bgfx::alloc( uint32_t( l_size + 1 ) );

    if ( !l_file.read( reinterpret_cast< char* >( l_mem->data ), l_size ) ) {
        return BGFX_INVALID_HANDLE;
    }

    l_mem->data[ l_mem->size - 1 ] = '\0'; // null terminator, required

    return bgfx::createShader( l_mem );
}

} // namespace shader

namespace runtime {

using applicationState_t = struct applicationState {
    applicationState() = default;
    applicationState( const applicationState& ) = delete;
    applicationState( applicationState&& ) = delete;
    ~applicationState() = default;
    auto operator=( const applicationState& ) -> applicationState& = delete;
    auto operator=( applicationState&& ) -> applicationState& = delete;

    // TODO: Implement
    auto load() -> bool {
        bool l_ok = false;

        // --- load shaders
        {
            // Build program
            {
                bgfx::ShaderHandle l_vsh = shader::load( vertexShaderPath );

                if ( !bgfx::isValid( l_vsh ) ) {
                    log::error( "Loading vertex shader" );

                    goto EXIT;
                }

                bgfx::ShaderHandle l_fsh = shader::load( fragmentShaderPath );

                if ( !bgfx::isValid( l_fsh ) ) {
                    log::error( "Loading fragment shader" );

                    bgfx::destroy( l_vsh );

                    goto EXIT;
                }

                g_program = bgfx::createProgram( l_vsh, l_fsh, true );

                if ( !bgfx::isValid( g_program ) ) {
                    log::error( "Failed to create program" );

                    goto EXIT;
                }
            }

            // vertex layout: position (float3) + texcoord0 (float2)
            vertexLayout.begin()
                .add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
                .add( bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float )
                .end();

            s_texColor =
                bgfx::createUniform( "s_texColor", bgfx::UniformType::Sampler );
        }

        // --- load FBX using Assimp
        {
            Assimp::Importer l_importer;
            const aiScene* l_scene = l_importer.ReadFile(
                modelPath,
                aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                    aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                    aiProcess_ImproveCacheLocality | aiProcess_FlipUVs );

            if ( !l_scene ) {
                log::error( std::string( "Assimp ReadFile failed: " ) +
                            l_importer.GetErrorString() );
                goto EXIT;
            }

            // Verify counts
            {
                log::info(
                    std::format( "Assimp: meshes = {}, materials = {}, "
                                 "embedded textures = {}",
                                 l_scene->mNumMeshes, l_scene->mNumMaterials,
                                 l_scene->mNumTextures ) );
                // Print embedded textures if any
                for ( unsigned l_t = 0; l_t < l_scene->mNumTextures; ++l_t ) {
                    const aiTexture* l_at = l_scene->mTextures[ l_t ];
                    const char* l_name = ( l_at && l_at->mFilename.length )
                                             ? l_at->mFilename.C_Str()
                                             : "<no name>";
                    log::debug( std::format(
                        "Embedded texture[{}]: name='{}' width={} height={}",
                        l_t, l_name, l_at ? l_at->mWidth : 0,
                        l_at ? l_at->mHeight : 0 ) );
                }
            }

            if ( !l_scene->HasMeshes() ) {
                log::error( "FBX contains no meshes" );
                goto EXIT;
            }

            // Clear any existing meshes
            meshes.clear();

            // helper to create bgfx texture from raw RGBA pixels
            auto l_createBgfxTextureFromRgba =
                [ & ]( const unsigned char* _rgba, int _w,
                       int _h ) -> bgfx::TextureHandle {
                const auto l_size = uint32_t( _w * _h * 4 );
                const bgfx::Memory* l_mem = bgfx::alloc( l_size );
                memcpy( l_mem->data, _rgba, l_size );
                return bgfx::createTexture2D(
                    ( uint16_t )_w, ( uint16_t )_h, false, 1,
                    bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE, l_mem );
            };

            // helper to create texture from aiTexture (embedded)
            auto l_createTextureFromAiTexture =
                [ & ]( const aiTexture* _at ) -> bgfx::TextureHandle {
                if ( !_at )
                    return BGFX_INVALID_HANDLE;

                // compressed (mHeight == 0): at->pcData is a compressed image
                // (PNG/JPEG) of size at->mWidth
                if ( _at->mHeight == 0 && _at->mWidth > 0 ) {
                    int l_w = 0, l_h = 0, l_comp = 0;
                    stbi_uc* l_decoded = stbi_load_from_memory(
                        reinterpret_cast< const stbi_uc* >( _at->pcData ),
                        static_cast< int >( _at->mWidth ), &l_w, &l_h, &l_comp,
                        4 );
                    if ( !l_decoded ) {
                        log::warning(
                            "Failed to decode embedded compressed texture" );
                        return BGFX_INVALID_HANDLE;
                    }
                    bgfx::TextureHandle l_th =
                        l_createBgfxTextureFromRgba( l_decoded, l_w, l_h );
                    stbi_image_free( l_decoded );
                    return l_th;
                }

                // uncompressed: mWidth = width, mHeight = height and pcData raw
                // RGBA
                if ( _at->mHeight > 0 && _at->mWidth > 0 ) {
                    const int l_w = _at->mWidth;
                    const int l_h = _at->mHeight;
                    const auto* l_data =
                        reinterpret_cast< const unsigned char* >( _at->pcData );
                    return l_createBgfxTextureFromRgba( l_data, l_w, l_h );
                }

                return BGFX_INVALID_HANDLE;
            };

            // iterate all meshes
            for ( unsigned l_mi = 0; l_mi < l_scene->mNumMeshes; ++l_mi ) {
                const aiMesh* l_am = l_scene->mMeshes[ l_mi ];

                if ( !l_am->HasPositions() ) {
                    log::warning( std::format(
                        "Mesh[{}] has no positions, skipping", l_mi ) );
                    continue;
                }

                struct vertex {
                    float x, y, z, u, v;
                };
                std::vector< vertex > l_verts;
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
                std::vector< uint32_t > l_indices;
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
                    log::warning( std::format(
                        "Mesh[{}] empty verts or indices, skipping", l_mi ) );
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

                l_mesh.vbh = bgfx::createVertexBuffer( l_vbMem, vertexLayout );
                l_mesh.ibh = bgfx::createIndexBuffer( l_ibMem );

                // texture: prefer embedded in material or in scene->mTextures
                l_mesh.texture = BGFX_INVALID_HANDLE;

                if ( l_scene->HasMaterials() ) {
                    const aiMaterial* l_mat =
                        l_scene->mMaterials[ l_am->mMaterialIndex ];
                    if ( l_mat ) {
                        aiString l_texPath;
                        if ( l_mat->GetTextureCount( aiTextureType_DIFFUSE ) >
                                 0 &&
                             l_mat->GetTexture( aiTextureType_DIFFUSE, 0,
                                                &l_texPath ) == AI_SUCCESS ) {
                            std::string l_tpath = l_texPath.C_Str();

                            // Embedded textures in FBX can be referenced as
                            // "*0", "*1", etc.
                            if ( !l_tpath.empty() && l_tpath[ 0 ] == '*' ) {
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
                                        log::warning( std::format(
                                            "Failed to create texture from "
                                            "embedded index {}",
                                            l_idx ) );
                                    }
                                }
                            } else {
                                // try load from file (naive)
                                int l_w = 0, l_h = 0, l_comp = 0;
                                stbi_uc* l_data = stbi_load(
                                    l_tpath.c_str(), &l_w, &l_h, &l_comp, 4 );
                                if ( l_data ) {
                                    l_mesh.texture =
                                        l_createBgfxTextureFromRgba( l_data,
                                                                     l_w, l_h );
                                    stbi_image_free( l_data );
                                } else {
                                    log::warning(
                                        std::format( "Failed to load material "
                                                     "texture from disk: {}",
                                                     l_tpath ) );
                                }
                            }
                        }
                    }
                }

                // fallback: if no texture loaded, but scene has embedded
                // textures (single texture use case)
                if ( !bgfx::isValid( l_mesh.texture ) &&
                     l_scene->mNumTextures > 0 ) {
                    // try first embedded texture
                    const aiTexture* l_at = l_scene->mTextures[ 0 ];
                    l_mesh.texture = l_createTextureFromAiTexture( l_at );
                    if ( !bgfx::isValid( l_mesh.texture ) ) {
                        log::warning(
                            "Failed to create texture from "
                            "scene->mTextures[0]" );
                    }
                }

                // final fallback white texture
                if ( !bgfx::isValid( l_mesh.texture ) ) {
                    const uint8_t l_white[ 4 ] = { 0xFF, 0xFF, 0xFF, 0xFF };
                    const bgfx::Memory* l_mem = bgfx::makeRef( l_white, 4 );
                    l_mesh.texture = bgfx::createTexture2D(
                        1, 1, false, 1, bgfx::TextureFormat::RGBA8, 0, l_mem );
                }

                meshes.push_back( l_mesh );
                log::info( std::format(
                    "Loaded mesh[{}]: verts={}, indices={}, tex={}", l_mi,
                    l_mesh.vertexCount, l_mesh.indexCount,
                    bgfx::isValid( l_mesh.texture ) ) );
            } // end for meshes

            // setup simple camera view/proj so model is visible
            {
                float l_view[ 16 ];
                float l_proj[ 16 ];

                bx::Vec3 l_eye{ 0.0f, 0.0f, -5.0f };
                bx::Vec3 l_at{ 0.0f, 0.0f, 0.0f };
                bx::Vec3 l_up{ 0.0f, 1.0f, 0.0f };

                bx::mtxLookAt( l_view, l_eye, l_at );
                const float l_fov = 60.0f;
                const float l_aspect = float( width ) / float( height );
                bx::mtxProj( l_proj, l_fov, l_aspect, 0.1f, 100.0f,
                             bgfx::getCaps()->homogeneousDepth );
                bgfx::setViewTransform( 0, l_view, l_proj );
            }
        }

        l_ok = true;

    EXIT:
        return l_ok;
    }

    // TODO: Implement
    auto unload() -> bool {
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

        if ( bgfx::isValid( g_program ) ) {
            bgfx::destroy( g_program );
            g_program = BGFX_INVALID_HANDLE;
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

    SDL_Window* window = nullptr;
    size_t logicalWidth = 1280;
    size_t logicalHeight = 720;
    float width = logicalWidth;
    float height = logicalHeight;
    std::atomic< size_t > totalFramesRendered = 0;

    camera_t camera;
    settings_t settings;
    input_t currentInput;

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string modelPath;

    bool status = false;
};

auto init( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        // Generate application state
        {
            // Metadata
            {
                log::info( std::format(
                    "Window name: '{}', Version: '{}', Identifier: '{}'",
                    _applicationState.settings.window.name,
                    _applicationState.settings.version,
                    _applicationState.settings.identifier ) );

                if ( !SDL_SetAppMetadata(
                         std::string( _applicationState.settings.window.name )
                             .c_str(),
                         std::string( _applicationState.settings.version )
                             .c_str(),
                         std::string( _applicationState.settings.identifier )
                             .c_str() ) ) {
                    log::error( std::format( "Setting render scale: '{}'",
                                             SDL_GetError() ) );

                    goto EXIT;
                }
            }

            // Setup recources to load
            {
                _applicationState.fragmentShaderPath = "fs.bin";
                _applicationState.vertexShaderPath = "vs.bin";
                _applicationState.modelPath = "t.fbx";
            }

            // Init SDL sub-systems
            SDL_Init( SDL_INIT_VIDEO );

            // Window
            {
                _applicationState.window = SDL_CreateWindow(
                    std::string( _applicationState.settings.window.name )
                        .c_str(),
                    _applicationState.settings.window.width,
                    _applicationState.settings.window.height,
                    ( SDL_WINDOW_INPUT_FOCUS ) );

                log::variable( _applicationState.window );

                if ( !_applicationState.window ) {
                    log::error( std::format(
                        "Window or Renderer creation: '{}'", SDL_GetError() ) );

                    goto EXIT;
                }

                _applicationState.width =
                    _applicationState.settings.window.width;
                _applicationState.height =
                    _applicationState.settings.window.width;

                log::variable( _applicationState.width );
                log::variable( _applicationState.height );
            }

            // Renderer
            {
                bgfx::Init l_initParameters{};

                // Build init parameters
                {
                    l_initParameters.deviceId = 0;
                    l_initParameters.type = bgfx::RendererType::Count;
                    l_initParameters.vendorId = BGFX_PCI_ID_NONE;

                    bgfx::PlatformData l_pd{};

                    // Build platform data
                    {
                        // Get window handle and display
                        {
                            SDL_PropertiesID l_properties =
                                SDL_GetWindowProperties(
                                    _applicationState.window );

                            auto l_display =
                                static_cast< Display* >( SDL_GetPointerProperty(
                                    l_properties,
                                    SDL_PROP_WINDOW_X11_DISPLAY_POINTER,
                                    nullptr ) );

                            log::variable( l_display );

                            if ( !l_display ) {
                                log::error( "Obtaining X11 display" );

                                goto EXIT;
                            }

                            Window l_windowNumber = SDL_GetNumberProperty(
                                l_properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER,
                                0 );

                            log::variable( l_windowNumber );

                            if ( !l_windowNumber ) {
                                log::error( "Obtaining X11 window" );

                                goto EXIT;
                            }

                            l_pd.nwh =
                                reinterpret_cast< void* >( l_windowNumber );
                            l_pd.ndt = l_display;
                        }

                        l_pd.backBuffer = nullptr;
                        l_pd.backBufferDS = nullptr;
                        l_pd.context = nullptr;

                        // X11
                        l_pd.type = bgfx::NativeWindowHandleType::Default;
                    }

                    l_initParameters.platformData = l_pd;

                    // Build init parameters resolution
                    {
                        int l_windowWidth = 0;
                        int l_windowHeight = 0;

                        if ( !SDL_GetWindowSize( _applicationState.window,
                                                 &l_windowWidth,
                                                 &l_windowHeight ) ) {
                            log::error( "Querying window size" );

                            goto EXIT;
                        }

                        l_initParameters.resolution.width = l_windowWidth;
                        l_initParameters.resolution.height = l_windowHeight;

                        log::variable( l_initParameters.resolution.width );
                        log::variable( l_initParameters.resolution.height );

                        l_initParameters.resolution.reset = BGFX_RESET_NONE;
                    }

#if defined( DEBUG )

                    l_initParameters.debug = true;

#endif
                }

                if ( !bgfx::init( l_initParameters ) ) {
                    log::error( "Initializing renderer" );

                    goto EXIT;
                }

                log::info( std::format(
                    "Current renderer: {}",
                    bgfx::getRendererName( bgfx::getRendererType() ) ) );

#if defined( DEBUG )

                bgfx::setDebug( BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS );

#endif

                bgfx::setViewClear( 0,
                                    ( BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH ) );

                bgfx::setViewRect( 0, 0, 0, _applicationState.width,
                                   _applicationState.height );
            }

            // TODO: Default scale mode
            // TODO: Set new SDL3 things

            // Load resources
            if ( !_applicationState.load() ) {
                log::error( "Loading application state" );

                goto EXIT;
            }
        }

        // Vsync
        if ( !vsync::init( _applicationState.settings.window.vsync,
                           _applicationState.settings.window.desiredFPS ) ) {
            log::error( "Initializing Vsync" );

            goto EXIT;
        }

        // FPS
        // Does not fail
        FPS::init( _applicationState.totalFramesRendered );

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

void quit( applicationState_t& _applicationState ) {
    // Report if SDL error occured before quitting
    {
        const std::string_view l_errorMessage = SDL_GetError();

        if ( !l_errorMessage.empty() ) {
            log::error(
                std::format( "Application quit: '{}'", l_errorMessage ) );
        }
    }

    // FPS
    FPS::quit();

    // Vsync
    vsync::quit();

    // BGFX
    bgfx::shutdown();

    // Application state
    {
        if ( !_applicationState.unload() ) {
            log::error( "Unloading application state" );
        }

        // Report if SDL error occured during quitting
        {
            const std::string_view l_errorMessage = SDL_GetError();

            if ( !l_errorMessage.empty() ) {
                log::error( std::format( "Application shutdown: '{}'",
                                         l_errorMessage ) );
            }
        }

        if ( _applicationState.window ) {
            SDL_DestroyWindow( _applicationState.window );
        }
    }

    SDL_Quit();
}

using event_t = SDL_Event;

namespace {

auto onWindowResize( applicationState_t& _applicationState,
                     const float _width,
                     const float _height ) -> bool {
    bool l_returnValue = false;

    {
        _applicationState.width = _width;
        _applicationState.height = _height;

        bgfx::reset( _width, _height );

        bgfx::setViewRect( 0, 0, 0, _width, _height );

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

auto handleKeyboardState( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        static size_t l_lastInputFrame = 0;
        const size_t l_totalFramesRendered =
            _applicationState.totalFramesRendered;

        if ( l_lastInputFrame < l_totalFramesRendered ) {
            input_t l_input;

            {
                int l_keysAmount = 0;
                const bool* l_keysState = SDL_GetKeyboardState( &l_keysAmount );

                __builtin_assume( l_keysAmount == SDL_SCANCODE_COUNT );

                for ( auto [ _index, _isPressed ] :
                      std::span( l_keysState, l_keysAmount ) |
                          std::views::enumerate ) {
                    if ( _isPressed ) {
                        auto l_scancode = static_cast< SDL_Scancode >( _index );

                        const control_t& l_control =
                            _applicationState.settings.controls.get(
                                l_scancode );

                        if ( l_control.scancode != SDL_SCANCODE_UNKNOWN ) {
                            l_input.direction |= l_control.input.direction;
                            l_input.button |= l_control.input.button;
                        }
                    }
                }
            }

            _applicationState.currentInput = l_input;
        }

        l_lastInputFrame = l_totalFramesRendered;

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace

auto event( applicationState_t& _applicationState, const event_t& _event )
    -> bool {
    bool l_returnValue = false;

    {
        const bool l_isEventEmpty = ( _event.type == 0 );

        // Empty means last event on current frame
        if ( l_isEventEmpty ) {
            l_returnValue = handleKeyboardState( _applicationState );

            if ( !l_returnValue ) {
                log::error( "Handling keyboard state" );

                goto EXIT;
            }

        } else {
            switch ( _event.type ) {
                case SDL_EVENT_QUIT: {
                    _applicationState.status = true;

                    l_returnValue = false;

                    goto EXIT;
                }

                case SDL_EVENT_WINDOW_RESIZED: {
                    const float l_newWidth = _event.window.data1;
                    const float l_newHeight = _event.window.data2;

                    l_returnValue = onWindowResize( _applicationState,
                                                    l_newWidth, l_newHeight );

                    if ( !l_returnValue ) {
                        log::error( "Handling window resize" );

                        goto EXIT;
                    }

                    break;
                }

                default: {
                }
            }
        }

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

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
                bgfx::submit( 0, g_program );
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

} // namespace

void printSupportedRenderers() {
    using rendererType_t = std::underlying_type_t< bgfx::RendererType::Enum >;

    std::array< bgfx::RendererType::Enum, bgfx::RendererType::Enum::Count >
        l_supportedRenderers{};

    const rendererType_t l_supportedRenderersAmount =
        bgfx::getSupportedRenderers( bgfx::getSupportedRenderers(),
                                     l_supportedRenderers.data() );

    log::debug( "Supported renderers:" );

    for ( rendererType_t _index :
          std::views::iota( static_cast< rendererType_t >( 0 ),
                            l_supportedRenderersAmount ) ) {
        log::debug( std::format(
            " - {}",
            bgfx::getRendererName( l_supportedRenderers.at( _index ) ) ) );
    }
}

auto main() -> int {
    runtime::applicationState_t l_applicationState;

    printSupportedRenderers();

    {
        if ( !runtime::init( l_applicationState ) ) {
            goto EXIT;
        }

        for ( ;; ) {
            vsync::begin();

            SDL_PumpEvents();

            runtime::event_t l_event{};

            while ( SDL_PollEvent( &l_event ) ) {
                if ( !runtime::event( l_applicationState, l_event ) ) {
                    goto EXIT;
                }
            }

            // NULL means last event on current frame
            if ( !runtime::event( l_applicationState, {} ) ) {
                break;
            }

            if ( !runtime::iterate( l_applicationState ) ) {
                break;
            }

            vsync::end();

            ( l_applicationState.totalFramesRendered )++;
        }
    }

EXIT:
    runtime::quit( l_applicationState );

    return ( ( l_applicationState.status ) ? ( EXIT_SUCCESS )
                                           : ( EXIT_FAILURE ) );
}
