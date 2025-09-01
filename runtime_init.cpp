#include <SDL3/SDL_scancode.h>
#include <X11/Xlib.h>
#include <bgfx/defines.h>

#include "FPS.hpp"
#include "common.hpp"
#include "controls.hpp"
#include "log.hpp"
#include "runtime.hpp"

namespace runtime {

auto init( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    do {
        // Generate application state
        {
            // Metadata
            {
                log::info( "Window name: '{}', Version: '{}', Identifier: '{}'",
                           _applicationState.settings.window.name,
                           _applicationState.settings.version,
                           _applicationState.settings.identifier );

                if ( !SDL_SetAppMetadata(
                         std::string( _applicationState.settings.window.name )
                             .c_str(),
                         std::string( _applicationState.settings.version )
                             .c_str(),
                         std::string( _applicationState.settings.identifier )
                             .c_str() ) ) {
                    log::error( "Setting render scale: '{}'", SDL_GetError() );

                    break;
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
                    ( SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_OPENGL ) );

                log::variable( _applicationState.window );

                if ( !_applicationState.window ) {
                    log::error( "Window or Renderer creation: '{}'",
                                SDL_GetError() );

                    break;
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
                    l_initParameters.type = bgfx::RendererType::OpenGL;
                    l_initParameters.vendorId = BGFX_PCI_ID_NONE;

                    bgfx::PlatformData l_platformData{};

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

                                break;
                            }

                            Window l_windowNumber = SDL_GetNumberProperty(
                                l_properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER,
                                0 );

                            log::variable( l_windowNumber );

                            if ( !l_windowNumber ) {
                                log::error( "Obtaining X11 window" );

                                break;
                            }

                            l_platformData.nwh =
                                std::bit_cast< void* >( l_windowNumber );
                            l_platformData.ndt = l_display;
                        }

                        l_platformData.backBuffer = nullptr;
                        l_platformData.backBufferDS = nullptr;
                        l_platformData.context = nullptr;

                        // X11
                        l_platformData.type =
                            bgfx::NativeWindowHandleType::Default;
                    }

                    l_initParameters.platformData = l_platformData;

                    // Build init parameters resolution
                    {
                        int l_windowWidth = 0;
                        int l_windowHeight = 0;

                        if ( !SDL_GetWindowSize( _applicationState.window,
                                                 &l_windowWidth,
                                                 &l_windowHeight ) ) {
                            log::error( "Querying window size" );

                            break;
                        }

                        l_initParameters.resolution.width = l_windowWidth;
                        l_initParameters.resolution.height = l_windowHeight;

                        log::variable( l_initParameters.resolution.width );
                        log::variable( l_initParameters.resolution.height );

                        l_initParameters.resolution.reset =
                            BGFX_RESET_SRGB_BACKBUFFER;
                    }

#if defined( DEBUG )

                    l_initParameters.debug = true;

#endif
                }

                log::info( "Initializing renderer" );

                if ( !bgfx::init( l_initParameters ) ) {
                    log::error( "Initializing renderer" );

                    break;
                }

                log::info( "Current renderer: {}",
                           bgfx::getRendererName( bgfx::getRendererType() ) );

#if defined( DEBUG )

                bgfx::setDebug( BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS );

#endif

                bgfx::setViewClear( 0,
                                    ( BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH ) );

                bgfx::setViewRect( 0, 0, 0, _applicationState.width,
                                   _applicationState.height );
            }

            // TODO: Set new SDL3 things

            // Load resources
            if ( !_applicationState.load() ) {
                log::error( "Loading application state" );

                break;
            }
        }

        // Vsync
        if ( !vsync::init( _applicationState.settings.window.vsync,
                           _applicationState.settings.window.desiredFPS ) ) {
            log::error( "Initializing Vsync" );

            break;
        }

        // FPS
        FPS::init( _applicationState.totalFramesRendered );

        log::debug( "Initialized" );

        l_returnValue = true;
    } while ( false );

    return ( l_returnValue );
}

} // namespace runtime
