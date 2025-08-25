#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

#include <ranges>

#include "log.hpp"
#include "runtime.hpp"
#include "vsync.hpp"

static void printSupportedRenderers() {
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
